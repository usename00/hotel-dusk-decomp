/**
 * Hotel Dusk: Room 215 (USA) - Native PC Port Engine
 * Phase 1: Core Architecture, Dual-Screen Book Mode, Asset Pipeline
 */

#ifndef HOTEL_DUSK_PC_H
#define HOTEL_DUSK_PC_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NDS_SCREEN_WIDTH   192
#define NDS_SCREEN_HEIGHT  256
#define PC_SCREEN_SCALE    3
#define SPINE_WIDTH        32

#define WINDOW_WIDTH       ((NDS_SCREEN_WIDTH * PC_SCREEN_SCALE * 2) + SPINE_WIDTH) // 1184
#define WINDOW_HEIGHT      (NDS_SCREEN_HEIGHT * PC_SCREEN_SCALE)                    // 768

// Hardware Simulation States
typedef struct {
    // Virtual DS Framebuffers (32-bit ARGB)
    uint32_t left_fb[NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT];
    uint32_t right_fb[NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT];

    // Touchscreen / Stylus State
    bool touch_pressed;
    int stylus_x; // 0..191
    int stylus_y; // 0..255

    // Hardware Sensors
    bool mic_blow_active;
    bool lid_closed;
    bool is_fullscreen;

    // Narrative & Scene Management
    int current_chapter;
    int current_scene; // 0 = Dusk Highway, 1 = Hotel Exterior, 2 = Mila Encounter
    
    // Typewriter Monologue
    char monologue_lines[128][64];
    int total_monologue_lines;
    int current_page_start;
    int lines_per_page;
    int revealed_chars;
    uint32_t last_char_tick;
} HotelDuskPC_Context;

// Global Context
extern HotelDuskPC_Context g_Ctx;

// Core Functions
bool PC_Init(void);
void PC_Shutdown(void);
void PC_HandleEvents(bool *running);
void PC_Update(void);
void PC_Render(void);

// Asset Decoding
bool PC_LoadCingBackground(const char *path, uint32_t *target_fb);
bool PC_LoadMonologue(const char *path);

// Drawing Utilities
void PC_DrawText(uint32_t *fb, int x, int y, const char *str, uint32_t color);
void PC_DrawRect(uint32_t *fb, int x, int y, int w, int h, uint32_t color);
void PC_Fill(uint32_t *fb, uint32_t color);

#endif // HOTEL_DUSK_PC_H
