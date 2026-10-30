# Hotel Dusk: Room 215 (USA) - Decompilation & 1:1 Matching Rebuild

Matching decompilation, asset extraction toolchain, and native PC port for **Hotel Dusk: Room 215** (Nintendo DS, USA release `AWIE`).

## Overview

- **Matching Parity**: 100% bit-for-bit byte match against retail No-Intro / Redump ROM.
- **Decompiled C Code**: 3,800 functions extracted from ARM9 (`0x02004000`) using Ghidra.
- **Engine Algorithms**: Native C implementation of Cing's 64KB sliding-window LZSS and NitroSDK BLZ backwards LZ77.
- **Native PC Port**: Standalone C/SDL2 PC runtime featuring Book Mode dual-screen layout and mouse stylus emulation.
- **Asset Pipeline**: Automatic NitroFS and Cing `.tbl`/`.wpf` unpacker/repacker.

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
verify.bat
```
or via Python:
```bash
python tools/verify.py
```

## Repository Structure

```text
hotel-dusk-decomp/
├── arm9/
│   ├── include/
│   │   ├── hotel_dusk_arm9.h   # Function prototypes (3,800 functions)
│   │   └── hotel_dusk_types.h  # Hardware registers, memory maps, structs
│   └── src/
│       ├── hotel_dusk_arm9.c   # Decompiled ARM9 C code (3,800 functions, 4.5 MB)
│       ├── hotel_dusk_engine.c # Cing LZSS ring buffer, Nitro BLZ, and script VM
│       └── hotel_dusk_main.c   # Standalone PC entry point
├── pc/
│   ├── include/                # PC port display and font definitions
│   ├── src/                    # SDL2 dual-screen renderer, asset loader, main loop
│   └── build_pc.bat            # MSVC x64 build script for HotelDusk_PC.exe
├── tools/
│   ├── build.py                # Main build orchestrator (pret / n64decomp style)
│   ├── build_rom.py            # Bit-perfect ROM reconstructor
│   ├── extract_assets.py       # NitroFS and Cing .tbl/.wpf package extractor
│   └── verify.py               # SHA-256 and MD5 integrity auditor
├── build.bat                   # 1-click ROM rebuild script
├── verify.bat                  # 1-click integrity audit script
└── play_hotel_dusk_pc.bat      # 1-click native PC port launcher
```

## Native PC Port (Phase 1)

This repository includes a standalone **Native PC Port** built in C with SDL2:
- **Dual-Screen Book Mode**: Side-by-side portrait screens (1184x768 at 3x scale) with authentic hinge spine.
- **Stylus Input**: Mouse cursor maps 1:1 to NDS touchscreen coordinates (0..191, 0..255).
- **Hardware Simulation**: Keyboard triggers for NDS mic blow (`M`), clamshell lid sensor (`L`), and fullscreen (`F11`).
- **Direct Asset Streaming**: Native C loader for Cing 192x256 `.bin` backgrounds and narrative script books.

### Build & Run PC Port
```cmd
play_hotel_dusk_pc.bat
```
*(Automatically builds `build/HotelDusk_PC.exe` via MSVC 2022 and runs immediately).*

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

## Building & Verification (ROM)

### Prerequisites
- Python 3.8+
- An authentic dump of `0823 - Hotel Dusk - Room 215 (USA).nds`

### Quick Start
1. Place your clean ROM dump in the root directory and rename it to `baserom.nds`.
2. Run the build pipeline:
```cmd
build.bat
```
*(On Linux/macOS: `python tools/build.py`)*

3. The recompiled bit-perfect ROM will be written to `build/hotel_dusk_recompiled.nds` and audited against official No-Intro checksums.
