#include "nrf52833.h"

/*
sources:
    https://github.com/elecfreaks/circuitpython_cutebot/blob/main/cutebot.py
    https://github.com/Krakenus/microbit-cutebot-micropython/blob/master/cutebot.py
    https://github.com/bbcmicrobit/micropython/blob/master/source/microbit/microbiti2c.cpp
    https://microbit-micropython.readthedocs.io/en/latest/i2c.html#
    https://makecode.microbit.org/device/pins

I2C:
    config:
        7-bit addressing
    pins:
        freq: 100000
        SCL==micro: (header)==P0.03 (nRF)
        SDA==micro: (header)==P0.02 (nRF)
*/

#define MOTOR_SPEED 20 // [0...100]
uint8_t I2CBUF_MOTOR_LEFT_FWD[]   = {0x01,0x02,MOTOR_SPEED,0};

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

void i2c_begin(void) {
    // Disable TWI
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 0=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0000 
    //    0    0    0    0    0    0    0    0 0x00000000
    NRF_TWI0->ENABLE              = 0x00000000;
}

void i2c_end(void) {
    // Enable TWI
    //  3           2            1           0
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // .... .... .... .... .... .... .... AAAA A: ENABLE: 5=Enabled
    // xxxx xxxx xxxx xxxx xxxx xxxx xxxx 0101 
    //    0    0    0    0    0    0    0    5 0x00000005
    NRF_TWI0->ENABLE              = 0x00000005;
}



void i2c_send(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t buflen) {

    NRF_TWI0->ADDRESS             = addr;
    uint8_t i;

    i=0;
    NRF_TWI0->TXD                 = buf[i];
    NRF_TWI0->EVENTS_TXDSENT      = 0;
    NRF_TWI0->TASKS_STARTTX       = 1;
    i++;
    while(i<buflen) {
        while(NRF_TWI0->EVENTS_TXDSENT==0);
        NRF_TWI0->EVENTS_TXDSENT  = 0;
        NRF_TWI0->TXD             = buf[i];
        i++;
    }
    while(NRF_TWI0->EVENTS_TXDSENT==0);
    NRF_TWI0->TASKS_STOP     = 1;
}

void db_i2c_read_regs(uint8_t addr, uint8_t reg, void *data, size_t len) {
    DB_TWIM->ADDRESS       = addr;
    DB_TWIM->TXD.MAXCNT    = 1;
    DB_TWIM->TXD.PTR       = (uint32_t)&reg;
    DB_TWIM->RXD.PTR       = (uint32_t)data;
    DB_TWIM->RXD.MAXCNT    = (uint8_t)len;
    DB_TWIM->SHORTS        = (1 << TWIM_SHORTS_LASTTX_STARTRX_Pos) | (1 << TWIM_SHORTS_LASTRX_STOP_Pos);
    DB_TWIM->TASKS_STARTTX = 1;
    _wait_for_transfer();
}

void db_i2c_write_regs(uint8_t addr, uint8_t reg, const void *data, size_t len) {
    assert(len + 1 <= DB_TWIM_TX_BUF_SIZE);
    // concatenate register address and input data in a single TX buffer
    _i2c_tx_vars.buffer[0] = reg;
    memcpy(&_i2c_tx_vars.buffer[1], data, len);

    // send the content to write to the register
    DB_TWIM->ADDRESS       = addr;
    DB_TWIM->TXD.PTR       = (uint32_t)_i2c_tx_vars.buffer;
    DB_TWIM->TXD.MAXCNT    = (uint8_t)len + 1;
    DB_TWIM->SHORTS        = (1 << TWIM_SHORTS_LASTTX_STOP_Pos);
    DB_TWIM->TASKS_STARTTX = 1;
    _wait_for_transfer();
}

int main(void) {
    
    i2c_init();

    i2c_send(SHT31_ADDR, I2CBUF_MOTOR_LEFT_FWD, sizeof(I2CBUF_MOTOR_LEFT_FWD));


    while(1);
}