#!/usr/bin/env python3
"""Create a disk or flash image for atomsim (--sdcard, --spi-flash).

The image is filled with one byte value, then each FILE@OFFSET is copied in at its offset.

    mkimg.py -o sd.img --size 1M                                        # blank SD card
    mkimg.py -o flash.img --fill 0xff sw/examples/banner/banner.bin@0xc0000   # flash for bootmode 0
"""
import argparse
import sys


def parse_size(s):
    """'1M', '512K', '0xc0000' or '4096' -> bytes"""
    units = {'K': 1 << 10, 'M': 1 << 20, 'G': 1 << 30}
    s = s.strip()
    if s[-1:].upper() in units:
        return int(s[:-1], 0) * units[s[-1].upper()]
    return int(s, 0)


def main():
    p = argparse.ArgumentParser(description="Create a disk or flash image for atomsim.",
                                epilog="example: mkimg.py -o flash.img --fill 0xff banner.bin@0xc0000")
    p.add_argument("files", nargs="*", metavar="FILE@OFFSET", help="file to place at OFFSET (e.g. prog.bin@0xc0000)")
    p.add_argument("-o", "--output", required=True, help="image to write")
    p.add_argument("-s", "--size", type=parse_size, help="image size, e.g. 1M (default: end of the last file)")
    p.add_argument("-f", "--fill", type=lambda s: int(s, 0), default=0, help="fill byte (default: 0; 0xff for an erased flash)")
    args = p.parse_args()

    if not 0 <= args.fill <= 0xff:
        sys.exit("mkimg: --fill must be a byte (0..0xff)")

    parts = []
    for spec in args.files:
        path, sep, off = spec.rpartition("@")
        if not sep:
            sys.exit(f"mkimg: {spec}: expected FILE@OFFSET")
        with open(path, "rb") as f:
            parts.append((parse_size(off), path, f.read()))

    end = max((off + len(data) for off, _, data in parts), default=0)
    size = args.size if args.size is not None else end
    if size == 0:
        sys.exit("mkimg: empty image: give --size or a file")
    if end > size:
        sys.exit(f"mkimg: files end at {end:#x}, past the image size {size:#x}")

    parts.sort()
    for (a_off, a_path, a_data), (b_off, b_path, _) in zip(parts, parts[1:]):
        if a_off + len(a_data) > b_off:
            sys.exit(f"mkimg: {a_path} and {b_path} overlap")

    img = bytearray([args.fill]) * size
    for off, path, data in parts:
        img[off:off + len(data)] = data
        print(f"{path}: {len(data)} bytes at {off:#x}")

    with open(args.output, "wb") as f:
        f.write(img)
    print(f"{args.output}: {size} bytes")


if __name__ == "__main__":
    main()
