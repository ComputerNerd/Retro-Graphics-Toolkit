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
	Copyright Sega16 (or whatever you wish to call me) (2012-2019)
*/
#include <FL/fl_ask.H>

#include "project.h"
#include "color_convert.h"
#include "zlibwrapper.h"
#include "classpalettebar.h"
#include "callbacktilemaps.h"
#include "palette.h"
#include "class_global.h"
#include "gui.h"
#include "callback_gui.h"
#include "callbacklua.h"
#include "luaconfig.h"
#include "luaProjects.hpp"
#include "runlua.h"
#include "filemisc.h"
#include "errorMsg.h"


extern editor *window;
std::vector<struct Project>* projects;
struct Project * currentProject;
Fl_Slider* curPrj;
static const char * defaultName = "Add a description here.";
uint32_t curProjectID;
static const char*const maskNames[] = {"palette", "tiles", "tilemap", "chunks", "sprites", "level", "{Undefined}"};
void Project::changeTileDim(unsigned w, unsigned h) {
	if (containsData(pjHaveTiles)) {
		unsigned sw = tileC->width(), sh = tileC->height();
		tileC->changeDim(w, h, getBitdepthSys());

		if (sw > w && sh > h && containsData(pjHaveMap) && tms) {
			for (size_t i = 0; i < tms->maps.size(); ++i) {
				tileMap*tm = new tileMap(tms->maps[i], this);
				tms->maps[i].resize_tile_map(tms->maps[i].mapSizeW * sw / w, tms->maps[i].mapSizeH * sh / h);
				delete tm;
			}
		}
	}
}
const char*maskToName(unsigned mask) {
	unsigned off = __builtin_ctz(mask);
	return maskNames[off >= 6 ? 6 : off];
}
void Project::haveMessage(uint32_t mask) {
	std::string msg = "Current project:";

	for (unsigned x = 0; x <= pjMaxMaskBit; ++x) {
		if (mask & (1 << x)) {
			msg.append(currentProject->containsData(1 << x) ? "\nhas " : "\ndoes not have ");
			msg.append(maskToName(1 << x));
		}
	}

	fl_alert("%s", msg.c_str());
}
bool Project::isShared(uint32_t mask) {
	bool andIt = true;

	for (unsigned m = 0; m <= pjMaxMaskBit; ++m) {
		if (mask & (1 << m)) {
			andIt &= share[m] >= 0;

			if (!andIt)
				break;
		}
	}

	return andIt;
}
bool Project::isUniqueData(uint32_t mask) {
	bool andIt = true;

	for (unsigned m = 0; m <= pjMaxMaskBit; ++m) {
		if (mask & (1 << m)) {
			andIt &= ((useMask & (1 << m)) && (share[m] < 0));

			if (!andIt)
				break;
		}
	}

	return andIt;
}
bool Project::containsData(uint32_t mask) {
	bool andIt = true;

	for (unsigned m = 0; m <= pjMaxMaskBit; ++m) {
		if (mask & (1 << m)) {
			andIt &= ((useMask & (1 << m)) || (share[m] >= 0));

			if (!andIt)
				break;
		}
	}

	return andIt;
}
bool Project::containsDataOR(uint32_t mask) {
	bool orIt = false;

	for (unsigned m = 0; m <= pjMaxMaskBit; ++m) {
		if (mask & (1 << m))
			orIt |= ((useMask & (1 << m)) || (share[m] >= 0));
	}

	return orIt;
}
void compactPrjMem(void) {
	int Cold = 0, Cnew = 0; //Old and new capacity

	for (uint_fast32_t i = 0; i < projects->size(); ++i) {
		Project& prj = projects->at(i);

		if (prj.containsData(pjHaveTiles)) {
			Cold += prj.tileC->tDat.capacity();
			Cold += prj.tileC->truetDat.capacity();
			prj.tileC->tDat.shrink_to_fit();
			prj.tileC->truetDat.shrink_to_fit();
			Cnew += prj.tileC->tDat.capacity();
			Cnew += prj.tileC->truetDat.capacity();
		}

		if (prj.containsData(pjHaveChunks)) {
			Cold += prj.Chunk->chunks.capacity();
			prj.Chunk->chunks.shrink_to_fit();
			Cnew += prj.Chunk->chunks.capacity();
		}

		if (prj.containsData(pjHaveSprites)) {
			for (uint32_t j = 0; j < prj.ms->sps.size(); ++j) {
				for (uint32_t n = 0; n < prj.ms->sps[j].amt; ++n) {
					Cold += prj.ms->sps[j].groups[n].list.capacity();
					prj.ms->sps[j].groups[n].list.shrink_to_fit();
					Cnew += prj.ms->sps[j].groups[n].list.capacity();
				}

				Cold += prj.ms->sps[j].groups.capacity();
				prj.ms->sps[j].groups.shrink_to_fit();
				Cnew += prj.ms->sps[j].groups.capacity();
			}
		}
	}

	printf("Old capacity: %d New capacity: %d saved %d bytes\n", Cold, Cnew, Cold - Cnew);
}

Project::Project() {
	Name.assign(defaultName);
	gameSystem = segaGenesis;
	subSystem = 3;
	settings = (15 << subsettingsDitherShift) | (aCiede2000 << nearestColorShift);
	curPlane = 0;
	useMask = 0;
	std::fill(share, &share[shareAmtPj], -1);
}

void Project::copyConstructorCommon(const Project&other) {
	Name = other.Name;
	gameSystem = other.gameSystem;
	subSystem = other.subSystem;
	settings = other.settings;
	luaSettings = other.luaSettings;
	useMask = other.useMask;
	memcpy(share, other.share, sizeof(Project::share));
	curPlane = other.curPlane;
	lDat = other.lDat;
	lCtrl = other.lCtrl;
	lScrpt = other.lScrpt;
	luaTabs = other.luaTabs;

	copyClasses(other);
}

Project::Project(const Project& other) {
	copyConstructorCommon(other);
}

void Project::moveConstructorCommon(Project&& other) {
	Name = std::move(other.Name);

	gameSystem = other.gameSystem;
	subSystem = other.subSystem;
	settings = other.settings;
	luaSettings = other.luaSettings;

	useMask = other.useMask;
	other.useMask = 0;

	memcpy(share, other.share, sizeof(Project::share));
	curPlane = other.curPlane;
	lDat = std::move(other.lDat);
	lCtrl = std::move(other.lCtrl);
	lScrpt = std::move(other.lScrpt);
	luaTabs = std::move(other.luaTabs);

	tms = std::move(other.tms);

	if (tms)
		tms->prj = this;

	other.tms = nullptr;

	tileC = std::move(other.tileC);

	if (tileC)
		tileC->prj = this;

	other.tileC = nullptr;

	Chunk = std::move(other.Chunk);

	if (Chunk)
		Chunk->prj = this;

	other.Chunk = nullptr;

	pal = std::move(other.pal);

	if (pal)
		pal->prj = this;

	other.pal = nullptr;

	ms = std::move(other.ms);

	if (ms)
		ms->setPrjPtr(this);

	other.ms = nullptr;

	lvl = std::move(other.lvl);

	if (lvl)
		lvl->prj = this;

	other.lvl = nullptr;
}

Project::Project(Project&& other) noexcept {
	moveConstructorCommon(std::move(other));
}

Project& Project::operator=(Project&& other) noexcept {
	if (this != &other)
		moveConstructorCommon(std::move(other));

	return *this;
}

Project& Project::operator=(const Project& other) {
	if (this != &other)
		copyConstructorCommon(other);

	return *this;
}

void Project::copyClasses(const Project&other) {
	if (isUniqueData(pjHavePal))
		pal = new palette(*other.pal, this);
	else if (isShared(pjHavePal))
		pal = other.pal;
	else
		pal = nullptr;

	if (isUniqueData(pjHaveTiles))
		tileC = new tiles(*other.tileC, this);
	else if (isShared(pjHaveTiles))
		tileC = other.tileC;
	else
		tileC = nullptr;

	if (isUniqueData(pjHaveMap))
		tms = new tilemaps(*other.tms, this);
	else if (isShared(pjHaveMap))
		tms = other.tms;
	else
		tms = nullptr;

	if (isUniqueData(pjHaveChunks))
		Chunk = new ChunkClass(*other.Chunk, this);
	else if (isShared(pjHaveChunks))
		Chunk = other.Chunk;
	else
		Chunk = nullptr;

	if (isUniqueData(pjHaveSprites))
		ms = new metasprites(*other.ms, this);
	else if (isShared(pjHaveSprites))
		ms = other.ms;
	else
		ms = nullptr;

	if (isUniqueData(pjHaveLevel))
		lvl = new level(*other.lvl, this);
	else if (isShared(pjHaveLevel))
		lvl = other.lvl;
	else
		lvl = nullptr;
}

static void initNewProject(unsigned at) {
	Project& prj = projects->at(at);
	prj.tileC = new tiles(&prj);
	prj.tms = new tilemaps(&prj);
	prj.Chunk = new ChunkClass(&prj);
	prj.ms = new metasprites(&prj);
	prj.pal = new palette(&prj);
	prj.lvl = new level(&prj);
	prj.useMask = pjDefaultMask;
}

static bool didInitLconf;
void initProject(void) {
	projects = new std::vector<struct Project>();

	if (!didInitLconf) {
		luaCreateProjectsTable(Lconf);
		setProjectConstants(Lconf);
	}

	resizeProjects(1);
	initNewProject(0);

	if (!didInitLconf) {
		updateProjectTablesLua(Lconf);
		didInitLconf = true;
	}
}

void setHaveProject(uint32_t id, uint32_t mask, bool set) {
	/*This function will allocate/free data if and only if it is not being shared
	if have is already enabled no new data will be allocated
	if have was enabled data will be freeded*/
	Project& prjAtID = projects->at(id);

	if ((mask & pjHavePal) && (prjAtID.share[0] < 0)) {
		if (set) {
			if (!(prjAtID.useMask & pjHavePal)) {
				prjAtID.pal = new palette(&prjAtID);
				prjAtID.useMask |= pjHavePal;
				palBar.setSys();

				if (prjAtID.gameSystem == NES)
					prjAtID.pal->updateEmphasis();
			}
		} else {
			if (prjAtID.useMask & pjHavePal) {
				delete prjAtID.pal;
				prjAtID.useMask &= ~pjHavePal;
				prjAtID.pal = nullptr;
			}
		}
	}

	if ((mask & pjHaveTiles) && (prjAtID.share[1] < 0)) {
		if (set) {
			if (!(prjAtID.useMask & pjHaveTiles)) {
				prjAtID.tileC = new tiles(&prjAtID, &prjAtID);
				prjAtID.useMask |= pjHaveTiles;
			}
		} else {
			if (prjAtID.useMask & pjHaveTiles) {
				delete prjAtID.tileC;
				prjAtID.useMask &= ~pjHaveTiles;
				prjAtID.tileC = nullptr;
			}
		}
	}

	if ((mask & pjHaveMap) && (prjAtID.share[2] < 0)) {
		if (set) {
			if (!(prjAtID.useMask & pjHaveMap)) {
				prjAtID.tms = new tilemaps(&prjAtID);
				prjAtID.useMask |= pjHaveMap;
			}
		} else {
			if (prjAtID.useMask & pjHaveMap) {
				delete prjAtID.tms;
				prjAtID.useMask &= ~pjHaveMap;
				prjAtID.tms = nullptr;
			}
		}
	}

	if ((mask & pjHaveChunks) && (prjAtID.share[3] < 0)) {
		if (set) {
			if (!(prjAtID.useMask & pjHaveChunks)) {
				prjAtID.Chunk = new ChunkClass(&prjAtID);
				prjAtID.useMask |= pjHaveChunks;
			}
		} else {
			if (prjAtID.useMask & pjHaveChunks) {
				delete prjAtID.Chunk;
				prjAtID.useMask &= ~pjHaveChunks;
				prjAtID.Chunk = nullptr;
			}
		}
	}

	if ((mask & pjHaveSprites) && (prjAtID.share[4] < 0)) {
		if (set) {
			if (!(prjAtID.useMask & pjHaveSprites)) {
				if (window)
					window->spriteglobaltxt->show();

				prjAtID.ms = new metasprites(&prjAtID);
				prjAtID.useMask |= pjHaveSprites;
			}
		} else {
			if (prjAtID.useMask & pjHaveSprites) {
				if (window)
					window->spriteglobaltxt->hide();

				delete prjAtID.ms;
				prjAtID.useMask &= ~pjHaveSprites;
				prjAtID.ms = nullptr;
			}
		}
	}

	if ((mask & pjHaveLevel) && (prjAtID.share[5] < 0)) {
		if (set) {
			if (!(prjAtID.useMask & pjHaveLevel)) {
				prjAtID.useMask |= pjHaveLevel;
				prjAtID.lvl = new level(&prjAtID);
			}
		} else {
			if (prjAtID.useMask & pjHaveLevel) {
				prjAtID.useMask &= ~pjHaveLevel;
				delete prjAtID.lvl;
				prjAtID.lvl = nullptr;
			}
		}
	}
}

void shareProject(uint32_t share, uint32_t with, uint32_t what, bool enable) {
	/*! share is the project that will now point to with's data
	what uses, use masks
	This function will not alter the GUI*/
	if (share == with) {
		fl_alert("One does not simply share with itself");
		return;
	}

	Project& prjAtShare = projects->at(share);
	Project& prjAtWith = projects->at(with);

	if (enable) {
		if (what & pjHavePal) {
			if ((prjAtShare.share[0] < 0) && (prjAtShare.useMask & pjHavePal))
				delete prjAtShare.pal;

			prjAtShare.share[0] = with;
			prjAtShare.pal = prjAtWith.pal;
		}

		if (what & pjHaveTiles) {
			if ((prjAtShare.share[1] < 0) && (prjAtShare.useMask & pjHaveTiles))
				delete prjAtShare.tileC;

			prjAtShare.share[1] = with;
			prjAtShare.tileC = prjAtWith.tileC;
		}

		if (what & pjHaveMap) {
			if ((prjAtShare.share[2] < 0) && (prjAtShare.useMask & pjHaveMap))
				delete prjAtShare.tms;

			prjAtShare.share[2] = with;
			prjAtShare.tms = prjAtWith.tms;
		}

		if (what & pjHaveChunks) {
			if ((prjAtShare.share[chunkEditor] < 0) && (prjAtShare.useMask & pjHaveChunks))
				delete prjAtShare.Chunk;

			prjAtShare.share[chunkEditor] = with;
			prjAtShare.Chunk = prjAtWith.Chunk;
		}

		if (what & pjHaveSprites) {
			if ((prjAtShare.share[spriteEditor] < 0) && (prjAtShare.useMask & pjHaveSprites))
				delete prjAtShare.ms;

			prjAtShare.share[spriteEditor] = with;
			prjAtShare.ms = prjAtWith.ms;
		}

		if (what & pjHaveLevel) {
			if ((prjAtShare.share[levelEditor] < 0) && (prjAtShare.useMask & pjHaveLevel))
				delete prjAtShare.lvl;

			prjAtShare.share[levelEditor] = with;
			prjAtShare.lvl = prjAtWith.lvl;
		}
	} else {
		if (what & pjHavePal) {
			if ((prjAtShare.share[pal_edit] >= 0) && (prjAtShare.useMask & pjHavePal))
				prjAtShare.pal = new palette(*prjAtWith.pal, &prjAtShare);

			prjAtShare.share[pal_edit] = -1;
		}

		if (what & pjHaveTiles) {
			if ((prjAtShare.share[tile_edit] >= 0) && (prjAtShare.useMask & pjHaveTiles)) //Create a copy of the shared data
				prjAtShare.tileC = new tiles(*prjAtWith.tileC, &prjAtShare);

			prjAtShare.share[tile_edit] = -1;
		}

		if (what & pjHaveMap) {
			if ((prjAtShare.share[tile_place] >= 0) && (prjAtShare.useMask & pjHaveMap))
				prjAtShare.tms = new tilemaps(*prjAtWith.tms, &prjAtShare);

			prjAtShare.share[tile_place] = -1; //Even if we don't have the data sharing can still be disabled
		}

		if (what & pjHaveChunks) {
			if ((prjAtShare.share[chunkEditor] >= 0) && (prjAtShare.useMask & pjHaveChunks))
				prjAtShare.Chunk = new ChunkClass(*prjAtWith.Chunk, &prjAtShare);

			prjAtShare.share[chunkEditor] = -1; //Even if we don't have the data sharing can still be disabled
		}

		if (what & pjHaveSprites) {
			if ((prjAtShare.share[spriteEditor] >= 0) && (prjAtShare.useMask & pjHaveSprites))
				prjAtShare.ms = new metasprites(*prjAtWith.ms, &prjAtShare);

			prjAtShare.share[spriteEditor] = -1; //Even if we don't have the data sharing can still be disabled
		}

		if (what & pjHaveLevel) {
			if ((prjAtShare.share[levelEditor] >= 0) && (prjAtShare.useMask & pjHaveLevel))
				prjAtShare.lvl = new level(*prjAtWith.lvl, &prjAtShare);

			prjAtShare.share[levelEditor] = -1; //Even if we don't have the data sharing can still be disabled
		}
	}
}

void prjChangePtr(unsigned id) {
	Project& project = projects->at(id);

	if (project.isUniqueData(pjHavePal))
		project.pal->prj = &project;

	if (project.isUniqueData(pjHaveTiles))
		project.tileC->prj = &project;

	if (project.isUniqueData(pjHaveMap))
		project.tms->changePrjPtr(&project);

	if (project.isUniqueData(pjHaveSprites))
		project.ms->setPrjPtr(&project);
}

int resizeProjects(size_t amt) {
	projects->resize(amt);

	for (size_t i = 0; i < projects->size(); ++i)
		prjChangePtr(i);

	if (curProjectID >= amt) {
		curProjectID = amt - 1;
		switchProjectSlider(curProjectID);
	}

	currentProject = &projects->at(curProjectID); //The address may have changed
	return 0;
}

void changeProjectAmt(void) {
	if (!window)
		return;

	if (curProjectID >= projects->size())
		switchProjectSlider(projects->size() - 1, false);

	window->projectSelect->maximum(projects->size() - 1);

	for (int x = 0; x < shareAmtPj; ++x)
		window->shareWith[x]->maximum(projects->size() - 1);
}

bool appendProject() {
	if (resizeProjects(projects->size() + 1))
		return false;

	initNewProject(projects->size() - 1);
	changeProjectAmt();
	return true;
}

void Project::deleteClasses() {
	if (isUniqueData(pjHavePal) && pal) {
		delete pal;
		pal = nullptr;
	}

	if (isUniqueData(pjHaveTiles) && tileC) {
		delete tileC;
		tileC = nullptr;
	}

	if (isUniqueData(pjHaveMap) && tms) {
		delete tms;
		tms = nullptr;
	}

	if (isUniqueData(pjHaveChunks)) {
		delete Chunk;
		Chunk = nullptr;
	}

	if (isUniqueData(pjHaveSprites)) {
		delete ms;
		ms = nullptr;
	}
}

Project::~Project() {
	deleteClasses();
}

bool removeProject(uint32_t id) {
	//removes selected project
	if (projects->size() <= 1) {
		fl_alert("You must have at least one project.");
		return false;
	}

	projects->erase(projects->begin() + id);

	currentProject = &projects->at(curProjectID); //The address may have changed

	changeProjectAmt();
	return true;
}

static void invaildProject(void) {
	fl_alert("This is not a valid Retro Graphics Toolkit project.");
}

extern Fl_Menu_Item subSysNES[];
extern Fl_Menu_Item subSysGenesis[];
extern Fl_Menu_Item subSysTMS9918[];
void switchProjectSlider(uint32_t id, bool oldExists) {
	if (!window)
		return;

	if (oldExists)
		currentProject->Name.assign(window->TxtBufProject->text());//Save text to old project

	window->projectSelect->value(id);
	auto oldID = curProjectID;
	curProjectID = id;
	currentProject = &projects->at(curProjectID);
	switchProject(curProjectID, oldID);
}
extern int curScript;
static void updateLuaScriptWindow(uint32_t id, uint32_t oldID, bool load = false) {
	if (!window)
		return;

	Project&prjAtID = projects->at(id);
	size_t amt = prjAtID.lScrpt.size();

	if ((curScript >= 0) && (!load))
		projects->at(oldID).lScrpt[curScript].str.assign(window->luaBufProject->text()); // currentProject is the old ID.

	window->luaScriptSel->clear();

	if (amt) {
		window->luaEditProject->show();

		for (unsigned i = 0; i < amt; ++i)
			window->luaScriptSel->add(prjAtID.lScrpt[i].name.c_str(), 0, switchCurLuaScript, (void*)(intptr_t)i);

		window->luaScriptSel->value(0);
		switchCurLuaScript(nullptr, (void*)(intptr_t)0);
	} else {
		window->luaEditProject->hide();
		curScript = -1;
	}
}
void switchProject(uint32_t id, uint32_t oldID, bool load) {
	Project&prj = projects->at(id);

	if (window) {
		updateLuaScriptWindow(id, oldID, load);
		window->TxtBufProject->text(prj.Name.c_str());//Make editor displays new text
		window->gameSysSel->value(prj.gameSystem);
		window->ditherPower->value(1 + ((prj.settings >> subsettingsDitherShift)&subsettingsDitherMask));
		window->ditherAlgSel->value(prj.settings & subsettingsDitherMask);

		if ((prj.settings & subsettingsDitherMask))
			window->ditherPower->hide();
		else
			window->ditherPower->show();
	}

	if (prj.containsData(pjHavePal))
		prj.pal->setVars(prj.gameSystem);

	switch (prj.gameSystem) {
		case segaGenesis:
			if (window) {
				window->subSysC->copy(subSysGenesis);
				window->subSysC->value((prj.subSystem & sgSHmask) >> sgSHshift);
			}

			if (prj.containsData(pjHaveTiles))
				prj.tileC->tileSize = 32;

			if (prj.containsData(pjHavePal)) {
				palBar.setSys(true, true);
				set_palette_type();
			}

			break;

		case NES:
			if (window) {
				window->subSysC->copy(subSysNES);
				window->subSysC->value(prj.subSystem & 1);
			}

			if (prj.containsData(pjHaveTiles))
				prj.tileC->tileSize = 16;

			if (prj.containsData(pjHavePal)) {
				palBar.setSys();
				prj.pal->updateEmphasis();
			}

			break;

		case masterSystem:
		case gameGear:
			if (prj.containsData(pjHaveTiles))
				prj.tileC->tileSize = 32;

			if (prj.containsData(pjHavePal)) {
				palBar.setSys();
				prj.pal->paletteToRgb();
			}

			break;

		case TMS9918:
			setGameSysTMS9918(&prj);

			if (window) {
				window->subSysC->copy(subSysTMS9918);
				window->subSysC->value(prj.getTMS9918subSys());
			}

			if (prj.containsData(pjHavePal))
				palBar.setSys();

			if (prj.containsData(pjHaveTiles))
				prj.tileC->tileSize = 8;

			break;

		default:
			show_default_error
	}

	//Make sure sliders have correct values
	if (prj.containsData(pjHaveMap)) {
		if (window) {
			updatePlaneTilemapMenu(id, window->planeSelect);
			window->updateMapWH(prj.tms->maps[prj.curPlane].mapSizeW, prj.tms->maps[prj.curPlane].mapSizeH);
			char tmp[16];
			snprintf(tmp, 16, "%u", prj.tms->maps[prj.curPlane].amt);
			window->map_amt->value(tmp);
		}
	}

	if (prj.containsData(pjHaveTiles) && window)
		updateTileSelectAmt(prj.tileC->amt);

	for (int x = 0; x < shareAmtPj; ++x) {
		if (window) {
			window->sharePrj[x]->value(prj.share[x] < 0 ? 0 : 1);
			window->havePrj[x]->value(prj.useMask >> x & 1);

			if (prj.share[x] < 0)
				window->havePrj[x]->show();
			else
				window->havePrj[x]->hide();

			if (prj.useMask >> x & 1) {
				if (window->tabsHidden[x]) {
					window->the_tabs->insert(*window->tabsMain[x], x);
					window->tabsHidden[x] = false;
				}
			} else {
				if (!window->tabsHidden[x]) {
					if (prj.share[x] < 0) {
						window->the_tabs->remove(window->tabsMain[x]);
						window->tabsHidden[x] = true;
					}
				}
			}
		}
	}

	if (prj.containsData(pjHaveMap) && window)
		window->BlocksCBtn->value(prj.tms->maps[prj.curPlane].isBlock ? 1 : 0);

	if (prj.containsData(pjHaveChunks) && window) {
		window->chunk_select->maximum(prj.Chunk->amt - 1);
		window->updateChunkSize(prj.Chunk->wi, prj.Chunk->hi);
	}

	if (prj.containsData(pjHaveMap))
		prj.tms->maps[prj.curPlane].toggleBlocks(prj.tms->maps[prj.curPlane].isBlock);

	if (prj.containsData(pjHaveChunks) && window)
		window->updateBlockTilesChunk(id);

	if (prj.containsData(pjHaveSprites) && window) {
		window->updateSpriteSliders(id);
		window->spriteglobaltxt->show();
		window->spriteglobaltxt->value(prj.ms->name.c_str());
	} else if (window)
		window->spriteglobaltxt->hide();

	lua_getglobal(Lconf, "switchProject");
	runLuaFunc(Lconf, 0, 0);

	if (window)
		window->redraw();
}
bool Project::loadProjectFile(FILE * fi, bool loadVersion, uint32_t version) {
	if (fgetc(fi) != 'R') {
		invaildProject();
		fclose(fi);
		return false;
	}

	char sc;

	if ((sc = fgetc(fi)) != 'P') {
		if (sc == 'G')
			fl_alert("This appears to be a project group; not a project file. If that is the case use the load project group option.");
		else
			invaildProject();

		fclose(fi);
		return false;
	}

	// First clear out any of the old classes.
	deleteClasses();

	fileToStr(fi, Name, defaultName);

	if (loadVersion) {
		fread(&version, 1, sizeof(uint32_t), fi);
		printf("Read as version %d\n", version);
	}

	if (version > currentProjectVersionNUM) {
		fl_alert("The latest project version Retro Graphics Toolkit supports is %u but you are opening a project with version: %u", currentProjectVersionNUM, version);
		fclose(fi);
		return false;
	}

	if (version)
		fread(&useMask, 1, sizeof(uint32_t), fi);
	else // Version zero did not support the use mask.
		useMask = pjHavePal | pjHaveTiles | pjHaveMap;

	uint32_t gameTemp;
	fread(&gameTemp, 1, sizeof(uint32_t), fi);
	gameSystem = (gameSystemEnum)gameTemp;

	if (version >= 4) {
		fread(&subSystem, 1, sizeof(uint32_t), fi);

		if ((version < 6) && (gameSystem == segaGenesis))
			subSystem = 3; //Old projects were storing the wrong number for 4bit graphics even though that is what is stored

		if ((version == 4) && (gameSystem == NES)) {
			subSystem ^= 1; //Fix the fact that NES2x2 and NES1x1 were switched around in version 4
			subSystem |= 2; //Default to 2 bit
		}
	} else
		subSystem = 3;

	if (version >= 8) {
		fread(&settings, 1, sizeof(uint32_t), fi);
		fread(&luaSettings, 1, sizeof(uint32_t), fi);
	} else {
		settings = 15 << subsettingsDitherShift | (aWeighted << nearestColorShift);
		luaSettings = 0;
	}

	if (useMask & pjHavePal) {
		if (share[0] < 0) {
			pal = new palette(this);

			pal->setVars(gameSystem);
			pal->read(fi, version >= 7);
		}
	}

	if (useMask & pjHaveTiles) {
		if (share[1] < 0) {
			tileC = new tiles(this);

			if (gameSystem == TMS9918)
				setGameSysTMS9918(this);
			else
				tileC->changeDim(8, 8, getBitdepthSys());

			fread(&tileC->amt, 1, sizeof(uint32_t), fi);

			if (version < 6)
				++tileC->amt;

			tileC->resizeAmt();
			decompressFromFile(tileC->tDat.data(), tileC->tileSize * (tileC->amt), fi);
			decompressFromFile(tileC->truetDat.data(), tileC->tcSize * (tileC->amt), fi);

			if (version >= 8) {
				size_t extSize = tileC->extAttrs.size();

				if (extSize > 0)
					decompressFromFile(tileC->extAttrs.data(), extSize, fi);
			}

			if (version <= 7 && (getTileType() != PLANAR_TILE))
				tileC->toPlanar(getTileType());
		}
	}

	if (useMask & pjHaveMap) {
		if (share[2] < 0) {
			tms = new tilemaps(this);

			uint32_t readCnt;

			if (version >= 8)
				fread(&readCnt, 1, sizeof(uint32_t), fi);
			else
				readCnt = 1;

			curPlane = 0;
			tms->setPlaneCnt(readCnt);

			for (unsigned i = 0; i < readCnt; ++i) {
				if (version >= 8) {
					char firstC = fgetc(fi);

					if (firstC) {
						tms->maps[i].planeName.clear();

						do {
							tms->maps[i].planeName.push_back(firstC);
						} while ((firstC = fgetc(fi)));
					} else
						tms->assignNum(i);
				} else
					tms->assignNum(i);

				fread(&tms->maps[i].mapSizeW, 1, sizeof(uint32_t), fi);
				fread(&tms->maps[i].mapSizeH, 1, sizeof(uint32_t), fi);

				if (version >= 2) {
					uint8_t isBlockTemp;
					fread(&isBlockTemp, 1, sizeof(uint8_t), fi);
					tms->maps[i].isBlock = isBlockTemp ? true : false;

					if (isBlockTemp)
						fread(&tms->maps[i].amt, 1, sizeof(uint32_t), fi);
					else
						tms->maps[i].amt = 1;

					tms->maps[i].mapSizeHA = tms->maps[i].mapSizeH * tms->maps[i].amt;
				} else
					tms->maps[i].mapSizeHA = tms->maps[i].mapSizeH;

				if (version >= 8)
					fread(&tms->maps[i].offset, 1, sizeof(int32_t), fi);
				else
					tms->maps[i].offset = 0;

				tms->maps[i].tileMapDat = (uint8_t*)realloc(tms->maps[i].tileMapDat, 4 * tms->maps[i].mapSizeW * tms->maps[i].mapSizeHA);
				decompressFromFile(tms->maps[i].tileMapDat, 4 * tms->maps[i].mapSizeW * tms->maps[i].mapSizeHA, fi);
			}
		}
	}

	if (useMask & pjHaveChunks) {
		if (share[3] < 0) {
			if (version >= 3) {
				Chunk = new ChunkClass(this);
				uint8_t useBlockTemp;
				fread(&useBlockTemp, 1, sizeof(uint8_t), fi);
				Chunk->useBlocks = useBlockTemp ? true : false;
				fread(&Chunk->wi, 1, sizeof(uint32_t), fi);
				fread(&Chunk->hi, 1, sizeof(uint32_t), fi);
				fread(&Chunk->amt, 1, sizeof(uint32_t), fi);

				if (version >= 8)
					fread(&Chunk->usePlane, 1, sizeof(uint32_t), fi);

				Chunk->resizeAmt();
				decompressFromFile(Chunk->chunks.data(), Chunk->wi * Chunk->hi * sizeof(struct ChunkAttrs)*Chunk->amt, fi);
			}
		}
	}

	if (useMask & pjHaveSprites) {
		if (share[4] < 0) {
			if (version >= 5) {
				ms = new metasprites(this);
				ms->load(fi, version);
			}
		}
	}

	if (useMask & pjHaveLevel) {
		if (share[levelEditor] < 0) {
			if (version >= 8) {
				lvl = new level(this);
				lvl->load(fi, version);
			}
		}
	}

	if (version >= 8) {
		uint32_t scriptAmt, tabsAmt, controlDat, userDat;
		fread(&scriptAmt, 1, sizeof(uint32_t), fi);
		fread(&tabsAmt, 1, sizeof(uint32_t), fi);
		fread(&controlDat, 1, sizeof(uint32_t), fi);
		fread(&userDat, 1, sizeof(uint32_t), fi);

		lScrpt.clear();

		if (scriptAmt) {
			lScrpt.resize(scriptAmt);

			for (unsigned i = 0; i < scriptAmt; ++i) {
				fileToStr(fi, lScrpt[i].name, std::to_string(i).c_str());
				fileToStr(fi, lScrpt[i].str, "");
			}
		}

		if (tabsAmt || userDat || controlDat)
			fl_alert("This version of Retro Graphics Toolkit does not fully support Lua user data please upgrade.");
	}

	return true;
}

bool loadProject(uint32_t id, const char*fname) {
	FILE * fi = fopen(fname, "rb");

	Project&prj = projects->at(id);
	std::fill(prj.share, &prj.share[shareAmtPj], -1); //One file projects do not support sharing

	if (prj.loadProjectFile(fi))
		fclose(fi);

	return true;
}

bool Project::saveProjectFile(FILE * fo, bool saveShared, bool saveVersion) {
	/*!
	File format
	char R
	char P
	Null terminated project description or just 0 if default string
	uint32_t version the reason this is stored is for backwards compatibility if I change the file format starts at version 0
	if (version >= 1) uint32_t have mask
	You can find the format in project.h
	if these bits are zero skip it
	uint32_t game system
	if(version >= 4) uint32_t sub System requires special handling for version==4
	if(version>=8) uint32_t settings
	palette data (128 bytes if sega genesis or 16 bytes if NES)
	if((version>=7)&&(gameSystem==NES)) 16 bytes for sprite specific palette
	Free locked reserved data 64 bytes if sega genesis or 32 (16 if version<7) if NES
	uint32_t tile count
	uint32_t compressed size tiles
	tile data will decompress to either 32 bytes * tile count if sega genesis or 16 bytes * tile count if NES and is compressed with zlib
	uint32_t compressed size truecolor tiles
	true color tile data always decompresses to 256 bytes * tile count and is compressed with zlib
	if(version>=8) uint32_t plane count and what pertains to tilemap repeats for plane count
	if(version>=8) name null terminated or 0 for default name
	uint32_t map size w
	uint32_t map size h
	if(version>=2){
		uint8_t isBlocks 0 if not blocks non-zero if so
		if(isBlocks)
			uint32_t blocks amount also treat w and h as width and height per block
	}
	if(version>=8) int32_t tilemap tile offset
	uint32_t compressed size map
	map data will decompress to map size w * map size h * 4 and is compressed with zlib
	if(version>=3){
		uint8_t use blocks for chunks non zero if so 0 if not
		uint32_t width per chunk
		uint32_t height per chunk
		uint32_t amount of chunks
		uint32_t compressed Chunk map size
		if (version >= 8) uint32_t usePlane
		Chunk data (zlib compressed)
	}
	if(version>=5) sprite data (see documentation in classSprites.cpp)
	if(version>=8) level data
	if (version >= 8) {
		uint32_t luaScriptCount
		uint32_t luaTabsCount
		uint32_t luaControlCount
		uint32_t luaUserDataCount

		Lua user data Format:
			for each with count
				const char*name null terminated
				uint32_t length
				void*data

		Lua control data Format:
			for each with count
				const char*controlName null terminated
				uint32_t type
				uint32_t length
				void*data
	}
	*/
	fputc('R', fo);
	fputc('P', fo);
	saveStrifNot(fo, Name.c_str(), defaultName);

	if (saveVersion) {
		uint32_t version = currentProjectVersionNUM;
		fwrite(&version, sizeof(uint32_t), 1, fo);
	}

	uint32_t haveTemp;

	if (saveShared) {
		haveTemp = useMask;

		for (unsigned x = 0; x < shareAmtPj; ++x)
			haveTemp |= (share[x] >= 0 ? 1 : 0) << x;
	} else
		haveTemp = useMask;

	fwrite(&haveTemp, sizeof(uint32_t), 1, fo);
	uint32_t gameTemp = (uint32_t)gameSystem;
	fwrite(&gameTemp, 1, sizeof(uint32_t), fo);
	fwrite(&subSystem, sizeof(uint32_t), 1, fo);
	fwrite(&settings, sizeof(uint32_t), 1, fo);
	fwrite(&luaSettings, sizeof(uint32_t), 1, fo);

	if (haveTemp & pjHavePal) {
		if (saveShared || (share[0] < 0))
			pal->write(fo);
	}

	if (haveTemp & pjHaveTiles) {
		if (saveShared || (share[1] < 0)) {
			fwrite(&tileC->amt, 1, sizeof(uint32_t), fo);
			compressToFile(tileC->tDat.data(), tileC->tileSize * (tileC->amt), fo);
			compressToFile(tileC->truetDat.data(), tileC->tcSize * (tileC->amt), fo);
			{
				size_t extSize = tileC->extAttrs.size();

				if (extSize)
					compressToFile(tileC->extAttrs.data(), extSize, fo);
			}
		}
	}

	if (haveTemp & pjHaveMap) {
		if (saveShared || (share[2] < 0)) {
			uint32_t cnt = tms->maps.size();
			fwrite(&cnt, 1, 4, fo);

			for (unsigned i = 0; i < tms->maps.size(); ++i) {
				//Write the name or if default just write 0
				char tmp[16];
				snprintf(tmp, 16, "%u", i);

				if (strcmp(tms->maps[i].planeName.c_str(), tmp)) {
					const char*st = tms->maps[i].planeName.c_str();

					do {
						fputc(*st, fo);
					} while (*st++);
				} else
					fputc(0, fo);

				fwrite(&tms->maps[i].mapSizeW, 1, sizeof(uint32_t), fo);
				fwrite(&tms->maps[i].mapSizeH, 1, sizeof(uint32_t), fo);
				uint8_t isBlockTemp = tms->maps[i].isBlock ? 1 : 0;
				fwrite(&isBlockTemp, 1, sizeof(uint8_t), fo);

				if (isBlockTemp)
					fwrite(&tms->maps[i].amt, 1, sizeof(uint32_t), fo);

				fwrite(&tms->maps[i].offset, 1, sizeof(int32_t), fo);
				compressToFile(tms->maps[i].tileMapDat, 4 * tms->maps[i].mapSizeW * tms->maps[i].mapSizeHA, fo);
			}
		}
	}

	if (haveTemp & pjHaveChunks) {
		if (saveShared || (share[3] < 0)) {
			uint8_t useBlockTemp = Chunk->useBlocks ? 1 : 0;
			fwrite(&useBlockTemp, 1, sizeof(uint8_t), fo);
			fwrite(&Chunk->wi, 1, sizeof(uint32_t), fo);
			fwrite(&Chunk->hi, 1, sizeof(uint32_t), fo);
			fwrite(&Chunk->amt, 1, sizeof(uint32_t), fo);
			fwrite(&Chunk->usePlane, 1, sizeof(uint32_t), fo);
			compressToFile(Chunk->chunks.data(), Chunk->wi * Chunk->hi * sizeof(struct ChunkAttrs)*Chunk->amt, fo);
		}
	}

	if (haveTemp & pjHaveSprites) {
		if (saveShared || (share[4] < 0))
			ms->save(fo);
	}

	if (haveTemp & pjHaveLevel) {
		if (saveShared || (share[levelEditor] < 0))
			lvl->save(fo);
	}

	uint32_t luaSize = lScrpt.size();
	fwrite(&luaSize, 1, sizeof(uint32_t), fo);
	luaSize = 0;
	fwrite(&luaSize, 1, sizeof(uint32_t), fo); //TODO implement saving of user data and control data
	fwrite(&luaSize, 1, sizeof(uint32_t), fo);
	fwrite(&luaSize, 1, sizeof(uint32_t), fo);

	if (lScrpt.size()) {
		if (this == currentProject)
			lScrpt[curScript].str.assign(window->luaBufProject->text());

		for (unsigned i = 0; i < lScrpt.size(); ++i) {
			saveStrifNot(fo, lScrpt[i].name.c_str(), (std::to_string(i)).c_str());
			saveStrifNot(fo, lScrpt[i].str.c_str(), "");
		}
	}

	return true;
}
bool saveProject(uint32_t id, const char*fname) {
	//Start by creating a save file dialog
	FILE * fo = fopen(fname, "wb");
	projects->at(id).saveProjectFile(fo, true);
	fclose(fo);
	return true;
}

bool saveAllProjects(void) {
	/*!The format is the same except it begins with
	char R
	char G
	uint32_t amount of projects stored
	uint32_t version
	Before each project header is
	int32_t share[shareAmtPj] For version 4 the value of shareAmtPj is 4 in version 5 the value is 5
	(format described in saveProject is repeated n amount of times let n = amount of projects stored) The only difference is version is not stored
	*/
	if (!load_file_generic("Save projects group as...", true))
		return true;

	FILE * fo = fopen(the_file.c_str(), "wb");
	fputc('R', fo);
	fputc('G', fo);
	const uint32_t projectSize = projects->size();
	fwrite(&projectSize, 1, sizeof(uint32_t), fo);
	uint32_t version = currentProjectVersionNUM;
	fwrite(&version, sizeof(uint32_t), 1, fo);

	for (uint32_t s = 0; s < projects->size(); ++s) {
		Project&prj = projects->at(s);
		fwrite(prj.share, shareAmtPj, sizeof(uint32_t), fo);
		prj.saveProjectFile(fo, false, false);
	}

	fclose(fo);
	return true;
}

static void invaildGroup(void) {
	fl_alert("This is not a valid project group");
}

static void readShare(unsigned amt, FILE*fi, unsigned x) {
	fread(projects->at(x).share, amt, sizeof(uint32_t), fi);

	if (amt < shareAmtPj)
		std::fill(&projects->at(x).share[amt - 1], &projects->at(x).share[shareAmtPj], -1);
}

bool loadAllProjects(const char*fname) {
	FILE * fi = fopen(fname, "rb");

	if (fgetc(fi) != 'R') {
		invaildGroup();
		fclose(fi);
		return false;
	}

	char sc;

	if ((sc = fgetc(fi)) != 'G') {
		if (sc == 'P')
			fl_alert("This appears to be a project file; not a project group. If that is the case use the correct menu item.");
		else
			invaildGroup();

		fclose(fi);
		return false;
	}

	uint32_t PC;
	fread(&PC, 1, sizeof(uint32_t), fi);

	resizeProjects(PC);

	changeProjectAmt();

	uint32_t version;
	fread(&version, 1, sizeof(uint32_t), fi);
	printf("Group is version %d\n", version);

	if (version > currentProjectVersionNUM) {
		fl_alert("The latest project version Retro Graphics Toolkit supports is %d but you are opening %d", currentProjectVersionNUM, version);
		fclose(fi);
		return false;
	}

	for (unsigned x = 0; x < projects->size(); ++x) {
		if (version < 4)
			readShare(3, fi, x);
		else if (version == 4)
			readShare(4, fi, x);
		else if (version < 8)
			readShare(5, fi, x);
		else
			readShare(shareAmtPj, fi, x);

		if (!(projects->at(x).loadProjectFile(fi, false, version)))
			return false;
	}

	for (unsigned x = 0; x < projects->size(); ++x) {
		Project&prj = projects->at(x);

		if (prj.share[0] >= 0)
			shareProject(x, prj.share[0], pjHavePal, true);

		if (prj.share[1] >= 0)
			shareProject(x, prj.share[1], pjHaveTiles, true);

		if (prj.share[2] >= 0)
			shareProject(x, prj.share[2], pjHaveMap, true);

		if (version >= 3) {
			if (prj.share[3] >= 0)
				shareProject(x, prj.share[3], pjHaveChunks, true);

			if (version >= 5) {
				if (prj.share[4] >= 0)
					shareProject(x, prj.share[4], pjHaveSprites, true);
			}
		}
	}

	fclose(fi);
	return true;
}
