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

#include <ctime>
#include <exception>
#include <memory>
#include <stdexcept>
#include <unordered_set>

#include "macros.h"
#include "nearestColor.h"
#include "class_tiles.h"
#include "dither.h"
#include "tilemap.h"
#include "errorMsg.h"
#include "undo.h"
#include "classpalettebar.h"
#include "gui.h"
#include "compressionWrapper.h"
tiles::tiles(struct Project*prj) {
	this->prj = prj;

	current_tile = 0;
	amt = 1;
	setDim(8, 8, prj->getBitdepthSys());
}

void tiles::setWidth(unsigned w) {
	sizew = w;
	sizewbytesbits = (w + 7) & (~7);
	sizewbytes = sizewbytesbits / 8;
}

tiles::tiles(const tiles&other, Project*prj) {
	this->prj = prj;
	current_tile = other.current_tile;
	amt = other.amt;
	tileSize = other.tileSize;

	setWidth(other.width());
	sizeh = other.height();

	tcSize = sizew * sizeh * 4;

	tDat = other.tDat;
	truetDat = other.truetDat;
	extAttrs = other.extAttrs;

	curBD = other.curBD;
}

tiles::~tiles() {
	tDat.clear();
	truetDat.clear();
	extAttrs.clear();
}

uint8_t tiles::getExtAttr(unsigned tile, unsigned y) {
	if (tile >= amt)
		throw std::out_of_range("tile >= amt");

	if (y >= height())
		throw std::out_of_range("y >= height()");

	switch (prj->getTMS9918subSys()) {
		case MODE_0:
			return prj->getPalColTMS9918();
			break;

		case MODE_1:
			return extAttrs[tile / prj->extAttrTilesPerByte()];
			break;

		case MODE_2:
			return extAttrs[tile * prj->extAttrBytesPerTile() + y];
			break;

		default:
			show_default_error
			return 0xF0;
	}
}

void tiles::setExtAttr(unsigned tile, unsigned y, uint8_t fgbg) {
	if (tile >= amt)
		throw std::out_of_range("tile >= amt");

	if (y >= height())
		throw std::out_of_range("y >= height()");

	switch (prj->getTMS9918subSys()) {
		case MODE_0:
			prj->setPalColTMS9918(fgbg);
			break;

		case MODE_1:
			tms9918Mode1RearrangeActions(true, tile, fgbg);
			break;

		case MODE_2:
			extAttrs[tile * prj->extAttrBytesPerTile() + y] = fgbg;
			break;

		default:
			show_default_error
	}
}

void tiles::insertTile(uint32_t at) {
	try {
		if (at > amt)
			resizeAmt(at);
		else
			++amt;

		tDat.insert(tDat.begin() + at * tileSize, tileSize, 0);
		truetDat.insert(truetDat.begin() + at * tcSize, tcSize, 0);
	} catch (std::exception& e) {
		fl_alert("Error inserting tile at %d\nAdditional details: %s", at - 1, e.what());
		exit(1);
	}
}
void tiles::setPixel(uint8_t*ptr, uint32_t x, uint32_t y, uint32_t val) {
	if (x >= sizew)
		x = sizew - 1;

	if (y >= sizeh)
		y = sizeh - 1;

	unsigned bd = prj->getBitdepthSysraw() + 1;
	unsigned maxp = (1 << bd) - 1;

	if (val > maxp)
		val = maxp;

	ptr += y * sizewbytes;
	x = (sizewbytesbits - 1) - x;

	for (unsigned shift = 0; shift < bd; ++shift) {
		*ptr &= ~(1 << x);
		*ptr |= (val & 1) << x;
		val >>= 1;
		ptr += ((sizew + 7) / 8) * sizeh;
	}
}
void tiles::setPixel(uint32_t tile, uint32_t x, uint32_t y, uint32_t val) {
	uint8_t*ptr = &tDat[(tile * tileSize)];
	setPixel(ptr, x, y, val);
}
uint32_t tiles::getPixel(const uint8_t*ptr, uint32_t x, uint32_t y) const {
	if (x >= sizew)
		x = sizew - 1;

	if (y >= sizeh)
		y = sizeh - 1;

	unsigned bdr = prj->getBitdepthSysraw();
	unsigned val = 0;
	x = (sizewbytesbits - 1) - x;
	ptr += y * sizewbytes;

	for (unsigned shift = 0; shift <= bdr; ++shift) {
		val |= ((*ptr >> x) & 1) << shift;
		ptr += sizewbytes * sizeh;
	}

	return val;
}

uint32_t tiles::getPixel(uint32_t tile, uint32_t x, uint32_t y) const {
	const uint8_t*ptr = &tDat[(tile * tileSize)];
	return getPixel(ptr, x, y);
}

uint32_t* tiles::getPixelPtrTC(uint32_t tile, uint32_t x, uint32_t y) {
	uint32_t*tt = (uint32_t*)((uint8_t*)truetDat.data() + (tile * tcSize));
	tt += y * sizew;
	tt += x;
	return tt;
}

void tiles::setPixelTc(uint32_t tile, uint32_t x, uint32_t y, uint32_t val) {
	uint32_t*tt = getPixelPtrTC(tile, x, y);
	*tt = val;
}

uint32_t tiles::getPixelTc(uint32_t tile, uint32_t x, uint32_t y) const {
	const uint32_t*tt = (const uint32_t*)((const uint8_t*)truetDat.data() + (tile * tcSize));
	tt += y * sizew;
	tt += x;
	return*tt;
}
void tiles::resizeAmt(uint32_t amtnew) {
	amt = amtnew;
	resizeAmt();
}
void tiles::resizeAmt(void) {
	try {
		tDat.resize(amt * tileSize);
		truetDat.resize(amt * tcSize);
		unsigned tp = prj->extAttrTilesPerByte();
		unsigned tpMult = prj->extAttrBytesPerTile();

		if (tp)
			tp = ((amt * tpMult) + tp - 1) / tp;

		extAttrs.resize(tp);

		if (prj == currentProject)
			updateTileSelectAmt(amt);
	} catch (std::exception&e) {
		fl_alert("Error: cannot resize tiles to %u\nAdditional details %s", amt, e.what());
		exit(1);
	}
}
void tiles::appendTile(unsigned many) {
	resizeAmt(amt + many);
}

void tiles::tms9918Mode1RearrangeActions(bool forceTileToAttribute, uint32_t tile, uint8_t attr) {

	tileAttrMap_t attrs;

	for (unsigned i = 0; i < amt; ++i) {
		// See if this is a padding tile or a duplicate blank tile.
		if (i != tile)
			attrs.emplace(getExtAttr(i, 0), i);
		else if (forceTileToAttribute)  // i == tile.
			attrs.emplace(attr, i);
	}

	tms9918Mode1RearrangeTiles(attrs, false);
}

void tiles::remove_tile_at(uint32_t tileDel) {
	if (tileDel >= amt) {
		fl_alert("Cannot delete tile %d as there are only %d tiles", tileDel, amt);
		return;
	}

	if (amt <= 1) {
		fl_alert("You cannot delete the last time. Instead disable tiles in the project settings to do this.");
		return;
	}

	if (extAttrs.size()) {
		switch (prj->gameSystem) {
			case TMS9918:
				switch (prj->getTMS9918subSys()) {
					case MODE_1:
					{
						// This requires special action.
						// The way this is handled is by first building an attribute list of all tiles except the one we would like to remove and not including padding tiles.
						tms9918Mode1RearrangeActions(false, tileDel, 0);
						// Afterwards the list is sorted and the new tiles won't contain the one we tried to remove.
					}

					return; // All the code below does not need to run. We have already taken care of all this.
				}

				break;

			default:
				show_default_error
		}
	}

	tDat.erase(tDat.begin() + (tileDel * tileSize), tDat.begin() + ((tileDel + 1)*tileSize));
	truetDat.erase(truetDat.begin() + (tileDel * tcSize), truetDat.begin() + ((tileDel + 1)*tcSize));


	--amt;
	resizeAmt(); // Ensure extended attributes have the correct size.
}
void tiles::truecolor_to_tile(unsigned palette_row, uint32_t cur_tile, bool isSprite) {
	truecolor_to_tile_ptr(palette_row, cur_tile, &truetDat[(cur_tile * tcSize)], true, isSprite);
}
void tiles::truecolor_to_tile_ptr(unsigned palette_row, uint32_t cur_tile, uint8_t * tileinput, bool useDither, bool isSprite, bool isIndexArray) {
	//dithers a truecolor tile to tile
	if (useDither && isIndexArray) {
		fl_alert("Invalid parameters");
		return;
	}

	uint8_t*truePtr = (uint8_t*)alloca(tcSize);

	if (isIndexArray)
		truePtr = tileinput;
	else
		memcpy(truePtr, tileinput, tcSize);

	void*outIdx = nullptr;

	if (useDither) {
		ditherImage(truePtr, width(), height(), true, true, true, palette_row, false, 0, isSprite, false, -1, cur_tile);
		outIdx = ditherImage(truePtr, width(), height(), true, false, true, palette_row, false, 0, isSprite, true, -1, cur_tile);
	}

	//now image needs to be checked for alpha
	uint8_t * tPtr = useDither ? (uint8_t*)outIdx : truePtr;

	for (unsigned y = 0; y < sizeh; ++y) {
		for (unsigned x = 0; x < sizew; ++x) {
			unsigned temp;

			setPixel(cur_tile, x, y, (*tPtr++) % prj->pal->perRow);
		}
	}

	free(outIdx);
}
void tiles::draw_truecolor(uint32_t tile_draw, unsigned x, unsigned y, bool usehflip, bool usevflip, unsigned zoom) {
	static bool dontShow;

	if (tile_draw >= amt) {
		if (unlikely(!dontShow)) {
			fl_alert("Warning tried to draw truecolor tile # %d at X: %d y: %d\nBut there is only %d tiles.\nNote that this message will not be shown again.\n Instead it will be outputted to stdout", tile_draw, x, y, amt);
			dontShow = true;
		} else
			printf("Warning tried to draw truecolor tile # %d at X: %d y: %d\nBut there is only %d tiles.\n", tile_draw, x, y, amt);

		return;
	}

	uint_fast32_t xx, yy, xxx, yyy;
	uint8_t*trueColTemp = (uint8_t*)alloca(tcSize);
	uint8_t*grid = (uint8_t*)alloca(tcSize * 3 / 4);
	uint8_t * grid_ptr = grid;
	uint8_t * truePtr;

	bool is255 = false;

	for (unsigned i = 0; i < height(); ++i) {
		for (unsigned j = 0; j < width(); ++j) {
			*grid_ptr++ = is255 ? 255 : 160;
			*grid_ptr++ = is255 ? 255 : 160;
			*grid_ptr++ = is255 ? 255 : 160;
			is255 ^= true;
		}

		is255 ^= true;
	}

	if (usehflip == false && usevflip == false)
		std::copy(truetDat.begin() + (tile_draw * tcSize), truetDat.begin() + ((tile_draw + 1)*tcSize), trueColTemp);
	else if (usehflip == true && usevflip == false)
		hflip_truecolor(tile_draw, (uint32_t *)trueColTemp);
	else if (usehflip == false && usevflip == true)
		vflip_truecolor(tile_draw, trueColTemp);
	else {
		hflip_truecolor(tile_draw, (uint32_t *)trueColTemp);
		vflip_truecolor_ptr(trueColTemp, trueColTemp);
	}

	truePtr = &trueColTemp[3];
	grid_ptr = grid;

	for (xxx = 0; xxx < sizew * sizeh; ++xxx) {
		for (xx = 0; xx < 3; xx++) {
			if (*truePtr) {
				double percent = (double) * truePtr / 255.0;
				uint8_t grid_nerd = *grid_ptr;
				//*grid_ptr++=((double)trueColTemp[(zz*4)+xx]*percent)+((double)*grid_ptr*(1.0-percent));//this could be undefined
				*grid_ptr++ = ((double)trueColTemp[(xxx * 4) + xx] * percent) + ((double)grid_nerd * (1.0 - percent));
			} else
				grid_ptr++;
		}

		truePtr += 4; //next alpha value
	}

	if (zoom > 1) {
		uint8_t*scaled = (uint8_t*)alloca(sizew * sizeh * zoom * zoom * 3);
		uint8_t*s = scaled;
		grid_ptr = grid;

		for (yy = 0; yy < sizeh; ++yy) {
			for (xx = 0; xx < sizew; ++xx) {
				for (yyy = 0; yyy < zoom; ++yyy) {
					grid_ptr = grid + (yy * sizew * 3) + (xx * 3);
					s = scaled + (yy * zoom * zoom * sizew * 3) + (xx * 3 * zoom) + (yyy * sizew * 3 * zoom);

					for (xxx = 0; xxx < zoom; ++xxx) {
						*s++ = grid_ptr[0];
						*s++ = grid_ptr[1];
						*s++ = grid_ptr[2];
					}
				}
			}
		}

		fl_draw_image(scaled, x, y, sizew * zoom, sizeh * zoom, 3);
	} else
		fl_draw_image(grid, x, y, sizew * zoom, sizeh * zoom, 3);
}
void tiles::draw_tile(int x_off, int y_off, uint32_t tile_draw, unsigned zoom, unsigned pal_row, bool Usehflip, bool Usevflip, bool isSprite, unsigned plane, bool alpha) {
	static bool dontShow;

	if (tile_draw >= amt) {
		if (unlikely(!dontShow)) {
			fl_alert("Warning tried to draw tile # %d at X: %d y: %d\nBut there is only %d tiles.\nNote that this message will not be shown again.\n Instead it will be outputted to stdout", tile_draw, x_off, y_off, amt);
			dontShow = true;
		} else
			printf("Warning tried to draw tile # %d at X: %d y: %d\nBut there is only %d tiles.\n", tile_draw, x_off, y_off, amt);

		return;
	}

	static uint8_t*temp_img_ptr;
	static unsigned tempPtrSize;
	unsigned bpp = alpha ? 4 : 3;

	if (tempPtrSize < (((sizeh * zoom) * (sizew * zoom))*bpp)) {
		tempPtrSize = ((sizeh * zoom) * (sizew * zoom)) * bpp;
		temp_img_ptr = (uint8_t*)realloc(temp_img_ptr, tempPtrSize);
	}

	if (!temp_img_ptr) {
		show_realloc_error(((sizeh * zoom) * (sizew * zoom))*bpp)
		return;
	}

	for (unsigned y = 0; y < sizeh; ++y) {
		for (unsigned x = 0; x < sizew; ++x) {
			unsigned rawP = getPixel(tile_draw, Usehflip ? (sizew - 1) - x : x, Usevflip ? (sizeh - 1) - y : y);
			unsigned pixOff = rawP * 3;

			if (prj->gameSystem == TMS9918 && prj->getTMS9918subSys() != MODE_3) {
				unsigned palEnt;

				switch (prj->getTMS9918subSys()) {
					case MODE_0:
						palEnt = prj->getPalColTMS9918(); // Only two colors supported for all tiles.
						break;

					case MODE_1:
						// Only one entry for every eight tiles.
						palEnt = extAttrs[tile_draw / prj->extAttrTilesPerByte()];
						break;

					case MODE_2:
						palEnt = extAttrs[tile_draw * prj->extAttrBytesPerTile() + y];
						break;

					default:
						show_default_error
				}

				if (pixOff)
					palEnt >>= 4;
				else
					palEnt &= 15;

				pixOff = palEnt * 3;
			} else if (prj->pal->haveAlt && isSprite) {
				pixOff += prj->pal->colorCnt * 3;
				pixOff += pal_row * prj->pal->perRowalt * 3;
			} else
				pixOff += pal_row * prj->pal->perRow * 3;

			for (unsigned i = 0; i < zoom; ++i) {
				for (unsigned j = 0; j < zoom; ++j) {
					if (rawP || (!alpha)) {
						for (unsigned k = 0; k < 3; ++k)
							temp_img_ptr[cal_offset_zoom_rgb((x * zoom) + j, (y * zoom) + i, zoom, k, bpp)] = prj->pal->rgbPal[pixOff + k];

						if (alpha)
							temp_img_ptr[cal_offset_zoom_rgb((x * zoom) + j, (y * zoom) + i, zoom, 3, bpp)] = 255;
					} else
						memset(&temp_img_ptr[cal_offset_zoom_rgb((x * zoom) + j, (y * zoom) + i, zoom, 0, bpp)], 0, bpp);
				}
			}
		}
	}

	fl_draw_image(temp_img_ptr, x_off, y_off, sizew * zoom, sizeh * zoom, bpp);
}
void tiles::hflip_truecolor(uint32_t id, uint32_t * out) {
	//out must contain at least tcSize bytes
	uint32_t * trueColPtr = (uint32_t *)&truetDat[id * tcSize];
	trueColPtr += sizew - 1;

	for (unsigned y = 0; y < sizeh; y++) {
		for (unsigned x = 0; x < sizew; ++x)
			*out++ = *trueColPtr--;

		trueColPtr += sizew * 2;
	}
}
void tiles::vflip_truecolor_ptr(uint8_t * in, uint8_t * out) {
	/*vflip_truecolor_ptr needs to be a separate function as the output of hflip may be inputted here to form a vertically and horizontally flipped tile*/
	uint16_t y;
	uint8_t temp[256];
	memcpy(temp, in, 256);

	for (y = 0; y < 256; y += 32)
		memcpy(&out[224 - y], &temp[y], 32);
}
void tiles::vflip_truecolor(uint32_t id, uint8_t * out) {
	vflip_truecolor_ptr(&truetDat[id * tcSize], out);
}
void tiles::hflip_tile(uint32_t id, uint8_t * out) {
	for (unsigned y = 0; y < sizeh; ++y) {
		for (unsigned x = 0; x < sizew; ++x)
			setPixel(out, (sizew - 1) - x, y, getPixel(id, x, y));
	}
}
void tiles::vflip_tile_ptr(const uint8_t*in, uint8_t*out) {
	uint8_t*tmp;

	if (in == out) {
		tmp = (uint8_t*)alloca(tileSize);
		memcpy(tmp, in, tileSize);
	} else
		tmp = (uint8_t*)in;

	for (unsigned y = 0; y < sizeh; ++y) {
		for (unsigned x = 0; x < sizew; ++x)
			setPixel(out, x, (sizeh - 1) - y, getPixel(tmp, x, y));
	}
}
void tiles::vflip_tile(uint32_t id, uint8_t * out) {
	vflip_tile_ptr(&tDat[id * tileSize], out);
}
void tiles::blank_tile(uint32_t tileUsage) {
	if (mode_editor == tile_edit) {
		memset(&truetDat[tileUsage * tcSize], 0, tcSize);
		truecolor_to_tile(palBar.selRow[1], tileUsage, false);
	} else
		memset(&tDat[tileUsage * tileSize], 0, tileSize);
}
void tiles::remove_duplicate_tiles(bool tColor) {
	pushTilemapAll(false);
	pushTileGroupPrepare(tTypeDelete);
	char bufT[1024];
	Fl_Window *win;
	Fl_Progress *progress;
	mkProgress(&win, &progress);
	uint32_t tile_remove_c = 0;
	int32_t cur_tile, curT;
	uint8_t*tileTemp;

	if (tColor)
		tileTemp = (uint8_t *)alloca(tcSize);
	else
		tileTemp = (uint8_t *)alloca(tileSize);

	std::vector<uint32_t> remap(amt);
	time_t lastt = time(NULL);

	for (uint32_t i = 0; i < amt; ++i)
		remap[i] = i;

	for (cur_tile = 0; cur_tile < amt; ++cur_tile) {
		snprintf(bufT, 1024, "Comparing tiles with: %d", cur_tile);
		win->copy_label(bufT);

		for (curT = amt - 1; curT >= cur_tile; curT--) {
			if (cur_tile == curT)//don't compare with itself
				continue;

			bool rm;

			if (tColor)
				rm = !memcmp(&truetDat[cur_tile * tcSize], &truetDat[curT * tcSize], tcSize);
			else
				rm = !memcmp(&tDat[cur_tile * tileSize], &tDat[curT * tileSize], tileSize);

			if (rm) {
				prj->tms->maps[prj->curPlane].sub_tile_map(curT, cur_tile, false, false);
				addTileGroup(curT, remap[curT]);
				remap.erase(remap.begin() + curT);
				remove_tile_at(curT);
				tile_remove_c++;
				continue;
			}

			if (prj->supportsFlippedTiles()) {
				if (tColor) {
					hflip_truecolor(curT, (uint32_t*)tileTemp);
					rm = !memcmp(&truetDat[cur_tile * tcSize], tileTemp, tcSize);
				} else {
					hflip_tile(curT, tileTemp);
					rm = !memcmp(&tDat[cur_tile * tileSize], tileTemp, tileSize);
				}

				if (rm) {
					prj->tms->maps[prj->curPlane].sub_tile_map(curT, cur_tile, true, false);
					addTileGroup(curT, remap[curT]);
					remap.erase(remap.begin() + curT);
					remove_tile_at(curT);
					tile_remove_c++;
					continue;
				}

				if (tColor) {
					vflip_truecolor_ptr(tileTemp, tileTemp);
					rm = !memcmp(&truetDat[cur_tile * tcSize], tileTemp, tcSize);
				} else {
					vflip_tile_ptr(tileTemp, tileTemp);
					rm = !memcmp(&tDat[cur_tile * tileSize], tileTemp, tileSize);
				}

				if (rm) {
					prj->tms->maps[prj->curPlane].sub_tile_map(curT, cur_tile, true, true);
					addTileGroup(curT, remap[curT]);
					remap.erase(remap.begin() + curT);
					remove_tile_at(curT);
					tile_remove_c++;
					continue;
				}

				if (tColor) {
					vflip_truecolor(curT, tileTemp);
					rm = !memcmp(&truetDat[cur_tile * tcSize], tileTemp, tcSize);
				} else {
					vflip_tile(curT, tileTemp);
					rm = !memcmp(&tDat[cur_tile * tileSize], tileTemp, tileSize);
				}

				if (rm) {
					prj->tms->maps[prj->curPlane].sub_tile_map(curT, cur_tile, false, true);
					addTileGroup(curT, remap[curT]);
					remap.erase(remap.begin() + curT);
					remove_tile_at(curT);
					tile_remove_c++;
					continue;
				}
			}

			if ((time(NULL) - lastt) >= 1) {
				lastt = time(NULL);
				progress->value((float)cur_tile / (float)amt);
				snprintf(bufT, 1024, "Removed %d tiles", tile_remove_c);
				progress->label(bufT);
				Fl::check();
			}
		}

		progress->value((float)cur_tile / (float)amt);
		snprintf(bufT, 1024, "Removed %d tiles", tile_remove_c);
		progress->label(bufT);
		Fl::check();
	}

	remap.clear();
	printf("Removed %d tiles\n", tile_remove_c);
	win->remove(progress);// remove progress bar from window
	delete (progress); // deallocate it
	delete win;
	Fl::check();
}
void tiles::tileToTrueCol(const uint8_t*input, uint8_t*output, unsigned row, bool useAlpha, bool alphaZero) {
	row *= prj->pal->perRow * 3;

	for (unsigned y = 0; y < sizeh; ++y) {
		for (unsigned x = 0; x < sizew; ++x) {
			unsigned temp = getPixel(input, x, y);
			*output++ = prj->pal->rgbPal[row + (temp * 3)];
			*output++ = prj->pal->rgbPal[row + (temp * 3) + 1];
			*output++ = prj->pal->rgbPal[row + (temp * 3) + 2];

			if (useAlpha) {
				if (alphaZero) {
					if (temp)
						*output++ = 255;
					else
						*output++ = 0;
				} else
					*output++ = 255;
			}
		}
	}
}
void tiles::toPlanar(enum tileType tt, unsigned mi, int mx) {
	uint8_t*tmp = (uint8_t*)alloca(tileSize);
	uint8_t*tPtr = tDat.data() + (mi * tileSize);
	unsigned bdr = prj->getBitdepthSysraw();

	if (mx < 0)
		mx = amt;

	for (unsigned i = mi; i < mx; ++i) {
		uint8_t*ptr = tmp;
		memcpy(tmp, tPtr, tileSize);

		switch (tt) {
			case LINEAR:
				for (unsigned y = 0; y < sizeh; ++y) {
					for (unsigned x = 0; x < sizew; ++x) {
						unsigned val;

						switch (bdr) {
							case 3:
								if (x & 1)
									val = (*ptr++) & 15;
								else
									val = *ptr >> 4;

								break;

							case 7:
								val = *ptr++;
								break;

							default:
								val = 0;
								show_default_error
						}

						setPixel(tPtr, x, y, val);
					}
				}

				break;

			case PLANAR_LINE:
				memset(tPtr, 0, tileSize);

				for (unsigned y = 0; y < sizeh; ++y) {
					for (unsigned b = 0; b <= bdr; ++b) {
						for (unsigned x = 0; x < sizew; ++x)
							setPixel(tPtr, x, y, (((*ptr >> ((sizew - 1) - x)) & 1) << b) | getPixel(tPtr, x, y));

						++ptr;
					}
				}

				break;
		}

		tPtr += tileSize;
	}
}
void*tiles::toLinear(void) {
	void*pt = malloc(amt * tileSize);
	uint8_t*ptr = (uint8_t*)pt;
	unsigned bdr = prj->getBitdepthSysraw();
	uint8_t*tPtr = tDat.data();

	for (unsigned i = 0; i < amt; ++i) {
		for (unsigned y = 0; y < sizeh; ++y) {
			for (unsigned x = 0; x < sizew; ++x) {
				unsigned val = getPixel(tPtr, x, y);

				switch (bdr) {
					case 3:
						if (x & 1)
							*ptr++ |= val;
						else
							*ptr = val << 4;

						break;

					case 7:
						*ptr++ = val;
						break;

					default:
						show_default_error
				}
			}
		}

		tPtr += tileSize;
	}

	return pt;
}
void*tiles::toLinePlanar(void) {
	void*pt = malloc(amt * tileSize);
	uint8_t*ptr = (uint8_t*)pt;
	unsigned bdr = prj->getBitdepthSysraw();
	uint8_t*tPtr = tDat.data();

	if (sizew > 8) {
		show_TODO_error
		return 0;
	}

	for (unsigned i = 0; i < amt; ++i) {
		for (unsigned y = 0; y < sizeh; ++y) {
			for (unsigned b = 0; b <= bdr; ++b) {
				*ptr = 0;

				for (unsigned x = 0; x < sizew; ++x)
					*ptr |= ((getPixel(tPtr, x, y) >> b) & 1) << ((sizew - 1) - x);

				++ptr;
			}
		}

		tPtr += tileSize;
	}

	return pt;
}
void tiles::setDim(unsigned w, unsigned h, unsigned bd) {
	setWidth(w);
	sizeh = h;
	curBD = bd;
	tcSize = sizew * sizeh * 4;
	tileSize = sizewbytesbits * sizeh * bd / 8;
	resizeAmt();
}
void tiles::swap(unsigned first, unsigned second) {
	if (first == second)
		return;

	uint8_t*tmp = (uint8_t*)alloca(tileSize);
	uint8_t*tmpT = (uint8_t*)alloca(tcSize);
	memcpy(tmp, tDat.data() + (first * tileSize), tileSize);
	memcpy(tmpT, truetDat.data() + (first * tcSize), tcSize);
	memcpy(tDat.data() + (first * tileSize), tDat.data() + (second * tileSize), tileSize);
	memcpy(tDat.data() + (second * tileSize), tmp, tileSize);
	memcpy(truetDat.data() + (first * tcSize), truetDat.data() + (second * tcSize), tcSize);
	memcpy(truetDat.data() + (second * tcSize), tmp, tcSize);
}
void tiles::changeDim(unsigned w, unsigned h, unsigned bd) {
	if (w == sizew && h == sizeh) {
		if (curBD == bd)
			return;
		else {
			setDim(w, h, bd);
			return;
		}
	}

	unsigned amto = amt;
	amt = amt * sizew / w * sizeh / h;
	unsigned sw = sizew, sh = sizeh;

	if (sw != w || sh != h)
		std::fill(tDat.begin(), tDat.end(), 0);

	setDim(w, h, bd);

	//If going to a smaller dimension break up the tiles; discard tile data keep only truecolor data.
	if (sw > w && sh > h && (sizew % w == 0) && (sizeh % h == 0)) {
		std::unique_ptr<tiles> old(new tiles(*this, prj));
		uint8_t*src = old->truetDat.data(), *dst = truetDat.data();

		for (unsigned i = 0; i < amto; ++i) {
			for (unsigned y = 0; y < sh; ++y) {
				for (unsigned x = 0; x < sw / w; ++x)
					memcpy(dst + (x * w * 4) + (y * sw * 4), src + (x * tcSize) + (y % h * w * 4), w * 4);
			}
		}
	}

	updateTileSelectAmt();
}
void tiles::save(const char*fname, fileType_t type, bool clipboard, int compression, const char*label) {
	uint8_t*savePtr;
	enum tileType tt = prj->getTileType();

	unsigned outputTileSize;

	switch (tt) {
		case LINEAR:
			savePtr = (uint8_t*)toLinear();
			outputTileSize = (width() * height() * curBD + 8 - 1) / 8;
			break;

		case PLANAR_LINE:
			savePtr = (uint8_t*)toLinePlanar();
			outputTileSize = tileSize;
			break;

		case PLANAR_TILE:
			savePtr = tDat.data();
			outputTileSize = tileSize;
			break;

		default:
			show_default_error
	}

	FILE* myfile;
	uint8_t* compdat;
	size_t compsize;

	if (clipboard)
		myfile = nullptr;
	else if (type != fileType_t::tBinary)
		myfile = fopen(fname, "w");
	else
		myfile = fopen(fname, "wb");

	if (likely(myfile || clipboard)) {
		if (compression)
			compdat = (uint8_t*)encodeType(savePtr, outputTileSize * amt, compsize, compression);

		char comment[2048];
		snprintf(comment, 2048, "%d tiles %s", amt, typeToText(compression));

		if (compression) {
			if (!saveBinAsText(compdat, compsize, myfile, type, comment, label, 8, boost::endian::order::native)) {
				free(compdat);
				fl_alert("Error: can not save file %s", the_file.c_str());

				if (tt != PLANAR_TILE)
					free(savePtr);

				return;
			}
		} else {
			if (!saveBinAsText(savePtr, (amt)*outputTileSize, myfile, type, comment, label, 8, boost::endian::order::native)) {
				fl_alert("Error: can not save file %s", the_file.c_str());

				if (tt != PLANAR_TILE)
					free(savePtr);

				return;
			}
		}

		if (compression)
			free(compdat);

		if (tt != PLANAR_TILE)
			free(savePtr);
	} else
		fl_alert("Error: can not save file %s", the_file.c_str());

	if (myfile)
		fclose(myfile);
}
void tiles::tms9918Mode1RearrangeTiles(tileAttrMap_t& attrs, bool forceKeepAllTiles) {
	// multimap always sorts by key. This is very good for us.
	uint8_t keyHold = attrs.cbegin()->first;
	unsigned tileCount = 0;
	unsigned curTile = 0;
	std::map<uint32_t, uint32_t> tileMapping;
	std::unordered_set<uint32_t> processedTiles;

	if (forceKeepAllTiles) {
		for (auto it = attrs.cbegin(); it != attrs.cend(); ++it)
			processedTiles.emplace(it->second);

		for (unsigned i = 0; i < amt; ++i) {
			const bool tileExists = processedTiles.find(i) != processedTiles.end();

			if (!tileExists)
				attrs.emplace(getExtAttr(i, 0), i);
		}

		processedTiles.clear();
	}

	// Remove duplicate blank tiles.
	bool hasBlankTile = false; // We will remove duplicate blank tiles (defined as all pixels == 0 for their rgba values).
	unsigned blankTileIdx = 0;

	for (auto it = attrs.cbegin(); it != attrs.cend();) {
		unsigned i = it->second;

		// See if it is blank.
		if (isAllZeroTruecolor(i)) {
			if (hasBlankTile && i != blankTileIdx) {
				// We already have a blank tile. Replace all uses of this tile with blankTileIdx.
				if (prj->tms)
					prj->tms->swapTile(i, blankTileIdx);

				// Remove this tile from the list.
				it = attrs.erase(it);
				continue; // Avoid the ++it.
			} else {
				// If we find another blank tile we will use this.
				hasBlankTile = true;
				blankTileIdx = i;
			}
		}

		++it;
	}

	std::vector<uint8_t> newTileData(tDat.size());
	std::vector<uint8_t> newTruecolorData(truetDat.size());

	for (auto it = attrs.cbegin(); it != attrs.cend(); ++it) {
		if (it->second >= amt) {
			tileMapping[it->second] = it->second; // Leave invalid entries alone.
			continue;
		}

		const bool alreadyProcessed = processedTiles.find(it->second) != processedTiles.end();

		if (alreadyProcessed)
			continue;

		uint8_t key = it->first;

		if (keyHold != key) {
			unsigned extraTiles = (tileCount & 7);

			if (extraTiles) {
				extraTiles = 8 - extraTiles; // This gives us how many tiles we need to add.
				printf("Padding tiles with %d tiles at index: %d\n", extraTiles, curTile);
				resizeAmt(amt + extraTiles);
				newTileData.resize(amt * tileSize);
				newTruecolorData.resize(amt * tcSize);
				curTile += extraTiles; // Skip past these blank tiles.
			}

			tileCount = 0;
		}

		extAttrs.at(curTile / 8) = key; // Set the attribute.

		// Copy the tiles.
		uint8_t * tileDst = newTileData.data();
		tileDst += curTile * tileSize;
		const uint8_t * tileSrc = tDat.data();
		tileSrc += it->second * tileSize;
		memcpy(tileDst, tileSrc, tileSize);
		// Do the same for truecolor tiles.
		tileDst = newTruecolorData.data();
		tileDst += curTile * tcSize;
		tileSrc = truetDat.data();
		tileSrc += it->second * tcSize;
		memcpy(tileDst, tileSrc, tcSize);
		tileMapping[it->second] = curTile;

		keyHold = key;
		++tileCount;
		++curTile;

		// Don't accidentally do the same tile again if it is in the list more than once.
		// This happens when the tile is on a tilemap more than once.
		processedTiles.emplace(it->second);
	}

	tDat = newTileData;
	truetDat = newTruecolorData;

	resizeAmt(curTile); // curTile contains the final amount of tiles.

	if (prj->tms) {
		for (unsigned i = 0; i < prj->tms->maps.size(); ++i) {
			class tileMap* mapClass = &prj->tms->maps[i];

			if (mapClass) {
				for (unsigned y = 0; y < mapClass->mapSizeHA; ++y) {
					for (unsigned x = 0; x < mapClass->mapSizeW; ++x) {
						unsigned inputTile = mapClass->get_tile(x, y);

						try {
							mapClass->set_tile(x, y, tileMapping.at(inputTile));
						} catch (...) {
							fl_alert("Cannot map tile %d at (%d, %d) in %d", inputTile, x, y, i);
						}
					}
				}
			}
		}
	}

	if (window && prj == currentProject) {
		// Update the currently shown tile.
		try {
			current_tile = tileMapping.at(current_tile);
		} catch (...) {}

		try {
			window->tile_select->value(tileMapping.at(window->tile_select->value()));
		} catch (...) {}

		try {
			window->tile_select_2->value(tileMapping.at(window->tile_select_2->value()));
		} catch (...) {}

		window->redraw();
	}
}
bool tiles::isAllZeroTruecolor(unsigned idx) {
	uint32_t*ptr = (uint32_t*)getPixelPtrTC(idx, 0, 0);

	if (tcSize & 3) {
		fl_alert("tcSize & 4. Please fix isAllZeroTruecolor.");
		return false; // Don't know if it is all zeros or not. Assuming that it is not.
	}

	for (unsigned i = 0; i < tcSize / 4; ++i)
		if (*ptr++)
			return false;

	return true;

}
