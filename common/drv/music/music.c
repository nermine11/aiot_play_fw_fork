#include <string.h>
#include <stdbool.h>
#include "music.h"
#include "pwm.h"

//=========================== variables =======================================

typedef struct {
    bool    song_playing;
    note_t* last_note;
} music_dbg_t;

music_dbg_t music_dbg;

typedef struct {
    uint32_t noteIdx;
    note_t*  notes;
    uint32_t numnotes;
    uint16_t speed;
} music_vars_t;

music_vars_t music_vars;

//=========================== prototypes ======================================

static void _start_song(uint16_t speed);
static void _play_cur_note(void);
static void _end_song(void);

//=========================== public ==========================================

void music_init(void) {

    //=== PWM
    pwm_init();

    //=== RTC1
    
    // 1098 7654 3210 9876 5432 1098 7654 3210
    // xxxx xxxx xxxx FEDC xxxx xxxx xxxx xxBA (C=compare 0)
    // 0000 0000 0000 0001 0000 0000 0000 0000 
    //    0    0    0    1    0    0    0    0 0x00010000
    NRF_RTC1->EVTENSET                 = 0x00010000;       // enable compare 0 event routing
    NRF_RTC1->INTENSET                 = 0x00010000;       // enable compare 0 interrupts

    // enable interrupts
    NVIC_SetPriority(RTC1_IRQn, 1);
    NVIC_ClearPendingIRQ(RTC1_IRQn);
    NVIC_EnableIRQ(RTC1_IRQn);
}

void music_play(songtitle_t songtitle) {
    
    switch (songtitle) {
        case SONGTITLE_STAR_WARS:
            music_vars.notes      = (note_t*)SONGNOTES_STAR_WARS_TRACK_3;
            music_vars.numnotes   = sizeof(SONGNOTES_STAR_WARS_TRACK_3)/sizeof(note_t);
            music_vars.speed      = SONGSPEED[SONGTITLE_STAR_WARS];
            break;
        case SONGTITLE_HARRY_POTTER:
            music_vars.notes      = (note_t*)SONGNOTES_HARRY_POTTER;
            music_vars.numnotes   = sizeof(SONGNOTES_HARRY_POTTER)/sizeof(note_t);
            music_vars.speed      = SONGSPEED[SONGTITLE_HARRY_POTTER];
            break;
        default:
            music_vars.notes      = (note_t*)SONGNOTES_DEFAULT;
            music_vars.numnotes   = sizeof(SONGNOTES_DEFAULT)/sizeof(note_t);
            music_vars.speed      = SONGSPEED[SONGTITLE_DEFAULT];
            break;
    }

    // play first note
    _start_song(music_vars.speed);
    _play_cur_note();
}

//=========================== private =========================================

static void _start_song(uint16_t speed) {
    music_dbg.song_playing   = true;
    music_vars.noteIdx       = 0;
    NRF_RTC1->PRESCALER      = speed;
    NRF_RTC1->TASKS_START    = 0x00000001;
}

static void _play_cur_note(void) {
    music_dbg.last_note      = &music_vars.notes[music_vars.noteIdx];
    pwm_setperiod(music_vars.notes[music_vars.noteIdx].val);
    NRF_RTC1->CC[0]          = music_vars.notes[music_vars.noteIdx].duration;
}

static void _end_song(void) {
    NRF_RTC1->TASKS_STOP     = 0x00000001;
    pwm_setperiod(NOTE_NONE);
    music_dbg.song_playing   = false;
}

//=========================== interrupt handlers ==============================

void RTC1_IRQHandler(void) {

    // handle compare[0]
    if (NRF_RTC1->EVENTS_COMPARE[0] == 0x00000001 ) {

        // clear flag
        NRF_RTC1->EVENTS_COMPARE[0]    = 0x00000000;

        // clear COUNTER
        NRF_RTC1->TASKS_CLEAR          = 0x00000001;

        // bump
        music_vars.noteIdx++;

        if (music_vars.noteIdx<music_vars.numnotes) {
            // play
            _play_cur_note();
        } else {
            // end
            _end_song();
        }
    }
}