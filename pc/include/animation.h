/**
 * Hotel Dusk: Room 215 (USA) - PC Port Character Rotoscope Animation System
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ANIM_FRAMES 8

typedef enum {
    ANIM_ED_PHONE = 0,     // Ed speaking on the phone
    ANIM_OPENING_CUTSCENE, // Rotoscope opening movie
    ANIM_KYLE_INSPECTION,  // Kyle examining hotel files
    ANIM_MILA_SILENCE,     // Mila mysterious gaze
    ANIM_COUNT
} AnimID;

typedef struct {
    char name[32];
    int frame_count;
    int current_frame;
    uint32_t frame_delay_ms;
    uint32_t timer_ms;
    bool is_looping;
    int frame_w;
    int frame_h;
    uint32_t *frames[MAX_ANIM_FRAMES];
} CharacterAnimation;

bool Anim_Init(const char *asset_root);
void Anim_Shutdown(void);
void Anim_SetCurrent(AnimID id);
AnimID Anim_GetCurrent(void);
void Anim_Update(uint32_t delta_ms);
void Anim_DrawCurrentFrame(uint32_t *dst_fb, int dst_w, int dst_h, int dst_x, int dst_y);

#endif // ANIMATION_H
