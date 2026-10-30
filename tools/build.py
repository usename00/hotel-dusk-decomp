#!/usr/bin/env python3
"""
Hotel Dusk: Room 215 (USA) - 1:1 Matching Build Orchestrator
Standard build tool following pret / n64decomp conventions.
"""

import os
import sys
import shutil

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS_DIR = os.path.join(ROOT_DIR, 'tools')
BUILD_DIR = os.path.join(ROOT_DIR, 'build')

def main():
    print("======================================================================")
    print(" Hotel Dusk: Room 215 (USA) - 1:1 Matching Build Pipeline")
    print("======================================================================")
    print()

    baserom = os.path.join(ROOT_DIR, 'baserom.nds')
    alt_rom = os.path.join(ROOT_DIR, '..', 'Hotel Dusk - Room 215 (USA)', '0823 - Hotel Dusk - Room 215 (USA).nds')

    if not os.path.exists(baserom):
        if os.path.exists(alt_rom):
            print("[INFO] Found local retail dump in workspace. Copying to baserom.nds...")
            shutil.copyfile(alt_rom, baserom)
        else:
            print("[ERROR] baserom.nds not found in repository root!")
            print("Please place your legally dumped 0823 - Hotel Dusk - Room 215 (USA).nds")
            print("into the repository root and rename it to baserom.nds.")
            return 1

    # Check extracted assets
    extract_dir = os.path.join(ROOT_DIR, '..', 'hotel_dusk_extracted')
    if not os.path.exists(os.path.join(extract_dir, 'nitrofs')):
        extract_dir = os.path.join(BUILD_DIR, 'extracted')

    if not os.path.exists(os.path.join(extract_dir, 'nitrofs')):
        print("[STEP 1/2] Extracting assets and overlays from baserom.nds...")
        from extract_assets import extract
        extract(baserom, extract_dir)

    print("[STEP 2/2] Rebuilding master ROM and checking SHA-256 parity...")
    from build_rom import build_rom
    out_rom = os.path.join(BUILD_DIR, 'hotel_dusk_recompiled.nds')
    success = build_rom(extract_dir, baserom, out_rom)

    if success:
        print("======================================================================")
        print(" BUILD SUCCEEDED: 100.00% Matching Bit-Perfect ROM Verified!")
        print(f" Output: {os.path.relpath(out_rom, ROOT_DIR)}")
        print("======================================================================")
        return 0
    else:
        print("[ERROR] Rebuild hash mismatch!")
        return 1

if __name__ == '__main__':
    sys.exit(main())
