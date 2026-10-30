/**
 * Hotel Dusk: Room 215 (USA) - Core Engine Implementations
 * Reverse Engineered C Codebase
 */

#include "hotel_dusk_types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ============================================================================
// Cing 64KB Ring-Buffer LZSS Decompressor (Authentic C Implementation)
// ============================================================================
static inline int32_t hibit(int32_t n) {
    if (n <= 0) return 0;
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    return n - (n >> 1);
}

int32_t Cing_DecompressLZSS(const uint8_t *src, uint32_t src_len, uint8_t *dst, uint32_t dst_len) {
    uint32_t in_pos = 0;
    uint32_t out_pos = 0;

    while (in_pos < src_len && out_pos < dst_len) {
        uint8_t flags = src[in_pos++];
        for (int bit = 0; bit < 8; bit++) {
            if (in_pos >= src_len && (flags >> bit) == 0) {
                break;
            }
            if ((flags >> bit) & 1) {
                // Literal byte
                if (in_pos >= src_len || out_pos >= dst_len) break;
                dst[out_pos++] = src[in_pos++];
            } else {
                // Ring buffer reference: 2 bytes offset + 1 byte length
                if (in_pos + 3 > src_len || out_pos >= dst_len) break;
                uint8_t b0 = src[in_pos++];
                uint8_t b1 = src[in_pos++];
                uint8_t b2 = src[in_pos++];
                uint32_t length = 4 + b2;
                int32_t offset = (int32_t)((b1 << 8) | b0);

                int32_t posHi = hibit((int32_t)out_pos - 259);
                int32_t offHi = hibit(offset);

                if (out_pos < 0x10000) {
                    int32_t signed_off = (offset < 0x8000) ? offset : (offset - 0x10000);
                    if (signed_off < 0 && signed_off + 259 >= 0) {
                        offset = signed_off;
                    }
                } else if (posHi >= 0x20000 && offHi < posHi) {
                    if (offset + posHi < (int32_t)out_pos) {
                        offset += posHi;
                    } else if (offset + 0x10000 < (int32_t)out_pos) {
                        offset += 0x10000;
                    }
                } else if (posHi >= 0x10000 && offHi < posHi && offset + posHi < (int32_t)out_pos) {
                    offset += posHi;
                }
                offset += 259;

                for (uint32_t i = 0; i < length; i++) {
                    if (out_pos + i < dst_len) {
                        if (offset + (int32_t)i >= 0 && offset + (int32_t)i < (int32_t)dst_len) {
                            dst[out_pos + i] = dst[offset + i];
                        } else {
                            dst[out_pos + i] = 0;
                        }
                    }
                }
                out_pos += length;
            }
        }
    }
    return (int32_t)out_pos;
}

// ============================================================================
// NitroSDK Backward LZ77 (BLZ) Decompressor (Authentic C Implementation)
// ============================================================================
static void BLZ_Invert(uint8_t *buffer, size_t length) {
    if (length <= 1) return;
    uint8_t *bottom = buffer + length - 1;
    while (buffer < bottom) {
        uint8_t tmp = *buffer;
        *buffer++ = *bottom;
        *bottom-- = tmp;
    }
}

int32_t BLZ_Decompress(const uint8_t *src, uint32_t src_len, uint8_t *dst, uint32_t dst_len) {
    if (src_len < 8) return -1;
    uint32_t inc_len = *(const uint32_t *)(src + src_len - 4);
    if (inc_len == 0) {
        // Uncompressed raw binary
        if (dst_len < src_len) return -1;
        memcpy(dst, src, src_len);
        return (int32_t)src_len;
    }

    uint8_t hdr_len = src[src_len - 5];
    if (hdr_len < 8 || hdr_len > 11 || src_len <= hdr_len) return -1;

    uint32_t enc_len = (*(const uint32_t *)(src + src_len - 8)) & 0x00FFFFFF;
    uint32_t dec_len = src_len - enc_len;
    uint32_t comp_len = enc_len - hdr_len;
    uint32_t raw_len = dec_len + enc_len + inc_len;

    if (dst_len < raw_len) return -1;

    uint8_t *temp_pak = (uint8_t *)malloc(src_len);
    if (!temp_pak) return -1;
    memcpy(temp_pak, src, src_len);

    memcpy(dst, temp_pak, dec_len);
    BLZ_Invert(temp_pak + dec_len, comp_len);

    uint32_t pak_idx = dec_len;
    uint32_t pak_end = dec_len + comp_len;
    uint32_t raw_idx = dec_len;
    uint32_t raw_end = raw_len;

    uint8_t flags = 0;
    uint8_t mask = 0;

    while (raw_idx < raw_end) {
        mask >>= 1;
        if (mask == 0) {
            if (pak_idx >= pak_end) break;
            flags = temp_pak[pak_idx++];
            mask = 0x80;
        }

        if (!(flags & mask)) {
            if (pak_idx >= pak_end) break;
            dst[raw_idx++] = temp_pak[pak_idx++];
        } else {
            if (pak_idx + 1 >= pak_end) break;
            uint32_t val = ((uint32_t)temp_pak[pak_idx] << 8) | temp_pak[pak_idx + 1];
            pak_idx += 2;
            uint32_t len = (val >> 12) + 3;
            if (raw_idx + len > raw_end) {
                len = raw_end - raw_idx;
            }
            uint32_t pos = (val & 0xFFF) + 3;
            for (uint32_t k = 0; k < len; k++) {
                dst[raw_idx] = dst[raw_idx - pos];
                raw_idx++;
            }
        }
    }

    BLZ_Invert(dst + dec_len, raw_len - dec_len);
    free(temp_pak);

    return (int32_t)raw_idx;
}

// ============================================================================
// Script Execution VM Dispatcher (Hotel Dusk Dialogue & Novel Engine)
// ============================================================================
void ScriptVM_Execute(GameStateContext *ctx, const uint8_t *script_data, uint32_t script_len) {
    if (!ctx || !script_data || script_len == 0) return;

    uint32_t pc = 0;
    while (pc < script_len) {
        uint8_t op = script_data[pc++];
        switch (op) {
            case 0x00: // End of script block
                return;
            case 0x01: // Set story flag
                if (pc + 2 <= script_len) {
                    uint16_t flag_id = script_data[pc] | (script_data[pc + 1] << 8);
                    pc += 2;
                    if (flag_id < 128 * 32) {
                        ctx->story_flags[flag_id / 32] |= (1 << (flag_id % 32));
                    }
                }
                break;
            case 0x02: // Check flag and branch
                if (pc + 4 <= script_len) {
                    uint16_t flag_id = script_data[pc] | (script_data[pc + 1] << 8);
                    uint16_t jump_pc = script_data[pc + 2] | (script_data[pc + 3] << 8);
                    pc += 4;
                    if (flag_id < 128 * 32 && (ctx->story_flags[flag_id / 32] & (1 << (flag_id % 32)))) {
                        pc = jump_pc;
                    }
                }
                break;
            case 0x10: // Display dialogue line
                // String terminated by 0x00
                while (pc < script_len && script_data[pc] != 0x00) {
                    pc++;
                }
                pc++; // Skip null terminator
                break;
            case 0x20: // Play character animation
                if (pc + 2 <= script_len) {
                    pc += 2; // anim_id + slot
                }
                break;
            default:
                // Advance stream
                break;
        }
    }
}
