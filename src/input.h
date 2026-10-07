// Joypad input
//
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

#ifndef H__INPUT_H_
#define H__INPUT_H_

#include <stdint.h>

#define JOYPAD_B 0b1000000000000000
#define JOYPAD_Y 0b0100000000000000
#define JOYPAD_SELECT 0b0010000000000000
#define JOYPAD_START 0b0001000000000000
#define JOYPAD_UP 0b0000100000000000
#define JOYPAD_DOWN 0b0000010000000000
#define JOYPAD_LEFT 0b0000001000000000
#define JOYPAD_RIGHT 0b0000000100000000
#define JOYPAD_A 0b0000000010000000
#define JOYPAD_X 0b0000000001000000
#define JOYPAD_L 0b0000000000100000
#define JOYPAD_R 0b0000000000010000

#define JOYPAD_BUTTONS 0b1100000011000000
#define JOYPAD_DPAD 0b0000111100000000
#define JOYPAD_SHOULDER 0b0000000000110000

extern uint16_t joypad_current;
extern uint16_t joypad_pressed;

void read_inputs__vblank();

#endif
