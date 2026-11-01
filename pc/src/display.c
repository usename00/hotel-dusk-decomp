/**
 * Hotel Dusk: Room 215 (USA) - PC Port Display & Input Subsystem
 */

#include "hotel_dusk_pc.h"
#include "font8x16.h"
#include <SDL.h>

static SDL_Window   *s_Window   = NULL;
static SDL_Renderer *s_Renderer = NULL;
static SDL_Texture  *s_TexLeft  = NULL;
static SDL_Texture  *s_TexRight = NULL;

HotelDuskPC_Context g_Ctx;

void PC_DrawText(uint32_t *fb, int start_x, int start_y, const char *str, uint32_t color) {
    if (!str || !fb) return;
    int cur_x = start_x;
    int cur_y = start_y;

    while (*str) {
        char c = *str++;
        if (c == '\n') {
            cur_x = start_x;
            cur_y += 18;
            continue;
        }
        if (c < 32 || c > 126) c = ' ';

        const uint8_t *glyph = g_Font8x16[c - 32];
        for (int y = 0; y < 16; y++) {
            uint8_t row = glyph[y];
            int py = cur_y + y;
            if (py < 0 || py >= NDS_SCREEN_HEIGHT) continue;

            for (int x = 0; x < 8; x++) {
                if (row & (1 << (7 - x))) {
                    int px = cur_x + x;
                    if (px >= 0 && px < NDS_SCREEN_WIDTH) {
                        fb[py * NDS_SCREEN_WIDTH + px] = color;
                    }
                }
            }
        }
        cur_x += 8;
    }
}

void PC_DrawTextWrapped(uint32_t *fb, int start_x, int start_y, int max_w, const char *str, int max_chars, uint32_t color) {
    if (!str || !fb || max_chars <= 0) return;
    int cur_x = start_x;
    int cur_y = start_y;
    int rendered = 0;

    char word[64];
    int w_idx = 0;
    const char *p = str;

    while (*p && rendered < max_chars) {
        if (*p == ' ' || *p == '\n') {
            word[w_idx] = '\0';
            int word_pixel_w = w_idx * 8;

            if (cur_x + word_pixel_w > start_x + max_w) {
                cur_x = start_x;
                cur_y += 18;
            }

            for (int i = 0; i < w_idx; i++) {
                char c = word[i];
                if (c >= 32 && c <= 126) {
                    const uint8_t *glyph = g_Font8x16[c - 32];
                    for (int gy = 0; gy < 16; gy++) {
                        uint8_t row = glyph[gy];
                        int py = cur_y + gy;
                        if (py >= 0 && py < NDS_SCREEN_HEIGHT) {
                            for (int gx = 0; gx < 8; gx++) {
                                if (row & (1 << (7 - gx))) {
                                    int px = cur_x + gx;
                                    if (px >= 0 && px < NDS_SCREEN_WIDTH) {
                                        fb[py * NDS_SCREEN_WIDTH + px] = color;
                                    }
                                }
                            }
                        }
                    }
                }
                cur_x += 8;
            }

            if (*p == '\n') {
                cur_x = start_x;
                cur_y += 18;
            } else {
                cur_x += 8; // Space
            }

            w_idx = 0;
            p++;
            rendered++;
        } else {
            if (w_idx < 63) {
                word[w_idx++] = *p;
            }
            p++;
            rendered++;
        }
    }

    // Flush remaining word
    if (w_idx > 0) {
        word[w_idx] = '\0';
        int word_pixel_w = w_idx * 8;
        if (cur_x + word_pixel_w > start_x + max_w) {
            cur_x = start_x;
            cur_y += 18;
        }
        for (int i = 0; i < w_idx; i++) {
            char c = word[i];
            if (c >= 32 && c <= 126) {
                const uint8_t *glyph = g_Font8x16[c - 32];
                for (int gy = 0; gy < 16; gy++) {
                    uint8_t row = glyph[gy];
                    int py = cur_y + gy;
                    if (py >= 0 && py < NDS_SCREEN_HEIGHT) {
                        for (int gx = 0; gx < 8; gx++) {
                            if (row & (1 << (7 - gx))) {
                                int px = cur_x + gx;
                                if (px >= 0 && px < NDS_SCREEN_WIDTH) {
                                    fb[py * NDS_SCREEN_WIDTH + px] = color;
                                }
                            }
                        }
                    }
                }
            }
            cur_x += 8;
        }
    }
}

void PC_DrawRect(uint32_t *fb, int rx, int ry, int rw, int rh, uint32_t color) {
    if (!fb) return;
    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= NDS_SCREEN_HEIGHT) continue;
        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= NDS_SCREEN_WIDTH) continue;
            fb[y * NDS_SCREEN_WIDTH + x] = color;
        }
    }
}

void PC_DrawRectOutline(uint32_t *fb, int rx, int ry, int rw, int rh, uint32_t color) {
    if (!fb) return;
    // Top & Bottom lines
    for (int x = rx; x < rx + rw; x++) {
        if (x >= 0 && x < NDS_SCREEN_WIDTH) {
            if (ry >= 0 && ry < NDS_SCREEN_HEIGHT) fb[ry * NDS_SCREEN_WIDTH + x] = color;
            if (ry + rh - 1 >= 0 && ry + rh - 1 < NDS_SCREEN_HEIGHT) fb[(ry + rh - 1) * NDS_SCREEN_WIDTH + x] = color;
        }
    }
    // Left & Right lines
    for (int y = ry; y < ry + rh; y++) {
        if (y >= 0 && y < NDS_SCREEN_HEIGHT) {
            if (rx >= 0 && rx < NDS_SCREEN_WIDTH) fb[y * NDS_SCREEN_WIDTH + rx] = color;
            if (rx + rw - 1 >= 0 && rx + rw - 1 < NDS_SCREEN_WIDTH) fb[y * NDS_SCREEN_WIDTH + (rx + rw - 1)] = color;
        }
    }
}

void PC_Fill(uint32_t *fb, uint32_t color) {
    if (!fb) return;
    for (int i = 0; i < NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT; i++) {
        fb[i] = color;
    }
}

bool PC_Init(void) {
    SDL_SetMainReady();
    memset(&g_Ctx, 0, sizeof(HotelDuskPC_Context));
    g_Ctx.mode = MODE_CONVERSATION;
    g_Ctx.current_chapter = 1;
    g_Ctx.current_scene = 0;
    g_Ctx.lines_per_page = 11;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("[ERROR] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    s_Window = SDL_CreateWindow(
        "Hotel Dusk: Room 215 - Native PC Port (Phase 2 Rotoscope & Audio)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!s_Window) {
        printf("[ERROR] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    s_Renderer = SDL_CreateRenderer(s_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!s_Renderer) {
        s_Renderer = SDL_CreateRenderer(s_Window, -1, SDL_RENDERER_SOFTWARE);
    }

    s_TexLeft = SDL_CreateTexture(s_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, NDS_SCREEN_WIDTH, NDS_SCREEN_HEIGHT);
    s_TexRight = SDL_CreateTexture(s_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, NDS_SCREEN_WIDTH, NDS_SCREEN_HEIGHT);

    if (!s_TexLeft || !s_TexRight) {
        printf("[ERROR] SDL_CreateTexture failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Retro pixel-perfect scaling
    printf("[PC PORT] Dual-Screen Display initialized (%dx%d Book Mode)\n", WINDOW_WIDTH, WINDOW_HEIGHT);
    return true;
}

void PC_Shutdown(void) {
    if (s_TexLeft)   SDL_DestroyTexture(s_TexLeft);
    if (s_TexRight)  SDL_DestroyTexture(s_TexRight);
    if (s_Renderer)  SDL_DestroyRenderer(s_Renderer);
    if (s_Window)    SDL_DestroyWindow(s_Window);
    SDL_Quit();
}

void PC_HandleEvents(bool *running) {
    SDL_Event e;
    DialogueState *dState = Dialogue_GetState();

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    *running = false;
                    break;
                case SDLK_TAB:
                    // Toggle between Conversation Mode and Notebook Memoir Mode
                    if (g_Ctx.mode == MODE_CONVERSATION) {
                        g_Ctx.mode = MODE_NOTEBOOK;
                        Audio_PlaySFX(SFX_NOTEBOOK_OPEN);
                    } else {
                        g_Ctx.mode = MODE_CONVERSATION;
                        Audio_PlaySFX(SFX_NOTEBOOK_CLOSE);
                    }
                    break;
                case SDLK_F11:
                    g_Ctx.is_fullscreen = !g_Ctx.is_fullscreen;
                    SDL_SetWindowFullscreen(s_Window, g_Ctx.is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    break;
                case SDLK_s:
                    Audio_ToggleMute();
                    break;
                case SDLK_m:
                    g_Ctx.mic_blow_active = !g_Ctx.mic_blow_active;
                    Audio_PlaySFX(SFX_PEN);
                    break;
                case SDLK_l:
                    g_Ctx.lid_closed = !g_Ctx.lid_closed;
                    if (g_Ctx.lid_closed) {
                        Audio_PlaySFX(SFX_DOOR);
                    } else {
                        Audio_PlaySFX(SFX_NOTEBOOK_OPEN);
                    }
                    break;
                case SDLK_SPACE:
                    if (g_Ctx.mode == MODE_CONVERSATION) {
                        Dialogue_Advance();
                    } else {
                        // Notebook page turn
                        Audio_PlaySFX(SFX_PAGE_TURN);
                        if (g_Ctx.current_page_start + g_Ctx.lines_per_page < g_Ctx.total_monologue_lines) {
                            g_Ctx.current_page_start += g_Ctx.lines_per_page;
                        } else {
                            g_Ctx.current_page_start = 0;
                        }
                    }
                    break;
                case SDLK_1:
                    if (g_Ctx.mode == MODE_CONVERSATION && dState->in_choice_mode) {
                        Dialogue_SelectChoice(0);
                    } else {
                        g_Ctx.current_scene = 0;
                        Audio_PlaySFX(SFX_DOOR);
                    }
                    break;
                case SDLK_2:
                    if (g_Ctx.mode == MODE_CONVERSATION && dState->in_choice_mode) {
                        Dialogue_SelectChoice(1);
                    } else {
                        g_Ctx.current_scene = 1;
                        Audio_PlaySFX(SFX_DOOR);
                    }
                    break;
                case SDLK_3:
                    if (g_Ctx.mode == MODE_CONVERSATION && dState->in_choice_mode) {
                        Dialogue_SelectChoice(2);
                    } else {
                        g_Ctx.current_scene = 2;
                        Audio_PlaySFX(SFX_DOOR);
                    }
                    break;
                default:
                    break;
            }
        } else if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            int mx = e.motion.x;
            int my = e.motion.y;
            uint32_t btn = SDL_GetMouseState(NULL, NULL);

            int right_start = (NDS_SCREEN_WIDTH * PC_SCREEN_SCALE) + SPINE_WIDTH;
            int right_end = right_start + (NDS_SCREEN_WIDTH * PC_SCREEN_SCALE);

            if (mx >= right_start && mx < right_end) {
                g_Ctx.stylus_x = (mx - right_start) / PC_SCREEN_SCALE;
                g_Ctx.stylus_y = my / PC_SCREEN_SCALE;
                g_Ctx.touch_pressed = (btn & SDL_BUTTON_LMASK) != 0;
            } else {
                g_Ctx.touch_pressed = false;
            }

            // Mouse Click Handling
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (mx >= right_start && mx < right_end) {
                    int sy = g_Ctx.stylus_y;
                    if (g_Ctx.mode == MODE_CONVERSATION && dState->in_choice_mode) {
                        // Check choice button clicks on Touchscreen
                        if (sy >= 170 && sy < 192) {
                            Dialogue_SelectChoice(0);
                        } else if (sy >= 196 && sy < 218) {
                            Dialogue_SelectChoice(1);
                        } else if (sy >= 222 && sy < 244) {
                            Dialogue_SelectChoice(2);
                        } else {
                            Dialogue_Advance();
                        }
                    } else if (g_Ctx.mode == MODE_CONVERSATION) {
                        Dialogue_Advance();
                    }
                } else if (mx < (NDS_SCREEN_WIDTH * PC_SCREEN_SCALE)) {
                    // Left Screen click: Advance dialogue or turn page
                    if (g_Ctx.mode == MODE_CONVERSATION) {
                        Dialogue_Advance();
                    } else {
                        Audio_PlaySFX(SFX_PAGE_TURN);
                        if (g_Ctx.current_page_start + g_Ctx.lines_per_page < g_Ctx.total_monologue_lines) {
                            g_Ctx.current_page_start += g_Ctx.lines_per_page;
                        } else {
                            g_Ctx.current_page_start = 0;
                        }
                    }
                }
            }
        }
    }
}

void PC_Render(void) {
    if (g_Ctx.lid_closed) {
        SDL_SetRenderDrawColor(s_Renderer, 8, 8, 10, 255);
        SDL_RenderClear(s_Renderer);
        SDL_RenderPresent(s_Renderer);
        return;
    }

    SDL_UpdateTexture(s_TexLeft, NULL, g_Ctx.left_fb, NDS_SCREEN_WIDTH * sizeof(uint32_t));
    SDL_UpdateTexture(s_TexRight, NULL, g_Ctx.right_fb, NDS_SCREEN_WIDTH * sizeof(uint32_t));

    SDL_SetRenderDrawColor(s_Renderer, 12, 12, 16, 255);
    SDL_RenderClear(s_Renderer);

    // Left Screen
    SDL_Rect r_left = {0, 0, NDS_SCREEN_WIDTH * PC_SCREEN_SCALE, NDS_SCREEN_HEIGHT * PC_SCREEN_SCALE};
    SDL_RenderCopy(s_Renderer, s_TexLeft, NULL, &r_left);

    // Authentic DS Book Hinge Spine
    int spine_x = NDS_SCREEN_WIDTH * PC_SCREEN_SCALE;
    SDL_Rect r_spine = {spine_x, 0, SPINE_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(s_Renderer, 22, 22, 28, 255);
    SDL_RenderFillRect(s_Renderer, &r_spine);

    // Spine Center Crease
    SDL_SetRenderDrawColor(s_Renderer, 8, 8, 12, 255);
    SDL_RenderDrawLine(s_Renderer, spine_x + SPINE_WIDTH / 2, 0, spine_x + SPINE_WIDTH / 2, WINDOW_HEIGHT);

    // Right Screen (Touchscreen)
    SDL_Rect r_right = {spine_x + SPINE_WIDTH, 0, NDS_SCREEN_WIDTH * PC_SCREEN_SCALE, NDS_SCREEN_HEIGHT * PC_SCREEN_SCALE};
    SDL_RenderCopy(s_Renderer, s_TexRight, NULL, &r_right);

    // Draw Stylus Touch indicator when pressed
    if (g_Ctx.touch_pressed) {
        int touch_screen_x = r_right.x + (g_Ctx.stylus_x * PC_SCREEN_SCALE);
        int touch_screen_y = r_right.y + (g_Ctx.stylus_y * PC_SCREEN_SCALE);

        SDL_SetRenderDrawColor(s_Renderer, 255, 230, 0, 220);
        SDL_Rect cursor_box = {touch_screen_x - 4, touch_screen_y - 4, 9, 9};
        SDL_RenderFillRect(s_Renderer, &cursor_box);
    }

    SDL_RenderPresent(s_Renderer);
}
