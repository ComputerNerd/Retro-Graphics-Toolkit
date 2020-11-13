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

#include "macros.h"
#include "gui.h"
#include "color_compare.h"
#include "color_convert.h"
#include "system.h"
#include "callback_project.h"
#include "lua.hpp"
#include "CIE.h"
#include "classpalettebar.h"
#include "undo.h"
#include "class_global.h"
#include "palette.h"
#include "runlua.h"
#include "luaconfig.h"
#include "callbacksprites.h"
#include "errorMsg.h"
static const char* GPLv3 = "This program is free software: you can redistribute it and/or modify\n"
                           "it under the terms of the GNU General Public License as published by\n"
                           "the Free Software Foundation, either version 3 of the License, or\n"
                           "(at your option) any later version.\n\n"
                           "This program is distributed in the hope that it will be useful,\n"
                           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
                           "GNU General Public License for more details.\n\n"
                           "You should have received a copy of the GNU General Public License\n"
                           "along with this program. If not, see <http://www.gnu.org/licenses/>.\n";
void setSubditherSetting(Fl_Widget*w, void*) {
	Fl_Slider*s = (Fl_Slider*)w;
	currentProject->settings &= ~(subsettingsDitherMask << subsettingsDitherShift);
	currentProject->settings |= (((uint32_t)s->value() - 1)&subsettingsDitherMask) << subsettingsDitherShift;
}
void redrawOnlyCB(Fl_Widget*, void*) {
	window->redraw();
}
void showAbout(Fl_Widget*, void*) {
	fl_alert("Retro Graphics Toolkit is written by sega16/nintendo8/sonic master or whatever username you know me as\nhttps://github.com/ComputerNerd/Retro-Graphics-Toolkit\nThis program was built on %s %s\n\n%s\nUses Lua version: " LUA_RELEASE "\n" LUA_COPYRIGHT "\n" LUA_AUTHORS, __DATE__, __TIME__, GPLv3);
}
Fl_Menu_Item subSysNES[] = {
	{"1x1 tile palette", 0, setNesTile, (void*)NES1x1},
	{"2x2 tile palette", 0, setNesTile, (void*)NES2x2},
	{0}
};
Fl_Menu_Item subSysGenesis[] = {
	{"Normal", 0, setSegaPalType, (void*)0},
	{"Shadow", 0, setSegaPalType, (void*)sgSon},
	{"Highlight", 0, setSegaPalType, (void*)sgSHmask},
	{0}
};
void setGameSysTMS9918(Project*prj) {
	enum TMS9918SubSys subSysnew = prj->getTMS9918subSys();

	switch (subSysnew) {
		case MODE_0:
		case MODE_1:
		case MODE_2:
			prj->setBitdepthSys(1);
			break;

		case MODE_3:
			prj->setBitdepthSys(4);
			break;
	}

	switch (subSysnew) {
		case MODE_0:
			prj->changeTileDim(6, 8);
			break;

		case MODE_1:
		case MODE_2:
			prj->changeTileDim(8, 8);
			break;

		case MODE_3:
			prj->changeTileDim(2, 2);
			break;
	}
}
static void TMS9918SetSubSysCB(Fl_Widget*, void*sys) {
	enum TMS9918SubSys subSysnew = (enum TMS9918SubSys)(uintptr_t)sys;
	currentProject->setTMS9918subSys(subSysnew);
	setGameSysTMS9918(currentProject);
}
Fl_Menu_Item subSysTMS9918[] = {
	{"Mode 0 (Text)", 0, TMS9918SetSubSysCB, (void*)MODE_0},
	{"Mode 1 (Graphics I)", 0, TMS9918SetSubSysCB, (void*)MODE_1},
	{"Mode 2 (Graphics II)", 0, TMS9918SetSubSysCB, (void*)MODE_2},
	{"Mode 3 (Multicolor)", 0, TMS9918SetSubSysCB, (void*)MODE_3},
	{0}
};
static void palCopyConvert(unsigned cols) {
	uint8_t*rgbTmp = (uint8_t*)alloca(cols * 3);
	uint8_t*palTypeTmp = (uint8_t*)alloca(cols);
	memcpy(palTypeTmp, currentProject->pal->palType, cols);
	memcpy(rgbTmp, currentProject->pal->rgbPal, cols * 3);
	palBar.setSys(false);
	uint8_t*nPtr = currentProject->pal->rgbPal;

	for (unsigned i = 0; i < cols; ++i, nPtr += 3)
		currentProject->pal->rgbToEntry(nPtr[0], nPtr[1], nPtr[2], i);

	memcpy(currentProject->pal->palType, palTypeTmp, cols);
}
void setGameSystem(Project*prjPtr, uint32_t prjIdx, gameSystemEnum sel) {
	const gameSystemEnum gold = prjPtr->gameSystem;

	prjPtr->gameSystem = sel;


	if (sel == NES) {
		updateNesTab(0, false);
		updateNesTab(0, true);
	}

	unsigned msprt = curSpritemeta;
	uint32_t sold = prjPtr->subSystem;
	unsigned bd = prjPtr->getBitdepthSys();
	unsigned bdold = bd;
	tiles*tilesOld = nullptr;

	if (prjPtr->isUniqueData(pjHaveTiles))
		tilesOld = new tiles(*prjPtr->tileC, prjPtr);

	if (prjPtr->isUniqueData(pjHavePal)) {
		palette * newPal = new palette(prjPtr);
		newPal->import(*prjPtr->pal);

		delete prjPtr->pal;
		prjPtr->pal = newPal;
		for (unsigned i = 0; i < projects->size(); ++i) {
			if (i == curProjectID)
				continue;

			Project*prj = &projects->at(i);
			int32_t *shr = prj->share;
			if (shr[pal_edit] == prjIdx) {
				prj->pal = newPal;
			}
		}

		palBar.setSys(true, true);
	}

	switch (sel) {
		case segaGenesis:
			bd = 4;
			prjPtr->gameSystem = segaGenesis;
			prjPtr->subSystem = 0;
			prjPtr->setBitdepthSys(bd);

			if (prjPtr->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(4);
					window->spritesize[1]->maximum(4);
				}

			}

			if (window) {
				window->subSysC->copy(subSysGenesis);
				window->subSysC->value((prjPtr->subSystem & sgSHmask) >> sgSHshift);
			}

			break;

		case NES:
			bd = 2;
			prjPtr->gameSystem = NES;
			prjPtr->subSystem = 0;
			prjPtr->setBitdepthSys(bd);
			prjPtr->subSystem |= NES2x2;

			if (prjPtr->isUniqueData(pjHaveMap)) {
				//on the NES tilemaps need to be a multiple of 2
				if (((prjPtr->tms->maps[prjPtr->curPlane].mapSizeW) & 1) && ((prjPtr->tms->maps[prjPtr->curPlane].mapSizeHA) & 1))
					prjPtr->tms->maps[prjPtr->curPlane].resize_tile_map(prjPtr->tms->maps[prjPtr->curPlane].mapSizeW + 1, prjPtr->tms->maps[prjPtr->curPlane].mapSizeHA + 1);
				else if ((prjPtr->tms->maps[prjPtr->curPlane].mapSizeW) & 1)
					prjPtr->tms->maps[prjPtr->curPlane].resize_tile_map(prjPtr->tms->maps[prjPtr->curPlane].mapSizeW + 1, prjPtr->tms->maps[prjPtr->curPlane].mapSizeHA);
				else if ((prjPtr->tms->maps[prjPtr->curPlane].mapSizeHA) & 1)
					prjPtr->tms->maps[prjPtr->curPlane].resize_tile_map(prjPtr->tms->maps[prjPtr->curPlane].mapSizeW, prjPtr->tms->maps[prjPtr->curPlane].mapSizeHA + 1);
			}

			if (prjPtr->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(1);
					window->spritesize[1]->maximum(2);
				}

			}

			if (window) {
				window->subSysC->copy(subSysNES);
				window->subSysC->value(prjPtr->subSystem & NES2x2);
			}

			break;

		case masterSystem:
		case gameGear:
			prjPtr->gameSystem = sel;
			prjPtr->subSystem = 0;
			bd = 4;
			prjPtr->setBitdepthSys(bd);

			if (prjPtr->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(2);
					window->spritesize[1]->maximum(2);
				}

			}

			break;

		case TMS9918:
			prjPtr->gameSystem = sel;
			prjPtr->subSystem = 0;
			prjPtr->setTMS9918subSys(MODE_2);

			bd = 1;
			prjPtr->setBitdepthSys(bd);

			if (prjPtr->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(2);
					window->spritesize[1]->maximum(2);
				}

			}

			if (window) {
				window->subSysC->copy(subSysTMS9918);
				window->subSysC->value(prjPtr->getTMS9918subSys());
			}

			break;

		case frameBufferPal:
		{	prjPtr->gameSystem = frameBufferPal;
			prjPtr->subSystem = 0;

			if (bd > 8)
				bd = 8;

			prjPtr->setBitdepthSys(bd);
		}
		break;

		default:
			show_default_error
			break;
	}

	if (prjPtr->tileC && prjPtr->isUniqueData(pjHaveTiles))
		prjPtr->tileC->changeDim(8, 8, bd);

	if (prjPtr->isUniqueData(pjHaveSprites)) {
		int spRow = prjPtr->fixedSpirtePalRow();

		if (spRow >= 0) {
			prjPtr->ms->sps[msprt].allToPalRow(spRow);
			palBar.changeRow(spRow, 3);
		}

		prjPtr->ms->sps[msprt].enforceMax();

	}

	if (prjPtr->containsData(pjHaveSprites)) {
		if (window && (prjIdx == curProjectID)) {
			window->updateSpriteSliders();
		}
	}

	if (prjPtr->isUniqueData(pjHaveTiles)) {
		if ((!((sel == masterSystem || sel == gameGear) && (gold == NES) && prjPtr->isUniqueData(pjHaveMap))) && (bd == bdold))
			goto freeIt;

		{	gameSystemEnum gnew = prjPtr->gameSystem;
			uint32_t snew = prjPtr->subSystem;

			for (unsigned i = 0; i < tilesOld->amt; ++i) {
				unsigned usedRow = 0;

				if ((sel == masterSystem || sel == gameGear) && (gold == NES) && prjPtr->isUniqueData(pjHaveMap)) {
					int x, y;

					for (unsigned t = 0; t < prjPtr->tms->maps.size(); ++t) {
						prjPtr->tms->maps[t].findFirst(x, y, i);

						if (x >= 0 && y >= 0) {
							usedRow = prjPtr->tms->maps[t].getPalRow(x, y);
							break;
						}
					}
				}

				for (unsigned y = 0; y < std::min(prjPtr->tileC->height(), tilesOld->height()); ++y) {
					for (unsigned x = 0; x < std::min(prjPtr->tileC->width(), tilesOld->width()); ++x) {
						prjPtr->gameSystem = gold;
						prjPtr->subSystem = sold;
						uint32_t px = tilesOld->getPixel(i, x, y);

						if ((sel == masterSystem || sel == gameGear) && (gold == NES) && prjPtr->isUniqueData(pjHaveMap))
							px += usedRow * 4;

						else if (bd != bdold) {
							if (bdold > bd)
								px >>= bdold - bd;
						}

						prjPtr->gameSystem = gnew;
						prjPtr->subSystem = snew;
						prjPtr->tileC->setPixel(i, x, y, px);
					}
				}
			}
		}
freeIt:
		delete tilesOld;
	}

	if (prjPtr->isUniqueData(pjHaveMap)) {
		if ((sel == masterSystem || sel == gameGear) && (gold == NES)) {
			for (unsigned i = 0; i < prjPtr->tms->maps.size(); ++i) {
				for (unsigned y = 0; y < prjPtr->tms->maps[i].mapSizeHA; ++y) {
					for (unsigned x = 0; x < prjPtr->tms->maps[i].mapSizeW; ++x)
						prjPtr->tms->maps[i].set_pal_row(x, y, 0);
				}
			}
		} else {
			for (unsigned i = 0; i < prjPtr->tms->maps.size(); ++i) {
				unsigned maxPal = 0;

				for (unsigned y = 0; y < prjPtr->tms->maps[i].mapSizeHA; ++y) {
					for (unsigned x = 0; x < prjPtr->tms->maps[i].mapSizeW; ++x) {
						unsigned tstPal = prjPtr->tms->maps[i].getPalRow(x, y);

						if (tstPal > maxPal)
							maxPal = tstPal;
					}
				}

				if (maxPal >= prjPtr->pal->rowCntPal) {
					unsigned divBy = (maxPal + 1 + (prjPtr->pal->rowCntPal / 2)) / prjPtr->pal->rowCntPal;

					for (unsigned y = 0; y < prjPtr->tms->maps[i].mapSizeHA; ++y) {
						for (unsigned x = 0; x < prjPtr->tms->maps[i].mapSizeW; ++x)
							prjPtr->tms->maps[i].set_pal_row(x, y, prjPtr->tms->maps[i].getPalRow(x, y) / divBy);
					}
				}
			}
		}
	}

}
void set_game_system(Fl_Widget*, void* selection) {
	gameSystemEnum sel = (gameSystemEnum)(intptr_t)selection;
	const gameSystemEnum gold = currentProject->gameSystem;

	if (unlikely(sel == currentProject->gameSystem)) {
		fl_alert("You are already in that mode");
		return;
	}

	pushProject();
	lua_getglobal(Lconf, "switchSystemBefore");
	lua_pushinteger(Lconf, gold);
	lua_pushinteger(Lconf, sel);
	runLuaFunc(Lconf, 2, 0);

	setGameSystem(currentProject, curProjectID, sel);

	for (unsigned i = 0; i < projects->size(); ++i) {
		if (i == curProjectID)
			continue;

		Project*prj = &projects->at(i);
		int32_t *shr = prj->share;
		for (unsigned j = 0; j < shareAmtPj; ++j) {
			if (shr[j] == curProjectID) {
				if (prj->gameSystem != sel) {
					setGameSystem(prj, i, sel);
				}
			}
		}
	}

	lua_getglobal(Lconf, "switchSystemAfter");
	lua_pushinteger(Lconf, gold);
	lua_pushinteger(Lconf, sel);
	runLuaFunc(Lconf, 2, 0);

	if (window)
		window->redraw();
}
void trueColTileToggle(Fl_Widget*, void*) {
	showTrueColor ^= true;
	window->damage(FL_DAMAGE_USER1);
}
void toggleRowSolo(Fl_Widget*, void*) {
	rowSolo ^= true;
	window->redraw();
}
