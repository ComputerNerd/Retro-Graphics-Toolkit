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
#include "runlua.h"
#include "filemisc.h"
#include "errorMsg.h"


extern editor *window;
std::vector<struct Project> projects;
struct Project * currentProject;
Fl_Slider* curPrj;
static const char * defaultName = "Add a description here.";
uint32_t curProjectID;
static const char*const maskNames[] = {"palette", "tiles", "tilemap", "chunks", "sprites", "level", "{Undefined}"};
void changeTileDim(unsigned w, unsigned h, struct Project*p) {
	if (p->containsData(pjHaveTiles)) {
		unsigned sw = p->tileC->width(), sh = p->tileC->height();
		p->tileC->changeDim(w, h, p->getBitdepthSys());

		if (sw > w && sh > h && p->containsData(pjHaveMap)) {
			for (size_t i = 0; i < p->tms->maps.size(); ++i) {
				tileMap*tm = new tileMap(p->tms->maps[i], p);
				p->tms->maps[i].resize_tile_map(p->tms->maps[i].mapSizeW * sw / w, p->tms->maps[i].mapSizeH * sh / h);
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

	for (uint_fast32_t i = 0; i < projects.size(); ++i) {
		if (projects[i].containsData(pjHaveTiles)) {
			Cold += projects[i].tileC->tDat.capacity();
			Cold += projects[i].tileC->truetDat.capacity();
			projects[i].tileC->tDat.shrink_to_fit();
			projects[i].tileC->truetDat.shrink_to_fit();
			Cnew += projects[i].tileC->tDat.capacity();
			Cnew += projects[i].tileC->truetDat.capacity();
		}

		if (projects[i].containsData(pjHaveChunks)) {
			Cold += projects[i].Chunk->chunks.capacity();
			projects[i].Chunk->chunks.shrink_to_fit();
			Cnew += projects[i].Chunk->chunks.capacity();
		}

		if (projects[i].containsData(pjHaveSprites)) {
			for (uint32_t j = 0; j < projects[i].ms->sps.size(); ++j) {
				for (uint32_t n = 0; n < projects[i].ms->sps[j].amt; ++n) {
					Cold += projects[i].ms->sps[j].groups[n].list.capacity();
					projects[i].ms->sps[j].groups[n].list.shrink_to_fit();
					Cnew += projects[i].ms->sps[j].groups[n].list.capacity();
				}

				Cold += projects[i].ms->sps[j].groups.capacity();
				projects[i].ms->sps[j].groups.shrink_to_fit();
				Cnew += projects[i].ms->sps[j].groups.capacity();
			}
		}
	}

	printf("Old capacity: %d New capacity: %d saved %d bytes\n", Cold, Cnew, Cold - Cnew);
}

Project::Project() {
	Name.assign(defaultName);
	gameSystem = segaGenesis;
	subSystem = 3;
	settings = (15 << subsettingsDitherShift) | (aWeighted << nearestColorShift);
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
	projects[at].tileC = new tiles(&projects[at]);
	projects[at].tms = new tilemaps(&projects[at]);
	projects[at].Chunk = new ChunkClass(&projects[at]);
	projects[at].ms = new metasprites(&projects[at]);
	projects[at].pal = new palette(&projects[at]);
	projects[at].lvl = new level(&projects[at]);
	projects[at].useMask = pjDefaultMask;
}

void initProject(void) {
	resizeProjects(1);
	initNewProject(0);
}

void setHaveProject(uint32_t id, uint32_t mask, bool set) {
	/*This function will allocate/free data if and only if it is not being shared
	if have is already enabled no new data will be allocated
	if have was enabled data will be freeded*/
	if ((mask & pjHavePal) && (projects[id].share[0] < 0)) {
		if (set) {
			if (!(projects[id].useMask & pjHavePal)) {
				projects[id].pal = new palette(&projects[id]);
				projects[id].useMask |= pjHavePal;
				palBar.setSys();

				if (projects[id].gameSystem == NES)
					projects[id].pal->updateEmphasis();
			}
		} else {
			if (projects[id].useMask & pjHavePal) {
				delete projects[id].pal;
				projects[id].useMask &= ~pjHavePal;
				projects[id].pal = nullptr;
			}
		}
	}

	if ((mask & pjHaveTiles) && (projects[id].share[1] < 0)) {
		if (set) {
			if (!(projects[id].useMask & pjHaveTiles)) {
				projects[id].tileC = new tiles(&projects[id], &projects[id]);
				projects[id].useMask |= pjHaveTiles;
			}
		} else {
			if (projects[id].useMask & pjHaveTiles) {
				delete projects[id].tileC;
				projects[id].useMask &= ~pjHaveTiles;
				projects[id].tileC = nullptr;
			}
		}
	}

	if ((mask & pjHaveMap) && (projects[id].share[2] < 0)) {
		if (set) {
			if (!(projects[id].useMask & pjHaveMap)) {
				projects[id].tms = new tilemaps(&projects[id]);
				projects[id].useMask |= pjHaveMap;
			}
		} else {
			if (projects[id].useMask & pjHaveMap) {
				delete projects[id].tms;
				projects[id].useMask &= ~pjHaveMap;
				projects[id].tms = nullptr;
			}
		}
	}

	if ((mask & pjHaveChunks) && (projects[id].share[3] < 0)) {
		if (set) {
			if (!(projects[id].useMask & pjHaveChunks)) {
				projects[id].Chunk = new ChunkClass(&projects[id]);
				projects[id].useMask |= pjHaveChunks;
			}
		} else {
			if (projects[id].useMask & pjHaveChunks) {
				delete projects[id].Chunk;
				projects[id].useMask &= ~pjHaveChunks;
				projects[id].Chunk = nullptr;
			}
		}
	}

	if ((mask & pjHaveSprites) && (projects[id].share[4] < 0)) {
		if (set) {
			if (!(projects[id].useMask & pjHaveSprites)) {
				if (window)
					window->spriteglobaltxt->show();

				projects[id].ms = new metasprites(&projects[id]);
				projects[id].useMask |= pjHaveSprites;
			}
		} else {
			if (projects[id].useMask & pjHaveSprites) {
				if (window)
					window->spriteglobaltxt->hide();

				delete projects[id].ms;
				projects[id].useMask &= ~pjHaveSprites;
				projects[id].ms = nullptr;
			}
		}
	}

	if ((mask & pjHaveLevel) && (projects[id].share[5] < 0)) {
		if (set) {
			if (!(projects[id].useMask & pjHaveLevel)) {
				projects[id].useMask |= pjHaveLevel;
				projects[id].lvl = new level(&projects[id]);
			}
		} else {
			if (projects[id].useMask & pjHaveLevel) {
				projects[id].useMask &= ~pjHaveLevel;
				delete projects[id].lvl;
				projects[id].lvl = nullptr;
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

	if (enable) {
		if (what & pjHavePal) {
			if ((projects[share].share[0] < 0) && (projects[share].useMask & pjHavePal))
				delete projects[share].pal;

			projects[share].share[0] = with;
			projects[share].pal = projects[with].pal;
		}

		if (what & pjHaveTiles) {
			if ((projects[share].share[1] < 0) && (projects[share].useMask & pjHaveTiles))
				delete projects[share].tileC;

			projects[share].share[1] = with;
			projects[share].tileC = projects[with].tileC;
		}

		if (what & pjHaveMap) {
			if ((projects[share].share[2] < 0) && (projects[share].useMask & pjHaveMap))
				delete projects[share].tms;

			projects[share].share[2] = with;
			projects[share].tms = projects[with].tms;
		}

		if (what & pjHaveChunks) {
			if ((projects[share].share[chunkEditor] < 0) && (projects[share].useMask & pjHaveChunks))
				delete projects[share].Chunk;

			projects[share].share[chunkEditor] = with;
			projects[share].Chunk = projects[with].Chunk;
		}

		if (what & pjHaveSprites) {
			if ((projects[share].share[spriteEditor] < 0) && (projects[share].useMask & pjHaveSprites))
				delete projects[share].ms;

			projects[share].share[spriteEditor] = with;
			projects[share].ms = projects[with].ms;
		}

		if (what & pjHaveLevel) {
			if ((projects[share].share[levelEditor] < 0) && (projects[share].useMask & pjHaveLevel))
				delete projects[share].lvl;

			projects[share].share[levelEditor] = with;
			projects[share].lvl = projects[with].lvl;
		}
	} else {
		if (what & pjHavePal) {
			if ((projects[share].share[pal_edit] >= 0) && (projects[share].useMask & pjHavePal))
				projects[share].pal = new palette(*projects[with].pal, &projects[share]);

			projects[share].share[pal_edit] = -1;
		}

		if (what & pjHaveTiles) {
			if ((projects[share].share[tile_edit] >= 0) && (projects[share].useMask & pjHaveTiles)) //Create a copy of the shared data
				projects[share].tileC = new tiles(*projects[with].tileC, &projects[share]);

			projects[share].share[tile_edit] = -1;
		}

		if (what & pjHaveMap) {
			if ((projects[share].share[tile_place] >= 0) && (projects[share].useMask & pjHaveMap))
				projects[share].tms = new tilemaps(*projects[with].tms, &projects[share]);

			projects[share].share[tile_place] = -1; //Even if we don't have the data sharing can still be disabled
		}

		if (what & pjHaveChunks) {
			if ((projects[share].share[chunkEditor] >= 0) && (projects[share].useMask & pjHaveChunks))
				projects[share].Chunk = new ChunkClass(*projects[with].Chunk, &projects[share]);

			projects[share].share[chunkEditor] = -1; //Even if we don't have the data sharing can still be disabled
		}

		if (what & pjHaveSprites) {
			if ((projects[share].share[spriteEditor] >= 0) && (projects[share].useMask & pjHaveSprites))
				projects[share].ms = new metasprites(*projects[with].ms, &projects[share]);

			projects[share].share[spriteEditor] = -1; //Even if we don't have the data sharing can still be disabled
		}

		if (what & pjHaveLevel) {
			if ((projects[share].share[levelEditor] >= 0) && (projects[share].useMask & pjHaveLevel))
				projects[share].lvl = new level(*projects[with].lvl, &projects[share]);

			projects[share].share[levelEditor] = -1; //Even if we don't have the data sharing can still be disabled
		}
	}
}

void prjChangePtr(unsigned id) {
	Project& project = projects[id];

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
	projects.resize(amt);

	for (size_t i = 0; i < projects.size(); ++i)
		prjChangePtr(i);

	if (curProjectID >= amt) {
		curProjectID = amt - 1;
		switchProjectSlider(curProjectID);
	}

	currentProject = &projects[curProjectID]; //The address may have changed
	return 0;
}

void changeProjectAmt(void) {
	if (!window)
		return;

	if (curProjectID >= projects.size())
		switchProjectSlider(projects.size() - 1, false);

	window->projectSelect->maximum(projects.size() - 1);

	for (int x = 0; x < shareAmtPj; ++x)
		window->shareWith[x]->maximum(projects.size() - 1);
}

bool appendProject() {
	if (resizeProjects(projects.size() + 1))
		return false;

	initNewProject(projects.size() - 1);
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
	if (projects.size() <= 1) {
		fl_alert("You must have at least one project.");
		return false;
	}

	projects.erase(projects.begin() + id);

	currentProject = &projects[curProjectID]; //The address may have changed

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
	curProjectID = id;
	currentProject = &projects[curProjectID];
	switchProject(curProjectID);
}
extern int curScript;
static void updateLuaScriptWindow(uint32_t id, bool load = false) {
	if (!window)
		return;

	if ((curScript >= 0) && (!load))
		currentProject->lScrpt[curScript].str.assign(window->luaBufProject->text());

	window->luaScriptSel->clear();
	size_t amt = projects[id].lScrpt.size();

	if (amt) {
		window->luaEditProject->show();

		for (unsigned i = 0; i < amt; ++i)
			window->luaScriptSel->add(projects[id].lScrpt[i].name.c_str(), 0, switchCurLuaScript, (void*)(intptr_t)i);

		window->luaScriptSel->value(0);
		switchCurLuaScript(nullptr, (void*)(intptr_t)0);
	} else {
		window->luaEditProject->hide();
		curScript = -1;
	}
}
void switchProject(uint32_t id, bool load) {
	if (window) {
		updateLuaScriptWindow(id, load);
		window->TxtBufProject->text(projects[id].Name.c_str());//Make editor displays new text
		window->gameSysSel->value(projects[id].gameSystem);
		window->ditherPower->value(1 + ((projects[id].settings >> subsettingsDitherShift)&subsettingsDitherMask));
		window->ditherAlgSel->value(projects[id].settings & subsettingsDitherMask);

		if ((projects[id].settings & subsettingsDitherMask))
			window->ditherPower->hide();
		else
			window->ditherPower->show();
	}

	if (projects[id].containsData(pjHavePal))
		projects[id].pal->setVars(projects[id].gameSystem);

	switch (projects[id].gameSystem) {
		case segaGenesis:
			if (window) {
				window->subSysC->copy(subSysGenesis);
				window->subSysC->value((projects[id].subSystem & sgSHmask) >> sgSHshift);
			}

			if (projects[id].containsData(pjHaveTiles))
				projects[id].tileC->tileSize = 32;

			if (projects[id].containsData(pjHavePal)) {
				palBar.setSys(true, true);
				set_palette_type();
			}

			break;

		case NES:
			if (window) {
				window->subSysC->copy(subSysNES);
				window->subSysC->value(projects[id].subSystem & 1);
			}

			if (projects[id].containsData(pjHaveTiles))
				projects[id].tileC->tileSize = 16;

			if (projects[id].containsData(pjHavePal)) {
				palBar.setSys();
				projects[id].pal->updateEmphasis();
			}

			break;

		case masterSystem:
		case gameGear:
			if (projects[id].containsData(pjHaveTiles))
				projects[id].tileC->tileSize = 32;

			if (projects[id].containsData(pjHavePal)) {
				palBar.setSys();
				projects[id].pal->paletteToRgb();
			}

			break;

		case TMS9918:
			setGameSysTMS9918(&projects[id]);

			if (window) {
				window->subSysC->copy(subSysTMS9918);
				window->subSysC->value(projects[id].getTMS9918subSys());
			}

			if (projects[id].containsData(pjHavePal))
				palBar.setSys();

			if (projects[id].containsData(pjHaveTiles))
				projects[id].tileC->tileSize = 8;

			break;

		default:
			show_default_error
	}

	//Make sure sliders have correct values
	if (projects[id].containsData(pjHaveMap)) {
		if (window) {
			updatePlaneTilemapMenu(id, window->planeSelect);
			window->updateMapWH(projects[id].tms->maps[projects[id].curPlane].mapSizeW, projects[id].tms->maps[projects[id].curPlane].mapSizeH);
			char tmp[16];
			snprintf(tmp, 16, "%u", projects[id].tms->maps[projects[id].curPlane].amt);
			window->map_amt->value(tmp);
		}
	}

	if (projects[id].containsData(pjHaveTiles) && window)
		updateTileSelectAmt(projects[id].tileC->amt);

	for (int x = 0; x < shareAmtPj; ++x) {
		if (window) {
			window->sharePrj[x]->value(projects[id].share[x] < 0 ? 0 : 1);
			window->havePrj[x]->value(projects[id].useMask >> x & 1);

			if (projects[id].share[x] < 0)
				window->havePrj[x]->show();
			else
				window->havePrj[x]->hide();

			if (projects[id].useMask >> x & 1) {
				if (window->tabsHidden[x]) {
					window->the_tabs->insert(*window->tabsMain[x], x);
					window->tabsHidden[x] = false;
				}
			} else {
				if (!window->tabsHidden[x]) {
					if (projects[id].share[x] < 0) {
						window->the_tabs->remove(window->tabsMain[x]);
						window->tabsHidden[x] = true;
					}
				}
			}
		}
	}

	if (projects[id].containsData(pjHaveMap) && window)
		window->BlocksCBtn->value(projects[id].tms->maps[projects[id].curPlane].isBlock ? 1 : 0);

	if (projects[id].containsData(pjHaveChunks) && window) {
		window->chunk_select->maximum(projects[id].Chunk->amt - 1);
		window->updateChunkSize(projects[id].Chunk->wi, projects[id].Chunk->hi);
	}

	if (projects[id].containsData(pjHaveMap))
		projects[id].tms->maps[projects[id].curPlane].toggleBlocks(projects[id].tms->maps[projects[id].curPlane].isBlock);

	if (projects[id].containsData(pjHaveChunks) && window)
		window->updateBlockTilesChunk(id);

	if (projects[id].containsData(pjHaveSprites) && window) {
		window->updateSpriteSliders(id);
		window->spriteglobaltxt->show();
		window->spriteglobaltxt->value(projects[id].ms->name.c_str());
	} else if (window)
		window->spriteglobaltxt->hide();

	lua_getglobal(Lconf, "switchProject");
	runLuaFunc(Lconf, 0, 0);

	if (window)
		window->redraw();
}
static bool loadProjectFile(uint32_t id, FILE * fi, bool loadVersion = true, uint32_t version = currentProjectVersionNUM) {
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

	struct Project& project = projects[id];

	// First clear out any of the old classes.
	project.deleteClasses();

	fileToStr(fi, project.Name, defaultName);

	if (loadVersion) {
		fread(&version, 1, sizeof(uint32_t), fi);
		printf("Read as version %d\n", version);
	}

	if (version > currentProjectVersionNUM) {
		fl_alert("The latest project version Retro Graphics Toolkit supports is %u but you are opening %u", currentProjectVersionNUM, version);
		fclose(fi);
		return false;
	}

	if (version)
		fread(&project.useMask, 1, sizeof(uint32_t), fi);
	else
		project.useMask = pjHavePal | pjHaveTiles | pjHaveMap;

	uint32_t gameTemp;
	fread(&gameTemp, 1, sizeof(uint32_t), fi);
	project.gameSystem = (gameSystemEnum)gameTemp;

	if (version >= 4) {
		fread(&project.subSystem, 1, sizeof(uint32_t), fi);

		if ((version < 6) && (project.gameSystem == segaGenesis))
			project.subSystem = 3; //Old projects were storing the wrong number for 4bit graphics even though that is what is stored

		if ((version == 4) && (project.gameSystem == NES)) {
			project.subSystem ^= 1; //Fix the fact that NES2x2 and NES1x1 were switched around in version 4
			project.subSystem |= 2; //Default to 2 bit
		}
	} else
		project.subSystem = 3;

	if (version >= 8) {
		fread(&project.settings, 1, sizeof(uint32_t), fi);
		fread(&project.luaSettings, 1, sizeof(uint32_t), fi);
	} else {
		project.settings = 15 << subsettingsDitherShift | (aWeighted << nearestColorShift);
		project.luaSettings = 0;
	}

	if (project.useMask & pjHavePal) {
		if (project.share[0] < 0) {
			project.pal = new palette(&project);

			project.pal->setVars(project.gameSystem);
			project.pal->read(fi, version >= 7);
		}
	}

	if (project.useMask & pjHaveTiles) {
		if (project.share[1] < 0) {
			project.tileC = new tiles(&project);

			if (project.gameSystem == TMS9918)
				setGameSysTMS9918(&project);
			else
				project.tileC->changeDim(8, 8, project.getBitdepthSys());

			fread(&project.tileC->amt, 1, sizeof(uint32_t), fi);

			if (version < 6)
				++project.tileC->amt;

			project.tileC->resizeAmt();
			decompressFromFile(project.tileC->tDat.data(), project.tileC->tileSize * (project.tileC->amt), fi);
			decompressFromFile(project.tileC->truetDat.data(), project.tileC->tcSize * (project.tileC->amt), fi);

			if (version >= 8) {
				size_t extSize = project.tileC->extAttrs.size();

				if (extSize > 0)
					decompressFromFile(project.tileC->extAttrs.data(), extSize, fi);
			}

			if (version <= 7 && (project.getTileType() != PLANAR_TILE))
				project.tileC->toPlanar(project.getTileType());
		}
	}

	if (project.useMask & pjHaveMap) {
		if (project.share[2] < 0) {
			project.tms = new tilemaps(&project);

			uint32_t readCnt;

			if (version >= 8)
				fread(&readCnt, 1, sizeof(uint32_t), fi);
			else
				readCnt = 1;

			project.curPlane = 0;
			project.tms->setPlaneCnt(readCnt);

			for (unsigned i = 0; i < readCnt; ++i) {
				if (version >= 8) {
					char firstC = fgetc(fi);

					if (firstC) {
						project.tms->maps[i].planeName.clear();

						do {
							project.tms->maps[i].planeName.push_back(firstC);
						} while ((firstC = fgetc(fi)));
					} else
						project.tms->assignNum(i);
				} else
					project.tms->assignNum(i);

				fread(&project.tms->maps[i].mapSizeW, 1, sizeof(uint32_t), fi);
				fread(&project.tms->maps[i].mapSizeH, 1, sizeof(uint32_t), fi);

				if (version >= 2) {
					uint8_t isBlockTemp;
					fread(&isBlockTemp, 1, sizeof(uint8_t), fi);
					project.tms->maps[i].isBlock = isBlockTemp ? true : false;

					if (isBlockTemp)
						fread(&project.tms->maps[i].amt, 1, sizeof(uint32_t), fi);
					else
						project.tms->maps[i].amt = 1;

					project.tms->maps[i].mapSizeHA = project.tms->maps[i].mapSizeH * project.tms->maps[i].amt;
				} else
					project.tms->maps[i].mapSizeHA = project.tms->maps[i].mapSizeH;

				if (version >= 8)
					fread(&project.tms->maps[i].offset, 1, sizeof(int32_t), fi);
				else
					project.tms->maps[i].offset = 0;

				project.tms->maps[i].tileMapDat = (uint8_t*)realloc(project.tms->maps[i].tileMapDat, 4 * project.tms->maps[i].mapSizeW * project.tms->maps[i].mapSizeHA);
				decompressFromFile(project.tms->maps[i].tileMapDat, 4 * project.tms->maps[i].mapSizeW * project.tms->maps[i].mapSizeHA, fi);
			}
		}
	}

	if (project.useMask & pjHaveChunks) {
		if (project.share[3] < 0) {
			if (version >= 3) {
				project.Chunk = new ChunkClass(&project);
				uint8_t useBlockTemp;
				fread(&useBlockTemp, 1, sizeof(uint8_t), fi);
				project.Chunk->useBlocks = useBlockTemp ? true : false;
				fread(&project.Chunk->wi, 1, sizeof(uint32_t), fi);
				fread(&project.Chunk->hi, 1, sizeof(uint32_t), fi);
				fread(&project.Chunk->amt, 1, sizeof(uint32_t), fi);

				if (version >= 8)
					fread(&project.Chunk->usePlane, 1, sizeof(uint32_t), fi);

				project.Chunk->resizeAmt();
				decompressFromFile(project.Chunk->chunks.data(), project.Chunk->wi * project.Chunk->hi * sizeof(struct ChunkAttrs)*project.Chunk->amt, fi);
			}
		}
	}

	if (project.useMask & pjHaveSprites) {
		if (project.share[4] < 0) {
			if (version >= 5) {
				project.ms = new metasprites(&project);
				project.ms->load(fi, version);
			}
		}
	}

	if (project.useMask & pjHaveLevel) {
		if (project.share[5] < 0) {
			if (version >= 8) {
				project.lvl = new level(&project);
				project.lvl->load(fi, version);
			}
		}
	}

	if (version >= 8) {
		uint32_t scriptAmt, tabsAmt, controlDat, userDat;
		fread(&scriptAmt, 1, sizeof(uint32_t), fi);
		fread(&tabsAmt, 1, sizeof(uint32_t), fi);
		fread(&controlDat, 1, sizeof(uint32_t), fi);
		fread(&userDat, 1, sizeof(uint32_t), fi);

		if (scriptAmt) {
			project.lScrpt.clear();
			project.lScrpt.resize(scriptAmt);

			for (unsigned i = 0; i < scriptAmt; ++i) {
				fileToStr(fi, project.lScrpt[i].name, std::to_string(i).c_str());
				fileToStr(fi, project.lScrpt[i].str, "");
			}
		}

		updateLuaScriptWindow(id, true);

		if (tabsAmt || userDat || controlDat)
			fl_alert("This version of Retro Graphics Toolkit does not fully support Lua user data please upgrade.");
	}

	return true;
}
bool loadProject(uint32_t id, const char*fname) {
	FILE * fi = fopen(fname, "rb");
	std::fill(projects[id].share, &projects[id].share[shareAmtPj], -1); //One file projects do not support sharing

	if (loadProjectFile(id, fi))
		fclose(fi);

	return true;
}
static bool saveProjectFile(uint32_t id, FILE * fo, bool saveShared, bool saveVersion = true) {
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
	saveStrifNot(fo, projects[id].Name.c_str(), defaultName);

	if (saveVersion) {
		uint32_t version = currentProjectVersionNUM;
		fwrite(&version, sizeof(uint32_t), 1, fo);
	}

	uint32_t haveTemp;

	if (saveShared) {
		haveTemp = projects[id].useMask;

		for (unsigned x = 0; x < shareAmtPj; ++x)
			haveTemp |= (projects[id].share[x] >= 0 ? 1 : 0) << x;
	} else
		haveTemp = projects[id].useMask;

	fwrite(&haveTemp, sizeof(uint32_t), 1, fo);
	uint32_t gameTemp = (uint32_t)projects[id].gameSystem;
	fwrite(&gameTemp, 1, sizeof(uint32_t), fo);
	fwrite(&projects[id].subSystem, sizeof(uint32_t), 1, fo);
	fwrite(&projects[id].settings, sizeof(uint32_t), 1, fo);
	fwrite(&projects[id].luaSettings, sizeof(uint32_t), 1, fo);

	if (haveTemp & pjHavePal) {
		if (saveShared || (projects[id].share[0] < 0))
			projects[id].pal->write(fo);
	}

	if (haveTemp & pjHaveTiles) {
		if (saveShared || (projects[id].share[1] < 0)) {
			fwrite(&projects[id].tileC->amt, 1, sizeof(uint32_t), fo);
			compressToFile(projects[id].tileC->tDat.data(), projects[id].tileC->tileSize * (projects[id].tileC->amt), fo);
			compressToFile(projects[id].tileC->truetDat.data(), projects[id].tileC->tcSize * (projects[id].tileC->amt), fo);
			{
				size_t extSize = projects[id].tileC->extAttrs.size();

				if (extSize)
					compressToFile(projects[id].tileC->extAttrs.data(), extSize, fo);
			}
		}
	}

	if (haveTemp & pjHaveMap) {
		if (saveShared || (projects[id].share[2] < 0)) {
			uint32_t cnt = projects[id].tms->maps.size();
			fwrite(&cnt, 1, 4, fo);

			for (unsigned i = 0; i < projects[id].tms->maps.size(); ++i) {
				//Write the name or if default just write 0
				char tmp[16];
				snprintf(tmp, 16, "%u", i);

				if (strcmp(projects[id].tms->maps[i].planeName.c_str(), tmp)) {
					const char*st = projects[id].tms->maps[i].planeName.c_str();

					do {
						fputc(*st, fo);
					} while (*st++);
				} else
					fputc(0, fo);

				fwrite(&projects[id].tms->maps[i].mapSizeW, 1, sizeof(uint32_t), fo);
				fwrite(&projects[id].tms->maps[i].mapSizeH, 1, sizeof(uint32_t), fo);
				uint8_t isBlockTemp = projects[id].tms->maps[i].isBlock ? 1 : 0;
				fwrite(&isBlockTemp, 1, sizeof(uint8_t), fo);

				if (isBlockTemp)
					fwrite(&projects[id].tms->maps[i].amt, 1, sizeof(uint32_t), fo);

				fwrite(&projects[id].tms->maps[i].offset, 1, sizeof(int32_t), fo);
				compressToFile(projects[id].tms->maps[i].tileMapDat, 4 * projects[id].tms->maps[i].mapSizeW * projects[id].tms->maps[i].mapSizeHA, fo);
			}
		}
	}

	if (haveTemp & pjHaveChunks) {
		if (saveShared || (projects[id].share[3] < 0)) {
			uint8_t useBlockTemp = projects[id].Chunk->useBlocks ? 1 : 0;
			fwrite(&useBlockTemp, 1, sizeof(uint8_t), fo);
			fwrite(&projects[id].Chunk->wi, 1, sizeof(uint32_t), fo);
			fwrite(&projects[id].Chunk->hi, 1, sizeof(uint32_t), fo);
			fwrite(&projects[id].Chunk->amt, 1, sizeof(uint32_t), fo);
			fwrite(&projects[id].Chunk->usePlane, 1, sizeof(uint32_t), fo);
			compressToFile(projects[id].Chunk->chunks.data(), projects[id].Chunk->wi * projects[id].Chunk->hi * sizeof(struct ChunkAttrs)*projects[id].Chunk->amt, fo);
		}
	}

	if (haveTemp & pjHaveSprites) {
		if (saveShared || (projects[id].share[4] < 0))
			projects[id].ms->save(fo);
	}

	if (haveTemp & pjHaveLevel) {
		if (saveShared || (projects[id].share[3] < 0))
			projects[id].lvl->save(fo);
	}

	uint32_t luaSize = projects[id].lScrpt.size();
	fwrite(&luaSize, 1, sizeof(uint32_t), fo);
	luaSize = 0;
	fwrite(&luaSize, 1, sizeof(uint32_t), fo); //TODO implement saving of user data and control data
	fwrite(&luaSize, 1, sizeof(uint32_t), fo);
	fwrite(&luaSize, 1, sizeof(uint32_t), fo);

	if (projects[id].lScrpt.size()) {
		currentProject->lScrpt[curScript].str.assign(window->luaBufProject->text());

		for (unsigned i = 0; i < projects[id].lScrpt.size(); ++i) {
			saveStrifNot(fo, projects[id].lScrpt[i].name.c_str(), (std::to_string(i)).c_str());
			saveStrifNot(fo, projects[id].lScrpt[i].str.c_str(), "");
		}
	}

	return true;
}
bool saveProject(uint32_t id, const char*fname) {
	//Start by creating a save file dialog
	FILE * fo = fopen(fname, "wb");
	saveProjectFile(id, fo, true);
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
	const uint32_t projectSize = projects.size();
	fwrite(&projectSize, 1, sizeof(uint32_t), fo);
	uint32_t version = currentProjectVersionNUM;
	fwrite(&version, sizeof(uint32_t), 1, fo);

	for (uint32_t s = 0; s < projects.size(); ++s) {
		fwrite(projects[s].share, shareAmtPj, sizeof(uint32_t), fo);
		saveProjectFile(s, fo, false, false);
	}

	fclose(fo);
	return true;
}
static void invaildGroup(void) {
	fl_alert("This is not a valid project group");
}
static void readShare(unsigned amt, FILE*fi, unsigned x) {
	fread(projects[x].share, amt, sizeof(uint32_t), fi);

	if (amt < shareAmtPj)
		std::fill(&projects[x].share[amt - 1], &projects[x].share[shareAmtPj], -1);
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

	for (unsigned x = 0; x < projects.size(); ++x) {
		if (version < 4)
			readShare(3, fi, x);
		else if (version == 4)
			readShare(4, fi, x);
		else if (version < 8)
			readShare(5, fi, x);
		else
			readShare(shareAmtPj, fi, x);

		if (!(loadProjectFile(x, fi, false, version)))
			return false;
	}

	for (unsigned x = 0; x < projects.size(); ++x) {
		if (projects[x].share[0] >= 0)
			shareProject(x, projects[x].share[0], pjHavePal, true);

		if (projects[x].share[1] >= 0)
			shareProject(x, projects[x].share[1], pjHaveTiles, true);

		if (projects[x].share[2] >= 0)
			shareProject(x, projects[x].share[2], pjHaveMap, true);

		if (version >= 3) {
			if (projects[x].share[3] >= 0)
				shareProject(x, projects[x].share[3], pjHaveChunks, true);

			if (version >= 5) {
				if (projects[x].share[4] >= 0)
					shareProject(x, projects[x].share[4], pjHaveSprites, true);
			}
		}
	}

	fclose(fi);
	return true;
}
