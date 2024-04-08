#include <string.h>
#include <stdbool.h>
#include "us.h"
#include "gpio.h"
#include "busywait.h"

//=========================== variables =======================================

typedef struct {
    bool     measurementOngoing;
    uint16_t counterHigh;
    uint16_t counterLow;
    uint16_t distance;
} us_vars_t;

us_vars_t us_vars;

typedef struct {
    uint32_t numcalls_us_init;
    uint32_t numcalls_us_measure;
    uint32_t numcalls_echo_pin_toggle_cb;
    uint32_t numcalls_echo_pin_toggle_cb_high;
    uint32_t numcalls_echo_pin_toggle_cb_low;
} us_dbg_t;

us_dbg_t us_dbg;

//=========================== prototypes ======================================

static void _wait_10us(void);
static void _echo_pin_toggle_cb(uint8_t pin_state);

//=========================== public ==========================================

void us_init(void) {

    // clear module variables
    memset(&us_vars,0x00,sizeof(us_vars_t));
    memset(&us_dbg, 0x00,sizeof(us_dbg_t) );
    us_vars.measurementOngoing = false;

    // debug
    us_dbg.numcalls_us_init++;

    // PIN_TRIGGER
    gpio_P017_output_init();
    gpio_P017_output_low();

    // PIN_ECHO
    gpio_P015_input_init(_echo_pin_toggle_cb);
}

/**
Returns the number of 32kHz ticks between the ECHO pin going high, then low.
This pin measures the time it takes for the pulse to go back and forth between
sensor and target.
Let's assume the speed of sound in air at 20C is 343 m/s.
So it's roughtly 1cm per tick, which is super convenient.
Of course, this is the RTT, to get a distance, we just divided the value by 2.
*/
uint16_t us_measure(void) {
    
    // debug
    us_dbg.numcalls_us_measure++;

    // arm
    us_vars.measurementOngoing         = true;
    us_vars.counterHigh                = 0x0000;
    us_vars.counterLow                 = 0x0000;

    // configure/start the RTC
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // xxxx xxxx xxxx FEDC xxxx xxxx xxxx xxBA (C=compare 0)
    // 0000 0000 0000 0001 0000 0000 0000 0000 
    //    0    0    0    1    0    0    0    0 0x00010000
    NRF_RTC1->EVTENSET                 = 0x00010000;       // enable compare 0 event routing
    NRF_RTC1->INTENSET                 = 0x00010000;       // enable compare 0 interrupts

    // enable interrupts
    NVIC_SetPriority(RTC1_IRQn, 2);
    NVIC_ClearPendingIRQ(RTC1_IRQn);
    NVIC_EnableIRQ(RTC1_IRQn);

    // have RTC timeout; start RTC
    NRF_RTC1->CC[0]                    = 32768>>1;         // 32768>>1 = 500ms
    NRF_RTC1->TASKS_CLEAR              = 0x00000001;       // clear
    NRF_RTC1->TASKS_START              = 0x00000001;       // start

    // trigger
    gpio_P017_output_high();
    _wait_10us();
    gpio_P017_output_low();
    
    // block until finished
    while (us_vars.measurementOngoing==true);
    
    if (us_vars.counterHigh==0x0000 && us_vars.counterLow==0x0000) {
        us_vars.distance               = US_DISTANCE_INVALID;
    } else {
        us_vars.distance               = us_vars.counterLow;
        us_vars.distance              -= us_vars.counterHigh;
        us_vars.distance              /= 2;
    }
        
    return us_vars.distance;
}

//=========================== private =========================================

static void _wait_10us(void) {
   volatile uint32_t counter;

   for (counter=0; counter<0xfff; counter++) {
       __NOP();
   }
}

static void _echo_pin_toggle_cb(uint8_t pin_state) {
    
    // debug
    us_dbg.numcalls_echo_pin_toggle_cb++;
    if (pin_state==1) {
        us_dbg.numcalls_echo_pin_toggle_cb_high++;
        us_vars.counterHigh = NRF_RTC1->COUNTER;
    } else {
        us_dbg.numcalls_echo_pin_toggle_cb_low++;
        us_vars.counterLow  = NRF_RTC1->COUNTER;
        // measurement now done
        us_vars.measurementOngoing     = false;
        NRF_RTC1->TASKS_STOP           = 0x00000001;
    }
}

//=========================== interrupt handlers ==============================

void RTC1_IRQHandler(void) {

    // handle compare[0]
    if (NRF_RTC1->EVENTS_COMPARE[0] == 0x00000001 ) {

        // clear flag
        NRF_RTC1->EVENTS_COMPARE[0]    = 0x00000000;

        // stop RTC1
        NRF_RTC1->TASKS_STOP           = 0x00000001;

        // measurement now done
        us_vars.measurementOngoing     = false;
    }
}