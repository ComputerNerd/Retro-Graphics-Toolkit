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
#include <FL/Fl_Browser.H>
#include <FL/fl_ask.H>

#include <algorithm>
#include <cmath>//For some reason this is needed when compiling with mingw otherwise hypot error is encountered
#include <stdlib.h>
#include <utility>
#include "system.h"
#include "project.h"
#include "undo.h"
#include "color_convert.h"
#include "callback_chunk.h"
#include "callbacksprites.h"
#include "classpalettebar.h"
#include "callbacktilemaps.h"
#include "class_global.h"
#include "gui.h"
#include "errorMsg.h"

enum undoTypes_t {
	uTile = 0,
	uTileAll,
	uTileGroup,
	uTilePixel,
	uTileAppend,//No struct
	uTileAppendgroupdat,//uTileGroup could be adapted for this but instead we use separate code in order to save ram.
	uTileNew,//No struct reuses ptr for insert tile after
	uTilemap,
	uTilemapattr,
	uTilemapEdit,
	uTilemapResize,
	uTilemapBlocksAmt,
	uTilemapPlaneDelete,
	uTilemapPlaneAdd,
	uPalette,
	uPaletteEntry,
	uChunk,
	uChunkDelete,
	uChunkAll,
	uChunkEdit,
	uChunkResize,
	uChunkAppend,
	uChunkNew,//No struct reuses ptr
	uSpriteNew,
	uSpriteNewgroup,//No struct reuses ptr
	uSpriteAppend,
	uSpriteAppendgroup,//No struct reuses ptr
	uSpriteAppendmeta,//No struct
	uSpriteWidth,
	uSpriteHeight,
	uSpritePalrow,
	uSpritestarttile,
	uSpriteloadat,
	uSpriteoffx,
	uSpriteoffy,
	uSpriteprio,
	uSpritehflip,
	uSpritevflip,
	uSpriteGroupDel,
	uSpriteDel,
	uSpriteAll,
	uCurProject,//For both system switches and project loads
	uProjectAppend,//no struct
	uProjectAll,
};
struct undoEvent { //This struct only holds which type of undo this is and one value if ptr is not used.
	undoTypes_t type;
	uint32_t prjId;//Project affected
	union {
		void*ptr;
		uint32_t vu;
		int32_t vi;
	};
};
struct undoTile { //The purpose of this struct is to completely undo a tile
	tileTypeMask_t type;
	uint32_t id;
	void*ptr;//when type is both first the truecolor tile will be stored then the regular tile
};
struct undoTileAll {
	tileTypeMask_t type;
	uint32_t amt;
	void*ptr;
};
struct undoTileGroup {
	tileTypeMask_t type;
	std::vector<uint32_t> lst; // Which tiles were changed.
	std::vector<uint8_t> data; // Similar situation to other tile structs as in what this contains and what order
};
struct undoAppendgroupdat {
	uint32_t amt;
	std::vector<uint8_t> dat;
	std::vector<uint8_t> truedat;
};
struct undoTilePixel {
	tileTypeMask_t type;
	uint32_t id, x, y, val;
};
struct undoTilemap { //For undoing the entire tilemap
	uint32_t plane;
	uint32_t w, h; //The width and height
	void*ptr;//Points to tilemap data that is w*h*4 bytes or attributes if so size is w*h
};
struct undoTilemapEdit {
	uint32_t plane;
	uint32_t x, y, val;
};
struct undoTilemapPlane {
	tileMap*old;
	uint32_t plane;
};
struct undoResize {
	uint32_t plane;
	uint32_t w, h, wnew, hnew; //Old width and height
	void*ptr;//Contains a pointer ONLY TO LOST DATA
};
struct undoPalette {
	void*ptr;
};
struct undoPaletteEntry {
	uint32_t id;
	paletteRawValue_t val;
};
struct undoChunkEdit {
	uint32_t id, x, y;
	struct ChunkAttrs val;
};
struct undoChunk {
	uint32_t id;
	struct ChunkAttrs*ptr;
};
struct undoChunkAll {
	uint32_t w, h, wnew, hnew; //The width and height
	uint32_t amt, amtnew;
	struct ChunkAttrs*ptr;
};
struct undoSpriteVal {
	uint32_t metaid, id, subid;
	uint32_t val;
};

struct undoSpriteValInt32 {
	uint32_t metaid, id, subid;
	int32_t val;
};

struct undoSpriteValbool {
	uint32_t metaid, id, subid;
	bool val;
};
struct undoSpriteDel {
	uint32_t metaid, id, subid;
	class sprite sp;
	int32_t offx, offy;
	uint32_t loadat;
};
struct undoSpriteGroupDel {
	uint32_t id[2];
	struct spriteGroup;
};
struct undoSpriteAppend {
	uint32_t id[2];
};
struct undoProject {
	size_t id;
	Project* ptr;
};
struct undoProjectAll {
	std::vector<struct Project> old;
};
static struct undoEvent*undoBuf;
static uint_fast32_t amount;
static uint_fast32_t memUsed;
static uint_fast32_t maxMen = 16 * 1024 * 1024; //Limit undo buffer to 16Mb this is better than limiting by depth as each item varies in size
static int_fast32_t pos = -1;
void showMemUsageUndo(Fl_Widget*, void*) {
	fl_alert("May not be accurate\nThe undo stack currently uses %u bytes of ram not including any overhead\nAmount of items %d", (unsigned)memUsed, (int)amount);
}
static void resizeArray(uint32_t amt) {
	if (undoBuf) {
		if (amt)
			undoBuf = (struct undoEvent*)realloc(undoBuf, amt * sizeof(struct undoEvent));
		else {
			free(undoBuf);
			undoBuf = 0;
		}
	} else {
		if (amt)
			undoBuf = (struct undoEvent*)malloc(amt * sizeof(struct undoEvent));
	}
}
static unsigned getSzTile(tileTypeMask_t type) {
	unsigned sz = 0;

	if (type & tTypeTile)
		sz += currentProject->tileC->tileSize;

	if (type & tTypeTruecolor)
		sz += currentProject->tileC->tcSize;

	return sz;
}
static uint32_t getSzResizeGeneric(uint32_t w, uint32_t h, uint32_t wnew, uint32_t hnew, uint32_t szelm, uint32_t n) { //szelm is the size per element of what you are resizing
	if ((w > wnew) || (h > hnew)) {
		uint32_t tmp = 0;

		if (w > wnew)
			tmp = (w - wnew) * hnew * szelm * n;

		if (h > hnew)
			tmp += (h - hnew) * w * szelm * n;

		return tmp;
	} else
		return 0;
}
static void cleanupEvent(uint32_t id) {
	struct undoEvent*uptr = undoBuf + id;

	switch (uptr->type) {
		case uTile:
		{	struct undoTile*ut = (struct undoTile*)uptr->ptr;
			unsigned sz = getSzTile(ut->type);
			free(ut->ptr);
			memUsed -= sz;

			free(uptr->ptr);
			memUsed -= sizeof(struct undoTile);
		}
		break;

		case uTilePixel:
			free(uptr->ptr);
			memUsed -= sizeof(struct undoTilePixel);
			break;

		case uTileAll:
		{	struct undoTileAll*ut = (struct undoTileAll*)uptr->ptr;
			unsigned sz = getSzTile(ut->type) * currentProject->tileC->amt;
			free(ut->ptr);
			memUsed -= sz;

			free(uptr->ptr);
			memUsed -= sizeof(struct undoTileAll);
		}
		break;

		case uTileNew:
		case uTileAppend:
		case uChunkAppend:
		case uChunkNew:
		case uSpriteNewgroup:
		case uSpriteAppendgroup:
		case uSpriteAppendmeta:
		case uProjectAppend:
			//Nothing to do here
			break;

		case uSpriteAppend:
			memUsed -= sizeof(struct undoSpriteAppend);
			free(uptr->ptr);
			break;

		case uTileGroup:
		{	struct undoTileGroup*ut = (struct undoTileGroup*)uptr->ptr;
			unsigned sz = getSzTile(ut->type) * ut->lst.size();
			ut->data.clear();
			memUsed -= sz;

			delete uptr->ptr;
			memUsed -= sizeof(struct undoTileGroup);
		}
		break;

		case uTileAppendgroupdat:
		{	struct undoAppendgroupdat*ut = (struct undoAppendgroupdat*)uptr->ptr;
			unsigned sz = getSzTile(tTypeBoth) * ut->amt;
			ut->dat.clear();
			ut->truedat.clear();
			delete uptr->ptr;
			memUsed -= sizeof(struct undoAppendgroupdat);
		}
		break;

		case uTilemapEdit:
			free(uptr->ptr);
			memUsed -= sizeof(struct undoTilemapEdit);
			break;

		case uTilemap:
		case uTilemapattr:
		{	struct undoTilemap*um = (struct undoTilemap*)uptr->ptr;
			uint_fast32_t sz = um->w * um->h;

			if (uptr->type == uTilemap)
				sz *= 4;

			free(um->ptr);
			memUsed -= sz;


			free(uptr->ptr);
			memUsed -= sizeof(undoTilemap);
		}
		break;

		case uTilemapResize:
		case uTilemapBlocksAmt:
		{	struct undoResize*um = (struct undoResize*)uptr->ptr;

			if (um->ptr) {
				free(um->ptr);
				memUsed -= getSzResizeGeneric(um->w, um->h, um->wnew, um->hnew, 4, 1);
			}

			free(uptr->ptr);
			memUsed -= sizeof(struct undoResize);
		}
		break;

		case uTilemapPlaneDelete:
		case uTilemapPlaneAdd:
		{	struct undoTilemapPlane*um = (struct undoTilemapPlane*)uptr->ptr;

			if (um->old) {
				memUsed -= um->old->mapSizeW * um->old->mapSizeHA * TileMapSizePerEntry;
				delete um->old;
			}

			free(uptr->ptr);
			memUsed -= sizeof(struct undoTilemapPlane);
		}
		break;

		case uPalette:
		{	struct undoPalette*up = (struct undoPalette*)uptr->ptr;
			unsigned sz = currentProject->pal->totalColors();
			sz *= currentProject->pal->esize;
			free(up->ptr);
			memUsed -= sz;

			free(uptr->ptr);
			memUsed -= sizeof(struct undoPalette);
		}
		break;

		case uPaletteEntry:
			free(uptr->ptr);
			memUsed -= sizeof(struct undoPaletteEntry);
			break;

		case uChunkResize:
		{	struct undoResize*um = (struct undoResize*)uptr->ptr;

			if (um->ptr) {
				free(um->ptr);
				memUsed -= getSzResizeGeneric(um->w, um->h, um->wnew, um->hnew, sizeof(struct ChunkAttrs), currentProject->Chunk->amt);
			}

			free(uptr->ptr);
			memUsed -= sizeof(struct undoResize);
		}
		break;

		case uChunkEdit:
			free(uptr->ptr);
			memUsed -= sizeof(struct undoChunkEdit);
			break;

		case uChunkAll:
		{	struct undoChunkAll*uc = (struct undoChunkAll*)uptr->ptr;
			free(uc->ptr);
			memUsed -= uc->w * uc->h * uc->amt * sizeof(struct ChunkAttrs);

			free(uptr->ptr);
			memUsed -= sizeof(struct undoChunkAll);
		}
		break;

		case uChunk:
		case uChunkDelete:
		{	struct undoChunk*uc = (struct undoChunk*)uptr->ptr;
			free(uc->ptr);
			memUsed -= currentProject->Chunk->wi * currentProject->Chunk->hi * sizeof(struct ChunkAttrs);

			free(uptr->ptr);
			memUsed -= sizeof(struct undoChunk);
		}
		break;

		case uSpriteWidth:
		case uSpriteHeight:
		case uSpritePalrow:
		case uSpritestarttile:
		case uSpriteloadat:
		case uSpriteoffx:
		case uSpriteoffy:
		{	struct undoSpriteVal*uc = (struct undoSpriteVal*)uptr->ptr;
			free(uptr->ptr);
			memUsed -= sizeof(struct undoSpriteVal);
		}
		break;

		case uSpriteprio:
		case uSpritehflip:
		case uSpritevflip:
		{	struct undoSpriteValbool*uc = (struct undoSpriteValbool*)uptr->ptr;
			free(uptr->ptr);
			memUsed -= sizeof(struct undoSpriteValbool);
		}
		break;

		case uCurProject:
		{	struct undoProject*up = (struct undoProject*)uptr->ptr;

			if (up->ptr)
				delete up->ptr;

			delete uptr->ptr;
			memUsed -= sizeof(struct undoProject);
		}
		break;

		case uProjectAll:
		{	struct undoProjectAll*up = (struct undoProjectAll*)uptr->ptr;

			up->old.clear();
			delete uptr->ptr;
			memUsed -= sizeof(struct undoProjectAll);
		}
		break;
	}
}
void clearUndoCB(Fl_Widget*, void*) {
	if (fl_ask("Warning this action cannot be undone.\nAre you sure that you want to do this?")) {
		for (uint_fast32_t i = 0; i < amount; ++i) {
			cleanupEvent(i);
			memUsed -= sizeof(struct undoEvent);
		}

		amount = 0;
		pos = -1;
		compactPrjMem();
	}
}
static void pushEventPrepare(void) {
	++pos;

	if ((pos <= amount) && amount) {
		for (uint_fast32_t i = pos; i < amount; ++i) {
			cleanupEvent(i);
			memUsed -= sizeof(struct undoEvent);
		}
	}

	amount = pos;
	resizeArray(++amount);
	struct undoEvent*uptr = undoBuf + pos;
	uptr->prjId = curProjectID;
	memUsed += sizeof(struct undoEvent);
}
static void tilesTo(uint8_t*ptr, uint32_t id, tileTypeMask_t type) {
	if (type & tTypeTile) {
		memcpy(ptr, currentProject->tileC->tDat.data() + (id * currentProject->tileC->tileSize), currentProject->tileC->tileSize);
		ptr += currentProject->tileC->tileSize;
	}

	if (type & tTypeTruecolor)
		memcpy(ptr, currentProject->tileC->truetDat.data() + (id * currentProject->tileC->tcSize), currentProject->tileC->tcSize);
}
static void tilesToU(uint8_t*ptr, uint32_t id, tileTypeMask_t type) {
	if (type & tTypeTile) {
		memcpy(currentProject->tileC->tDat.data() + (id * currentProject->tileC->tileSize), ptr, currentProject->tileC->tileSize);
		ptr += currentProject->tileC->tileSize;
	}

	if (type & tTypeTruecolor)
		memcpy(currentProject->tileC->truetDat.data() + (id * currentProject->tileC->tcSize), ptr, currentProject->tileC->tcSize);
}
static void cpyAllTiles(uint8_t*ptr, unsigned amt, tileTypeMask_t type) {
	if (type & tTypeTile) {
		memcpy(ptr, currentProject->tileC->tDat.data(), amt * currentProject->tileC->tileSize);
		ptr += amt * currentProject->tileC->tileSize;
	}

	if (type & tTypeTruecolor)
		memcpy(ptr, currentProject->tileC->truetDat.data(), amt * currentProject->tileC->tcSize);
}
static void cpyAllTilesU(uint8_t*ptr, unsigned amt, tileTypeMask_t type) {
	if (amt != currentProject->tileC->amt) {
		currentProject->tileC->resizeAmt(amt);
		updateTileSelectAmt();
	}

	if (type & tTypeTile) {
		memcpy(currentProject->tileC->tDat.data(), ptr, amt * currentProject->tileC->tileSize);
		ptr += amt * currentProject->tileC->tileSize;
	}

	if (type & tTypeTruecolor)
		memcpy(currentProject->tileC->truetDat.data(), ptr, amt * currentProject->tileC->tcSize);
}
static void attrCpy(uint8_t*dst, uint8_t*src, uint_fast32_t n) {
	while (n--) {
		*dst++ = *src;
		src += 4;
	}
}
static void attrCpyU(uint8_t*dst, uint8_t*src, uint_fast32_t n) {
	while (n--) {
		*dst = *src++;
		dst += 4;
	}
}
static void cpyResizeGeneric(uint8_t*dst, uint8_t*src, uint32_t w, uint32_t h, uint32_t wnew, uint32_t hnew, uint32_t szelm, uint32_t n, bool reverse) {
	if ((w > wnew) || (h > hnew)) {
		while (n--) {
			if (w > wnew) {
				for (uint32_t i = 0; i < std::min(hnew, h); ++i) {
					src += wnew * szelm;

					if (reverse)
						memcpy(src, dst, (w - wnew)*szelm);
					else
						memcpy(dst, src, (w - wnew)*szelm);

					dst += (w - wnew) * szelm;
					src += (w - wnew) * szelm;
				}
			} else
				src += w * std::min(hnew, h) * szelm;

			if (h > hnew) {
				for (uint32_t i = hnew; i < h; ++i) {
					if (reverse)
						memcpy(src, dst, w * szelm);
					else
						memcpy(dst, src, w * szelm);

					dst += w * szelm;
					src += w * szelm;
				}
			}
		}
	}
}

#define mkSpritePop_(strt, which) {struct strt*us=(struct strt*)uptr->ptr; \
	std::swap(currentProject->ms->sps[us->metaid].groups[us->id].list[us->subid].which, us->val); \
	window->updateSpriteSliders();}

#define mkSpritePop(which) mkSpritePop_(undoSpriteVal, which)
#define mkSpritePopInt32(which) mkSpritePop_(undoSpriteValInt32, which)
#define mkSpritePopbool(which) mkSpritePop_(undoSpriteValbool, which)

static void isCorrectPlane(uint32_t plane) {
	if (plane != currentProject->curPlane) {
		window->planeSelect->value(plane);
		setCurPlaneTilemaps(0, (void*)(uintptr_t)plane);
		window->redraw();
	}
}
static void removePlane(uint32_t plane) {
	currentProject->tms->removePlane(plane);

	if (currentProject->curPlane == plane) {
		if (currentProject->curPlane)
			--currentProject->curPlane;
		else
			++currentProject->curPlane;

		window->planeSelect->value(currentProject->curPlane);
		updatePlaneTilemapMenu();
		setCurPlaneTilemaps(0, (void*)(uintptr_t)currentProject->curPlane);
	} else
		updatePlaneTilemapMenu();
}
static bool shouldChangePrj(undoTypes_t t) {
	switch (t) {
		case uCurProject:
		case uProjectAppend:
		case uProjectAll:
			return false;
			break;

		default:
			return true;
	}
}
static void UndoRedo(bool redo) {
	if ((pos < 0) && (!redo))
		return;

	if (!amount)
		return;

	if (redo && (pos >= (int_fast32_t(amount) - 1)))
		return;

	if (redo && (pos <= int_fast32_t(amount)))
		++pos;

	struct undoEvent*uptr = undoBuf + pos;

	if (shouldChangePrj(uptr->type) && (uptr->prjId != curProjectID))
		switchProjectSlider(uptr->prjId);

	switch (uptr->type) {
		case uTile:
		{
			struct undoTile*ut = (struct undoTile*)uptr->ptr;

			if (ut->type & tTypeDeleteFlag) {
				if (redo)
					currentProject->tileC->remove_tile_at(ut->id);
				else {
					currentProject->tileC->insertTile(ut->id);
					// Restore the deleted data.
					tilesToU((uint8_t*)ut->ptr, ut->id, ut->type);
				}

				updateTileSelectAmt();
			} else {
				// Swap the data stored in ut->id with ut->ptr.
				std::unique_ptr<uint8_t[]> tmpBuf(new uint8_t[getSzTile(ut->type)]);
				tilesTo(&tmpBuf[0], ut->id, ut->type);
				tilesToU((uint8_t*)ut->ptr, ut->id, ut->type);
			}
		}
		break;

		case uTilePixel:
		{
			struct undoTilePixel*ut = (struct undoTilePixel*)uptr->ptr;

			if (ut->type == tTypeTruecolor) {
				uint32_t tmp = currentProject->tileC->getPixelTc(ut->id, ut->x, ut->y);
				currentProject->tileC->setPixelTc(ut->id, ut->x, ut->y, ut->val);
				ut->val = tmp;
			} else {
				uint32_t tmp = currentProject->tileC->getPixel(ut->id, ut->x, ut->y);
				currentProject->tileC->setPixel(ut->id, ut->x, ut->y, ut->val);
				ut->val = tmp;
			}
		}
		break;

		case uTileAll:
		{
			struct undoTileAll*ut = (struct undoTileAll*)uptr->ptr;
			// Swap the contents of the tile data buffer with ut->ptr.
			uint32_t amtNew = ut->amt;
			uint32_t amtOld = currentProject->tileC->amt;
			uint8_t*oldDatBuf = (uint8_t*)malloc(getSzTile(ut->type) * amtOld); // Create a buffer large enough to hold the old data.
			cpyAllTiles(oldDatBuf, amtOld, ut->type);
			cpyAllTilesU((uint8_t*)ut->ptr, amtNew, ut->type);
			ut->amt = amtOld; // Backup the old amount.
			free(ut->ptr); // Free the old data.
			ut->ptr = (void*)oldDatBuf;
		}
		break;

		case uTileGroup:
		{
			struct undoTileGroup*ut = (struct undoTileGroup*)uptr->ptr;
			unsigned sz = getSzTile(ut->type);

			// First ensure that the correct number of tiles are avaliable to put the data in.
			if (ut->type & tTypeDeleteFlag) {
				std::vector<uint32_t> tmp = ut->lst;
				std::sort(tmp.begin(), tmp.end());

				if (redo) {
					for (int_fast32_t i = tmp.size(); i--;)
						currentProject->tileC->remove_tile_at(tmp[i]);
				} else {
					uint32_t fullSize = currentProject->tileC->amt + ut->lst.size();

					for (int_fast32_t i = 0; i < tmp.size(); ++i) {
						if (tmp[i] < currentProject->tileC->amt)
							currentProject->tileC->insertTile(tmp[i]);
					}

					currentProject->tileC->resizeAmt(fullSize);

					// Fill in the tiles with data.
					for (int_fast32_t i = ut->lst.size() - 1; i >= 0; --i)
						tilesToU(ut->data.data() + (i * sz), ut->lst[i], ut->type);

				}

				updateTileSelectAmt();
			} else {
				// Swap the old and new data.
				std::vector<uint8_t> tmpDat;
				tmpDat.resize(sz);

				for (int_fast32_t i = ut->lst.size() - 1; i >= 0; --i)
					tilesTo(tmpDat.data() + (i * sz), ut->lst[i], ut->type);

				// Fill the tiles with the old data.
				for (int_fast32_t i = ut->lst.size() - 1; i >= 0; --i)
					tilesToU(ut->data.data() + (i * sz), ut->lst[i], ut->type);

				ut->data = tmpDat;
			}
		}
		break;

		case uTileAppendgroupdat:
		{
			struct undoAppendgroupdat*ut = (struct undoAppendgroupdat*)uptr->ptr;

			if (redo) {
				unsigned amtold = currentProject->tileC->amt;
				currentProject->tileC->resizeAmt(amtold + ut->amt);
				memcpy(currentProject->tileC->tDat.data() + ((amtold * currentProject->tileC->tileSize)), ut->dat.data(), currentProject->tileC->tileSize * ut->amt);
				memcpy(currentProject->tileC->truetDat.data() + ((amtold * currentProject->tileC->tcSize)), ut->truedat.data(), currentProject->tileC->tcSize * ut->amt);
			} else
				currentProject->tileC->resizeAmt(currentProject->tileC->amt - ut->amt);

			updateTileSelectAmt();
		}
		break;

		case uTileAppend:
			if (redo)
				currentProject->tileC->appendTile();
			else
				currentProject->tileC->resizeAmt(currentProject->tileC->amt - 1);

			updateTileSelectAmt();
			break;

		case uTileNew:
			if (redo)
				currentProject->tileC->insertTile(uptr->vu);
			else
				currentProject->tileC->remove_tile_at(uptr->vu);

			updateTileSelectAmt();
			break;

		case uTilemapEdit:
		{
			struct undoTilemapEdit*um = (struct undoTilemapEdit*)uptr->ptr;
			isCorrectPlane(um->plane);

			uint32_t tmp = currentProject->tms->maps[um->plane].getRaw(um->x, um->y);
			currentProject->tms->maps[um->plane].setRaw(um->x, um->y, um->val);
			um->val = tmp;

			if (tileEditModePlace_G)
				window->updateTileMapGUI(um->x, um->y);
		}
		break;

		case uTilemap:
		case uTilemapattr:
		{
			struct undoTilemap*um = (struct undoTilemap*)uptr->ptr;
			isCorrectPlane(um->plane);
			unsigned wOld = currentProject->tms->maps[um->plane].mapSizeW;
			unsigned hOld = currentProject->tms->maps[um->plane].mapSizeHA;
			unsigned tmpBufSize = wOld * hOld;
			unsigned newBufSize = um->w * um->h;

			if (uptr->type == uTilemap) {
				tmpBufSize *= TileMapSizePerEntry;
				newBufSize *= TileMapSizePerEntry;
			}

			uint8_t*tmpBuf = (uint8_t*)malloc(tmpBufSize);

			if (uptr->type == uTilemapattr)
				attrCpy(tmpBuf, currentProject->tms->maps[um->plane].tileMapDat, tmpBufSize);
			else
				memcpy((void*)tmpBuf, currentProject->tms->maps[um->plane].tileMapDat, tmpBufSize);

			currentProject->tms->maps[um->plane].resize_tile_map(um->w, um->h);

			if (uptr->type == uTilemapattr)
				attrCpyU(currentProject->tms->maps[um->plane].tileMapDat, (uint8_t*)um->ptr, newBufSize);
			else
				memcpy(currentProject->tms->maps[um->plane].tileMapDat, um->ptr, newBufSize);

			um->w = wOld;
			um->h = hOld;

			free(um->ptr);
			um->ptr = tmpBuf;
		}
		break;

		case uTilemapResize:
		{
			struct undoResize*um = (struct undoResize*)uptr->ptr;
			isCorrectPlane(um->plane);

			if (redo)
				currentProject->tms->maps[um->plane].resize_tile_map(um->wnew, um->hnew);
			else {
				currentProject->tms->maps[um->plane].resize_tile_map(um->w, um->h);

				if (um->ptr)
					cpyResizeGeneric((uint8_t*)um->ptr, currentProject->tms->maps[um->plane].tileMapDat, um->w, um->h, um->wnew, um->hnew, 4, 1, true);
			}
		}
		break;

		case uTilemapBlocksAmt:
		{
			struct undoResize*um = (struct undoResize*)uptr->ptr;
			isCorrectPlane(um->plane);

			if (redo) {
				currentProject->tms->maps[um->plane].blockAmt(um->hnew / currentProject->tms->maps[um->plane].mapSizeH);
				char tmp[16];
				snprintf(tmp, 16, "%u", um->hnew / currentProject->tms->maps[um->plane].mapSizeH);
			} else {
				currentProject->tms->maps[um->plane].blockAmt(um->h / currentProject->tms->maps[um->plane].mapSizeH);

				if (um->ptr)
					cpyResizeGeneric((uint8_t*)um->ptr, currentProject->tms->maps[um->plane].tileMapDat, um->w, um->h, um->wnew, um->hnew, 4, 1, true);

				char tmp[16];
				snprintf(tmp, 16, "%u", um->h / currentProject->tms->maps[um->plane].mapSizeH);
			}
		}
		break;

		case uTilemapPlaneDelete:
		{
			struct undoTilemapPlane*um = (struct undoTilemapPlane*)uptr->ptr;

			if (redo)
				removePlane(um->plane);

			else {
				currentProject->tms->maps.insert(currentProject->tms->maps.begin() + um->plane, tileMap(*um->old));
				updatePlaneTilemapMenu();

				if (um->plane == currentProject->curPlane)
					setCurPlaneTilemaps(0, (void*)(uintptr_t)um->plane);
			}
		}
		break;

		case uTilemapPlaneAdd:
		{
			struct undoTilemapPlane*um = (struct undoTilemapPlane*)uptr->ptr;

			if (redo) {
				currentProject->tms->maps.insert(currentProject->tms->maps.begin() + um->plane, tileMap(currentProject));
				char tmp[16];
				snprintf(tmp, 16, "%u", um->plane);
				currentProject->tms->maps[um->plane].planeName.assign(tmp);
				updatePlaneTilemapMenu();

				if (um->plane == currentProject->curPlane)
					setCurPlaneTilemaps(0, (void*)(uintptr_t)um->plane);
			} else
				removePlane(um->plane);

		}
		break;

		case uPalette:
		{
			struct undoPalette*up = (struct undoPalette*)uptr->ptr;
			unsigned sz, el = currentProject->pal->totalColors();
			sz = el * currentProject->pal->esize;
			std::unique_ptr<uint8_t[]> tmpCopy(new uint8_t[sz]);

			memcpy((void*)&tmpCopy[0], currentProject->pal->palDat, sz); // Create a copy of the current palette.
			memcpy(currentProject->pal->palDat, up->ptr, sz); // Set the palette back to the old data.
			memcpy(up->ptr, (void*)&tmpCopy[0], sz); // up->ptr now contains what used to be in currentProject->pal->palDat.

			for (unsigned i = 0; i < el; ++i)
				currentProject->pal->updateRGBindex(i);
		}

		palBar.updateSliders();
		break;

		case uPaletteEntry:
		{
			struct undoPaletteEntry*up = (struct undoPaletteEntry*)uptr->ptr;

			switch (currentProject->pal->esize) {
				case 1:
				{
					uint8_t tmp = currentProject->pal->palDat[up->id];
					currentProject->pal->palDat[up->id] = up->val;
					up->val = tmp;
				}
				break;

				case 2:
				{
					uint16_t*ptr = (uint16_t*)currentProject->pal->palDat + up->id;
					std::swap(*ptr, up->val);
				}
				break;
			}

			currentProject->pal->updateRGBindex(up->id);

			switch (mode_editor) {
				case pal_edit:
					palBar.selBox[0] = up->id % currentProject->pal->perRow;
					palBar.changeRow(up->id / currentProject->pal->perRow, 0);
					break;

				case tile_edit:
					palBar.selBox[1] = up->id % currentProject->pal->perRow;
					palBar.changeRow(up->id / currentProject->pal->perRow, 1);
					{
						unsigned focus = 0;

						for (unsigned i = 0; i < currentProject->pal->rowCntPal; ++i)
							focus |= Fl::focus() == window->palRTE[i];

						for (unsigned i = 0; i < currentProject->pal->rowCntPal; ++i) {
							if (focus && (i == palBar.selRow[1]))
								Fl::focus(window->palRTE[i]);

							window->palRTE[i]->value(i == palBar.selRow[1]);
						}
					}
					break;

				case tile_place:
					palBar.selBox[2] = up->id % currentProject->pal->perRow;
					palBar.changeRow(up->id / currentProject->pal->perRow, 2);
					{	unsigned focus = 0;

						for (unsigned i = 0; i < currentProject->pal->rowCntPal; ++i)
							focus |= Fl::focus() == window->palRTE[i + 4];

						for (unsigned i = 0; i < currentProject->pal->rowCntPal; ++i) {
							if (focus && (i == palBar.selRow[2]))
								Fl::focus(window->palRTE[i + 4]);

							window->palRTE[i + 4]->value(i == palBar.selRow[2]);
						}
					}
					break;

				case spriteEditor:
					palBar.selBox[3] = up->id % currentProject->pal->perRow;
					palBar.changeRow(up->id / currentProject->pal->perRow, 3);
					window->spritepalrow->value(palBar.selRow[3]);
					break;
			}
		}
		break;

		case uChunkResize:
		{
			struct undoResize*um = (struct undoResize*)uptr->ptr;

			if (redo)
				currentProject->Chunk->resize(um->wnew, um->hnew);
			else {
				currentProject->Chunk->resize(um->w, um->h);

				if (um->ptr)
					cpyResizeGeneric((uint8_t*)um->ptr, (uint8_t*)currentProject->Chunk->chunks.data(), um->w, um->h, um->wnew, um->hnew, sizeof(struct ChunkAttrs), currentProject->Chunk->amt, true);
			}

			window->updateChunkSize();
		}
		break;

		case uChunkEdit:
		{
			struct undoChunkEdit*uc = (struct undoChunkEdit*)uptr->ptr;
			ChunkAttrs oldValue = currentProject->Chunk->getElm(uc->id, uc->x, uc->y);
			currentProject->Chunk->setElm(uc->id, uc->x, uc->y, uc->val);
			uc->val = oldValue;

			if (tileEditModeChunk_G)
				window->updateChunkGUI(uc->x, uc->y);
		}
		break;

		case uChunkAppend:
		{
			int add = redo ? 1 : -1;
			currentProject->Chunk->resizeAmt(currentProject->Chunk->amt + add);
			window->updateChunkSel();
		}
		break;

		case uChunkNew:
			if (redo)
				currentProject->Chunk->insert(uptr->vu);
			else
				currentProject->Chunk->removeAt(uptr->vu);

			window->updateChunkSel();
			break;

		case uChunkAll:
		{
			struct undoChunkAll*uc = (struct undoChunkAll*)uptr->ptr;
			unsigned wOld = currentProject->Chunk->wi;
			unsigned hOld = currentProject->Chunk->hi;
			unsigned oldAmt = currentProject->Chunk->amt;
			unsigned tmpBufSize = wOld * hOld * oldAmt * sizeof(struct ChunkAttrs);
			unsigned oldBufSize = uc->w * uc->h * uc->amt * sizeof(struct ChunkAttrs);
			struct ChunkAttrs* tmpBuf = (struct ChunkAttrs*)malloc(tmpBufSize);
			memcpy(tmpBuf, currentProject->Chunk->chunks.data(), tmpBufSize);

			currentProject->Chunk->resize(uc->w, uc->h);
			currentProject->Chunk->resizeAmt(uc->amt);

			memcpy(currentProject->Chunk->chunks.data(), uc->ptr, oldBufSize);

			uc->w = wOld;
			uc->h = hOld;
			uc->amt = oldAmt;
			free(uc->ptr);
			uc->ptr = tmpBuf;

			window->updateChunkSize();
		}
		break;

		case uChunk:
			fl_alert("TODO");
			break;

		case uChunkDelete:
		{	struct undoChunk*uc = (struct undoChunk*)uptr->ptr;

			if (redo)
				currentProject->Chunk->removeAt(uc->id);
			else {
				currentProject->Chunk->insert(uc->id);
				memcpy(currentProject->Chunk->chunks.data() + (currentProject->Chunk->wi * currentProject->Chunk->hi * uc->id), uc->ptr, currentProject->Chunk->wi * currentProject->Chunk->hi * sizeof(struct ChunkAttrs));
			}

			window->updateChunkSel();
		}
		break;

		case uSpriteAppend:
		{	struct undoSpriteAppend*us = (struct undoSpriteAppend*)uptr->ptr;

			if (redo)
				currentProject->ms->sps[us->id[0]].setAmtingroup(us->id[1], currentProject->ms->sps[us->id[0]].groups[us->id[1]].list.size() + 1);
			else
				currentProject->ms->sps[us->id[0]].delingroup(us->id[1], currentProject->ms->sps[us->id[0]].groups[us->id[1]].list.size() - 1);

			window->updateSpriteSliders();
		}
		break;

		case uSpriteAppendgroup:
			if (redo)
				currentProject->ms->sps[uptr->vu].setAmt(currentProject->ms->sps[uptr->vu].amt + 1);
			else
				currentProject->ms->sps[uptr->vu].del(currentProject->ms->sps[uptr->vu].amt - 1);

			window->updateSpriteSliders();
			break;

		case uSpriteAppendmeta:
			if (redo)
				currentProject->ms->sps.emplace_back(sprites(currentProject));
			else
				currentProject->ms->sps.pop_back();

			window->updateSpriteSliders();
			break;

		case uSpriteWidth:
			mkSpritePop(w)
			break;

		case uSpriteHeight:
			mkSpritePop(h)
			break;

		case uSpritePalrow:
			mkSpritePop(palrow)
			break;

		case uSpritestarttile:
			mkSpritePop(starttile)
			break;

		case uSpriteloadat:
			mkSpritePop(loadat)
			break;

		case uSpriteoffx:
			mkSpritePopInt32(offx)
			break;

		case uSpriteoffy:
			mkSpritePopInt32(offy)
			break;

		case uSpriteprio:
			mkSpritePopbool(prio)
			break;

		case uSpritehflip:
			mkSpritePopbool(hflip)
			break;

		case uSpritevflip:
			mkSpritePopbool(vflip)
			break;

		case uCurProject:
		{	struct undoProject*up = (struct undoProject*)uptr->ptr;
			Project*tmp = new Project(*currentProject);
			tmp->copyClasses(*currentProject);

			projects[up->id] = *up->ptr;
			projects[up->id].copyClasses(*up->ptr);
			delete up->ptr;
			up->ptr = tmp;

			prjChangePtr(up->id);
			switchProjectSlider(up->id);
		}
		break;

		case uProjectAll:
		{
			struct undoProjectAll*up = (struct undoProjectAll*)uptr->ptr;
			std::vector<struct Project> oldBackup = projects;

			for (size_t i = 0; i < oldBackup.size(); ++i)
				oldBackup[i].copyClasses(projects[i]);

			projects = up->old; // up->old already contains a copy of the classes.
			up->old = oldBackup;

			for (size_t i = 0; i < projects.size(); ++i)
				prjChangePtr(i);

			changeProjectAmt();
			switchProjectSlider(curProjectID);
		}
		break;

		case uProjectAppend:
			if (redo)
				appendProject();
			else
				removeProject(projects.size() - 1);

			break;
	}

	if (!redo)
		--pos;

	window->redraw();
}
void undoCB(Fl_Widget*, void*) {
	UndoRedo(false);
}
void redoCB(Fl_Widget*, void*) {
	UndoRedo(true);
}
void pushTile(uint32_t id, tileTypeMask_t type) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTile;
	uptr->ptr = malloc(sizeof(struct undoTile));
	memUsed += sizeof(struct undoTile);
	unsigned sz = getSzTile(type);
	struct undoTile*ut = (struct undoTile*)uptr->ptr;
	ut->ptr = malloc(sz);
	ut->id = id;
	ut->type = type;
	memUsed += sz;
	tilesTo((uint8_t*)ut->ptr, id, type);
}
void pushTilenew(uint32_t id) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTileNew;
	uptr->vu = id;
}
void pushTilePixel(uint32_t id, uint32_t x, uint32_t y, tileTypeMask_t type) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTilePixel;
	uptr->ptr = malloc(sizeof(struct undoTilePixel));
	memUsed += sizeof(struct undoTilePixel);
	struct undoTilePixel*ut = (struct undoTilePixel*)uptr->ptr;
	ut->id = id;
	ut->x = x;
	ut->y = y;
	ut->type = type;

	if (type == tTypeTruecolor)
		ut->val = currentProject->tileC->getPixelTc(id, x, y);
	else
		ut->val = currentProject->tileC->getPixel(id, x, y);
}
void pushTilesAll(tileTypeMask_t type) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTileAll;
	uptr->ptr = malloc(sizeof(struct undoTileAll));
	memUsed += sizeof(struct undoTileAll);
	struct undoTileAll*ut = (struct undoTileAll*)uptr->ptr;
	ut->amt = currentProject->tileC->amt;
	unsigned sz = getSzTile(type) * ut->amt;
	ut->ptr = malloc(sz);
	memUsed += sz;
	ut->type = type;
	cpyAllTiles((uint8_t*)ut->ptr, ut->amt, type);
}
void pushTileAppend(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTileAppend;
}
void pushTileGroupPrepare(tileTypeMask_t type) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTileGroup;
	uptr->ptr = new struct undoTileGroup;
	memUsed += sizeof(struct undoTileGroup);
	struct undoTileGroup*ut = (struct undoTileGroup*)uptr->ptr;
	ut->type = type;
}
void addTileGroup(uint32_t tile, int32_t forceid) {
	struct undoEvent*uptr = undoBuf + pos;
	struct undoTileGroup*ut = (struct undoTileGroup*)uptr->ptr;

	if (forceid > 0)
		ut->lst.push_back(forceid);
	else
		ut->lst.push_back(tile);

	unsigned sz = getSzTile(ut->type);
	ut->data.resize(sz * ut->lst.size());
	tilesTo(ut->data.data() + (sz * (ut->lst.size() - 1)), tile, ut->type);
}
void pushTileappendGroupPrepare(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTileAppendgroupdat;
	uptr->ptr = new struct undoAppendgroupdat;
	memUsed += sizeof(struct undoAppendgroupdat);
	struct undoAppendgroupdat*ut = (struct undoAppendgroupdat*)uptr->ptr;
	ut->amt = 0;
}
void addTileappendGroup(uint8_t*tdat, uint8_t*truetdat) {
	struct undoEvent*uptr = undoBuf + pos;
	struct undoAppendgroupdat*ut = (struct undoAppendgroupdat*)uptr->ptr;
	++ut->amt;
	ut->dat.resize(ut->amt * currentProject->tileC->tileSize);
	ut->truedat.resize(ut->amt * currentProject->tileC->tcSize);
	memcpy(ut->dat.data() + ((ut->amt - 1)*currentProject->tileC->tileSize), tdat, currentProject->tileC->tileSize);
	memcpy(ut->truedat.data() + ((ut->amt - 1)*currentProject->tileC->tcSize), truetdat, currentProject->tileC->tcSize);
}
void pushTilemapAll(bool attrOnly) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;

	if (attrOnly)
		uptr->type = uTilemapattr;
	else
		uptr->type = uTilemap;

	uptr->ptr = malloc(sizeof(struct undoTilemap));
	memUsed += sizeof(struct undoTilemap);
	struct undoTilemap*um = (struct undoTilemap*)uptr->ptr;
	um->plane = currentProject->curPlane;
	um->w = currentProject->tms->maps[currentProject->curPlane].mapSizeW;
	um->h = currentProject->tms->maps[currentProject->curPlane].mapSizeHA;

	if (attrOnly) {
		um->ptr = malloc(um->w * um->h);
		attrCpy((uint8_t*)um->ptr, currentProject->tms->maps[currentProject->curPlane].tileMapDat, um->w * um->h);
	} else {
		um->ptr = malloc(um->w * um->h * 4);
		memcpy(um->ptr, currentProject->tms->maps[currentProject->curPlane].tileMapDat, um->w * um->h * 4);
	}
}
struct undoTilemapPlane*pushTilemapPlaneComm(uint32_t plane) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->ptr = malloc(sizeof(struct undoTilemapPlane));
	memUsed += sizeof(struct undoTilemapPlane);
	struct undoTilemapPlane*um = (struct undoTilemapPlane*)uptr->ptr;
	um->plane = plane;
	um->old = 0;
	return um;
}
void pushTilemapPlaneDelete(uint32_t plane) {
	struct undoTilemapPlane*um = pushTilemapPlaneComm(plane);
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTilemapPlaneDelete;
	um->old = new tileMap(currentProject->tms->maps[plane], currentProject);
	memUsed += um->old->mapSizeW * um->old->mapSizeHA * TileMapSizePerEntry;
}
void pushTilemapPlaneAdd(uint32_t plane) {
	pushTilemapPlaneComm(plane);
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTilemapPlaneAdd;
}
void pushTilemapEdit(uint32_t x, uint32_t y) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uTilemapEdit;
	uptr->ptr = malloc(sizeof(struct undoTilemapEdit));
	memUsed += sizeof(struct undoTilemapEdit);
	struct undoTilemapEdit*um = (struct undoTilemapEdit*)uptr->ptr;
	um->x = x;
	um->y = y;
	um->plane = currentProject->curPlane;
	um->val = currentProject->tms->maps[currentProject->curPlane].getRaw(x, y);
}
static void pushResize(uint32_t wnew, uint32_t hnew, uint32_t w, uint32_t h, uint8_t*ptr, undoTypes_t type, uint32_t szelm, uint32_t n) {
	if ((wnew == w) && (hnew == h))
		return;

	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = type;
	uptr->ptr = malloc(sizeof(struct undoResize));
	memUsed += sizeof(struct undoResize);
	struct undoResize*um = (struct undoResize*)uptr->ptr;
	um->plane = currentProject->curPlane;
	um->w = w;
	um->h = h;
	um->wnew = wnew;
	um->hnew = hnew;
	uint32_t sz = getSzResizeGeneric(um->w, um->h, wnew, hnew, szelm, n);

	if (sz) {
		um->ptr = malloc(sz);
		memUsed += sz;
		cpyResizeGeneric((uint8_t*)um->ptr, ptr, um->w, um->h, wnew, hnew, szelm, n, false);
	} else
		um->ptr = 0;
}
void pushTilemapResize(uint32_t wnew, uint32_t hnew) {
	pushResize(wnew, hnew, currentProject->tms->maps[currentProject->curPlane].mapSizeW, currentProject->tms->maps[currentProject->curPlane].mapSizeHA, currentProject->tms->maps[currentProject->curPlane].tileMapDat, uTilemapResize, TileMapSizePerEntry, 1);
}
void pushTilemapBlocksAmt(uint32_t amtnew) {
	pushResize(currentProject->tms->maps[currentProject->curPlane].mapSizeW, currentProject->tms->maps[currentProject->curPlane].mapSizeH * amtnew, currentProject->tms->maps[currentProject->curPlane].mapSizeW, currentProject->tms->maps[currentProject->curPlane].mapSizeHA, currentProject->tms->maps[currentProject->curPlane].tileMapDat, uTilemapBlocksAmt, TileMapSizePerEntry, 1);
}
void pushPaletteAll(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uPalette;
	uptr->ptr = malloc(sizeof(struct undoPalette));
	memUsed += sizeof(struct undoPalette);
	struct undoPalette*up = (struct undoPalette*)uptr->ptr;
	unsigned sz = currentProject->pal->colorCnt + currentProject->pal->colorCntalt;
	sz *= currentProject->pal->esize;
	up->ptr = malloc(sz);
	memcpy(up->ptr, currentProject->pal->palDat, sz);
	memUsed += sz;
}
void pushPaletteEntry(uint32_t id) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uPaletteEntry;
	uptr->ptr = malloc(sizeof(struct undoPaletteEntry));
	memUsed += sizeof(struct undoPaletteEntry);
	struct undoPaletteEntry*up = (struct undoPaletteEntry*)uptr->ptr;
	up->id = id;

	switch (currentProject->pal->esize) {
		case 1:
			up->val = (int32_t)currentProject->pal->palDat[id];
			break;

		case 2:
		{	uint16_t*ptr = (uint16_t*)currentProject->pal->palDat + id;
			up->val = *ptr;
		}
		break;

		default:
			show_default_error
	}
}
void pushChunkResize(uint32_t wnew, uint32_t hnew) {
	pushResize(wnew, hnew, currentProject->Chunk->wi, currentProject->Chunk->hi, (uint8_t*)currentProject->Chunk->chunks.data(), uChunkResize, sizeof(struct ChunkAttrs), currentProject->Chunk->amt);
}
void pushChunkEdit(uint32_t id, uint32_t x, uint32_t y) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uChunkEdit;
	uptr->ptr = malloc(sizeof(struct undoChunkEdit));
	memUsed += sizeof(struct undoChunkEdit);
	struct undoChunkEdit*uc = (struct undoChunkEdit*)uptr->ptr;
	uc->x = x;
	uc->y = y;
	uc->id = id;
	uc->val = currentProject->Chunk->getElm(id, x, y);
}
void pushChunkNew(uint32_t id) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uChunkNew;
	uptr->vu = id;
}
void pushChunkAppend(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uChunkAppend;
}
void pushChunk(uint32_t id, bool rm) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;

	if (rm)
		uptr->type = uChunkDelete;
	else
		uptr->type = uChunk;

	uptr->ptr = malloc(sizeof(struct undoChunk));
	memUsed += sizeof(struct undoChunk);
	struct undoChunk*uc = (struct undoChunk*)uptr->ptr;
	uc->id = id;
	uc->ptr = (struct ChunkAttrs*)malloc(currentProject->Chunk->wi * currentProject->Chunk->hi * sizeof(struct ChunkAttrs));
	memcpy(uc->ptr, currentProject->Chunk->chunks.data() + (currentProject->Chunk->wi * currentProject->Chunk->hi * id), currentProject->Chunk->wi * currentProject->Chunk->hi * sizeof(struct ChunkAttrs));
}
void pushChunksAll(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uChunkAll;
	uptr->ptr = malloc(sizeof(struct undoChunkAll));
	memUsed += sizeof(struct undoChunkAll);
	struct undoChunkAll*uc = (struct undoChunkAll*)uptr->ptr;
	uc->w = currentProject->Chunk->wi;
	uc->h = currentProject->Chunk->hi;
	uc->amt = currentProject->Chunk->amt;
	uc->ptr = (struct ChunkAttrs*)malloc(uc->w * uc->h * uc->amt * sizeof(struct ChunkAttrs));
	memcpy(uc->ptr, currentProject->Chunk->chunks.data(), uc->w * uc->h * uc->amt * sizeof(struct ChunkAttrs));
}
void pushSpriteAppend(uint32_t id) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uSpriteAppend;
	uptr->ptr = malloc(sizeof(struct undoSpriteAppend));
	struct undoSpriteAppend*us = (struct undoSpriteAppend*)uptr->ptr;
	us->id[0] = window->metaspritesel->value();
	us->id[1] = id;
	memUsed += sizeof(struct undoSpriteAppend);
}
void pushSpriteAppendgroup(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uSpriteAppendgroup;
	uptr->vu = window->metaspritesel->value();
}
void pushSpriteAppendmeta(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uSpriteAppendmeta;
}

#define mkSpritePush_(strt, thetype, which) pushEventPrepare(); \
	struct undoEvent*uptr=undoBuf+pos; \
	uptr->type=thetype; \
	uptr->ptr=malloc(sizeof(struct strt)); \
	memUsed+=sizeof(struct strt); \
	struct strt*us=(struct strt*)uptr->ptr; \
	us->id=curSpritegroup; \
	us->subid=curSprite; \
	us->metaid=window->metaspritesel->value(); \
	us->val=currentProject->ms->sps[us->metaid].groups[us->id].list[us->subid].which

#define mkSpritePush(thetype, which) mkSpritePush_(undoSpriteVal, thetype, which)

#define mkSpritePushInt32(thetype, which) mkSpritePush_(undoSpriteValInt32, thetype, which)
#define mkSpritePushbool(thetype, which) mkSpritePush_(undoSpriteValbool, thetype, which)

void pushSpriteWidth(void) {
	mkSpritePush(uSpriteWidth, w);
}
void pushSpriteHeight(void) {
	mkSpritePush(uSpriteHeight, h);
}
void pushSpritePalrow(void) {
	mkSpritePush(uSpritePalrow, palrow);
}
void pushSpriteStarttile(void) {
	mkSpritePush(uSpritestarttile, starttile);
}
void pushSpriteLoadat(void) {
	mkSpritePush(uSpriteloadat, loadat);
}
void pushSpriteOffx(void) {
	mkSpritePushInt32(uSpriteoffx, offx);
}
void pushSpriteOffy(void) {
	mkSpritePushInt32(uSpriteoffy, offy);
}
void pushSpriteHflip(void) {
	mkSpritePushbool(uSpritehflip, hflip);
}
void pushSpriteVflip(void) {
	mkSpritePushbool(uSpritevflip, vflip);
}
void pushSpritePrio(void) {
	mkSpritePushbool(uSpriteprio, prio);
}
void pushProject(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uCurProject;
	uptr->ptr = new struct undoProject;
	memUsed += sizeof(struct undoProject);
	struct undoProject*up = (struct undoProject*)uptr->ptr;
	up->ptr = new Project(*currentProject);
	up->ptr->copyClasses(*currentProject);
	up->id = curProjectID;
}
void pushProjectAppend(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uProjectAppend;
}
void pushProjectAll(void) {
	pushEventPrepare();
	struct undoEvent*uptr = undoBuf + pos;
	uptr->type = uProjectAll;
	uptr->ptr = new struct undoProjectAll;
	memUsed += sizeof(struct undoProjectAll);
	struct undoProjectAll*up = (struct undoProjectAll*)uptr->ptr;
	up->old = projects;

	for (size_t i = 0; i < projects.size(); ++i)
		up->old[i].copyClasses(projects[i]);
}
static Fl_Window*win;
static void closeHistory(Fl_Widget*, void*) {
	win->hide();
}
void historyWindow(Fl_Widget*, void*) {
	win = new Fl_Window(350, 450, "History");
	win->begin();
	Fl_Button * Close = new Fl_Button(143, 418, 64, 24, "Close");
	Close->callback(closeHistory);
	Fl_Browser*hist = new Fl_Browser(8, 32, 336, 386);
	char tmp[2048];
	snprintf(tmp, 2048, "%u items sorted from oldest to newest\nPosition selected: %d (can be -1)", (unsigned)amount, (int)pos);
	hist->copy_label(tmp);
	hist->align(FL_ALIGN_TOP);

	for (unsigned n = 0; n < amount; ++n) {
		struct undoEvent*uptr = undoBuf + n;

		switch (uptr->type) {
			case uTile:
			{	struct undoTile*ut = (struct undoTile*)uptr->ptr;

				if (ut->type & tTypeDeleteFlag)
					snprintf(tmp, 2048, "Delete tile %d", ut->id);
				else
					snprintf(tmp, 2048, "Change tile %d", ut->id);
			}
			break;

			case uTilePixel:
			{	struct undoTilePixel*ut = (struct undoTilePixel*)uptr->ptr;

				if (ut->type & tTypeTruecolor)
					snprintf(tmp, 2048, "Edit truecolor tile pixel X: %d Y: %d", ut->x, ut->y);
				else
					snprintf(tmp, 2048, "Edit tile pixel X: %d Y: %d", ut->x, ut->y);
			}
			break;

			case uTileAll:
			{	struct undoTileAll*ut = (struct undoTileAll*)uptr->ptr;
				snprintf(tmp, 2048, "Change all tiles amount: %u", ut->amt);
			}
			break;

			case uTileAppend:
				strcpy(tmp, "Append tile");
				break;

			case uTileNew:
				snprintf(tmp, 2048, "Insert tile at %u", uptr->vu);

			case uTileGroup:
			{	struct undoTileGroup*ut = (struct undoTileGroup*)uptr->ptr;
				snprintf(tmp, 2048, "Tile group tiles affected: %u", (unsigned)ut->lst.size());
			}
			break;

			case uTileAppendgroupdat:
			{	struct undoAppendgroupdat*ut = (struct undoAppendgroupdat*)uptr->ptr;
				snprintf(tmp, 2048, "Append %u tiles with data", ut->amt);
			}
			break;

			case uTilemap:
				strcpy(tmp, "Change tilemap");
				break;

			case uTilemapattr:
				strcpy(tmp, "Change tilemap attributes");
				break;

			case uChunkResize:
			case uTilemapResize:
			{	struct undoResize*um = (struct undoResize*)uptr->ptr;
				snprintf(tmp, 2048, "Resize from w: %d h: %d to w: %d h: %d", um->w, um->h, um->wnew, um->hnew);
			}
			break;

			case uTilemapBlocksAmt:
			{	struct undoResize*um = (struct undoResize*)uptr->ptr;
				snprintf(tmp, 2048, "Change blocks amount from %u on plane %u", um->h / currentProject->tms->maps[um->plane].mapSizeH, um->plane);
			}
			break;

			case uTilemapEdit:
			{	struct undoTilemapEdit*um = (struct undoTilemapEdit*)uptr->ptr;
				snprintf(tmp, 2048, "Edit tilemap X: %d Y: %d on plane: %u", um->x, um->y, um->plane);
			}
			break;

			case uTilemapPlaneDelete:
			{	struct undoTilemapPlane*um = (struct undoTilemapPlane*)uptr->ptr;
				snprintf(tmp, 2048, "Delete plane %u", um->plane);
			}
			break;

			case uTilemapPlaneAdd:
			{	struct undoTilemapPlane*um = (struct undoTilemapPlane*)uptr->ptr;
				snprintf(tmp, 2048, "Add plane %u", um->plane);
			}
			break;

			case uPalette:
				strcpy(tmp, "Change entire palette");
				break;

			case uPaletteEntry:
			{	struct undoPaletteEntry*up = (struct undoPaletteEntry*)uptr->ptr;
				snprintf(tmp, 2048, "Change palette entry: %d", up->id);
			}
			break;

			case uChunkEdit:
			{	struct undoChunkEdit*uc = (struct undoChunkEdit*)uptr->ptr;
				snprintf(tmp, 2048, "Edit Chunk ID: %d X: %d Y: %d", uc->id, uc->x, uc->y);
			}
			break;

			case uChunk:
			{	struct undoChunk*uc = (struct undoChunk*)uptr->ptr;
				snprintf(tmp, 2048, "Change chunk: %d", uc->id);
			}
			break;

			case uChunkDelete:
			{	struct undoChunk*uc = (struct undoChunk*)uptr->ptr;
				snprintf(tmp, 2048, "Delete chunk: %d", uc->id);
			}
			break;

			case uChunkAppend:
				strcpy(tmp, "Append chunk");
				break;

			case uChunkNew:
				snprintf(tmp, 2048, "Insert chunk at %u", uptr->vu);
				break;

			case uChunkAll:
				strcpy(tmp, "Change all chunks");
				break;

			case uSpriteAppend:
			{	struct undoSpriteAppend*us = (struct undoSpriteAppend*)uptr->ptr;
				snprintf(tmp, 2048, "Append sprite to group: %u meta: %u", us->id[1], us->id[0]);
			}
			break;

			case uSpriteAppendgroup:
				snprintf(tmp, 2048, "Append sprite group meta: %u", uptr->vu);
				break;

			case uSpriteAppendmeta:
				strcpy(tmp, "Append sprite meta sprite");
				break;

			case uSpriteWidth:
				strcpy(tmp, "Change sprite width");
				break;

			case uSpriteHeight:
				strcpy(tmp, "Change sprite height");
				break;

			case uSpritePalrow:
				strcpy(tmp, "Change sprite palette row");
				break;

			case uSpritestarttile:
				strcpy(tmp, "Change sprite start tile");
				break;

			case uSpriteloadat:
				strcpy(tmp, "Change sprite load at");
				break;

			case uSpriteoffx:
				strcpy(tmp, "Change sprite offset x");
				break;

			case uSpriteoffy:
				strcpy(tmp, "Change sprite offset y");
				break;

			case uSpriteprio:
				strcpy(tmp, "Change sprite priority");
				break;

			case uSpritehflip:
				strcpy(tmp, "Change sprite hflip");
				break;

			case uSpritevflip:
				strcpy(tmp, "Change sprite vflip");
				break;

			case uCurProject:
				strcpy(tmp, "Change current project");
				break;

			case uProjectAll:
				strcpy(tmp, "Change all projects");
				break;

			case uProjectAppend:
				strcpy(tmp, "Append blank project");
				break;

			default:
				snprintf(tmp, 2048, "TODO unhandled %d", uptr->type);
		}

		hist->add(tmp);
	}

	hist->select(pos + 1);
	win->end();
	win->set_modal();
	win->show();

	while (win->shown())
		Fl::wait();

	delete win;
}
