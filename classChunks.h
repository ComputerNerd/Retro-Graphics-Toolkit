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
#pragma once
#include <stdint.h>
#include <vector>
#include "project.h"
struct __attribute__ ((__packed__)) ChunkAttrs {
	uint32_t block;
	uint32_t flags;
};
class ChunkClass {
private:
	unsigned getOff(const uint32_t id, const uint32_t x, const uint32_t y)const;
	void getXYblock(unsigned id, unsigned x, unsigned y, unsigned& xo, unsigned& yo)const;
public:
	uint32_t amt;//Amount of chunks
	uint32_t wi, hi; //How many blocks/Tiles the chunk contains
	bool useBlocks;
	unsigned usePlane;
	std::vector<struct ChunkAttrs> chunks;
	Project*prj;
	ChunkClass(Project*prj);
	ChunkClass(const ChunkClass& other, Project*prj);
	~ChunkClass();
	void insert(uint32_t at);
	void append() {
		resizeAmt(amt + 1);
	};
	void setElm(uint32_t id, uint32_t x, uint32_t y, struct ChunkAttrs c);
	struct ChunkAttrs getElm(uint32_t id, uint32_t x, uint32_t y)const;
	void removeAt(uint32_t at);
	void resizeAmt(uint32_t amtnew);
	void resizeAmt(void);
	bool getPrio_t(uint32_t id, uint32_t x, uint32_t y)const;
	bool getPrio(uint32_t id, uint32_t x, uint32_t y)const;
	uint8_t getTileRow_t(uint32_t id, uint32_t x, uint32_t y)const; //x and y refer to tiles not blocks
	unsigned getTile_t(uint32_t id, uint32_t x, uint32_t y)const; //x and y refer to tiles not blocks
	void setBlock(uint32_t id, uint32_t x, uint32_t y, uint32_t block); //Which chunk,offset x,offset y (offsets relative to chunk)
	void setFlag(uint32_t id, uint32_t x, uint32_t y, uint32_t flag);
	uint32_t getFlag(uint32_t id, uint32_t x, uint32_t y)const;
	void setSolid(uint32_t id, uint32_t x, uint32_t y, unsigned solid);
	void setHflip(uint32_t id, uint32_t x, uint32_t y, bool hflip);
	void setVflip(uint32_t id, uint32_t x, uint32_t y, bool vflip);
	void setPrio(uint32_t id, uint32_t x, uint32_t y, bool prio);
	uint32_t getBlock(uint32_t id, uint32_t x, uint32_t y)const;
	bool getHflip(uint32_t id, uint32_t x, uint32_t y)const;
	bool getVflip(uint32_t id, uint32_t x, uint32_t y)const;
	unsigned getSolid(uint32_t id, uint32_t x, uint32_t y)const;
	void drawChunk(uint32_t id, int xo, int yo, int zoom, int scrollX = 0, int scrollY = 0);
	void scrollChunks(void);
	void importSonic1(bool append);
	void exportSonic1(void)const;
	void resize(uint32_t wnew, uint32_t hnew);
	void subBlock(unsigned oid, unsigned nid);
};
