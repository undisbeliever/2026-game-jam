// SNES PPU and MMIO register definitions
//
// Excludes the S-CPU multiplication and division registers
// as they are used by the vbcc65816 and jcc816 C compilers.

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

#ifndef H__REGISTERS_H__
#define H__REGISTERS_H__

#include <inttypes.h>

#define PPU_INIDISP (*(volatile uint8_t *)0x2100)
#define PPU_OBJSEL (*(volatile uint8_t *)0x2101)
#define PPU_OAMADD (*(volatile uint16_t *)0x2102)
#define PPU_OAMADDL (*(volatile uint8_t *)0x2102)
#define PPU_OAMADDH (*(volatile uint8_t *)0x2103)
#define PPU_OAMDATA (*(volatile uint8_t *)0x2104)
#define PPU_BGMODE (*(volatile uint8_t *)0x2105)
#define PPU_MOSAIC (*(volatile uint8_t *)0x2106)
#define PPU_BG1SC (*(volatile uint8_t *)0x2107)
#define PPU_BG2SC (*(volatile uint8_t *)0x2108)
#define PPU_BG3SC (*(volatile uint8_t *)0x2109)
#define PPU_BG4SC (*(volatile uint8_t *)0x210a)
#define PPU_BG12NBA (*(volatile uint8_t *)0x210b)
#define PPU_BG34NBA (*(volatile uint8_t *)0x210c)
#define PPU_BG1HOFS (*(volatile uint8_t *)0x210d)
#define PPU_BG1VOFS (*(volatile uint8_t *)0x210e)
#define PPU_BG2HOFS (*(volatile uint8_t *)0x210f)
#define PPU_BG2VOFS (*(volatile uint8_t *)0x2110)
#define PPU_BG3HOFS (*(volatile uint8_t *)0x2111)
#define PPU_BG3VOFS (*(volatile uint8_t *)0x2112)
#define PPU_BG4HOFS (*(volatile uint8_t *)0x2113)
#define PPU_BG4VOFS (*(volatile uint8_t *)0x2114)
#define PPU_M7HOFS (*(volatile uint8_t *)0x210d)
#define PPU_M7VOFS (*(volatile uint8_t *)0x210e)
#define PPU_VMAIN (*(volatile uint8_t *)0x2115)
#define PPU_VMADD (*(volatile uint16_t *)0x2116)
#define PPU_VMADDL (*(volatile uint8_t *)0x2116)
#define PPU_VMADDH (*(volatile uint8_t *)0x2117)
#define PPU_VMDATA (*(volatile uint16_t *)0x2118)
#define PPU_VMDATAL (*(volatile uint8_t *)0x2118)
#define PPU_VMDATAH (*(volatile uint8_t *)0x2119)
#define PPU_M7SEL (*(volatile uint8_t *)0x211a)
#define PPU_M7A (*(volatile uint8_t *)0x211b)
#define PPU_M7B (*(volatile uint8_t *)0x211c)
#define PPU_M7C (*(volatile uint8_t *)0x211d)
#define PPU_M7D (*(volatile uint8_t *)0x211e)
#define PPU_M7X (*(volatile uint8_t *)0x211f)
#define PPU_M7Y (*(volatile uint8_t *)0x2120)
#define PPU_CGADD (*(volatile uint8_t *)0x2121)
#define PPU_CGDATA (*(volatile uint8_t *)0x2122)
#define PPU_W12SEL (*(volatile uint8_t *)0x2123)
#define PPU_W34SEL (*(volatile uint8_t *)0x2124)
#define PPU_WOBJSEL (*(volatile uint8_t *)0x2125)
#define PPU_WH0 (*(volatile uint8_t *)0x2126)
#define PPU_WH1 (*(volatile uint8_t *)0x2127)
#define PPU_WH2 (*(volatile uint8_t *)0x2128)
#define PPU_WH3 (*(volatile uint8_t *)0x2129)
#define PPU_WBGLOG (*(volatile uint8_t *)0x212a)
#define PPU_WOBJLOG (*(volatile uint8_t *)0x212b)
#define PPU_TM (*(volatile uint8_t *)0x212c)
#define PPU_TS (*(volatile uint8_t *)0x212d)
#define PPU_TMW (*(volatile uint8_t *)0x212e)
#define PPU_TSW (*(volatile uint8_t *)0x212f)
#define PPU_CGWSEL (*(volatile uint8_t *)0x2130)
#define PPU_CGADSUB (*(volatile uint8_t *)0x2131)
#define PPU_COLDATA (*(volatile uint8_t *)0x2132)
#define PPU_SETINI (*(volatile uint8_t *)0x2133)
#define PPU_MPYML (*(volatile const uint16_t *)0x2134)
#define PPU_MPYHL (*(volatile const uint16_t *)0x2135)
#define PPU_MPYL (*(volatile const uint8_t *)0x2134)
#define PPU_MPYM (*(volatile const uint8_t *)0x2135)
#define PPU_MPYH (*(volatile const uint8_t *)0x2136)
#define PPU_SLHV (*(volatile const uint8_t *)0x2137)
#define PPU_OAMDATAREAD (*(volatile const uint8_t *)0x2138)
#define PPU_VMDATAREAD (*(volatile const uint8_t *)0x2139)
#define PPU_VMDATALREAD (*(volatile const uint8_t *)0x2139)
#define PPU_VMDATAHREAD (*(volatile const uint8_t *)0x213a)
#define PPU_CGDATAREAD (*(volatile const uint8_t *)0x213b)
#define PPU_OPHCT (*(volatile const uint8_t *)0x213c)
#define PPU_OPVCT (*(volatile const uint8_t *)0x213d)
#define PPU_STAT77 (*(volatile const uint8_t *)0x213e)
#define PPU_STAT78 (*(volatile const uint8_t *)0x213f)

#define MMIO_APUIO_OUT_0 (*(volatile uint8_t *)0x2140)
#define MMIO_APUIO_OUT_1 (*(volatile uint8_t *)0x2141)
#define MMIO_APUIO_OUT_2 (*(volatile uint8_t *)0x2142)
#define MMIO_APUIO_OUT_3 (*(volatile uint8_t *)0x2143)
#define MMIO_APUIO_IN_0 (*(volatile const uint8_t *)0x2140)
#define MMIO_APUIO_IN_1 (*(volatile const uint8_t *)0x2141)
#define MMIO_APUIO_IN_2 (*(volatile const uint8_t *)0x2142)
#define MMIO_APUIO_IN_3 (*(volatile const uint8_t *)0x2143)
#define MMIO_WMDATA (*(volatile uint8_t *)0x2180)
#define MMIO_WMADDML (*(volatile uint16_t *)0x2181)
#define MMIO_WMADDL (*(volatile uint8_t *)0x2181)
#define MMIO_WMADDM (*(volatile uint8_t *)0x2182)
#define MMIO_WMADDH (*(volatile uint8_t *)0x2183)
#define MMIO_JOYOUT (*(volatile uint8_t *)0x4016)
#define MMIO_JOYSER0 (*(volatile uint8_t *)0x4016)
#define MMIO_JOYSER1 (*(volatile uint8_t *)0x4017)
#define MMIO_NMITIMEN (*(volatile uint8_t *)0x4200)
#define MMIO_WRIO (*(volatile uint8_t *)0x4201)
#define MMIO_HTIME (*(volatile uint8_t *)0x4207)
#define MMIO_HTIMEL (*(volatile uint8_t *)0x4207)
#define MMIO_HTIMEH (*(volatile uint8_t *)0x4208)
#define MMIO_VTIME (*(volatile uint8_t *)0x4209)
#define MMIO_VTIMEL (*(volatile uint8_t *)0x4209)
#define MMIO_VTIMEH (*(volatile uint8_t *)0x420a)
#define MMIO_MEMSEL (*(volatile uint8_t *)0x420d)
#define MMIO_RDNMI (*(volatile const uint8_t *)0x4210)
#define MMIO_TIMEUP (*(volatile const uint8_t *)0x4211)
#define MMIO_HVBJOY (*(volatile const uint8_t *)0x4212)
#define MMIO_RDIO (*(volatile const uint8_t *)0x4213)

#define MMIO_JOY1 (*(volatile const uint16_t *)0x4218)
#define MMIO_JOY2 (*(volatile const uint16_t *)0x421a)
#define MMIO_JOY3 (*(volatile const uint16_t *)0x421c)
#define MMIO_JOY4 (*(volatile const uint16_t *)0x421e)

#define MMIO_JOY1L (*(volatile const uint8_t *)0x4218)
#define MMIO_JOY1H (*(volatile const uint8_t *)0x4219)
#define MMIO_JOY2L (*(volatile const uint8_t *)0x421a)
#define MMIO_JOY2H (*(volatile const uint8_t *)0x421b)
#define MMIO_JOY3L (*(volatile const uint8_t *)0x421c)
#define MMIO_JOY3H (*(volatile const uint8_t *)0x421d)
#define MMIO_JOY4L (*(volatile const uint8_t *)0x421e)
#define MMIO_JOY4H (*(volatile const uint8_t *)0x421f)

#define DMA_DMAEN (*(volatile uint8_t *)0x420b)
#define DMA_HDMAEN (*(volatile uint8_t *)0x420c)

#define DMA_DMAP0 (*(volatile uint8_t *)0x4300)
#define DMA_DMAP1 (*(volatile uint8_t *)0x4310)
#define DMA_DMAP2 (*(volatile uint8_t *)0x4320)
#define DMA_DMAP3 (*(volatile uint8_t *)0x4330)
#define DMA_DMAP4 (*(volatile uint8_t *)0x4340)
#define DMA_DMAP5 (*(volatile uint8_t *)0x4350)
#define DMA_DMAP6 (*(volatile uint8_t *)0x4360)
#define DMA_DMAP7 (*(volatile uint8_t *)0x4370)

#define DMA_BBAD0 (*(volatile uint8_t *)0x4301)
#define DMA_BBAD1 (*(volatile uint8_t *)0x4311)
#define DMA_BBAD2 (*(volatile uint8_t *)0x4321)
#define DMA_BBAD3 (*(volatile uint8_t *)0x4331)
#define DMA_BBAD4 (*(volatile uint8_t *)0x4341)
#define DMA_BBAD5 (*(volatile uint8_t *)0x4351)
#define DMA_BBAD6 (*(volatile uint8_t *)0x4361)
#define DMA_BBAD7 (*(volatile uint8_t *)0x4371)

#define DMA_DMAP_BBAD_0 (*(volatile uint16_t *)0x4300)
#define DMA_DMAP_BBAD_1 (*(volatile uint16_t *)0x4310)
#define DMA_DMAP_BBAD_2 (*(volatile uint16_t *)0x4320)
#define DMA_DMAP_BBAD_3 (*(volatile uint16_t *)0x4330)
#define DMA_DMAP_BBAD_4 (*(volatile uint16_t *)0x4340)
#define DMA_DMAP_BBAD_5 (*(volatile uint16_t *)0x4350)
#define DMA_DMAP_BBAD_6 (*(volatile uint16_t *)0x4360)
#define DMA_DMAP_BBAD_7 (*(volatile uint16_t *)0x4370)

#define DMA_A1T0 (*(volatile uint16_t *)0x4302)
#define DMA_A1T1 (*(volatile uint16_t *)0x4312)
#define DMA_A1T2 (*(volatile uint16_t *)0x4322)
#define DMA_A1T3 (*(volatile uint16_t *)0x4332)
#define DMA_A1T4 (*(volatile uint16_t *)0x4342)
#define DMA_A1T5 (*(volatile uint16_t *)0x4352)
#define DMA_A1T6 (*(volatile uint16_t *)0x4362)
#define DMA_A1T7 (*(volatile uint16_t *)0x4372)

#define DMA_A1B0 (*(volatile uint8_t *)0x4304)
#define DMA_A1B1 (*(volatile uint8_t *)0x4314)
#define DMA_A1B2 (*(volatile uint8_t *)0x4324)
#define DMA_A1B3 (*(volatile uint8_t *)0x4334)
#define DMA_A1B4 (*(volatile uint8_t *)0x4344)
#define DMA_A1B5 (*(volatile uint8_t *)0x4354)
#define DMA_A1B6 (*(volatile uint8_t *)0x4364)
#define DMA_A1B7 (*(volatile uint8_t *)0x4374)

#define DMA_DAS0 (*(volatile uint16_t *)0x4305)
#define DMA_DAS1 (*(volatile uint16_t *)0x4315)
#define DMA_DAS2 (*(volatile uint16_t *)0x4325)
#define DMA_DAS3 (*(volatile uint16_t *)0x4335)
#define DMA_DAS4 (*(volatile uint16_t *)0x4345)
#define DMA_DAS5 (*(volatile uint16_t *)0x4355)
#define DMA_DAS6 (*(volatile uint16_t *)0x4365)
#define DMA_DAS7 (*(volatile uint16_t *)0x4375)

#define DMA_DASB0 (*(volatile uint8_t *)0x4307)
#define DMA_DASB1 (*(volatile uint8_t *)0x4317)
#define DMA_DASB2 (*(volatile uint8_t *)0x4327)
#define DMA_DASB3 (*(volatile uint8_t *)0x4337)
#define DMA_DASB4 (*(volatile uint8_t *)0x4347)
#define DMA_DASB5 (*(volatile uint8_t *)0x4357)
#define DMA_DASB6 (*(volatile uint8_t *)0x4367)
#define DMA_DASB7 (*(volatile uint8_t *)0x4377)

#define DMA_A2A0 (*(volatile uint16_t *)0x4308)
#define DMA_A2A1 (*(volatile uint16_t *)0x4318)
#define DMA_A2A2 (*(volatile uint16_t *)0x4328)
#define DMA_A2A3 (*(volatile uint16_t *)0x4338)
#define DMA_A2A4 (*(volatile uint16_t *)0x4348)
#define DMA_A2A5 (*(volatile uint16_t *)0x4358)
#define DMA_A2A6 (*(volatile uint16_t *)0x4368)
#define DMA_A2A7 (*(volatile uint16_t *)0x4378)

#define DMA_NLTR0 (*(volatile uint8_t *)0x430a)
#define DMA_NLTR1 (*(volatile uint8_t *)0x431a)
#define DMA_NLTR2 (*(volatile uint8_t *)0x432a)
#define DMA_NLTR3 (*(volatile uint8_t *)0x433a)
#define DMA_NLTR4 (*(volatile uint8_t *)0x434a)
#define DMA_NLTR5 (*(volatile uint8_t *)0x435a)
#define DMA_NLTR6 (*(volatile uint8_t *)0x436a)
#define DMA_NLTR7 (*(volatile uint8_t *)0x437a)

#define INIDISP_FORCE_BLANK 0x80
#define INIDISP_BRIGHTNESS_MASK 0x0f

#define OBJSEL_SIZE_8_16 0x00
#define OBJSEL_SIZE_8_32 0x20
#define OBJSEL_SIZE_8_64 0x40
#define OBJSEL_SIZE_16_32 0x60
#define OBJSEL_SIZE_16_64 0x80
#define OBJSEL_SIZE_32_64 0xa0
#define OBJSEL_NAME_SHIFT 3
#define OBJSEL_NAME_WALIGN 4096
#define OBJSEL_BASE_SHIFT 0
#define OBJSEL_BASE_WALIGN 8192
#define OBJSEL_(base, name, size) (((base) / 8192) | ((name) << 3) | (size))

#define BGMODE_M1_BG3P 0x09

#define BGMODE_BG1_16PX 0x10
#define BGMODE_BG2_16PX 0x20
#define BGMODE_BG3_16PX 0x40
#define BGMODE_BG4_16PX 0x80

#define MOSAIC_BG1 0x01
#define MOSAIC_BG2 0x02
#define MOSAIC_BG3 0x03
#define MOSAIC_BG4 0x04
#define MOSAIC_SIZE_SHIFT 4

#define BGnSC_BASE_SHIFT 2
#define BGnSC_BASE_WALIGN 1024
#define BGnSC_MAP_32X32 0
#define BGnSC_MAP_64X32 1
#define BGnSC_MAP_32X64 2
#define BGnSC_MAP_64X64 3
#define BGnSC_(base, size) ((((base) / 1024) << 2) | (size))

#define BGnnNBA_SHFIT 4
#define BGnnNBA_WALIGN 4096
#define BGnnNBA_(base1, base2) (((base1) / 4096) | (((base2) / 4096) << 4))

#define VMAIN_INCREMENT_1 0x00
#define VMAIN_INCREMENT_32 0x01
#define VMAIN_INCREMENT_128 0x02
#define VMAIN_REMAP_2BPP 0x04
#define VMAIN_REMAP_4BPP 0x08
#define VMAIN_REMAP_8BPP 0x0c
#define VMAIN_INCREMENT_L 0x00
#define VMAIN_INCREMENT_H 0x80

#define M7SEL_H_FLIP 0x01
#define M7SEL_V_FLIP 0x02
#define M7SEL_OOB_REPEAT 0x00
#define M7SEL_OOB_COLOR0 0x80
#define M7SEL_OOB_TILE0 0xc0

#define WSEL_W1_INVERT 0x01
#define WSEL_W1_ENABLE 0x02
#define WSEL_W2_INVERT 0x04
#define WSEL_W2_ENABLE 0x08
#define WSEL_(a, b) ((a) | (b) << 4)

#define WBGLOG_OR 0
#define WBGLOG_AND 1
#define WBGLOG_XOR 2
#define WBGLOG_XNOR 3
#define WBGLOG_(bg1, bg2, bg3, bg4) ((bg1) | ((bg2) << 2) | ((bg3) << 4) | ((bg4) << 6))
#define WOBJLOG_(obj, color) ((obj) | ((color) << 2))

#define T_BG1 1
#define T_BG2 2
#define T_BG3 4
#define T_BG4 8
#define T_OBJ 16

#define CGWSEL_DIRECT_COLOR 0x01
#define CGWSEL_ADD_SUBSCREEN 0x02
#define CGWSEL_SUBSCREEN_TRANSPARENT_NOWHERE 0x00
#define CGWSEL_SUBSCREEN_TRANSPARENT_OUTSIDE 0x10
#define CGWSEL_SUBSCREEN_TRANSPARENT_INSIDE 0x20
#define CGWSEL_SUBSCREEN_TRANSPARENT_ALWAYS 0x30
#define CGWSEL_MAINSCREEN_BLACK_NOWHERE 0x00
#define CGWSEL_MAINSCREEN_BLACK_OUTSIDE 0x40
#define CGWSEL_MAINSCREEN_BLACK_INSIDE 0x80
#define CGWSEL_MAINSCREEN_BLACK_ALWAYS 0xc0

#define CGADSUB_BG1 0x01
#define CGADSUB_BG2 0x02
#define CGADSUB_BG3 0x04
#define CGADSUB_BG4 0x08
#define CGADSUB_OBJ 0x10
#define CGADSUB_BACKDROP 0x20
#define CGADSUB_HALF_COLOR 0x40
#define CGADSUB_SUBTRACT 0x80

#define COLDATA_BLUE 0x80
#define COLDATA_GREEN 0x40
#define COLDATA_RED 0x20
#define COLDATA_INTENSITY_MASK 0x1f

#define SETINI_SCREEN_INTERLACE 0x01
#define SETINI_SCREEN_OBJ_INTERLACE 0x02
#define SETINI_OVERSCAN 0x04
#define SETINI_HIGH_RES 0x08
#define SETINI_EXTBG 0x40
#define SETINI_EXT_SYNC 0x80

#define STAT77_VERSION_MASK 0x0f
#define STAT77_RANGE_OVER 0x40
#define STAT77_TIME_OVER 0x80

#define STAT78_VERSION_MASK 0x0f
#define STAT78_PAL 0x10
#define STAT78_COUNTER_LATCH 0x40
#define STAT78_INTERLACE 0x80

#define NMITIMEN_AUTO_READ 0x01
#define NMITIMEN_IRQ_H 0x20
#define NMITIMEN_IRQ_V 0x40
#define NMITIMEN_VBLANK 0x80

#define WRIO_PORT2 0x40
#define WRIO_PORT1 0x80

#define MEMSEL_FASTROM 1

#define RDNMI_MASK 0x0f
#define RDNMI_NMI_FLAG 0x80

#define HVBJOY_AUTO_READ 0x01
#define HVBJOY_HBLANK 0x40
#define HVBJOY_VBLANK 0x80

#define RDIO_PORT1 0x80
#define RDIO_PORT2 0x40

#define JOYPAD_L_A 0b10000000
#define JOYPAD_L_X 0b01000000
#define JOYPAD_L_L 0b00100000
#define JOYPAD_L_R 0b00010000

#define JOYPAD_H_B 0b10000000
#define JOYPAD_H_Y 0b01000000
#define JOYPAD_H_SELECT 0b00100000
#define JOYPAD_H_START 0b00010000
#define JOYPAD_H_UP 0b00001000
#define JOYPAD_H_DOWN 0b00000100
#define JOYPAD_H_LEFT 0b00000010
#define JOYPAD_H_RIGHT 0b00000001
#define JOYPAD_H_DPAD 0b00001111

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

#define DMAP_TRANSFER_ONE 0b000
#define DMAP_TRANSFER_TWO 0b001
#define DMAP_TRANSFER_WRITE_TWICE 0b010
#define DMAP_TRANSFER_TWO_WRITE_TWICE 0b011
#define DMAP_TRANSFER_FOUR 0b100

#define DMAP_TRANSFER_READ_TWICE DMAP_TRANSFER_WRITE_TWICE
#define DMAP_TRANSFER_TWO_READ_TWICE DMAP_TRANSFER_TWO_WRITE_TWICE

#define DMAP_FIXED 0x08
#define DMAP_DECREMENT 0x10
#define DMAP_INDIRECT 0x40
#define DMAP_B_TO_A 0x80

#define DMAP_BBAD_(bbad, reg) ((bbad) | (((reg) - 0x2100) << 8))

#endif
