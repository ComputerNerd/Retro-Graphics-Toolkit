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
#define shareAmtPj 6
#ifndef _PROJECT_H
#define _PROJECT_H 1
#include "includes.h"
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
enum luaLocEnum{PALETTE_C,TILEMAP_C,CHUNK_C,SPRITE_C,LEVEL_C,GLOBAL_C/*Not allowed for luaControl*/};
struct luaDat{
	uint32_t len;
	std::string name;
	enum luaLocEnum datLoc;
	void*dat;
};
enum luaControlType{INPUT_T,RADIO_T,INT_INPUT_T,CHECKBOX_T,MENU_T};
struct luaControl{
	uint32_t len;
	enum luaLocEnum ctrlLoc;
	enum luaControlType ctrlType;
	bool cbInFile;
	std::string name,callback,callbackwhere;
	void*dat;//Points to a structure that depends on
};
struct luaScript{
	std::string str,name;
};
struct Project{/*!<Holds all data needed for a project based system for example tile screen and level 1 are 2 separate projects*/
	Project();
	Project(const Project&other);
	void copyClasses(const Project&other);
	~Project();
	bool isShared(uint32_t mask);
	bool isUniqueData(uint32_t mask);
	bool containsData(uint32_t mask);
	bool containsDataOR(uint32_t mask);
	enum TMS9918SubSys getTMS9918subSys()const{
		return (enum TMS9918SubSys)(subSystem&3);
	}
	void setTMS9918subSys(enum TMS9918SubSys sys){
		subSystem&=~3;
		subSystem|=sys;
	}
	void setBitdepthSys(unsigned bd);
	int getBitdepthSysraw(void)const;
	int getBitdepthSys(void)const{
		return getBitdepthSysraw()+1;
	}
	unsigned szPerExtPalRow(void);
	unsigned extAttrTilesPerByte(void);
	bool isFixedPalette(void);
	bool isPlanarTiles(void);
	int fixedSpirtePalRow(void);
	std::string Name;
	gameSystemEnum gameSystem;
	uint32_t subSystem;
	uint32_t settings;
	uint32_t useMask;/*!<Sharing can be used regardless of use mask*/
	class tilemaps*tms;
	tiles*tileC;
	ChunkClass*Chunk;
	palette*pal;
	sprites*spritesC;
	int32_t share[shareAmtPj];/*!<Negative if not sharing or project id (which is always positive) if sharing*/
	unsigned curPlane;
	uint32_t lauDatAmt;
	uint32_t luaControlAmt;
	uint32_t lScptAmt;
	struct luaDat*lDat;
	struct luaControl*lCtrl;
	struct luaScript*lScpt;
};
extern struct Project ** projects;
extern uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
extern struct Project * currentProject;
extern Fl_Slider* curPrj;
void changeTileDim(unsigned w,unsigned h,struct Project*p);
const char*maskToName(unsigned mask);
void compactPrjMem(void);
void initProject(void) __attribute__((constructor(101)));/*!< this needs to be ran before class constructors*/
void setHaveProject(uint32_t id,uint32_t mask,bool set);
void shareProject(uint32_t share,uint32_t with,uint32_t what,bool enable);
void prjChangePtr(unsigned id);
bool appendProject();
bool removeProject(uint32_t id);
void switchProject(uint32_t id);
bool loadProject(uint32_t id);
bool saveProject(uint32_t id);
bool saveAllProjects(void);
bool loadAllProjects(void);
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
