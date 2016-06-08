vdp_data_port:		equ $C00000
vdp_control_port:	equ $C00004
vdp_counter:		equ $C00008
v_vdp_buffer2:	= $FFFFF640	; VDP instruction buffer (2 bytes)
; ---------------------------------------------------------------------------
; DMA copy data from 68K (ROM/RAM) to the CRAM
; input: source, length, destination
; ---------------------------------------------------------------------------

writeCRAM:	macro
		lea	(vdp_control_port).l,a5
		move.l	#$94000000+(((\2>>1)&$FF00)<<8)+$9300+((\2>>1)&$FF),(a5)
		move.l	#$96000000+(((\1>>1)&$FF00)<<8)+$9500+((\1>>1)&$FF),(a5)
		move.w	#$9700+((((\1>>1)&$FF0000)>>16)&$7F),(a5)
		move.w	#$C000+(\3&$3FFF),(a5)
		move.w	#$80+((\3&$C000)>>14),(v_vdp_buffer2).w
		move.w	(v_vdp_buffer2).w,(a5)
		endm
; ---------------------------------------------------------------------------
; disable interrupts
; ---------------------------------------------------------------------------

disable_ints:	macro
		move	#$2700,sr
		endm

; ---------------------------------------------------------------------------
; enable interrupts
; ---------------------------------------------------------------------------

enable_ints:	macro
		move	#$2300,sr
		endm

; ---------------------------------------------------------------------------
; Set a VRAM address via the VDP control port.
; input: 16-bit VRAM address, control port (default is ($C00004).l)
; ---------------------------------------------------------------------------

locVRAM:	macro loc,controlport
		if (narg=1)
		move.l	#($40000000+((loc&$3FFF)<<16)+((loc&$C000)>>14)),(vdp_control_port).l
		else
		move.l	#($40000000+((loc&$3FFF)<<16)+((loc&$C000)>>14)),controlport
		endc
		endm

bitStart:	equ 7
bitA:		equ 6
bitC:		equ 5
bitB:		equ 4
bitR:		equ 3
bitL:		equ 2
bitDn:		equ 1
bitUp:		equ 0

	rsset $FF0000
amountGroups	rs.w 1
;placeHolder	rs.w 1
joyPad		rs.l 1
txtBuf		rs.b 16
txtBuf2		rs.b 16

Vectors:
	dc.l Vectors, Entrypoint, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l HBlank, Error, VBlank, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
	dc.l Error, Error, Error, Error
Header:
	dc.b "SEGA MEGA DRIVE " ; Console name
	dc.b "(C) Sega 16 2014" ; Copyright/Date
DomesticName:
	dc.b "Code template by drx - www.hacking-cult.org	 " ; Domestic Name
	dc.b "Code template by drx - www.hacking-cult.org	 " ; International Name
	dc.b "GM 00000000-00" ; Version
Checksum:
	dc.w $1337  ; Checksum
	dc.b "J			   " ; I/O Support
RomStartLoc:
	dc.l 0 ; ROM Start
RomEndLoc:
	dc.l RomEnd ; ROM End
RamStartLoc:
	dc.l $FF0000 ; RAM Start
RamEndLoc:
	dc.l $FFFFFF ; RAM End

	dc.b $20,$20,$20,$20 ; 'RA',$F8,$20 if SRAM = on

SramStart:
	dc.l $20202020 ; $200000 if SRAM = on
SramEnd:
	dc.l $20202020 ; $20xxxx if SRAM = on

	dc.b  "	"
	dc.b  "				"
	dc.b  "				"
	dc.b  "				"
	dc.b  "F			   "  ; enable any hardware configuration


;---------------------
; Code start
;---------------------

Entrypoint:

	tst.l ($A10008).l ;Test Port A control
	bne PortA_Ok

	tst.w ($A1000C).l ;Test Port C control

PortA_Ok:
	bne SkipSetup

	move.b ($A10001).l,d0 ;version
	andi.b #$F,d0
	beq SkipSecurity ;if the smd/gen model is 1, skip the security
	move.l #'SEGA',($A14000).l

SkipSecurity:

	move.w ($C00004).l,d0 ;hang if VDP locked due to security failure

	moveq #0,d0
	movea.l d0,a6
	move.l a6,usp ;set usp to $0

;---------------------
; Setup VDP registers
;---------------------
	lea (VDPSetupArray,pc),a0
	move.w #(VDPSetupArrayEnd-VDPSetupArray)/2,d1 ;$18 VDP registers

VDPSetupLoop:
	move.w (a0)+,($C00004).l
	dbra d1,VDPSetupLoop

	move.l #$40000080,($C00004).l
	move.w #0,($C00000).l ;clean the screen


;---------------------
; Init the Z80
;---------------------

	move.w #$100,($A11100).l ;Stop the Z80
	move.w #$100,($A11200).l ;Deassert reset to the Z80

Waitforz80:
	btst #0,($A11100).l
	bne Waitforz80 ;Wait for z80 to halt

	lea (Z80Init,pc),a0
	lea ($A00000).l,a1
	move.w #Z80InitEnd-Z80Init,d1

InitZ80:
	move.b (a0)+,(a1)+
	dbra d1,InitZ80

	move.w #0,($A11200).l ;Assert reset to the Z80
	move.w #0,($A11100).l ;Start the Z80
	move.w #$100,($A11200).l ;Deassert reset to the Z80


;---------------------
; Reset the RAM
;---------------------

	lea ($FFFF0000).l,a0
	move.w #$3fff,d1

ClearRAM:
	move.l #0,(a0)+
	dbra d1,ClearRAM


;---------------------
; VDP again
;---------------------


	move.w	#$8174,($C00004).l
	move.w	#$8F02,($C00004).l


;---------------------
; Clear the CRAM
;---------------------

	move.l #$C0000000,($C00004).l ;Set VDP ctrl to CRAM write
	move.w #$3f,d1

ClearCRAM:
	move.w #0,($C00000).l
	dbra d1,ClearCRAM


;---------------------
; Clear the VDP stuff
;---------------------

	move.l #$40000010,($C00004).l
	move.w #$13,d1

ClearStuff:
	move.l #0,($C00000).l
	dbra d1,ClearStuff


;---------------------
; Init the PSG
;---------------------

	move.b #$9F,($C00011).l
	move.b #$BF,($C00011).l
	move.b #$DF,($C00011).l
	move.b #$FF,($C00011).l

;---------------------
; Load the z80 driver
;---------------------

	move.w #$100,($A11100).l ;Stop the Z80
	move.w #$100,($A11200).l ;Deassert reset to the Z80

Waitforz80a:
	btst #0,($A11100).l
	bne Waitforz80a ;Wait for z80 to halt

	lea (Z80Driver,pc),a0
	lea ($A00000).l,a1
	move.w #Z80DriverEnd-Z80Driver,d1

LoadZ80Driver:
	move.b (a0)+,(a1)+
	dbra d1,LoadZ80Driver

	move.w #0,($A11200).l ;Assert reset to the Z80
	move.w #0,($A11100).l ;Start the Z80
	move.w #$100,($A11200).l ;Deassert reset to the Z80

;---------------------
; Clear the registers
; and set the SR
;---------------------

	movem.l ($FF0000).l,d0-a6
	lea ($FFFE00).l,sp
	move #$2700,sr
SkipSetup:
	moveq	#$40,d0
	move.b	d0,($A10009).l	; init port 1 (joypad 1)
	move.b	d0,($A1000B).l	; init port 2 (joypad 2)
	move.b	d0,($A1000D).l	; init port 3 (expansion)

;-----------------------
; Here starts your code
;-----------------------
Main:
	writeCRAM palDat,16*2,0
	move.l	#$40200000,($c00004).l;set to tile 1
	;font is stored as 1-bit convert to 4-bit there are 95 characters to load
	lea (font8x8_basic).l,a0
	lea (vdp_data_port).l,a1
	move.w #95-1,d0
	moveq #0,d4
nextTile:
	moveq #8-1,d1
nextRow:
	moveq #8-1,d2
	moveq #0,d3
	move.b (a0)+,d4
nextPixel:
	rol.l #4,d3
	ror.b d4
	bcc skipPixel
	ori.l #$60000000,d3
skipPixel:
	dbra.w d2,nextPixel
	move.l d3,(a1)
	dbra.w d1,nextRow
	dbra.w d0,nextTile

	lea (Message).l,a0
	moveq #1,d0
	moveq #1,d1
	bsr WriteString
	lea (Message2).l,a0
	moveq #1,d0
	moveq #2,d1
	bsr WriteString
	lea (Message3).l,a0
	moveq #1,d0
	moveq #3,d1
	bsr WriteString
	lea (Message4).l,a0
	moveq #1,d0
	moveq #4,d1
	bsr WriteString
	;Figure out the total
	lea (DPLC).l,a0
isZero:
	move.w (a0)+,d0
	beq isZero
	lsr d0
	move.w d0,amountGroups
	lea (txtBuf).l,a0
	lea (txtBuf2).l,a1
	bsr WordToNumUnsigned
	lea (txtBuf).l,a0
	moveq #22,d0
	moveq #2,d1
	bsr WriteString
	moveq #0,d7
	moveq #0,d6
mainloop:
	locVRAM	$F800
	lea 	(vdp_data_port).l,a0
clrSprites:
	move.l #0,(a0)
	move.l #0,(a0)
	dbra d6,clrSprites

	move.w d7,d0
	lea (DPLC).l,a0
	bsr Sonic_Load_PLC

	lea (txtBuf).l,a0
	lea (txtBuf2).l,a1
	move.w d7,d0
	bsr WordToNumUnsigned

	move.b #$20,(a0)+
	move.b #0,(a0)
	lea (txtBuf).l,a0
	moveq #23,d0
	moveq #3,d1
	bsr WriteString
	
	lea (Mapping).l,a0
	move.w d7,d0
	bsr DisplayMapping

	lea (txtBuf).l,a0
	lea (txtBuf2).l,a1
	move.w d6,d0
	bsr WordToNumUnsigned

	move.b #$20,(a0)+
	move.b #0,(a0)
	lea (txtBuf).l,a0
	moveq #23,d0
	moveq #4,d1
	bsr WriteString
joyloop:
	bsr ReadJoypads
	;Check if left or right is pressed
	btst.b	#bitL,(joyPad)
	bne subSprite
	btst.b	#bitR,(joyPad)
	bne addSprite
	bra joyloop
subSprite:
	bsr ReadJoypads
	btst.b	#bitL,(joyPad)
	bne subSprite
	tst d7
	beq noAction
	subq.w #1,d7
	bra mainloop
noAction:
	bra joyloop
addSprite:
	bsr ReadJoypads
	btst.b	#bitR,(joyPad)
	bne addSprite
	move.w (amountGroups),d5
	subq.w #1,d5
	cmp.w d5,d7
	bcc noAction
	addq.w #1,d7
	bra mainloop
Message:
	dc.b "Sonic 3 mapping with sonic 2 DPLC",0
	even
Message2:
	dc.b "Total sprite groups:",0
	even
Message3:
	dc.b "Currently displaying:",0
	even
Message4:
	dc.b "Sprites in group:",0
	even
; ---------------------------------------------------------------------------
; Subroutine to	read joypad input, and send it to the RAM
; ---------------------------------------------------------------------------

; ||||||||||||||| S U B	R O U T	I N E |||||||||||||||||||||||||||||||||||||||

ReadJoypads:				; XREF: VBlank, HBlank
		lea	(joyPad),a0 ; address where joypad states are written
		lea	($A10003).l,a1	; first	joypad port
		bsr.s	@read		; do the first joypad
		addq.w	#2,a1		; do the second	joypad

	@read:
		move.b	#0,(a1)
		nop	
		nop	
		move.b	(a1),d0
		lsl.b	#2,d0
		andi.b	#$C0,d0
		move.b	#$40,(a1)
		nop	
		nop	
		move.b	(a1),d1
		andi.b	#$3F,d1
		or.b	d1,d0
		not.b	d0
		move.b	(a0),d1
		eor.b	d0,d1
		move.b	d0,(a0)+
		and.b	d0,d1
		move.b	d1,(a0)+
		rts	
; End of function ReadJoypads

WriteString:
	lea.l   $C00000,a4
	lea.l   $C00004,a5
	move.b  d1,d2
	and.l   #$FF,d2
	swap	d2
	lsl.l   #7,d2
	move.b  d0,d3
	and.l   #$FF,d3
	swap	d3
	asl.l   #1,d3
	add.l   d3,d2
	add.l   #$40000003,d2
	move.l  d2,(a5)
ws01:
	tst.b   (a0)
	beq.s   ws99
	move.b  (a0)+,d2
	sub.b   #' ',d2
	andi.w  #$FF,d2
	move.w  d2,(a4)
	bra.s   ws01
ws99:
	rts
	;d0 - frame
	;a0 - DPLC pointer
Sonic_Load_PLC:	;This is sliglty based on code from sonic and knuckles dissasembly
	add.w	d0,d0
	adda.w	(a0,d0.w),a0
	move.w	(a0)+,d2
	subq.w	#1,d2
	bmi.s	locret_12D20
	move.l	#$4c000000,($c00004).l	;Tile 96 is the start tile
	lea 	(vdp_data_port).l,a2
loc_12CF8:
	lea	(tileDat).l,a1
	moveq	#0,d1
	move.w	(a0)+,d1
	move.w	d1,d3
	lsr.w	#8,d3
	andi.w	#$F0,d3
	andi.w	#$FFF,d1
	lsl.l	#5,d1
	adda.l	d1,a1
	lsr.w 	#1,d3
	addq.w	#7,d3
tilesLoop:
	move.l	(a1)+,(a2)
	dbra.w	d3,tilesLoop
	dbra.w	d2,loc_12CF8
locret_12D20:
	rts
DisplayMapping:
	add.w	d0,d0
	adda.w	(a0,d0.w),a0
	move.w	(a0)+,d2
	move.w	d2,d6
	subq.w	#1,d2
	bmi.s	locret_12D20
	locVRAM	$F800
	lea 	(vdp_data_port).l,a1
	moveq #1,d1;Link
nextSprite:
	move.b (a0)+,d0
	ext.w d0
	addi.w #288,d0
	move.w d0,(a1);Y position
	moveq #0,d0
	move.b (a0)+,d0
	lsl.w #8,d0
	or.w d1,d0
	move.w d0,(a1);Width and height and link
	addq.w #1,d1
	move.w (a0)+,d0
	move.w d0,d3
	andi.w #2047,d3
	add.w #96,d3
	andi.w #-2048,d0
	or.w d3,d0
	move.w d0,(a1);VDP attributes
	move.w (a0)+,d0
	addi.w #240,d0
	move.w d0,(a1);X position
	dbra.w	d2,nextSprite
	rts
WordToNumUnsigned:
	;D0 the word you want converted
	;A0 the buffer to hold output
	;A1 temporary buffer
	;Registers used by this function
	;D1 length minus one
	;D2 temporary
	;Also stack is used and D0 is clobbered
	;First get remainder
	moveq #-1,d1
nextDigit:
	divu.w #10,d0
	move.l d0,d2
	clr.w d2
	swap d2
	addi.b #48,d2
	move.b d2,(a1)+
	andi.l #$FFFF,d0
	addi.w #1,d1
	tst d0
	bne nextDigit
revStr:
	move.b -(a1),(a0)+
	dbra.w d1,revStr
	move.b #0,(a0)
	rts
;---------------------
; VDP registers array
;---------------------

VDPSetupArray:
	dc.w $8004 ;9-bit palette = 1 (otherwise would be 3-bit), HBlank = 0
	dc.w $8134 ;Genesis display = 1, DMA = 1, VBlank = 1, display = 0
	dc.w $8230 ;Scroll A - $C000
	dc.w $8338 ;Window - $E000
	dc.w $8407 ;Scroll B - $E000
	dc.w $857C ;Sprites - $F800
	dc.w $8600 ;Unused
	dc.w $8700 ;Backdrop color - $00
	dc.w $8800 ;Unused
	dc.w $8900 ;Unused
	dc.w $8A00 ;H Interrupt register
	dc.w $8B00 ;Full screen scroll, no external interrupts
	dc.w $8C81 ;40 cells display
	dc.w $8D3F ;H Scroll - $FC00
	dc.w $8E00 ;Unused
	dc.w $8F02 ;VDP auto increment
	dc.w $9001 ;64 cells scroll
	dc.w $9100 ;Window H position
	dc.w $9200 ;Window V position
	dc.w $93FF ;DMA stuff (off)
	dc.w $94FF ;DMA stuff (off)
	dc.w $9500 ;DMA stuff (off)
	dc.w $9600 ;DMA stuff (off)
	dc.w $9780 ;DMA stuff (off)
VDPSetupArrayEnd:
;---------------------
; Z80 init code
;---------------------

Z80Init:
	dc.w $af01, $d91f, $1127, $0021, $2600, $f977
	dc.w $edb0, $dde1, $fde1, $ed47, $ed4f, $d1e1
	dc.w $f108, $d9c1, $d1e1, $f1f9, $f3ed, $5636
	dc.w $e9e9
Z80InitEnd:
;---------------------
; Error exceptions
;---------------------
Error:
	rte
;---------------------
; Horizontal Blank
;---------------------
HBlank:
	rte
;---------------------
; Vertical Blank
;---------------------
VBlank:
	rte
;---------------------
; Music driver (z80)
;---------------------		

Z80Driver:
		dc.b	$c3,$46,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
		dc.b	$00,$00,$00,$00,$00,$f3,$ed,$56,$31,$00,$20,$3a,$39,$00,$b7,$ca,$4c,$00,$21,$3a,$00,$11,$40,$00,$01,$06,$00,$ed,$b0,$3e,$00,$32,$39,$00,$3e,$b4,$32,$02,$40,$3e,$c0,$32,$03,$40,$3e,$2b,$32,$00,$40,$3e,$80,$32,$01,$40,$3a,$43,$00,$4f,$3a,$44,$00,$47,$3e,$06,$3d
		dc.b	$c2,$81,$00,$21,$00,$60,$3a,$41,$00,$07,$77,$3a,$42,$00,$77,$0f,$77,$0f,$77,$0f,$77,$0f,$77,$0f,$77,$0f,$77,$0f,$77,$3a,$40,$00,$6f,$3a,$41,$00,$f6,$80,$67,$3e,$2a,$32,$00,$40,$7e,$32,$01,$40,$21,$40,$00,$7e,$c6,$01,$77,$23,$7e,$ce,$00,$77,$23,$7e,$ce,$00,$77
		dc.b	$3a,$39,$00,$b7,$c2,$4c,$00,$0b,$78,$b1,$c2,$7f,$00,$3a,$45,$00,$b7,$ca,$4c,$00,$3d,$3a,$45,$00,$06,$ff,$0e,$ff,$c3,$7f,$00
		even
Z80DriverEnd:
palDat:
	incbin Pal-Sonic.bin
	even
tileDat:
	incbin Art-Sonic.bin
	even
DPLC:
	incbin DPLC-Sonic.bin
	even
Mapping:
	incbin Map-Sonic.bin
	even

	include "font8x8_basic.asm"
	even
RomEnd:
