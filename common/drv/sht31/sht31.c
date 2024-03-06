#include <string.h>
#include "i2c.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void SHT31_init(void) {
    i2c_init();
}

void SHT31_readTempHumidity(uint16_t* temp_raw, uint16_t* humidity_raw) {
    uint8_t data[6];

    // send command to measure temperature
    i2c_send(SHT31_ADDR, &CMD_MEASURE, sizeof(CMD_MEASURE));

    // read temperature data
    i2c_read(SHT31_ADDR, data, sizeof(data));
    
    // return
    *temp_raw     = data[0] << 8 | data[1];
    *humidity_raw = data[3] << 8 | data[4];
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
