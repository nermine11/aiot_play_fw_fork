#include <stdio.h>
#include "nrf52833.h"
#include "busywait.h"

/*
sources:
    https://github.com/elecfreaks/circuitpython_cutebot/blob/main/cutebot.py
    https://github.com/Krakenus/microbit-cutebot-micropython/blob/master/cutebot.py
    https://github.com/bbcmicrobit/micropython/blob/master/source/microbit/microbiti2c.cpp
    https://microbit-micropython.readthedocs.io/en/latest/i2c.html#
    https://makecode.microbit.org/device/pins
    https://github.com/DotBots/DotBot-firmware/blob/main/bsp/nrf/i2c.c
    https://labprojectsbd.com/2023/03/21/how-to-interface-sht31-with-stm32/#Example_code
    https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf

I2C:
    config:
        7-bit addressing
    pins:
        freq: 100000
        SCL==micro: (header)==P0.03 (nRF)
        SDA==micro: (header)==P0.02 (nRF)
*/

// Default SHT31 address
#define SHT31_ADDR 0X44 

// Measurement Commands for Single Shot Data Acquisition Mode
uint8_t CMD_MEASURE_TEMP[]      = {0x2C, 0x06};
uint8_t CMD_MEASURE_HUMIDITY[]  = {0x2C, 0x10};

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


void i2c_send(uint8_t addr, uint8_t* buf, uint8_t buflen) {
    NRF_TWI0->ADDRESS             = addr;

    uint8_t i = 0;
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


void i2c_read(uint8_t addr, uint8_t* buf, uint8_t buflen) {
    NRF_TWI0->ADDRESS = addr;

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

void SHT31_readTempHumidity(float* temp, float* humidity) {
    uint8_t data[2];
    uint16_t temp_raw, humidity_raw;

    // Send command to measure temperature
    i2c_send(SHT31_ADDR, &CMD_MEASURE_TEMP, sizeof(CMD_MEASURE_TEMP));
    busywait_approx_125ms();

    // Read temperature data
    i2c_read(SHT31_ADDR, data, sizeof(data));
    temp_raw = data[0] << 8 | data[1];
    *temp = ((float)temp_raw * 175.0f / 65535.0f) - 45.0f;

    // Send command to measure humidity
    i2c_send(SHT31_ADDR, &CMD_MEASURE_HUMIDITY, sizeof(CMD_MEASURE_HUMIDITY));
    busywait_approx_125ms();

    // Read humidity data
    i2c_read(SHT31_ADDR, data, sizeof(data));
    humidity_raw = data[0] << 8 | data[1];
    *humidity = ((float)humidity_raw * 100.0f / 65535.0f);
}

int main(void) {
    float temperature, humidity;
    i2c_init();

    while (1) {
        i2c_begin();

        SHT31_readTempHumidity(&temperature, &humidity);

        printf("Temperature: %.2f°C\n", temperature);
        printf("Humidity: %.2f%%\n", humidity);

        i2c_end();

        busywait_approx_1s();
    }
}