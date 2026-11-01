/**
 * Hotel Dusk: Room 215 (USA) - PC Port Character Rotoscope Animation System
 */

#include "animation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CharacterAnimation s_Anims[ANIM_COUNT];
static AnimID s_CurrentAnim = ANIM_ED_PHONE;

static uint32_t* LoadCingFrame(const char *filepath, int *out_w, int *out_h) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    int hdr_size = 32;
    int w = 192;
    int h = 256;

    if (file_size == 66080) { // 256x256 image
        hdr_size = 32;
        w = 256;
        h = 256;
    } else if (file_size == 49696) { // 192x256 image (32 byte header)
        hdr_size = 32;
        w = 192;
        h = 256;
    } else if (file_size == 49680) { // 192x256 image (16 byte header)
        hdr_size = 16;
        w = 192;
        h = 256;
    } else {
        fclose(f);
        return NULL;
    }

    uint8_t hdr[32];
    uint8_t pal_raw[512];
    uint8_t *indices = (uint8_t *)malloc(w * h);
    if (!indices) {
        fclose(f);
        return NULL;
    }

    fread(hdr, 1, hdr_size, f);
    fread(pal_raw, 1, 512, f);
    size_t read_bytes = fread(indices, 1, w * h, f);
    fclose(f);

    if ((int)read_bytes != w * h) {
        free(indices);
        return NULL;
    }

    // Convert palette RGB555 -> ARGB8888
    uint32_t palette[256];
    for (int i = 0; i < 256; i++) {
        uint16_t val = (uint16_t)(pal_raw[i * 2] | (pal_raw[i * 2 + 1] << 8));
        uint32_t r = (val & 0x1F) << 3;
        uint32_t g = ((val >> 5) & 0x1F) << 3;
        uint32_t b = ((val >> 10) & 0x1F) << 3;
        palette[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }

    uint32_t *output = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!output) {
        free(indices);
        return NULL;
    }

    for (int i = 0; i < w * h; i++) {
        output[i] = palette[indices[i]];
    }

    free(indices);
    *out_w = w;
    *out_h = h;
    return output;
}

bool Anim_Init(const char *asset_root) {
    memset(s_Anims, 0, sizeof(s_Anims));

    // 1. ED_PHONE Animation (ED08_1 .. ED08_4)
    CharacterAnimation *ed = &s_Anims[ANIM_ED_PHONE];
    strcpy(ed->name, "ED_PHONE");
    ed->frame_delay_ms = 140; // ~7 FPS classic rotoscope wobble
    ed->is_looping = true;
    ed->frame_count = 0;

    for (int i = 1; i <= 4; i++) {
        char path[260];
        snprintf(path, sizeof(path), "%s/data/show/VSScene/ED08_%d.bin", asset_root, i);
        int fw = 0, fh = 0;
        uint32_t *frame = LoadCingFrame(path, &fw, &fh);
        if (frame) {
            ed->frames[ed->frame_count++] = frame;
            ed->frame_w = fw;
            ed->frame_h = fh;
        }
    }

    // 2. OPENING_CUTSCENE Animation (09-01L .. 09-04L)
    CharacterAnimation *op = &s_Anims[ANIM_OPENING_CUTSCENE];
    strcpy(op->name, "OPENING_CUTSCENE");
    op->frame_delay_ms = 160;
    op->is_looping = true;
    op->frame_count = 0;

    for (int i = 1; i <= 4; i++) {
        char path[260];
        snprintf(path, sizeof(path), "%s/data/show/op2/0829OPImg/09-0%dL.bin", asset_root, i);
        int fw = 0, fh = 0;
        uint32_t *frame = LoadCingFrame(path, &fw, &fh);
        if (frame) {
            op->frames[op->frame_count++] = frame;
            op->frame_w = fw;
            op->frame_h = fh;
        }
    }

    // 3. KYLE_INSPECTION (08-03L / 08-03R)
    CharacterAnimation *kyle = &s_Anims[ANIM_KYLE_INSPECTION];
    strcpy(kyle->name, "KYLE_INSPECTION");
    kyle->frame_delay_ms = 250;
    kyle->is_looping = true;
    kyle->frame_count = 0;

    const char *kyle_files[] = {
        "data/show/car/08-03L.bin",
        "data/show/car/08-03R.bin"
    };
    for (int i = 0; i < 2; i++) {
        char path[260];
        snprintf(path, sizeof(path), "%s/%s", asset_root, kyle_files[i]);
        int fw = 0, fh = 0;
        uint32_t *frame = LoadCingFrame(path, &fw, &fh);
        if (frame) {
            kyle->frames[kyle->frame_count++] = frame;
            kyle->frame_w = fw;
            kyle->frame_h = fh;
        }
    }

    // 4. MILA_SILENCE (miraL / miraR)
    CharacterAnimation *mila = &s_Anims[ANIM_MILA_SILENCE];
    strcpy(mila->name, "MILA_SILENCE");
    mila->frame_delay_ms = 350;
    mila->is_looping = true;
    mila->frame_count = 0;

    const char *mila_files[] = {
        "data/show/car/miraL.bin",
        "data/show/car/miraR.bin"
    };
    for (int i = 0; i < 2; i++) {
        char path[260];
        snprintf(path, sizeof(path), "%s/%s", asset_root, mila_files[i]);
        int fw = 0, fh = 0;
        uint32_t *frame = LoadCingFrame(path, &fw, &fh);
        if (frame) {
            mila->frames[mila->frame_count++] = frame;
            mila->frame_w = fw;
            mila->frame_h = fh;
        }
    }

    printf("[ROTOSCOPE] Loaded 4 character animation sets (Ed: %d, Op: %d, Kyle: %d, Mila: %d frames)\n",
           ed->frame_count, op->frame_count, kyle->frame_count, mila->frame_count);

    s_CurrentAnim = ANIM_ED_PHONE;
    return true;
}

void Anim_Shutdown(void) {
    for (int i = 0; i < ANIM_COUNT; i++) {
        for (int f = 0; f < s_Anims[i].frame_count; f++) {
            if (s_Anims[i].frames[f]) {
                free(s_Anims[i].frames[f]);
                s_Anims[i].frames[f] = NULL;
            }
        }
    }
}

void Anim_SetCurrent(AnimID id) {
    if (id >= 0 && id < ANIM_COUNT) {
        s_CurrentAnim = id;
        s_Anims[id].current_frame = 0;
        s_Anims[id].timer_ms = 0;
    }
}

AnimID Anim_GetCurrent(void) {
    return s_CurrentAnim;
}

void Anim_Update(uint32_t delta_ms) {
    CharacterAnimation *cur = &s_Anims[s_CurrentAnim];
    if (cur->frame_count <= 1) return;

    cur->timer_ms += delta_ms;
    if (cur->timer_ms >= cur->frame_delay_ms) {
        cur->timer_ms -= cur->frame_delay_ms;
        cur->current_frame++;
        if (cur->current_frame >= cur->frame_count) {
            if (cur->is_looping) {
                cur->current_frame = 0;
            } else {
                cur->current_frame = cur->frame_count - 1;
            }
        }
    }
}

void Anim_DrawCurrentFrame(uint32_t *dst_fb, int dst_w, int dst_h, int dst_x, int dst_y) {
    if (!dst_fb) return;
    CharacterAnimation *cur = &s_Anims[s_CurrentAnim];
    if (cur->frame_count == 0) return;

    uint32_t *frame_data = cur->frames[cur->current_frame];
    if (!frame_data) return;

    int fw = cur->frame_w;
    int fh = cur->frame_h;

    // Centering offset if frame is wider than target
    int offset_x = (fw > dst_w) ? (fw - dst_w) / 2 : 0;
    int offset_y = (fh > dst_h) ? (fh - dst_h) / 2 : 0;

    for (int y = 0; y < dst_h; y++) {
        int sy = y + offset_y;
        if (sy < 0 || sy >= fh) continue;
        int dy = dst_y + y;
        if (dy < 0 || dy >= dst_h) continue;

        for (int x = 0; x < dst_w; x++) {
            int sx = x + offset_x;
            if (sx < 0 || sx >= fw) continue;
            int dx = dst_x + x;
            if (dx < 0 || dx >= dst_w) continue;

            uint32_t pixel = frame_data[sy * fw + sx];
            dst_fb[dy * dst_w + dx] = pixel;
        }
    }
}
