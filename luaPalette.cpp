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
#include "luaPalette.hpp"
#include "luaPaletteEntry.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"

static int lua_palette_paletteToRgb(lua_State*L) {
	projects[getSizeTUserData(L)]->pal->paletteToRgb();
	return 0;
}

static int lua_palette_save(lua_State*L) {
	//void savePalette(const char*fname,unsigned start,unsigned end,bool skipzero,fileType_t type,int clipboard,const char*label="palDat");
	projects[getSizeTUserData(L)]->pal->savePalette(lua_tostring(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4), lua_toboolean(L, 5), (fileType_t)lua_tointeger(L, 6), lua_toboolean(L, 7), lua_tostring(L, 8));
	return 0;
}

static int lua_palette_maxInRow(lua_State*L) {
	lua_pushinteger(L, projects[getSizeTUserData(L)]->pal->calMaxPerRow(luaL_optinteger(L, 2, 0)));
	return 1;
}

static int palette__get_(lua_State *L) {
	int type = lua_type(L, 2);
	size_t idx = getSizeTUserData(L);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects[idx]->pal->colorCnt + projects[idx]->pal->colorCntalt) {
			luaopen_PaletteEntry(L, idx, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char* k = luaL_checkstring(L, 2);

		if (!strcmp("current", k)) {
			lua_pushinteger(L, projects[idx]->curPlane + 1);
			return 1;
		} else if (!strcmp("save", k)) {
			lua_pushcfunction(L, &lua_palette_save);
			return 1;
		} else if (!strcmp("toRgbAll", k)) {
			lua_pushcfunction(L, &lua_palette_paletteToRgb);
			return 1;
		} else if (!strcmp("maxInRow", k)) {
			lua_pushcfunction(L, &lua_palette_maxInRow);
			return 1;
		} else if (!strcmp("cnt", k)) {
			lua_pushinteger(L, projects[idx]->pal->colorCnt);
			return 1;
		} else if (!strcmp("perRow", k)) {
			lua_pushinteger(L, projects[idx]->pal->perRow);
			return 1;
		} else if (!strcmp("perRowAlt", k)) {
			lua_pushinteger(L, projects[idx]->pal->perRowalt);
			return 1;
		} else if (!strcmp("rowCnt", k)) {
			lua_pushinteger(L, projects[idx]->pal->rowCntPal);
			return 1;
		} else if (!strcmp("rowCntAlt", k)) {
			lua_pushinteger(L, projects[idx]->pal->rowCntPalalt);
			return 1;
		} else if (!strcmp("haveAlt", k)) {
			lua_pushinteger(L, projects[idx]->pal->haveAlt);
			return 1;
		} else if (!strcmp("esize", k)) {
			lua_pushinteger(L, projects[idx]->pal->esize);
			return 1;
		}
	}

	return 0;
}

static int palette__len_(lua_State *L) {
	size_t idx = getSizeTUserData(L);
	lua_pushinteger(L, projects[idx]->pal->colorCnt + projects[idx]->pal->colorCntalt);
	return 1;
}

static int palette___tostring(lua_State *L) {
	lua_pushfstring(L, "palette table: %p", projects[getSizeTUserData(L)]->tms);
	return 1;
}

static const struct luaL_Reg palette_member_methods[] = {
	{ "__index", palette__get_       },
	{ "__len", palette__len_       },
	{ "__tostring", palette___tostring  },
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
