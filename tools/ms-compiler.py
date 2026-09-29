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

from _file_parsers import line_reader

import re
import sys
import struct
import argparse
from collections import OrderedDict
from typing import NamedTuple, TextIO, Final, Generator, Optional

MS_BIAS_X = 128
MS_BIAS_Y = 64


class MultiFileErrors(Exception):
    def __init__(self, message: str, errors: list[tuple[int, str]]):
        self.message: Final = message
        self.errors: Final = errors

    def print(self, fp: TextIO) -> None:
        fp.write(f"{ self.message }:\n")
        for line, e in self.errors:
            fp.write(f"    line {line}: { e }\n")


class MsSprite(NamedTuple):
    line_no: int
    size: bool
    x: int
    y: int
    tile: int
    palette: Optional[int]
    order: Optional[int]
    hflip: bool
    vflip: bool


class MsFrame(NamedTuple):
    line_no: int
    palette: Optional[int]
    order: Optional[int]
    sprites: list[MsSprite]


class MsClonedFrame(NamedTuple):
    line_no: int
    clone: str
    hflip: bool
    vflip: bool


class MsFrameset(NamedTuple):
    name: str
    tile_offset: int
    palette: Optional[int]
    order: Optional[int]
    frames: OrderedDict[str, MsFrame | MsClonedFrame]


FRAMESET_HEADER_REGEX: Final = re.compile(r"^\[(\w+)\]$")
FRAME_PARAMETER_REGEX: Final = re.compile(r"^(\w+) = (.+)$")
FRAME_NAME_REGEX: Final = re.compile(r"^##\s+(.+)$")

FRAME_CLONE_REGEX: Final = re.compile(r"^clone (\w+)(\s+hflip| vflip)*")
FRAME_SPRITE_REGEX: Final = re.compile(r"^(s|l).*")

SIZE_MAP: Final = {
    "s": False,
    "small": False,
    "l": True,
    "large": True,
}


class PeekableLineReader:
    def __init__(self, fp: TextIO, comment_char: str):
        self.errors: list[tuple[int, str]] = list()

        self.lines = list(line_reader(fp, ";"))
        self.lines.reverse()
        self.next_line_no, self.next_line = self.lines.pop()
        self.prev_line_no = 0

    def peek(self) -> str:
        return self.next_line

    def take_next(self) -> str:
        next_line = self.next_line
        try:
            self.prev_line_no = self.next_line_no
            self.next_line_no, self.next_line = self.lines.pop()
        except IndexError:
            self.next_line = ""
        return next_line

    def take_if_matches_regex(self, r) -> Optional[re.Match]:
        m = r.match(self.next_line)
        if m:
            try:
                self.prev_line_no = self.next_line_no
                self.next_line_no, self.next_line = self.lines.pop()
            except IndexError:
                self.next_line = ""
        return m

    def not_empty(self) -> bool:
        return bool(self.next_line) and bool(self.lines)

    def add_error(self, message: str):
        self.errors.append((self.prev_line_no, message))

    def add_error_on_line(self, line_no: int, message: str):
        self.errors.append((line_no, message))


def parse_number(s: str) -> int:
    if s.startswith("$"):
        return int(s[1:], 16)
    else:
        return int(s)


def _read_ms_clone_frame(line_no: int, m: re.Match) -> MsClonedFrame:
    hflip = False
    vflip = False

    if flip_str := m.group(2):
        for f in flip_str.split():
            if f == "hflip":
                hflip = True
            elif f == "vflip":
                vflip = True

    return MsClonedFrame(line_no, m.group(1), hflip, vflip)


def _read_ms_frame(lr: PeekableLineReader) -> MsFrame | MsClonedFrame:
    frame_line_no = lr.prev_line_no

    if m := lr.take_if_matches_regex(FRAME_CLONE_REGEX):
        return _read_ms_clone_frame(lr.prev_line_no, m)

    frame_palette = None
    frame_order = None
    sprites: list[MsSprite] = list()

    while m := lr.take_if_matches_regex(FRAME_PARAMETER_REGEX):
        pname = m.group(1)
        if pname == "palette":
            frame_palette = parse_number(m.group(2))
        elif pname == "order":
            frame_order = parse_number(m.group(2))
        else:
            lr.add_error("Unknown frame parameter")

    while m := lr.take_if_matches_regex(FRAME_SPRITE_REGEX):
        try:
            line = m.group(0).split()
            if len(line) >= 4:
                # Format:
                # size x y tile [pN] [oN] [hflip] [vflip]
                size = SIZE_MAP[line[0]]
                x = parse_number(line[1])
                y = parse_number(line[2])
                tile = parse_number(line[3])
                palette = None
                order = None
                hflip = False
                vflip = False

                for word in line[4:]:
                    if word.startswith("p"):
                        palette = int(word[1:])
                    elif word.startswith("o"):
                        order = int(word[1:])
                    elif word == "hflip":
                        hflip = True
                    elif word == "vflip":
                        vflip = True
                    else:
                        raise ValueError(f"Unknown sprite value: {word}")

                sprites.append(
                    MsSprite(
                        lr.prev_line_no, size, x, y, tile, palette, order, hflip, vflip
                    )
                )
            else:
                raise ValueError("Not enough items in list")
        except Exception as e:
            lr.add_error(f"Invalid frame sprite: {e}")

    if not sprites:
        lr.add_error_on_line(frame_line_no, "No sprites in frame")

    return MsFrame(frame_line_no, frame_palette, frame_order, sprites)


def _read_ms_frameset(
    name: str,
    lr: PeekableLineReader,
) -> Optional[MsFrameset]:
    tile_offset = 0
    palette = None
    order = None
    frames: OrderedDict[str, MsFrame | MsClonedFrame] = OrderedDict()

    while m := lr.take_if_matches_regex(FRAME_PARAMETER_REGEX):
        pname = m.group(1)
        if pname == "tile_offset":
            tile_offset = int(m.group(2))
        elif pname == "palette":
            palette = int(m.group(2))
        elif pname == "order":
            order = int(m.group(2))
        else:
            lr.add_error("Unknown frameset parameter")

    while m := lr.take_if_matches_regex(FRAME_NAME_REGEX):
        frame_name = m.group(1)
        frame = _read_ms_frame(lr)

        if frame_name in frames:
            lr.add_error(f"Error: Duplicate frame name: {frame_name}")
        frames[frame_name] = frame

    return MsFrameset(name, tile_offset, palette, order, frames)


def read_metasprites_file(filename: str) -> list[MsFrameset]:
    framesets: list[MsFrameset] = list()

    with open(filename, "r") as fp:
        lr = PeekableLineReader(fp, ";")

        while lr.not_empty():
            line = lr.take_next()
            if m := FRAMESET_HEADER_REGEX.match(line):
                if fs := _read_ms_frameset(m.group(1), lr):
                    framesets.append(fs)
            else:
                lr.add_error("Expected a frameset name in [] brackets")

    if lr.errors:
        raise MultiFileErrors("Error reading MetaSprite file", lr.errors)

    return framesets


class ErrorList:
    def __init__(self) -> None:
        self.errors: list[tuple[int, str]] = list()

    def add_error(self, line_no: int, message: str) -> None:
        self.errors.append((line_no, message))


def compile_ms_frame(f: MsFrame, msfs: MsFrameset, errors: ErrorList) -> bytes:
    palette = f.palette if f.palette is not None else msfs.palette
    order = f.order if f.order is not None else msfs.order

    out = bytearray()

    if len(f.sprites) > 16:
        errors.add_error(f.line_no, "too many sprites in frame")

    for s in f.sprites:
        xpos = s.x + MS_BIAS_X
        ypos = s.y + MS_BIAS_Y
        tile = msfs.tile_offset + s.tile
        palette = s.palette if s.palette is not None else palette
        order = s.order if s.order is not None else order

        if not 1 <= xpos <= 255:
            errors.add_error(s.line_no, f"sprite X position out of range: {s.x}")
            xpos = 1

        if not 0 <= ypos <= 127:
            errors.add_error(s.line_no, f"sprite Y position out of range: {s.y}")
            ypos = 1

        if not 0 <= tile <= 511:
            errors.add_error(s.line_no, f"invalid sprite tile: {tile} (${tile:03x})")

        if palette is None:
            errors.add_error(s.line_no, "no palette in sprite")
            palette = 0
        if not 0 <= palette <= 7:
            errors.add_error(s.line_no, f"invalid sprite palette: {palette}")

        if order is None:
            errors.add_error(s.line_no, "no order in sprite")
            order = 0
        if not 0 <= order <= 3:
            errors.add_error(s.line_no, f"invalid sprite order: {order}")

        assert xpos != 0

        out.append(xpos)
        out.append((ypos << 1) | int(s.size))
        out.append(tile & 0xFF)
        out.append(
            (tile >> 8)
            | (palette << 1)
            | (order << 4)
            | (s.hflip << 6)
            | (s.vflip << 7)
        )

    out.append(0)

    return bytes(out)


def flip_sprite(s: MsSprite, hflip: bool, vflip: bool) -> MsSprite:
    size = 16 if s.size else 8

    return MsSprite(
        line_no=s.line_no,
        size=s.size,
        x=-s.x - size if hflip else s.x,
        y=-s.y - size if vflip else s.y,
        tile=s.tile,
        palette=s.palette,
        order=s.order,
        hflip=s.hflip ^ hflip,
        vflip=s.vflip ^ vflip,
    )


def clone_frame(
    cf: MsClonedFrame, msfs: MsFrameset, cloned_frames: dict[str, MsFrame]
) -> MsFrame:
    source: Optional[MsFrame | MsClonedFrame] = cloned_frames.get(cf.clone)

    if source is None:
        source = msfs.frames.get(cf.clone)

    if isinstance(source, MsFrame):
        sprites = [flip_sprite(s, cf.hflip, cf.vflip) for s in source.sprites]
        return source._replace(line_no=cf.line_no, sprites=sprites)
    elif isinstance(source, MsClonedFrame):
        raise RuntimeError(f"Unable to clone an uncloned frame: {cf.clone}")
    else:
        raise RuntimeError(f"Cannot find frame: {cf.clone}")


def write_u16(out: bytearray, value: int) -> None:
    out.append(value & 0xFF)
    out.append(value >> 8)


def compile_metasprites(metasprites: list[MsFrameset]) -> bytes:
    errors = ErrorList()

    cloned_frames: dict[str, MsFrame] = {}

    out_fs_table = bytearray()
    out_frame_table = bytearray()
    out_frame_data = bytearray()

    frame_deduplicator: dict[bytes, int] = {}

    fs_table_offset = len(metasprites) * 2
    frame_table_offset = fs_table_offset + sum(len(fs.frames) * 2 for fs in metasprites)

    for msfs in metasprites:
        write_u16(out_fs_table, len(out_frame_table) + fs_table_offset)

        for fname, f in msfs.frames.items():
            if isinstance(f, MsFrame):
                fd = compile_ms_frame(f, msfs, errors)
            else:
                try:
                    cf = clone_frame(f, msfs, cloned_frames)
                    cloned_frames[fname] = cf
                    fd = compile_ms_frame(cf, msfs, errors)
                except Exception as e:
                    errors.add_error(f.line_no, str(e))
                    fd = bytes()

            f_offset = frame_deduplicator.get(fd)
            if f_offset is None:
                f_offset = len(out_frame_data) + frame_table_offset
                out_frame_data += fd
            write_u16(out_frame_table, f_offset)

    if errors.errors:
        raise MultiFileErrors("Error compiling metasprites", errors.errors)

    return out_fs_table + out_frame_table + out_frame_data


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", required=True, help="palette output file")
    parser.add_argument("filename", action="store", help="metasprites filename")

    args = parser.parse_args()

    return args


def main():
    args = parse_arguments()

    try:
        metasprites = read_metasprites_file(args.filename)
        ms_data = compile_metasprites(metasprites)

        with open(args.output, "wb") as fp:
            fp.write(ms_data)

    except MultiFileErrors as e:
        e.print(sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
