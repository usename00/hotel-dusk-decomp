/**
 * Hotel Dusk: Room 215 (USA) - Reverse Engineered C Codebase
 * Game Code: AWIE | Title: HOTELDUSK
 * Target: ARM946E-S @ 0x02004000
 */

#ifndef HOTEL_DUSK_TYPES_H
#define HOTEL_DUSK_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// NDS Hardware & Memory Map Definitions
// ============================================================================
#define NDS_ARM9_RAM_BASE       0x02004000
#define NDS_ARM9_ENTRY_POINT    0x02004800
#define NDS_MAIN_MEMORY_SIZE    0x00400000 // 4MB Main RAM
#define NDS_SCREEN_WIDTH        256
#define NDS_SCREEN_HEIGHT       192

// ============================================================================
// NitroSDK Overlay Table Entry (OVT)
// ============================================================================
typedef struct {
    uint32_t overlay_id;        // Overlay ID (0..44)
    uint32_t ram_address;       // Target RAM address (e.g. 0x021FCB40)
    uint32_t ram_size;          // Uncompressed size in RAM
    uint32_t bss_size;          // BSS section size
    uint32_t sinit_init;        // Static initializer start
    uint32_t sinit_end;         // Static initializer end
    uint32_t file_id;           // File ID in FAT
    uint32_t flags;             // Bit 24: Compressed flag | File size in lower 24 bits
} FSOverlayInfo;

// ============================================================================
// Cing Container Formats (.tbl / .wpf)
// ============================================================================
#define CING_MAGIC_COMPRESSED   0x123DDA01
#define CING_MAGIC_UNCOMPRESSED 0x123DDA00

typedef struct {
    char     filename[24];      // Relative file path (null-terminated ASCII)
    uint32_t file_size;         // Size of chunk
    uint32_t wpf_offset;        // Absolute byte offset within .wpf container
} CingTblRecord;

typedef struct {
    char     filename[24];      // Subfile internal identifier
    uint32_t file_size;         // Total chunk record size
    uint32_t next_offset;       // Offset to next file chunk
    uint32_t magic;             // 0x123DDA01 (LZSS) or 0x123DDA00 (Raw)
    uint32_t uncompressed_size; // Target expansion buffer size
    uint32_t compressed_size;   // Size of compressed payload (0xFFFFFFFF if raw)
    uint32_t reserved;          // Padding / zero
} CingWpfChunkHeader;

// ============================================================================
// Cing LZSS Ring Buffer Decompressor State
// ============================================================================
#define CING_LZSS_WINDOW_SIZE   0x10000 // 64 KB Ring Buffer

typedef struct {
    uint8_t  window[CING_LZSS_WINDOW_SIZE];
    uint16_t window_pos;
    uint32_t output_pos;
    uint32_t output_size;
} CingLZSSContext;

// ============================================================================
// Game State & Script Engine Types
// ============================================================================
typedef enum {
    SCENE_BOOT = 0,
    SCENE_TITLE,
    SCENE_NOVEL_BOOK,           // Book Mode portrait dual-screen narrative
    SCENE_ROOM_EXPLORATION,     // 3D first-person room navigation
    SCENE_CONVERSATION,         // Rotoscoped character dialogue & interrogation
    SCENE_TRICK_PUZZLE,         // Interactive mini-game / touch puzzle
    SCENE_GAME_OVER
} GameSceneMode;

typedef struct {
    uint32_t current_chapter;   // Chapter 1..10
    uint32_t current_room_id;   // Room 101, 215, Lobby, etc.
    GameSceneMode scene_mode;
    uint32_t story_flags[128];  // Progress / discovery bitfield
    uint32_t touch_x;
    uint32_t touch_y;
    bool     touch_pressed;
    bool     mic_blow_active;
    bool     lid_closed;
} GameStateContext;

#ifdef __cplusplus
}
#endif

#endif // HOTEL_DUSK_TYPES_H
