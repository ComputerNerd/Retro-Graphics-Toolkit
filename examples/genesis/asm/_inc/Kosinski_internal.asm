; ---------------------------------------------------------------------------
; This is the internal portion of the code, which is shared by KosDec and
; Process_Kos_Queue. DO NOT CALL THIS DIRECTLY.
; ---------------------------------------------------------------------------
; For format explanation see https://segaretro.org/Kosinski_compression
; ---------------------------------------------------------------------------
; Permission to use, copy, modify, and/or distribute this software for any
; purpose with or without fee is hereby granted.
;
; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
; WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
; ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
; ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
; OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
; ---------------------------------------------------------------------------
	moveq	#(1<<_Kos_LoopUnroll)-1,d7
	if _Kos_UseLUT==1
		moveq	#0,d0
		moveq	#0,d1
		lea	KosDec_ByteMap(pc),a4	; Load LUT pointer.
	endif
	move.b	(a0)+,d0				; Get desc field low-byte.
	move.b	(a0)+,d1				; Get desc field hi-byte.
	if _Kos_UseLUT==1
		move.b	(a4,d0.w),d0		; Invert bit order...
		move.b	(a4,d1.w),d1		; ... for both bytes.
	endif
	moveq	#7,d2					; Set repeat count to 8.
	moveq	#0,d3					; d3 will be desc field switcher.
	bra.s	.FetchNewCode
; ---------------------------------------------------------------------------
.FetchCodeLoop:
	; Code 1 (Uncompressed byte).
	_Kos_RunBitStream
	move.b	(a0)+,(a1)+

.FetchNewCode:
	_Kos_ReadBit
	bcs.s	.FetchCodeLoop			; If code = 1, branch.

	; Codes 00 and 01.
	moveq	#-1,d5
	lea	(a1),a5
	_Kos_RunBitStream
  if _Kos_ExtremeUnrolling==1
	_Kos_ReadBit
	bcs.w	.Code_01

	; Code 00 (Dictionary ref. short).
	_Kos_RunBitStream
	_Kos_ReadBit
	bcs.s	.Copy45
	_Kos_RunBitStream
	_Kos_ReadBit
	bcs.s	.Copy3
	_Kos_RunBitStream
	move.b	(a0)+,d5				; d5 = displacement.
	adda.w	d5,a5
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	bra.s	.FetchNewCode
; ---------------------------------------------------------------------------
.Copy3:
	_Kos_RunBitStream
	move.b	(a0)+,d5				; d5 = displacement.
	adda.w	d5,a5
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	bra.w	.FetchNewCode
; ---------------------------------------------------------------------------
.Copy45:
	_Kos_RunBitStream
	_Kos_ReadBit
	bcs.s	.Copy5
	_Kos_RunBitStream
	move.b	(a0)+,d5				; d5 = displacement.
	adda.w	d5,a5
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	bra.w	.FetchNewCode
; ---------------------------------------------------------------------------
.Copy5:
	_Kos_RunBitStream
	move.b	(a0)+,d5				; d5 = displacement.
	adda.w	d5,a5
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	move.b	(a5)+,(a1)+
	bra.w	.FetchNewCode
; ---------------------------------------------------------------------------
  else
	moveq	#0,d4					; d4 will contain copy count.
	_Kos_ReadBit
	bcs.s	.Code_01

	; Code 00 (Dictionary ref. short).
	_Kos_RunBitStream
	_Kos_ReadBit
	addx.w	d4,d4
	_Kos_RunBitStream
	_Kos_ReadBit
	addx.w	d4,d4
	_Kos_RunBitStream
	move.b	(a0)+,d5				; d5 = displacement.

.StreamCopy:
	adda.w	d5,a5
	move.b	(a5)+,(a1)+				; Do 1 extra copy (to compensate +1 to copy counter).

.copy:
	move.b	(a5)+,(a1)+
	dbra	d4,.copy
	bra.w	.FetchNewCode
  endif
; ---------------------------------------------------------------------------
.Code_01:
	moveq	#0,d4					; d4 will contain copy count.
	; Code 01 (Dictionary ref. long / special).
	_Kos_RunBitStream
	move.b	(a0)+,d6				; d6 = %LLLLLLLL.
	move.b	(a0)+,d4				; d4 = %HHHHHCCC.
	move.b	d4,d5					; d5 = %11111111 HHHHHCCC.
	lsl.w	#5,d5					; d5 = %111HHHHH CCC00000.
	move.b	d6,d5					; d5 = %111HHHHH LLLLLLLL.
	if _Kos_LoopUnroll==3
		and.w	d7,d4				; d4 = %00000CCC.
	else
		andi.w	#7,d4				; d4 = %00000CCC.
	endif
	bne.s	.StreamCopy				; if CCC=0, branch.

	; special mode (extended counter)
	move.b	(a0)+,d4				; Read cnt
	beq.s	.Quit					; If cnt=0, quit decompression.
	subq.b	#1,d4
	beq.w	.FetchNewCode			; If cnt=1, fetch a new code.

	adda.w	d5,a5
	move.b	(a5)+,(a1)+				; Do 1 extra copy (to compensate +1 to copy counter).
	move.w	d4,d6
	not.w	d6
	and.w	d7,d6
	add.w	d6,d6
	lsr.w	#_Kos_LoopUnroll,d4
	jmp	.largecopy(pc,d6.w)
; ---------------------------------------------------------------------------
.largecopy:
	rept (1<<_Kos_LoopUnroll)
		move.b	(a5)+,(a1)+
	endm
	dbra	d4,.largecopy
	bra.w	.FetchNewCode
; ---------------------------------------------------------------------------
  if _Kos_ExtremeUnrolling==1
.StreamCopy:
	adda.w	d5,a5
	move.b	(a5)+,(a1)+				; Do 1 extra copy (to compensate +1 to copy counter).
	if _Kos_LoopUnroll==3
		eor.w	d7,d4
	else
		eori.w	#7,d4
	endif
	add.w	d4,d4
	jmp	.mediumcopy(pc,d4.w)
; ---------------------------------------------------------------------------
.mediumcopy:
	rept 8
		move.b	(a5)+,(a1)+
	endm
	bra.w	.FetchNewCode
  endif
; ---------------------------------------------------------------------------
.Quit:
; ===========================================================================

