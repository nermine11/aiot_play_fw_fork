#include <string.h>
#include "us.h"
#include "gpio.h"
#include "busywait.h"

//=========================== variables =======================================

typedef struct {
    uint32_t dummy;
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

    // debug
    us_dbg.numcalls_us_init++;

    // PIN_TRIGGER
    gpio_P017_output_init();
    gpio_P017_output_low();

    // PIN_ECHO
    gpio_P015_input_init(_echo_pin_toggle_cb);
}

uint16_t us_measure(void) {
    
    // debug
    us_dbg.numcalls_us_measure++;

    // trigger
    gpio_P017_output_high();
    _wait_10us();
    gpio_P017_output_low();

    // TODO
    return 1000;
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
    } else {
        us_dbg.numcalls_echo_pin_toggle_cb_low++;
    }
}

//=========================== interrupt handlers ==============================
