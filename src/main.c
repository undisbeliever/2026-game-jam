// main.c

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
#include "ppu.h"
#include "registers.h"
#include "resources.h"

#define VRAM_BG1_TILES 0x1000

// ::TODO remove::
extern uint8_t mapData[MAX_MAP_DATA_SIZE];

int main(void) {
    reset_registers();

    PPU_BGMODE = BGMODE_M1_BG3P | BGMODE_BG1_16PX;

    PPU_BG1SC = BGnSC_(VRAM_MAP_TILEMAP_WADDR, BGnSC_MAP_32X32);
    PPU_BG12NBA = BGnnNBA_(VRAM_BG1_TILES, 0);

    PPU_TM = T_BG1;

    dma_resource_to_cgram(RES_Tower_palette, 0);
    dma_map_tiles_resource(RES_Tower_tiles, VRAM_BG1_TILES);

    camera_x = MAP_ORIGIN;
    camera_y = MAP_ORIGIN;

    // ::TODO load map data from a resource::
    mapHeader.height = 128;
    for (unsigned i = 0; i < MAX_MAP_DATA_SIZE; i++) {
        mapData[i] = 0;
    }
    for (unsigned y = 0; y < 128; y++) {
        uint16_t x = (y / 2) & 31;
        mapData[y * 32 + x] = 0x13;
        mapData[y * 32 + 31 - x] = 0x13;
        mapData[y * 32 + 31] = mapData[y * 32] = 0x04 | ((y & 1) * 16);
    }
    draw_map__forceblank();

    enable_vblank_interrupts();

    while (1) {
        wait_for_vblank();
        update_map__vblank();

        while (MMIO_HVBJOY & HVBJOY_AUTO_READ) {
        }

        if (MMIO_JOY1H & JOYPAD_H_START) {
            // Redraw map
            PPU_INIDISP = INIDISP_FORCE_BLANK | INIDISP_BRIGHTNESS_MASK;
            draw_map__forceblank();
        } else {
            PPU_INIDISP = 15;
        }

        uint16_t camera_speed = 3;
        if (MMIO_JOY1H & JOYPAD_H_Y) {
            camera_speed = 300;
        }

        if (MMIO_JOY1H & JOYPAD_H_UP) {
            camera_y -= camera_speed;
        }
        if (MMIO_JOY1H & JOYPAD_H_DOWN) {
            camera_y += camera_speed;
        }
        if (MMIO_JOY1H & JOYPAD_H_LEFT) {
            camera_x -= camera_speed;
        }
        if (MMIO_JOY1H & JOYPAD_H_RIGHT) {
            camera_x += camera_speed;
        }
        process_map_scrolling();
    }

    return 0;
}
