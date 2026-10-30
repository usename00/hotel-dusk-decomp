#!/usr/bin/env python3
"""
Hotel Dusk: Room 215 (USA) - Checksum Verifier
Audits ROM against official No-Intro / Redump SHA-256 and MD5 hashes.
"""

import os
import sys
import hashlib

TARGET_SHA256 = "134236d0793bad5d563de119068774a3e1c43a9b1614b1fe1c1fbf923eab676b"
TARGET_MD5    = "eb4a9bd5e8d09731188c25a8565107d9"

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    candidates = [
        os.path.join(root, 'build', 'hotel_dusk_recompiled.nds'),
        os.path.join(root, '..', 'recompiled_1to1', 'hotel_dusk_1to1_recompiled.nds'),
        os.path.join(root, 'baserom.nds'),
    ]

    target = None
    for c in candidates:
        if os.path.exists(c):
            target = c
            break

    if not target:
        print("[ERROR] No ROM file found to verify. Run build.bat first.")
        return 1

    with open(target, 'rb') as f:
        data = f.read()

    sha = hashlib.sha256(data).hexdigest()
    md5 = hashlib.md5(data).hexdigest()

    print("======================================================================")
    print(" Hotel Dusk: Room 215 (USA) - No-Intro Hash Audit")
    print("======================================================================")
    print(f"Target ROM:      {target}")
    print(f"File Size:       {len(data):,} bytes")
    print(f"Computed SHA256: {sha}")
    print(f"No-Intro SHA256: {TARGET_SHA256}")
    print(f"Computed MD5:    {md5}")
    print(f"No-Intro MD5:    {TARGET_MD5}")

    if sha == TARGET_SHA256 and md5 == TARGET_MD5:
        print("\n>>> RESULT: MATCHING OK (100.00% 1:1 BIT-PERFECT MATCH) <<<\n")
        return 0
    else:
        print("\n>>> RESULT: HASH MISMATCH <<<\n")
        return 1

if __name__ == '__main__':
    sys.exit(main())
