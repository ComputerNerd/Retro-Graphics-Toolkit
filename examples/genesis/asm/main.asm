vdp_data_port:		equ $C00000
vdp_control_port:	equ $C00004
vdp_counter:		equ $C00008
v_vdp_buffer2:	= $FFFFF640	; VDP instruction buffer (2 bytes)
; ---------------------------------------------------------------------------
; DMA copy data from 68K (ROM/RAM) to the VRAM
; input: source, length, destination
; ---------------------------------------------------------------------------

writeVRAM:	macro
		lea	(vdp_control_port).l,a5
		move.l	#$94000000+(((\2>>1)&$FF00)<<8)+$9300+((\2>>1)&$FF),(a5)
		move.l	#$96000000+(((\1>>1)&$FF00)<<8)+$9500+((\1>>1)&$FF),(a5)
		move.w	#$9700+((((\1>>1)&$FF0000)>>16)&$7F),(a5)
		move.w	#$4000+(\3&$3FFF),(a5)
		move.w	#$80+((\3&$C000)>>14),(v_vdp_buffer2).w
		move.w	(v_vdp_buffer2).w,(a5)
		endm

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

Vectors:
    dc.l $FFFE00, Entrypoint, Error, Error
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
    dc.b "Code template by drx - www.hacking-cult.org     " ; Domestic Name
    dc.b "Code template by drx - www.hacking-cult.org     " ; International Name
    dc.b "GM 00000000-00" ; Version
Checksum:
    dc.w $1337  ; Checksum
    dc.b "J               " ; I/O Support
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

    dc.b  "    "
    dc.b  "                "
    dc.b  "                "
    dc.b  "                "
    dc.b  "F               "  ; enable any hardware configuration

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
	lea ($FFFE00).l,a7
	move #$2700,sr
SkipSetup:


;-----------------------
; Here starts your code
;-----------------------
Main:
	writeCRAM	palDat,64*2,0
	lea	(tileDat).l,a0
	lea	($FF0000).l,a1
	bsr.w	KosDec
	writeVRAM	$FF0000,1120*32,32
	lea	(tilemapDat).l,a0
	lea	($FF0000).l,a1
	move.w	#1,d0;Offset by one tile
	bsr.w	EniDec
	lea	($FF0000).l,a1
	move.l	#$40000003,d0
	moveq	#40-1,d1
	moveq	#28-1,d2
	bsr.w	TilemapToVRAM
;---------------------------------
; nothing else for the 68000 to do
;---------------------------------
loop:
	bra.s	loop
	include "Kosinski Decompression.asm"
	include "Enigma Decompression.asm"
; ---------------------------------------------------------------------------
; Subroutine to	copy a tile map from RAM to VRAM namespace

; input:
;	a1 = tile map address
;	d0 = VRAM address
;	d1 = width (cells)
;	d2 = height (cells)
; ---------------------------------------------------------------------------

; ||||||||||||||| S U B	R O U T	I N E |||||||||||||||||||||||||||||||||||||||


TilemapToVRAM:				; XREF: GM_Sega; GM_Title; SS_BGLoad
		lea	(vdp_data_port).l,a6
		move.l	#$800000,d4

	Tilemap_Line:
		move.l	d0,4(a6)
		move.w	d1,d3

	Tilemap_Cell:
		move.w	(a1)+,(a6)	; write value to namespace
		dbf	d3,Tilemap_Cell
		add.l	d4,d0		; goto next line
		dbf	d2,Tilemap_Line
		rts	
; End of function TilemapToVRAM

;---------------------
; VDP registers array
;---------------------

VDPSetupArray:
	dc.w $8004 ;9-bit palette = 1 (otherwise would be 3-bit), HBlank = 0
	dc.w $8134 ;Genesis display = 1, DMA = 1, VBlank = 1, display = 0
	dc.w $8230 ;Scroll A - $C000
	dc.w $8338 ;Window - $E000
	dc.w $8407 ;Scroll B - $E000
	dc.w $857c ;Sprites - $F800
	dc.w $8600 ;Unused
	dc.w $8700 ;Backdrop color - $00
	dc.w $8800 ;Unused
	dc.w $8900 ;Unused
	dc.w $8A00 ;H Interrupt register
	dc.w $8B00 ;Full screen scroll, no external interrupts
	;dc.w $8C81 ;40 cells display
	dc.w $8C89 ;40 cells display shadow highlight mode enabled
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
Z80DriverEnd:
; Colors 0-63
palDat:
	dc.b 0,0,2,66,0,102,0,108,2,34,12,66,2,106,0,72,0,34,0,68,2,134,0,38,14,238,4,72,0,98,0,174
	dc.b 0,0,0,64,2,130,2,32,0,166,8,64,12,134,2,166,0,100,0,32,0,66,4,204,0,98,0,34,2,100,2,66
	dc.b 0,0,12,64,6,100,6,66,14,194,14,128,10,32,14,160,6,32,2,170,12,136,14,238,14,64,4,32,14,98,2,66
	dc.b 0,0,0,100,0,34,4,68,14,66,2,34,6,68,12,102,14,194,12,66,14,168,14,204,12,168,12,204,6,136,14,238
	even
tileDat:
	;Contains 1120 tiles
	incbin tiles.kos
	even
tilemapDat:
	;40x28 tiles
	incbin tilemap.eni
	even
RomEnd:
