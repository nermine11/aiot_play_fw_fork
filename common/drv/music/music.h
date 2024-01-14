#ifndef __MUSIC_H
#define __MUSIC_H

#include "stdint.h"
#include "notes.h"

//=========================== define ==========================================

//=== speed

// RTC1 runs at 32 kHz
// we set the speed of the song by tuning the prescaler of RTC1
// the number is in 1/8th notes
// speed (in bpm) = (32000*60)/(8*(value+1))
// value          = ((32000*60)/(8*speed)-1)

#define SPEED_60bpm  3999 // SONGTITLE_DEFAULT 
#define SPEED_80bpm  3000
#define SPEED_91bpm  120//2636 // SONGTITLE_STAR_WARS
#define SPEED_100bpm 2399
#define SPEED_120bpm 1999
#define SPEED_141bpm 1701 // SONGTITLE_HARRY_POTTER
#define SPEED_200bpm 1199

//=== songs

typedef struct {
    uint32_t val;      // PWM period
    uint16_t duration; // in 1/8th
} note_t;

typedef enum {
    SONGTITLE_STAR_WARS,
    SONGTITLE_HARRY_POTTER,
    SONGTITLE_DEFAULT
} songtitle_t;

static const uint16_t SONGSPEED[] = {
    SPEED_91bpm,  // SONGTITLE_STAR_WARS
    SPEED_141bpm, // SONGTITLE_HARRY_POTTER
    SPEED_60bpm,  // SONGTITLE_DEFAULT
};

#include "Star_Wars_Medley.h"

static const note_t SONGNOTES_HARRY_POTTER[] = {
    {NOTE_DO_3,  8},
};

static const note_t SONGNOTES_DEFAULT[] = {
    {NOTE_DO_3,  8},
    {NOTE_RE_3,  8},
    {NOTE_MI_3,  8},
    {NOTE_FA_3,  8},
    {NOTE_SOL_3, 8},
    {NOTE_LA_3,  8},
    {NOTE_SI_3,  8},
    {NOTE_NONE,  16},
    {NOTE_DO_3,  4},
    {NOTE_RE_3,  1},
    {NOTE_MI_3,  16},
    {NOTE_FA_3,  6},
    {NOTE_SOL_3, 8},
    {NOTE_LA_3,  8},
    {NOTE_SI_3,  8},
    {NOTE_DO_4,  8},
};

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void music_init(void);
void music_play(songtitle_t songtitle);

#endif
