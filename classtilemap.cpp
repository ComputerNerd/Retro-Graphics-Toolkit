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
#include <FL/fl_ask.H>

#include <memory>

#include "class_global.h"
#include "macros.h"
#include "project.h"
#include "callback_tilemap.h"
#include "filemisc.h"
#include "classtilemap.h"
#include "compressionWrapper.h"
#include "quant.h"
#include "dither.h"
#include "palette.h"
#include "gui.h"
#include "filereader.h"
#include "errorMsg.h"
tileMap::tileMap(Project*prj)noexcept: tileMap(1, 1, prj) {}
tileMap::tileMap(uint32_t w, uint32_t h, Project*prj)noexcept {
	this->prj = prj;
	amt = 1;
	mapSizeW = w;
	mapSizeHA = mapSizeH = h;
	isBlock = false;
	tileMapDat = (uint8_t*)calloc(w * h, TileMapSizePerEntry);
	offset = 0;
}
tileMap::tileMap(const tileMap&other, Project*prj)noexcept {
	if (this != &other) {
		planeName = other.planeName;
		this->prj = prj;
		mapSizeW = other.mapSizeW;
		mapSizeH = other.mapSizeH;
		mapSizeHA = other.mapSizeHA;
		isBlock = other.isBlock;
		offset = other.offset;

		if (isBlock) {
			amt = other.amt;
			mapSizeHA = mapSizeH * amt;
		} else {
			amt = 1;
			mapSizeHA = mapSizeH;
		}

		tileMapDat = (uint8_t*)malloc(mapSizeW * mapSizeHA * TileMapSizePerEntry);
		memcpy(tileMapDat, other.tileMapDat, mapSizeW * mapSizeHA * TileMapSizePerEntry);
	}
}
tileMap::tileMap(const tileMap&other)noexcept {
	tileMap(other, other.prj);
}
tileMap::tileMap(tileMap&& other)noexcept {
	if (this != &other) {
		planeName = other.planeName;
		prj = other.prj;
		mapSizeW = other.mapSizeW;
		mapSizeH = other.mapSizeH;
		mapSizeHA = other.mapSizeHA;
		isBlock = other.isBlock;
		offset = other.offset;
		amt = other.amt;
		tileMapDat = other.tileMapDat;
		other.tileMapDat = 0;
	}
}
tileMap& tileMap::operator=(tileMap&& other)noexcept {
	tileMap((tileMap&&)other);
	return *this;
}
tileMap& tileMap::operator=(const tileMap& other)noexcept {
	tileMap(other, other.prj);
	return *this;
}
tileMap::~tileMap()noexcept {
	free(tileMapDat);
	tileMapDat = nullptr;
}
void tileMap::ditherAsImage(bool entire) {
	uint8_t*image;
	uint32_t w, h;
	w = mapSizeW;
	h = mapSizeHA;
	w *= prj->tileC->width();
	h *= prj->tileC->height();
	image = (uint8_t *)malloc(w * h * 4);

	if (!image)
		show_malloc_error(w * h * 4)
		if (entire) {
			truecolor_to_image(image, -1);
			ditherImage(image, w, h, true, true);
			void*indexPtr = ditherImage(image, w, h, true, false, false, 0, false, 0, false, true);
			truecolorimageToTiles((uint8_t*)indexPtr, -1, false, false, true, true);
			free(indexPtr);
		} else {
			for (unsigned row = 0; row < prj->pal->rowCntPal; ++row) {
				printf("Row: %u\n", row);
				truecolor_to_image(image, row);
				ditherImage(image, w, h, true, true);
				void*indexPtr = ditherImage(image, w, h, true, false, false, 0, false, 0, false, true);
				//convert back to tiles
				truecolorimageToTiles((uint8_t*)indexPtr, row, false, false, true, true);
				free(indexPtr);
			}
		}

	free(image);
}
void tileMap::allRowSet(unsigned row) {
	unsigned x, y;

	for (y = 0; y < mapSizeHA; ++y) {
		for (x = 0; x < mapSizeW; ++x)
			set_pal_row(x, y, row);
	}
}
static void sumTile(uint8_t*tilePtr, uint32_t*sums, Project*prj) {
	uint32_t sum[3];//In hopes that the compiler is smart enough to keep these in registers
	memset(sum, 0, sizeof(sum));

	for (unsigned j = 0; j < prj->tileC->tcSize; ++j) {
		if (tilePtr[3]) {
			sum[0] += tilePtr[0];
			sum[1] += tilePtr[1];
			sum[2] += tilePtr[2];
		}

		tilePtr += 4;
	}

	sums[0] = sum[0];
	sums[1] = sum[1];
	sums[2] = sum[2];
}
bool tileMap::pickTileRowQuantChoice(unsigned rows) {
	unsigned w = mapSizeW, h = mapSizeHA;
	unsigned char userpal[3][256];

	if ((prj->gameSystem == NES) && (prj->subSystem & NES2x2)) {
		w /= 2;
		h /= 2;
	}

	uint8_t*imgin = (uint8_t*)malloc(w * h * 3);

	if (!imgin) {
		show_malloc_error(w * h * 3)
		return false;
	}

	uint8_t*imgout = (uint8_t*)malloc(w * h);

	if (!imgout) {
		show_malloc_error(w * h)
		return false;
	}

	uint32_t sums[3];
	uint8_t*imgptr = imgin;

	if ((prj->gameSystem == NES) && (prj->subSystem & NES2x2)) {
		for (unsigned y = 0; y < mapSizeHA; y += 2) {
			for (unsigned x = 0; x < mapSizeW; x += 2) {
				memset(sums, 0, sizeof(sums));

				for (unsigned i = 0; i < prj->pal->rowCntPal; ++i) {
					uint32_t sumtmp[3];
					sumTile(prj->tileC->truetDat.data() + (get_tile(x + (i & 1), y + (i / 2))*prj->tileC->tcSize), sumtmp, prj);
					sums[0] += sumtmp[0];
					sums[1] += sumtmp[1];
					sums[2] += sumtmp[2];
				}

				*imgptr++ = sums[0] / 256;
				*imgptr++ = sums[1] / 256;
				*imgptr++ = sums[2] / 256;
			}
		}
	} else {
		for (unsigned y = 0; y < mapSizeHA; ++y) {
			for (unsigned x = 0; x < mapSizeW; ++x) {
				sumTile(prj->tileC->truetDat.data() + (get_tile(x, y)*prj->tileC->tcSize), sums, prj);
				*imgptr++ = sums[0] / 64;
				*imgptr++ = sums[1] / 64;
				*imgptr++ = sums[2] / 64;
			}
		}
	}

	dl3quant(imgin, w, h, rows, userpal, false, 0);
	dl3floste(imgin, imgout, w, h, rows, 0, userpal);
	imgptr = imgout;

	if ((prj->gameSystem == NES) && (prj->subSystem & NES2x2)) {
		for (unsigned y = 0; y < mapSizeHA; y += 2) {
			for (unsigned x = 0; x < mapSizeW; x += 2) {
				set_pal_row(x, y, (*imgptr) % rows);
				set_pal_row(x + 1, y, (*imgptr) % rows);
				set_pal_row(x, y + 1, (*imgptr) % rows);
				set_pal_row(x + 1, y + 1, (*imgptr++) % rows);
			}
		}
	} else {
		for (unsigned y = 0; y < mapSizeHA; ++y) {
			for (unsigned x = 0; x < mapSizeW; ++x)
				set_pal_row(x, y, (*imgptr++) % rows);
		}
	}

	free(imgin);
	free(imgout);
	return true;
}
bool tileMap::inRange(uint32_t x, uint32_t y)const {
	if (mapSizeW < x || mapSizeHA < y) {
		printf("Out of range %u %u\n", x, y);
		return false;
	} else
		return true;
}
void tileMap::setRaw(uint32_t x, uint32_t y, uint32_t val) {
	if (inRange(x, y)) {
		uint32_t*tptr = (uint32_t*)tileMapDat;
		tptr += (y * mapSizeW) + x;
		*tptr = val;
	}
}
uint32_t tileMap::getRaw(uint32_t x, uint32_t y)const {
	if (inRange(x, y)) {
		uint32_t*tptr = (uint32_t*)tileMapDat;
		tptr += (y * mapSizeW) + x;
		return *tptr;
	} else
		return 0;
}
void tileMap::resizeBlocks(uint32_t wn, uint32_t hn) {
	uint32_t amtTemp = mapSizeW * mapSizeH * amt;
	amtTemp /= (wn * hn);

	if (amtTemp) {
		amt = amtTemp;
		mapSizeW = wn;
		mapSizeH = hn;
		mapSizeHA = mapSizeH * amt;

		if (window) {
			char tmp[16];
			snprintf(tmp, 16, "%u", amt);
			window->map_amt->value(tmp);
		}
	} else if (window)
		window->updateMapWH();

	ScrollUpdate();
}
void tileMap::blockAmt(uint32_t newAmt) {
	if (newAmt == amt)
		return;

	tileMapDat = (uint8_t*)realloc(tileMapDat, TileMapSizePerEntry * mapSizeW * mapSizeH * newAmt);

	if (newAmt > amt)
		memset(tileMapDat + (amt * TileMapSizePerEntry * mapSizeW * mapSizeH), 0, (newAmt - amt)*mapSizeW * mapSizeH * TileMapSizePerEntry);

	amt = newAmt;
	mapSizeHA = mapSizeH * amt;
	ScrollUpdate();
}
const char * MapWidthTxt = "Map width";
const char * MapHeightTxt = "Map height";
static void rect2rect(uint8_t*in, uint8_t*out, unsigned xin, unsigned yin, unsigned win, unsigned wout, unsigned hout) {
	in += (yin * win) + xin;

	while (hout--) {
		memcpy(out, in, wout);
		in += win;
		out += wout;
	}
}
void tileMap::toggleBlocks(bool set) {
	if (set != isBlock) {
		if (set) {
			//First get user input on the dimensions of each block
			char * str_ptr;
			str_ptr = (char *)fl_input("Enter block width");

			if (!str_ptr)
				return;

			if (!verify_str_number_only(str_ptr))
				return;

			uint32_t w, h;
			w = atoi(str_ptr);

			if (mapSizeW % w) {
				fl_alert("You must enter a number that will result in an integer when dividing by %d", mapSizeW);
				return;
			}

			str_ptr = (char *)fl_input("Enter block height");

			if (!str_ptr)
				return;

			if (!verify_str_number_only(str_ptr))
				return;

			h = atoi(str_ptr);

			if (mapSizeH % h) {
				fl_alert("You must enter a number that will result in an integer when dividing by %d", mapSizeH);
				return;
			}

			if ((mapSizeW != w) || (mapSizeH != h)) {
				//The tiles will need to be rearranged
				uint8_t*tmp = (uint8_t*)malloc(mapSizeW * mapSizeH * TileMapSizePerEntry);
				uint8_t*out = tmp;

				for (uint_fast32_t y = 0; y < mapSizeH; y += h) {
					for (uint_fast32_t x = 0; x < mapSizeW; x += w) {
						rect2rect(tileMapDat, out, x * TileMapSizePerEntry, y, mapSizeW * TileMapSizePerEntry, w * TileMapSizePerEntry, h);
						out += w * h * TileMapSizePerEntry;
					}
				}

				memcpy(tileMapDat, tmp, mapSizeH * mapSizeW * TileMapSizePerEntry);
				free(tmp);
			}

			isBlock = true;
			amt = (mapSizeW * mapSizeH) / (w * h);
			mapSizeW = w;
			mapSizeH = h;
			mapSizeHA = mapSizeH * amt;
		} else {
			isBlock = false;
			mapSizeH *= amt;
			amt = 1; //amt must = 1 when blocks are not in use
			mapSizeHA = mapSizeH;
		}
	}

	if (window) {
		window->updateMapWH();

		if (set) {
			window->map_w->label("Block width");
			window->map_w->callback(resizeBlocksCB);
			window->map_h->label("Block height");
			window->map_h->callback(resizeBlocksCB);
			window->map_amt->show();
			char tmp[16];
			snprintf(tmp, 16, "%u", amt);
			window->map_amt->value(tmp);
		} else {
			window->map_w->callback(callback_resize_map);
			window->map_w->label(MapWidthTxt);
			window->map_h->callback(callback_resize_map);
			window->map_h->label(MapHeightTxt);
			window->map_amt->hide();
		}

		updateTileSelectAmt();
		ScrollUpdate();
	}
}
bool tileMap::get_hflip(uint32_t x, uint32_t y)const {
	if (inRange(x, y))
		return (tileMapDat[((y * mapSizeW) + x) * 4] >> 3) & 1;
	else
		return false;
}
bool tileMap::get_vflip(uint32_t x, uint32_t y)const {
	if (inRange(x, y))
		return (tileMapDat[((y * mapSizeW) + x) * 4] >> 4) & 1;
	else
		return false;
}
bool tileMap::get_prio(uint32_t x, uint32_t y)const {
	if (inRange(x, y))
		return (tileMapDat[((y * mapSizeW) + x) * 4] >> 7) & 1;
	else
		return false;
}
unsigned tileMap::getPalRow(uint32_t x, uint32_t y)const {
	if (inRange(x, y)) {
		if ((prj->gameSystem == NES) && (prj->subSystem & NES2x2)) {
			x &= ~1;
			y &= ~1;
		}

		return (tileMapDat[((y * mapSizeW) + x) * 4] >> 5) & 3;
	} else
		return 0;
}

void tileMap::set_pal_row(uint32_t x, uint32_t y, unsigned row) {
	if ((prj->gameSystem == NES) && (prj->subSystem & NES2x2)) {
		x &= ~1;
		y &= ~1;

		tileMapDat[((y * mapSizeW) + x) * 4] &= ~(3 << 5);
		tileMapDat[((y * mapSizeW) + x) * 4] |= row << 5;

		tileMapDat[((y * mapSizeW) + (x | 1)) * 4] &= ~(3 << 5);
		tileMapDat[((y * mapSizeW) + (x | 1)) * 4] |= row << 5;

		tileMapDat[(((y | 1)*mapSizeW) + x) * 4] &= ~(3 << 5);
		tileMapDat[(((y | 1)*mapSizeW) + x) * 4] |= row << 5;

		tileMapDat[(((y | 1)*mapSizeW) + (x | 1)) * 4] &= ~(3 << 5);
		tileMapDat[(((y | 1)*mapSizeW) + (x | 1)) * 4] |= row << 5;
	} else {
		tileMapDat[((y * mapSizeW) + x) * 4] &= ~(3 << 5);
		tileMapDat[((y * mapSizeW) + x) * 4] |= row << 5;
	}
}

int32_t tileMap::get_tile(uint32_t x, uint32_t y)const {
	//first calculate which tile we want
	if ((mapSizeW <= x) || (mapSizeHA) <= y) {
		printf("Error tile (%d,%d) does not exist on this map\n", x, y);
		return -1;
	}

	uint32_t selected_tile = ((y * mapSizeW) + x) * 4;
	//get both bytes
	uint8_t temp_1, temp_2, temp_3;
	temp_1 = tileMapDat[selected_tile + 1]; //least significant is stored in the lowest address
	temp_2 = tileMapDat[selected_tile + 2];
	temp_3 = tileMapDat[selected_tile + 3]; //most significant
	return (temp_1 << 16) + (temp_2 << 8) + temp_3;
}

int32_t tileMap::get_tileRow(uint32_t x, uint32_t y, unsigned useRow)const {
	//first calculate which tile we want
	uint32_t selected_tile = ((y * mapSizeW) + x) * 4;

	if ((prj->gameSystem == NES) && (prj->subSystem & NES2x2)) {
		x &= ~1;
		y &= ~1;
	}

	uint32_t selected_tile_pal = ((y * mapSizeW) + x) * 4;

	//get both bytes
	if (((tileMapDat[selected_tile_pal] >> 5) & 3) == useRow) {
		uint8_t temp_1, temp_2, temp_3;
		temp_1 = tileMapDat[selected_tile + 1]; //least significant is stored in the lowest address
		temp_2 = tileMapDat[selected_tile + 2];
		temp_3 = tileMapDat[selected_tile + 3]; //most significant
		return (temp_1 << 16) + (temp_2 << 8) + temp_3;
	} else
		return -1;
}

void tileMap::set_vflip(uint32_t x, uint32_t y, bool vflip_set) {
	if (vflip_set)
		tileMapDat[((y * mapSizeW) + x) * 4] |= 1 << 4;
	else
		tileMapDat[((y * mapSizeW) + x) * 4] &= ~(1 << 4);
}

static int bondsCheckTile(int tile, int mx, unsigned x, unsigned y) {
	if (tile < 0)
		tile = 0;

	if (tile > mx) {
		printf("Warning tile value %d exceeded %d at x: %u y: %u\n", tile, mx, x, y);
		tile = mx;
	}

	return tile;
}

bool tileMap::saveToFile(const char*fname, fileType_t type, int clipboard, int compression, const char*label, const char*nesFname, const char*labelNES) {
	uint32_t x, y;
	FILE * myfile;
	size_t fileSize;
	uint8_t* mapptr;

	if (clipboard)
		myfile = 0;
	else if (type != fileType_t::tBinary)
		myfile = fopen(fname, "w");
	else
		myfile = fopen(fname, "wb");

	if (likely(myfile || clipboard)) {
		switch (prj->gameSystem) {
			case segaGenesis:
			{	uint16_t * TheMap;
				fileSize = (mapSizeW * mapSizeH * amt) * 2;
				mapptr = (uint8_t*)malloc(fileSize);
				TheMap = (uint16_t*)mapptr;

				for (y = 0; y < mapSizeH * amt; ++y) {
					for (x = 0; x < mapSizeW; ++x) {
						int tile = get_tile(x, y);
						tile += offset;
						tile = bondsCheckTile(tile, 2047, x, y);
						*TheMap = (uint16_t)tileMapDat[((y * mapSizeW) + x) * 4] << 8; //get attributes
						*TheMap++ |= (uint16_t)tile; //add tile
					}
				}
			}
			break;

			case masterSystem:
			case gameGear:
			{	uint8_t * TheMap;
				fileSize = (mapSizeW * mapSizeH * amt) * 2;
				mapptr = (uint8_t*)malloc(fileSize);
				TheMap = mapptr;

				for (y = 0; y < mapSizeH * amt; ++y) {
					for (x = 0; x < mapSizeW; ++x) {
						/*
						 MSB          LSB
						 ---pcvhn nnnnnnnn

						 - = Unused. Some games use these bits as flags for collision and damage
						     zones. (such as Wonderboy in Monster Land, Zillion 2)
						 p = Priority flag. When set, sprites will be displayed underneath the
						     background pattern in question.
						 c = Palette select.
						 v = Vertical flip flag.
						 h = Horizontal flip flag.
						 n = Pattern index, any one of 512 patterns in VRAM can be selected.
						 */
						int tile = get_tile(x, y);
						tile += offset;
						tile = bondsCheckTile(tile, 511, x, y);
						*TheMap++ = tile & 255;
						*TheMap++ = ((tile >> 8) & 1) | (get_hflip(x, y) << 1) | (get_vflip(x, y) << 2) | (getPalRow(x, y) << 3) | (get_prio(x, y) << 4);
					}
				}
			}
			break;

			case NES:
			case TMS9918: // Both systems have eight bit tile entries.
			{	uint8_t * TheMap;
				fileSize = mapSizeW * mapSizeHA;
				mapptr = (uint8_t *)malloc(fileSize);
				TheMap = mapptr;

				for (y = 0; y < mapSizeHA; ++y) {
					for (x = 0; x < mapSizeW; ++x) {
						int tile = get_tile(x, y);
						tile += offset;
						tile = bondsCheckTile(tile, 255, x, y);
						*TheMap++ = tile;
					}
				}
			}
			break;

			default:
				show_default_error
		}

		if (compression) {
			void*TheMap = mapptr;
			mapptr = (uint8_t*)encodeType(TheMap, fileSize, fileSize, compression);
			free(TheMap);
		}

		char temp[2048];
		snprintf(temp, 2048, "Width %d Height %d %s", mapSizeW, mapSizeHA, typeToText(compression));
		int bits;

		if ((prj->gameSystem == segaGenesis) && (!compression))
			bits = 16;
		else
			bits = 8;

		if (!saveBinAsText(mapptr, fileSize, myfile, type, temp, label, bits, getEndianBySystem())) {
			free(mapptr);
			return false;
		}

		free(mapptr);

		if (myfile)
			fclose(myfile);

		puts("File Saved");
	} else
		return false;

	if (prj->gameSystem == NES) {
		if (clipboard)
			fl_alert("The tilemap is currently on the clipboard. Paste this now and then press okay to place the attributes on the clipboard");

		if (nesFname) {
			if (clipboard)
				myfile = nullptr;
			else if (type != fileType_t::tBinary)
				myfile = fopen(nesFname, "w");
			else
				myfile = fopen(nesFname, "wb");

			if (likely(myfile || clipboard)) {
				uint8_t * AttrMap = (uint8_t *)malloc(((mapSizeW + 2) / 4) * ((mapSizeHA + 2) / 4));
				uint8_t * freeAttrMap = AttrMap;

				for (y = 0; y < mapSizeHA; y += 4) {
					for (x = 0; x < mapSizeW; x += 4)
						* AttrMap++ = getPalRow(x, y) | (getPalRow(x + 2, y) << 2) | (getPalRow(x, y + 2) << 4) | (getPalRow(x + 2, y + 2) << 6);
				}

				if (saveBinAsText(freeAttrMap, ((mapSizeW + 2) / 4) * ((mapSizeHA + 2) / 4), myfile, type, 0, labelNES, 8, boost::endian::order::native) == false)
					return false;

				free(freeAttrMap);

				if (myfile)
					fclose(myfile);
			} else
				return false;
		}
	}

	return true;
}
bool tileMap::saveToFile(void) {
	/*!
	Saves tilemap to file returns true on success or cancellation
	returns false if there was an error but remember if the user cancels this it is not an error
	*/
	//first see how this file should be saved
	uint32_t x, y;
	int compression;
	fileType_t type = askSaveType();
	int clipboard;

	if (type != fileType_t::tBinary) {
		clipboard = clipboardAsk();

		if (clipboard == 2)
			return true;
	} else
		clipboard = 0;

	bool pickedFile;
	char*fname = 0, *nesFname = 0;

	if (clipboard)
		pickedFile = true;
	else
		fname = loadsavefile("Save tilemap to...", true);

	if (!fname)
		return true;

	if (fname) {
		if (prj->gameSystem == NES) {
			nesFname = loadsavefile("Save tilemap attributes to...", true);

			if (!nesFname) {
				free(fname);
				return true;
			}
		}

		compression = compressionAsk();

		if (compression < 0) {
			free(fname);
			free(nesFname);
			return true;
		}

		saveToFile(fname, type, clipboard, compression, planeName.c_str(), nesFname);
	}

	free(fname);
	free(nesFname);
	return true;
}
static void zero_error_tile_map(int32_t x) {
	//this is a long string I do not want it stored more than once
	fl_alert("Please enter value greater than zero you on the other hand entered %d", x);
}

boost::endian::order tileMap::getEndianBySystem() {
	if (prj->gameSystem == segaGenesis)
		return boost::endian::order::big;
	else
		return boost::endian::order::little;
}

bool tileMap::loadFromFile() {
	//start by loading the file
	/*Only will return false when there is a malloc error or file error.
	Return true upon user cancellation or the user not entering a number correctly.*/

	boost::endian::order systemEndian = getEndianBySystem();

	unsigned bytesPerElement;

	switch (prj->gameSystem) {
		case segaGenesis:
		case masterSystem:
		case gameGear:
			bytesPerElement = 2;
			break;

		case NES:
		case TMS9918:
			bytesPerElement = 1;
			break;

		default:
			show_default_error
			return false;
	}


	filereader f = filereader(systemEndian, bytesPerElement, "Load a tilemap");

	if (f.amt == 0)
		return true;

	size_t file_size;
	//get width and height
	int blocksLoad = fl_ask("Are you loading blocks?");
	std::string tilemap_file = the_file;
	int32_t w, h;
	char * str_ptr;

	if (blocksLoad)
		str_ptr = (char *)fl_input("Enter block width");
	else
		str_ptr = (char *)fl_input("Enter width");

	if (!str_ptr)
		return true;

	if (!verify_str_number_only(str_ptr))
		return true;

	w = atoi(str_ptr);

	if (w <= 0) {
		zero_error_tile_map(w);
		return true;
	}

	if (prj->gameSystem == NES && (w & 1) && (prj->subSystem & NES2x2)) {
		fl_alert("Error unlike in Sega Genesis mode NES mode needs the width and height to be a multiple to 2");
		return true;
	}

	if (blocksLoad)
		str_ptr = (char *)fl_input("Enter block height");
	else
		str_ptr = (char *)fl_input("Enter height");

	if (!str_ptr)
		return true;

	if (!verify_str_number_only(str_ptr))
		return true;

	h = atoi(str_ptr);

	if (h <= 0) {
		zero_error_tile_map(h);
		return true;
	}

	if (prj->gameSystem == NES && (h & 1) && (prj->subSystem & NES2x2)) {
		fl_alert("Error unlike in Sega Genesis mode NES mode needs the width and height the be a multiple to 2");
		return true;
	}

	//we can now load the map
	int32_t offset;
	str_ptr = (char *)fl_input("Enter offset\nThis number will be subtracted from the tilemap's tile value can be positive or negative");

	if (!str_ptr)
		return true;

	if (!verify_str_number_only(str_ptr))
		return true;

	offset = atoi(str_ptr);

	if (window)
		window->tmapOffset->value(str_ptr);

	this->offset = offset;

	if (window)
		window->BlocksCBtn->value(blocksLoad ? 1 : 0);

	unsigned index = f.selDat();
	file_size = f.lens[index];
	size_t size_temp;

	uint32_t blocksLoaded = file_size / w / h;

	switch (prj->gameSystem) {
		case segaGenesis:
		case masterSystem:
		case gameGear:
			if (blocksLoad)
				size_temp = blocksLoaded * w * h;
			else
				size_temp = w * h * 2; //Size of data that is to be loaded

			blocksLoaded /= 2;
			break;

		case NES:
			if (blocksLoad)
				size_temp = blocksLoaded * w * h;
			else
				size_temp = w * h;

			break;

		default:
			show_default_error
	}

	printf("W %d H %d blocks loaded %d\n", w, h, blocksLoaded);

	if (window)
		window->updateMapWH();

	mapSizeW = w;
	mapSizeH = h;

	if (blocksLoad)
		amt = blocksLoaded;
	else
		amt = 1;

	mapSizeHA = mapSizeH * amt;

	if (size_temp > file_size)
		fl_alert("Warning: The Tile map that you are attempting to load is smaller than a tile map that has the width and height that you specified\nThis missing data will be padded with tile zero");

	//start converting to tile
	//free(tile_map);
	tileMapDat = (uint8_t *)realloc(tileMapDat, (w * h) * 4 * amt);
	uint8_t * tempMap = (uint8_t *) malloc(size_temp);

	if (unlikely(!tileMapDat)) {
		show_malloc_error(size_temp)
	}

	memcpy(tempMap, f.dat[index], size_temp);

	uint32_t x, y;

	for (y = 0; y < mapSizeH * amt; ++y) {
		for (x = 0; x < mapSizeW; ++x) {
			switch (prj->gameSystem) {
				case segaGenesis:
					if (((x + (y * w) + 1) * 2) <= file_size) {
						uint16_t* ptr16 = (uint16_t*)tempMap;
						uint16_t temp = *ptr16++;
						tempMap = (uint8_t*)ptr16;
						//set attributes
						tileMapDat[((y * mapSizeW) + x) * 4] = (uint8_t)(temp >> 8) & 0xF8;
						temp &= 2047;

						int32_t tempCalculated = int32_t(temp) - offset;
						set_tile(x, y, tempCalculated > 0 ? tempCalculated : 0);
					} else
						set_tile(x, y, 0);

					break;

				case masterSystem:
				case gameGear:
					if (((x + (y * w) + 1) * 2) <= file_size) {
						uint8_t attrs = *tempMap++;
						uint16_t tile = *tempMap++;
						tile |= (attrs & 1) << 8;
						set_tile_full(x, y, tile, (attrs >> 3) & 1, (attrs >> 1) & 1, (attrs >> 2) & 1, (attrs >> 4) & 1);
					} else
						set_tile(x, y, 0);

					break;

				case NES:
					if ((x + (y * w) + 1) <= file_size) {
						int temp = *tempMap++;

						if (temp - offset > 0)
							set_tile(x, y, (int32_t)temp - offset);
						else
							set_tile(x, y, 0);
					} else
						set_tile(x, y, 0);

					break;

				default:
					show_default_error
			}
		}
	}

	if (prj->gameSystem == NES) {
		//now load attributes
		filereader f2 = filereader(boost::endian::order::native, 1, "Load Attributes");

		if (f2.amt) {
			unsigned indx2 = f.selDat();
			uint8_t* tempbuf = (uint8_t*)f.dat[indx2];

			for (y = 0; y < mapSizeH * amt; y += 4) {
				for (x = 0; x < mapSizeW; x += 4) {
					set_pal_row(x, y, *tempbuf & 3);
					set_pal_row(x, y + 1, *tempbuf & 3);
					set_pal_row(x + 1, y, *tempbuf & 3);
					set_pal_row(x + 1, y + 1, *tempbuf & 3);

					set_pal_row(x + 2, y, ((*tempbuf) >> 2) & 3);
					set_pal_row(x + 2, y + 1, ((*tempbuf) >> 2) & 3);
					set_pal_row(x + 3, y, ((*tempbuf) >> 2) & 3);
					set_pal_row(x + 3, y + 1, ((*tempbuf) >> 2) & 3);

					++tempbuf;
				}
			}
		}
	}

	tempMap -= file_size;
	free(tempMap);
	isBlock = blocksLoad;
	toggleBlocks(blocksLoad);

	if (window)
		window->redraw();

	return true;
}

void tileMap::swapTile(uint32_t oldTile, uint32_t newTile) {
	uint_fast32_t x, y;
	int32_t temp;

	for (y = 0; y < mapSizeHA; y++) {
		for (x = 0; x < mapSizeW; x++) {
			temp = get_tile(x, y);

			if (temp == oldTile)
				set_tile(x, y, newTile);
		}
	}
}

void tileMap::sub_tile_map(uint32_t oldTile, uint32_t newTile, bool hflip, bool vflip) {
	uint_fast32_t x, y;
	int_fast32_t temp;

	for (y = 0; y < mapSizeHA; y++) {
		for (x = 0; x < mapSizeW; x++) {
			temp = get_tile(x, y);

			if (temp == oldTile) {
				set_tile(x, y, newTile);

				if (hflip == true)
					set_hflip(x, y, true);

				if (vflip)
					set_vflip(x, y, true);
			}
			else if (temp > oldTile) {
				temp--;

				if (temp < 0)
					temp = 0;

				set_tile(x, y, temp);
			}
		}
	}
}
void tileMap::set_tile_full(uint32_t x, uint32_t y, uint32_t tile, unsigned palette_row, bool use_hflip, bool use_vflip, bool highorlow_prio) {
	if (mapSizeW < x || (mapSizeHA) < y) {
		printf("Error (%d,%d) cannot be set to a tile as it is not on the map", x, y);
		return;
	}

	uint32_t selected_tile = ((y * mapSizeW) + x) * 4;
	uint8_t flags;
	/*
	7  6  5  4  3  2  1 0
	15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
	    p  c   c  v  h  n n n n n n n n n n n

	p = Priority flag
	c = Palette select
	v = Vertical flip
	h = Horizontal flip
	n = Pattern name
	*/
	//the extended tile mapping format is a generic format it goes like this
	//The first byte stores attributes in Sega Genesis format except with no tile data
	//the next two bytes store the tile number
	flags = palette_row << 5;
	flags |= use_hflip << 3;
	flags |= use_vflip << 4;
	flags |= highorlow_prio << 7;
	tileMapDat[selected_tile] = flags;
	//in little endian the least significant byte is stored in the lowest address
	tileMapDat[selected_tile + 1] = (tile >> 16) & 255;
	tileMapDat[selected_tile + 2] = (tile >> 8) & 255;
	tileMapDat[selected_tile + 3] = tile & 255;
}
void tileMap::set_tile(uint32_t x, uint32_t y, uint32_t tile) {
	if (mapSizeW < x || (mapSizeH * amt) < y) {
		printf("Error (%d,%d) cannot be set to a tile as it is not on the map", x, y);
		return;
	}

	uint32_t selected_tile = ((y * mapSizeW) + x) * 4;
	tileMapDat[selected_tile + 1] = (tile >> 16) & 255;
	tileMapDat[selected_tile + 2] = (tile >> 8) & 255;
	tileMapDat[selected_tile + 3] = tile & 255;
}
void tileMap::set_hflip(uint32_t x, uint32_t y, bool hflip_set) {
	if (hflip_set)
		tileMapDat[((y * mapSizeW) + x) * 4] |= 1 << 3;
	else
		tileMapDat[((y * mapSizeW) + x) * 4] &= ~(1 << 3);
}
void tileMap::set_prio(uint32_t x, uint32_t y, bool prio_set) {
	if (prio_set)
		tileMapDat[((y * mapSizeW) + x) * 4] |= 1 << 7;
	else
		tileMapDat[((y * mapSizeW) + x) * 4] &= ~(1 << 7);
}
void tileMap::ScrollUpdate(void) {
	if (!window)
		return;

	uint32_t old_scroll = window->map_x_scroll->value();
	uint32_t tile_size_placer = window->place_tile_size->value();
	int32_t map_scroll = mapSizeW - ((window->w() - map_off_x) / tile_size_placer / 8);

	if (map_scroll < 0)
		map_scroll = 0;

	if (old_scroll > map_scroll) {
		old_scroll = map_scroll;
		map_scroll_pos_x = map_scroll;
	}

	if (map_scroll) {
		window->map_x_scroll->show();
		window->map_x_scroll->value(old_scroll, 1, 0, map_scroll + 2);
	} else
		window->map_x_scroll->hide();

	old_scroll = window->map_y_scroll->value();
	map_scroll = (mapSizeHA) - ((window->h() - map_off_y) / tile_size_placer / 8); //size of all offscreen tiles in pixels

	if (map_scroll < 0)
		map_scroll = 0;

	if (old_scroll > map_scroll) {
		old_scroll = map_scroll;
		map_scroll_pos_y = map_scroll;
	}

	if (map_scroll) {
		window->map_y_scroll->show();
		window->map_y_scroll->value(old_scroll, 1, 0, map_scroll + 2);
	} else
		window->map_y_scroll->hide();
}
void tileMap::resize_tile_map(uint32_t new_x, uint32_t new_y) {
	//mapSizeW and mapSizeH hold old map size
	if ((new_x == mapSizeW) && (new_y == mapSizeH))
		return;

	if (new_x == 0 || new_y == 0) {
		fl_alert("Cannot have a map of size 0");
		return;
	}

	//now create a temp buffer to hold the old data
	uint8_t * temp = 0;
	temp = (uint8_t *)malloc((new_x * new_y) * TileMapSizePerEntry);

	if (!temp) {
		show_malloc_error((new_x * new_y)*TileMapSizePerEntry)
		return;
	}

	uint8_t*extTmp;

	//now copy old data to temp
	for (unsigned y = 0; y < new_y; ++y) {
		for (unsigned x = 0; x < new_x; ++x) {
			uint32_t posCal = ((y * new_x) + x);
			uint32_t sel_map = posCal * TileMapSizePerEntry;

			if (x < mapSizeW && y < mapSizeH) {
				uint32_t sel_map_old = ((y * mapSizeW) + x) * TileMapSizePerEntry;
				memcpy(temp + sel_map, tileMapDat + sel_map_old, TileMapSizePerEntry);
			} else
				memset(temp + sel_map, 0, TileMapSizePerEntry);
		}
	}

	tileMapDat = (uint8_t *)realloc(tileMapDat, (new_x * new_y) * TileMapSizePerEntry);

	if (!tileMapDat) {
		show_realloc_error((new_x * new_y)*TileMapSizePerEntry)
		return;
	}

	memcpy(tileMapDat, temp, (new_x * new_y)*TileMapSizePerEntry);
	free(temp);
	mapSizeW = new_x;
	mapSizeH = new_y;

	if (selTileE_G[0] >= new_x)
		selTileE_G[0] = new_x - 1;

	if (selTileE_G[1] >= new_y)
		selTileE_G[1] = new_y - 1;

	mapSizeHA = mapSizeH;

	if (isBlock)
		mapSizeH /= amt;

	if (window) {
		ScrollUpdate();
		window->updateMapWH();
	}
}
bool tileMap::truecolor_to_image(uint8_t * the_image, int useRow, bool useAlpha) {
	/*!
	the_image pointer to image must be able to hold the image using rgba 32bit or rgb 24bit if not using alpha
	useRow what row to use or -1 for no row
	*/
	if (the_image == 0) {
		fl_alert("Error malloc must be called before generating this image");
		return false;
	}

	unsigned pixelSize = useAlpha ? 4 : 3;

	unsigned w = mapSizeW * prj->tileC->width();
	unsigned wBytes = w * pixelSize;
	unsigned h = mapSizeHA * prj->tileC->height();

	unsigned x_tile = 0, y_tile = 0;
	int_fast32_t truecolor_tile_ptr = 0;
	unsigned pSize2 = prj->tileC->width() * pixelSize;
	unsigned stepTileRowBytes = wBytes * prj->tileC->height();

	for (uint64_t a = 0; a < (h * w * pixelSize); a += stepTileRowBytes) { //a tiles y
		for (uint_fast32_t b = 0; b < w * pixelSize; b += pSize2) { //b tiles x
			if (useRow >= 0)
				truecolor_tile_ptr = get_tileRow(x_tile, y_tile, useRow) * prj->tileC->tcSize;
			else
				truecolor_tile_ptr = get_tile(x_tile, y_tile) * prj->tileC->tcSize;

			if ((truecolor_tile_ptr != -prj->tileC->tcSize) && (truecolor_tile_ptr < (prj->tileC->amt * prj->tileC->tcSize))) {
				for (uint_fast32_t y = 0; y < stepTileRowBytes; y += wBytes) { //pixels y
					if (useAlpha)
						memcpy(&the_image[a + b + y], &prj->tileC->truetDat[truecolor_tile_ptr], prj->tileC->width() * 4);
					else {
						unsigned xx = 0;

						for (unsigned x = 0; x < prj->tileC->width() * 4; x += 4) { //pixels x
							the_image[a + b + y + xx] = prj->tileC->truetDat[truecolor_tile_ptr + x];
							the_image[a + b + y + xx + 1] = prj->tileC->truetDat[truecolor_tile_ptr + x + 1];
							the_image[a + b + y + xx + 2] = prj->tileC->truetDat[truecolor_tile_ptr + x + 2];
							xx += 3;
						}
					}

					truecolor_tile_ptr += prj->tileC->width() * 4;
				}
			} else {
				for (uint32_t y = 0; y < stepTileRowBytes; y += wBytes) //pixels y
					memset(&the_image[a + b + y], 0, pSize2);
			}

			++x_tile;
		}

		x_tile = 0;
		++y_tile;
	}

	return true;
}
void tileMap::truecolorimageToTiles(uint8_t * image, int rowusage, bool useAlpha, bool copyToTruecolor, bool convert, bool isIndexArray) {
	if (isIndexArray && useAlpha) {
		fl_alert("Invalid parameters for truecolorimageToTiles.");
		return;
	}

	unsigned type_temp = palTypeGen;
	unsigned tempSet = 0;
	uint8_t*truecolor_tile = (uint8_t*)alloca(isIndexArray ? (prj->tileC->width() * prj->tileC->height()) : prj->tileC->tcSize);
	uint_fast32_t x_tile = 0, y_tile = 0;
	uint_fast32_t pSize = isIndexArray ? 1 : (useAlpha ? 4 : 3);
	uint_fast32_t pTile = prj->tileC->width() * pSize;
	uint_fast32_t pTileHeight = prj->tileC->height() * pSize;
	uint_fast32_t w = mapSizeW * prj->tileC->width();
	uint_fast32_t wBytes = w * pSize;
	uint_fast32_t h = mapSizeHA * prj->tileC->height();

	for (uint_fast32_t a = 0; a < h * wBytes; a += w * pTileHeight) { //a tiles y
		for (uint_fast32_t b = 0; b < wBytes; b += pTile) { //b tiles x
			int32_t current_tile;

			if (rowusage < 0) {
				current_tile = get_tile(x_tile, y_tile);

				if (current_tile >= prj->tileC->amt)
					goto dont_convert_tile;
			} else {
				current_tile = get_tileRow(x_tile, y_tile, rowusage);

				if (current_tile >= prj->tileC->amt)
					goto dont_convert_tile;

				if (current_tile == -1)
					goto dont_convert_tile;
			}

			{	uint8_t*truecolor_tile_ptr = truecolor_tile;

				for (uint32_t y = 0; y < w * pTileHeight; y += wBytes) { //pixels y
					if (useAlpha || isIndexArray) {
						memcpy(truecolor_tile_ptr, &image[a + b + y], prj->tileC->width() * pSize);
						truecolor_tile_ptr += prj->tileC->width() * pSize;
					} else {
						for (uint8_t xx = 0; xx < prj->tileC->width() * 3; xx += 3) {
							memcpy(truecolor_tile_ptr, &image[a + b + y + xx], 3);
							truecolor_tile_ptr += 3;
							*truecolor_tile_ptr++ = 255;
						}
					}
				}
			}

			//convert back to tile
			if ((type_temp != 0) && (prj->gameSystem == segaGenesis)) {
				tempSet = (get_prio(x_tile, y_tile) ^ 1) * 8;
				set_palette_type_force(tempSet);
			}

			if (copyToTruecolor)
				memcpy(prj->tileC->truetDat.data() + (current_tile * prj->tileC->tcSize), truecolor_tile, prj->tileC->tcSize);

			if (convert)
				prj->tileC->truecolor_to_tile_ptr(getPalRow(x_tile, y_tile), current_tile, truecolor_tile, false, false, isIndexArray);

dont_convert_tile:
			++x_tile;
		}

		x_tile = 0;
		++y_tile;
	}

	if (prj->gameSystem == segaGenesis)
		set_palette_type();
}
void tileMap::drawPart(unsigned offx, unsigned offy, unsigned x, unsigned y, unsigned w, unsigned h, int rowSolo, unsigned zoom, bool trueCol) {
	w += x;
	h += y;
	bool shadowHighlight = prj->gameSystem == segaGenesis && palTypeGen;

	for (unsigned j = y; j < h; ++j) {
		unsigned ox = offx;

		for (unsigned i = x; i < w; ++i) {
			if (shadowHighlight) {
				unsigned temp = (get_prio(i, j) ^ 1) * 8;
				set_palette_type_force(temp);
			}

			bool canShow;
			unsigned palRow = getPalRow(i, j);

			if (rowSolo >= 0)
				canShow = palRow == rowSolo;

			else
				canShow = true;

			if (canShow) {
				bool hflip = get_hflip(i, j);
				bool vflip = get_vflip(i, j);
				bool prio = get_prio(i, j);
				int32_t tile = get_tile(i, j);

				if (tile >= 0) {
					if (trueCol)
						prj->tileC->draw_truecolor(tile, ox, offy, hflip, vflip, zoom);
					else
						prj->tileC->draw_tile(ox, offy, tile, zoom, palRow, hflip, vflip, false, prj->curPlane);
				}
			}

			ox += prj->tileC->width() * zoom;
		}

		offy += prj->tileC->height() * zoom;
	}

	if (shadowHighlight)
		set_palette_type();
}
void tileMap::drawBlock(unsigned block, unsigned xo, unsigned yo, unsigned flags, unsigned zoom) {
	uint32_t Ty = block * mapSizeH;

	if (flags & 2)
		yo += (mapSizeH - 1) * prj->tileC->height() * zoom;

	int xoo;

	for (uint32_t yb = 0; yb < mapSizeH; ++yb) {
		xoo = xo;

		if (flags & 1)
			xoo += (mapSizeW - 1) * prj->tileC->width() * zoom;

		for (uint32_t xb = 0; xb < mapSizeW; ++xb) {
			bool hflip = get_hflip(xb, Ty), vflip = get_vflip(xb, Ty);
			unsigned row = getPalRow(xb, Ty);
			int32_t tile = get_tile(xb, Ty);

			if (tile >= 0) {
				if (flags == 3) { //Both
					if (showTrueColor)
						prj->tileC->draw_truecolor(tile, xoo, yo, hflip ^ true, vflip ^ true, zoom);
					else
						prj->tileC->draw_tile(xoo, yo, tile, zoom, row, hflip ^ true, vflip ^ true);
				} else if (flags & 2) { //Y-flip
					if (showTrueColor)
						prj->tileC->draw_truecolor(tile, xoo, yo, hflip, vflip ^ true, zoom);
					else
						prj->tileC->draw_tile(xoo, yo, tile, zoom, row, hflip, vflip ^ true);
				} else if (flags & 1) { //X-flip
					if (showTrueColor)
						prj->tileC->draw_truecolor(tile, xoo, yo, hflip ^ true, vflip, zoom);
					else
						prj->tileC->draw_tile(xoo, yo, tile, zoom, row, hflip ^ true, vflip);
				} else { //No flip
					if (showTrueColor)
						prj->tileC->draw_truecolor(tile, xoo, yo, hflip, vflip, zoom);
					else
						prj->tileC->draw_tile(xoo, yo, tile, zoom, row, hflip, vflip);
				}
			}

			if (flags & 1)
				xoo -= prj->tileC->width() * zoom;
			else
				xoo += prj->tileC->width() * zoom;
		}

		if (flags & 2)
			yo -= prj->tileC->height() * zoom;
		else
			yo += prj->tileC->height() * zoom;

		++Ty;
	}
}
void tileMap::findFirst(int&x, int&y, unsigned tile)const {
	for (unsigned j = 0; j < mapSizeHA; ++j) {
		for (unsigned i = 0; i < mapSizeW; ++i) {
			if (tile == get_tile(i, j)) {
				x = i;
				y = j;
				return;
			}
		}
	}

	x = y = -1;
}
#define TRANSPARENT 0
#define SOLID_BLACK 1
#define MEDIUM_GREEN 2
#define LIGHT_GREEN 3
#define DARK_BLUE 4
#define LIGHT_BLUE 5
#define DARK_RED 6
#define CYAN 7
#define MEDIUM_RED 8
#define LIGHT_RED 9
#define DARK_YELLOW 10
#define LIGHT_YELLOW 11
#define DARK_GREEN 12
#define MAGENTA 13
#define GRAY 14
#define WHITE 15
static const uint8_t complementryColor[] = {
	SOLID_BLACK,  // TRANSPARENT
	TRANSPARENT,  // SOLID_BLACK
	DARK_GREEN,   // MEDIUM_GREEN
	MEDIUM_GREEN, // LIGHT_GREEN
	LIGHT_BLUE,   // DARK_BLUE
	DARK_BLUE,    // LIGHT_BLUE
	MEDIUM_RED,   // DARK_RED
	LIGHT_BLUE,   // CYAN
	DARK_RED,     // MEDIUM_RED
	MEDIUM_RED,   // LIGHT_RED
	LIGHT_YELLOW, // DARK_YELLOW
	DARK_YELLOW,  // LIGHT_YELLOW
	MEDIUM_GREEN, // DARK_GREEN
	LIGHT_RED,    // MAGENTA
	WHITE,        // GRAY
	GRAY,         // WHITE

};
static void pickFromHist2(unsigned*hist, unsigned*largest, unsigned cnt, Project*prj) {
	memset(largest, 0, 2 * sizeof(unsigned));
	unsigned bgColor = prj->getPalColTMS9918() & 15;

	if (bgColor) {
		hist[0] += hist[bgColor];
		hist[bgColor] = 0;
	}

	unsigned start = bgColor == 1 ? 2 : 1;
	unsigned cmp = hist[start];
	largest[0] = start;

	for (unsigned x = start + 1; x < cnt; ++x) { // Foreground color.
		if (x != bgColor) {
			if (hist[x] >= cmp) {
				cmp = hist[x];
				largest[0] = x;
			}
		}
	}

	if (hist[0])
		largest[1] = 0;
	else {
		bool allZeros = true;

		for (unsigned x = 0; x < cnt; ++ x) {
			if (x != largest[0]) {
				if (hist[x] != 0) {
					allZeros = false;
					break;
				}
			}
		}

		if (allZeros)
			largest[1] = complementryColor[largest[0]];
		else {
			for (unsigned x = 0; x < cnt; ++ x) {
				if (x != largest[0]) {
					cmp = hist[x];
					largest[1] = x;
					start = x;
					break;
				}
			}

			for (unsigned x = start + 1; x < cnt; ++x) {
				if (x != largest[0]) {
					if (hist[x] >= cmp) {
						cmp = hist[x];
						largest[1] = x;
					}
				}
			}
		}

		// largest[1] is the background color.
		if (largest[0] < largest[1]) {
			unsigned tmp = largest[1];
			largest[1] = largest[0];
			largest[0] = tmp;
		}
	}

	if (largest[0] == largest[1])
		puts("largest[0] should never equal largest[1]");

}
void tileMap::pickExtAttrs(void) {
	switch (prj->gameSystem) {
		case TMS9918:
		{
			unsigned w = mapSizeW * prj->tileC->width();
			unsigned h = mapSizeHA * prj->tileC->height();
			uint8_t*imgTmp = (uint8_t*)malloc(w * h * 4);
			truecolor_to_image(imgTmp);
			uint8_t*indexList = (uint8_t*)ditherImage(imgTmp, w, h, true, true, false, 0, false, 0, false, true);

			if (!indexList)
				return;

			switch (prj->getTMS9918subSys()) {
				case MODE_0: { // Pick the two most used colors.
					uint32_t hist[16];
					memset(hist, 0, sizeof(hist));
					uint8_t* tPtr = imgTmp;
					uint8_t* idxPtr = indexList;

					for (unsigned i = 0; i < w * h; ++i) {
						if (tPtr[3])
							++hist[*idxPtr];
						else
							++hist[0];

						tPtr += 4;
						++idxPtr;
					}


					unsigned cmp = hist[0];
					bool allEqual = true;

					for (unsigned x = 1; x < 16; ++x) {
						allEqual &= cmp == hist[x];

						if (!allEqual)
							break;
					}

					if (allEqual)
						prj->setPalColTMS9918(0xF0);
					else {
						unsigned largest[2];
						pickFromHist2(hist, largest, 16, prj);
						prj->setPalColTMS9918((largest[0] << 4) | largest[1]); // Upper foreground, Lower background.
					}
				}
				break;

				case MODE_1: {
					tileAttrMap_t attrs;

					for (unsigned j = 0; j < mapSizeHA; ++j) {
						for (unsigned i = 0; i < mapSizeW; ++i) {
							unsigned hist[16];
							int32_t tile = get_tile(i, j);

							if (tile < prj->tileC->amt) {
								memset(hist, 0, sizeof(hist));

								for (unsigned y = 0; y < prj->tileC->height(); ++y) {
									unsigned offset = (j * prj->tileC->height() * prj->tileC->width() * mapSizeW) + (i * prj->tileC->width()) + (y * mapSizeW * prj->tileC->width());
									uint8_t*iPtr = indexList + offset;
									uint8_t*tPtr = imgTmp + (offset * 4) + 3;

									for (unsigned x = 0; x < prj->tileC->width(); ++x) {
										if (*tPtr)
											++hist[(*iPtr++) & 15];
										else
											++hist[0];

										tPtr += 4;
									}
								}

								unsigned cmp = hist[0];
								bool allEqual = true;

								for (unsigned x = 1; x < 16; ++x) {
									allEqual &= cmp == hist[x];

									if (!allEqual)
										break;
								}

								if (allEqual)
									attrs.emplace(0xF0, tile);
								else {
									unsigned largest[2];
									pickFromHist2(hist, largest, 16, prj);
									attrs.emplace((largest[0] << 4) | largest[1], tile);
								}
							}
						}
					}

					prj->tileC->tms9918Mode1RearrangeTiles(attrs);
				}
				break;

				case MODE_2: {

					for (unsigned j = 0; j < mapSizeHA; ++j) {
						for (unsigned i = 0; i < mapSizeW; ++i) {
							for (unsigned y = 0; y < prj->tileC->height(); ++y) {
								unsigned tile = get_tile(i, j);
								if (tile >= prj->tileC->amt) // Skip out of bounds tiles.
									continue;
								unsigned offset = (j * prj->tileC->height() * prj->tileC->width() * mapSizeW) + (i * prj->tileC->width()) + (y * mapSizeW * prj->tileC->width());
								uint8_t*iPtr = indexList + offset;
								uint8_t*tPtr = imgTmp + (offset * 4) + 3;
								unsigned hist[16];
								memset(hist, 0, sizeof(hist));

								for (unsigned x = 0; x < prj->tileC->width(); ++x) {
									if (*tPtr)
										++hist[(*iPtr++) & 15];
									else
										++hist[0];

									tPtr += 4;
								}

								unsigned cmp = hist[0];
								bool allEqual = true;

								for (unsigned x = 1; x < 16; ++x) {
									allEqual &= cmp == hist[x];

									if (!allEqual)
										break;
								}


								if (allEqual) {
									prj->tileC->setExtAttr(tile, y, 0xF0); // White foreground, black background.
								} else {
									unsigned largest[2];
									pickFromHist2(hist, largest, 16, prj);
									prj->tileC->setExtAttr(tile, y, (largest[0] << 4) | largest[1]);
								}
							}
						}
					}

				}
				break;

				default:
					show_default_error
			}

			free(imgTmp);
			free(indexList);

		}
		break;

		default:
			show_default_error
	}
}
void tileMap::removeBlock(unsigned id) {
	if (isBlock) {
		if (id < (amt - 1)) {
			size_t perElmB = mapSizeW * mapSizeH * TileMapSizePerEntry;
			memmove(tileMapDat + id * perElmB, tileMapDat + (id + 1)*perElmB, (amt - id - 1)*perElmB);
		}

		if (id < amt)
			blockAmt(amt - 1);
	}
}
void tileMap::fixPaletteRows(unsigned num, unsigned dom) {
	for (unsigned y = 0; y < mapSizeHA; ++y) {
		for (unsigned x = 0; x < mapSizeW; ++x)
			set_pal_row(x, y, getPalRow(x, y) * num / dom);

	}

}
