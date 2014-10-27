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
/*!
This is where all "project" releated stuff goes
For example the save project file function goes here
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
#define currentProjectVersionNUM 8
extern uint32_t curProjectID;
struct Project{/*!<Holds all data needed for a project based system for examaple tile screen and level 1 are 2 seperate projects*/
	std::string Name;
	uint32_t gameSystem;
	uint32_t subSystem;
	uint32_t settings;//Stores dither algorithm and engine settings such as sonic 1 and such.
	uint32_t useMask;/*!<Sharing can be used regardless of use mask*/
	tileMap* tileMapC;
	tiles* tileC;
	ChunkClass*Chunk;
	sprites*spritesC;
	uint8_t* rgbPal;
	uint8_t* palDat;
	uint8_t* palType;/*!<Sets 3 different types for each palette entry free locked and reserved*/
	int32_t share[shareAmtPj];/*!<Negative if not sharing or project id (which is always positive) if sharing*/
	unsigned colorCnt;//Total entries in palette for project
	unsigned colorCntalt;//Alternative palette color count
	unsigned rowCntPal;
	unsigned rowCntPalalt;
	bool haveAltspritePal;
};
extern struct Project ** projects;
extern uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
extern struct Project * currentProject;
extern Fl_Slider* curPrj;
const char*maskToName(unsigned mask);
bool containsDataProj(uint32_t prj,uint32_t mask);
bool containsDataProjOR(uint32_t prj,uint32_t mask);
bool containsDataCurProj(uint32_t mask);
bool containsDataCurProjOR(uint32_t mask);
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

#define settingsDitherMask 255
#define subsettingsDitherMask 255
#define subsettingsDitherShift 8

#endif
