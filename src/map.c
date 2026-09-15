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

#include "map.h"
#include "memory.h"
#include "ppu.h"
#include "registers.h"
#include <stdbool.h>
#include <stdint.h>

#define MAP_TILE_PX 16

#define MAP_WIDTH 32
#define ROWS_ABOVE_SCREEN 3
#define ROWS_TO_RENDER 21

struct MapTilesHeader mapTiles;
struct MapDataHeader mapHeader;

// ::TODO move to wram7e::
// ::TODO make static::
uint8_t mapData[MAX_MAP_DATA_SIZE];

// ::TODO move to camera subsystem::
uint16_t camera_x;
uint16_t camera_y;

/**
 * Current top-left position of the map data that is loaded into the VRAM.
 *
 * MAY be out of bounds and/or negative, the map subsystem will always draw
 * 3 or 4 tilemap rows above and below the screen.
 *
 * MUST be a multiple of `MAP_WIDTH`
 */
static uint16_t map_cursor;

#define SCROLL_BUFFER_SIZE 64

static bool map_scrollBufferDirty;
/** A tilemap row to transfer to VRAM when the map is vertically scrolled */
static uint8_t map_scrollBuffer[SCROLL_BUFFER_SIZE];
/** The VRAM word address to upload `map_scrollBuffer` to */
static uint16_t map_scrollBufferVramWaddr;

/** Map background layer horizontal offset */
static union DoubleWriteShadow map_hOffset;

/** Map background layer vertical offset */
static union DoubleWriteShadow map_vOffset;

/** Maximum `camera_x` value */
#define CAMERA_MAX_X 0x4100
/** Maximum `camera_y` value */
static uint16_t map_MaxCameraY;

/**
 * The map Y coordinate of the topmost visible tile.
 *
 * Used to determine if the camera crosses a tile boundary and a new
 * map row needs to be drawn to VRAM.
 *
 * MUST be a multiple of `TILE_PX`.
 */
static uint16_t map_prevMaskedCameraY;

/**
 * Offset between the sub-tile `camera_y` position and `map_vOffset`.
 * Increments by `TILE_PX` whenever a new tile row is drawn to VRAM.
 */
static uint16_t map_cameraYScrollOffset;

/**
 * The VRAM word address of the upper tilemap seam row.
 */
static uint16_t map_cameraVramWaddr;

/**
 * Draws the map to a VRAM tilemap.
 *
 * REQUIRES: force-blank
 * REQUIRES: map tiles and map data loaded into memory, camera position set.
 */
void draw_map__forceblank(void) {
    map_MaxCameraY = mapHeader.height * MAP_TILE_PX + (MAP_ORIGIN - SCREEN_HEIGHT);

    if (camera_x < MAP_ORIGIN) {
        camera_x = MAP_ORIGIN;
    } else if (camera_x >= CAMERA_MAX_X + 1) {
        camera_x = CAMERA_MAX_X;
    }
    map_hOffset.value = camera_x;
    PPU_BG1HOFS = map_hOffset.bytes.l;
    PPU_BG1HOFS = map_hOffset.bytes.h;

    if (camera_y < MAP_ORIGIN) {
        camera_y = MAP_ORIGIN;
    } else if (camera_y >= map_MaxCameraY) {
        camera_y = map_MaxCameraY;
    }
    map_vOffset.value = (camera_y & 0xf) | (ROWS_ABOVE_SCREEN * MAP_TILE_PX);
    PPU_BG1VOFS = map_vOffset.bytes.l;
    PPU_BG1VOFS = 0;

    map_scrollBufferDirty = false;

    // -1 so first visible scanline is camera_y
    map_cameraYScrollOffset = ROWS_ABOVE_SCREEN * MAP_TILE_PX - 1;
    map_prevMaskedCameraY = camera_y & 0xfff0;

    map_cursor = ((camera_y & 0x1ff0) << 1) - ROWS_ABOVE_SCREEN * NAMETABLE_WIDTH;

    uint16_t mapPos = map_cursor;

    PPU_VMAIN = VMAIN_INCREMENT_1 | VMAIN_INCREMENT_H;
    PPU_VMADD = VRAM_MAP_TILEMAP_WADDR;
    map_cameraVramWaddr = 0;

    for (unsigned i = 0; i < ROWS_TO_RENDER * MAP_WIDTH; i++) {
        uint8_t t = mapData[mapPos++];
        PPU_VMDATAL = mapTiles.tileset_l[t];
        PPU_VMDATAH = mapTiles.tileset_h[t];
    }
}

/**
 * Draws a single tilemap row to the `map_scrollBuffer`.
 */
static void populate_scroll_buffer(uint16_t cursor) {
    uint8_t bi = 0;
    do {
        uint8_t t = mapData[cursor++];
        map_scrollBuffer[bi++] = mapTiles.tileset_l[t];
        map_scrollBuffer[bi++] = mapTiles.tileset_h[t];
    } while (bi < SCROLL_BUFFER_SIZE);

    map_scrollBufferDirty = true;
}

/**
 * Scrolls the map to face the camera, redrawing a new tilemap row whenever
 * a vertical tile boundary is crossed.
 *
 * This function limits vertical scrolling to ONE 16px tile per frame.
 * The camera SHOULD NOT scroll faster than 16px/frame.
 *
 * INPUT: `camera_x`/`camera_y` camera position.
 */
void process_map_scrolling(void) {
    if (camera_x < MAP_ORIGIN) {
        camera_x = MAP_ORIGIN;
    } else if (camera_x >= CAMERA_MAX_X + 1) {
        camera_x = CAMERA_MAX_X;
    }
    map_hOffset.value = camera_x;

    if (camera_y < MAP_ORIGIN) {
        camera_y = MAP_ORIGIN;
    } else if (camera_y >= map_MaxCameraY) {
        camera_y = map_MaxCameraY;
    }

    int16_t offset = camera_y - map_prevMaskedCameraY;
    if (offset < 0) {
        map_prevMaskedCameraY -= MAP_TILE_PX;
        map_cameraYScrollOffset -= MAP_TILE_PX;

        map_cameraVramWaddr -= NAMETABLE_WIDTH;
        map_scrollBufferVramWaddr = map_cameraVramWaddr & 0x3e0;

        map_cursor -= MAP_WIDTH;
        populate_scroll_buffer(map_cursor);
    } else if (offset >= MAP_TILE_PX) {
        map_prevMaskedCameraY += MAP_TILE_PX;
        map_cameraYScrollOffset += MAP_TILE_PX;

        map_cameraVramWaddr += NAMETABLE_WIDTH;
        map_scrollBufferVramWaddr = (map_cameraVramWaddr + (ROWS_TO_RENDER - 1) * NAMETABLE_WIDTH) & 0x3e0;

        map_cursor += MAP_WIDTH;
        populate_scroll_buffer(map_cursor + (ROWS_TO_RENDER - 1) * NAMETABLE_WIDTH);
    }
    map_vOffset.value = (camera_y & 0xf) + map_cameraYScrollOffset;
}

/**
 * Update the map's scroll position and DMA new tilemap row.
 *
 * REQUIRES: In VBlank
 */
void update_map__vblank(void) {
    if (map_scrollBufferDirty) {
        PPU_VMAIN = VMAIN_INCREMENT_1 | VMAIN_INCREMENT_H;
        PPU_VMADD = map_scrollBufferVramWaddr;

        DMA_DMAP_BBAD_0 = DMAP_BBAD_(DMAP_TRANSFER_TWO, 0x2118); // VMDATA
        DMA_DAS0 = SCROLL_BUFFER_SIZE;
        DMA_A1T0 = (uint16_t)(NEAR_PTR void *)(&map_scrollBuffer);
        DMA_A1B0 = 0x7e;
        DMA_DMAEN = 1;

        map_scrollBufferDirty = false;
    }

    PPU_BG1HOFS = map_hOffset.bytes.l;
    PPU_BG1HOFS = map_hOffset.bytes.h;

    PPU_BG1VOFS = map_vOffset.bytes.l;
    PPU_BG1VOFS = map_vOffset.bytes.h;
}
