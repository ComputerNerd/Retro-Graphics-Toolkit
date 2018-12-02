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
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "project.h"
struct __attribute__ ((__packed__)) levDat {
	uint32_t id, dat, xormask;
	int64_t extra;
};
struct __attribute__ ((__packed__)) levobjDat { //For sprite layout
	uint32_t x, y, prjid, metaid, groupid, dat, xormask;
	int64_t extra;
};
enum source {TILES, BLOCKS, CHUNKS};
struct __attribute__ ((__packed__)) levelInfo {
	uint32_t w, h, nx, dx, ny, dy;
	int32_t repeatx, repeaty, src;
	int64_t extra;
};
struct level {
	Project*prj;
	uint32_t layeramt;
	std::vector<struct levelInfo>lvlI;//Allow different sized layers
	std::vector<std::vector<struct levDat>*>dat;
	std::vector<std::vector<struct levobjDat>*>odat;
	std::vector<std::string>layernames;
	level(Project*prj);
	level(const level&o, Project*prj);
	void addLayer(unsigned at, bool after);
	void removeLayer(unsigned which);
	bool inRangeLayer(unsigned tst);
	struct levelInfo*getInfo(unsigned layer);
	void setInfo(unsigned layer, struct levelInfo i);
	struct levDat*getlevDat(unsigned layer, unsigned x, unsigned y);
	void setlevDat(unsigned layer, unsigned x, unsigned y, struct levDat d);
	struct levobjDat*getObjDat(unsigned layer, unsigned idx);
	void setlevObjDat(unsigned layer, unsigned idx, struct levobjDat d);
	void setlayeramt(unsigned amt, bool lastLayerDim);
	void resizeLayer(unsigned idx, unsigned nw, unsigned nh);
	void save(FILE*fp);
	void load(FILE*fp, uint32_t version);
	void subType(unsigned oid, unsigned nid, enum source s, int plane);
};
