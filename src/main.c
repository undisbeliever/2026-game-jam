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

void reset_registers(void);

#define SNES_COLOR(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

#define N_COLORS 7
const uint16_t COLOR_DATA[N_COLORS] = {
    SNES_COLOR(15, 00, 00), // red
    SNES_COLOR(00, 15, 00), // green
    SNES_COLOR(00, 00, 15), // blue
    SNES_COLOR(31, 15, 20), // pink
    SNES_COLOR(20, 31, 20), // light green
    SNES_COLOR(22, 27, 30), // light blue
    SNES_COLOR(31, 27, 00), // gold
};

int main(void) {
    reset_registers();

    enable_vblank_interrupts();

    uint8_t color = 0;

    while (1) {
        if (color >= N_COLORS) {
            color = 0;
        }
        wait_for_vblank();

        PPU_CGADD = 0;
        PPU_CGDATA = COLOR_DATA[color];
        PPU_CGDATA = COLOR_DATA[color] >> 8;

        // Enable the screen at full brightness
        PPU_INIDISP = 15;

        for (int i = 0; i < 59; i++) {
            wait_for_vblank();
        }

        color++;
    }

    return 0;
}
