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

#include "input.h"
#include "map.h"
#include "metasprites.h"
#include "player.h"
#include "ppu.h"
#include "registers.h"
#include "resources.h"

#define VRAM_BG1_TILES 0x1000
#define VRAM_SPRITE_TILES 0x6000

static void load_map(uint8_t mapResource) {
    reset_registers();

    PPU_BGMODE = BGMODE_M1_BG3P | BGMODE_BG1_16PX;

    PPU_OBJSEL = OBJSEL_(VRAM_SPRITE_TILES, 0, OBJSEL_SIZE_8_16);

    PPU_BG1SC = BGnSC_(VRAM_MAP_TILEMAP_WADDR, BGnSC_MAP_32X32);
    PPU_BG12NBA = BGnnNBA_(VRAM_BG1_TILES, 0);

    PPU_TM = T_BG1 | T_OBJ;

    dma_map_resource(mapResource);

    camera_x = MAP_ORIGIN;
    camera_y = MAP_ORIGIN;

    dma_resource_to_cgram(mapHeader.paletteResource, 0);
    dma_map_tiles_resource(mapHeader.tilesResource, VRAM_BG1_TILES);

    dma_resource_to_vram(RES_Metasprite_tiles, VRAM_SPRITE_TILES);
    dma_resource_to_cgram(RES_Metasprite_palettes, 128);

    draw_map__forceblank();
    start_metasprites();

    player_init();

    enable_vblank_interrupts();
}

int main(void) {
    uint8_t map = RES_Map02;
    load_map(map);

    while (1) {
        wait_for_vblank();
        dma_oambuffer__vblank();
        update_map__vblank();
        read_inputs__vblank();

        start_metasprites();

        if (joypad_pressed & JOYPAD_START) {
            // Redraw map
            PPU_INIDISP = INIDISP_FORCE_BLANK | INIDISP_BRIGHTNESS_MASK;
            draw_map__forceblank();
        } else {
            PPU_INIDISP = 15;
        }

        if (joypad_pressed & JOYPAD_SELECT) {
            if (map == RES_Map01) {
                map = RES_Map02;
                load_map(map);
            } else {
                map = RES_Map01;
                load_map(map);
            }
        }

        player_process();

        if (joypad_current & JOYPAD_L) {
            camera_x -= 2;
        }
        if (joypad_current & JOYPAD_R) {
            camera_x += 3;
        }
        process_map_scrolling();
        set_metasprite_camera(camera_x, camera_y);

        draw_player();

        finalize_metasprites();
    }

    return 0;
}
