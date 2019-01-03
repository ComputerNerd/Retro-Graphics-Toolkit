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
#include "dub/dub.h"
#include "luaHelpers.hpp"
#include "project.h"
#include "luaLevelLayers.hpp"

static int lua_level_subType(lua_State*L) {
	getProjectIDX
	projects[projectIDX].lvl->subType(lua_tointeger(L, 2), lua_tointeger(L, 3), (enum source)lua_tointeger(L, 4), lua_tointeger(L, 5));
	return 0;
}

static int level__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectIDX

	if (type == LUA_TSTRING) {
		const char* k = luaL_checkstring(L, 2);

		if (!strcmp("layers", k)) {
			luaopen_LevelLayers(L, projectIDX);
			return 1;
		}
	}

	return 0;
}

static int level___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "level table: %p", projects[projectIDX].lvl);
	return 1;
}

static const struct luaL_Reg level_member_methods[] = {
	{ "__index", level__get_       },
	{ "__tostring", level___tostring  },
	{ "subType", lua_level_subType},
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_Level(lua_State *L, size_t projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "level");
	// <mt>

	// register member methods
	dub::fregister(L, level_member_methods);
	dub::setup(L, "level");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "level");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}
