// Resources loading code

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

#include "gen-enums.h"
#include "memory.h"
#include "registers.h"

struct ResourceTableEntry {
    uint16_t size;
    uint16_t addr;
    uint8_t bank;
};

#ifdef __JCC__
const struct ResourceTableEntry RESOURCES_TABLE[N_RESOURCES] = {0};

inline static void populate_dma_regs(uint8_t id) {
    DMA_DAS0 = RESOURCES_TABLE[id].size;
    DMA_A1T0 = RESOURCES_TABLE[id].addr;
    DMA_A1B0 = RESOURCES_TABLE[id].bank;
}
#endif

#ifdef __VBCC__
__far const struct ResourceTableEntry RESOURCES_TABLE[N_RESOURCES] = {0};

/**
 * Load the DMA registers with the resource address and size.
 *
 * Written in assembly to prevent vbcc from optimising away the
 * RESOURCES_TABLE reads and filling the DMA registers with 0.
 */
__regsused("a/x/r0/r1") char *populate_dma_regs(__reg("a") uint8_t) =
    " inline"
    ""
    "\n  and #$00ff"
    "\n  sta r0"
    "\n  asl"
    "\n  asl"
    // Carry clear
    "\n  adc r0"
    "\n  tax"
    "\n  lda >_RESOURCES_TABLE + 2,x"
    "\n  sta $804302" // A1T0L
    "\n  lda >_RESOURCES_TABLE + 3,x"
    "\n  sta $804303" // A1T1H + A1B0
    "\n  lda >_RESOURCES_TABLE + 0,x"
    "\n  sta $804305"; // DAS0
#endif

#ifdef __mos__

__attribute__((section("bank81"))) volatile const struct ResourceTableEntry RESOURCES_TABLE[N_RESOURCES] = {0};

/**
 * Load the DMA registers with the resource address and size.
 *
 * Written in assembly to prevent llvm-mos from optimising away the
 * RESOURCES_TABLE indexing and filling the DMA registers with RESOURCES_TABLE[0].
 * (ie, Always loading resource 0.)
 */
inline static void populate_dma_regs(uint8_t id) {
    // ::HACK prevent llvm-mos from dropping RESOURCES_TABLE::
    // ::TODO figure out why I get an "undefined symbol: RESOURCES_TABLE" error when I remove this line::
    DMA_A1B0 = RESOURCES_TABLE[0].bank;

    // Cannot mark "a" as clobbered in `__asm__`.
    // Have to mark it as a dummy output instead.
    // `volatile` in `__asm__` ensure the ASM is not dropped.
    uint8_t dummy_a;

    // clang-format off
    __attribute__((leaf)) __asm__ volatile(
        "\n  rep #$30"
    // i16
    // a16
        "\n  and #mos16($ff)"
        "\n  sta __rc0"
        "\n  asl"
        "\n  asl"
        // Carry clear
        "\n  adc __rc0"
        "\n  tax"
        "\n  lda >RESOURCES_TABLE + 2,x"
        "\n  sta $804302" // A1T0L
        "\n  lda >RESOURCES_TABLE + 3,x"
        "\n  sta $804303" // A1T1H + A1B0
        "\n  lda >RESOURCES_TABLE + 0,x"
        "\n  sta $804305" // DAS0
        ""
        "\n  sep #$30"
    // a8
    // i8
        // Output:
        : "=a"(dummy_a)
        // Input:
        : "a"(id)
         // Clobbers
        : "p", "x", "rs0"
    );
    // clang-format on
}
#endif

/**
 * DMA a resource to memory.
 *
 * REQUIRES: Force-Blank, HDMA disabled
 *
 * @param dmapBbad The values to use for the DMAP and BBAD DMA registers
 */
static void dma_resource(uint8_t id, uint16_t dmapBbad) {
    DMA_DMAP_BBAD_0 = dmapBbad;

    populate_dma_regs(id);

    // Ensure we are in force-blank and HDMA is disabled
    DMA_HDMAEN = 0;
    PPU_INIDISP = INIDISP_FORCE_BLANK | INIDISP_BRIGHTNESS_MASK;

    DMA_DMAEN = 1;
}

/**
 * DMA a resource header to Work-RAM and prep a DMA to transfer the remaining data.
 *
 * After this function is called, the DMA registers will point to the data after
 * the resource header.
 *
 * REQUIRES: Force-Blank, HDMA disabled
 *
 * @param id the resource id
 * @param ptr Work-RAM header pointer (MUST be in bank $7e)
 * @param size header size (SHOULD be smaller than resource data)
 */
static void dma_resource_header(uint8_t id, volatile NEAR_PTR void *ptr, uint16_t size) {
    MMIO_WMADDML = (uint16_t)ptr;
    MMIO_WMADDH = 0;

    populate_dma_regs(id);
    DMA_DMAP_BBAD_0 = DMAP_BBAD_(DMAP_TRANSFER_ONE, 0x2180); // WMDATA

    uint16_t sizeAfterHeader = DMA_DAS0 - size;
    if (sizeAfterHeader >= DMA_DAS0) {
        sizeAfterHeader = 1;
    }
    DMA_DAS0 = size;
    DMA_DMAEN = 1;

    DMA_DAS0 = sizeAfterHeader;
}

/**
 * DMA a resource to CGRAM.
 *
 * REQUIRES: Force-Blank, HDMA disabled
 */
void dma_resource_to_cgram(uint8_t id, uint8_t cgramIndex) {
    PPU_CGADD = cgramIndex;

    dma_resource(id, DMAP_BBAD_(DMAP_TRANSFER_ONE, 0x2122));
}

/**
 * DMA a resource to VRAM.
 *
 * REQUIRES: Force-Blank, HDMA disabled
 */
void dma_resource_to_vram(uint8_t id, uint16_t vramWordAddr) {
    PPU_VMAIN = VMAIN_INCREMENT_1 | VMAIN_INCREMENT_H;

    PPU_VMADD = vramWordAddr;

    dma_resource(id, DMAP_BBAD_(DMAP_TRANSFER_TWO, 0x2118));
}

/**
 * Image resource header.
 *
 * After the header is:
 *  * tilemap data (tilemapSize * 256 bytes)
 *  * tile data
 */
struct ImageHeader {
    uint8_t tilemapSize;
};
#define IMAGE_HEADER_SIZE 1

/**
 * DMA an image resource to VRAM.
 *
 * REQUIRES: Force-Blank, HDMA disabled
 *
 * @param id resource id
 * @param mapWordAddr VRAM tilemap word address
 * @param tilesWordAddr VRAM tile word address
 */
void dma_image_resource(uint8_t id, uint16_t mapWordAddr, uint16_t tilesWordAddr) {
    struct ImageHeader header;
    dma_resource_header(id, &header, IMAGE_HEADER_SIZE);

    DMA_DMAP_BBAD_0 = DMAP_BBAD_(DMAP_TRANSFER_TWO, 0x2118); // VMDATA

    PPU_VMAIN = VMAIN_INCREMENT_1 | VMAIN_INCREMENT_H;
    PPU_VMADD = mapWordAddr;

    // DAS0 = resource size after header
    const uint16_t tileSize = DMA_DAS0 - (header.tilemapSize * 256);

    // DMA map data
    DMA_DAS0 = header.tilemapSize * 256;
    DMA_DMAEN = 1;

    // DMA tile data
    PPU_VMADD = tilesWordAddr;
    DMA_DAS0 = tileSize;
    DMA_DMAEN = 1;
}
