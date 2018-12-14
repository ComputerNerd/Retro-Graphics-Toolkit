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
   Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#pragma once
#include <cstdio>
#include <stdint.h>
#include <string>
#include <vector>
#include <FL/Fl_Shared_Image.H>
#include "classSprite.h"
#include "gamedef.h"
#include "project.h"
struct spriteGroup {
	std::string name;//Useful for nice formated output
	std::vector<struct sprite> list;
};
class sprites {
private:
	bool extraOptDPLC;
	void mappingItem(void*in, uint32_t id, gameType_t game);
	void DplcItem(void*in, uint32_t which, gameType_t game);
	bool alreadyLoaded(uint32_t id, uint32_t subid)const;
	bool checkDupmapping(uint32_t id, uint32_t&which)const;
	bool checkDupdplc(uint32_t id, uint32_t&which)const;
	bool recttoSprite(int x0, int x1, int y0, int y1, int where, Fl_Shared_Image*loaded_image, bool grayscale, unsigned*remap, uint8_t*palMap, uint8_t*mask, bool useMask, bool useAlpha);
	void handleDPLC(unsigned which, void*buf, unsigned n);
	std::vector<uint8_t> optDPLC(unsigned which, gameType_t game)const;
	uint32_t getTileOnSprite(unsigned x, unsigned y, unsigned which, unsigned i)const;
public:
	Project*prj;
	uint32_t amt;//The amount of sprites
	std::string name;
	std::vector<struct spriteGroup>groups;
	sprites(Project*prj);
	sprites(const sprites&other, Project*prj);
	~sprites();
	void fixDel(unsigned at, unsigned amt);
	void freeOptmizations(unsigned which);
	void importSpriteSheet(const char*fname = nullptr);
	void exportDPLC(gameType_t game)const;
	void importDPLC(gameType_t game);
	void exportMapping(gameType_t game)const;
	void importMapping(gameType_t game);
	void draw(uint32_t id, uint32_t x, uint32_t y, int32_t zoom, bool mode, int32_t*outx = 0, int32_t*outy = 0);
	void minmaxoffx(uint32_t id, int32_t&minx, int32_t&maxx)const;
	void minmaxoffy(uint32_t id, int32_t&miny, int32_t&maxy)const;
	void spriteGroupToImage(uint8_t*img, uint32_t id, int row = -1, bool alpha = true);
	void spriteImageToTiles(uint8_t*img, uint32_t id, int rowUsage, bool alpha = true, bool isIndexArray = false);
	uint32_t width(uint32_t id)const;
	uint32_t height(uint32_t id)const;
	void importImg(uint32_t to);//the parameter "to" counts from 0
	bool load(FILE*fp, uint32_t version);
	bool save(FILE*fp)const;
	void setAmt(uint32_t amtnew);
	void setAmtingroup(uint32_t id, uint32_t amtnew);
	void del(uint32_t id);
	void delingroup(uint32_t id, uint32_t subid);
	void enforceMax(unsigned wmax, unsigned hmax);
	void allToPalRow(unsigned palRow);
};
