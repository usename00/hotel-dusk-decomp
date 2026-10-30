/**
 * Hotel Dusk: Room 215 (USA) - PC Port Native Asset Loader
 */

#include "hotel_dusk_pc.h"

bool PC_LoadCingBackground(const char *path, uint32_t *target_fb) {
    if (!path || !target_fb) return false;

    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("[WARN] Could not open background file: %s\n", path);
        return false;
    }

    uint8_t header[32];
    uint8_t pal_raw[512];
    uint8_t *pixel_indices = (uint8_t *)malloc(NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT);

    if (!pixel_indices) {
        fclose(f);
        return false;
    }

    fread(header, 1, 32, f);
    fread(pal_raw, 1, 512, f);
    size_t read_bytes = fread(pixel_indices, 1, NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT, f);
    fclose(f);

    if (read_bytes != NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT) {
        free(pixel_indices);
        return false;
    }

    // Convert 256 RGB555 palette colors to 32-bit ARGB8888
    uint32_t palette[256];
    for (int i = 0; i < 256; i++) {
        uint16_t val = (uint16_t)(pal_raw[i * 2] | (pal_raw[i * 2 + 1] << 8));
        uint32_t r = (val & 0x1F) << 3;
        uint32_t g = ((val >> 5) & 0x1F) << 3;
        uint32_t b = ((val >> 10) & 0x1F) << 3;
        palette[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }

    // Map indexed pixels to 32-bit framebuffer
    for (int i = 0; i < NDS_SCREEN_WIDTH * NDS_SCREEN_HEIGHT; i++) {
        uint8_t idx = pixel_indices[i];
        target_fb[i] = palette[idx];
    }

    free(pixel_indices);
    return true;
}

bool PC_LoadMonologue(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("[WARN] Monologue file not found: %s\n", path);
        return false;
    }

    char line[128];
    g_Ctx.total_monologue_lines = 0;

    while (fgets(line, sizeof(line), f) && g_Ctx.total_monologue_lines < 128) {
        // Strip trailing newline / carriage return
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) {
            line[--len] = '\0';
        }

        if (len > 0) {
            strncpy(g_Ctx.monologue_lines[g_Ctx.total_monologue_lines], line, 63);
            g_Ctx.monologue_lines[g_Ctx.total_monologue_lines][63] = '\0';
            g_Ctx.total_monologue_lines++;
        }
    }

    fclose(f);
    printf("[PC PORT] Loaded %d lines of monologue from %s\n", g_Ctx.total_monologue_lines, path);
    return true;
}
