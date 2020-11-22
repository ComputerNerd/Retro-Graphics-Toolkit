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
	Copyright Sega16 (or whatever you wish to call me) (2012-2020)
*/

/** @file runlua.cpp
 * This file runlua.cpp contains various functions related to the Lua bindings and for running Lua code
 *  @author Sega16*/
#include <string>
#include <FL/Fl_Color_Chooser.H>
#include <cmath>//Mingw workaround
#include <FL/Fl_File_Chooser.H>
#include <libgen.h>
#ifdef __MINGW32__
#include <direct.h>
#endif
#include <cstdio>
#include <errno.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>

#include "runlua.h"
#include "savepng.h"
#include "luaconfig.h"
#include "gui.h"
#include "project.h"
#include "color_convert.h"
#include "callback_gui.h"
#include "dither.h"
#include "CIE.h"
#include "class_global.h"
#include "palette.h"
#include "callback_project.h"
#include "lua_zlib.h"
#include "dub/dub.h"
#include <mdcomp/comper.hh>
#include <mdcomp/enigma.hh>
#include <mdcomp/nemesis.hh>
#include <mdcomp/kosinski.hh>
#include <mdcomp/saxman.hh>
#include "filemisc.h"
#include "undoLua.h"
#include "posix.h"
#include "callback_tilemap.h"
#include "luafltk.hpp"
#include "luaHelpers.hpp"
#include "luaSprites.hpp"
#include "luaProject.hpp"
#include "luaProjects.hpp"
#include "compressionWrapper.h"
#include "filereader_filereader.hpp"


static int panic(lua_State *L) {
	fl_alert("PANIC: unprotected error in call to Lua API (%s)\n", lua_tostring(L, -1));
	throw 0;//Otherwise abort() would be called when not needed
}
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
	if (nsize)
		return realloc(ptr, nsize);
	else {
		free(ptr);
		return nullptr;
	}
}
static int lua_palette_fixSliders(lua_State*L) {
	set_mode_tabs(0, 0);

	if (window)
		window->redraw();

	return 0;
}
static int lua_palette_sortByHSL(lua_State*L) {
	sortBy(luaL_optinteger(L, 1, 0), luaL_optboolean(L, 2, true));
	return 0;
}
static const luaL_Reg lua_paletteAPI[] = {
	{"fixSliders", lua_palette_fixSliders},
	{"sortByHSL", lua_palette_sortByHSL},
	{0, 0}
};
static int lua_tilemap_generate_optimal_paletteapply(lua_State *L) {
	try {
		settings *s = *((settings **)dub::checksdata(L, 1, "settings"));
		generate_optimal_paletteapply(nullptr, s);
		return 0;
	} catch (std::exception &e) {
		lua_pushfstring(L, "generate_optimal_paletteapply: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "generate_optimal_paletteapply: Unknown exception");
	}

	return dub::error(L);
}
static const luaL_Reg lua_tilemapAPI[] = {
	{"generatePalette", lua_tilemap_generate_optimal_paletteapply},
	{0, 0}
};
/** Set attributes (key, value)
 *
 */
static int settings__set_(lua_State *L) {

	settings *self = *((settings **)dub::checksdata_n(L, 1, "settings"));
	const char *key = luaL_checkstring(L, 2);
	int key_h = dub::hash(key, 10);

	switch (key_h) {
		case 9: {
			if (DUB_ASSERT_KEY(key, "sprite")) break;

			self->sprite = luaL_checkboolean(L, 3);
			return 0;
		}

		case 0: {
			if (DUB_ASSERT_KEY(key, "alg")) break;

			self->alg = luaL_checkinteger(L, 3);
			return 0;
		}

		case 2: {
			if (DUB_ASSERT_KEY(key, "ditherAfter")) break;

			self->ditherAfter = luaL_checkboolean(L, 3);
			return 0;
		}

		case 5: {
			if (DUB_ASSERT_KEY(key, "entireRow")) break;

			self->entireRow = luaL_checkboolean(L, 3);
			return 0;
		}

		case 8: {
			if (DUB_ASSERT_KEY(key, "colSpace")) break;

			self->colSpace = luaL_checkinteger(L, 3);
			return 0;
		}

		case 1: {
			if (DUB_ASSERT_KEY(key, "rowAuto")) break;

			self->rowAuto = luaL_checkinteger(L, 3);
			return 0;
		}
	}

	if (lua_istable(L, 1))
		lua_rawset(L, 1);

	else
		luaL_error(L, KEY_EXCEPTION_MSG, key);

	return 0;
}

/** Get attributes (key)
 *
 */
static int settings__get_(lua_State *L) {

	settings *self = *((settings **)dub::checksdata_n(L, 1, "settings", true));
	const char *key = luaL_checkstring(L, 2);
	// <self> "key" <mt>
	// rawget(mt, key)
	lua_pushvalue(L, 2);
	// <self> "key" <mt> "key"
	lua_rawget(L, -2);

	if (!lua_isnil(L, -1)) {
		// Found method.
		return 1;
	} else {
		// Not in mt = attribute access.
		lua_pop(L, 2);
	}

	int key_h = dub::hash(key, 10);

	switch (key_h) {
		case 9: {
			if (DUB_ASSERT_KEY(key, "sprite")) break;

			lua_pushboolean(L, self->sprite);
			return 1;
		}

		case 0: {
			if (DUB_ASSERT_KEY(key, "alg")) break;

			lua_pushinteger(L, self->alg);
			return 1;
		}

		case 2: {
			if (DUB_ASSERT_KEY(key, "ditherAfter")) break;

			lua_pushboolean(L, self->ditherAfter);
			return 1;
		}

		case 5: {
			if (DUB_ASSERT_KEY(key, "entireRow")) break;

			lua_pushboolean(L, self->entireRow);
			return 1;
		}

		case 8: {
			if (DUB_ASSERT_KEY(key, "colSpace")) break;

			lua_pushinteger(L, self->colSpace);
			return 1;
		}

		case 1: {
			if (DUB_ASSERT_KEY(key, "rowAuto")) break;

			lua_pushinteger(L, self->rowAuto);
			return 1;
		}
	}

	return 0;
}

/** settings()
 *
 */
static int settings_settings(lua_State *L) {
	try {
		settings *retval__ = new settings();
		dub::pushudata(L, retval__, "settings", true);
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "new: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "new: Unknown exception");
	}

	return dub::error(L);
}

/** Read off(size_t i)
 *
 */
static int settings_off(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);

		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			unsigned v = dub::checkinteger(L, 3);

			if (!i || i > MAX_ROWS_PALETTE) return 0;

			self->off[i - 1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);

			if (!i || i > MAX_ROWS_PALETTE) return 0;

			lua_pushinteger(L, self->off[i - 1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "off: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "off: Unknown exception");
	}

	return dub::error(L);
}

/** Read perRow(size_t i)
 *
 */
static int settings_perRow(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);

		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			unsigned v = dub::checkinteger(L, 3);

			if (!i || i > MAX_ROWS_PALETTE) return 0;

			self->perRow[i - 1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);

			if (!i || i > MAX_ROWS_PALETTE) return 0;

			lua_pushinteger(L, self->perRow[i - 1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "perRow: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "perRow: Unknown exception");
	}

	return dub::error(L);
}

/** Read useRow(size_t i)
 *
 */
static int settings_useRow(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);

		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			bool v = dub::checkboolean(L, 3);

			if (!i || i > MAX_ROWS_PALETTE) return 0;

			self->useRow[i - 1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);

			if (!i || i > MAX_ROWS_PALETTE) return 0;

			lua_pushboolean(L, self->useRow[i - 1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "useRow: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "useRow: Unknown exception");
	}

	return dub::error(L);
}

/** Read rowAutoEx(size_t i)
 *
 */
static int settings_rowAutoEx(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);

		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			int v = dub::checkinteger(L, 3);

			if (!i || i > 2) return 0;

			self->rowAutoEx[i - 1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);

			if (!i || i > 2) return 0;

			lua_pushinteger(L, self->rowAutoEx[i - 1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "rowAutoEx: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "rowAutoEx: Unknown exception");
	}

	return dub::error(L);
}



// --=============================================== __tostring
static int settings___tostring(lua_State *L) {
	settings *self = *((settings **)dub::checksdata_n(L, 1, "settings"));
	lua_pushfstring(L, "settings: %p", self);

	return 1;
}

// --=============================================== METHODS

static const struct luaL_Reg settings_member_methods[] = {
	{ "__newindex", settings__set_       },
	{ "__index", settings__get_       },
	{ "new", settings_settings    },
	{ "off", settings_off         },
	{ "perRow", settings_perRow      },
	{ "useRow", settings_useRow      },
	{ "rowAutoEx", settings_rowAutoEx   },
	{ "__tostring", settings___tostring  },
	{ "deleted", dub::isDeleted       },
	{ NULL, NULL},
};

int luaopen_settings(lua_State *L)
{
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "settings");
	// <mt>

	// register member methods
	dub::fregister(L, settings_member_methods);
	// setup meta-table
	dub::setup(L, "settings");
	// <mt>
	return 1;
}

static void updateLevelTable(lua_State*L);

bool luaL_checkboolean(lua_State* L, int n) {
	if (lua_isboolean(L, n))
		return lua_toboolean(L, n);
	else
		luaL_error(L, "Expected boolean for argument %d. Type: %s.", n, lua_typename(L, lua_type(L, -1)));

	return false;
}

static const struct dub::const_Reg level_const[] = {
	{ "TILES", ::TILES              },
	{ "BLOCKS", ::BLOCKS             },
	{ "CHUNKS", ::CHUNKS             },
	{ NULL, 0},
};
static int lua_project_set(lua_State*L);
static int lua_project_update(lua_State*L) {
	switchProjectSlider(curProjectID);
	updateProjectTablesLua(L);
	return 0;
}
static int lua_project_append(lua_State*L) {
	appendProject();
	return 0;
}
static int lua_project_remove(lua_State*L) {
	removeProject(lua_tointeger(L, 1));
	return 0;
}
static int lua_project_setSystem(lua_State*L) {
	set_game_system(nullptr, (void*)(uintptr_t)lua_tointeger(L, 1));
	return 0;
}
static const luaL_Reg lua_projectAPI[] = { /*!This is the project table. The global project contains the following functions*/
	{"set", lua_project_set},
	{"update", lua_project_update},
	{"append", lua_project_append},
	{"remove", lua_project_remove},
	{"setSystem", lua_project_setSystem},
	{0, 0}
};

static void updateLevelTable(lua_State*L) {
	lua_createtable(L, 0, (arLen(level_const) - 1));
	dub::register_const(L, level_const);
	lua_setglobal(L, "level");
}

void setProjectConstants(lua_State*L) {
	lua_createtable(L, 0, (sizeof(lua_projectAPI) / sizeof((lua_projectAPI)[0]) - 1) + 12);
	luaL_setfuncs(L, lua_projectAPI, 0);
	mkKeyunsigned(L, "palMask", pjHavePal);
	mkKeyunsigned(L, "tilesMask", pjHaveTiles);
	mkKeyunsigned(L, "mapMask", pjHaveMap);
	mkKeyunsigned(L, "chunksMask", pjHaveChunks);
	mkKeyunsigned(L, "spritesMask", pjHaveSprites);
	mkKeyunsigned(L, "levelMask", pjHaveLevel);
	mkKeyunsigned(L, "allMask", pjAllMask);
	mkKeyunsigned(L, "segaGenesis", segaGenesis);
	mkKeyunsigned(L, "NES", NES);
	mkKeyunsigned(L, "gameGear", gameGear);
	mkKeyunsigned(L, "masterSystem", masterSystem);
	lua_setglobal(L, "project");
}

void updateProjectTablesLua(lua_State*L) {
	//Retro Graphics Toolkit bindings
	lua_pushnil(L);
	lua_setglobal(L, "palette");

	if (currentProject->containsData(pjHavePal)) {
		lua_createtable(L, 0, (sizeof(lua_paletteAPI) / sizeof((lua_paletteAPI)[0]) - 1));
		luaL_setfuncs(L, lua_paletteAPI, 0);

		lua_setglobal(L, "palette");
	}

	lua_pushnil(L);
	lua_setglobal(L, "tilemaps");

	if (currentProject->containsData(pjHaveMap)) {
		lua_createtable(L, 0, (sizeof(lua_tilemapAPI) / sizeof((lua_tilemapAPI)[0]) - 1));
		luaL_setfuncs(L, lua_tilemapAPI, 0);

		lua_setglobal(L, "tilemaps");
	}

	updateLevelTable(L);

}
static int lua_project_set(lua_State*L) {
	unsigned off = luaL_optinteger(L, 1, 1) - 1;

	if ((off >= projects->size()) || (off == curProjectID))
		lua_pushboolean(L, false); //Failure
	else {
		switchProjectSlider(off);
		updateProjectTablesLua(L);
		lua_pushboolean(L, true);
	}

	return 1;
}
static int lua_rgt_redraw(lua_State*L) {
	if (window)
		window->redraw();

	return 0;
}
static int lua_rgt_damage(lua_State*L) {
	if (window)
		window->damage(FL_DAMAGE_USER1);

	return 0;
}
static int lua_rgt_hide(lua_State*L) {
	if (window)
		window->hide();

	return 0;
}
static int lua_rgt_clipboardAsk(lua_State*L) {
	lua_pushinteger(L, clipboardAsk());

	return 1;
}
static int lua_rgt_askSaveType(lua_State*L) {
	lua_pushinteger(L, (lua_Integer)askSaveType(lua_toboolean(L, 1), (fileType_t)luaL_checkinteger(L, 2)));

	return 1;
}
static int lua_rgt_compressionAsk(lua_State*L) {
	lua_pushinteger(L, (lua_Integer)compressionAsk());

	return 1;
}
static int lua_rgt_ditherImage(lua_State*L) {
	unsigned len = lua_rawlen(L, 1);

	if (!len) {
		fl_alert("ditherImage error: parameter 1 must be a table");
		return 0;
	}

	unsigned w = luaL_optinteger(L, 2, 0);
	unsigned h = luaL_optinteger(L, 3, 0);

	if (!w || !h) {
		fl_alert("Invalid width/height");
		return 0;
	}

	bool useAlpha = luaL_optinteger(L, 4, 0);
	unsigned bpp = useAlpha + 3;
	unsigned sz = w * h * bpp;
	uint8_t*image = (uint8_t*)malloc(sz);
	fillucharFromTab(L, 1, len, sz, image);
	ditherImage(image, w, h, useAlpha, luaL_optinteger(L, 5, 0), luaL_optinteger(L, 6, 0), luaL_optinteger(L, 7, 0), luaL_optinteger(L, 8, 0), luaL_optinteger(L, 9, 0), luaL_optinteger(L, 10, 0));
	uint8_t*imgptr = image;

	for (unsigned i = 1; i <= std::min(len, sz); ++i) {
		lua_pushinteger(L, *imgptr++);
		lua_rawseti(L, 1, i);
	}

	free(image);
	return 0;
}
static int lua_rgt_rgbToLab(lua_State*L) {
	double l, a, b;
	Rgb2Lab(&l, &a, &b, luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0));
	lua_pushnumber(L, l);
	lua_pushnumber(L, a);
	lua_pushnumber(L, b);
	return 3;
}
static int lua_rgt_labToRgb(lua_State*L) {
	double r, g, b;
	Lab2Rgb(&r, &g, &b, luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0));
	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);
	return 3;
}
static int lua_rgt_rgbToLch(lua_State*L) {
	double l, c, h;
	Rgb2Lch(&l, &c, &h, luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0));
	lua_pushnumber(L, l);
	lua_pushnumber(L, c);
	lua_pushnumber(L, h);
	return 3;
}
static int lua_rgt_lchToRgb(lua_State*L) {
	double r, g, b;
	Lch2Rgb(&r, &g, &b, luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0));
	lua_pushnumber(L, r);
	lua_pushnumber(L, g);
	lua_pushnumber(L, b);
	return 3;
}
static int lua_rgt_rgbToHsl(lua_State*L) {
	double h, s, l;
	rgbToHsl(luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0), &h, &s, &l);
	lua_pushnumber(L, h);
	lua_pushnumber(L, s);
	lua_pushnumber(L, l);
	return 3;
}

static int lua_rgt_hslToRgb(lua_State*L) {
	uint8_t r, g, b;
	hslToRgb(luaL_optnumber(L, 1, 0.0), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0), r, g, b);
	lua_pushinteger(L, r);
	lua_pushinteger(L, g);
	lua_pushinteger(L, b);
	return 3;
}

static int lua_rgt_setTab(lua_State*L) {
	if (!window)
		return 0;

	int idx = luaL_checkinteger(L, 1);

	if (idx < 0)
		idx = window->tabsMain.size() - 1;

	idx %= window->tabsMain.size();
	window->the_tabs->value(window->tabsMain[idx]);
	set_mode_tabs(nullptr, nullptr);

	return 0;
}
static int lua_rgt_syncProject(lua_State*L) {
	updateProjectTablesLua(L);
	return 0;
}
static int lua_rgt_w(lua_State*L) {
	if (window) {
		lua_pushinteger(L, window->w());
		return 1;
	} else
		return 0;
}
static int lua_rgt_h(lua_State*L) {
	if (window) {
		lua_pushinteger(L, window->h());
		return 1;
	} else
		return 0;
}

void tableToVector(lua_State*L, unsigned idx, std::vector<uint8_t>&vu8) {
	int len = lua_rawlen(L, idx);
	vu8.clear();
	vu8.reserve(len);

	for (int i = 1; i <= len; ++i) {
		lua_rawgeti(L, idx, i);
		int tmp = lua_tointeger(L, -1);

		if (tmp < 0)
			tmp = 0;

		if (tmp > 255)
			tmp = 255;

		lua_pop(L, 1);
		vu8.emplace_back(tmp);
	}
}

static int lua_rgt_savePNG(lua_State*L) {
	// int savePNG(const char * fileName, uint32_t width, uint32_t height, void * ptr, uint8_t*pal = nullptr, unsigned pn = 0, bool hasAlpha = false);
	unsigned width = luaL_checkinteger(L, 2);
	unsigned height = luaL_checkinteger(L, 3);
	bool hasAlpha = luaL_checkboolean(L, 6);
	// First check if we have a palette.
	int palType = lua_type(L, 5);
	std::vector<uint8_t> pal;

	if (palType == LUA_TTABLE)
		tableToVector(L, 5, pal);

	else if (palType != LUA_TNIL) {
		fl_alert("Unknown palette table type.");
		return 0;
	}

	if (!lua_istable(L, 4)) {
		fl_alert("Image must be a table.");
		return 0;
	}

	std::vector<uint8_t> img;
	tableToVector(L, 4, img);

	lua_pushinteger(L, savePNG(luaL_checkstring(L, 1), // Filename
	                           width,
	                           height,
	                           img.data(),
	                           pal.size() == 0 ? nullptr : pal.data(),
	                           pal.size() / 3,
	                           hasAlpha
	                          ));
	return 1;
}

static int lua_rgt_stringToTable(lua_State*L) {
	lua_newtable(L);
	int idx = 0;

	size_t len;
	const uint8_t*str = (const uint8_t*)lua_tolstring(L, 1, &len);

	if (str == nullptr)
		luaL_error(L, "lua_tolstring returned null in lua_rgt_stringToTable.");

	for (size_t i = 0; i < len; ++i) {
		lua_pushinteger(L, str[i]);
		lua_rawseti(L, -2, ++idx);
	}

	return 1;
}

static int lua_rgt_ucharTableToString(lua_State*L) {
	std::vector<uint8_t> tmp;
	tableToVector(L, 1, tmp);
	lua_pushlstring(L, (const char*)tmp.data(), tmp.size());
	return 1;
}

#if 0
extern "C" {
#include "ldo.h"
}
// This is disabled because it requires ldo.h. This is not provided with some system packages.
static int lua_rgt_testluaD_throw(lua_State*L) {
	// Lets the user test luaD_throw which if not handled correctly could call abort. This should never happen.
	// Because I am running the scripts in protected mode calling luaD_throw causes lua_pcall to return.
	// This is good because it means that the user will see the error instead of crashing Retro Graphics Toolkit.
	luaD_throw(L, 'T' << 24 | 'E' << 16 | 'S' << 8 | 'T');
	return 0;
}
#endif

static const luaL_Reg lua_rgtAPI[] = {
	{"redraw", lua_rgt_redraw},
	{"damage", lua_rgt_damage},
	{"hide", lua_rgt_hide},
	{"clipboardAsk", lua_rgt_clipboardAsk},
	{"askSaveType", lua_rgt_askSaveType},
	{"compressionAsk", lua_rgt_compressionAsk},
	{"ditherImage", lua_rgt_ditherImage},
	{"rgbToLab", lua_rgt_rgbToLab},
	{"labToRgb", lua_rgt_labToRgb},
	{"rgbToLch", lua_rgt_rgbToLch},
	{"lchToRgb", lua_rgt_lchToRgb},
	{"rgbToHsl", lua_rgt_rgbToHsl},
	{"hslToRgb", lua_rgt_hslToRgb},
	{"setTab", lua_rgt_setTab},
	{"syncProject", lua_rgt_syncProject},
	{"w", lua_rgt_w},
	{"h", lua_rgt_h},
	{"savePNG", lua_rgt_savePNG},
	{"stringToTable", lua_rgt_stringToTable},
	{"ucharTableToString", lua_rgt_ucharTableToString},
#if 0
	{"testluaD_throw", lua_rgt_testluaD_throw},
#endif
	{0, 0}
};
static const struct keyPairi rgtConsts[] = {
	{"paletteTab", pal_edit},
	{"tileTab", tile_edit},
	{"planeTab", tile_place},
	{"chunkTab", chunkEditor},
	{"spritesTab", spriteEditor},
	{"levelTab", levelEditor},
	{"settingsTab", settingsTab},
	{"luaTab", luaTab},
	{"tCancel", (int)fileType_t::tCancel},
	{"tBinary", (int)fileType_t::tBinary},
	{"tCheader", (int)fileType_t::tCheader},
	{"tASM", (int)fileType_t::tASM},
	{"tBEX", (int)fileType_t::tBEX}
};

static const struct keyPairi tileTypes[] = {
	{"linear", (int)tileType::LINEAR},
	{"planar", (int)tileType::PLANAR_TILE},
	{"planarLine", (int)tileType::PLANAR_LINE}
};

static const struct keyPairi endians[] = {
	{"native", (int)boost::endian::order::native},
	{"little", (int)boost::endian::order::little},
	{"big", (int)boost::endian::order::big}
};

static const struct keyPairi compressionTypes[] = {
	{"cancel", (int)CompressionType::Cancel},
	{"uncompressed", (int)CompressionType::Uncompressed},
	{"nemesis", (int)CompressionType::Nemesis},
	{"kosinski", (int)CompressionType::Kosinski},
	{"enigma", (int)CompressionType::Enigma},
	{"saxman", (int)CompressionType::Saxman},
	{"comper", (int)CompressionType::Comper},
};
static int lua_tabs_append(lua_State*L) {
	int rx, ry, rw, rh;

	if (window) {
		window->the_tabs->client_area(rx, ry, rw, rh);
		window->tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Lua scripting"));
	}

	return 0;
}
static int lua_tabs_endAppend(lua_State*L) {
	if (window)
		window->tabsMain[window->tabsMain.size() - 1]->end();

	return 0;
}
static const luaL_Reg lua_tabAPI[] = {
	{"appendTab", lua_tabs_append},
	{"endAppendTab", lua_tabs_endAppend},
	/*{"deleteTab", lua_tabs_append},
	{"getTabs", lua_tabs_append},*/
	{0, 0}
};
static void tableToSS(lua_State*L, unsigned idx, std::stringstream&ss) {
	int len = lua_rawlen(L, idx);

	for (int i = 1; i <= len; ++i) {
		lua_rawgeti(L, idx, i);
		int tmp = lua_tointeger(L, -1);

		if (tmp < 0)
			tmp = 0;

		if (tmp > 255)
			tmp = 255;

		lua_pop(L, 1);
		unsigned char tmpc = tmp;
		ss << tmpc;
	}

	ss.seekp(0, ss.beg);
}


static void SStoTable(lua_State*L, std::stringstream&ss) {
	lua_newtable(L);

	ss.seekg(0, ss.end);
	size_t len = ss.tellg();

	ss.seekg(0, ss.beg);
	for (int idx = 1; idx <= len; ++idx) {
		unsigned char c = ss.get();
		lua_pushinteger(L, c);
		lua_rawseti(L, -2, idx);
	}
}
#define mkDecompress(name) static int lua_##name##Decompress(lua_State*L){ \
	std::stringstream ss; \
	if(lua_type(L,1)==LUA_TSTRING){ \
		std::ifstream ifs (lua_tostring(L,1), std::ifstream::in|std::ifstream::binary); \
		name decoder; \
		decoder.decode(ifs,ss); \
		SStoTable(L,ss); \
		return 1; \
	}else if(lua_type(L,1)==LUA_TTABLE){ \
		std::stringstream ss_src; \
		tableToSS(L,1,ss_src); \
		name decoder; \
		decoder.decode(ss_src,ss); \
		SStoTable(L,ss); \
		return 1; \
	}else \
		return 0; \
}

#define mkDecompressEx(name,ex) static int lua_##name##Decompress(lua_State*L){ \
	std::stringstream ss; \
	if(lua_type(L,1)==LUA_TSTRING){ \
		std::ifstream ifs (lua_tostring(L,1), std::ifstream::in|std::ifstream::binary); \
		name decoder; \
		decoder.decode(ifs,ss,ex); \
		SStoTable(L,ss); \
		return 1; \
	}else if(lua_type(L,1)==LUA_TTABLE){ \
		std::stringstream ss_src; \
		tableToSS(L,1,ss_src); \
		name decoder; \
		decoder.decode(ss_src,ss,ex); \
		SStoTable(L,ss); \
		return 1; \
	}else \
		return 0; \
}
#define SINGLE_ARG(...) __VA_ARGS__
mkDecompress(comper)
mkDecompress(enigma)
mkDecompress(nemesis)
mkDecompress(kosinski)
mkDecompressEx(saxman, luaL_optboolean(L, 2, true))

#define mkCompress(name) static int lua_##name##Compress(lua_State*L){ \
	if(lua_type(L,1)==LUA_TTABLE){ \
		std::stringstream ss_src; \
		tableToSS(L,1,ss_src); \
		name encoder; \
		if(lua_type(L,2)==LUA_TSTRING){ \
			std::ofstream ofs(lua_tostring(L,2), std::ofstream::out|std::ofstream::binary); \
			encoder.encode(ss_src,ofs); \
			return 0; \
		}else{ \
			std::stringstream ss; \
			encoder.encode(ss_src,ss); \
			SStoTable(L,ss); \
			return 1; \
		} \
	}else \
		return 0; \
}
#define mkCompressEx(name,ex) static int lua_##name##Compress(lua_State*L){ \
	if(lua_type(L,1)==LUA_TTABLE){ \
		std::stringstream ss_src; \
		tableToSS(L,1,ss_src); \
		name encoder; \
		if(lua_type(L,2)==LUA_TSTRING){ \
			std::ofstream ofs(lua_tostring(L,2), std::ofstream::out|std::ofstream::binary); \
			encoder.encode(ss_src,ofs,ex); \
			return 0; \
		}else{ \
			std::stringstream ss; \
			encoder.encode(ss_src,ss,ex); \
			SStoTable(L,ss); \
			return 1; \
		} \
	}else \
		return 0; \
}
mkCompress(comper)
mkCompress(enigma)
mkCompress(kosinski)
mkCompress(nemesis)
mkCompressEx(saxman, luaL_checkboolean(L, 2))
static const luaL_Reg lua_kensAPI[] = {
	{"comperDecompress", lua_comperDecompress},
	{"comperCompress", lua_comperCompress},
	{"enigmaDecompress", lua_enigmaDecompress},
	{"enigmaCompress", lua_enigmaCompress},
	{"nemesisDecompress", lua_nemesisDecompress},
	{"nemesisCompress", lua_nemesisCompress},
	{"kosinskiCcompress", lua_kosinskiCompress},
	{"kosinskiDecompress", lua_kosinskiDecompress},
	{"saxmanDecompress", lua_saxmanDecompress},
	{"saxmanCompress", lua_saxmanCompress},
	{0, 0}
};
void runLuaFunc(lua_State*L, unsigned args, unsigned results) {
	try {
		if (lua_pcall(L, args, results, 0) != LUA_OK)
			luaL_error(L, "error: %s", lua_tostring(L, -1));
	} catch (std::exception &e) {
		fl_alert("Lua error: %s\nlua_tostring(): \%s", e.what(), lua_tostring(L, -1));
	} catch (...) {
		fl_alert("Lua error while running script\nthrow was called and the exception is unknown\nlua_tostring(): %s", lua_tostring(L, -1));
	}
}
void runLua(lua_State*L, const char*str, bool isFile) {
	try {
		int s;

		if (isFile)
			s = luaL_loadfile(L, str);
		else
			s = luaL_loadstring(L, str);

		if (s != LUA_OK && !lua_isnil(L, -1)) {
			const char *msg = lua_tostring(L, -1);

			if (msg == NULL) msg = "(error object is not a string)";

			fl_alert("%s", msg);
			lua_pop(L, 1);
		} else {
			// execute Lua program
			s = lua_pcall(L, 0, LUA_MULTRET, 0);

			if (s != LUA_OK) {
				const char *msg = (lua_type(L, -1) == LUA_TSTRING) ? lua_tostring(L, -1)
				                  : NULL;

				if (msg == NULL) msg = "(error object is not a string)";

				fl_alert("%s", msg);
				lua_pop(L, 1);
			}
		}
	} catch (std::exception &e) {
		fl_alert("Lua error: %s\nlua_tostring(): \%s", e.what(), lua_tostring(L, -1));
	} catch (...) {
		fl_alert("Lua error while running script\nthrow was called and the exception is unknown\nlua_tostring(): %s", lua_tostring(L, -1));
	}
}

lua_State* moonfltk_main_lua_state = 0;

void registerProjectTables(lua_State*L) {
	luaCreateProjectsTable(L);

	updateProjectTablesLua(L);

	setProjectConstants(L);
}

lua_State*createLuaState(void) {
	lua_State *L = lua_newstate(l_alloc, NULL);

	if (L) {
		moonfltk_main_lua_state = L;

		lua_atpanic(L, &panic);
		luaL_openlibs(L);

		createFLTKbindings(L);

		if (projects)
			registerProjectTables(L);

		lua_createtable(L, 0, arLen(rgtConsts) + arLen(lua_rgtAPI) - 1);
		luaL_setfuncs(L, lua_rgtAPI, 0);

		for (unsigned x = 0; x < arLen(rgtConsts); ++x)
			mkKeyint(L, rgtConsts[x].key, rgtConsts[x].pair);

		lua_setglobal(L, "rgt");

		lua_createtable(L, 0, arLen(tileTypes) - 1);
		for (unsigned x = 0; x < arLen(tileTypes); ++x)
			mkKeyint(L, tileTypes[x].key, tileTypes[x].pair);
		lua_setglobal(L, "tileTypes");

		lua_createtable(L, 0, arLen(endians) - 1);
		for (unsigned x = 0; x < arLen(endians); ++x)
			mkKeyint(L, endians[x].key, endians[x].pair);
		lua_setglobal(L, "endians");


		lua_createtable(L, 0, arLen(compressionTypes) - 1);

		for (unsigned x = 0; x < arLen(compressionTypes); ++x)
			mkKeyint(L, compressionTypes[x].key, compressionTypes[x].pair);

		lua_setglobal(L, "compressionType");

		luaL_newlib(L, lua_kensAPI);
		lua_setglobal(L, "mdcomp");

		luaopen_zlib(L);
		lua_setglobal(L, "zlib");

		luaopen_undoLua(L);
		lua_setglobal(L, "undo");

#ifndef __MINGW32__
		luaopen_posix_sys_time(L);
		lua_setglobal(L, "time");
		luaopen_posix_sys_msg(L);
		lua_setglobal(L, "msg");
		luaopen_posix_sys_times(L);
		lua_setglobal(L, "times");
		luaopen_posix_sys_resource(L);
		lua_setglobal(L, "resource");
		luaopen_posix_sys_utsname(L);
		lua_setglobal(L, "utsname");
		luaopen_posix_sys_wait(L);
		lua_setglobal(L, "wait");
		luaopen_posix_sys_stat(L);
		lua_setglobal(L, "stat");
		luaopen_posix_sys_socket(L);
		lua_setglobal(L, "socket");
		//luaopen_posix_sys_statvfs(L);
		luaopen_posix_grp(L);
		lua_setglobal(L, "grp");
		luaopen_posix_time(L);
		lua_setglobal(L, "time");
		luaopen_posix_glob(L);
		lua_setglobal(L, "glob");
		luaopen_posix_syslog(L);
		lua_setglobal(L, "syslog");
		luaopen_posix_stdlib(L);
		lua_setglobal(L, "stdlib");
		luaopen_posix_termio(L);
		lua_setglobal(L, "termio");
		luaopen_posix_ctype(L);
		lua_setglobal(L, "ctype");
		luaopen_posix_fcntl(L);
		lua_setglobal(L, "fcntl");
		luaopen_posix_poll(L);
		lua_setglobal(L, "poll");
		luaopen_posix_signal(L);
		lua_setglobal(L, "signal");
		luaopen_posix_utime(L);
		lua_setglobal(L, "utime");
		luaopen_posix_pwd(L);
		lua_setglobal(L, "pwd");
		luaopen_posix_errno(L);
		lua_setglobal(L, "errno");
		luaopen_posix_stdio(L);
		lua_setglobal(L, "stdio");
		luaopen_posix_sched(L);
		lua_setglobal(L, "sched");
		luaopen_posix_fnmatch(L);
		lua_setglobal(L, "fnmatch");
		luaopen_posix_unistd(L);
		lua_setglobal(L, "unistd");
#endif
		luaopen_posix_libgen(L);
		lua_setglobal(L, "libgen");
		luaopen_posix_dirent(L);
		lua_setglobal(L, "dirent");


		luaopen_settings(L);
		lua_setglobal(L, "settings");

		luaopen_filereader_filereader(L);
		lua_setglobal(L, "filereader");
	} else
		fl_alert("lua_newstate failed.");

	return L;
}
void runLuaCD(const char*fname) {

	char*dup = strdup(fname);
	char*dup2 = strdup(fname);
#ifdef _WIN32
	_chdir(dirname(dup));
#else
	chdir(dirname(dup));
#endif
	runLua(Lconf, basename(dup2));
	free(dup);
	free(dup2);
}
void runLuaCB(Fl_Widget*, void*) {
	char*st;

	if (st = loadsavefile("Select a Lua script", false)) {
		runLuaCD(st);
		free(st);
	}
}
