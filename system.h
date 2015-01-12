/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
*/
#pragma once
#include <stdint.h>
#define MAX_ROWS_PALETTE 4//TODO refractor to allow "unlimited" rows
//System declarations
enum gameSystemEnum{segaGenesis,NES,masterSystem,gameGear,TMS9918,SNES,frameBufferPal,frameBuffer};
/* Subsystem declarations
 * Subsystem as the name implies depends on which system is selected
 * These are not compatible when switching systems
 * For the Sega Genesis, Master System, Game Gear and TMS9918 bits 1-0 contain bit depth 0 means 1 bit
 * For the Sega Genesis bit 2 sets if shadow highlight is enabled and bit 3 sets if highlight should be displayed instead of shadow
 * For the NES bit 1 contains bit depth 1 if 2 bit 0 if 1 bit
 * For palette framebuffer bits 2-0 contain bit depth add 1 to get actual just like the others
 * Bits 6-2 contain screen depth for example if set to 15 (16 remember always +1) that would mean palette is based on rgb565 colors
 * For framebuffer bits 4-0 contain bit depth again remember to add one to get actual bit depth
 * Valid values are
 * 0 - 1 bit black and white
 * 14 - rgb555
 * 15 - rgb565
 * 23 - rgb888
 * 31 - rgba 8888
 * Bit 4 specifies order this depends on bit depth
 * When 15 or 16`specifies byte swapping
 * When 24 bit or 32 stores as bgr or rgb
 * if bit depth is 32 bit then bit 5 is used to determine if alpha value should be before or after rgb/bgr
 * */
#define sgSHshift 2
#define sgSHmask (3<<sgSHshift)
#define sgSon (1<<2)
#define sgHon (2<<2)
#define NES2x2 1//Note for version 4 or earlier projects and hence in earlier versions of Retro Graphics Toolkit bit 0 was inverted
#define NES1x1 0
#define NES2bit 2
#define NES1bit 0
#define NESempShift 2
#define NESempMask 3 //This mask should be used after bit shifting
#define NESempShiftAlt 4
void setBitdepthcurSys(unsigned bd);
int getBitdepthcurSysraw(void);
static inline int getBitdepthcurSys(void){
	return getBitdepthcurSysraw()+1;
}
int fixedSpirtePalRow(uint32_t gameSystem);
bool isPlanarTiles(uint32_t gameSystem);
