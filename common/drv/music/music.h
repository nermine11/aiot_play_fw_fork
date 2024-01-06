#ifndef __MUSIC_H
#define __MUSIC_H

//=========================== define ==========================================

typedef enum {
    SONG_STAR_WARS,
    SONG_HARRY_POTTER
} song_t;

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void music_init(void);
void music_play(song_t song);

#endif
