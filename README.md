# Hotel Dusk: Room 215 (USA) - Decompilation & 1:1 Matching Rebuild

Matching decompilation and asset extraction toolchain for **Hotel Dusk: Room 215** (Nintendo DS, USA release `AWIE`).

## Overview

- **Matching Parity**: 100% bit-for-bit byte match against retail No-Intro / Redump ROM.
- **Decompiled C Code**: 3,800 functions extracted from ARM9 (`0x02004000`) using Ghidra 12.1.4 Public.
- **Engine Algorithms**: Native C implementation of Cing's 64KB sliding-window LZSS and NitroSDK BLZ backwards LZ77.
- **Assets**: 7,462 extracted files (765 NitroFS files + 6,605 unpacked files from 275 `.tbl`/`.wpf` archives).

## Verification

Target ROM: `0823 - Hotel Dusk - Room 215 (USA).nds` (134,217,728 bytes)

```text
Original SHA-256:   134236d0793bad5d563de119068774a3e1c43a9b1614b1fe1c1fbf923eab676b
Recompiled SHA-256: 134236d0793bad5d563de119068774a3e1c43a9b1614b1fe1c1fbf923eab676b
Original MD5:       eb4a9bd5e8d09731188c25a8565107d9
Recompiled MD5:     eb4a9bd5e8d09731188c25a8565107d9
Parity:             100.00% exact match (0 byte delta)
```

Run verification on Windows:
```cmd
recompiled_1to1\verify_checksums.bat
```

## Technical Notes

### 1. Cing Container Format (`.tbl` / `.wpf`)
Archives use a split table/pack format:
- `.tbl`: 32-byte records (`char name[24]`, `uint32 size`, `uint32 wpf_offset`).
- `.wpf`: 48-byte chunk header (`name[24]`, `chunk_size`, `next_off`, `magic`, `uncompressed_sz`, `compressed_sz`, `pad`).
- Magic `0x123DDA01`: Compressed payload.
- Magic `0x123DDA00`: Uncompressed raw payload (`compressed_sz == 0xFFFFFFFF`).

### 2. Cing LZSS Algorithm
Sliding-window ring buffer of 65,536 bytes (`0x10000`).
- Flag byte (8 bits, LSB first).
- Flag `1`: Literal byte copied to output and ring buffer.
- Flag `0`: 3 bytes `b0, b1, b2`. `raw_offset = b0 | (b1 << 8)`, `length = 4 + b2`. Source offset uses historical wrap index `(raw_offset + 259) & 0xFFFF`.

### 3. Overlays & ARM9 BLZ Compression
All 45 overlays and the tail of `arm9.bin` are compressed using NitroSDK backwards LZ77 (`MI_UncompressBLZ`).
- Trailer layout (last 8-12 bytes): `enc_len` (3 bytes) + `hdr_len` (1 byte) + `diff` (4 bytes).
- Uncompressed ARM9 executable size: 1,031,352 bytes (expanded from 603,240 bytes).

## Directory Layout

```text
hotel_dusk_decompiled_c/
  hotel_dusk_arm9.c       Decompiled ARM9 functions (3,800 functions, 4.5 MB)
  hotel_dusk_arm9.h       Header prototypes (194 KB)
  hotel_dusk_engine.c     Cing LZSS, BLZ, and script VM implementations
  hotel_dusk_types.h      Hardware registers, memory map, structs
  hotel_dusk_main.c       PC entry point and initialization
  build.bat               MSVC x64 build script

hotel_dusk_extracted/
  nitrofs/                765 NitroFS files across 65 directories
  unpacked/               6,605 unpacked Cing container subfiles (scripts, anims)
  overlays/               45 decompressed overlays (overlay_0000..0044.bin)
  binaries/               arm9_decompressed.bin (1MB), arm9_raw.bin, arm7.bin

recompiled_1to1/
  build_1to1_recompile.py Rebuild script from extracted components
  verify_checksums.bat    Hash audit script
  play_hotel_dusk_1to1.bat 1-click launcher for recompiled build
```

## Building

### Rebuild 1:1 ROM
Requires Python 3.8+:
```cmd
python recompiled_1to1\build_1to1_recompile.py
```

### Compile Native C Code
Requires MSVC 2022 (Community / Build Tools):
```cmd
cd hotel_dusk_decompiled_c
build.bat
```
Produces `hotel_dusk_core.exe`.
