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

#include <exception>

#include "macros.h"
#include "callback_chunk.h"
#include "filemisc.h"
#include "undo.h"
#include "gamedef.h"
#include "gui.h"
#include "class_global.h"
#include "compressionWrapper.h"
#include "filereader.h"
ChunkClass::ChunkClass(Project*prj) {
	this->prj = prj;
	chunks.resize(256);
	amt = 1;
	wi = hi = 16; //16*16=256
	useBlocks = false;
	usePlane = 0;
}
ChunkClass::ChunkClass(const ChunkClass& other, Project*prj) {
	this->prj = prj;
	usePlane = other.usePlane;
	wi = other.wi;
	hi = other.hi;
	amt = other.amt;
	useBlocks = other.useBlocks;
	chunks = other.chunks;
}
ChunkClass::~ChunkClass(void) {
	chunks.clear();
}
void ChunkClass::insert(uint32_t at) {
	struct ChunkAttrs tmp;
	memset(&tmp, 0, sizeof(tmp));
	chunks.insert(chunks.begin() + (at * sizeOfChunk()), sizeOfChunk(), tmp);
	++amt;
}
void ChunkClass::setElm(uint32_t id, uint32_t x, uint32_t y, struct ChunkAttrs c) {
	struct ChunkAttrs*ch = chunks.data() + (id * wi * hi) + (y * wi) + x;
	ch->flags = c.flags;
	ch->block = c.block;
}
struct ChunkAttrs ChunkClass::getElm(uint32_t id, uint32_t x, uint32_t y)const {
	return chunks[(id * wi * hi) + (y * wi) + x];
}
void ChunkClass::removeAt(uint32_t at) {
	if (amt < 2) {
		fl_alert("If you don't want chunks uncheck \"have chunks\" instead of deleting");
		return;
	}

	try {
		chunks.erase(chunks.begin() + (at * wi * hi), chunks.begin() + ((at + 1)*wi * hi));
	} catch (std::exception&e) {
		fl_alert("Error cannot remove tile %d\nAdditional details: %s", at, e.what());
		exit(1);
	}

	--amt;

	if (window && window->chunk_select->value() >= amt) {
		window->chunk_select->value(amt - 1);
		currentChunk = amt - 1;
	}
}
void ChunkClass::resizeAmt(uint32_t amtnew) {
	chunks.resize(amtnew * wi * hi);
	amt = amtnew;

	if (window && window->chunk_select->value() >= amt) {
		window->chunk_select->value(amt - 1);
		currentChunk = amt - 1;
	}
}
void ChunkClass::resizeAmt(void) {
	resizeAmt(amt);
}

void ChunkClass::getXYblock(unsigned id, unsigned x, unsigned y, unsigned& xo, unsigned& yo)const {
	xo = x % prj->tms->maps[usePlane].mapSizeW;
	yo = (chunks[(id * wi * hi) + (y * wi / prj->tms->maps[usePlane].mapSizeH) + (x / prj->tms->maps[usePlane].mapSizeW)].block
	      * prj->tms->maps[usePlane].mapSizeH)
	     + (y % prj->tms->maps[usePlane].mapSizeH);
}
bool ChunkClass::getPrio_t(uint32_t id, uint32_t x, uint32_t y)const { //The _t means based on tiles not blocks
	if (useBlocks) {
		getXYblock(id, x, y, x, y);
		return prj->tms->maps[usePlane].get_prio(x, y);
	} else
		return ((getFlag(id, x, y) >> 2) & 1) ? true : false;
}
uint8_t ChunkClass::getTileRow_t(uint32_t id, uint32_t x, uint32_t y)const {
	if (useBlocks) {
		getXYblock(id, x, y, x, y);
		return prj->tms->maps[usePlane].getPalRow(x, y);
	} else
		return (getFlag(id, x, y) >> 3) & 3;
}

unsigned ChunkClass::getTile_t(uint32_t id, uint32_t x, uint32_t y)const {
	if (useBlocks) {
		getXYblock(id, x, y, x, y);
		return prj->tms->maps[usePlane].get_tile(x, y);
	} else
		return (getFlag(id, x, y) >> 3) & 3;
}
unsigned ChunkClass::getSolid(uint32_t id, uint32_t x, uint32_t y)const {
	unsigned shift;

	if (useBlocks)
		shift = 2;
	else
		shift = 5;

	return (chunks[getOff(id, x, y)].flags >> shift) & 3;
}
uint32_t ChunkClass::getBlock(uint32_t id, uint32_t x, uint32_t y)const {
	return chunks[getOff(id, x, y)].block;
}
bool ChunkClass::getHflip(uint32_t id, uint32_t x, uint32_t y)const {
	return chunks[getOff(id, x, y)].flags & 1;
}
bool ChunkClass::getVflip(uint32_t id, uint32_t x, uint32_t y)const {
	return (chunks[getOff(id, x, y)].flags & 2) >> 1;
}
unsigned ChunkClass::getOff(const uint32_t id, const uint32_t x, const uint32_t y)const {
	return (id * wi * hi) + (y * wi) + x;
}
bool ChunkClass::getPrio(uint32_t id, uint32_t x, uint32_t y)const {
	return (chunks[getOff(id, x, y)].flags & 4) >> 2;
}
void ChunkClass::setBlock(uint32_t id, uint32_t x, uint32_t y, uint32_t block) {
	chunks[getOff(id, x, y)].block = block;
	/*This contains which block/tile to use*/
}
void ChunkClass::setFlag(uint32_t id, uint32_t x, uint32_t y, uint32_t flag) {
	chunks[getOff(id, x, y)].flags = flag;
	/*!If not using blocks flags will contain the following
	bit 0 hflip
	bit 1 vflip
	bit 2 priority
	bit 3,4 palette row
	All other bits are unused and can be used for video game usage
	If using blocks flags will simply contain video game settings
	Here are video game settings used. If using tiles instead of blocks add 3 to bit count and ignore x and y flip
	bit 0 x-flip
	bit 1 y-flip
	bit 2,3 solidity 00 means not solid, 01 means top solid, 10 means left/right/bottom solid, and 11 means all solid.
	*/
}
uint32_t ChunkClass::getFlag(uint32_t id, uint32_t x, uint32_t y)const {
	return chunks[getOff(id, x, y)].flags;
}
void ChunkClass::setSolid(uint32_t id, uint32_t x, uint32_t y, unsigned solid) {
	unsigned shift;

	if (useBlocks)
		shift = 2;
	else
		shift = 5;

	unsigned off = getOff(id, x, y);
	chunks[off].flags &= ~(3 << shift);
	chunks[off].flags |= solid << shift;
}
void ChunkClass::setHflip(uint32_t id, uint32_t x, uint32_t y, bool hflip) {
	unsigned off = getOff(id, x, y);

	if (hflip)
		chunks[off].flags |= 1;
	else
		chunks[off].flags &= ~1;
}
void ChunkClass::setVflip(uint32_t id, uint32_t x, uint32_t y, bool vflip) {
	unsigned off = getOff(id, x, y);

	if (vflip)
		chunks[off].flags |= 2;
	else
		chunks[off].flags &= ~2;
}
void ChunkClass::setPrio(uint32_t id, uint32_t x, uint32_t y, bool prio) {
	unsigned off = getOff(id, x, y);

	if (prio)
		chunks[off].flags |= 4;
	else
		chunks[off].flags &= ~4;
}
void ChunkClass::drawChunk(uint32_t id, int xo, int yo, int zoom, int scrollX, int scrollY) {
	if (!window)
		return;

	struct ChunkAttrs * cptr = chunks.data();

	for (uint32_t y = scrollY; y < hi; ++y) {
		cptr = &chunks[(id * wi * hi) + (y * wi) + scrollX];
		int xoo = xo;

		for (uint32_t x = scrollX; x < wi; ++x) {
			if (useBlocks) {
				prj->tms->maps[usePlane].drawBlock(cptr->block, xoo, yo, cptr->flags & 3, zoom);
				xoo += prj->tms->maps[usePlane].mapSizeW * prj->tileC->width() * zoom;

			} else {
				prj->tileC->draw_tile(xoo, yo, cptr->block, zoom, (cptr->flags >> 3) & 3, cptr->flags & 1, (cptr->flags >> 1) & 1);
				xoo += prj->tileC->width() * zoom;
			}

			cptr++;

			if ((xoo) > (window->w()))
				break;
		}

		if (useBlocks)
			yo += prj->tileC->height() * zoom * prj->tms->maps[usePlane].mapSizeH;
		else
			yo += prj->tileC->height() * zoom;

		if (yo > (window->h()))
			break;
	}
}
void ChunkClass::scrollChunks(void) {
	if (!window)
		return;

	unsigned oldS = window->chunkX->value();
	int zoom = window->chunk_tile_size->value();
	int off;

	if (useBlocks)
		off = (wi * prj->tms->maps[usePlane].mapSizeW) - ((window->w() - ChunkOff[0]) / (zoom * prj->tileC->width()));
	else
		off = wi - ((window->w() - ChunkOff[0]) / (zoom * prj->tileC->width()));

	if (oldS > off)
		scrollChunks_G[0] = oldS = off;

	if (off > 0) {
		window->chunkX->show();
		window->chunkX->value(oldS, 1, 0, off + 2);
	} else
		window->chunkX->hide();

	oldS = window->chunkY->value();

	if (useBlocks)
		off = (hi * prj->tms->maps[usePlane].mapSizeH) - ((window->h() - ChunkOff[1]) / (zoom * prj->tileC->height()));
	else
		off = hi - ((window->h() - ChunkOff[1]) / (zoom * prj->tileC->height()));

	if (oldS > off)
		scrollChunks_G[1] = oldS = off;

	if (off > 0) {
		window->chunkY->show();
		window->chunkY->value(oldS, 1, 0, off + 2);
	} else
		window->chunkY->hide();
}

static void errorNum(void) {
	fl_alert("Please enter a value greater than zero.");
}

void ChunkClass::importSonic1(bool append) {
	if (fl_ask("Custom width and height?")) {
		char*ptr = (char*)fl_input("Width");

		if (!ptr)
			return;

		if (!verify_str_number_only(ptr))
			return;

		int witmp = atoi(ptr);

		if (witmp <= 0) {
			errorNum();
			return;
		}

		ptr = (char*)fl_input("Height");

		if (!ptr)
			return;

		if (!verify_str_number_only(ptr))
			return;

		int hitmp = atoi(ptr);

		if (hitmp <= 0) {
			errorNum();
			return;
		}

		if (append)
			resize(witmp, hitmp);

		wi = witmp;
		hi = hitmp;
	} else
		wi = hi = 16;

	pushChunksAll();
	uint16_t* dat;
	size_t fileSize;
	filereader f = filereader(boost::endian::order::big, 2, "Select a Sonic One chunk file");
	unsigned i = f.selDat();

	dat = (uint16_t*)f.dat[i];
	fileSize = f.lens[i];

	uint32_t off;

	if (append)
		off = amt;
	else
		off = 0;

	if (window)
		window->updateChunkSize(wi, hi);

	amt = (fileSize / (wi * hi * 2)) + off;
	chunks.resize(amt * wi * hi);
	struct ChunkAttrs*cptr = chunks.data();
	cptr += off * wi * hi;
	uint16_t * datC = dat;

	for (uint32_t l = 0; l < (fileSize / (wi * hi * 2)); ++l) {
		for (uint32_t y = 0; y < hi; ++y) {
			for (uint32_t x = 0; x < wi; ++x) {
				cptr->block = *datC & 1023;
				cptr->flags = (*datC >> 11) & 15;
				++cptr;
				++datC;
			}
		}
	}
}
void ChunkClass::exportSonic1(void)const {
	FILE*fp;
	int clipboard;
	fileType_t type = askSaveType();
	size_t fileSize;

	if (type != fileType_t::tBinary) {
		clipboard = clipboardAsk();

		if (clipboard == 2)
			return;
	} else
		clipboard = 0;

	bool pickedFile;

	std::string the_file;

	if (clipboard)
		pickedFile = true;
	else
		pickedFile = loadOrSaveFile(the_file, "Save tilemap to", true);

	if (pickedFile) {
		CompressionType compression = compressionAsk();

		if (compression == CompressionType::Cancel)
			return;

		if (clipboard)
			fp = 0;
		else if (type != fileType_t::tBinary)
			fp = fopen(the_file.c_str(), "w");
		else
			fp = fopen(the_file.c_str(), "wb");

		if (likely(fp || clipboard)) {
			const struct ChunkAttrs*cptr = chunks.data();
			uint16_t*tmp = (uint16_t*)malloc(wi * hi * 2 * amt);
			uint16_t*ptmp = tmp;
			fileSize = wi * hi * amt * 2;

			for (uint32_t i = 0; i < wi * hi * amt; ++i) {
				uint32_t temp = cptr->block;

				if (temp > 1023) {
					printf("Block overflow %d\n", temp);
					temp = 1023;
				}

				temp |= (cptr->flags & 15) << 11;
				*ptmp++ = temp;
				++cptr;
			}

			if (compression != CompressionType::Uncompressed) {
				void*tmpold = tmp;
				// The endian must be corrected before compressing.
				uint16_t * ptr = (uint16_t*)tmp;

				for (unsigned i = 0; i < wi * hi * amt; ++i) {
					uint16_t tmp = *ptr;
					boost::endian::conditional_reverse_inplace<boost::endian::order::native, boost::endian::order::big>(tmp);
					*ptr++ = tmp;
				}

				tmp = (uint16_t*)encodeType(tmp, fileSize, fileSize, compression);
				free(tmpold);
			}

			char temp[2048];
			snprintf(temp, 2048, "Width: %d Height: %d Amount: %d %s", wi, hi, amt, typeToText(compression));

			if (!saveBinAsText(tmp, fileSize, fp, type, temp, "mapDat", compression != CompressionType::Uncompressed ? 8 : 16,
			                   compression != CompressionType::Uncompressed ? boost::endian::order::native : boost::endian::order::big)) {
				free(tmp);
				return;
			}

			free(tmp);

			if (fp)
				fclose(fp);
		}
	}
}

void ChunkClass::resize(uint32_t wnew, uint32_t hnew) {
	if ((wnew == wi) && (hnew == hi))
		return;

	struct ChunkAttrs*tmp = (struct ChunkAttrs*)malloc(sizeof(struct ChunkAttrs) * wi * hi * amt);
	memcpy(tmp, chunks.data(), sizeof(struct ChunkAttrs)*wi * hi * amt);
	chunks.resize(amt * wnew * hnew);
	struct ChunkAttrs*cptr = chunks.data(), *tptr = tmp;

	for (uint32_t z = 0; z < amt; ++z) {
		for (uint32_t y = 0; y < std::min(hi, hnew); ++y) {
			if (wnew > wi) {
				memcpy(cptr, tptr, wi * sizeof(struct ChunkAttrs));
				memset(cptr + wi, 0, (wnew - wi)*sizeof(struct ChunkAttrs));
			} else
				memcpy(cptr, tptr, wnew * sizeof(struct ChunkAttrs));

			cptr += wnew;
			tptr += wi;
		}

		if (hnew > hi) {
			for (uint32_t y = hi; y < hnew; ++y) {
				memset(cptr, 0, wnew * sizeof(struct ChunkAttrs));
				cptr += wnew;
			}
		} else
			tptr += (hi - hnew) * wi;
	}

	wi = wnew;
	hi = hnew;
	free(tmp);
	scrollChunks();
}
void ChunkClass::subBlock(unsigned oid, unsigned nid) {
	uint_fast32_t x, y, i;
	int_fast32_t temp;

	for (i = 0; i < amt; ++i) {
		for (y = 0; y < hi; ++y) {
			for (x = 0; x < wi; ++x) {
				temp = getBlock(i, x, y);

				if (temp == oid)
					setBlock(i, x, y, nid);
				else if (temp > oid) {
					temp--;

					if (temp < 0)
						temp = 0;

					setBlock(i, x, y, temp);
				}
			}
		}
	}
}
