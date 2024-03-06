#ifndef __I2C_H
#define __I2C_H

#include "nrf52833.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void i2c_init(void);
void i2c_send(uint8_t addr, uint8_t* buf, uint8_t buflen);
void i2c_read(uint8_t addr, uint8_t* buf, uint8_t buflen);

#endif
