#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set fenc=utf-8 ai ts=4 sw=4 sts=4 et:
#
#
# SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
# SPDX-License-Identifier: Zlib
#
# Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


import PIL.Image
import argparse

from _snes import convert_palette_image


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('-o', '--output', required=True,
                        help='palette output file')
    parser.add_argument('palette_image', action='store',
                        help='Palette png image')

    args = parser.parse_args()

    return args;



def main():
    args = parse_arguments()

    palette_image = PIL.Image.open(args.palette_image)

    palette_data = convert_palette_image(palette_image)

    with open(args.output, 'wb') as fp:
        fp.write(palette_data)



if __name__ == '__main__':
    main()

