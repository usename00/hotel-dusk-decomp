/**
 * Hotel Dusk: Room 215 (USA) - Native PC Port Core
 * Phase 1: Native Windows Decomp Engine (MSVC x64 + SDL2)
 */

#include "hotel_dusk_pc.h"
#include <SDL.h>

// Pre-cached scene backgrounds
static uint32_t s_SceneL[3][NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT];
static uint32_t s_SceneR[3][NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT];
static bool s_ScenesLoaded[3] = {false, false, false};

static char s_AssetRoot[260] = "";

static void DetectAssetRoot(void) {
    const char *candidates[] = {
        "../../hotel_dusk_extracted/unpacked",
        "../hotel_dusk_extracted/unpacked",
        "hotel_dusk_extracted/unpacked",
        "build/extracted/unpacked",
        "extracted/unpacked",
        "H:/Games/wdss/REA/hotel_dusk_extracted/unpacked"
    };

    for (int i = 0; i < 6; i++) {
        char test_path[260];
        snprintf(test_path, sizeof(test_path), "%s/data/show/car/duskL.bin", candidates[i]);
        FILE *f = fopen(test_path, "rb");
        if (f) {
            fclose(f);
            strncpy(s_AssetRoot, candidates[i], sizeof(s_AssetRoot) - 1);
            printf("[PC PORT] Verified asset root: %s\n", s_AssetRoot);
            return;
        }
    }
    printf("[WARN] Could not auto-detect asset root. Using fallback.\n");
    strcpy(s_AssetRoot, "H:/Games/wdss/REA/hotel_dusk_extracted/unpacked");
}

static void LoadAllScenes(void) {
    char pathL[260], pathR[260];

    // Scene 0: Dusk Highway
    snprintf(pathL, sizeof(pathL), "%s/data/show/car/duskL.bin", s_AssetRoot);
    snprintf(pathR, sizeof(pathR), "%s/data/show/car/duskR.bin", s_AssetRoot);
    s_ScenesLoaded[0] = PC_LoadCingBackground(pathL, s_SceneL[0]) && PC_LoadCingBackground(pathR, s_SceneR[0]);

    // Scene 1: Hotel Dusk Exterior
    snprintf(pathL, sizeof(pathL), "%s/data/show/car/fhotelL.bin", s_AssetRoot);
    snprintf(pathR, sizeof(pathR), "%s/data/show/car/fhotelR.bin", s_AssetRoot);
    s_ScenesLoaded[1] = PC_LoadCingBackground(pathL, s_SceneL[1]) && PC_LoadCingBackground(pathR, s_SceneR[1]);

    // Scene 2: Mila Encounter
    snprintf(pathL, sizeof(pathL), "%s/data/show/car/miraL.bin", s_AssetRoot);
    snprintf(pathR, sizeof(pathR), "%s/data/show/car/miraR.bin", s_AssetRoot);
    s_ScenesLoaded[2] = PC_LoadCingBackground(pathL, s_SceneL[2]) && PC_LoadCingBackground(pathR, s_SceneR[2]);

    // Monologue
    char text_path[260];
    snprintf(text_path, sizeof(text_path), "%s/data/local/us/textdata_usa/EP1_BOOK.txt", s_AssetRoot);
    PC_LoadMonologue(text_path);
}

void PC_Update(void) {
    int sc = g_Ctx.current_scene;
    if (sc < 0 || sc > 2) sc = 0;

    // 1. Copy base scene background to Left & Right virtual framebuffers
    if (s_ScenesLoaded[sc]) {
        memcpy(g_Ctx.left_fb, s_SceneL[sc], sizeof(g_Ctx.left_fb));
        memcpy(g_Ctx.right_fb, s_SceneR[sc], sizeof(g_Ctx.right_fb));
    } else {
        PC_Fill(g_Ctx.left_fb, 0xFF14141E);
        PC_Fill(g_Ctx.right_fb, 0xFF14141E);
    }

    // 2. Render Left Screen UI: Kyle Hyde's Memoir Notebook
    // Dark semi-transparent narrative overlay box
    PC_DrawRect(g_Ctx.left_fb, 8, 8, NDS_SCREEN_WIDTH - 16, NDS_SCREEN_HEIGHT - 16, 0xCC111116);
    PC_DrawRect(g_Ctx.left_fb, 10, 10, NDS_SCREEN_WIDTH - 20, 20, 0xFF2A2830);

    PC_DrawText(g_Ctx.left_fb, 14, 12, "KYLE HYDE - ROOM 215", 0xFFFFD700);

    // Monologue text lines
    int start = g_Ctx.current_page_start;
    int end = start + g_Ctx.lines_per_page;
    if (end > g_Ctx.total_monologue_lines) end = g_Ctx.total_monologue_lines;

    int text_y = 36;
    for (int i = start; i < end; i++) {
        PC_DrawText(g_Ctx.left_fb, 14, text_y, g_Ctx.monologue_lines[i], 0xFFE0E0E0);
        text_y += 18;
    }

    // Footer page navigator
    char footer[64];
    int cur_page = (start / g_Ctx.lines_per_page) + 1;
    int max_pages = (g_Ctx.total_monologue_lines + g_Ctx.lines_per_page - 1) / g_Ctx.lines_per_page;
    snprintf(footer, sizeof(footer), "Page %d/%d [SPACE: Next]", cur_page, max_pages);
    PC_DrawText(g_Ctx.left_fb, 14, NDS_SCREEN_HEIGHT - 24, footer, 0xFF8888AA);


    // 3. Render Right Screen UI: Interactive Touchscreen HUD & Controls
    // Top banner
    PC_DrawRect(g_Ctx.right_fb, 6, 6, NDS_SCREEN_WIDTH - 12, 20, 0xDD181822);
    PC_DrawText(g_Ctx.right_fb, 12, 8, "TOUCHSCREEN [BOOK MODE]", 0xFF44FF88);

    // Bottom Status HUD
    PC_DrawRect(g_Ctx.right_fb, 6, NDS_SCREEN_HEIGHT - 54, NDS_SCREEN_WIDTH - 12, 48, 0xDD111118);

    // Touch readout
    char touch_info[64];
    if (g_Ctx.touch_pressed) {
        snprintf(touch_info, sizeof(touch_info), "TOUCH: (%03d, %03d) [DOWN]", g_Ctx.stylus_x, g_Ctx.stylus_y);
        PC_DrawText(g_Ctx.right_fb, 10, NDS_SCREEN_HEIGHT - 50, touch_info, 0xFFFFCC00);
    } else {
        snprintf(touch_info, sizeof(touch_info), "TOUCH: (%03d, %03d) [IDLE]", g_Ctx.stylus_x, g_Ctx.stylus_y);
        PC_DrawText(g_Ctx.right_fb, 10, NDS_SCREEN_HEIGHT - 50, touch_info, 0xFF777788);
    }

    // Hardware Sensors Status
    char hw_info[64];
    snprintf(hw_info, sizeof(hw_info), "MIC(M): %s | LID(L): %s",
        g_Ctx.mic_blow_active ? "BLOWING" : "OFF",
        g_Ctx.lid_closed ? "CLOSED" : "OPEN"
    );
    PC_DrawText(g_Ctx.right_fb, 10, NDS_SCREEN_HEIGHT - 34, hw_info, 0xFFAAAAFF);

    // Scene Switcher Hint
    char scene_info[64];
    snprintf(scene_info, sizeof(scene_info), "Scene [1,2,3]: %s",
        sc == 0 ? "Dusk Hwy" : (sc == 1 ? "Hotel Front" : "Mila")
    );
    PC_DrawText(g_Ctx.right_fb, 10, NDS_SCREEN_HEIGHT - 18, scene_info, 0xFF00E5FF);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("======================================================================\n");
    printf(" Hotel Dusk: Room 215 (USA) - Native PC Port Engine\n");
    printf(" 1:1 Matching Decompilation Runtime (MSVC x64 + SDL2)\n");
    printf("======================================================================\n\n");

    if (!PC_Init()) {
        printf("[FATAL] Initialization failed.\n");
        return 1;
    }

    DetectAssetRoot();
    LoadAllScenes();

    bool running = true;
    uint32_t last_time = SDL_GetTicks();

    printf("\n[CONTROLS]\n");
    printf(" - Mouse Left Click: Touchscreen Stylus Interaction\n");
    printf(" - Spacebar: Advance Monologue / Turn Page\n");
    printf(" - 1, 2, 3: Switch Background Scenes\n");
    printf(" - M: Toggle Microphone Blow\n");
    printf(" - L: Toggle NDS Clamshell Lid Sensor\n");
    printf(" - F11: Toggle Fullscreen\n");
    printf(" - ESC: Exit Game\n\n");

    while (running) {
        PC_HandleEvents(&running);
        PC_Update();
        PC_Render();

        // 60 FPS cap
        uint32_t now = SDL_GetTicks();
        uint32_t frame_time = now - last_time;
        if (frame_time < 16) {
            SDL_Delay(16 - frame_time);
        }
        last_time = SDL_GetTicks();
    }

    PC_Shutdown();
    printf("[PC PORT] Clean shutdown completed.\n");
    return 0;
}
