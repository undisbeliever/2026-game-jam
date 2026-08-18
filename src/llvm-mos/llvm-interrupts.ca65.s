; Interrupt Service Routines for llvm-mos
;
; SPDX-FileCopyrightText: © 2026 Marcus Rowe <undisbeliever@gmail.com>
; SPDX-License-Identifier: Zlib
;
; Copyright © 2026 Marcus Rowe <undisbeliever@gmail.com>
;
; This software is provided 'as-is', without any express or implied warranty.
; In no event will the authors be held liable for any damages arising from the
; use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
;      1. The origin of this software must not be misrepresented; you must not
;         claim that you wrote the original software. If you use this software
;         in a product, an acknowledgment in the product documentation would be
;         appreciated but is not required.
;
;      2. Altered source versions must be plainly marked as such, and must not
;         be misrepresented as being the original software.
;
;      3. This notice may not be removed or altered from any source
;         distribution.
;


; `_p` escapes to `.`
.segment "_ptext_pinterrupts"

.p816

.export AbortIsr
.export BreakIsr
.export CopIsr
.export IrqIsr
.export NmiIsr
.export UnusedIsr

.export Reset

.import main
.import nmi_counter

.import __STACK_BOTTOM


INIDISP = $2100
WMADDML = $2181
WMADDH = $2183

NMITIMEN = $4200
MSEL = $420d
DMAEN = $420b
HDMAEN = $420c

DMAP0 = $4300
BBAD0 = $4301
A1T0 = $4302
A1T0L = $4302
A1T0H = $4303
A1B0 = $4304
DAS0 = $4305
DAS0L = $4305
DAS0H = $4306
DASB0 = $4307
A2A0 = $4308
A2A0L = $4308
A2A0H = $4309
NLTR0 = $430a



AbortIsr = BreakIsr
CopIsr = BreakIsr
IrqIsr = BreakIsr
UnusedIsr = BreakIsr


; A basic Interrupt Server Routine that increments `nmi_counter`.
;
; A unknown
; I unknown
; DB unknown
; DP unknown
.proc NmiIsr
    sep     #$20
    pha

    lda     f:nmi_counter
    inc
    sta     f:nmi_counter

    pla
    rti
.endproc


; A basic Interrupt Server Routine that disables interrupts and stops the CPU
;
; A unknown
; I unknown
; DB unknown
; DP unknown
.proc BreakIsr
    ; Switch to native mode
    clc
    xce

    ; Reset status registers and disable IRQ interrupts
    rep     #%11011011
    sep     #%00100100
; a8
; i16

    phk
    plb
; DB = 0

    ; ::TODO reset registers::
    ; ::TODO show red output::

    ; Interrupts are disabled.
    ; This spinloop will loop forever and do absolutely nothing until the console has been reset.
    SpinLoop:
        ; Disable interrupts and HDMA
        stz     NMITIMEN
        stz     HDMAEN

        wai
        bra     SpinLoop
.endproc



; Initialisation code
;
; Sets CPU registers, switches to FastROM, clears Work-RAM and calls main().
.proc Reset
    ; Change Program Bank to a FastROM bank
    jml     .loword(@FastRomReset) | $800000
@FastRomReset:

    ; Switch to native mode
    clc
    xce

    ; Clear all flags except memory-size and IRQ-disable
    rep     #$ff
    sep     #$24
.a8
.i16

    ldx     #__STACK_BOTTOM
    txs

    pea     0
    pld
; D = 0

    phk
    plb
; DB = 0x80

    ; Disable interrupts and HDMA
    stz     NMITIMEN
    stz     HDMAEN

    ; Enable Fast-ROM
    lda     #1
    sta     MSEL

    ; Disable screen
    lda     #$8f
    sta     INIDISP


    ; Fill Work-RAM with zeros using two 64KiB fixed address DMA transfers to WMDATA
@__LdxZero:
    ldx     #0          ; Also contains the zero-byte to write using DMA
    stx     WMADDML
    stz     WMADDH

    ; DMAP/BBAD = Fixed address transfer to byte register $2180
    ldy     #$8008
    sty     DMAP0

    ldy     #@__LdxZero + 1
    sty     A1T0
    stz     A1B0

    ; X = 0
    stx     DAS0

    lda     #1
    sta     DMAEN

    ; second transfer
    ; X = 0
    stx     DAS0
    sta     DMAEN


    sep     #$30
.a8
.i16
    jsr     main
    bra     BreakIsr::SpinLoop
.endproc


