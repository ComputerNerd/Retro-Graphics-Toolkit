;Simple example showing how to display art with data created from Retro Graphics Toolkit
;Code based on Nerdy Nights tutorials
;Header from http://forums.nesdev.com/viewtopic.php?t=6160
;----------------------------------------------------------------
; constants
;----------------------------------------------------------------

PRG_COUNT = 1 ;1 = 16KB, 2 = 32KB
MIRRORING = %0001 ;%0000 = horizontal, %0001 = vertical, %1000 = four-screen

;----------------------------------------------------------------
; variables
;----------------------------------------------------------------

   .enum $0000

   ;NOTE: declare variables using the DSB and DSW directives, like this:

   ;MyVariable0 .dsb 1
   ;MyVariable1 .dsb 3

   .ende

   ;NOTE: you can also split the variable declarations into individual pages, like this:

   ;.enum $0100
   ;.ende

   ;.enum $0200
   ;.ende

;----------------------------------------------------------------
; iNES header
;----------------------------------------------------------------

   .db "NES", $1a ;identification of the iNES header
   .db PRG_COUNT ;number of 16KB PRG-ROM pages
   .db $01 ;number of 8KB CHR-ROM pages
   .db $00|MIRRORING ;mapper 0 and mirroring
   .dsb 9, $00 ;clear the remaining bytes

;----------------------------------------------------------------
; program bank(s)
;----------------------------------------------------------------

   .base $10000-(PRG_COUNT*$4000)

Reset:

   ;NOTE: initialization code goes here
	SEI          ; disable IRQs
	CLD          ; disable decimal mode
	LDX #$40
	STX $4017    ; disable APU frame IRQ
	LDX #$FF
	TXS          ; Set up stack
	INX          ; now X = 0
	STX $2000    ; disable NMI
	STX $2001    ; disable rendering
	STX $4010    ; disable DMC IRQs

vblankwait1:       ; First wait for vblank to make sure PPU is ready
	BIT $2002
	BPL vblankwait1

clrmem:
	LDA #$00
	STA $0000, x
	STA $0100, x
	STA $0200, x
	STA $0400, x
	STA $0500, x
	STA $0600, x
	STA $0700, x
	LDA #$FE
	STA $0300, x
	INX
	BNE clrmem
   
vblankwait2:      ; Second wait for vblank, PPU is ready after this
	BIT $2002
	BPL vblankwait2
	LDA $2002    ; read PPU status to reset the high/low latch to high
	LDA #$3F
	STA $2006    ; write the high byte of $3F10 address
	LDA 0
	STA $2006    ; write the low byte of $3F10 address

	LDX #$00		; start out at 0
LoadPalettesLoop:
	LDA PaletteData, x	; load data from address (PaletteData + the value in x)
	STA $2007		; write to PPU
	INX			; X = X + 1
	CPX #16			; Compare X to hex $20, decimal 32
	BNE LoadPalettesLoop	; Branch to LoadPalettesLoop if compare was Not Equal to zero


	LDA $2002    ; read PPU status to reset the high/low latch to high
	LDA #$20
	STA $2006    ; write the high byte of $3F10 address
	LDA 0
	STA $2006    ; write the low byte of $3F10 address

	;Now we will load the tilemap (name table)
	;Unroll to simplify code
	LDX 0
Loop1:
	LDA tilemap,x
	STA $2007		; write to PPU
	INX
	BNE Loop1

	LDX 0
Loop2:
	LDA (tilemap+256),x
	STA $2007		; write to PPU
	INX
	BNE Loop2

	LDX 0
Loop3:
	LDA (tilemap+512),x
	STA $2007		; write to PPU
	INX
	BNE Loop3

	LDX 0
Loop4:
	LDA (tilemap+768),x
	STA $2007		; write to PPU
	INX
	CPX #192
	BNE Loop4

	LDA $2002    ; read PPU status to reset the high/low latch to high
	LDA #$23
	STA $2006    ; write the high byte of $3F10 address
	LDA #$C0
	STA $2006    ; write the low byte of $3F10 address


	;Now copy attributes
	LDX 0
attrCpy:
	LDA attr,x
	STA $2007		; write to PPU
	INX
	CPX #64
	BNE attrCpy

  LDA #%10000000   ; enable NMI, sprites from Pattern Table 0, background from Pattern Table 0
  STA $2000

  LDA #%00011110   ; enable sprites, enable background, no clipping on left side
  STA $2001

Forever:
  JMP Forever     ;jump back to Forever, infinite loop
 
PaletteData:
palDat:
	dc.b 15,56,40,8,15,8,24,40,15,24,40,39,15,40,56,48

tilemap:
; Width 32 Height 30 Uncompressed
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,1,2,3,4,0,0,0,0,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,7,8,9,10,0,11,12,13,14,15,16,17,18,19,20,21,22,23,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,7,24,25,26,0,27,28,29,30,31,0,32,33,34,35,36,37,38,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,7,39,40,41,42,43,44,45,46,47,48,32,49,0,50,51,52,53,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,54,55,0,56,57,58,59,60,0,61,62,63,64,0,65,66,67,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,68,69,70,0,0,0,0,0,0,0,0,0,0,0,71,72,0,0,71,72,0,0,0,0,0,0,0,0,0
	dc.b 0,0,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,0,94,95,237,96,82,97,0,0
	dc.b 0,0,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,88,113,114,115,116,117,118,119,120,121,122,123,124,0,0
	dc.b 0,0,125,126,127,128,129,130,238,131,132,133,134,7,135,0,136,137,89,0,31,0,89,138,139,0,0,140,141,142,0,0
	dc.b 0,0,239,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,0,159,157,158,160,161,162,163,164,165,166,0,0
	dc.b 0,0,0,0,0,0,0,0,239,0,0,0,0,7,167,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,168,169,169,169,170,0,0,0,0,0,0,171,172,173,174,0,0,175,176,177,178,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,240,179,0,180,181,182,0,183,184,185,7,186,7,187,188,189,190,191,192,193,194,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,195,0,196,240,197,198,199,200,201,7,186,7,202,203,0,204,205,204,206,207,0,0,0,0,0
	dc.b 0,0,0,0,0,0,88,208,0,209,210,211,212,213,214,215,7,186,7,216,217,218,204,205,219,220,221,0,0,0,0,0
	dc.b 0,0,0,0,0,0,222,223,0,224,225,226,0,227,228,229,149,230,149,231,232,233,234,235,200,236,241,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	dc.b 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
attr:
	dc.b 0,0,0,0,0,0,0,0,0,192,240,192,192,240,0,0,0,136,255,255,63,207,3,0,192,255,240,243,204,252,255,48
	dc.b 12,243,63,207,12,204,63,3,0,15,255,255,238,255,255,0,0,0,0,0,0,0,0,0,0,128,64,0,128,64,192,64
	

NMI:

	;NOTE: NMI code goes here
	RTI
IRQ:

	;NOTE: IRQ code goes here
	RTI
;----------------------------------------------------------------
; interrupt vectors
;----------------------------------------------------------------

   .org $fffa

   .dw NMI
   .dw Reset
   .dw IRQ

;----------------------------------------------------------------
; CHR-ROM bank
;----------------------------------------------------------------
   .incbin "logo.bin"
   .align 8192
