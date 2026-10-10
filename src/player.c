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

#include "player.h"
#include "input.h"
#include "map.h"
#include "metasprites.h"
#include <stdbool.h>

// clang-format off

// SoA for player movement values
// Order: NTSC walk, NTSC run
// 8.8 signed fixed point
// ::TODO add PAL constants
// ::TODO adjust::
static const int16_t PLAYER_ACCELERATION[2]     = { 0x0059,  0x0066};
static const int16_t PLAYER_DECELERATION[2]     = { 0x0066,  0x0022};
static const int16_t PLAYER_MAX_XVEL_RIGHT[2]   = {+0x0140, +0x0220};
static const int16_t PLAYER_MAX_XVEL_LEFT[2]    = {-0x0140, -0x0220};
static const int16_t PLAYER_JUMP_VEL[2]         = {-0x0380, -0x0380};

// SoA for player movement values
// Order: NTSC b released, NTSC b pressed
// 8.8 signed fixed point
// ::TODO add PAL constants::
// ::TODO adjust::
static const int16_t PLAYER_GRAVITY[2]          = { 0x0040,  0x001e};
static const int16_t PLAYER_MAX_YVEL[2]         = { 0x0500,  0x0500};
// clang-format on

enum PlayerSprites {
    PSF_Stand,
    PSF_Walk1,
    PSF_Walk2,
    PSF_Walk3,
    PSF_Walk4,
    PSF_Jump,
    PSF_Fall,

    PSF_StandLeft,
};

static uint8_t player_movementIndex;
static volatile uint8_t player_facingDirection;
static uint8_t player_spriteFrame;

struct EntityMovement player_movement;
uint8_t player_tileCollision;

void player_init(void) {
    // ::TODO check for pal::
    player_movementIndex = 0;

#ifndef __JCC__
    player_movement = (struct EntityMovement){0};
#else
    // ::BUGFIX Prevents jcc816 compiler crash::
    player_movement.xPos.fixedPoint = 0;
    player_movement.yPos.fixedPoint = 0;
    player_movement.xVelocity = 0;
    player_movement.yVelocity = 0;
#endif

    player_movement.xPos.parts.pixel = MAP_ORIGIN + 64;
    player_movement.yPos.parts.pixel = MAP_ORIGIN + 160;

    player_facingDirection = PSF_Stand;
    player_spriteFrame = PSF_Stand;

    player_tileCollision = 0;
}

void player_process(void) {
    uint8_t player_walkFrame;

    const uint8_t mi = (joypad_current & JOYPAD_Y) ? player_movementIndex + 1 : player_movementIndex;

    {
        int16_t xv = player_movement.xVelocity;

        if (joypad_current & JOYPAD_RIGHT) {
            if (xv >= 0) {
                xv += PLAYER_ACCELERATION[mi];
                if (xv >= PLAYER_MAX_XVEL_RIGHT[mi]) {
                    xv = PLAYER_MAX_XVEL_RIGHT[mi];
                }
                player_walkFrame = ((player_movement.xPos.parts.pixel >> 3) & 3) + PSF_Walk1;
            } else {
                xv += PLAYER_DECELERATION[mi];
                player_walkFrame = PSF_Walk3;
            }

            player_facingDirection = PSF_Stand;

        } else if (joypad_current & JOYPAD_LEFT) {
            if (xv < 0) {
                xv -= PLAYER_ACCELERATION[mi];
                if (xv < PLAYER_MAX_XVEL_LEFT[mi]) {
                    xv = PLAYER_MAX_XVEL_LEFT[mi];
                }
                player_walkFrame = ((player_movement.xPos.parts.pixel >> 3) & 3) + PSF_Walk1;
            } else {
                xv -= PLAYER_DECELERATION[mi];
                player_walkFrame = PSF_Walk3;
            }

            player_facingDirection = PSF_StandLeft;

        } else {
            if (xv > 0) {
                xv -= PLAYER_DECELERATION[mi];
                if (xv < 0) {
                    xv = 0;
                }
            } else {
                xv += PLAYER_DECELERATION[mi];
                if (xv >= 0) {
                    xv = 0;
                }
            }

            player_walkFrame = PSF_Stand;
        }

        player_movement.xVelocity = xv;
    }

    {
        int16_t yv = player_movement.yVelocity;

        if (player_tileCollision) {
            // ::TODO coyote time::
            if (joypad_pressed & JOYPAD_B) {
                yv = PLAYER_JUMP_VEL[mi];
            } else {
                yv = 0;
            }

            player_spriteFrame = player_walkFrame + player_facingDirection;
        } else {
            const uint8_t gi = (joypad_current & JOYPAD_B) ? player_movementIndex + 1 : player_movementIndex;

            yv += PLAYER_GRAVITY[gi];

            if (yv < 0) {
                player_spriteFrame = PSF_Jump + player_facingDirection;
            } else {
                if (yv >= PLAYER_MAX_YVEL[gi]) {
                    yv = PLAYER_MAX_YVEL[gi];
                }

                player_spriteFrame = PSF_Fall + player_facingDirection;
            }
        }

        player_movement.yVelocity = yv;
    }

    player_movement.xPos.fixedPoint += player_movement.xVelocity;
    player_movement.yPos.fixedPoint += player_movement.yVelocity;

    // Dummy player tile collisions
    if (player_movement.xPos.parts.pixel < MAP_ORIGIN) {
        player_movement.xPos.parts.pixel = MAP_ORIGIN;
    } else if (player_movement.xPos.parts.pixel >= map_maxXPos) {
        player_movement.xPos.parts.pixel = map_maxXPos;
    }

    if (player_movement.yPos.parts.pixel < MAP_ORIGIN) {
        player_movement.yPos.parts.pixel = MAP_ORIGIN;
    } else if (player_movement.yPos.parts.pixel >= map_maxYPos) {
        player_movement.yPos.parts.pixel = map_maxYPos;
    }

    if (player_movement.yPos.parts.pixel >= MAP_ORIGIN + 160) {
        player_movement.yPos.parts.pixel = MAP_ORIGIN + 160;
        player_tileCollision = true;
    } else {
        player_tileCollision = false;
    }
}

// ::BUGFIX must add __noinline on vbcc to draw the player at correct position & frame. ::
// ::: Without it, the player is always standing right at a fixed camera position::
#ifdef __VBCC__
__noinline
#endif
    void
    draw_player(void) {
    draw_metasprite_camera(0, player_spriteFrame,
                           player_movement.xPos.parts.pixel,
                           player_movement.yPos.parts.pixel);
}
