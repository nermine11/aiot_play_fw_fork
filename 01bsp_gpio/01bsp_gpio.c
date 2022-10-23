#include "board.h"
#include "gpio.h"
#include "busywait.h"

//=========================== defines =========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== main ============================================

int main(void) {

    // bsp
    board_init();

    // all GPIOs output
    gpio_P002_output_init();
    gpio_P003_output_init();
    gpio_P004_output_init();
    gpio_P005_output_init();
    gpio_P011_output_init();
    gpio_P015_output_init();
    gpio_P017_output_init();
    gpio_P018_output_init();
    gpio_P019_output_init();
    gpio_P020_output_init();
    gpio_P028_output_init();
    gpio_P029_output_init();
    gpio_P030_output_init();

    // main loop
    while(1) {
        
        // all GPIOs high
        gpio_P002_output_high();
        gpio_P003_output_high();
        gpio_P004_output_high();
        gpio_P005_output_high();
        gpio_P011_output_high();
        gpio_P015_output_high();
        gpio_P017_output_high();
        gpio_P018_output_high();
        gpio_P019_output_high();
        gpio_P020_output_high();
        gpio_P028_output_high();
        gpio_P029_output_high();
        gpio_P030_output_high();

        // busy wait
        busywait_approx_250ms();

        // all GPIOs low
        gpio_P002_output_low();
        gpio_P003_output_low();
        gpio_P004_output_low();
        gpio_P005_output_low();
        gpio_P011_output_low();
        gpio_P015_output_low();
        gpio_P017_output_low();
        gpio_P018_output_low();
        gpio_P019_output_low();
        gpio_P020_output_low();
        gpio_P028_output_low();
        gpio_P029_output_low();
        gpio_P030_output_low();

        // busy wait
        busywait_approx_250ms();
    }
}

//=========================== private =========================================
