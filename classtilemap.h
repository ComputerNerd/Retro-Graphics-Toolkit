/*
   This file is part of Retro Graphics Toolkit

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
#define TileMapSizePerEntry 4
class tileMap{
private:
public:
	tileMap();
	tileMap(uint32_t w,uint32_t h);
	tileMap(const tileMap& other);
	~tileMap();
	uint8_t * tileMapDat;/*!< Holds tilemap data*/
	int32_t offset;
	uint32_t mapSizeW,mapSizeH,mapSizeHA;
	bool isBlock;
	uint32_t amt;
	void ditherAsImage(bool entire);
	bool pickTileRowQuantChoice(unsigned rows);
	bool inRange(uint32_t x,uint32_t y);
	void setRaw(uint32_t x,uint32_t y,uint32_t val);
	uint32_t getRaw(uint32_t x,uint32_t y);
	void blockAmt(uint32_t newAmt);
	void resizeBlocks(uint32_t wn,uint32_t hn);
	void toggleBlocks(bool set);
	bool get_hflip(uint32_t x,uint32_t y);
	bool get_vflip(uint32_t x,uint32_t y);
	bool get_prio(uint32_t x,uint32_t y);
	uint32_t get_tile(uint32_t x,uint32_t y);
	int32_t get_tileRow(uint32_t x,uint32_t y,uint8_t useRow);
	uint8_t get_palette_map(uint32_t x,uint32_t y);
	void set_vflip(uint32_t x,uint32_t y,bool vflip_set);
	void set_pal_row(uint32_t x,uint32_t y,uint8_t row);
	bool saveToFile();
	bool loadFromFile();
	void sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip);
	void pickRow(unsigned amount);
	void pickRowDelta(bool showProgress=false,Fl_Progress *progress=0);
	void allRowSet(unsigned row);
	void set_tile_full(uint32_t tile,uint32_t x,uint32_t y,uint8_t palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio);
	void set_tile(uint32_t x,uint32_t y,uint32_t tile);
	void set_prio(uint32_t x,uint32_t y,bool prio_set);
	void set_hflip(uint32_t x,uint32_t y,bool hflip_set);
	void ScrollUpdate(void);
	void resize_tile_map(uint32_t new_x,uint32_t new_y);
	bool truecolor_to_image(uint8_t * the_image,int8_t useRow=-1,bool useAlpha=true);
	void truecolorimageToTiles(uint8_t * image,int rowusage,bool useAlpha=true);
};
