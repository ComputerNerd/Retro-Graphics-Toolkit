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
	Copyright Sega16 (or whatever you wish to call me) (2012-2018)
*/
#include "project.h"
#include "system.h"
static void setbdmask(unsigned bd, unsigned mask, struct Project*p) {
	if (bd > mask)
		bd = mask;

	p->subSystem &= ~mask;
	p->subSystem |= bd;
}
void Project::setBitdepthSys(unsigned bd) {
	--bd;

	switch (gameSystem) {
		case segaGenesis:
		case masterSystem:
		case gameGear:
			setbdmask(bd, 3, this);
			break;

		case NES:
			if (bd)
				subSystem |= 2;
			else
				subSystem &= ~2;

			break;

		case TMS9918:
			//Do nothing
			break;

		case frameBufferPal:
			setbdmask(bd, 7, this);
			break;

		default:
			show_default_error
	}

	if (containsData(pjHaveTiles)) {
		tileC->tileSize = tileC->bitsPerPlaneRow() * tileC->height() * getBitdepthSys() / 8;
		tileC->resizeAmt();
	}
}
int Project::getBitdepthSysraw(void)const {
	switch (gameSystem) {
		case segaGenesis:
		case masterSystem:
		case gameGear:
			return (subSystem & 3);
			break;

		case NES:
			return ((subSystem >> 1) & 1);
			break;

		case frameBufferPal:
			return (subSystem & 7);
			break;

		case TMS9918:
			return getTMS9918subSys() == MODE_3 ? 3 : 0;
			break;

		default:
			show_default_error
			return 0;
	}
}
int Project::fixedSpirtePalRow(void) {
	switch (gameSystem) {
		case masterSystem:
		case gameGear:
			return 1;

		default:
			return -1;
	}
}
enum tileType Project::getTileType(void) {
	switch (gameSystem) {
		case NES:
					return PLANAR_TILE;
			break;

		case masterSystem:
		case gameGear:
			return PLANAR_LINE;

		default:
			return LINEAR;
	}
}
bool Project::isFixedPalette(void) {
	switch (gameSystem) {
		case TMS9918:
			return true;
			break;

		default:
			return false;
	}
}

bool Project::hasExtAttrs(void) {
	switch (gameSystem) {
		case TMS9918:
			return getTMS9918subSys() != MODE_3;
			break;

		default:
			return false;
	}
}

bool Project::supportsFlippedTiles(void) {
	switch (gameSystem) {
		case TMS9918:
		case NES:
			return false;
			break;

		default:
			return true;
	}
}

unsigned Project::extAttrTilesPerByte(void) {
	switch (gameSystem) {
		case TMS9918:
		{	enum TMS9918SubSys subSys = getTMS9918subSys();

			switch (subSys) {
				case MODE_0:
				case MODE_3:
					return 0;
					break;

				case MODE_1:
					return 8;
					break;

				case MODE_2:
					return 1;
					break;

				default:
					show_default_error
			}
		}
		break;

		default:
			return 0;
	}
}

unsigned Project::extAttrBytesPerTile(void) {
	switch (gameSystem) {
		case TMS9918:
		{	enum TMS9918SubSys subSys = getTMS9918subSys();

			switch (subSys) {
				case MODE_0:
				case MODE_3:
					return 0;
					break;

				case MODE_1:
					return 1; // This is the numberator. It will later be divided by eight.
					break;

				case MODE_2:
					return 8; // Eight bytes per tile.
					break;

				default:
					show_default_error
			}
		}
		break;

		default:
			return 0;
	}
}

unsigned Project::extAttrFixedSize(void) {
	switch (gameSystem) {
		case TMS9918:
		{	enum TMS9918SubSys subSys = getTMS9918subSys();

			switch (subSys) {
				case MODE_0:
					return 1; // Fixed byte to set the background and foreground color.
					break;

				case MODE_3:
				case MODE_2:
				case MODE_1:
					return 0;
					break;

				default:
					show_default_error
			}
		}
		break;

		default:
			return 0;
	}
}
