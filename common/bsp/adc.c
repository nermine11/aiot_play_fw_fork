#include <string.h>
#include "gpio.h"

//=========================== variables =======================================

typedef struct {
    uint8_t        dummy;
} adc_vars_t;

adc_vars_t adc_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void adc_init(void) {
    // TODO
}

// P0.02
uint8_t adc_read_P002(void) {
    return 0; // TODO
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
