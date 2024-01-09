#ifndef __PWM_H
#define __PWM_H

#include "nrf52833.h"

//=========================== define ==========================================

// period = (16000000/freq_Hz)

// we use a prescaler of 4, so the PWM counter runs at 16MHz/4=4MHz
// period = (4000000/freq_Hz)

#define NOTE_NONE      0 // 246.94 Hz
#define NOTE_SI_2  16198 // 246.94 Hz
#define NOTE_DO_3  15289 // 261.63 Hz
#define NOTE_RE_3  13621 // 293.66 Hz
#define NOTE_MI_3  12135 // 329.63 Hz
#define NOTE_FA_3  11454 // 349.23 Hz
#define NOTE_SOL_3 10204 // 392.00 Hz
#define NOTE_LA_3   9091 // 440.00 Hz
#define NOTE_SI_3   8099 // 493.88 Hz
#define NOTE_DO_4   7645 // 523.25 Hz

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void pwm_init(void);
void pwm_setperiod(uint16_t period);

#endif
