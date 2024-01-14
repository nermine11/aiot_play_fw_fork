#include <string.h>
#include <stdbool.h>
#include "board.h"
#include "periodictimer.h"
#include "ntw.h"
#include "music.h"

//=========================== defines =========================================

#define TICKS_PER_SLOT       234            // 0.00725*32768
#define ASN1_POLLING_PERIOD  (32768>>0)     // 32768>>1 = 500 ms

//=========================== typedef =========================================

typedef enum {
    STEP_1_WAITING_ASN3,
    STEP_2_WAITING_ASN4_ROLLOVER,
} step_t;

//=========================== variables =======================================

typedef struct {
    step_t         step;
    uint8_t        asn[5];
    dn_err_t       rc;
    uint32_t       deviceAddr;
} app_vars_t;

app_vars_t app_vars;

typedef struct {
    uint32_t       numGetTime;
    uint32_t       numReceive;
    uint32_t       num_STEP_1_WAITING_ASN3;
    uint32_t       num_STEP_2_WAITING_ASN4_ROLLOVER;
    uint32_t       num_rc_error;
} app_dbg_t;

app_dbg_t app_dbg;

//=========================== prototypes ======================================

void _ntw_getTime_cb(dn_ipmt_getParameter_time_rpt* reply);
void _ntw_receive_cb(uint8_t* buf, uint8_t bufLen);

//=========================== main ============================================

int main(void) {

    // initialize variables
    memset(&app_vars,0x00,sizeof(app_vars));
    memset(&app_dbg, 0x00,sizeof(app_dbg));

    app_vars.deviceAddr = NRF_FICR->DEVICEADDR[0];
    // bsp
    board_init();

    // ntw
    ntw_init(
        _ntw_getTime_cb, // ntw_getTime_cb
        _ntw_receive_cb  // ntw_receive_cb
    );

    // music
    music_init();

    // RTC0
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

    // query ASN every 500ms
    app_vars.step                      = STEP_1_WAITING_ASN3;
    NRF_RTC0->CC[0]                    = ASN1_POLLING_PERIOD;
    NRF_RTC0->TASKS_START              = 0x00000001;

    // main loop
    while(1) {

        // wait for event
        board_sleep();
    }
}

//=========================== private =========================================

void _ntw_getTime_cb(dn_ipmt_getParameter_time_rpt* reply) {
    uint32_t num_asns_to_wait;
    uint32_t num_ticks_to_wait;

    // debug
    app_dbg.numGetTime++;

    do {
        if (reply->RC!=DN_ERR_NONE) {
            app_dbg.num_rc_error++;
            break;
        }
        if (reply->upTime==0) {
            break;
        }

        // copy over to local copy for easier debug
        memcpy(app_vars.asn,reply->asn,sizeof(app_vars.asn));

        switch (app_vars.step) {
            case STEP_1_WAITING_ASN3:
                app_dbg.num_STEP_1_WAITING_ASN3++;
                if ( (app_vars.asn[3]&0x3f)==0) {
                    // step 2: I'm at the right ASN[3]
                    // wait for ASN[4] to roll over

                    num_asns_to_wait  = 0xff-app_vars.asn[4];
                    num_ticks_to_wait = num_asns_to_wait*TICKS_PER_SLOT;
                    app_vars.step     = STEP_2_WAITING_ASN4_ROLLOVER;
                    NRF_RTC0->CC[0]   = num_ticks_to_wait;
                }
                break;
            case STEP_2_WAITING_ASN4_ROLLOVER:
                app_dbg.num_STEP_2_WAITING_ASN4_ROLLOVER++;
                app_vars.step         = STEP_1_WAITING_ASN3;
                NRF_RTC0->CC[0]       = ASN1_POLLING_PERIOD;
                music_play(SONGTITLE_HARRY_POTTER);
                break;
        }

    } while(0);
}

void _ntw_receive_cb(uint8_t* buf, uint8_t bufLen) {
    
    // debug
    app_dbg.numReceive++;
}

//=========================== interrupt handlers ==============================

void RTC0_IRQHandler(void) {
    
    // handle compare[0]
    if (NRF_RTC0->EVENTS_COMPARE[0] == 0x00000001 ) {

        // clear flag
        NRF_RTC0->EVENTS_COMPARE[0]    = 0x00000000;

        // clear COUNTER
        NRF_RTC0->TASKS_CLEAR          = 0x00000001;

        // handle
        ntw_getTime();
    }
}
