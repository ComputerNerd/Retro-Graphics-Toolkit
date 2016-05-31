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
	Copyright Sega16 (or whatever you wish to call me) (2012-2016)
*/
#pragma once
#define TileMapSizePerEntry 4
#include "project.h"
#include "filemisc.h"
class tileMap{
private:
public:
	tileMap(Project*prj)noexcept;
	tileMap(uint32_t w,uint32_t h,Project*prj)noexcept;
	tileMap(tileMap&& other)noexcept;
	tileMap& operator=(tileMap&& other)noexcept;
	tileMap& operator=(const tileMap&other)noexcept;
	tileMap(const tileMap&other,Project*prj)noexcept;
	tileMap(const tileMap&other)noexcept;
	~tileMap();
	struct Project*prj;
	uint8_t*tileMapDat;/*!< Holds tilemap data*/
	uint8_t*extPalRows;
	int32_t offset;
	uint32_t mapSizeW,mapSizeH,mapSizeHA;
	uint32_t getNumElms(void)const{
		return mapSizeW*mapSizeHA;
	}
	bool isBlock;
	uint32_t amt;
	void ditherAsImage(bool entire);
	bool pickTileRowQuantChoice(unsigned rows);
	bool inRange(uint32_t x,uint32_t y)const;
	void setRaw(uint32_t x,uint32_t y,uint32_t val);
	uint32_t getRaw(uint32_t x,uint32_t y)const;
	void blockAmt(uint32_t newAmt);
	void resizeBlocks(uint32_t wn,uint32_t hn);
	void toggleBlocks(bool set);
	bool get_hflip(uint32_t x,uint32_t y)const;
	bool get_vflip(uint32_t x,uint32_t y)const;
	bool get_prio(uint32_t x,uint32_t y)const;
	uint32_t get_tile(uint32_t x,uint32_t y)const;
	int32_t get_tileRow(uint32_t x,uint32_t y,unsigned useRow)const;
	unsigned getPalRow(uint32_t x,uint32_t y)const;
	static unsigned getPalRowExt(const uint8_t*ptr,uint32_t y,bool fg);
	unsigned getPalRowExt(uint32_t x,uint32_t y,bool fg)const;
	const uint8_t*getExtPtr(uint32_t x,uint32_t y)const;
	void set_hflip(uint32_t x,uint32_t y,bool hflip_set);
	void set_vflip(uint32_t x,uint32_t y,bool vflip_set);
	void set_pal_row(uint32_t x,uint32_t y,unsigned row);
	void setPalRowExt(uint32_t x,uint32_t y,unsigned row,bool fg);
	void set_tile_full(uint32_t x,uint32_t y,uint32_t tile,unsigned palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio);
	void set_tile(uint32_t x,uint32_t y,uint32_t tile);
	void set_prio(uint32_t x,uint32_t y,bool prio_set);
	void allRowSet(unsigned row);
	bool saveToFile(const char*fname,fileType_t type,int clipboard,int compression,const char*label="mapDat",const char*nesFname=nullptr,const char*labelNES="attrMapDat");
	bool saveToFile(void);
	bool loadFromFile();
	void sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip);
	void pickRow(unsigned amount,int type=-1,int method=-1);
	void pickRowDelta(bool showProgress=false,Fl_Progress *progress=nullptr,int alg=-1,int order=-1);
	void ScrollUpdate(void);
	void resize_tile_map(uint32_t new_x,uint32_t new_y);
	bool truecolor_to_image(uint8_t * the_image,int useRow=-1,bool useAlpha=true);
	void truecolorimageToTiles(uint8_t * image,int rowusage,bool useAlpha=true,bool copyToTruecolor=false,bool convert=true,bool isIndexArray=false);
	void drawBlock(unsigned block,unsigned xoo,unsigned yo,unsigned flags,unsigned zoom);
	void drawPart(unsigned offx,unsigned offy,unsigned x,unsigned y,unsigned w,unsigned h,int rowSolo,unsigned zoom,bool trueCol);
	void findFirst(int&x,int&y,unsigned tile)const;
	void pickExtAttrs(void);
	size_t getExtAttrsSize(void)const;
	void removeBlock(unsigned id);
};
