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
enum undoTypes_t{
	uTile=0,
	uTileGroup,
	uTilePixel,
	uTileAppend,
	uTilemap,
	uTilemapEdit,
	uTilemapResize,
	uPalette,
	uPaletteEntry
};
enum tileTypeMask_t{
	tTypeTile=1,
	tTypeTruecolor,
	tTypeBoth,
	tTypeDelete=7//This sets bit tTypeBoth|4
};
struct undoEvent{//This struct mearly holds which type of undo this is
	undoTypes_t type;
	void*ptr;//Can also be reused for information for example appendTile will store tile id if doing so limit yourself to 32bit values.
};
struct undoTile{//The purpose of this struct if to completly undo a tile
	tileTypeMask_t type;
	uint32_t id;
	void*ptr;//when type is both first the truecolor tile will be stored then the regular tile
};
struct undoTileGroup{//Easily make changes to countigous tiles
	tileTypeMask_t type;
	uint32_t start,finish;// [start,finish]
	void*ptr;
};
struct undoTilePixel{
	tileTypeMask_t type;
	uint32_t id,x,y,val;
};
struct undoTilemap{//For undoing the entire tilemap
	uint32_t w,h;//The width and height
	void*ptr;//Points to tilemap data that is w*h*4 bytes
};
struct undoTilemapEdit{
	uint32_t x,y,val;
};
struct undoTilemapResize{
	uint32_t w,h;//Old width and height
	void*ptr;//Contains a point ONLY TO LOST DATA if the tilemap was made bigger this will be NULL
};
//There is no struct for undo Palette the undoEvent struct will just point to the old data
struct undoPaletteEntry{
	uint32_t id,val;
};
void popUndo(void);
void historyWindow(void);//Controls settings and shows history
void pushTile(uint32_t id,tileTypeMask_t type);
void pushTilePixel(uint32_t id,uint32_t x,uint32_t y,tileTypeMask_t type);
void pushTilesAll(tileTypeMask_t type);
void pushTilemapEdit(uint32_t x,uint32_t y);
void pushTilemapResize(uint32_t wnew,uint32_t hnew);
void pushTilemapAll();
void pushPaletteEntry(uint32_t id);
void pushPaletteEntryall(void);
