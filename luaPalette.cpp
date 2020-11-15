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
#include "luaPalette.hpp"
#include "luaPaletteEntry.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"
#include "runlua.h"
#include "classpalettebar.h"

static int lua_palette_paletteToRgb(lua_State*L) {
	getProjectIDX
	projects->at(projectIDX).pal->paletteToRgb();
	return 0;
}

static int lua_palette_save(lua_State*L) {
	getProjectIDX
	//void savePalette(const char*fname,unsigned start,unsigned end,bool skipzero,fileType_t type,int clipboard,const char*label="palDat");
	projects->at(projectIDX).pal->savePalette(lua_tostring(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4), lua_toboolean(L, 5), (fileType_t)lua_tointeger(L, 6), lua_toboolean(L, 7), lua_tostring(L, 8));
	return 0;
}

static int lua_palette_load(lua_State*L) {
	getProjectIDX
	projects->at(projectIDX).pal->loadFromFile(lua_tostring(L, 2),
	        (fileType_t)luaL_checkinteger(L, 3),
	        luaL_checkinteger(L, 4),
	        (CompressionType)luaL_checkinteger(L, 5));
	return 0;
}

static int lua_palette_maxInRow(lua_State*L) {
	getProjectIDX
	lua_pushinteger(L, projects->at(projectIDX).pal->calMaxPerRow(luaL_optinteger(L, 2, 0)));
	return 1;
}

static int lua_palette_rgbToValue(lua_State*L) {
	getProjectIDX
	lua_pushinteger(L, projects->at(projectIDX).pal->rgbToValue(luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0), luaL_optinteger(L, 4, 0)));
	return 1;
}

static int lua_palette_valueToRGB(lua_State*L) {
	getProjectIDX
	rgbArray_t res = projects->at(projectIDX).pal->valueToRGB(luaL_optinteger(L, 2, 0));

	for (int i = 0; i < 3; ++i)
		lua_pushinteger(L, res[i]);

	return 3;
}

static int lua_palette_rgbToNearestSystemColor(lua_State*L) {
	getProjectIDX
	rgbArray_t tmp;

	for (unsigned i = 0; i < 3; ++i)
		tmp[i] = luaL_optinteger(L, 2 + i, 0);

	auto res = projects->at(projectIDX).pal->rgbToNearestSystemColor(tmp);

	for (unsigned i = 0; i < 3; ++i)
		lua_pushinteger(L, res[i]);

	return 3;
}

static int lua_palette_importRGB(lua_State*L) {
	getProjectRef

	int arg2Type = lua_type(L, 2);

	if (arg2Type != LUA_TTABLE) {
		fputs("The first argument to importRGB should be a table.", stderr);
		return 0;
	}

	std::vector<uint8_t> rgbPal;
	tableToVector(L, 2, rgbPal);

	palette tmp(rgbPal.data(), luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), luaL_checkinteger(L, 6), luaL_checkinteger(L, 7));

	prj.pal->import(tmp);

	palBar.updateSliders();

	return 0;
}

static int palette__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectRef

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < prj.pal->colorCnt + prj.pal->colorCntalt) {
			luaopen_PaletteEntry(L, projectIDX, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char* k = luaL_checkstring(L, 2);

		if (!strcmp("cnt", k)) {
			lua_pushinteger(L, prj.pal->colorCnt);
			return 1;
		} else if (!strcmp("cntAlt", k)) {
			lua_pushinteger(L, prj.pal->colorCntalt);
			return 1;
		} else if (!strcmp("cntTotal", k)) {
			lua_pushinteger(L, prj.pal->colorCntalt + prj.pal->colorCnt);
			return 1;
		} else if (!strcmp("perRow", k)) {
			lua_pushinteger(L, prj.pal->perRow);
			return 1;
		} else if (!strcmp("perRowAlt", k)) {
			lua_pushinteger(L, prj.pal->perRowalt);
			return 1;
		} else if (!strcmp("rowCnt", k)) {
			lua_pushinteger(L, prj.pal->rowCntPal);
			return 1;
		} else if (!strcmp("rowCntAlt", k)) {
			lua_pushinteger(L, prj.pal->rowCntPalalt);
			return 1;
		} else if (!strcmp("haveAlt", k)) {
			lua_pushboolean(L, prj.pal->haveAlt);
			return 1;
		} else if (!strcmp("esize", k)) {
			lua_pushinteger(L, prj.pal->esize);
			return 1;
		} else if (!strcmp("fixedSpriteRow", k)) {
			lua_pushinteger(L, prj.pal->fixedSpriteRow);
			return 1;
		} else if (!strcmp("fixedPalette", k)) {
			lua_pushboolean(L, prj.isFixedPalette());
			return 1;
		}
	}

	return 0;
}

static int palette__len_(lua_State *L) {
	getProjectRef
	lua_pushinteger(L, prj.pal->colorCnt + prj.pal->colorCntalt);
	return 1;
}

static int palette___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "palette table: %p", projects->at(projectIDX).pal);
	return 1;
}

static const struct luaL_Reg palette_member_methods[] = {
	{ "__index", palette__get_       },
	{ "__len", palette__len_       },
	{ "__tostring", palette___tostring  },
	{ "save", lua_palette_save},
	{ "load", lua_palette_load},
	{ "toRgbAll", lua_palette_paletteToRgb},
	{ "maxInRow", lua_palette_maxInRow},
	{ "rgbToValue", lua_palette_rgbToValue},
	{ "valueToRGB", lua_palette_valueToRGB},
	{ "rgbToNearestSystemColor", lua_palette_rgbToNearestSystemColor},
	{ "importRGB", lua_palette_importRGB},
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_Palette(lua_State *L, size_t projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "palette");
	// <mt>

	// register member methods
	dub::fregister(L, palette_member_methods);
	dub::setup(L, "palette");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "palette");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}
