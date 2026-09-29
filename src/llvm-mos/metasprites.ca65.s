; Metasprite drawing functions for llvm-mos
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

.p816


.export draw_metasprite_biased, finalize_metasprites

.importzp __rc2, __rc4

.importzp oamBufferPos, oamHiTableBitBuffer, oamHiTablePos
.import oamBuffer, prevOamBufferPos

.import METASPRITE_ROM_DATA

; ::HACK read METASPRITE_ROM_DATA from the correct data bank::
; ::: llvm-mos exports the symbol with absolute address size::
MS_DATA = $810000 | METASPRITE_ROM_DATA
far_oamBuffer = $7e0000 | oamBuffer
far_oamHiBuffer = ($7e0000 | oamBuffer) + 512


; ::TODO move draw_metasprite_biased to bank81::
; ::TODO call `draw_metasprite_baised` using jsl::
; `_p` escapes to `.`
.segment "_ptext_pmetasprites"


; Draw a metasprite to the oamBuffer
;
; INPUT:
;   A = frameset id
;   X = frame id
;   rs1 (rc2/rc3) = xpos
;   rs2 (rc4/rc5) = ypos
.a8
.i8
; DB = $80
.proc draw_metasprite_biased
_xpos = __rc2
_ypos = __rc4

    pea     $807e
    plb
; DB = $7e

    rep     #$30
.a16
.i16
    txy

    and     #$00ff
    asl
    tax

    tya
    asl
    ; carry clear
    adc     f:MS_DATA,x
    tax
    lda     f:MS_DATA,x
    tax

    ldy     oamBufferPos

        DrawLoop:
            lda     f:MS_DATA + 0,x
            and     #$00ff
            beq     EndLoop
            clc
            adc     _xpos
            ; Move xpos offscreen if xPos <= -16 or >= 256
            cmp     #.loword(-16 + 1)
            bcs     :+
                cmp     #256
                bcs     SkipSprite
            :

            sta     oamBuffer + 0,y
            asl

            sep     #$20
        .a8
            ror     oamHiTableBitBuffer

            lda     f:MS_DATA + 1,x
            lsr
            ror     oamHiTableBitBuffer
            bcc     SkipHiWrite
                ; oamHiTableBitBuffer is full, write to hiTable
                xba

                ; ASSUMES: OAM Hi table is completely inside a page
                .assert oamBuffer .mod 32 = 0, lderror, "oamBuffer must be 32 byte aligned"
                lda     oamHiTableBitBuffer
                sta     (oamHiTablePos)
                inc     oamHiTablePos

                ; Reset oamHiTableBitBuffer
                lda     #$80
                sta     oamHiTableBitBuffer

                xba
            SkipHiWrite:

            ; A = unmasked ypos
            rep     #$21
        .a16
            and     #$007f
            ; carry clear
            adc     _ypos
            ; Move ypos offscreen if yPos <= -16 or >= 224
            cmp     #.loword(-16 + 1)
            bcs     :+
                cmp     #224
                bcc     :+
                    ; Do not skip sprite if offscreen in the Y direction
                    ; (the hiTable has already been written)
                    lda     #.loword(-16)
            :
            sta     oamBuffer + 1,y

            lda     f:MS_DATA + 2,x
            sta     oamBuffer + 2,y

            iny
            iny
            iny
            iny

        SkipSprite:
            inx
            inx
            inx
            inx

            cpy     #512
            bcc     DrawLoop

EndLoop:
    sty     oamBufferPos

Return:
    sep     #$30
.a8
.i8
    plb
; DB = $80
    rts
.endproc



.a8
.i8
; DB = $80
.proc finalize_metasprites
    lda     oamHiTableBitBuffer
    cmp     #$80
    beq     HiTableBufferEmpty

        HiTableLoop:
            lsr
            lsr
            bcc     HiTableLoop

        ; Hack that writes to oamHiTablePos with DB = $80 and 8 bit index
        .assert .hibyte(far_oamHiBuffer + 512) = .hibyte(far_oamHiBuffer + 544), lderror
        ldx     oamHiTablePos
        sta     far_oamHiBuffer & $ffff00,x
    HiTableBufferEmpty:

    rts
.endproc
