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

#define FIXED_AXIS_TILES 32
#define FIXED_AXIS_PX 512

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
 * MUST be a multiple of `FIXED_AXIS_TILES`
 */
static uint16_t map_cursor;

#define SCROLL_BUFFER_SIZE 64

/**
 * When non-zero: `map_scrollBuffer` is loaded to `map_scrollBufferVramWaddr`
 * using VMAIN = `map_scrollBufferDirty`
 */
static uint8_t map_scrollBufferDirty;
/** A tilemap row to transfer to VRAM when the map is vertically scrolled */
static uint8_t map_scrollBuffer[SCROLL_BUFFER_SIZE];
/** The VRAM word address to upload `map_scrollBuffer` to */
static uint16_t map_scrollBufferVramWaddr;

/** Map background layer horizontal offset */
static union DoubleWriteShadow map_hOffset;

/** Map background layer vertical offset */
static union DoubleWriteShadow map_vOffset;

/** Maximum `camera_x` value */
static uint16_t map_maxCameraX;

/** Maximum `camera_y` value */
static uint16_t map_maxCameraY;

/**
 * The map X/Y coordinate (depending on orientation) of the top-left most visible tile.
 *
 * Used to determine if the camera crosses a tile boundary and a new
 * map row needs to be drawn to VRAM.
 *
 * MUST be a multiple of `TILE_PX`.
 */
static uint16_t map_prevMaskedCamera;

/**
 * Offset between the sub-tile `camera_x`/`camera_y` position and `map_vOffset`.
 * Increments by `TILE_PX` whenever a new tile row is drawn to VRAM.
 */
static uint16_t map_cameraScrollOffset;

/**
 * The unmasked VRAM word address of the upper tilemap seam row.
 *
 * CAUTION: Not masked (bounds) within the VRAM tilemap.
 * Must be masked when converting to a VRAM address.
 */
static uint16_t map_tilemapSeam;

static inline void draw_map__forceblank__vertical(void) {
    map_hOffset.value = camera_x;
    map_vOffset.value = (camera_y & 0xf) + (ROWS_ABOVE_SCREEN * MAP_TILE_PX - 1);

    map_scrollBufferDirty = 0;

    // -1 so first visible scanline is camera_y
    map_cameraScrollOffset = ROWS_ABOVE_SCREEN * MAP_TILE_PX - 1;
    map_prevMaskedCamera = camera_y & 0xfff0;

    map_cursor = ((camera_y & 0x1ff0) << 1) - ROWS_ABOVE_SCREEN * NAMETABLE_WIDTH;

    uint16_t mapPos = map_cursor;

    PPU_VMAIN = VMAIN_INCREMENT_1 | VMAIN_INCREMENT_H;
    PPU_VMADD = VRAM_MAP_TILEMAP_WADDR;
    map_tilemapSeam = 0;

    for (unsigned i = 0; i < ROWS_TO_RENDER * FIXED_AXIS_TILES; i++) {
        uint8_t t = mapData[mapPos++];
        PPU_VMDATAL = mapTiles.tileset_l[t];
        PPU_VMDATAH = mapTiles.tileset_h[t];
    }
}

static inline void draw_map__forceblank__horizontal(void) {
    map_hOffset.value = (camera_x & 0xf) | (ROWS_ABOVE_SCREEN * MAP_TILE_PX);
    map_vOffset.value = camera_y - 1;

    map_scrollBufferDirty = 0;

    map_cameraScrollOffset = ROWS_ABOVE_SCREEN * MAP_TILE_PX;
    map_prevMaskedCamera = camera_x & 0xfff0;

    map_cursor = ((camera_x & 0x1ff0) << 1) - ROWS_ABOVE_SCREEN * NAMETABLE_WIDTH;

    uint16_t mapPos = map_cursor;

    PPU_VMAIN = VMAIN_INCREMENT_32 | VMAIN_INCREMENT_H;
    map_tilemapSeam = 0;

    for (uint8_t col = 0; col < ROWS_TO_RENDER; col++) {
        PPU_VMADD = col;

        for (unsigned row = 0; row < FIXED_AXIS_TILES; row++) {
            uint8_t t = mapData[mapPos++];
            PPU_VMDATAL = mapTiles.tileset_l[t];
            PPU_VMDATAH = mapTiles.tileset_h[t];
        }
    }
}

/**
 * Draws the map to a VRAM tilemap.
 *
 * REQUIRES: force-blank
 * REQUIRES: map tiles and map data loaded into memory, camera position set.
 */
void draw_map__forceblank(void) {
    if (!mapHeader.orientation) {
        map_maxCameraX = MAP_ORIGIN + FIXED_AXIS_PX - SCREEN_WIDTH;
        map_maxCameraY = mapHeader.dynamicAxisLength * MAP_TILE_PX + (MAP_ORIGIN - SCREEN_HEIGHT);
    } else {
        map_maxCameraX = mapHeader.dynamicAxisLength * MAP_TILE_PX + (MAP_ORIGIN - SCREEN_WIDTH);
        map_maxCameraY = MAP_ORIGIN + FIXED_AXIS_PX - SCREEN_HEIGHT;
    }

    if (camera_x < MAP_ORIGIN) {
        camera_x = MAP_ORIGIN;
    } else if (camera_x >= map_maxCameraX) {
        camera_x = map_maxCameraX;
    }
    if (camera_y < MAP_ORIGIN) {
        camera_y = MAP_ORIGIN;
    } else if (camera_y >= map_maxCameraY) {
        camera_y = map_maxCameraY;
    }

    if (mapHeader.orientation) {
        draw_map__forceblank__horizontal();
    } else {
        draw_map__forceblank__vertical();
    }

    PPU_BG1HOFS = map_hOffset.bytes.l;
    PPU_BG1HOFS = map_hOffset.bytes.h;

    PPU_BG1VOFS = map_vOffset.bytes.l;
    PPU_BG1VOFS = map_vOffset.bytes.h;
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
}

/**
 * Scrolls the map to face the camera, redrawing a new tilemap row whenever
 * a vertical tile boundary is crossed.
 *
 * REQUIRES: vertically orientated map, camera within bounds.
 *
 * INPUT: `camera_x`/`camera_y` camera position.
 */
static inline void process_map_scrolling__vertical(void) {
    map_hOffset.value = camera_x;

    int16_t offset = camera_y - map_prevMaskedCamera;
    if (offset < 0) {
        map_prevMaskedCamera -= MAP_TILE_PX;
        map_cameraScrollOffset -= MAP_TILE_PX;

        map_tilemapSeam -= NAMETABLE_WIDTH;
        map_scrollBufferVramWaddr = map_tilemapSeam & 0x3e0;

        map_cursor -= FIXED_AXIS_TILES;
        populate_scroll_buffer(map_cursor);
        map_scrollBufferDirty = VMAIN_INCREMENT_1 | VMAIN_INCREMENT_H;
    } else if (offset >= MAP_TILE_PX) {
        map_prevMaskedCamera += MAP_TILE_PX;
        map_cameraScrollOffset += MAP_TILE_PX;

        map_tilemapSeam += NAMETABLE_WIDTH;
        map_scrollBufferVramWaddr = (map_tilemapSeam + (ROWS_TO_RENDER - 1) * NAMETABLE_WIDTH) & 0x3e0;

        map_cursor += FIXED_AXIS_TILES;
        populate_scroll_buffer(map_cursor + (ROWS_TO_RENDER - 1) * NAMETABLE_WIDTH);
        map_scrollBufferDirty = VMAIN_INCREMENT_1 | VMAIN_INCREMENT_H;
    }
    map_vOffset.value = (camera_y & 0xf) + map_cameraScrollOffset;
}

/**
 * Scrolls the map to face the camera, redrawing a new tilemap column whenever
 * a vertical tile boundary is crossed.
 *
 * REQUIRES: vertically orientated map, camera within bounds.
 *
 * INPUT: `camera_x`/`camera_y` camera position.
 */
static inline void process_map_scrolling__horizontal(void) {
    map_vOffset.value = camera_y - 1;

    int16_t offset = camera_x - map_prevMaskedCamera;
    if (offset < 0) {
        map_prevMaskedCamera -= MAP_TILE_PX;
        map_cameraScrollOffset -= MAP_TILE_PX;

        map_tilemapSeam--;
        map_scrollBufferVramWaddr = map_tilemapSeam & 0x1f;

        map_cursor -= FIXED_AXIS_TILES;
        populate_scroll_buffer(map_cursor);
        map_scrollBufferDirty = VMAIN_INCREMENT_32 | VMAIN_INCREMENT_H;
    } else if (offset >= MAP_TILE_PX) {
        map_prevMaskedCamera += MAP_TILE_PX;
        map_cameraScrollOffset += MAP_TILE_PX;

        map_tilemapSeam++;
        map_scrollBufferVramWaddr = (map_tilemapSeam + (ROWS_TO_RENDER - 1)) & 0x1f;

        map_cursor += FIXED_AXIS_TILES;
        populate_scroll_buffer(map_cursor + (ROWS_TO_RENDER - 1) * NAMETABLE_WIDTH);
        map_scrollBufferDirty = VMAIN_INCREMENT_32 | VMAIN_INCREMENT_H;
    }
    map_hOffset.value = (camera_x & 0xf) + map_cameraScrollOffset;
}

/**
 * Scrolls the map to face the camera, redrawing tilemap rows or columns as required.
 *
 * This function limits dynamic axis scrolling to ONE 16px tile per frame.
 * The camera SHOULD NOT scroll faster than 16px/frame.
 *
 * INPUT: `camera_x`/`camera_y` camera position.
 */
void process_map_scrolling(void) {
    if (camera_x < MAP_ORIGIN) {
        camera_x = MAP_ORIGIN;
    } else if (camera_x >= map_maxCameraX) {
        camera_x = map_maxCameraX;
    }

    if (camera_y < MAP_ORIGIN) {
        camera_y = MAP_ORIGIN;
    } else if (camera_y >= map_maxCameraY) {
        camera_y = map_maxCameraY;
    }

    if (mapHeader.orientation) {
        process_map_scrolling__horizontal();
    } else {
        process_map_scrolling__vertical();
    }
}

/**
 * Update the map's scroll position and DMA new tilemap row.
 *
 * REQUIRES: In VBlank
 */
void update_map__vblank(void) {
    if (map_scrollBufferDirty) {
        PPU_VMAIN = map_scrollBufferDirty;
        PPU_VMADD = map_scrollBufferVramWaddr;

        DMA_DMAP_BBAD_0 = DMAP_BBAD_(DMAP_TRANSFER_TWO, 0x2118); // VMDATA
        DMA_DAS0 = SCROLL_BUFFER_SIZE;
        DMA_A1T0 = (uint16_t)(NEAR_PTR void *)(&map_scrollBuffer);
        DMA_A1B0 = 0x7e;
        DMA_DMAEN = 1;

        map_scrollBufferDirty = 0;
    }

    PPU_BG1HOFS = map_hOffset.bytes.l;
    PPU_BG1HOFS = map_hOffset.bytes.h;

    PPU_BG1VOFS = map_vOffset.bytes.l;
    PPU_BG1VOFS = map_vOffset.bytes.h;
}
