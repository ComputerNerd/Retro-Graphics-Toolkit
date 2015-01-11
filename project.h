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
#define shareAmtPj 6
#ifndef _PROJECT_H
#define _PROJECT_H 1
#include "global.h"
#include "system.h"
#include "tilemap.h"
#include "class_tiles.h"
#include "classtilemap.h"
#include "classChunks.h"
#include "classSprites.h"
#include "color_compare.h"
#include "classpalette.h"
#include "classtilemaps.h"
#define currentProjectVersionNUM 8
extern uint32_t curProjectID;
struct luaDat{
	uint32_t len;
	void*dat;
};
enum luaControlEnum{PALETTE_C,TILEMAP_C,CHUNK_C,SPRITE_C,LEVEL_C};
struct luaControl{
	uint32_t len;
	enum luaControlEnum ctrlType;
	const char*name;
	void*dat;
};
struct Project{/*!<Holds all data needed for a project based system for example tile screen and level 1 are 2 separate projects*/
	Project();
	Project(const Project& other);
	~Project();
	bool isShared(uint32_t mask);
	bool isUniqueData(uint32_t mask);
	bool containsData(uint32_t mask);
	bool containsDataOR(uint32_t mask);
	std::string Name;
	gameSystemEnum gameSystem;
	uint32_t subSystem;
	uint32_t settings;
	uint32_t useMask;/*!<Sharing can be used regardless of use mask*/
	tilemaps*tms;
	tiles*tileC;
	ChunkClass*Chunk;
	palette*pal;
	sprites*spritesC;
	int32_t share[shareAmtPj];/*!<Negative if not sharing or project id (which is always positive) if sharing*/
	unsigned curPlane;
	uint32_t lauDatAmt;
	uint32_t luaControlAmt;
	struct luaDat*lDat;
	struct luaControl*lCtrl;
};
extern struct Project ** projects;
extern uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
extern struct Project * currentProject;
extern Fl_Slider* curPrj;
const char*maskToName(unsigned mask);
void compactPrjMem(void);
void initProject(void) __attribute__((constructor(101)));/*!< this needs to be ran before class constructors*/
void setHaveProject(uint32_t id,uint32_t mask,bool set);
void shareProject(uint32_t share,uint32_t with,uint32_t what,bool enable);
bool appendProject();
bool removeProject(uint32_t id);
void switchProject(uint32_t id);
bool loadProject(uint32_t id);
bool saveProject(uint32_t id);
bool saveAllProjects(void);
bool loadAllProjects(bool Old);
#define pjHavePal 1
#define pjHaveTiles 2
#define pjHaveMap 4
#define pjHaveChunks 8
#define pjHaveSprites 16
#define pjHaveLevel 32
#define pjNeedsTiles (pjHaveMap|pjHaveChunks|pjHaveSprites|pjHaveLevel) //Needs refers to what else needs it and does not include itself
#define pjNeedsPalette (pjNeedsTiles|pjHaveTiles)
#define pjMaxMaskBit 5
#define pjDefaultMask (pjHavePal|pjHaveTiles|pjHaveMap|pjHaveChunks|pjHaveSprites|pjHaveLevel)
#define pjAllMask pjDefaultMask

#define settingsDitherMask 255//Used for dither algorithm
#define subsettingsDitherMask 255
#define subsettingsDitherShift 8
#define nearestColorSettingsMask 255
#define nearestColorShift 16
#endif
