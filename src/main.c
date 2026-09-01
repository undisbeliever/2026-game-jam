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

#include "ppu.h"
#include "registers.h"
#include "resources.h"

#define VRAM_BG1_MAP 0x0000
#define VRAM_BG1_TILES 0x1000

int main(void) {
    reset_registers();

    PPU_BGMODE = BGMODE_M1_BG3P;

    PPU_BG1SC = BGnSC_(VRAM_BG1_MAP, BGnSC_MAP_32X32);
    PPU_BG12NBA = BGnnNBA_(VRAM_BG1_TILES, 0);

    PPU_TM = T_BG1;

    dma_resource_to_cgram(RES_TitleScreen_palette, 0);
    dma_image_resource(RES_TitleScreenFg, VRAM_BG1_MAP, VRAM_BG1_TILES);

    enable_vblank_interrupts();

    while (1) {
        wait_for_vblank();

        PPU_INIDISP = 15;
    }

    return 0;
}
