// Map subsystem

// SPDX-FileCopyrightText: © 2026 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2026 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//         claim that you wrote the original software. If you use this software
//         in a product, an acknowledgment in the product documentation would be
//         appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//         be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source
//         distribution.
//

#ifndef H__MAP_H_
#define H__MAP_H_

#include <stdint.h>

/** VRAM word address of the map tilemap */
#define VRAM_MAP_TILEMAP_WADDR 0x0000

/** The top-left position of the map */
#define MAP_ORIGIN 0x4000

/**
 * Map Tile resource header.
 *
 * After the header is:
 *  * tile data
 */
struct MapTilesHeader {
    uint8_t tileset_l[256];
    uint8_t tileset_h[256];
};
#define MAP_TILES_HEADER_SIZE 512

extern struct MapTilesHeader mapTiles;

struct MapDataHeader {
    uint8_t height;
};
#define MAP_DATA_HEADER_SIZE 0

// ::TODO increase to 8192 and make private::
#define MAX_MAP_DATA_SIZE 4096

extern struct MapDataHeader mapHeader;

extern uint16_t camera_x;
extern uint16_t camera_y;

void draw_map__forceblank(void);
void process_map_scrolling(void);
void update_map__vblank(void);

#endif
