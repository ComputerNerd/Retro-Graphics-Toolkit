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
#include "lua.h"
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
			changeTileDim(6, 8, prj);
			break;

		case MODE_1:
		case MODE_2:
			changeTileDim(8, 8, prj);
			break;

		case MODE_3:
			changeTileDim(2, 2, prj);
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

	if (sel == NES) {
		updateNesTab(0, false);
		updateNesTab(0, true);
	}

	unsigned msprt = curSpritemeta;
	uint32_t sold = currentProject->subSystem;
	unsigned bd = currentProject->getBitdepthSys();
	unsigned bdold = bd;
	tiles*tilesOld = 0;

	if (currentProject->containsData(pjHaveTiles))
		tilesOld = new tiles(*currentProject->tileC, currentProject);

	if (currentProject->containsData(pjHavePal)) {
		palBar.sysCache = UNKNOWN_SYSTEM;

		switch (sel) {
			case segaGenesis:
				if (currentProject->gameSystem == NES || currentProject->gameSystem == masterSystem || currentProject->gameSystem == gameGear) {
					uint8_t rgbTmp[32 * 3];
					uint8_t palTypeTmp[32];
					memcpy(rgbTmp, currentProject->pal->rgbPal, 32 * 3);
					memcpy(palTypeTmp, currentProject->pal->palType, 32);
					currentProject->gameSystem = sel;
					palBar.setSys(false);

					if (gold == NES) {
						for (unsigned i = 0; i < 4; ++i) {
							memcpy(currentProject->pal->rgbPal + (i * 16 * 3), rgbTmp + (i * 4 * 3), 4 * 3);
							memcpy(currentProject->pal->rgbPal + (i * 16 * 3) + (4 * 3), rgbTmp + (i * 4 * 3) + (4 * 3 * 4), 4 * 3);
							memset(currentProject->pal->rgbPal + (i * 16 * 3) + (8 * 3), 0, (16 - 8) * 3);
							memcpy(currentProject->pal->palType + (i * 16), palTypeTmp + (i * 4), 4);
							memcpy(currentProject->pal->palType + (i * 16) + 4, palTypeTmp + (i * 4) + (4 * 4), 4);
							memset(currentProject->pal->palType + (i * 16) + 8, 0, (16 - 8));
						}
					} else {
						memcpy(currentProject->pal->palType, palTypeTmp, 32);
						memset(currentProject->pal->rgbPal + (32 * 3), 0, 32 * 3);
						memset(currentProject->pal->palType + 32, 0, 32);
					}

					uint8_t*nPtr = currentProject->pal->rgbPal;

					for (unsigned i = 0; i < currentProject->pal->colorCnt; ++i, nPtr += 3)
						currentProject->pal->rgbToEntry(nPtr[0], nPtr[1], nPtr[2], i);
				} else if (currentProject->gameSystem == TMS9918) {
					currentProject->gameSystem = sel;
					palCopyConvert(16);
				} else
					show_TODO_error
					break;

			case NES:
				if (currentProject->gameSystem == segaGenesis) {
					sortBy(2, true);
					uint8_t rgbTmp[64 * 3];
					memcpy(rgbTmp, currentProject->pal->rgbPal, 64 * 3);
					currentProject->gameSystem = sel;
					palBar.setSys(false);
					updateEmphesis();
					uint8_t*nPtr = currentProject->pal->rgbPal;
					uint8_t*rgbPtr = rgbTmp;

					for (unsigned k = 0; k < 4; ++k) {
						//Preserve background color
						*nPtr++ = rgbPtr[0];
						*nPtr++ = rgbPtr[1];
						*nPtr++ = rgbPtr[2];
						rgbPtr += 4 * 3;

						for (unsigned j = 4 * 3; j < 16 * 3; j += 4 * 3) {
							double Lv, Cv, Hv;
							Rgb2Lch255(&Lv, &Cv, &Hv, rgbPtr[0], rgbPtr[1], rgbPtr[2]);
							rgbPtr += 3;

							for (unsigned i = 1; i < 4; ++i) {
								double L, C, H;
								Rgb2Lch255(&L, &C, &H, rgbPtr[0], rgbPtr[1], rgbPtr[2]);

								if (C > Cv) {
									Lv = L;
									Cv = C;
									Hv = H;
								}

								rgbPtr += 3;
							}

							Lch2Rgb255(nPtr, nPtr + 1, nPtr + 2, Lv, Cv, Hv);
							nPtr += 3;
						}
					}

					memset(currentProject->pal->palType, 0, currentProject->pal->colorCnt + currentProject->pal->colorCntalt);
					nPtr = currentProject->pal->rgbPal;

					for (unsigned i = 0; i < 16; ++i, nPtr += 3)
						currentProject->pal->rgbToEntry(nPtr[0], nPtr[1], nPtr[2], i);

					memcpy(currentProject->pal->rgbPal + (currentProject->pal->colorCnt * 3), currentProject->pal->rgbPal, std::min(currentProject->pal->colorCnt, currentProject->pal->colorCntalt) * 3);
					memcpy(currentProject->pal->palDat + (currentProject->pal->colorCnt * currentProject->pal->esize), currentProject->pal->palDat, std::min(currentProject->pal->colorCnt, currentProject->pal->colorCntalt)*currentProject->pal->esize);
				} else if (currentProject->gameSystem == masterSystem || currentProject->gameSystem == gameGear) {
					currentProject->gameSystem = sel;
					palCopyConvert(32);
				} else if (currentProject->gameSystem == TMS9918) {
					currentProject->gameSystem = sel;
					palCopyConvert(16);
				} else
					show_TODO_error
					break;

			case masterSystem:
			case gameGear:
				if (currentProject->gameSystem == segaGenesis) {
					sortBy(2, true);
					sortBy(0, true);
					//Average each color
					uint8_t rgbTmp[64 * 3];
					memcpy(rgbTmp, currentProject->pal->rgbPal, 64 * 3);
					currentProject->gameSystem = sel;
					palBar.setSys(false);
					uint8_t*rgbPtr = rgbTmp;
					uint8_t*nPtr = currentProject->pal->rgbPal;

					for (unsigned i = 0; i < 32; ++i) {
						double L[2], c[2], h[2];
						Rgb2Lch255(L, c, h, *rgbPtr, rgbPtr[1], rgbPtr[2]);
						rgbPtr += 3;
						Rgb2Lch255(L + 1, c + 1, h + 1, *rgbPtr, rgbPtr[1], rgbPtr[2]);
						Lch2Rgb255(nPtr, nPtr + 1, nPtr + 2, (L[0] + L[1]) / 2., (c[0] + c[1]) / 2., (h[0] + h[1]) / 2.);
						nPtr += 3;
					}

					memset(currentProject->pal->palType, 0, currentProject->pal->colorCnt + currentProject->pal->colorCntalt);
					nPtr = currentProject->pal->rgbPal;

					for (unsigned i = 0; i < 32; ++i, nPtr += 3)
						currentProject->pal->rgbToEntry(nPtr[0], nPtr[1], nPtr[2], i);
				} else if (currentProject->gameSystem == NES || currentProject->gameSystem == masterSystem || currentProject->gameSystem == gameGear) {
					currentProject->gameSystem = sel;
					palCopyConvert(32);
				} else if (currentProject->gameSystem == TMS9918) {
					currentProject->gameSystem = sel;
					palCopyConvert(16);
				} else
					show_TODO_error
					break;

			case TMS9918:
				currentProject->gameSystem = sel;
				currentProject->subSystem = 0;
				currentProject->setTMS9918subSys(MODE_2);
				break;
		}

		currentProject->pal->paletteToRgb();
		palBar.updateSliders();
	}

	switch (sel) {
		case segaGenesis:
			bd = 4;
			currentProject->gameSystem = segaGenesis;
			currentProject->subSystem = 0;
			currentProject->setBitdepthSys(bd);

			if (currentProject->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(4);
					window->spritesize[1]->maximum(4);
				}

				currentProject->ms->sps[msprt].enforceMax(4, 4);
			}

			if (window) {
				window->subSysC->copy(subSysGenesis);
				window->subSysC->value((currentProject->subSystem & sgSHmask) >> sgSHshift);
			}

			break;

		case NES:
			bd = 2;
			currentProject->gameSystem = NES;
			currentProject->subSystem = 0;
			currentProject->setBitdepthSys(bd);
			currentProject->subSystem |= NES2x2;

			if (currentProject->containsData(pjHaveMap)) {
				//on the NES tilemaps need to be a multiple of 2
				if (((currentProject->tms->maps[currentProject->curPlane].mapSizeW) & 1) && ((currentProject->tms->maps[currentProject->curPlane].mapSizeHA) & 1))
					currentProject->tms->maps[currentProject->curPlane].resize_tile_map(currentProject->tms->maps[currentProject->curPlane].mapSizeW + 1, currentProject->tms->maps[currentProject->curPlane].mapSizeHA + 1);
				else if ((currentProject->tms->maps[currentProject->curPlane].mapSizeW) & 1)
					currentProject->tms->maps[currentProject->curPlane].resize_tile_map(currentProject->tms->maps[currentProject->curPlane].mapSizeW + 1, currentProject->tms->maps[currentProject->curPlane].mapSizeHA);
				else if ((currentProject->tms->maps[currentProject->curPlane].mapSizeHA) & 1)
					currentProject->tms->maps[currentProject->curPlane].resize_tile_map(currentProject->tms->maps[currentProject->curPlane].mapSizeW, currentProject->tms->maps[currentProject->curPlane].mapSizeHA + 1);
			}

			if (currentProject->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(1);
					window->spritesize[1]->maximum(2);
				}

				currentProject->ms->sps[msprt].enforceMax(1, 2);
			}

			if (window) {
				window->subSysC->copy(subSysNES);
				window->subSysC->value(currentProject->subSystem & NES2x2);
			}

			break;

		case masterSystem:
		case gameGear:
			currentProject->gameSystem = sel;
			currentProject->subSystem = 0;
			bd = 4;
			currentProject->setBitdepthSys(bd);

			if (currentProject->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(1);
					window->spritesize[1]->maximum(2);
				}

				currentProject->ms->sps[msprt].enforceMax(1, 2);
			}

			break;

		case TMS9918:
			currentProject->gameSystem = sel;
			bd = 1;
			currentProject->setBitdepthSys(bd);

			if (currentProject->containsData(pjHaveSprites)) {
				if (window) {
					window->spritesize[0]->maximum(1);
					window->spritesize[1]->maximum(2);
				}

				currentProject->ms->sps[msprt].enforceMax(1, 2);
			}

			if (window) {
				window->subSysC->copy(subSysTMS9918);
				window->subSysC->value(currentProject->getTMS9918subSys());
			}

			break;

		case frameBufferPal:
		{	currentProject->gameSystem = frameBufferPal;
			currentProject->subSystem = 0;

			if (bd > 8)
				bd = 8;

			currentProject->setBitdepthSys(bd);
		}
		break;

		default:
			show_default_error
			break;
	}

	currentProject->tileC->changeDim(8, 8, bd);

	if (currentProject->containsData(pjHaveSprites)) {
		int spRow = currentProject->fixedSpirtePalRow();

		if (spRow >= 0) {
			currentProject->ms->sps[msprt].allToPalRow(spRow);
			palBar.changeRow(spRow, 3);
		}

		if (window)
			window->updateSpriteSliders();
	}

	if (currentProject->containsData(pjHaveTiles)) {
		if ((!((sel == masterSystem || sel == gameGear) && (gold == NES) && currentProject->containsData(pjHaveMap))) && (bd == bdold))
			goto freeIt;

		{	gameSystemEnum gnew = currentProject->gameSystem;
			uint32_t snew = currentProject->subSystem;

			for (unsigned i = 0; i < tilesOld->amt; ++i) {
				unsigned usedRow = 0;

				if ((sel == masterSystem || sel == gameGear) && (gold == NES) && currentProject->containsData(pjHaveMap)) {
					int x, y;

					for (unsigned t = 0; t < currentProject->tms->maps.size(); ++t) {
						currentProject->tms->maps[t].findFirst(x, y, i);

						if (x >= 0 && y >= 0) {
							usedRow = currentProject->tms->maps[t].getPalRow(x, y);
							break;
						}
					}
				}

				for (unsigned y = 0; y < std::min(currentProject->tileC->height(), tilesOld->height()); ++y) {
					for (unsigned x = 0; x < std::min(currentProject->tileC->width(), tilesOld->width()); ++x) {
						currentProject->gameSystem = gold;
						currentProject->subSystem = sold;
						uint32_t px = tilesOld->getPixel(i, x, y);

						if ((sel == masterSystem || sel == gameGear) && (gold == NES) && currentProject->containsData(pjHaveMap))
							px += usedRow * 4;

						else if (bd != bdold) {
							if (bdold > bd)
								px >>= bdold - bd;
						}

						currentProject->gameSystem = gnew;
						currentProject->subSystem = snew;
						currentProject->tileC->setPixel(i, x, y, px);
					}
				}
			}
		}
freeIt:
		delete tilesOld;
	}

	if (currentProject->containsData(pjHaveMap)) {
		if ((sel == masterSystem || sel == gameGear) && (gold == NES)) {
			for (unsigned i = 0; i < currentProject->tms->maps.size(); ++i) {
				for (unsigned y = 0; y < currentProject->tms->maps[i].mapSizeHA; ++y) {
					for (unsigned x = 0; x < currentProject->tms->maps[i].mapSizeW; ++x)
						currentProject->tms->maps[i].set_pal_row(x, y, 0);
				}
			}
		} else {
			for (unsigned i = 0; i < currentProject->tms->maps.size(); ++i) {
				unsigned maxPal = 0;

				for (unsigned y = 0; y < currentProject->tms->maps[i].mapSizeHA; ++y) {
					for (unsigned x = 0; x < currentProject->tms->maps[i].mapSizeW; ++x) {
						unsigned tstPal = currentProject->tms->maps[i].getPalRow(x, y);

						if (tstPal > maxPal)
							maxPal = tstPal;
					}
				}

				if (maxPal >= currentProject->pal->rowCntPal) {
					unsigned divBy = (maxPal + 1 + (currentProject->pal->rowCntPal / 2)) / currentProject->pal->rowCntPal;

					for (unsigned y = 0; y < currentProject->tms->maps[i].mapSizeHA; ++y) {
						for (unsigned x = 0; x < currentProject->tms->maps[i].mapSizeW; ++x)
							currentProject->tms->maps[i].set_pal_row(x, y, currentProject->tms->maps[i].getPalRow(x, y) / divBy);
					}
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
