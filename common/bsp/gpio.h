#ifndef __GPIO_H
#define __GPIO_H

#include "nrf52833.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void gpio_init(void);
// P0.02
void gpio_P002_output_init(void);
void gpio_P002_output_high(void);
void gpio_P002_output_low(void);
void gpio_P002_input(void);
// P0.03
void gpio_P003_output_init(void);
void gpio_P003_output_high(void);
void gpio_P003_output_low(void);
void gpio_P003_input(void);
// P0.04
void gpio_P004_output_init(void);
void gpio_P004_output_high(void);
void gpio_P004_output_low(void);
void gpio_P004_input(void);
// P0.05
void gpio_P005_output_init(void);
void gpio_P005_output_high(void);
void gpio_P005_output_low(void);
void gpio_P005_input(void);
// P0.11
void gpio_P011_output_init(void);
void gpio_P011_output_high(void);
void gpio_P011_output_low(void);
void gpio_P011_input(void);
// P0.15
void gpio_P015_output_init(void);
void gpio_P015_output_high(void);
void gpio_P015_output_low(void);
void gpio_P015_input(void);
// P0.17
void gpio_P017_output_init(void);
void gpio_P017_output_high(void);
void gpio_P017_output_low(void);
void gpio_P017_input(void);
// P0.18
void gpio_P018_output_init(void);
void gpio_P018_output_high(void);
void gpio_P018_output_low(void);
void gpio_P018_input(void);
// P0.19
void gpio_P019_output_init(void);
void gpio_P019_output_high(void);
void gpio_P019_output_low(void);
void gpio_P019_input(void);
// P0.20
void gpio_P020_output_init(void);
void gpio_P020_output_high(void);
void gpio_P020_output_low(void);
void gpio_P020_input(void);
// P0.28
void gpio_P028_output_init(void);
void gpio_P028_output_high(void);
void gpio_P028_output_low(void);
void gpio_P028_input(void);
// P0.29
void gpio_P029_output_init(void);
void gpio_P029_output_high(void);
void gpio_P029_output_low(void);
void gpio_P029_input(void);
// P0.30
void gpio_P030_output_init(void);
void gpio_P030_output_high(void);
void gpio_P030_output_low(void);
void gpio_P030_input(void);

#endif