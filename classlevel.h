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
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include "project.h"
struct __attribute__ ((__packed__)) levDat{
	uint32_t id,dat;
};
struct __attribute__ ((__packed__)) levobjDat{//For sprite layout
	bool priority;//xor with sprites priority
	uint32_t x,y,id[3]/*project id,meta id,group id*/,dat;
};
enum source{TILES,BLOCKS,CHUNKS};
struct __attribute__ ((__packed__)) levelInfo{
	uint32_t w,h,n,d;
	int32_t repeatx,repeaty,src;
};
struct level{
	Project*prj;
	uint32_t layeramt;
	std::vector<struct levelInfo>lvlI;//Allow different sized layers
	std::vector<std::vector<struct levDat>*>dat;
	std::vector<std::vector<struct levobjDat>*>odat;
	level(Project*prj);
	level(const level&o,Project*prj);
	void addLayer(unsigned at,bool after);
	void removeLayer(unsigned which);
	void setId(unsigned x,unsigned y,unsigned layer,unsigned val);
	void setDat(unsigned x,unsigned y,unsigned layer,unsigned val);
	uint32_t getId(unsigned x,unsigned y,unsigned layer)const;
	uint32_t getDat(unsigned x,unsigned y,unsigned layer)const;
	void setlayeramt(unsigned amt,bool lastLayerDim);
	void draw(unsigned x,unsigned y,unsigned zoom,int solo,bool showSprites)const;
	void save(FILE*fp);
	void load(FILE*fp,uint32_t version);
};
