/**
 * Hotel Dusk: Room 215 (USA) - PC Port Audio Subsystem
 * Native SDL2 Multi-channel Mixer (BGM & Sound Effects)
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    SFX_TEXT_CLICK = 0,
    SFX_OK,
    SFX_CANCEL,
    SFX_NOTEBOOK_OPEN,
    SFX_PAGE_TURN,
    SFX_NOTEBOOK_CLOSE,
    SFX_DIALOGUE_NEXT,
    SFX_PEN,
    SFX_BELL,
    SFX_DOOR,
    SFX_JINGLE_SUCCESS,
    SFX_JINGLE_MYSTERY,
    SFX_JINGLE_PUZZLE,
    SFX_JINGLE_EVENT,
    SFX_JINGLE_FLASHBACK,
    SFX_JINGLE_GAMEOVER,
    SFX_COUNT
} SFX_ID;

bool Audio_Init(const char *asset_dir);
void Audio_Shutdown(void);
void Audio_PlaySFX(SFX_ID id);
void Audio_StartBGM(void);
void Audio_StopBGM(void);
void Audio_ToggleMute(void);
bool Audio_IsMuted(void);

#endif // AUDIO_H
