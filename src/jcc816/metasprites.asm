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


.a16
.i16
; DB = $80
.proc draw_metasprite_biased
    pea     $807e
    plb
; DB = $7e

    lda     draw_metasprite_biased@frameset
    and     #$00ff
    asl
    tax

    lda     draw_metasprite_biased@frame
    and     #$00ff
    asl
    ; carry clear
    adc     f:METASPRITE_ROM_DATA,x
    tax
    lda     f:METASPRITE_ROM_DATA,x
    tax

    ldy     .loword(oamBufferPos)

        DrawLoop:
            lda     f:METASPRITE_ROM_DATA + 0,x
            and     #$00ff
            beq     EndLoop
            clc
            adc     draw_metasprite_biased@biased_x
            ; Move xpos offscreen if xPos <= -16 or >= 256
            cmp     #.loword(-16 + 1)
            bcs     :+
                cmp     #256
                bcs     SkipSprite
            :

            sta     .loword(oamBuffer + 0),y
            asl

            sep     #$20
        .a8
            ror     .loword(oamHiTableBitBuffer)

            lda     f:METASPRITE_ROM_DATA + 1,x
            lsr
            ror     .loword(oamHiTableBitBuffer)
            bcc     SkipHiWrite
                ; oamHiTableBitBuffer is full, write to hiTable
                xba

                phy
                    ldy     .loword(oamHiTablePos)

                    lda     .loword(oamHiTableBitBuffer)
                    sta     $0000,y

                    iny
                    sty     .loword(oamHiTablePos)
                ply

                ; Reset oamHiTableBitBuffer
                lda     #$80
                sta     .loword(oamHiTableBitBuffer)

                xba
            SkipHiWrite:

            ; A = unmasked ypos
            rep     #$21
        .a16
            and     #$007f
            ; carry clear
            adc     draw_metasprite_biased@biased_y
            ; Move ypos offscreen if yPos <= -16 or >= 224
            cmp     #.loword(-16 + 1)
            bcs     :+
                cmp     #224
                bcc     :+
                    ; Do not skip sprite if offscreen in the Y direction
                    ; (the hiTable has already been written)
                    lda     #.loword(-16)
            :
            sta     .loword(oamBuffer + 1),y

            lda     f:METASPRITE_ROM_DATA + 2,x
            sta     .loword(oamBuffer + 2),y

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
    sty     .loword(oamBufferPos)

Return:
    plb
; DB = $80
    rtl
.endproc



.a16
.i16
; DB = ???
.proc finalize_metasprites
    lda     oamHiTablePos
    tax

    sep     #$20
.a8

    lda     oamHiTableBitBuffer
    cmp     #$80
    beq     HiTableBufferEmpty

        HiTableLoop:
            lsr
            lsr
            bcc     HiTableLoop

        sta     $7e0000,x
    HiTableBufferEmpty:

    rep     #$30
.a16
    rtl
.endproc
