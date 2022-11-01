#ifndef __PERIODICTIMER_H
#define __PERIODICTIMER_H

#include "nrf52833.h"

//=========================== define ==========================================

//=========================== typedef =========================================

typedef void (*ntw_receive_cbt)(uint8_t* buf, uint8_t bufLen);

//=========================== variables =======================================

//=========================== prototypes ======================================

void ntw_init(ntw_receive_cbt ntw_receive_cb);
void ntw_transmit(uint8_t* buf, uint8_t bufLen);

/**
\}
\}
*/

#endif