; Metasprite functions for vbcc
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

    zpage r0, r1, r2
    zpage _oamBufferPos, _oamHiTableBitBuffer, _oamHiTablePos


; Draw a metasprite to the oamBuffer
;
; INPUT:
;   A = frameset id
;   r0 = frame id
;   r1 = xpos
;   r2 = ypox
; DB = 00
    section	"DONTMERGE_text.near._draw_metasprite_biased","acrx"
    global _draw_metasprite_biased
_draw_metasprite_biased:
    a16
    x16

._frame = r0
._xpos = r1
._ypos = r2

    pea     $807e
    plb
; DB = $7e

    and     #$00ff
    asl
    tax

    lda     ._frame
    and     #$00ff
    asl
    ; carry clear
    adc     >_METASPRITE_ROM_DATA,x
    tax
    lda     >_METASPRITE_ROM_DATA,x
    tax

    ldy     _oamBufferPos

        .DrawLoop:
            lda     >_METASPRITE_ROM_DATA + 0,x
            and     #$00ff
            beq     .EndLoop
            clc
            adc     ._xpos
            ; Move xpos offscreen if xPos <= -16 or >= 256
            cmp     #-16 + 1
            bcs     :+
                cmp     #256
                bcs     .SkipSprite
            :

            sta     _oamBuffer + 0,y
            asl

            sep     #$20
        a8
            ror     _oamHiTableBitBuffer

            lda     >_METASPRITE_ROM_DATA + 1,x
            lsr
            ror     _oamHiTableBitBuffer
            bcc     .SkipHiWrite
                ; oamHiTableBitBuffer is full, write to hiTable
                xba

                ; ASSUMES: OAM Hi table is completely inside a page
                assert _oamBuffer % 32 = 0
                lda     _oamHiTableBitBuffer
                sta     (_oamHiTablePos)
                inc     _oamHiTablePos

                ; Reset oamHiTableBitBuffer
                lda     #$80
                sta     _oamHiTableBitBuffer

                xba
            .SkipHiWrite:

            ; A = unmasked ypos
            rep     #$21
        a16
            and     #$007f
            ; carry clear
            adc     ._ypos
            ; Move ypos offscreen if yPos <= -16 or >= 224
            cmp     #-16 + 1
            bcs     :+
                cmp     #224
                bcc     :+
                    ; Do not skip sprite if offscreen in the Y direction
                    ; (the hiTable has already been written)
                    lda     #-16
            :
            sta     _oamBuffer + 1,y

            lda     >_METASPRITE_ROM_DATA + 2,x
            sta     _oamBuffer + 2,y

            iny
            iny
            iny
            iny

        .SkipSprite:
            inx
            inx
            inx
            inx

            cpy     #512
            bcc     .DrawLoop

.EndLoop:
    sty     _oamBufferPos

.Return:
    plb
; DB = $80
    rtl



    section	"DONTMERGE_text.near._finalize_metasprites.0","acrx"
    global _finalize_metasprites
_finalize_metasprites:
    a16
    x16
    ; DB = $80

    sep     #$20
 a8

    lda     _oamHiTableBitBuffer
    cmp     #$80
    beq     .HiTableBufferEmpty

        .HiTableLoop:
            lsr
            lsr
            bcc     .HiTableLoop

        ; Have to use 16-bit X here.
        ; `sta _oamBuffer&$ffff00` is an "illegal relocation" error
        ldx     _oamHiTablePos
        sta     $7e0000,x
    .HiTableBufferEmpty:

    rep     #$20
 a16
    rtl
