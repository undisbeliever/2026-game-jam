// Interrupts and VBlank management code

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

/** Incremented every VBlank interrupt */
volatile uint8_t nmi_counter;

/** Number of lag frames recorded in the game */
uint16_t n_lag_frames;

/** The current frame number, increments every `wait_for_vblank()` call */
uint32_t frame_counter;

#ifdef __JCC__
// NMI (Non-Maskable Interrupt)
[[interrupt_NMI, no_ISR1]] void NmiISR(void) { nmi_counter++; }

// ::TODO inline assembly::
inline static void wait_for_interrupt(void) {}
#endif

#ifdef __VBCC__
// NMI ISR defined in assembly

inline static void wait_for_interrupt(void) { __asm(" wai"); }
#endif

#ifdef __mos__
// NMI ISR defined in assembly

inline static void wait_for_interrupt(void) { __asm("wai"); }
#endif

/**
 * Enable VBlank interrupts
 */
void enable_vblank_interrupts(void) {
  MMIO_RDNMI;
  MMIO_NMITIMEN = NMITIMEN_VBLANK;
  nmi_counter = 0;
}

/**
 * Wait until the start of VBlank
 */
void wait_for_vblank(void) {
  frame_counter++;

  // ::TODO replace with saturation add using assembly::
  if (nmi_counter >= 2) {
    n_lag_frames += (nmi_counter - 1);
  }

  nmi_counter = 0;

  do {
    wait_for_interrupt();
  } while (nmi_counter == 0);
}

/**
 * Reset the PPU registers and disable interrupts and HDMA.
 */
void reset_registers(void) {
  // https://snes.nesdev.org/wiki/Init_code#Reset_Registers_Routine

  // disable interrupts and HDMA before clearing registers
  MMIO_NMITIMEN = 0;
  DMA_DMAEN = 0;

  // enable Force Blank, full brightness
  PPU_INIDISP = INIDISP_FORCE_BLANK | INIDISP_BRIGHTNESS_MASK;

  // CPU registers
  MMIO_MEMSEL = 0; // set to 1 if FastROM
  MMIO_WRIO = 0xFF;

  // Objects
  PPU_OBJSEL = 0;
  PPU_OAMADD = 0; // disable OAM priority rotation

  // Backgrounds
  PPU_BGMODE = 0;
  PPU_MOSAIC = 0;

  PPU_BG1SC = 0;
  PPU_BG2SC = 0;
  PPU_BG3SC = 0;
  PPU_BG4SC = 0;

  PPU_BG12NBA = 0;
  PPU_BG34NBA = 0;

  // Scroll Registers
  PPU_BG1HOFS = 0;
  PPU_BG1HOFS = 0; // set horizontal offset to 0

  PPU_BG1VOFS = 0xFF;
  PPU_BG1VOFS = 0xFF; // set vertical offset to -1

  PPU_BG2HOFS = 0;
  PPU_BG2HOFS = 0;

  PPU_BG2VOFS = 0xFF;
  PPU_BG2VOFS = 0xFF;

  PPU_BG3HOFS = 0;
  PPU_BG3HOFS = 0;

  PPU_BG3VOFS = 0xFF;
  PPU_BG3VOFS = 0xFF;

  PPU_BG4HOFS = 0;
  PPU_BG4HOFS = 0;

  PPU_BG4VOFS = 0xFF;
  PPU_BG4VOFS = 0xFF;

  // VRAM Registers
  PPU_VMAIN = VMAIN_INCREMENT_H | VMAIN_INCREMENT_1;

  // Mode 7
  PPU_M7SEL = 0; // no flipping or screen repeat
  // Not setting matrix

  // Windows
  PPU_W12SEL = 0;
  PPU_W34SEL = 0;
  PPU_WOBJSEL = 0;
  PPU_WH0 = 0;
  PPU_WH1 = 0;
  PPU_WH2 = 0;
  PPU_WH3 = 0;
  PPU_WBGLOG = 0;
  PPU_WOBJLOG = 0;

  // Layer Enable
  PPU_TM = 0;
  PPU_TS = 0;
  PPU_TMW = 0;
  PPU_TSW = 0;

  // Color Math
  PPU_CGWSEL =
      CGWSEL_MAINSCREEN_BLACK_NOWHERE | CGWSEL_SUBSCREEN_TRANSPARENT_ALWAYS;
  PPU_CGADSUB = 0;
  PPU_COLDATA = 0xE0; // set Fixed color data to black

  // Misc
  PPU_SETINI = 0;
}
