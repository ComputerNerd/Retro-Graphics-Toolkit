/* This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#pragma once
#include "classChunks.h"
#include "classSprite.h"
#include "classSprites.h"
#include "classtilemap.h"
#include "project.h"
enum tileTypeMask_t{
	tTypeTile=1,
	tTypeTruecolor,
	tTypeBoth,
	tTypeDeleteFlag,//Used for checking if delete. Do not pass to any tile functions instead tTypeDelete should be used
	tTypeDelete=7//This sets bit tTypeBoth|tTypeDeleteFlag
};
void undoCB(Fl_Widget*,void*);
void redoCB(Fl_Widget*,void*);
void clearUndoCB(Fl_Widget*,void*);
void showMemUsageUndo(Fl_Widget*,void*);
void historyWindow(Fl_Widget*,void*);//Controls settings and shows history
void pushTile(uint32_t id,tileTypeMask_t type);
void pushTilenew(uint32_t id);
void pushTilePixel(uint32_t id,uint32_t x,uint32_t y,tileTypeMask_t type);
void pushTileGroupPrepare(tileTypeMask_t type);
void addTileGroup(uint32_t tile,int32_t forceid=-1);
void pushTileappendGroupPrepare(void);
void addTileappendGroup(uint8_t*tdat,uint8_t*truetdat);
void pushTilesAll(tileTypeMask_t type);
void pushTileAppend(void);
void pushTilemapEdit(uint32_t x,uint32_t y);
void pushTilemapResize(uint32_t wnew,uint32_t hnew);
void pushTilemapBlocksAmt(uint32_t amtnew);
void pushTilemapAll(bool attrOnly);
void pushTilemapPlaneDelete(uint32_t plane);
void pushTilemapPlaneAdd(uint32_t plane);
void pushExtAttrs(uint32_t plane);
void pushPaletteEntry(uint32_t id);
void pushPaletteAll(void);
void pushChunk(uint32_t id,bool rm);
void pushChunksAll(void);
void pushChunkResize(uint32_t wnew,uint32_t hnew);
void pushChunkEdit(uint32_t id,uint32_t x,uint32_t y);
void pushChunkAppend(void);
void pushChunkNew(uint32_t id);
void pushSpriteAppend(uint32_t id);
void pushSpriteAppendgroup(void);
void pushSpriteAppendmeta(void);
void pushSpriteWidth(void);
void pushSpriteHeight(void);
void pushSpritePalrow(void);
void pushSpriteStarttile(void);
void pushSpriteLoadat(void);
void pushSpriteOffx(void);
void pushSpriteOffy(void);
void pushSpriteHflip(void);
void pushSpriteVflip(void);
void pushSpritePrio(void);
void pushProject(void);
void pushProjectAppend(void);
void pushProjectAll(void);
