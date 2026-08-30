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

import re
from typing import NamedTuple, TextIO, Final, Generator


class ResourceItem(NamedTuple):
    name: str
    filename: str


class Resources(NamedTuple):
    resources: list[ResourceItem]


NAME_REGEX: Final = re.compile(r"^[a-zA-Z][a-zA-Z0-9_]*$")


def is_name_valid(name: str) -> bool:
    return NAME_REGEX.match(name) is not None


def line_reader(fp: TextIO) -> Generator[tuple[int, str], None, None]:
    for line_no, line in enumerate(fp, 1):
        line, _, comment = line.partition("#")
        line = line.strip()
        if line:
            yield line_no, line


def read_resources_txt(filename: str) -> Resources:
    errors = list()

    with open(filename, "r") as fp:
        resources = list()

        for line_no, line in line_reader(fp):
            s = line.split()
            if len(s) == 2:
                r_name, r_filename = s
                if is_name_valid(r_name):
                    resources.append(ResourceItem(r_name, r_filename))
                else:
                    errors.append(f"line {line_no}: Invalid name")
            else:
                errors.append(
                    f"line {line_no}: Invalid format, expected 2 words (name, filename)"
                )

    if not errors:
        return Resources(
            resources=resources,
        )
    else:
        raise RuntimeError(f"Error reading {filename}", errors)
