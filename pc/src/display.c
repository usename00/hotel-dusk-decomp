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

void PC_Fill(uint32_t *fb, uint32_t color) {
    if (!fb) return;
    for (int i = 0; i < NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT; i++) {
        fb[i] = color;
    }
}

bool PC_Init(void) {
    memset(&g_Ctx, 0, sizeof(HotelDuskPC_Context));
    g_Ctx.current_chapter = 1;
    g_Ctx.current_scene = 0;
    g_Ctx.lines_per_page = 11;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("[ERROR] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    s_Window = SDL_CreateWindow(
        "Hotel Dusk: Room 215 - Native PC Port (1:1 Matching Decomp Engine)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!s_Window) {
        printf("[ERROR] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    s_Renderer = SDL_CreateRenderer(
        s_Window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!s_Renderer) {
        s_Renderer = SDL_CreateRenderer(s_Window, -1, SDL_RENDERER_SOFTWARE);
    }

    s_TexLeft = SDL_CreateTexture(
        s_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        NDS_SCREEN_WIDTH, NDS_SCREEN_HEIGHT
    );

    s_TexRight = SDL_CreateTexture(
        s_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        NDS_SCREEN_WIDTH, NDS_SCREEN_HEIGHT
    );

    if (!s_TexLeft || !s_TexRight) {
        printf("[ERROR] SDL_CreateTexture failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Crisp pixelated retro scaling

    printf("[PC PORT] Display initialized: %dx%d (Dual-Screen Book Mode)\n", WINDOW_WIDTH, WINDOW_HEIGHT);
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
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *running = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    *running = false;
                    break;
                case SDLK_F11:
                    g_Ctx.is_fullscreen = !g_Ctx.is_fullscreen;
                    SDL_SetWindowFullscreen(s_Window, g_Ctx.is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                    break;
                case SDLK_SPACE:
                    // Advance monologue page
                    if (g_Ctx.current_page_start + g_Ctx.lines_per_page < g_Ctx.total_monologue_lines) {
                        g_Ctx.current_page_start += g_Ctx.lines_per_page;
                    } else {
                        g_Ctx.current_page_start = 0; // Loop back
                    }
                    g_Ctx.revealed_chars = 0;
                    break;
                case SDLK_1:
                    g_Ctx.current_scene = 0; // Dusk Highway
                    break;
                case SDLK_2:
                    g_Ctx.current_scene = 1; // Hotel Front
                    break;
                case SDLK_3:
                    g_Ctx.current_scene = 2; // Mila
                    break;
                case SDLK_m:
                    g_Ctx.mic_blow_active = !g_Ctx.mic_blow_active;
                    printf("[HARDWARE] Mic Blow %s\n", g_Ctx.mic_blow_active ? "ACTIVATED" : "DEACTIVATED");
                    break;
                case SDLK_l:
                    g_Ctx.lid_closed = !g_Ctx.lid_closed;
                    printf("[HARDWARE] NDS Lid %s\n", g_Ctx.lid_closed ? "CLOSED" : "OPENED");
                    break;
                default:
                    break;
            }
        } else if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            int mx = e.motion.x;
            int my = e.motion.y;
            uint32_t btn = SDL_GetMouseState(NULL, NULL);

            // Right Screen is Touch Screen (offset = 192 * 3 + SPINE_WIDTH = 576 + 32 = 608)
            int right_start = (NDS_SCREEN_WIDTH * PC_SCREEN_SCALE) + SPINE_WIDTH;
            int right_end = right_start + (NDS_SCREEN_WIDTH * PC_SCREEN_SCALE);

            if (mx >= right_start && mx < right_end) {
                g_Ctx.stylus_x = (mx - right_start) / PC_SCREEN_SCALE;
                g_Ctx.stylus_y = my / PC_SCREEN_SCALE;
                g_Ctx.touch_pressed = (btn & SDL_BUTTON_LMASK) != 0;
            } else {
                g_Ctx.touch_pressed = false;
            }

            // Click on Left Screen advances dialogue
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (mx < right_start) {
                    if (g_Ctx.current_page_start + g_Ctx.lines_per_page < g_Ctx.total_monologue_lines) {
                        g_Ctx.current_page_start += g_Ctx.lines_per_page;
                    } else {
                        g_Ctx.current_page_start = 0;
                    }
                    g_Ctx.revealed_chars = 0;
                }
            }
        }
    }
}

void PC_Render(void) {
    if (g_Ctx.lid_closed) {
        // Lid closed: Both screens go pitch black (mimicking closed DS clamshell)
        SDL_SetRenderDrawColor(s_Renderer, 10, 10, 12, 255);
        SDL_RenderClear(s_Renderer);

        // Draw hint text
        SDL_RenderPresent(s_Renderer);
        return;
    }

    // Update textures from virtual framebuffers
    SDL_UpdateTexture(s_TexLeft, NULL, g_Ctx.left_fb, NDS_SCREEN_WIDTH * sizeof(uint32_t));
    SDL_UpdateTexture(s_TexRight, NULL, g_Ctx.right_fb, NDS_SCREEN_WIDTH * sizeof(uint32_t));

    // Clear background
    SDL_SetRenderDrawColor(s_Renderer, 15, 15, 20, 255);
    SDL_RenderClear(s_Renderer);

    // Left Screen Rect
    SDL_Rect r_left = {
        0, 0,
        NDS_SCREEN_WIDTH * PC_SCREEN_SCALE,
        NDS_SCREEN_HEIGHT * PC_SCREEN_SCALE
    };
    SDL_RenderCopy(s_Renderer, s_TexLeft, NULL, &r_left);

    // Book Spine (Authentic Nintendo DS Hinge)
    int spine_x = NDS_SCREEN_WIDTH * PC_SCREEN_SCALE;
    SDL_Rect r_spine = {
        spine_x, 0,
        SPINE_WIDTH,
        NDS_SCREEN_HEIGHT * PC_SCREEN_SCALE
    };
    SDL_SetRenderDrawColor(s_Renderer, 28, 28, 35, 255);
    SDL_RenderFillRect(s_Renderer, &r_spine);

    // Spine Center Seam
    SDL_SetRenderDrawColor(s_Renderer, 10, 10, 14, 255);
    SDL_RenderDrawLine(s_Renderer, spine_x + SPINE_WIDTH / 2, 0, spine_x + SPINE_WIDTH / 2, WINDOW_HEIGHT);

    // Right Screen Rect (Touchscreen)
    SDL_Rect r_right = {
        spine_x + SPINE_WIDTH, 0,
        NDS_SCREEN_WIDTH * PC_SCREEN_SCALE,
        NDS_SCREEN_HEIGHT * PC_SCREEN_SCALE
    };
    SDL_RenderCopy(s_Renderer, s_TexRight, NULL, &r_right);

    // Draw Stylus Touch Indicator if pressed
    if (g_Ctx.touch_pressed) {
        int touch_screen_x = r_right.x + (g_Ctx.stylus_x * PC_SCREEN_SCALE);
        int touch_screen_y = r_right.y + (g_Ctx.stylus_y * PC_SCREEN_SCALE);

        SDL_SetRenderDrawColor(s_Renderer, 255, 220, 0, 200);
        SDL_Rect cursor_box = {touch_screen_x - 4, touch_screen_y - 4, 9, 9};
        SDL_RenderFillRect(s_Renderer, &cursor_box);
    }

    SDL_RenderPresent(s_Renderer);
}
