/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#include "errorMsg.h"
#include "color_convert.h"
#include "palette.h"
#include "project.h"
unsigned palTypeGen;
typedef std::pair<double, int> HSLpair;
static bool comparatorHSL(const HSLpair& l, const HSLpair& r) {
	return l.first < r.first;
}
void sortBy(unsigned type, bool perRow) {
	unsigned totalCol[2] = {currentProject->pal->colorCnt, currentProject->pal->colorCntalt};
	unsigned off = 0;
	unsigned offe = 0;
	unsigned offr = 0;

	for (unsigned p = 0; p < 2; ++p) {
		HSLpair* MapHSL = new HSLpair[totalCol[p]];

		for (unsigned x = 0; x < totalCol[p] * 3; x += 3) {
			double h, l, s;
			rgbToHsl255(currentProject->pal->rgbPal[x + offr], currentProject->pal->rgbPal[x + 1 + offr], currentProject->pal->rgbPal[x + 2 + offr], &h, &s, &l);
			MapHSL[x / 3].first = pickIt(h, s, l, type);
			MapHSL[x / 3].second = x / 3;
		}

		if (perRow) {
			for (unsigned i = 0; i < (p ? currentProject->pal->rowCntPalalt : currentProject->pal->rowCntPal); ++i)
				std::sort(MapHSL + (currentProject->pal->perRow * i), MapHSL + (currentProject->pal->perRow * (i + 1)), comparatorHSL);
		} else
			std::sort(MapHSL, MapHSL + (totalCol[p]), comparatorHSL);

		uint8_t* newPal = (uint8_t*)alloca((totalCol[p]) * currentProject->pal->esize);
		uint8_t* newPalRgb = (uint8_t*)alloca(totalCol[p] * 3);
		uint8_t* newPalType = (uint8_t*)alloca(totalCol[p]);

		for (unsigned x = 0; x < totalCol[p]; ++x) {
			memcpy(newPal + (x * currentProject->pal->esize), currentProject->pal->palDat + (MapHSL[x].second * currentProject->pal->esize) + offe, currentProject->pal->esize);
			memcpy(newPalRgb + (x * 3), currentProject->pal->rgbPal + (MapHSL[x].second * 3) + offr, 3);
			newPalType[x] = currentProject->pal->palType[MapHSL[x].second + off];
		}

		memcpy(currentProject->pal->palDat + offe, newPal, totalCol[p]*currentProject->pal->esize);
		memcpy(currentProject->pal->rgbPal + offr, newPalRgb, totalCol[p] * 3);
		memcpy(currentProject->pal->palType + off, newPalType, totalCol[p]);
		delete[] MapHSL;
		off = totalCol[p];
		offe = off * currentProject->pal->esize;
		offr = off * 3;

		if (!currentProject->pal->haveAlt)
			break;
	}
}
const uint8_t palTabGameGear[] = {0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 236, 255};
const uint8_t palTabMasterSystem[] = {0, 85, 170, 255}; //From http://segaretro.org/Palette
const uint8_t*palTab = palTabGenReal;
const uint8_t*palTabPtr[] = {palTabGenReal, palTabGen255div7, palTabGen36, palTabGen32};
const uint8_t palTabGenReal[] =   {0, 49, 87, 119, 146, 174, 206, 255, 0, 27, 49, 71, 87, 103, 119, 130, 130, 146, 157, 174, 190, 206, 228, 255}; //from http://gendev.spritesmind.net/forum/viewtopic.php?t=1389
const uint8_t palTabGen255div7[] = {0, 36, 73, 109, 146, 182, 219, 255, 0, 18, 36, 55, 73, 91, 109, 128, 128, 146, 164, 182, 200, 219, 237, 255};
const uint8_t palTabGen36[] =     {0, 36, 72, 108, 144, 180, 216, 252, 0, 18, 36, 54, 72, 90, 108, 126, 126, 144, 162, 180, 198, 216, 234, 252};
const uint8_t palTabGen32[] =     {0, 32, 64, 96, 128, 160, 192, 224, 0, 16, 32, 48, 64, 80, 96, 112, 112, 128, 144, 160, 176, 192, 208, 224};
void set_palette_type_force(unsigned type) {
	palTypeGen = type;

	//now reconvert all the colors
	for (unsigned pal = 0; pal < 64; ++pal)
		currentProject->pal->updateRGBindex(pal);
}
void set_palette_type(void) {
	if (currentProject->subSystem & sgSon) {
		if (currentProject->subSystem & sgHon)
			set_palette_type_force(16);
		else
			set_palette_type_force(8);
	} else
		set_palette_type_force(0);
}
