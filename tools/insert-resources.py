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

"""
Inserts the resources into the sfc binary.
"""

from _file_parsers import read_resources_txt, Resources

import argparse
import os.path
import re
from typing import NamedTuple, TextIO, Final, Generator, Optional
from dataclasses import dataclass

ROM_HEADER_SIZE : Final = 0xFFD7

class Lorom:
    BANK_SIZE: Final = 0x8000
    FIRST_BANK: Final = 0x80
    BANK_OFFSET: Final = 0x8000

    @staticmethod
    def addr_to_rom_offset(addr: int) -> int:
        bank = addr >> 16

        if bank == 0x7E or bank == 0x7F or addr & 0xFFFF < 0x8000:
            raise ValueError(f"addr is not a LOROM address: {addr:06x}")

        return ((addr & 0x3F_0000) >> 1) | (addr & 0x7FFF)


class Hirom:
    BANK_SIZE: Final = 0x10000
    FIRST_BANK: Final = 0xC0
    BANK_OFFSET: Final = 0

    @staticmethod
    def addr_to_rom_offset(addr: int) -> int:
        bank = addr >> 16

        if (
            bank == 0x7E
            or bank == 0x7F
            or (bank & 0x7F < 0x40 and addr & 0xFFFF < 0x8000)
        ):
            raise ValueError(f"addr is not a HIROM address: {addr:06x}")

        return addr & 0x3F_FFFF


class Symbols(NamedTuple):
    resources_table: int


def read_symbol_file_lines(sym_filename: str) -> Generator[tuple[str, str], None, None]:
    with open(sym_filename, "r") as f:
        sym_file = f.read()

    if sym_file.startswith("version\tmajor=2"):
        ## ca65 dbg file
        r = re.compile(r'sym.*name="(\w+)",.*,val=0x([A-Fa-f0-9]+),')
        for line in sym_file.splitlines():
            if m := r.match(line):
                yield m.group(1), m.group(2)
    else:
        for line in sym_file.splitlines():
            addr, name = line.split()
            yield name, addr


def read_symbol_file(sym_filename: str) -> Symbols:
    resources_table = None

    for name, addr_str in read_symbol_file_lines(sym_filename):
        if name == "RESOURCES_TABLE" or name == "_RESOURCES_TABLE":
            resources_table = int(addr_str, 16)

    if resources_table is None:
        raise RuntimeError("RESOURCES_TABLE not found in symbol file")

    return Symbols(resources_table=resources_table)


def read_bin_limit(filename: str, limit: int) -> bytes:
    with open(filename, "rb") as f:
        data = f.read()

        if len(data) > limit:
            raise ValueError(
                f"file is too large (read {len(data)} bytes, max {limit}): {filename}"
            )

        return data


def read_resource_files(
    res_list: Resources, mapping: type[Lorom | Hirom], out_dir: str
) -> list[tuple[int, bytes]]:
    return [
        (i, read_bin_limit(os.path.join(out_dir, r.filename), mapping.BANK_SIZE))
        for i, r in enumerate(res_list.resources)
    ]


@dataclass
class ResourceBank:
    data: bytearray
    starting_addr: int


@dataclass
class ResourceTableEntry:
    address: Optional[int]
    size: int


class PackedResources(NamedTuple):
    mapping: type[Lorom | Hirom]
    banks: list[ResourceBank]
    resource_table: list[ResourceTableEntry]


def pack_resources(
    resources: list[tuple[int, bytes]], mapping: type[Lorom | Hirom], rom_size: int
) -> PackedResources:
    starting_bank: Final = (rom_size + mapping.BANK_SIZE - 1) // mapping.BANK_SIZE + mapping.FIRST_BANK
    banks: list[ResourceBank] = []
    resource_table = [ResourceTableEntry(None, len(r)) for i, r in resources]

    resources.sort(key=lambda r: len(r[1]), reverse=True)

    for ri, rdata in resources:
        try:
            b = next(b for b in banks if len(rdata) + len(b.data) < mapping.BANK_SIZE)
            resource_table[ri].address = b.starting_addr + len(b.data)
            b.data += rdata
        except StopIteration:
            # No bank can fit the data, create a new one
            r_addr = (starting_bank + len(banks)) * 0x10000 + mapping.BANK_OFFSET
            banks.append(ResourceBank(starting_addr=r_addr, data=bytearray(rdata)))
            resource_table[ri].address = r_addr

    return PackedResources(mapping=mapping, banks=banks, resource_table=resource_table)


def build_rom(
    base_rom: bytes, resources: PackedResources, symbols: Symbols
) -> bytearray:
    mapping = resources.mapping
    bank_size = resources.mapping.BANK_SIZE

    rom_size = len(resources.banks) * bank_size + len(base_rom)
    # Pad to the next power of 2
    rom_size = 1 << ((rom_size - 1).bit_length())

    out = bytearray(rom_size)
    assert len(out) == rom_size

    out[0 : len(base_rom)] = base_rom

    rt_offset = mapping.addr_to_rom_offset(symbols.resources_table)
    rt_end: Final = rt_offset + len(resources.resource_table) * 5

    if any(v != 0 for v in out[rt_offset:rt_end]):
        raise RuntimeError("RESOURCES_TABLE is not empty or is the wrong size")

    for r in resources.resource_table:
        assert r.address is not None

        out[rt_offset : rt_offset + 5] = [
            r.size & 0xFF,
            r.size >> 8,
            r.address & 0xFF,
            (r.address >> 8) & 0xFF,
            r.address >> 16,
        ]
        rt_offset += 5
    assert rt_offset == rt_end

    for b in resources.banks:
        assert len(b.data) <= bank_size, "bank is too large"

        b_offset = mapping.addr_to_rom_offset(b.starting_addr)
        out[b_offset : b_offset + len(b.data)] = b.data

    assert len(out) == rom_size

    # Update ROM size in the SFC header
    rh_size_offset = mapping.addr_to_rom_offset(ROM_HEADER_SIZE)
    out[rh_size_offset] = (rom_size + 1).bit_length() - 11

    # ::TODO write header checksum::

    return out


def parse_arguments():
    parser = argparse.ArgumentParser(allow_abbrev=False)

    mgroup = parser.add_mutually_exclusive_group(required=True)
    mgroup.add_argument(
        "--lorom", action="store_true", help="sfc file uses LOROM mapping"
    )
    mgroup.add_argument(
        "--hirom", action="store_true", help="sfc file uses HIROM mapping"
    )

    parser.add_argument("-o", "--output", action="store", help="output file")

    parser.add_argument(
        "rom_filename", action="store", help="ROM without resources file"
    )
    parser.add_argument("sym_filename", action="store", help="Symbol file")
    parser.add_argument(
        "resources_filename", action="store", help="resources list file"
    )
    parser.add_argument("out_dir", action="store", help="output resources directory")

    args = parser.parse_args()

    return args


def main() -> None:
    args = parse_arguments()

    mapping: type[Lorom | Hirom]
    if args.lorom:
        mapping = Lorom
    elif args.hirom:
        mapping = Hirom
    else:
        raise RuntimeError("Unknown mapping type")

    resource_list = read_resources_txt(args.resources_filename)
    resources = read_resource_files(resource_list, mapping, args.out_dir)

    base_rom = read_bin_limit(args.rom_filename, 256 * 1024)
    symbols = read_symbol_file(args.sym_filename)

    packed_resources = pack_resources(resources, mapping, len(base_rom))

    out = build_rom(base_rom, packed_resources, symbols)

    with open(args.output, "wb") as fp:
        fp.write(out)


if __name__ == "__main__":
    main()
