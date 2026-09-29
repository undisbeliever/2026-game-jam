// Metasprite drawing functions

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

#include "metasprites.h"
#include "gen-enums.h"
#include "memory.h"
#include "registers.h"
#include "resources.h"

#define MS_BIAS_X 128
#define MS_BIAS_Y 64

#define MAX_MS_DATA_SIZE 8192

#define OAM_BUFFER_LO_SIZE 512
#define OAM_BUFFER_HI_SIZE 32

/** Current metasprite OAM position (within `oamBuffer`) */
ZEROPAGE uint16_t oamBufferPos;

/**
 * bit buffer for managing the OAM hi table.
 *
 * Set to `0x80` when the buffer is empty.
 *
 * Data is written 1 bit at a time using `ror` instructions.
 * If `ror` sets the carry flag, the buffer is full and must
 * be loaded into `oamHiTablePos`.
 */
ZEROPAGE uint8_t oamHiTableBitBuffer;

/** near-pointer to next oamHiTableBitBuffer` value is written to */
ZEROPAGE NEAR_PTR uint8_t *oamHiTablePos;

#if __mos__

/**
 * Metasprite ROM data:
 *
 * Data format (all byte offsets within :
 *  * u16 framesetTable[] - byte offset for the frame table list
 *  * u16 frameOffset[]   - byte offset
 *  * frameData - 4 bytes per sprite, ZERO terminated frame data
 *      * u8 xpos - 128 baised x position (`xPos + MS_BIAS_X`)
 *      * u8 yPosAndSize - `yyyy yyyS`
 *          * `y` = 64 biased y position (`yPos + MS_BIAS_Y`)
 *          * `S` = size bit (0 = small, 1 = large)
 *      * u16 charAttr - matches the character and attribute OAM bytes
 */
__attribute__((section("bank81"))) volatile const uint8_t METASPRITE_ROM_DATA[MAX_MS_DATA_SIZE] = {};

__attribute__((aligned(32))) WRAM7E uint8_t oamBuffer[544];

void draw_metasprite_biased(uint8_t frameset, uint8_t frame, uint16_t biased_x, uint16_t biased_y) __attribute__((leaf));
#endif

#ifdef __VBCC__
volatile const uint8_t METASPRITE_ROM_DATA[MAX_MS_DATA_SIZE] = {0};

#pragma pack(push, 32)
WRAM7E uint8_t oamBuffer[544];
#pragma pack(pop)

void draw_metasprite_biased(__reg("a") uint8_t frameset, __reg("r0") uint8_t frame, __reg("r1") uint16_t biased_x, __reg("r2") uint16_t biased_y);

#endif

#ifdef __JCC__
volatile const uint8_t METASPRITE_ROM_DATA[MAX_MS_DATA_SIZE] = {0};

uint8_t oamBuffer[544];

[[A16, XY16]] extern void draw_metasprite_biased(uint8_t frameset, uint8_t frame, uint16_t biased_x, uint16_t biased_y);
#endif

/**
 * Reset the OAM buffer positions.
 *
 * MUST be called once per frame BEFORE any metasprites are drawn.
 */
void start_metasprites(void) {
    oamBufferPos = 0;

#ifndef __JCC__
    oamHiTablePos = &oamBuffer[512];
#else
    // Fixes a compile error
    oamHiTablePos = (NEAR_PTR uint8_t *)&oamBuffer + 512;
#endif

    oamHiTableBitBuffer = 0x80;
}

const volatile uint8_t FixedOffscreenByte = -16;

/**
 * Transfer the OAM buffer to OAM.
 *
 * REQUIRES: `oamBufferPos <= 512`
 */
void dma_oambuffer__vblank(void) {
    PPU_OAMADD = 0;

    // Transfer `oamBuffer[..oamBufferPos]` to the OAM
    DMA_DMAP_BBAD_0 = DMAP_BBAD_(DMAP_TRANSFER_ONE, 0x2104); // OAMDATA
    DMA_DAS0 = oamBufferPos;
    DMA_A1T0 = (uint16_t)(NEAR_PTR void *)(&oamBuffer);
    DMA_A1B0 = 0x7e;
    DMA_DMAEN = 1;

    if (oamBufferPos < OAM_BUFFER_LO_SIZE) {
        // `oamBuffer` is not full.
        // Fill remaining OAM with `FixedOffscreenByte` to move all unused sprites offscreen.

        DMA_DAS1 = OAM_BUFFER_LO_SIZE - oamBufferPos;
#ifndef __JCC__
        DMA_DMAP_BBAD_1 = DMAP_BBAD_(DMAP_TRANSFER_ONE | DMAP_FIXED, 0x2104); // OAMDATA
#else
        DMA_DMAP_BBAD_1 = DMAP_TRANSFER_ONE | DMAP_FIXED | 0x0400;
#endif
        DMA_A1T1 = (uint16_t)(NEAR_PTR void *)(&FixedOffscreenByte);
        DMA_A1B1 = 0;
        DMA_DMAEN = 2;
    }

    // Transfer OAM Hi table buffer to OAM
#ifndef __JCC__
    DMA_A1T0 = (uint16_t)(NEAR_PTR void *)(&oamBuffer[OAM_BUFFER_LO_SIZE]);
#else
    // Fixes a compile error
    DMA_A1T0 = (uint16_t)(NEAR_PTR uint8_t *)&oamBuffer + 512;
#endif

    DMA_DAS0L = OAM_BUFFER_HI_SIZE;
    DMA_DMAEN = 1;
}

/**
 * Draw metasprite at the given screen coordinates.
 */
void draw_metasprite_screen(uint8_t frameset, uint8_t frame, int16_t x, int16_t y) {
    draw_metasprite_biased(frameset, frame, x - MS_BIAS_X, y - MS_BIAS_Y);
}
