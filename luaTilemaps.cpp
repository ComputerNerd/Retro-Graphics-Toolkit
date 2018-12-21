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
#include "luaTilemaps.hpp"
#include "luaTilemap.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"
static int tilemaps__get_(lua_State *L) {
	int type = lua_type(L, 2);
	size_t idx = getSizeTUserData(L);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects[idx].tms->maps.size()) {
			luaopen_Tilemap(L, idx, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("current", k)) {
			luaopen_Tilemap(L, idx, projects[idx].curPlane);
			return 1;
		} else if (!strcmp("currentIdx", k)) {
			lua_pushinteger(L, projects[idx].curPlane + 1);
			return 1;
		}
	}

	return 0;
}

static int tilemaps__len_(lua_State *L) {
	getProjectIDX
	lua_pushinteger(L, projects[projectIDX].tms->maps.size());
	return 1;
}

static int tilemaps___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "tilemaps table: %p", projects[projectIDX].tms);
	return 1;
}


static const struct luaL_Reg tilemaps_member_methods[] = {
	{ "__index", tilemaps__get_       },
	{ "__len", tilemaps__len_       },
	{ "__tostring", tilemaps___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_Tilemaps(lua_State *L, size_t projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tilemaps");
	// <mt>

	// register member methods
	dub::fregister(L, tilemaps_member_methods);
	dub::setup(L, "tilemaps");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "tilemaps");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}
