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
#include "luaLevelLayerRow.hpp"
#include "level_levDat.h"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"

static int levelLayerRow__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		getProjectIDX
		const size_t levelLayerIDX = idxPtr[1];

		int k = luaL_checkinteger(L, 2) - 1;
		struct levelInfo* info = projects[projectIDX].lvl->getInfo(levelLayerIDX);

		if (k >= 0 && k < info->w) {
			luaopen_level_levDat(L, projects[projectIDX].lvl->getlevDat(levelLayerIDX, k, idxPtr[2]));
			return 1;
		}
	}

	return 0;
}

static int levelLayerRow__len_(lua_State *L) {
	getProjectIDX
	size_t levelLayerIDX = idxPtr[1];
	struct levelInfo* info = projects[projectIDX].lvl->getInfo(levelLayerIDX);

	lua_pushinteger(L, info->w);
	return 1;
}

static int levelLayerRow___tostring(lua_State *L) {
	getIdxPtrChk
	lua_pushfstring(L, "LevelLayer row: %d from levelLayer: %d from project: %d", idxPtr[2], idxPtr[1], idxPtr[0]);
	return 1;
}

static const struct luaL_Reg levelLayerRow_member_methods[] = {
	{ "__index", levelLayerRow__get_       },
	{ "__len", levelLayerRow__len_       },
	{ "__tostring", levelLayerRow___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_LevelLayerRow(lua_State *L, size_t projectIDX, size_t levelLayerIDX, size_t rowIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "levelLayerRow");
	// <mt>

	// register member methods
	dub::fregister(L, levelLayerRow_member_methods);
	dub::setup(L, "levelLayerRow");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 3);
	idxUserData[0] = projectIDX;
	idxUserData[1] = levelLayerIDX;
	idxUserData[2] = rowIDX;

	luaL_getmetatable(L, "levelLayerRow");
	lua_setmetatable(L, -2);
	return 1;
}

