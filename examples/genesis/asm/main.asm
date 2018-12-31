; This example is based on: https://github.com/BigEvilCorporation/megadrive_samples
;==============================================================
; SEGA MEGA DRIVE/GENESIS - DEMO 1 - HELLO WORLD SAMPLE
;==============================================================
; by Big Evil Corporation
;==============================================================

; A small, discreet, and complete Hello World sample, with
; a healthy dose of comments and explanations for beginners.
; Runs on genuine hardware, and (hopefully) all emulators.
;
; To assemble this program with ASM68K.EXE:
;    ASM68K.EXE /p hello.asm,hello.bin,hello.map,hello.lst
;
; To assemble this program with SNASM68K.EXE:
;    SNASM68K.EXE /p hello.asm,hello.map,hello.lst,hello.bin
;
; hello.asm = this source file
; hello.bin = the binary file, fire this up in your emulator!
; hello.lst = listing file, shows assembled addresses alongside your source code, open in a text editor
; hello.map = symbol map file for linking (unused)

;==============================================================

	CPU 68000
	padding off

; A label defining the start of ROM so we can compute the total size.
ROM_Start:

;==============================================================
; CPU VECTOR TABLE
;==============================================================
; A table of addresses that the CPU needs to know about -
; things like stack address, "main()" function address,
; vertical/horizontal interrupt addresses, etc.
;==============================================================
; For any interrupts we don't want to handle in this demo,
; we specify INT_Null (an interrupt at the bottom of the
; file that doesn't do anything).
;==============================================================
; This must be the very first thing in the ROM, since the CPU
; reads it from $0000 on bootup.
;==============================================================
	dc.l   $00FFE000			; Initial stack pointer value
	dc.l   CPU_EntryPoint		; Start of program
	dc.l   CPU_Exception 		; Bus error
	dc.l   CPU_Exception 		; Address error
	dc.l   CPU_Exception 		; Illegal instruction
	dc.l   CPU_Exception 		; Division by zero
	dc.l   CPU_Exception 		; CHK CPU_Exception
	dc.l   CPU_Exception 		; TRAPV CPU_Exception
	dc.l   CPU_Exception 		; Privilege violation
	dc.l   INT_Null				; TRACE exception
	dc.l   INT_Null				; Line-A emulator
	dc.l   INT_Null				; Line-F emulator
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Spurious exception
	dc.l   INT_Null				; IRQ level 1
	dc.l   INT_Null				; IRQ level 2
	dc.l   INT_Null				; IRQ level 3
	dc.l   INT_HInterrupt		; IRQ level 4 (horizontal retrace interrupt)
	dc.l   INT_Null  			; IRQ level 5
	dc.l   INT_VInterrupt		; IRQ level 6 (vertical retrace interrupt)
	dc.l   INT_Null				; IRQ level 7
	dc.l   INT_Null				; TRAP #00 exception
	dc.l   INT_Null				; TRAP #01 exception
	dc.l   INT_Null				; TRAP #02 exception
	dc.l   INT_Null				; TRAP #03 exception
	dc.l   INT_Null				; TRAP #04 exception
	dc.l   INT_Null				; TRAP #05 exception
	dc.l   INT_Null				; TRAP #06 exception
	dc.l   INT_Null				; TRAP #07 exception
	dc.l   INT_Null				; TRAP #08 exception
	dc.l   INT_Null				; TRAP #09 exception
	dc.l   INT_Null				; TRAP #10 exception
	dc.l   INT_Null				; TRAP #11 exception
	dc.l   INT_Null				; TRAP #12 exception
	dc.l   INT_Null				; TRAP #13 exception
	dc.l   INT_Null				; TRAP #14 exception
	dc.l   INT_Null				; TRAP #15 exception
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	dc.l   INT_Null				; Unused (reserved)
	
;==============================================================
; SEGA MEGA DRIVE ROM HEADER
;==============================================================
; A structure that specifies some metadata about the ROM, like
; its name, author, version number, release date, region,
; and any special peripherals used.
; Note that the Mega Drive console itself doesn't read any of
; this, it's more a convenience for the programmer, but
; most emulators will read the title and region.
;==============================================================
; If your emulator doesn't show the correct ROM name, then this
; table is in the wrong place or in the wrong format.
;==============================================================
	dc.b "SEGA MEGA DRIVE "                                 ; Console name
	dc.b "BIGEVILCORP.    "                                 ; Copyright holder and release date
	dc.b "HELLO WORLD                                     " ; Domestic name
	dc.b "HELLO WORLD                                     " ; International name
	dc.b "GM XXXXXXXX-XX"                                   ; Version number
	dc.w $0000                                             ; Checksum
	dc.b "J               "                                 ; I/O support
	dc.l ROM_Start                                          ; Start address of ROM
	dc.l ROM_End-1                                          ; End address of ROM
	dc.l $00FF0000                                         ; Start address of RAM
	dc.l $00FF0000+$0000FFFF                              ; End address of RAM
	dc.l $00000000                                         ; SRAM enabled
	dc.l $00000000                                         ; Unused
	dc.l $00000000                                         ; Start address of SRAM
	dc.l $00000000                                         ; End address of SRAM
	dc.l $00000000                                         ; Unused
	dc.l $00000000                                         ; Unused
	dc.b "                                        "         ; Notes (unused)
	dc.b "JUE             "                                 ; Country codes
	
;==============================================================
; INITIAL VDP REGISTER VALUES
;==============================================================
; 24 register values to be copied to the VDP during initialisation.
; These specify things like initial width/height of the planes,
; addresses within VRAM to find scroll/sprite data, the
; background palette/colour index, whether or not the display
; is on, and clears initial values for things like DMA.
;==============================================================
VDPRegisters:
	dc.b $14 ; $00:  H interrupt on, palettes on
	dc.b $74 ; $01:  V interrupt on, display on, DMA on, Genesis mode on
	dc.b $30 ; $02:  Pattern table for Scroll Plane A at VRAM $C000 (bits 3-5 = bits 13-15)
	dc.b $00 ; $03:  Pattern table for Window Plane at VRAM $0000 (disabled) (bits 1-5 = bits 11-15)
	dc.b $07 ; $04:  Pattern table for Scroll Plane B at VRAM $E000 (bits 0-2 = bits 11-15)
	dc.b $78 ; $05:  Sprite table at VRAM $F000 (bits 0-6 = bits 9-15)
	dc.b $00 ; $06:  Unused
	dc.b $00 ; $07:  Background colour: bits 0-3 = colour, bits 4-5 = palette
	dc.b $00 ; $08:  Unused
	dc.b $00 ; $09:  Unused
	dc.b $08 ; $0A: Frequency of Horiz. interrupt in Rasters (number of lines travelled by the beam)
	dc.b $00 ; $0B: External interrupts off, V scroll fullscreen, H scroll fullscreen
	dc.b $89 ; $0C: Shadows and highlights on, interlace off, H40 mode (320 x 224 screen res)
	dc.b $3F ; $0D: Horiz. scroll table at VRAM $FC00 (bits 0-5)
	dc.b $00 ; $0E: Unused
	dc.b $02 ; $0F: Autoincrement 2 bytes
	dc.b $01 ; $10: Scroll plane size: 64x32 tiles
	dc.b $00 ; $11: Window Plane X pos 0 left (pos in bits 0-4, left/right in bit 7)
	dc.b $00 ; $12: Window Plane Y pos 0 up (pos in bits 0-4, up/down in bit 7)
	dc.b $FF ; $13: DMA length lo byte
	dc.b $FF ; $14: DMA length hi byte
	dc.b $00 ; $15: DMA source address lo byte
	dc.b $00 ; $16: DMA source address mid byte
	dc.b $80 ; $17: DMA source address hi byte, memory-to-VRAM mode (bits 6-7)
	
	align 2
	
;==============================================================
; CONSTANTS
;==============================================================
; Defines names for commonly used values and addresses to make
; the code more readable.
;==============================================================
	
; VDP port addresses
vdp_control				equ $00C00004
vdp_data				equ $00C00000

; VDP commands
vdp_cmd_vram_write		equ $40000000
vdp_cmd_cram_write		equ $C0000000

; VDP memory addresses
; according to VDP registers $2 and $4 (see table above)
vram_addr_tiles			equ $0000
vram_addr_plane_a		equ $C000
vram_addr_plane_b		equ $E000

; Screen width and height (in pixels)
vdp_screen_width		equ $0140
vdp_screen_height		equ $00F0

; The plane width and height (in tiles)
; according to VDP register $10 (see table above)
vdp_plane_width			equ $40
vdp_plane_height		equ $20

; Hardware version address
hardware_ver_address	equ $00A10001

; TMSS
tmss_address			equ $00A14000
tmss_signature			equ 'SEGA'

; The size of a word and longword
size_word				equ 2
size_long				equ 4

; The size of one palette (in bytes, words, and longwords)
size_palette_b			equ $80
size_palette_w			equ size_palette_b/size_word
size_palette_l			equ size_palette_b/size_long

; The size of one graphics tile (in bytes, words, and longwords)
size_tile_b				equ $20
size_tile_w				equ size_tile_b/size_word
size_tile_l				equ size_tile_b/size_long


;==============================================================
; VRAM WRITE MACROS
;==============================================================
; Some utility macros to help generate addresses and commands for
; writing data to video memory, since they're tricky (and
; error prone) to calculate manually.
; They were taken from the Sonic 1 disassembly.
;==============================================================

vdp_data_port:		equ $C00000
vdp_control_port:	equ $C00004
vdp_counter:		equ $C00008
v_vdp_buffer2:	= $FFFFF640	; VDP instruction buffer (2 bytes)
; ---------------------------------------------------------------------------
; DMA copy data from 68K (ROM/RAM) to the VRAM
; input: source, length, destination
; ---------------------------------------------------------------------------

writeVRAM:	macro source, length, destination
		lea	(vdp_control_port).l,a5
		move.l	#$94000000+(((length>>1)&$FF00)<<8)+$9300+((length>>1)&$FF),(a5)
		move.l	#$96000000+(((source>>1)&$FF00)<<8)+$9500+((source>>1)&$FF),(a5)
		move.w	#$9700+((((source>>1)&$FF0000)>>16)&$7F),(a5)
		move.w	#$4000+(destination&$3FFF),(a5)
		move.w	#$80+((destination&$C000)>>14),(v_vdp_buffer2).w
		move.w	(v_vdp_buffer2).w,(a5)
		endm

; ---------------------------------------------------------------------------
; DMA copy data from 68K (ROM/RAM) to the CRAM
; input: source, length, destination
; ---------------------------------------------------------------------------

writeCRAM:	macro source, length, destination
		lea	(vdp_control_port).l,a5
		move.l	#$94000000+(((length>>1)&$FF00)<<8)+$9300+((length>>1)&$FF),(a5)
		move.l	#$96000000+(((source>>1)&$FF00)<<8)+$9500+((source>>1)&$FF),(a5)
		move.w	#$9700+((((source>>1)&$FF0000)>>16)&$7F),(a5)
		move.w	#$C000+(destination&$3FFF),(a5)
		move.w	#$80+((destination&$C000)>>14),(v_vdp_buffer2).w
		move.w	(v_vdp_buffer2).w,(a5)
		endm
	
;==============================================================
; PALETTE
;==============================================================
; A single colour palette (64 colours) we'll be using to draw the image.
; Colour #0 is always transparent, no matter what colour value
; you specify.
;==============================================================
; Each colour is in binary format 0000 BBB0 GGG0 RRR0,
; so $0000 is black, $0EEE is white (NOT $0FFF, since the
; bottom bit is discarded), $000E is red, $00E0 is green, and
; $0E00 is blue.
;==============================================================
Palette:
	dc.w $0,$242,$66,$6C,$222,$C42,$26A,$48,$22,$44,$286,$26,$EEE,$448,$62,$AE
	dc.w $0,$40,$282,$220,$A6,$840,$C86,$2A6,$64,$20,$42,$4CC,$62,$22,$264,$242
	dc.w $0,$C40,$664,$642,$EC2,$E80,$A20,$EA0,$620,$2AA,$C88,$EEE,$E40,$420,$E62,$242
	dc.w $0,$64,$22,$444,$E42,$222,$644,$C66,$EC2,$C42,$EA8,$ECC,$CA8,$CCC,$688,$EEE

;==============================================================
; CODE ENTRY POINT
;==============================================================
; The "main()" function. Your code starts here. Once the CPU
; has finished initialising, it will jump to this entry point
; (specified in the vector table at the top of the file).
;==============================================================
CPU_EntryPoint:

	;==============================================================
	; Initialise the Mega Drive
	;==============================================================

	; Write the TMSS signature (if a model 1+ Mega Drive)
	jsr    VDP_WriteTMSS

	; Load the initial VDP registers
	jsr    VDP_LoadRegisters

	;==============================================================
	; Initialise status register and set interrupt level.
	; This begins firing vertical and horizontal interrupts.
	;==============================================================
	move.w #$2300, sr
	
	;==============================================================
	; Write the palette to CRAM (colour memory)
	;==============================================================
	
	; Setup the VDP to write to CRAM address $0000 (first palette)
	writeCRAM	Palette, 64 * 2, 0
	;==============================================================
	; Write the tiles to VRAM
	;==============================================================
	
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
	; Finished!
	
	;==============================================================
	; Loop forever
	;==============================================================
	; This loops forever, effectively ending our code. The VDP will
	; still continue to run (and fire vertical/horizontal interrupts)
	; of its own accord, so it will continue to render our Hello World
	; even though the CPU is stuck in this loop.
	.InfiniteLp:
	bra.s .InfiniteLp

; From the Sonic 1 disassembly.
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
;==============================================================
; INTERRUPT ROUTINES
;==============================================================
; The interrupt routines, as specified in the vector table at
; the top of the file.
; Note that we use RTE to return from an interrupt, not
; RTS like a subroutine.
;==============================================================

; Vertical interrupt - run once per frame
INT_VInterrupt:
	; Doesn't do anything in this demo
	rte

; Horizontal interrupt - run once per N scanlines (N = specified in VDP register $A)
INT_HInterrupt:
	; Doesn't do anything in this demo
	rte

; NULL interrupt - for interrupts we don't care about
INT_Null:
	rte

; Exception interrupt - called if an error has occured
CPU_Exception:
	; Just halt the CPU if an error occurred. Later on, you may want to write
	; an exception handler to draw the current state of the machine to screen
	; (registers, stack, error type, etc) to help debug the problem.
	stop   #$2700
	rte
	
;==============================================================
; UTILITY FUNCTIONS
;==============================================================
; Subroutines to initialise the TMSS, and load all VDP registers
;==============================================================

VDP_WriteTMSS:

	; The TMSS (Trademark Security System) locks up the VDP if we don't
	; write the string 'SEGA' to a special address. This was to discourage
	; unlicensed developers, since doing this displays the "LICENSED BY SEGA
	; ENTERPRISES LTD" message to screen (on Mega Drive models 1 and higher).
	;
	; First, we need to check if we're running on a model 1+, then write
	; 'SEGA' to hardware address $A14000.

	move.b hardware_ver_address, d0			; Move Megadrive hardware version to d0
	andi.b #$0F, d0						; The version is stored in last four bits, so mask it with 0F
	beq    .SkipTMSS						; If version is equal to 0, skip TMSS signature
	move.l #tmss_signature, tmss_address	; Move the string "SEGA" to $A14000
	.SkipTMSS:

	; Check VDP
	move.w vdp_control, d0					; Read VDP status register (hangs if no access)
	
	rts

VDP_LoadRegisters:

	; To initialise the VDP, we write all of its initial register values from
	; the table at the top of the file, using a loop.
	;
	; To write a register, we write a word to the control port.
	; The top bit must be set to 1 (so $8000), bits 8-12 specify the register
	; number to write to, and the bottom byte is the value to set.
	;
	; In binary:
	;   100X XXXX YYYY YYYY
	;   X = register number
	;   Y = value to write

	; Set VDP registers
	lea    VDPRegisters, a0		; Load address of register table into a0
	move.w #$18-1, d0			; 24 registers to write (-1 for loop counter)
	move.w #$8000, d1			; 'Set register 0' command to d1

	.CopyRegLp:
	move.b (a0)+, d1			; Move register value from table to lower byte of d1 (and post-increment the table address for next time)
	move.w d1, vdp_control		; Write command and value to VDP control port
	addi.w #$0100, d1			; Increment register #
	dbra   d0, .CopyRegLp		; Decrement d0, and jump back to top of loop if d0 is still >= 0
	
	rts

	include "Enigma.asm"
	include "Kosinski.asm"

; A label defining the end of ROM so we can compute the total size.

tileDat:
	;Contains 1120 tiles
	binclude tiles.kos
	align 2
tilemapDat:
	;40x28 tiles
	binclude tilemap.eni
	align 2
ROM_End:
