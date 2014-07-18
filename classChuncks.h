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
#include <stdint.h>
#pragma once
struct __attribute__ ((__packed__)) ChunckAttrs{
	uint32_t block;
	uint32_t flags;
/*! A chuck usally contains a block id and flags which control what to do with the block*/
};
class ChunckClass{
public:
	uint32_t amt;//Amount of chuncks
	uint32_t wi,hi;//How many blocks/Tiles the chunck contains
	bool useBlocks;
	struct ChunckAttrs * chuncks;
	ChunckClass();
	ChunckClass(const ChunckClass& other);
	~ChunckClass();
	bool getPrio(uint32_t id,uint32_t x,uint32_t y);
	uint8_t getTileRow(uint32_t id,uint32_t x,uint32_t y);//x and y refer to tiles not blocks
	void setBlock(uint32_t id,uint32_t x,uint32_t y,uint32_t block);//Which chunck,offset x,offset y (offsets relative to chunck)
	void setFlag(uint32_t id,uint32_t x,uint32_t y,uint32_t flag);
	void drawChunck(uint32_t id,int xo,int yo,int zoom,int scrollX=0,int scrollY=0);
	void scrollChuncks(void);
	void importSonic1(const char * filename,bool append);
};
