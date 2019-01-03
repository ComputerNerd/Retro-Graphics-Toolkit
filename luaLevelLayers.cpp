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
#include "luaLevelLayer.hpp"
#include "project.h"

static int levelLayers__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectIDX

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);

		if (k > 0 && k <= projects[projectIDX].lvl->layeramt) {
			luaopen_LevelLayer(L, projectIDX, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING)
		const char*k = luaL_checkstring(L, 2);

	return 0;
}

static int levelLayers___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "levelLayers table: %p", projects[projectIDX].lvl);
	return 1;
}

static int levelLayers__len_(lua_State *L) {
	getProjectIDX
	lua_pushinteger(L, projects[projectIDX].lvl->layeramt);
	return 1;
}

static int lua_levelLayers_amount(lua_State*L) {
	getProjectIDX

	projects[projectIDX].lvl->setlayeramt(luaL_checkinteger(L, 2), luaL_checkboolean(L, 3));
	return 0;
}

static const struct luaL_Reg levelLayers_member_methods[] = {
	{ "__index", levelLayers__get_       },
	{ "__tostring", levelLayers___tostring  },
	{ "__len", levelLayers__len_       },
	{ "amount", lua_levelLayers_amount},
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_LevelLayers(lua_State *L, size_t projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "levelLayers");
	// <mt>

	// register member methods
	dub::fregister(L, levelLayers_member_methods);
	dub::setup(L, "levelLayers");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "levelLayers");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}

