#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: © 2026 Marcus Rowe <undisbeliever@gmail.com>
# SPDX-License-Identifier: Zlib
#
# Copyright © 2026 Marcus Rowe <undisbeliever@gmail.com>
#
# This software is provided 'as-is', without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the
# use of this software.
#
# Permission is granted to anyone to use this software for any purpose, including
# commercial applications, and to alter it and redistribute it freely, subject to
# the following restrictions:
#
#    1. The origin of this software must not be misrepresented; you must not
#       claim that you wrote the original software. If you use this software in
#       a product, an acknowledgment in the product documentation would be
#       appreciated but is not required.
#
#    2. Altered source versions must be plainly marked as such, and must not be
#       misrepresented as being the original software.
#
#    3. This notice may not be removed or altered from any source distribution.

from _snes import create_palettes_map, get_palette_id, extract_tileset_tiles, convert_snes_tileset

import PIL.Image
import argparse


def extract_tiles(snescolor_tiles, palettes_map):
    invalid_tiles = list()
    out = list()

    for tile_index, tile in enumerate(snescolor_tiles):
        palette_id, pal_map = get_palette_id(tile, palettes_map)

        if pal_map:
            out.append(bytes([pal_map[c] for c in tile]))
        else:
            invalid_tiles.append(tile_index)

    if invalid_tiles:
        raise ValueError(f"Cannot find palette for tiles {invalid_tiles}")

    return out


FORMATS_BPP = {
    "2bpp": 2,
    "4bpp": 4,
    "8bpp": 8,
}


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", required=True, help="output file")
    parser.add_argument(
        "-f", "--format", required=True, choices=FORMATS_BPP.keys(), help="tile format"
    )
    parser.add_argument("tiles", action="store", help="tiles PNG image")
    parser.add_argument("palette", action="store", help="palette PNG image")

    args = parser.parse_args()

    return args


def main():
    args = parse_arguments()

    bpp = FORMATS_BPP[args.format]

    image = PIL.Image.open(args.tiles)
    palette_image = PIL.Image.open(args.palette)

    tiles = extract_tiles(
        extract_tileset_tiles(image), create_palettes_map(palette_image, bpp)
    )
    tile_data = convert_snes_tileset(tiles, bpp)

    with open(args.output, "wb") as fp:
        fp.write(tile_data)


if __name__ == "__main__":
    main()
