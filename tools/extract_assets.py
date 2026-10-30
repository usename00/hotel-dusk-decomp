#!/usr/bin/env python3
"""
Hotel Dusk: Room 215 (USA) - Asset Extraction & Decompression Tool
Extracts NitroFS, overlays, binaries, and Cing (.tbl/.wpf) packages from baserom.nds.
"""

import os
import sys
import struct

def blz_invert(buf, start, length):
    buf[start:start+length] = buf[start:start+length][::-1]

def blz_decode(data):
    pak_len = len(data)
    if pak_len < 8: return data
    inc_len = struct.unpack('<I', data[-4:])[0]
    if inc_len == 0: return data
    hdr_len = data[-5]
    if hdr_len < 8 or hdr_len > 11 or pak_len <= hdr_len: return data
    enc_len = struct.unpack('<I', data[-8:-4])[0] & 0x00FFFFFF
    dec_len = pak_len - enc_len
    comp_len = enc_len - hdr_len
    raw_len = dec_len + enc_len + inc_len

    pak = bytearray(data)
    raw = bytearray(raw_len)
    raw[:dec_len] = pak[:dec_len]
    blz_invert(pak, dec_len, comp_len)

    pak_idx = dec_len
    pak_end = dec_len + comp_len
    raw_idx = dec_len
    raw_end = raw_len
    flags = 0
    mask = 0

    while raw_idx < raw_end:
        mask >>= 1
        if mask == 0:
            if pak_idx >= pak_end: break
            flags = pak[pak_idx]
            pak_idx += 1
            mask = 0x80
        if not (flags & mask):
            if pak_idx >= pak_end: break
            raw[raw_idx] = pak[pak_idx]
            raw_idx += 1
            pak_idx += 1
        else:
            if pak_idx + 1 >= pak_end: break
            val = (pak[pak_idx] << 8) | pak[pak_idx+1]
            pak_idx += 2
            length = (val >> 12) + 3
            if raw_idx + length > raw_end:
                length = raw_end - raw_idx
            pos = (val & 0xFFF) + 3
            for _ in range(length):
                raw[raw_idx] = raw[raw_idx - pos]
                raw_idx += 1

    blz_invert(raw, dec_len, raw_len - dec_len)
    return bytes(raw[:raw_idx])

def hibit(n):
    if n <= 0: return 0
    n |= (n >> 1)
    n |= (n >> 2)
    n |= (n >> 4)
    n |= (n >> 8)
    n |= (n >> 16)
    return n - (n >> 1)

def decompress_cing_lzss(data, size_un, size_co):
    uncompressed = bytearray(size_un)
    pos = 0
    in_pos = 0
    while in_pos < size_co:
        inp = data[in_pos]
        in_pos += 1
        for i in range(8):
            if in_pos >= size_co and (inp >> i) == 0: break
            bit = (inp >> i) & 1
            if bit:
                if in_pos >= size_co or pos >= size_un: break
                uncompressed[pos] = data[in_pos]
                in_pos += 1
                pos += 1
            else:
                if in_pos + 3 > size_co or pos >= size_un: break
                b0 = data[in_pos]
                b1 = data[in_pos+1]
                b2 = data[in_pos+2]
                in_pos += 3
                length = 4 + b2
                offset = (b1 << 8) + b0
                
                posHi = hibit(pos - 259)
                offHi = hibit(offset)
                if pos < 0x10000:
                    signed = offset if offset < 0x8000 else offset - 0x10000
                    if signed < 0 and signed + 259 >= 0: offset = signed
                elif posHi >= 0x20000 and offHi < posHi:
                    if offset + posHi < pos: offset += posHi
                    elif offset + 0x10000 < pos: offset += 0x10000
                elif posHi >= 0x10000 and offHi < posHi and offset + posHi < pos:
                    offset += posHi
                offset += 259
                
                for x in range(length):
                    if pos + x < size_un:
                        if 0 <= offset + x < len(uncompressed):
                            uncompressed[pos + x] = uncompressed[offset + x]
                        else:
                            uncompressed[pos + x] = 0
                pos += length
    return bytes(uncompressed[:pos])

def extract(rom_path, out_dir):
    nitro_dir = os.path.join(out_dir, 'nitrofs')
    overlays_dir = os.path.join(out_dir, 'overlays')
    bin_dir = os.path.join(out_dir, 'binaries')
    unpacked_dir = os.path.join(out_dir, 'unpacked')

    for d in [nitro_dir, overlays_dir, bin_dir, unpacked_dir]:
        os.makedirs(d, exist_ok=True)

    print(f'Reading ROM: {rom_path}...')
    with open(rom_path, 'rb') as f:
        hdr = f.read(0x200)
        arm9_off, arm9_entry, arm9_ram, arm9_sz = struct.unpack('<IIII', hdr[0x20:0x30])
        arm7_off, arm7_entry, arm7_ram, arm7_sz = struct.unpack('<IIII', hdr[0x30:0x40])
        fnt_off, fnt_sz = struct.unpack('<II', hdr[0x40:0x48])
        fat_off, fat_sz = struct.unpack('<II', hdr[0x48:0x50])
        arm9_ovt_off, arm9_ovt_sz = struct.unpack('<II', hdr[0x50:0x58])

        # ARM9
        f.seek(arm9_off)
        arm9_raw = f.read(arm9_sz)
        with open(os.path.join(bin_dir, 'arm9_raw.bin'), 'wb') as out_f:
            out_f.write(arm9_raw)
        with open(os.path.join(bin_dir, 'arm9_decompressed.bin'), 'wb') as out_f:
            out_f.write(blz_decode(arm9_raw))

        # ARM7
        f.seek(arm7_off)
        arm7_raw = f.read(arm7_sz)
        with open(os.path.join(bin_dir, 'arm7.bin'), 'wb') as out_f:
            out_f.write(arm7_raw)

        # Overlays
        f.seek(fat_off)
        fat_data = f.read(fat_sz)
        f.seek(arm9_ovt_off)
        ovt_data = f.read(arm9_ovt_sz)
        num_overlays = arm9_ovt_sz // 32

        for i in range(num_overlays):
            ov_id, ram_addr, ram_sz, bss_sz, init_start, init_end, file_id, flags = struct.unpack('<IIIIIIII', ovt_data[i*32:(i+1)*32])
            start, end = struct.unpack('<II', fat_data[file_id*8:(file_id+1)*8])
            f.seek(start)
            ov_raw = f.read(end - start)
            with open(os.path.join(overlays_dir, f'overlay_{ov_id:04d}_raw.bin'), 'wb') as out_f:
                out_f.write(ov_raw)
            with open(os.path.join(overlays_dir, f'overlay_{ov_id:04d}.bin'), 'wb') as out_f:
                out_f.write(blz_decode(ov_raw))

        # NitroFS
        f.seek(fnt_off)
        fnt_data = f.read(fnt_sz)
        entry_offset, first_file_id, total_dirs = struct.unpack('<IHH', fnt_data[0:8])
        dir_table = [(entry_offset, first_file_id, total_dirs)]
        for i in range(1, total_dirs):
            dir_table.append(struct.unpack('<IHH', fnt_data[i*8:(i+1)*8]))

        dir_paths = {0: ''}
        nitro_files = []
        for dir_idx in range(total_dirs):
            eo, first_fid, pid = dir_table[dir_idx]
            pos = eo
            cur_fid = first_fid
            cur_p = dir_paths.get(dir_idx, '')
            while pos < len(fnt_data):
                tl = fnt_data[pos]
                pos += 1
                if tl == 0: break
                is_d = (tl & 0x80) != 0
                nl = tl & 0x7F
                name = fnt_data[pos:pos+nl].decode('latin1')
                pos += nl
                if is_d:
                    s_id = struct.unpack('<H', fnt_data[pos:pos+2])[0] & 0x0FFF
                    pos += 2
                    dir_paths[s_id] = f'{cur_p}/{name}' if cur_p else name
                else:
                    fp = f'{cur_p}/{name}' if cur_p else name
                    nitro_files.append((cur_fid, fp))
                    cur_fid += 1

        tbl_wpf = {}
        for fid, rel_path in nitro_files:
            start, end = struct.unpack('<II', fat_data[fid*8:(fid+1)*8])
            f.seek(start)
            data = f.read(end - start)
            dest_file = os.path.join(nitro_dir, rel_path.replace('/', os.sep))
            os.makedirs(os.path.dirname(dest_file), exist_ok=True)
            with open(dest_file, 'wb') as out_f:
                out_f.write(data)
            
            base, ext = os.path.splitext(rel_path)
            if ext == '.tbl': tbl_wpf.setdefault(base, {})['tbl'] = dest_file
            elif ext == '.wpf': tbl_wpf.setdefault(base, {})['wpf'] = dest_file

        # Unpack Cing archives
        for base, pair in tbl_wpf.items():
            if 'tbl' not in pair or 'wpf' not in pair: continue
            with open(pair['tbl'], 'rb') as tf: tbl_buf = tf.read()
            with open(pair['wpf'], 'rb') as wf: wpf_buf = wf.read()
            target_dir = os.path.join(unpacked_dir, base.replace('/', os.sep))
            os.makedirs(target_dir, exist_ok=True)
            for i in range(0, len(tbl_buf), 32):
                chunk = tbl_buf[i:i+32]
                name = chunk[:24].rstrip(b'\x00').decode('latin1', errors='ignore')
                if not name: break
                size, off = struct.unpack('<II', chunk[24:32])
                if off + 48 > len(wpf_buf): continue
                chunk_hdr = wpf_buf[off:off+48]
                magic, s_un, s_co, zero = struct.unpack('<IIII', chunk_hdr[32:48])
                clean_name = name.lstrip('\\/').replace('/', '_').replace('\\', '_')
                out_subpath = os.path.join(target_dir, clean_name)
                if (magic & 0xFF) == 1 and s_co != 0xFFFFFFFF:
                    unc = decompress_cing_lzss(wpf_buf[off+48:off+48+s_co], s_un, s_co)
                else:
                    unc = wpf_buf[off+48:off+48+s_un]
                with open(out_subpath, 'wb') as sf: sf.write(unc)

    print(f'Extraction complete! All files saved to {out_dir}')

if __name__ == '__main__':
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    rom = sys.argv[1] if len(sys.argv) > 1 else os.path.join(root, 'baserom.nds')
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(root, 'build', 'extracted')
    if not os.path.exists(rom):
        print(f'Error: ROM not found at {rom}. Place baserom.nds in repository root.')
        sys.exit(1)
    extract(rom, out)
