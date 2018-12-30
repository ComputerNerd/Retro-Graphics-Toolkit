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
#include "errorMsg.h"
#include "classpalettebar.h"
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
			break;

		case TMS9918:
			return (getTMS9918subSys() == MODE_3) ? LINEAR : PLANAR_TILE;
			break;

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
					return 1; // This is the numerator. It will later be divided by eight.
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

void Project::setTMS9918subSys(enum TMS9918SubSys sys) {
	TMS9918SubSys oldVal = getTMS9918subSys();
	subSystem &= ~3;
	subSystem |= sys;

	if (oldVal != sys)
		palBar.setSys(false, true);
}

void Project::setSpriteSizeID(unsigned sizeID) {
	sizeID &= 1;

	switch (gameSystem) {
		case NES:
		case masterSystem:
		case gameGear:
			subSystem &= ~(1 << 2);
			subSystem |= sizeID << 2;
			break;

		case TMS9918:
			subSystem &= ~(1 << 11);
			subSystem |= sizeID << 11;
			break;

		default:
			show_default_error
	}

}

unsigned Project::getSpriteSizeID() const {
	switch (gameSystem) {
		case NES:
		case masterSystem:
		case gameGear:
			return (subSystem >> 2) & 1;

		case TMS9918:
			return (subSystem >> 11) & 1;

		default:
			show_default_error
	}

	return 0;
}

void Project::getSpriteSizeMinMax(unsigned & minx, unsigned & miny, unsigned & maxx, unsigned & maxy) {
	switch (gameSystem) {
		case segaGenesis:
			minx = miny = 1;
			maxx = maxy = 4;
			break;

		case NES:
			minx = maxx = 1;

			if (getSpriteSizeID())
				miny = maxy = 2;
			else
				miny = maxy = 1;

			break;

		case masterSystem:
		case gameGear:
		case TMS9918:
			if (getSpriteSizeID())
				minx = miny = maxx = maxy = 2;
			else
				minx = miny = maxx = maxy = 1;

			break;

		default:
			show_default_error

	}
}
