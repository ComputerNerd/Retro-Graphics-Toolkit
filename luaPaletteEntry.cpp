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
#include "luaPaletteEntry.hpp"
#include "luaTilemap.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"
#include "errorMsg.h"

static int palette__set_(lua_State *L) {
	getProjectIDX
	size_t entryIDX = idxPtr[1];

	const char *key = luaL_checkstring(L, 2);

	if (!strcmp("r", key))
		projects[projectIDX]->pal->rgbPal[entryIDX * 3] = luaL_checkinteger(L, 3);
	else if (!strcmp("g", key))
		projects[projectIDX]->pal->rgbPal[entryIDX * 3 + 1] = luaL_checkinteger(L, 3);
	else if (!strcmp("b", key))
		projects[projectIDX]->pal->rgbPal[entryIDX * 3 + 2] = luaL_checkinteger(L, 3);
	else if (!strcmp("raw", key)) {
		unsigned val = luaL_optinteger(L, 3, 0);

		switch (projects[projectIDX]->pal->esize) {
			case 1:
				projects[projectIDX]->pal->palDat[entryIDX] = val;
				break;

			case 2:
				projects[projectIDX]->pal->palDat[entryIDX * 2] = val & 255;
				projects[projectIDX]->pal->palDat[entryIDX * 2 + 1] = val >> 8;
				break;

			default:
				show_default_error
				return 0;
		}

		projects[projectIDX]->pal->updateRGBindex(entryIDX);
	}

	return 0;
}

static int lua_palette_convertFromRGB(lua_State*L) {
	getProjectIDX
	size_t entryIDX = idxPtr[1];
	size_t ent3 = entryIDX * 3;

	projects[projectIDX]->pal->rgbToEntry(projects[projectIDX]->pal->rgbPal[ent3], projects[projectIDX]->pal->rgbPal[ent3 + 1], projects[projectIDX]->pal->rgbPal[ent3 + 2], entryIDX);

	return 1;
}

static int lua_palette_setRGB(lua_State*L) {
	getProjectIDX
	size_t entryIDX = idxPtr[1];

	projects[projectIDX]->pal->rgbToEntry(luaL_checkinteger(L, 2), luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), entryIDX);

	return 1;
}

static int paletteEntry__get_(lua_State *L) {
	int type = lua_type(L, 2);
	getProjectIDX
	size_t entryIDX = idxPtr[1];

	if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("r", k)) {
			entryIDX *= 3;
			lua_pushinteger(L, projects[projectIDX]->pal->rgbPal[entryIDX]);
			return 1;
		} else if (!strcmp("g", k)) {
			entryIDX *= 3;
			lua_pushinteger(L, projects[projectIDX]->pal->rgbPal[entryIDX + 1]);
			return 1;
		} else if (!strcmp("b", k)) {
			entryIDX *= 3;
			lua_pushinteger(L, projects[projectIDX]->pal->rgbPal[entryIDX + 2]);
			return 1;
		} else if (!strcmp("raw", k)) {
			switch (projects[projectIDX]->pal->esize) {
				case 1:
					lua_pushinteger(L, projects[projectIDX]->pal->palDat[entryIDX]);
					break;

				case 2:
					entryIDX *= 2;
					lua_pushinteger(L, projects[projectIDX]->pal->palDat[entryIDX] | (projects[projectIDX]->pal->palDat[entryIDX + 1] << 8));
					break;

				default:
					show_default_error
					return 0;
			}

			return 1;
		} else if (!strcmp("type", k)) {
			lua_pushinteger(L, projects[projectIDX]->pal->palType[luaL_optinteger(L, 1, 0)]);
			return 1;
		} else if (!strcmp("convertFromRGB", k)) {
			lua_pushcfunction(L, &lua_palette_convertFromRGB);
			return 1;
		} else if (!strcmp("setRGB", k)) {
			lua_pushcfunction(L, &lua_palette_setRGB);
			return 1;
		} else if (!strcmp("pType", k)) {
			lua_pushinteger(L, projects[projectIDX]->pal->palType[entryIDX]);
			return 1;
		}
	}

	return 0;
}

static int paletteEntry___tostring(lua_State *L) {
	lua_pushstring(L, "paletteEntry table");
	return 1;
}

static const struct luaL_Reg paletteEntry_member_methods[] = {
	{ "__newindex", palette__set_       },
	{ "__index", paletteEntry__get_       },
	{ "__tostring", paletteEntry___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_PaletteEntry(lua_State *L, size_t projectIDX, size_t entryIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "paletteEntry");
	// <mt>

	// register member methods
	dub::fregister(L, paletteEntry_member_methods);
	dub::setup(L, "paletteEntry");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	luaL_getmetatable(L, "paletteEntry");
	idxUserData[0] = projectIDX;
	idxUserData[1] = entryIDX;
	lua_setmetatable(L, -2);
	return 1;
}
