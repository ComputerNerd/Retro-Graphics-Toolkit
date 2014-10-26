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
class tiles{
public:
	tiles();
	tiles(const tiles& other);
	~tiles();
	uint8_t tileSize;/*!< Sets the size on one tile in bytes 16 or 32 depends of if NES or genesis*/
	uint32_t amt;/*!< Amount of tiles*/
	uint32_t current_tile;/*!< The current tile that is being edited*/
	std::vector<uint8_t> tDat;/*!< Pointer to the tile data*/
	std::vector<uint8_t> truetDat;/*< Pointer to Truecolor tile data*/
	unsigned sizew,sizeh;
	unsigned tcSize;
	void insertTile(uint32_t at);
	void setPixel(uint32_t tile,uint32_t x,uint32_t y,uint32_t val);
	uint32_t getPixel(uint32_t tile,uint32_t x,uint32_t y);
	void setPixelTc(uint32_t tile,uint32_t x,uint32_t y,uint32_t val);
	uint32_t getPixelTc(uint32_t tile,uint32_t x,uint32_t y);
	void resizeAmt(uint32_t amtnew);//Resizes array to hold enough for set amount
	void resizeAmt(void);
	void appendTile(unsigned many=1);
	void remove_tile_at(uint32_t);
	void truecolor_to_tile_ptr(uint8_t palette_row,uint32_t cur_tile,uint8_t * tileinput,bool Usedither,bool isSprite);
	void truecolor_to_tile(uint8_t,uint32_t,bool isSprite);/*!< truecolor_to_tile will update/dither the selected tile*/
	void draw_truecolor(uint32_t,unsigned x,unsigned y,bool usehflip,bool usevflip,unsigned zoom);
	void draw_tile(int,int,uint32_t,int,uint8_t,bool,bool,bool isSprite=false);
	void hflip_truecolor(uint32_t,uint32_t *);
	void vflip_truecolor(uint32_t,uint8_t *);
	void vflip_truecolor_ptr(uint8_t *,uint8_t *);
	void hflip_tile(uint32_t,uint8_t *);
	void vflip_tile(uint32_t,uint8_t *);
	void vflip_tile_ptr(uint8_t *,uint8_t *);
	void blank_tile(uint32_t);/*!<This makes the tile use color 0*/ 
	void remove_duplicate_tiles(bool tColor);
#if __LP64__
	bool cmp_trueC(uint32_t one,uint64_t * two);/*!<Used to compare two truecolor tiles returns true if the same*/
#else
	bool cmp_trueC(uint32_t one,uint32_t * two);/*!<Used to compare two truecolor tiles returns true if the same*/
#endif
#if __LP64__
	bool cmp_tiles(uint32_t one,uint64_t * two);/*!<Used to compare two tiles returns true if the same*/
#else
	bool cmp_tiles(uint32_t one,uint32_t * two);/*!<Used to compare two tiles returns true if the same*/
#endif
};
