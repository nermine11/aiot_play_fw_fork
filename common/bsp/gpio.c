#include "gpio.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

static void _gpio_output_init(uint8_t pin);
static void _gpio_output_high(uint8_t pin);
static void _gpio_output_low(uint8_t pin);
static void _gpio_input_init(uint8_t pin);

//=========================== public ==========================================

void gpio_init(void) {
    // nothing to do for now
}

// P0.02
void gpio_P002_output_init(void) { _gpio_output_init(02); }
void gpio_P002_output_high(void) { _gpio_output_high(02); }
void gpio_P002_output_low(void)  { _gpio_output_low(02);  }
void gpio_P002_input_init(void)  { _gpio_input_init(02);  }
// P0.03
void gpio_P003_output_init(void) { _gpio_output_init(03); }
void gpio_P003_output_high(void) { _gpio_output_high(03); }
void gpio_P003_output_low(void)  { _gpio_output_low(03);  }
void gpio_P003_input_init(void)  { _gpio_input_init(03);  }
// P0.04
void gpio_P004_output_init(void) { _gpio_output_init(04); }
void gpio_P004_output_high(void) { _gpio_output_high(04); }
void gpio_P004_output_low(void)  { _gpio_output_low(04);  }
void gpio_P004_input_init(void)  { _gpio_input_init(04);  }
// P0.05
void gpio_P005_output_init(void) { _gpio_output_init(05); }
void gpio_P005_output_high(void) { _gpio_output_high(05); }
void gpio_P005_output_low(void)  { _gpio_output_low(05);  }
void gpio_P005_input_init(void)  { _gpio_input_init(05);  }
// P0.11
void gpio_P011_output_init(void) { _gpio_output_init(11); }
void gpio_P011_output_high(void) { _gpio_output_high(11); }
void gpio_P011_output_low(void)  { _gpio_output_low(11);  }
void gpio_P011_input_init(void)  { _gpio_input_init(11);  }
// P0.15
void gpio_P015_output_init(void) { _gpio_output_init(15); }
void gpio_P015_output_high(void) { _gpio_output_high(15); }
void gpio_P015_output_low(void)  { _gpio_output_low(15);  }
void gpio_P015_input_init(void)  { _gpio_input_init(15);  }
// P0.17
void gpio_P017_output_init(void) { _gpio_output_init(17); }
void gpio_P017_output_high(void) { _gpio_output_high(17); }
void gpio_P017_output_low(void)  { _gpio_output_low(17);  }
void gpio_P017_input_init(void)  { _gpio_input_init(17);  }
// P0.18
void gpio_P018_output_init(void) { _gpio_output_init(18); }
void gpio_P018_output_high(void) { _gpio_output_high(18); }
void gpio_P018_output_low(void)  { _gpio_output_low(18);  }
void gpio_P018_input_init(void)  { _gpio_input_init(18);  }
// P0.19
void gpio_P019_output_init(void) { _gpio_output_init(19); }
void gpio_P019_output_high(void) { _gpio_output_high(19); }
void gpio_P019_output_low(void)  { _gpio_output_low(19);  }
void gpio_P019_input_init(void)  { _gpio_input_init(19);  }
// P0.20
void gpio_P020_output_init(void) { _gpio_output_init(20); }
void gpio_P020_output_high(void) { _gpio_output_high(20); }
void gpio_P020_output_low(void)  { _gpio_output_low(20);  }
void gpio_P020_input_init(void)  { _gpio_input_init(20);  }
// P0.28
void gpio_P028_output_init(void) { _gpio_output_init(28); }
void gpio_P028_output_high(void) { _gpio_output_high(28); }
void gpio_P028_output_low(void)  { _gpio_output_low(28);  }
void gpio_P028_input_init(void)  { _gpio_input_init(28);  }
// P0.29
void gpio_P029_output_init(void) { _gpio_output_init(29); }
void gpio_P029_output_high(void) { _gpio_output_high(29); }
void gpio_P029_output_low(void)  { _gpio_output_low(29);  }
void gpio_P029_input_init(void)  { _gpio_input_init(29);  }
// P0.30
void gpio_P030_output_init(void) { _gpio_output_init(30); }
void gpio_P030_output_high(void) { _gpio_output_high(30); }
void gpio_P030_output_low(void)  { _gpio_output_low(30);  }
void gpio_P030_input_init(void)  { _gpio_input_init(30);  }

//=========================== private =========================================

void _gpio_output_init(uint8_t pin) {
    NRF_P0->PIN_CNF[pin]          = 0x00000003;
}

void _gpio_output_high(uint8_t pin) {
    NRF_P0->OUTSET                = (0x00000001 << pin);
}

void _gpio_output_low(uint8_t pin) {
    NRF_P0->OUTCLR                = (0x00000001 << pin);
}

void _gpio_input_init(uint8_t pin) {
    // TODO
}

//=========================== interrupt handlers ==============================

