#include <string.h>
#include "board.h"
#include "adc.h"
#include "busywait.h"

//=========================== defines =========================================

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    int16_t  adc_val;
    uint8_t  adc_num_reads;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

//=========================== main ============================================

int main(void) {
   
    // reset variables
    memset(&app_vars,0x00,sizeof(app_vars_t));

    // bsp
    board_init();

    // ADC init
    adc_init();

    // main loop
    while(1) {
        
        // read
        app_vars.adc_val = adc_read_P002();

        // increment
        app_vars.adc_num_reads++;

        // wait
        busywait_approx_500ms();
    }
}

//=========================== private =========================================
