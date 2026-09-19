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

from _file_parsers import read_resources_txt, Resources

import argparse
import gzip
import sys
import base64
import struct
import xml.etree.ElementTree
from typing import NamedTuple, TextIO, Final, Generator


class MultiError(Exception):
    def __init__(self, message: str, errors: list[str]):
        self.message: Final = message
        self.errors: Final = errors

    def print(self, fp: TextIO) -> None:
        fp.write(f"{ self.message }:\n")
        for e in self.errors:
            fp.write(f"    { e }\n")


class MapFile(NamedTuple):
    width: int
    height: int
    map_data: bytes
    properties: dict[str, str]


def read_xml_attrib_int(tag: xml.etree.ElementTree.Element, name: str) -> int:
    a = tag.attrib.get(name)
    if not a:
        raise RuntimeError(f"<{tag.tag}>: Missing {name} attribute")

    try:
        return int(a)
    except ValueError:
        raise RuntimeError(f"<{tag.tag} {name}>: Cannot convert attribute to int")


def read_layer_tag(
    tag: xml.etree.ElementTree.Element, map_width: int, map_height: int
) -> list[int]:
    layer_width = read_xml_attrib_int(tag, "width")
    layer_height = read_xml_attrib_int(tag, "height")

    if (layer_width, layer_height) != (map_width, map_height):
        raise RuntimeError("<layer> size does not match <map> size")

    if len(tag) != 1 or tag[0].tag != "data":
        raise RuntimeError("<layer> has no data")

    data_tag = tag[0]
    if data_tag.attrib.get("compression") != "gzip":
        raise RuntimeError("<layer> is not gzip compressed")
    if not data_tag.text:
        raise RuntimeError("<layer> has no data")

    data = gzip.decompress(base64.b64decode(data_tag.text))

    if len(data) != layer_width * layer_height * 4:
        raise RuntimeError("<layer> data is the incorrect size")

    return [i[0] for i in struct.iter_unpack("<I", data)]


EXPECTED_MAP_ATTRIBUTES: Final = (
    ("orientation", "orthogonal"),
    ("renderorder", "right-down"),
    ("tilewidth", "16"),
    ("tileheight", "16"),
    ("infinite", "0"),
)


def read_tmx_map_file(filename: str) -> MapFile:
    errors = list[str]()

    with open(filename, "r") as fp:
        xml_tree = xml.etree.ElementTree.parse(fp)

    root = xml_tree.getroot()

    if root.tag != "map":
        raise RuntimeError(f"Not a TMX file (no <map> root tag)")

    for name, value in EXPECTED_MAP_ATTRIBUTES:
        if root.attrib.get(name) != value:
            errors.append(f'Invalid <map> attribute: requires {name}="{value}"')

    map_width = read_xml_attrib_int(root, "width")
    map_height = read_xml_attrib_int(root, "height")

    firstgid = None
    layer_data = None
    properties = dict()

    for child in root:
        if child.tag == "tileset":
            if firstgid is None:
                firstgid = read_xml_attrib_int(child, "firstgid")
            else:
                errors.append("map can only have one <tileset>")

        elif child.tag == "layer":
            if layer_data is None:
                try:
                    layer_data = read_layer_tag(child, map_width, map_height)
                except Exception as e:
                    errors.append(str(e))
            else:
                errors.append("map can only have one tile layer")

        elif child.tag == "properties":
            for child in child:
                if child.tag == "property":
                    p_name = child.attrib.get("name")
                    p_value = child.attrib.get("value")
                    if p_name and p_value:
                        if p_name not in properties:
                            properties[p_name] = p_value
                        else:
                            errors.append('Duplicate <property name="{p_name}">')
                    else:
                        errors.append("Invalid <property> tag: missing name or value")

    if layer_data is None:
        errors.append("No map layer")

    if firstgid is None:
        errors.append("No tileset")

    map_data = bytearray()
    if layer_data and firstgid is not None:
        flipped_tiles = list()
        invalid_tiles = list()

        for i, t in enumerate(layer_data):
            if t >= 0x100_0000:
                flipped_tiles.append(f"({i % map_width}, {i / map_width})")
            t = t - firstgid
            if t < 0 or t > 255:
                invalid_tiles.append(f"({i % map_width}, {i / map_width})")
            map_data.append(t)

        if flipped_tiles:
            errors.append(
                "flipped tiles are not supported: (" + " ".join(invalid_tiles) + ")"
            )

        if invalid_tiles:
            errors.append("out of bounds map tiles: (" + " ".join(invalid_tiles) + ")")

    if errors:
        raise MultiError("Invalid map", errors)

    return MapFile(map_width, map_height, map_data, properties)


def find_resource_id(
    map_file: MapFile, p_name: str, resources: Resources, errors: list[str]
) -> int:
    res_name = map_file.properties.get(p_name)
    if not res_name:
        errors.append(f"Missing property: {p_name}")
        return 0

    for i, r in enumerate(resources.resources):
        if r.name == res_name:
            return i

    errors.append(f"Cannot find {p_name} resources : {res_name}")
    return 0


def compile_map(map_file: MapFile, resources: Resources) -> bytes:
    errors = list[str]()

    MAX_DATA_SIZE: Final = 4096
    MAX_HEIGHT: Final = MAX_DATA_SIZE / 32

    if map_file.width != 32:
        errors.append("Map must be 32 tiles wide")

    if map_file.height < 16 or map_file.height > MAX_HEIGHT:
        errors.append(f"Invalid map height: must be between 16 - {MAX_HEIGHT}")

    map_tiles = find_resource_id(map_file, "tiles", resources, errors)
    palette = find_resource_id(map_file, "palette", resources, errors)

    if errors:
        raise MultiError("Error compiling map", errors)

    return (
        bytes(
            [
                map_tiles,
                palette,
                map_file.height,
            ]
        )
        + map_file.map_data
    )


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", required=True, help="output file")
    parser.add_argument("tmx_filename", action="store", help="TMX map file")
    parser.add_argument("resources_filename", action="store", help="resources file")

    args = parser.parse_args()

    return args


def main():
    args = parse_arguments()

    try:
        map_file = read_tmx_map_file(args.tmx_filename)
        resources = read_resources_txt(args.resources_filename)

        map_data = compile_map(map_file, resources)

        with open(args.output, "wb") as fp:
            fp.write(map_data)
    except MultiError as e:
        e.print(sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
