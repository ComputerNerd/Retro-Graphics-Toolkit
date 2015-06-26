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
   Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#pragma once
struct palette{
	uint8_t*rgbPal;
	uint8_t*palDat;
	uint8_t*palType;/*!<Sets 3 different types for each palette entry free, locked and reserved*/
	unsigned colorCnt;//Total entries in palette for project
	unsigned colorCntalt;//Alternative palette color count
	unsigned rowCntPal;//Number of palette rows
	unsigned rowCntPalalt;
	unsigned perRow;//Colors per each palette row
	unsigned perRowalt;
	unsigned esize;//Bytes per palette entry
	bool haveAlt;//Does the current game system use an alternative sprite palette?
	palette(void);
	~palette(void);
	palette(const palette& other);
	void setVars(enum gameSystemEnum gameSystem);
	void read(FILE*fp,bool supportsAlt);
	void write(FILE*fp);
	void updateRGBindex(unsigned index);
	void clear(void);
	void rgbToEntry(unsigned r,unsigned g,unsigned b,unsigned ent);
	uint8_t to_nes_color_rgb(uint8_t red,uint8_t green,uint8_t blue);
	uint8_t to_nes_color(unsigned pal_index);
	uint8_t toNesChan(uint8_t ri,uint8_t gi,uint8_t bi,uint8_t chan);
	uint16_t to_sega_genesis_colorRGB(uint8_t r,uint8_t g,uint8_t b,uint16_t pal_index);
	uint16_t to_sega_genesis_color(uint16_t pal_index);
	unsigned calMaxPerRow(unsigned row);
	void swapEntry(unsigned one,unsigned two);
	unsigned getMaxRows(bool alt) const{
		return (alt&&haveAlt)?rowCntPalalt:rowCntPal;
	}
	void paletteToRgb(void){
		for(unsigned i=0;i<colorCnt+colorCntalt;++i){
			updateRGBindex(i);
	}
}
};
