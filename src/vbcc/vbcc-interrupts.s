; Interrupt Service Routines for vbcc
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

NMITIMEN = $4200
HDMAEN = $420c

    global _nmi_counter


; A basic Interrupt Server Routine that increments `nmi_counter`.
;
; A unknown
; I unknown
; DB unknown
; DP unknown
    section	"DONTMERGE_text.near.__irq_vblank.0","acrx"
    global	___irq_vblank
___irq_vblank:
    sep     #$20
a8
    pha

    ; > is far addressing
    lda     >_nmi_counter
    inc
    sta     >_nmi_counter

    pla
    rti



; A basic Interrupt Server Routine that disables interrupts and stops the CPU
;
; A unknown
; I unknown
; DB unknown
; DP unknown
    section	"DONTMERGE_text.near.__irq_brk.0","acrx"
    global	___irq_brk
___irq_brk:
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
    BRK_SpinLoop:
        ; Disable interrupts and HDMA
        stz     NMITIMEN
        stz     HDMAEN

        wai
        bra     BRK_SpinLoop

