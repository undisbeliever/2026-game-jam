#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set fenc=utf-8 ai ts=4 sw=4 sts=4 et:
#
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

from _snes import (
    convert_large_snes_tileset,
    convert_large_tilemap_and_tileset,
    create_tilemap_data_low,
    create_tilemap_data_high,
    create_palettes_map,
    extract_large_tile,
    get_palette_id,
)

import PIL.Image
import argparse
from typing import Generator


def extract_map_tiles(image) -> Generator[list[int], None, None]:
    """Extracts 16x16px metatile tiles from the image."""

    if image.mode != "RGB":
        image = image.convert("RGB")

    if image.size != (256, 256):
        raise ValueError("MT Image MUST be 256x256px in size")

    for ty in range(0, 256, 16):
        for tx in range(0, 256, 16):
            yield extract_large_tile(image, tx, ty)


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", required=True, help="palette output file")
    parser.add_argument("image_filename", action="store", help="Tileset png image")
    parser.add_argument("palette_image", action="store", help="Palette png image")

    args = parser.parse_args()

    return args


def main():
    args = parse_arguments()

    image = PIL.Image.open(args.image_filename)
    palette_image = PIL.Image.open(args.palette_image)

    tilemap, tiles = convert_large_tilemap_and_tileset(
        extract_map_tiles(image), create_palettes_map(palette_image, 4)
    )

    tile_data = convert_large_snes_tileset(tiles, 4)

    tilemap_low = create_tilemap_data_low(tilemap)
    tilemap_high = create_tilemap_data_high(tilemap, 0)

    # ::TODO add properties data::

    assert len(tilemap_low) == 256
    assert len(tilemap_high) == 256

    with open(args.output, "wb") as fp:
        fp.write(tilemap_low)
        fp.write(tilemap_high)
        fp.write(tile_data)


if __name__ == "__main__":
    main()
