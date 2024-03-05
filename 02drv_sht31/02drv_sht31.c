#include "nrf52833.h"

/*
sources:
    https://github.com/elecfreaks/circuitpython_cutebot/blob/main/cutebot.py
    https://github.com/Krakenus/microbit-cutebot-micropython/blob/master/cutebot.py
    https://github.com/bbcmicrobit/micropython/blob/master/source/microbit/microbiti2c.cpp
    https://microbit-micropython.readthedocs.io/en/latest/i2c.html#
    https://makecode.microbit.org/device/pins
    https://github.com/DotBots/DotBot-firmware/blob/main/bsp/nrf/i2c.c

I2C:
    config:
        7-bit addressing
    pins:
        freq: 100000
        SCL==micro: (header)==P0.03 (nRF)
        SDA==micro: (header)==P0.02 (nRF)
*/


#define SHT31_ADDR 0X44 

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
    NRF_P0->PIN_CNF[3]           = 0x00000602; // SCL (P0.03)
    NRF_P0->PIN_CNF[2]           = 0x00000602; // SDA (P0.02)

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 5=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0101 
    //    0    0    0    0    0    0    0    5 0x00000005
    NRF_TWI0->ENABLE              = 0x00000005;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    03 (P0.03)
    // .... .... .... .... .... .... ..B. .... B: PORT:    0 (P0.03)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx00 0011 
    //    0    0    0    0    0    0    0    3 0x00000003
    NRF_TWI0->PSEL.SCL            = 0x00000003;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... ...A AAAA A: PIN:    02 (P0.02)
    // .... .... .... .... .... .... ..B. .... B: PORT:    0 (P0.02)
    // C... .... .... .... .... .... .... .... C: CONNECT: 0=Connected
    // 0xxx xxxx xxxx xxxx xxxx xxxx xx00 0010
    //    0    0    0    0    0    0    0    2 0x00000002
    NRF_TWI0->PSEL.SDA            = 0x00000002;

    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // AAAA AAAA AAAA AAAA AAAA AAAA AAAA AAAA A: FREQUENCY: 0x01980000==K100==100 kbps
    NRF_TWI0->FREQUENCY           = 0x01980000;
}

void i2c_end(void) {
    // Disable TWI
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 0=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0000 
    //    0    0    0    0    0    0    0    0 0x00000000
    NRF_TWI0->ENABLE              = 0x00000000;
}

void i2c_begin(void) {
    // Enable TWI
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 5=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0101 
    //    0    0    0    0    0    0    0    5 0x00000005
    NRF_TWI0->ENABLE              = 0x00000005;
}


void i2c_send(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t buflen) {
    // Set device address
    NRF_TWI0->ADDRESS             = addr;

    // Choose register to write to
    NRF_TWI0->TXD                 = reg;
    NRF_TWI0->EVENTS_TXDSENT      = 0;
    NRF_TWI0->TASKS_STARTTX       = 1;

    for (uint8_t i = 0; i < buflen; i++) {
        while(NRF_TWI0->EVENTS_TXDSENT==0);
        NRF_TWI0->EVENTS_TXDSENT  = 0;
        NRF_TWI0->TXD             = buf[i];
    }

    while(NRF_TWI0->EVENTS_TXDSENT==0);
    NRF_TWI0->TASKS_STOP     = 1;
}

void i2c_read(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t buflen) {
    // Set device address
    NRF_TWI0->ADDRESS = addr;
    
    // Choose register to read from
    NRF_TWI0->TXD                 = reg;
    NRF_TWI0->EVENTS_TXDSENT      = 0;
    NRF_TWI0->TASKS_STARTTX       = 1;

    while (NRF_TWI0->EVENTS_TXDSENT == 0);
    NRF_TWI0->EVENTS_TXDSENT      = 0;
    
    // Begin receive mode
    NRF_TWI0->TASKS_STARTRX = 1;
    for (uint8_t i = 0; i < buflen; i++) {
        while (NRF_TWI0->EVENTS_RXDREADY == 0);
        NRF_TWI0->EVENTS_RXDREADY = 0;
        buf[i]                    = NRF_TWI0->RXD;
    }
    
    NRF_TWI0->EVENTS_STOPPED = 0;
    NRF_TWI0->TASKS_STOP = 1;
    while (NRF_TWI0->EVENTS_STOPPED == 0);
}


int main(void) {
    
    i2c_init();

    // Enable TWI
    i2c_begin();

    // Read Temperature & Humidity
    
    // I2CBUF_MOTOR_LEFT_FWD = {0x01,0x02,MOTOR_SPEED,0}
    // i2c_send(SHT31_ADDR, 0x01, I2CBUF_MOTOR_LEFT_FWD, sizeof(I2CBUF_MOTOR_LEFT_FWD));
    // READ SENSOR TEMPERATURE AND HUMIDITY

    // Disable TWI
    i2c_end();


    while(1);
}