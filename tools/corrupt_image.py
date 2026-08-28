#!/usr/bin/env python3
"""
corrupt_image.py

Creates deliberately broken artifacts for negative testing. Two modes:

  flip     - copies an image and flips a single bit somewhere in the
             middle of it, leaving the original .sig/manifest in place
             (so the copy will now fail hash verification).

  copysig  - copies a valid .sig file from a DIFFERENT component onto
             a target name, simulating "wrong signature attached"
             (will fail cryptographic verification even though the
             hash matches, because it was signed by the wrong key,
             or the hash it covers won't match).

Usage:
    python3 corrupt_image.py flip <input.bin> <output.bin>
    python3 corrupt_image.py copysig <source.sig> <output.sig>
"""

import sys
import shutil


def flip_bit(input_path, output_path):
    with open(input_path, "rb") as f:
        data = bytearray(f.read())

    if len(data) == 0:
        print("error: input file is empty, nothing to flip")
        sys.exit(1)

    mid = len(data) // 2
    original_byte = data[mid]
    data[mid] ^= 0x01  # flip the lowest bit
    print(f"flipped byte at offset {mid}: 0x{original_byte:02x} -> 0x{data[mid]:02x}")

    with open(output_path, "wb") as f:
        f.write(data)

    print(f"corrupted image written: {output_path}")


def copy_wrong_sig(source_sig, output_sig):
    shutil.copyfile(source_sig, output_sig)
    print(f"copied signature from '{source_sig}' onto '{output_sig}'")
    print("this simulates an attacker attaching a signature that will not")
    print("match this image's hash / was produced with the wrong key.")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    mode = sys.argv[1]

    if mode == "flip":
        if len(sys.argv) != 4:
            print(__doc__)
            sys.exit(1)
        flip_bit(sys.argv[2], sys.argv[3])
    elif mode == "copysig":
        if len(sys.argv) != 4:
            print(__doc__)
            sys.exit(1)
        copy_wrong_sig(sys.argv[2], sys.argv[3])
    else:
        print(f"unknown mode: {mode}")
        print(__doc__)
        sys.exit(1)


if __name__ == "__main__":
    main()
