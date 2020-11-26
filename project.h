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
#ifndef _PROJECT_H
#define _PROJECT_H 1

#define shareAmtPj 6
#include <FL/Fl_Slider.H>
#include <string>
#include <map>
#include <vector>

#include "system.h"
#include "tilemap.h"
#include "class_tiles.h"
#include "classtilemap.h"
#include "classChunks.h"
#include "classSprites.h"
#include "color_compare.h"
#include "classpalette.h"
#include "classtilemaps.h"
#include "classlevel.h"
#include "metasprites.h"
#include "palette.h"
#define currentProjectVersionNUM 8
extern uint32_t curProjectID;
enum luaLocEnum {PALETTE_C, TILEMAP_C, CHUNK_C, SPRITE_C, LEVEL_C, GLOBAL_C/*Not allowed for luaControl*/, CUSTOM_TAB_0/*And so on*/};
enum luaControlType {INPUT_T, RADIO_T, INT_INPUT_T, CHECKBOX_T, CHOICE_T};
struct inputLua {
	std::string label, value;
};
struct intInputLua {
	std::string label;
	int64_t val;
};
struct luaControl {
	uint32_t len;
	enum luaLocEnum ctrlLoc;
	enum luaControlType ctrlType;
	bool cbInFile;
	std::string callback, callbackwhere;
	void*info;
};
struct luaScript {
	std::string str, name;
};
struct Project { /*!<Holds all data needed for a project based system for example tile screen and level 1 are 2 separate projects*/
	Project();
	Project(const Project& other); // Copy constructor
	Project& operator=(const Project& other);
	Project(Project&& other) noexcept;
	Project& operator=(Project&& other) noexcept;
	void copyConstructorCommon(const Project&other);
	void moveConstructorCommon(Project&& other);
	void copyClasses(const Project&other);
	~Project();
	void deleteClasses();
	void haveMessage(uint32_t mask);
	bool isShared(uint32_t mask);
	bool isUniqueData(uint32_t mask);
	bool containsData(uint32_t mask);
	bool containsDataOR(uint32_t mask);

	enum TMS9918SubSys getTMS9918subSys()const {
		return (enum TMS9918SubSys)(subSystem & 3);
	}
	void setTMS9918subSys(enum TMS9918SubSys sys);
	uint8_t getPalColTMS9918() {
		return (subSystem >> 3) & 255;
	}
	void setPalColTMS9918(uint8_t val) {
		subSystem &= (~(255 << 3));
		subSystem |= (val << 3);
	}

	void setBitdepthSys(unsigned bd);
	int getBitdepthSysraw(void)const;
	int getBitdepthSys(void)const {
		return getBitdepthSysraw() + 1;
	}
	int getPalTab(void)const {
		palTab = palTabPtr[(subSystem >> 3) & 7];
		return (subSystem >> 3) & 7;
	}
	void setPalTab(unsigned val) {
		val &= 7;
		subSystem &= ~(7 << 3);
		subSystem |= val << 3;
		palTab = palTabPtr[val];
	}
	bool isNES2x2Tilemap(void) {
		return gameSystem == NES && (!!(subSystem & NES2x2));
	}
	unsigned extAttrTilesPerByte(void);
	unsigned extAttrBytesPerTile(void);
	bool isFixedPalette(void);
	bool hasExtAttrs(void);
	bool supportsFlippedTiles(void);
	void getSpriteSizeMinMax(unsigned & minx, unsigned & miny, unsigned & maxx, unsigned & maxy);
	enum tileType getTileType(void);
	int fixedSpirtePalRow(void);
	void setSpriteSizeID(unsigned sizeID);
	unsigned getSpriteSizeID() const;
	std::string Name;
	gameSystemEnum gameSystem;
	uint32_t subSystem;
	uint32_t settings;
	uint32_t luaSettings;
	uint32_t useMask;/*!<Sharing can be used regardless of use mask*/
	class tilemaps*tms;
	class tiles*tileC;
	class ChunkClass*Chunk;
	palette*pal;
	struct metasprites*ms;
	struct level*lvl;
	int32_t share[shareAmtPj];/*!<Negative if not sharing or project id (which is always positive) if sharing*/
	unsigned curPlane;
	std::map<std::vector<uint8_t>, std::vector<uint8_t>> luaStringStore;
	std::vector<struct luaControl>lCtrl;
	std::vector<struct luaScript>lScrpt;
	std::vector<std::string>luaTabs;


	bool loadProjectFile(FILE * fi, bool loadVersion = true, uint32_t version = currentProjectVersionNUM);
	bool saveProjectFile(FILE * fo, bool saveShared, bool saveVersion = true);
	void changeTileDim(unsigned w, unsigned h);
};
extern std::vector<struct Project>* projects;
extern struct Project * currentProject;
extern Fl_Slider* curPrj;
const char*maskToName(unsigned mask);
void compactPrjMem(void);
void initProject(void);
void setHaveProject(uint32_t id, uint32_t mask, bool set);
void shareProject(uint32_t share, uint32_t with, uint32_t what, bool enable);
void prjChangePtr(unsigned id);
int resizeProjects(size_t amt);
void changeProjectAmt(void);
bool appendProject();
bool removeProject(uint32_t id);
void switchProjectSlider(uint32_t id, bool oldExists = true);
void switchProject(uint32_t id, uint32_t oldID, bool load = false);
bool loadProject(uint32_t id, const char*fname);
bool saveProject(uint32_t id, const char*fname);
bool saveAllProjects(const char* projectGroupFilename);
bool loadAllProjects(const char*fname);
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
