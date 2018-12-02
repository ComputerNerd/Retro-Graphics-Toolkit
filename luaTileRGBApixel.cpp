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
#include "luaTileRGBApixel.hpp"
#include "luaTilemap.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"
static int tileRGBApixel__set_(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = idxPtr[0];
	size_t tileIDX = idxPtr[1];
	size_t yIDX = idxPtr[2];
	size_t xIDX = idxPtr[3];
	int type = lua_type(L, 2);

	uint8_t*rgbaPixel = (uint8_t*)projects[projectIDX]->tileC->getPixelPtrTC(tileIDX, xIDX, yIDX);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < 4) {
			rgbaPixel[k] = luaL_checkinteger(L, 3);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("r", k))
			rgbaPixel[0] = luaL_checkinteger(L, 3);

		else if (!strcmp("g", k))
			rgbaPixel[1] = luaL_checkinteger(L, 3);

		else if (!strcmp("b", k))
			rgbaPixel[2] = luaL_checkinteger(L, 3);

		else if (!strcmp("a", k))
			rgbaPixel[3] = luaL_checkinteger(L, 3);

	}

	return 0;
}
static int tileRGBApixel__get_(lua_State *L) {
	int type = lua_type(L, 2);
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = idxPtr[0];
	size_t tileIDX = idxPtr[1];
	size_t yIDX = idxPtr[2];
	size_t xIDX = idxPtr[3];

	uint8_t*rgbaPixel = (uint8_t*)projects[projectIDX]->tileC->getPixelPtrTC(tileIDX, xIDX, yIDX);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < 4) {
			lua_pushinteger(L, rgbaPixel[k]);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("r", k)) {
			lua_pushinteger(L, rgbaPixel[0]);
			return 1;
		} else if (!strcmp("g", k)) {
			lua_pushinteger(L, rgbaPixel[1]);
			return 1;
		} else if (!strcmp("b", k)) {
			lua_pushinteger(L, rgbaPixel[2]);
			return 1;
		}
	}

	return 0;
}

static int tileRGBApixel__len_(lua_State *L) {
	lua_pushinteger(L, projects[getSizeTUserData(L)]->tileC->sizeh);
	return 1;
}

static int tileRGBApixel___tostring(lua_State *L) {
	lua_pushstring(L, "tileRGBApixel table");
	return 1;
}

static const struct luaL_Reg tileRGBApixel_member_methods[] = {
	{ "__newindex", tileRGBApixel__set_       },
	{ "__index", tileRGBApixel__get_       },
	{ "__len", tileRGBApixel__len_       },
	{ "__tostring", tileRGBApixel___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_TileRGBApixel(lua_State *L, size_t projectIDX, size_t tileIDX, size_t yIDX, size_t xIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tileRGBApixel");
	// <mt>

	// register member methods
	dub::fregister(L, tileRGBApixel_member_methods);
	dub::setup(L, "tileRGBApixel");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 4);
	luaL_getmetatable(L, "tileRGBApixel");
	idxUserData[0] = projectIDX;
	idxUserData[1] = tileIDX;
	idxUserData[2] = yIDX;
	idxUserData[3] = xIDX;
	lua_setmetatable(L, -2);
	return 1;
}
