#include <string.h>
#include "i2c.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void i2c_init(void) {
   //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... ...A A: DIR:   0=Input
    // .... .... .... .... .... .... .... ..B. B: INPUT: 1=Disconnect
    // .... .... .... .... .... .... .... CC.. C: PULL:  0=Disabled
    // .... .... .... .... .... .DDD .... .... D: DRIVE: 6=S0D1
    // .... .... .... ..EE .... .... .... .... E: SENSE: 0=Disabled
    // xxxx xxxx xxxx xx00 xxxx x110 xxxx 0010 
    //    0    0    0    0    0    6    0    2 0x00000602
    NRF_P0->PIN_CNF[3]                 = 0x00000602; // SCL (P0.03)
    NRF_P0->PIN_CNF[2]                 = 0x00000602; // SDA (P0.02)

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 5=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0101 
    //    0    0    0    0    0    0    0    5 0x00000005
    NRF_TWI0->ENABLE                   = 0x00000005;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    03 (P0.03)
    // .... .... .... .... .... .... ..B. .... B: PORT:    0 (P0.03)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx00 0011 
    //    0    0    0    0    0    0    0    3 0x00000003
    NRF_TWI0->PSEL.SCL                 = 0x00000003;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    02 (P0.02)
    // .... .... .... .... .... .... ..B. .... B: PORT:    0 (P0.02)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx00 0010
    //    0    0    0    0    0    0    0    2 0x00000002
    NRF_TWI0->PSEL.SDA                 = 0x00000002;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // AAAA AAAA AAAA AAAA AAAA AAAA AAAA AAAA A: FREQUENCY: 0x01980000==K100==100 kbps
    NRF_TWI0->FREQUENCY                = 0x01980000;
}

void i2c_send(uint8_t addr, uint8_t* buf, uint8_t buflen) {
    uint8_t i = 0;

    NRF_TWI0->ADDRESS                  = addr;
    NRF_TWI0->TXD                      = buf[i];
    NRF_TWI0->EVENTS_TXDSENT           = 0;
    NRF_TWI0->TASKS_STARTTX            = 1;
    i++;

    while(i<buflen) {
        while(NRF_TWI0->EVENTS_TXDSENT == 0);
        NRF_TWI0->EVENTS_TXDSENT       = 0;
        NRF_TWI0->TXD                  = buf[i];
        i++;
    }

    while(NRF_TWI0->EVENTS_TXDSENT == 0);
    NRF_TWI0->TASKS_STOP               = 1;
}


void i2c_read(uint8_t addr, uint8_t* buf, uint8_t buflen) {
    uint8_t i;
    
    NRF_TWI0->ADDRESS                  = addr;

    NRF_TWI0->TASKS_STARTRX            = 1;
    for (i = 0; i < buflen-1; i++) {
        while (NRF_TWI0->EVENTS_RXDREADY == 0);
        NRF_TWI0->EVENTS_RXDREADY      = 0;
        buf[i]                         = NRF_TWI0->RXD;
    }

    // trigger STOP task before extracting last byte
    NRF_TWI0->TASKS_STOP               = 1;
    while (NRF_TWI0->EVENTS_RXDREADY == 0);
    NRF_TWI0->EVENTS_RXDREADY          = 0;
    buf[i]                             = NRF_TWI0->RXD;

    NRF_TWI0->EVENTS_STOPPED           = 0;
    while (NRF_TWI0->EVENTS_STOPPED == 0);
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
