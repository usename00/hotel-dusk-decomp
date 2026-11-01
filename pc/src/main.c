/**
 * Hotel Dusk: Room 215 (USA) - Native PC Port Core
 * Phase 2: Rotoscope Animation, Interactive Dialogue, Native Audio
 */

#define SDL_MAIN_HANDLED
#include "hotel_dusk_pc.h"
#include <SDL.h>

static uint32_t s_SceneL[3][NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT];
static uint32_t s_SceneR[3][NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT];
static bool s_ScenesLoaded[3] = {false, false, false};

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
            strncpy(g_Ctx.asset_root, candidates[i], sizeof(g_Ctx.asset_root) - 1);
            printf("[PC PORT] Verified asset root: %s\n", g_Ctx.asset_root);
            return;
        }
    }
    strcpy(g_Ctx.asset_root, "H:/Games/wdss/REA/hotel_dusk_extracted/unpacked");
}

static void LoadAllScenes(void) {
    char pathL[260], pathR[260];

    // Scene 0: Dusk Highway
    snprintf(pathL, sizeof(pathL), "%s/data/show/car/duskL.bin", g_Ctx.asset_root);
    snprintf(pathR, sizeof(pathR), "%s/data/show/car/duskR.bin", g_Ctx.asset_root);
    s_ScenesLoaded[0] = PC_LoadCingBackground(pathL, s_SceneL[0]) && PC_LoadCingBackground(pathR, s_SceneR[0]);

    // Scene 1: Hotel Front
    snprintf(pathL, sizeof(pathL), "%s/data/show/car/fhotelL.bin", g_Ctx.asset_root);
    snprintf(pathR, sizeof(pathR), "%s/data/show/car/fhotelR.bin", g_Ctx.asset_root);
    s_ScenesLoaded[1] = PC_LoadCingBackground(pathL, s_SceneL[1]) && PC_LoadCingBackground(pathR, s_SceneR[1]);

    // Scene 2: Mila Encounter
    snprintf(pathL, sizeof(pathL), "%s/data/show/car/miraL.bin", g_Ctx.asset_root);
    snprintf(pathR, sizeof(pathR), "%s/data/show/car/miraR.bin", g_Ctx.asset_root);
    s_ScenesLoaded[2] = PC_LoadCingBackground(pathL, s_SceneL[2]) && PC_LoadCingBackground(pathR, s_SceneR[2]);

    // Monologue
    char text_path[260];
    snprintf(text_path, sizeof(text_path), "%s/data/local/us/textdata_usa/EP1_BOOK.txt", g_Ctx.asset_root);
    PC_LoadMonologue(text_path);
}

void PC_Update(uint32_t delta_ms) {
    DialogueState *dState = Dialogue_GetState();
    Dialogue_Update(delta_ms);
    Anim_Update(delta_ms);

    int sc = g_Ctx.current_scene;
    if (sc < 0 || sc > 2) sc = 0;

    // =========================================================================
    // 1. LEFT SCREEN RENDERING
    // =========================================================================
    if (g_Ctx.mode == MODE_NOTEBOOK) {
        // Kyle Hyde's Memoir Notebook
        PC_Fill(g_Ctx.left_fb, 0xFF181820);
        PC_DrawRect(g_Ctx.left_fb, 6, 6, NDS_SCREEN_WIDTH - 12, NDS_SCREEN_HEIGHT - 12, 0xFF24222C);
        PC_DrawRect(g_Ctx.left_fb, 8, 8, NDS_SCREEN_WIDTH - 16, 22, 0xFF363242);
        PC_DrawText(g_Ctx.left_fb, 12, 11, "CASE NOTES - KYLE HYDE", 0xFFFFD700);

        int start = g_Ctx.current_page_start;
        int end = start + g_Ctx.lines_per_page;
        if (end > g_Ctx.total_monologue_lines) end = g_Ctx.total_monologue_lines;

        int text_y = 36;
        for (int i = start; i < end; i++) {
            PC_DrawText(g_Ctx.left_fb, 12, text_y, g_Ctx.monologue_lines[i], 0xFFECECEC);
            text_y += 18;
        }

        char footer[64];
        int cur_page = (start / g_Ctx.lines_per_page) + 1;
        int max_pages = (g_Ctx.total_monologue_lines + g_Ctx.lines_per_page - 1) / g_Ctx.lines_per_page;
        snprintf(footer, sizeof(footer), "Page %d/%d [SPACE: Next | TAB: Talk]", cur_page, max_pages);
        PC_DrawText(g_Ctx.left_fb, 12, NDS_SCREEN_HEIGHT - 22, footer, 0xFF8888AA);
    } else {
        // Conversation Mode: Left Screen shows scene background + Case Status HUD
        if (s_ScenesLoaded[sc]) {
            memcpy(g_Ctx.left_fb, s_SceneL[sc], sizeof(g_Ctx.left_fb));
        } else {
            PC_Fill(g_Ctx.left_fb, 0xFF121218);
        }

        // Top Banner
        PC_DrawRect(g_Ctx.left_fb, 6, 6, NDS_SCREEN_WIDTH - 12, 22, 0xDD12121A);
        PC_DrawRectOutline(g_Ctx.left_fb, 6, 6, NDS_SCREEN_WIDTH - 12, 22, 0xFF445566);
        PC_DrawText(g_Ctx.left_fb, 12, 9, "HOTEL DUSK: ROOM 215", 0xFFFFD700);

        // Investigation Briefing Card
        PC_DrawRect(g_Ctx.left_fb, 8, 34, NDS_SCREEN_WIDTH - 16, 120, 0xCC101016);
        PC_DrawRectOutline(g_Ctx.left_fb, 8, 34, NDS_SCREEN_WIDTH - 16, 120, 0xFF3A3A4A);
        PC_DrawText(g_Ctx.left_fb, 14, 40, "CURRENT MISSION:", 0xFF00E5FF);
        PC_DrawText(g_Ctx.left_fb, 14, 58, "- Find Room 215", 0xFFE0E0E0);
        PC_DrawText(g_Ctx.left_fb, 14, 74, "- Search for Red Box", 0xFFE0E0E0);
        PC_DrawText(g_Ctx.left_fb, 14, 90, "- Investigate Bradley", 0xFFE0E0E0);
        PC_DrawText(g_Ctx.left_fb, 14, 108, "- Owner: Dunning Smith", 0xFFAAAAAA);
        PC_DrawText(g_Ctx.left_fb, 14, 126, "- Mute Girl: Mila", 0xFFAAAAAA);

        // Hotkey bar at bottom of Left Screen
        PC_DrawRect(g_Ctx.left_fb, 8, NDS_SCREEN_HEIGHT - 64, NDS_SCREEN_WIDTH - 16, 56, 0xEE161620);
        PC_DrawRectOutline(g_Ctx.left_fb, 8, NDS_SCREEN_HEIGHT - 64, NDS_SCREEN_WIDTH - 16, 56, 0xFF333344);
        PC_DrawText(g_Ctx.left_fb, 14, NDS_SCREEN_HEIGHT - 60, "[TAB] Toggle Notebook", 0xFFFFCC00);
        PC_DrawText(g_Ctx.left_fb, 14, NDS_SCREEN_HEIGHT - 44, "[1,2,3] Change Scene", 0xFF88CCFF);
        char snd_info[64];
        snprintf(snd_info, sizeof(snd_info), "[S] Audio: %s", Audio_IsMuted() ? "MUTED" : "ON");
        PC_DrawText(g_Ctx.left_fb, 14, NDS_SCREEN_HEIGHT - 28, snd_info, Audio_IsMuted() ? 0xFFFF4444 : 0xFF44FF88);
    }

    // =========================================================================
    // 2. RIGHT SCREEN RENDERING (TOUCHSCREEN + ROTOSCOPE ANIMATION + DIALOGUE)
    // =========================================================================
    if (g_Ctx.mode == MODE_NOTEBOOK) {
        // Notebook Touch Memo Pad
        PC_Fill(g_Ctx.right_fb, 0xFF1C1A24);
        PC_DrawRect(g_Ctx.right_fb, 6, 6, NDS_SCREEN_WIDTH - 12, NDS_SCREEN_HEIGHT - 12, 0xFF282534);
        PC_DrawRectOutline(g_Ctx.right_fb, 6, 6, NDS_SCREEN_WIDTH - 12, NDS_SCREEN_HEIGHT - 12, 0xFF443E54);
        PC_DrawText(g_Ctx.right_fb, 12, 12, "TOUCH MEMO PAD", 0xFFFFAA00);

        PC_DrawText(g_Ctx.right_fb, 14, 40, "Click & Drag on this", 0xFFCCCCDD);
        PC_DrawText(g_Ctx.right_fb, 14, 58, "screen with mouse to", 0xFFCCCCDD);
        PC_DrawText(g_Ctx.right_fb, 14, 76, "use the Stylus Pen.", 0xFFCCCCDD);

        PC_DrawText(g_Ctx.right_fb, 14, 110, "Press SPACE to turn", 0xFF88CCFF);
        PC_DrawText(g_Ctx.right_fb, 14, 128, "notebook pages.", 0xFF88CCFF);

        PC_DrawText(g_Ctx.right_fb, 14, 160, "Press TAB to return", 0xFFFFCC00);
        PC_DrawText(g_Ctx.right_fb, 14, 178, "to conversation mode.", 0xFFFFCC00);

        // Draw Touch tracker
        char pos_str[48];
        snprintf(pos_str, sizeof(pos_str), "Stylus: (%03d, %03d)", g_Ctx.stylus_x, g_Ctx.stylus_y);
        PC_DrawText(g_Ctx.right_fb, 14, NDS_SCREEN_HEIGHT - 26, pos_str, g_Ctx.touch_pressed ? 0xFFFFD700 : 0xFF777788);
    } else {
        // Conversation Mode:
        // Top half: Animated rotoscope character
        PC_Fill(g_Ctx.right_fb, 0xFF101014);
        Anim_DrawCurrentFrame(g_Ctx.right_fb, NDS_SCREEN_WIDTH, 144, 0, 0);

        // Character Nameplate
        DialogueNode *cNode = &dState->nodes[dState->current_node];
        int name_len = (int)strlen(cNode->speaker);
        int badge_w = (name_len * 8) + 16;
        PC_DrawRect(g_Ctx.right_fb, 8, 126, badge_w, 20, 0xEE161620);
        PC_DrawRectOutline(g_Ctx.right_fb, 8, 126, badge_w, 20, cNode->name_color);
        PC_DrawText(g_Ctx.right_fb, 14, 128, cNode->speaker, cNode->name_color);

        // Dialogue Box (Bottom half)
        PC_DrawRect(g_Ctx.right_fb, 6, 148, NDS_SCREEN_WIDTH - 12, 102, 0xF012121A);
        PC_DrawRectOutline(g_Ctx.right_fb, 6, 148, NDS_SCREEN_WIDTH - 12, 102, 0xFF555566);

        if (!dState->in_choice_mode) {
            // Dialogue text
            PC_DrawTextWrapped(g_Ctx.right_fb, 12, 154, NDS_SCREEN_WIDTH - 24, cNode->text, dState->revealed_chars, 0xFFFFFFFF);

            // Advance prompt
            if (dState->is_finished_revealing) {
                PC_DrawText(g_Ctx.right_fb, NDS_SCREEN_WIDTH - 84, NDS_SCREEN_HEIGHT - 20, "[SPACE >]", 0xFFFFD700);
            }
        } else {
            // Interactive Choice Buttons
            PC_DrawText(g_Ctx.right_fb, 12, 152, "SELECT INQUIRY:", 0xFF00E5FF);

            for (int i = 0; i < cNode->choice_count; i++) {
                int btn_y = 170 + (i * 26);
                bool hovered = (g_Ctx.stylus_y >= btn_y && g_Ctx.stylus_y < btn_y + 22);

                uint32_t btn_bg = hovered ? 0xFF2A3A4A : 0xFF1C1C26;
                uint32_t btn_border = hovered ? 0xFF00E5FF : 0xFF444455;
                uint32_t text_col = hovered ? 0xFFFFFFFF : 0xFFCCCCDD;

                PC_DrawRect(g_Ctx.right_fb, 10, btn_y, NDS_SCREEN_WIDTH - 20, 22, btn_bg);
                PC_DrawRectOutline(g_Ctx.right_fb, 10, btn_y, NDS_SCREEN_WIDTH - 20, 22, btn_border);
                PC_DrawText(g_Ctx.right_fb, 14, btn_y + 3, cNode->choices[i].text, text_col);
            }
        }
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("======================================================================\n");
    printf(" Hotel Dusk: Room 215 (USA) - Native PC Port Engine\n");
    printf(" Phase 2: Rotoscope Animation, Interactive Dialogue, Native Audio\n");
    printf("======================================================================\n\n");

    if (!PC_Init()) {
        printf("[FATAL] Display initialization failed.\n");
        return 1;
    }

    DetectAssetRoot();

    // Initialize Audio
    char sfx_dir[260];
    const char *sfx_candidates[] = {
        "pc/assets/sfx",
        "assets/sfx",
        "../pc/assets/sfx",
        "../../hotel-dusk-decomp/pc/assets/sfx",
        "H:/Games/wdss/REA/hotel-dusk-decomp/pc/assets/sfx"
    };
    for (int i = 0; i < 5; i++) {
        char test_file[260];
        snprintf(test_file, sizeof(test_file), "%s/sfx_ok.wav", sfx_candidates[i]);
        FILE *tf = fopen(test_file, "rb");
        if (tf) {
            fclose(tf);
            strncpy(sfx_dir, sfx_candidates[i], sizeof(sfx_dir) - 1);
            break;
        }
    }
    Audio_Init(sfx_dir);

    // Initialize Rotoscope Character Animations
    Anim_Init(g_Ctx.asset_root);

    // Initialize Dialogue Engine
    Dialogue_Init();

    // Pre-cache Scene Backgrounds & Monologue
    LoadAllScenes();

    bool running = true;
    uint32_t last_time = SDL_GetTicks();

    printf("\n[PC PORT CONTROLS]\n");
    printf(" - Left Click / Mouse: Stylus Touch & Dialogue Choice Interaction\n");
    printf(" - Spacebar: Advance Dialogue / Turn Notebook Page\n");
    printf(" - Tab: Toggle between Conversation Mode and Kyle's Memoir Notebook\n");
    printf(" - 1, 2, 3: Select Dialogue Inquiries / Switch Scenes\n");
    printf(" - S: Toggle Audio Mute\n");
    printf(" - M: Simulate Microphone Blow\n");
    printf(" - L: Simulate NDS Clamshell Lid Sensor (Sleep / Wake)\n");
    printf(" - F11: Fullscreen Mode\n");
    printf(" - ESC: Exit Game\n\n");

    while (running) {
        uint32_t now = SDL_GetTicks();
        uint32_t delta_ms = now - last_time;
        if (delta_ms > 100) delta_ms = 100;
        last_time = now;

        PC_HandleEvents(&running);
        PC_Update(delta_ms);
        PC_Render();

        // 60 FPS Frame Limiter
        uint32_t frame_duration = SDL_GetTicks() - now;
        if (frame_duration < 16) {
            SDL_Delay(16 - frame_duration);
        }
    }

    Audio_Shutdown();
    Anim_Shutdown();
    PC_Shutdown();

    printf("[PC PORT] Clean shutdown completed.\n");
    return 0;
}
