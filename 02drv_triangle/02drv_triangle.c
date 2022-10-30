#include "board.h"
#include "dn_ipmt.h"

//=========================== defines =========================================

const uint8_t APP_VERSION[]       = {0x00,0x02};

// mote state
#define MOTE_STATE_IDLE           0x01
#define MOTE_STATE_SEARCHING      0x02
#define MOTE_STATE_NEGOCIATING    0x03
#define MOTE_STATE_CONNECTED      0x04
#define MOTE_STATE_OPERATIONAL    0x05

// service types
#define SERVICE_TYPE_BW           0x00

// timings
#define SERIAL_RESPONSE_TIMEOUT   16384          // 16384@32kHz = 500ms
#define CMD_PERIOD                32768          // 32768@32kHz = 1s
#define ONE_SEC                   32768          // 32768@32kHz = 1s
#define DATA_PERIOD_S                10          // number of second between data packets

// app
#define NUM_STEPS                 8
#define DIRECTION_UP              1
#define DIRECTION_DOWN            0

// api
#define SRC_PORT                  0xf0b8
#define DST_PORT                  0xf0b8

//=========================== typedef =========================================

typedef void (*fsm_timer_callback)(void);
typedef void (*fsm_reply_callback)(void);

//=========================== variables ========================&===============

typedef struct {
    // fsm
    fsm_timer_callback  fsmCb;
    // reply
    fsm_reply_callback  replyCb;
    // app
    uint8_t             counter;
    uint8_t             secUntilTx;
    uint8_t             direction;
    // ipmt
    uint8_t             socketId;                          // ID of the mote's UDP socket
    uint8_t             replyBuf[MAX_FRAME_LENGTH];        // holds notifications from ipmt
    uint8_t             notifBuf[MAX_FRAME_LENGTH];        // notifications buffer internal to ipmt
} app_vars_t;

app_vars_t app_vars;

typedef struct {
    uint32_t            num_calls_api_getMoteStatus;
    uint32_t            num_calls_api_getMoteStatus_reply;
    uint32_t            num_calls_api_openSocket;
    uint32_t            num_calls_api_openSocket_reply;
    uint32_t            num_calls_api_bindSocket;
    uint32_t            num_calls_api_bindSocket_reply;
    uint32_t            num_calls_api_join;
    uint32_t            num_calls_api_join_reply;
    uint32_t            num_calls_api_sendTo_now;
    uint32_t            num_calls_api_sendTo_reply;
    uint32_t            num_notif_EVENTS;
    uint32_t            num_notif_RECEIVE;
    uint32_t            num_task_loops;
    uint32_t            num_ISR_RTC0_IRQHandler;
    uint32_t            num_ISR_RTC0_IRQHandler_COMPARE0;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

// app
uint16_t nextValue(void);
// fsm
void     fsm_scheduleEvent(uint16_t delay, fsm_timer_callback cb);
void     fsm_cancelEvent(void);
void     fsm_setCallback(fsm_reply_callback cb);
// ipmt
void     dn_ipmt_notif_cb(uint8_t cmdId, uint8_t subCmdId);
void     dn_ipmt_reply_cb(uint8_t cmdId);
// api
void     api_response_timeout(void);
void     api_getMoteStatus(void);
void     api_getMoteStatus_reply(void);
void     api_openSocket(void);
void     api_openSocket_reply(void);
void     api_bindSocket(void);
void     api_bindSocket_reply(void);
void     api_join(void);
void     api_join_reply(void);
void     api_sendTo(void);
void     api_sendTo_reply(void);
// bsp
void     lfxtal_start(void);
void     hfclock_start(void);

//=========================== main ============================================

int main(void) {

    // initialize variables
    memset(&app_vars,0x00,sizeof(app_vars));
    app_vars.socketId = 22; // default value
    memset(&app_dbg, 0x00,sizeof(app_dbg));
    
    // bsp
    board_init();

    // initialize the ipmt module
    dn_ipmt_init(
        dn_ipmt_notif_cb,                // notifCb
        app_vars.notifBuf,               // notifBuf
        sizeof(app_vars.notifBuf),       // notifBufLen
        dn_ipmt_reply_cb                 // replyCb
    );

    // schedule the first event
    fsm_scheduleEvent(CMD_PERIOD, &api_getMoteStatus);

    // main loop
    while(1) {

        // wait for event
        __SEV(); // set event
        __WFE(); // wait for event
        __WFE(); // wait for event

        // debug
        app_dbg.num_task_loops++;
    }
}

//=========================== private =========================================

//=== app

uint16_t nextValue(void) {
   uint16_t newValue;
   
   // decide whether to go up or down
   if (app_vars.counter==0) {
      app_vars.direction = DIRECTION_UP;
   }
   if (app_vars.counter==NUM_STEPS-1) {
      app_vars.direction = DIRECTION_DOWN;
   }
   
   // calculate new value
   if (app_vars.direction==DIRECTION_UP) {
      app_vars.counter++;
   } else {
      app_vars.counter--;
   }
   
   newValue  = 0x10000/NUM_STEPS;
   newValue *= app_vars.counter;
   
   return newValue;
}

//=== fsm

void fsm_scheduleEvent(uint16_t delay, fsm_timer_callback cb) {
   
    // remember what function to call
    app_vars.fsmCb                     = cb;

    // configure/start the RTC
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // xxxx xxxx xxxx FEDC xxxx xxxx xxxx xxBA (C=compare 0)
    // 0000 0000 0000 0001 0000 0000 0000 0000 
    //    0    0    0    1    0    0    0    0 0x00010000
    NRF_RTC0->EVTENSET                 = 0x00010000;       // enable compare 0 event routing
    NRF_RTC0->INTENSET                 = 0x00010000;       // enable compare 0 interrupts

    // enable interrupts
    NVIC_SetPriority(RTC0_IRQn, 1);
    NVIC_ClearPendingIRQ(RTC0_IRQn);
    NVIC_EnableIRQ(RTC0_IRQn);
    
    //
    NRF_RTC0->CC[0]                    = delay;            // 32768>>3 = 125 ms
    NRF_RTC0->TASKS_START              = 0x00000001;       // start RTC0
}

void fsm_cancelEvent(void) {
   // stop the RTC
   NRF_RTC0->TASKS_STOP                = 0x00000001;       // stop RTC0
   
   // clear function to call
   app_vars.fsmCb                      = NULL;
}

void fsm_setCallback(fsm_reply_callback cb) {
   app_vars.replyCb                    = cb;
}

//=== ipmt

void dn_ipmt_notif_cb(uint8_t cmdId, uint8_t subCmdId) {
    dn_ipmt_events_nt*   dn_ipmt_events_notif;
    dn_ipmt_receive_nt*  dn_ipmt_receive_notif;

    switch (cmdId) {
      case CMDID_EVENTS:
   
          // debug
          app_dbg.num_notif_EVENTS++;

          // parse
          dn_ipmt_events_notif = (dn_ipmt_events_nt*)app_vars.notifBuf;

          // handle
          switch (dn_ipmt_events_notif->state) {
              case MOTE_STATE_IDLE:
                  fsm_scheduleEvent(CMD_PERIOD,api_getMoteStatus);
                  break;
              case MOTE_STATE_OPERATIONAL:
                  fsm_scheduleEvent(CMD_PERIOD,api_sendTo);
                  break;
              default:
                  // nothing to do
                  break;
          }
          break;
      case CMDID_RECEIVE:
          
          // debug
          app_dbg.num_notif_RECEIVE++;

          // parse
          dn_ipmt_receive_notif = (dn_ipmt_receive_nt*)app_vars.notifBuf;

          // TODO: do something with received code

      default:
         // nothing to do
         break;
    }
}

void dn_ipmt_reply_cb(uint8_t cmdId) {
   app_vars.replyCb();
}

//=== api

void api_response_timeout(void) {
   
   // issue cancel command
   dn_ipmt_cancelTx();
   
   // schedule first event
   fsm_scheduleEvent(CMD_PERIOD, &api_getMoteStatus);
}

// getMoteStatus

void api_getMoteStatus(void) {
   
   // debug
   app_dbg.num_calls_api_getMoteStatus++;

   // arm callback
   fsm_setCallback(api_getMoteStatus_reply);
   
   // issue function
   dn_ipmt_getParameter_moteStatus(
      (dn_ipmt_getParameter_moteStatus_rpt*)(app_vars.replyBuf)
   );

   // schedule timeout event
   fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, api_response_timeout);
}

void api_getMoteStatus_reply(void) {
   dn_ipmt_getParameter_moteStatus_rpt* reply;
   
   // debug
   app_dbg.num_calls_api_getMoteStatus_reply++;

   // cancel timeout
   fsm_cancelEvent();
   
   // parse reply
   reply = (dn_ipmt_getParameter_moteStatus_rpt*)app_vars.replyBuf;
   
   // choose next step
   switch (reply->state) {
      case MOTE_STATE_IDLE:
         fsm_scheduleEvent(CMD_PERIOD, &api_openSocket);
         break;
      case MOTE_STATE_OPERATIONAL:
         fsm_scheduleEvent(CMD_PERIOD, api_sendTo);
         break;
      default:
         fsm_scheduleEvent(CMD_PERIOD, api_getMoteStatus);
         break;
   }
}

// openSocket

void api_openSocket(void) {
  
   // debug
   app_dbg.num_calls_api_openSocket++;

   // arm callback
   fsm_setCallback(api_openSocket_reply);
   
   // issue function
   dn_ipmt_openSocket(
      0,                                              // protocol
      (dn_ipmt_openSocket_rpt*)(app_vars.replyBuf)    // reply
   );
   
   // schedule timeout event
   fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, api_response_timeout);
}

void api_openSocket_reply(void) {
   dn_ipmt_openSocket_rpt* reply;
   
   // debug
   app_dbg.num_calls_api_openSocket_reply++;

   // cancel timeout
   fsm_cancelEvent();
   
   // parse reply
   reply = (dn_ipmt_openSocket_rpt*)app_vars.replyBuf;
   
   // store the socketID
   app_vars.socketId = reply->socketId;
   
   // choose next step
   fsm_scheduleEvent(CMD_PERIOD, api_bindSocket);
}

// bindSocket

void api_bindSocket(void) {
   
   // debug
   app_dbg.num_calls_api_bindSocket++;

   // arm callback
   fsm_setCallback(api_bindSocket_reply);
   
   // issue function
   dn_ipmt_bindSocket(
      app_vars.socketId,                              // socketId
      SRC_PORT,                                       // port
      (dn_ipmt_bindSocket_rpt*)(app_vars.replyBuf)    // reply
   );

   // schedule timeout event
   fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, api_response_timeout);
}

void api_bindSocket_reply(void) {
   
   // debug
   app_dbg.num_calls_api_bindSocket_reply++;

   // cancel timeout
   fsm_cancelEvent();
   
   // choose next step
   fsm_scheduleEvent(CMD_PERIOD, api_join);
}

// join

void api_join(void) {
   
   // debug
   app_dbg.num_calls_api_join++;

   // arm callback
   fsm_setCallback(api_join_reply);
   
   // issue function
   dn_ipmt_join(
      (dn_ipmt_join_rpt*)(app_vars.replyBuf)     // reply
   );

   // schedule timeout event
   fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, api_response_timeout);
}

void api_join_reply(void) {

   // debug
   app_dbg.num_calls_api_join_reply++;

   // cancel timeout
   fsm_cancelEvent();
   
   // choose next step
   // no next step at this point. FSM will advance when we received a "joined"
   // notification
}

// sendTo

void api_sendTo(void) {
   uint8_t  payload[2];
   uint8_t  dest_addr[16];
   
   // send only every DATA_PERIOD_S seconds
   if (app_vars.secUntilTx>0) {
      
      // decrement number of second to still wait
      app_vars.secUntilTx--;
      
      // cancel timeout
      fsm_cancelEvent();
      
      // choose next step
      fsm_scheduleEvent(ONE_SEC, api_sendTo);
      
      return;
   } else {
      app_vars.secUntilTx = DATA_PERIOD_S;
   }

   // debug
   app_dbg.num_calls_api_sendTo_now++;
   
   // arm callback
   fsm_setCallback(api_sendTo_reply);
   
   // create payload
   dn_write_uint16_t(payload, nextValue());
   memcpy(dest_addr,ipv6Addr_manager,16);
   
   // issue function
   dn_ipmt_sendTo(
      app_vars.socketId,                                   // socketId
      dest_addr,                                           // destIP
      DST_PORT,                                            // destPort
      SERVICE_TYPE_BW,                                     // serviceType
      0,                                                   // priority
      0xffff,                                              // packetId
      payload,                                             // payload
      sizeof(payload),                                     // payloadLen
      (dn_ipmt_sendTo_rpt*)(app_vars.replyBuf)             // reply
   );

   // schedule timeout event
   fsm_scheduleEvent(SERIAL_RESPONSE_TIMEOUT, api_response_timeout);
}

void api_sendTo_reply(void) {
   
   // debug
   app_dbg.num_calls_api_sendTo_reply++;

   // cancel timeout
   fsm_cancelEvent();
   
   // choose next step
   fsm_scheduleEvent(ONE_SEC, api_sendTo);
}

//=========================== interrupt handlers ==============================

void RTC0_IRQHandler(void) {

    // debug
    app_dbg.num_ISR_RTC0_IRQHandler++;

    // handle compare[0]
    if (NRF_RTC0->EVENTS_COMPARE[0] == 0x00000001 ) {

        // clear flag
        NRF_RTC0->EVENTS_COMPARE[0]    = 0x00000000;

        // clear COUNTER
        NRF_RTC0->TASKS_CLEAR          = 0x00000001;

        // debug
        app_dbg.num_ISR_RTC0_IRQHandler_COMPARE0++;

        // handle
        app_vars.fsmCb();
    }
}
