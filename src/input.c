// input.c

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

#include "registers.h"

/**
 * The buttons that are held down.
 *
 * CAUTION: MUST NOT be modified outside `input.c`
 */
uint16_t joypad_current;

/**
 * The buttons that were newly pressed.
 */
uint16_t joypad_pressed;

/**
 * Read the joypads and detect newly pressed buttons.
 *
 * SHOULD BE called at the end of the VBlank routine.
 */
void read_inputs__vblank() {
    while (MMIO_HVBJOY & HVBJOY_AUTO_READ) {
    }

    uint16_t joy;

    if ((MMIO_JOY1L & JOYPAD_L_TYPE_MASK) == 0) {
        joy = MMIO_JOY1;
    } else {
        joy = 0;
    }

    joypad_pressed = (joypad_current ^ 0xffff) & joy;
    joypad_current = joy;
}
