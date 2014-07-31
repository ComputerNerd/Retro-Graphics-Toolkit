/* This file is part of Retro Graphics Toolkit

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
#include "classChunks.h"
enum undoTypes_t{
	uTile=0,
	uTileAll,
	uTileGroup,
	uTilePixel,
	uTileAppend,//No struct
	uTileNew,//No struct reuses ptr for insert tile after
	uTilemap,
	uTilemapattr,
	uTilemapEdit,
	uTilemapResize,
	uPalette,
	uPaletteEntry,
	uChunk,
	uChunkDelete,
	uChunkAll,
	uChunkEdit,
	uChunkResize,
	uChunkAppend,
	uChunkNew,//No struct reuses ptr
	uSwitchSys,
	uSwitchPrj,//No struct reuses ptr
	uLoadPrj,
	ULoadPrjGroup,
};
enum tileTypeMask_t{
	tTypeTile=1,
	tTypeTruecolor,
	tTypeBoth,
	tTypeDeleteFlag,//Used for checking if delete do not pass to any tile functions
	tTypeDelete=7//This sets bit tTypeBoth|tTypeDeleteFlag
};
struct undoEvent{//This struct mearly holds which type of undo this is
	undoTypes_t type;
	void*ptr;//Can also be reused for information for example appendTile will store tile id if doing so limit yourself to 32bit values. Even if void* is 64bit on your system also can point to pointer created either by malloc or new
};
struct undoTile{//The purpose of this struct if to completly undo a tile
	tileTypeMask_t type;
	uint32_t id;
	void*ptrnew;
	void*ptr;//when type is both first the truecolor tile will be stored then the regular tile
};
struct undoTileAll{
	tileTypeMask_t type;
	uint32_t amt,amtnew;
	void*ptr;
	void*ptrnew;
};
struct undoTileGroup{
	tileTypeMask_t type;
	std::vector<uint32_t> lst;//Contains group of affect tiles
	std::vector<uint8_t> data;//Similar situation to other tile structs as in what this contains and what order
	std::vector<uint8_t> datanew;
};
struct undoTilePixel{
	tileTypeMask_t type;
	uint32_t id,x,y,val,valnew;
};
struct undoTilemap{//For undoing the entire tilemap
	uint32_t w,h,wnew,hnew;//The width and height
	void*ptr;//Points to tilemap data that is w*h*4 bytes or attributes if so size is w*h
	void*ptrnew;
};
struct undoTilemapEdit{
	uint32_t x,y,val,valnew;
};
struct undoResize{
	uint32_t w,h,wnew,hnew;//Old width and height
	void*ptr;//Contains a pointer ONLY TO LOST DATA
};
struct undoPalette{
	void*ptr;
	void*ptrnew;
};
struct undoPaletteEntry{
	uint32_t id,val,valnew;
};
struct undoChunkEdit{
	uint32_t id,x,y;
	struct ChunkAttrs valnew,val;
};
struct undoChunk{
	uint32_t id;
	struct ChunkAttrs*valnew,*val;
};
struct undoChunkAll{
	uint32_t w,h,wnew,hnew;//The width and height
	uint32_t amt,amtnew;
	struct ChunkAttrs*valnew,*val;
};
void showMemUsageUndo(Fl_Widget*,void*);
void UndoRedo(bool redo);
void historyWindow(Fl_Widget*,void*);//Controls settings and shows history
void pushTile(uint32_t id,tileTypeMask_t type);
void pushTilenew(uint32_t id);
void pushTilePixel(uint32_t id,uint32_t x,uint32_t y,tileTypeMask_t type);
void pushTileGroupPrepare(tileTypeMask_t type);
void addTileGroup(uint32_t tile,int32_t forceid=-1);
void pushTilesAll(tileTypeMask_t type);
void pushTileAppend(void);
void pushTilemapEdit(uint32_t x,uint32_t y);
void pushTilemapResize(uint32_t wnew,uint32_t hnew);
void pushTilemapAll(bool attrOnly);
void pushPaletteEntry(uint32_t id);
void pushPaletteAll(void);
void pushChunk(uint32_t id,bool rm);
void pushChunksAll(void);
void pushChunkResize(uint32_t wnew,uint32_t hnew);
void pushChunkEdit(uint32_t id,uint32_t x,uint32_t y);
void pushChunkAppend(void);
void pushChunkNew(uint32_t id);
void pushSwitchSys(void);
void pushSwitchPrj(void);
