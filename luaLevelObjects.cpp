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
#include <FL/fl_ask.H>

#include "luaLevelObjects.hpp"
#include "level_levobjDat.h"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "level_levelInfo.h"

static int lua_levelObjects_remove(lua_State*L) {
	getProjectIDX
	size_t layerIDX = idxPtr[1];

	projects[projectIDX].lvl->odat[layerIDX]->erase(projects[projectIDX].lvl->odat[layerIDX]->begin() + luaL_checkinteger(L, 2));
	return 0;
}

static int lua_levelObjects_append(lua_State*L) {
	getProjectIDX
	size_t layerIDX = idxPtr[1];

	projects[projectIDX].lvl->odat[layerIDX]->emplace_back();
	return 0;
}

static int levelObjects__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectIDX
	const size_t idx2 = idxPtr[1];
	struct level* lvl = projects[projectIDX].lvl;

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < lvl->odat.size()) {
			luaopen_level_levobjDat(L, lvl->getObjDat(idx2, k));
			return 1;
		}
	}

	return 0;
}

static int levelObjects___tostring(lua_State *L) {
	getIdxPtrChk
	lua_pushfstring(L, "LevelObjects from layer: %d from project: %d", idxPtr[1], idxPtr[0]);
	return 1;
}

static int levelObjects__len_(lua_State *L) {
	getProjectIDX
	size_t layerIDX = idxPtr[1];

	lua_pushinteger(L, projects[projectIDX].lvl->odat[layerIDX]->size());
	return 1;
}

static const struct luaL_Reg levelObjects_member_methods[] = {
	{ "__index", levelObjects__get_       },
	{ "__len", levelObjects__len_       },
	{ "__tostring", levelObjects___tostring  },
	{ "remove", lua_levelObjects_remove},
	{ "append", lua_levelObjects_append},
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_LevelObjects(lua_State *L, size_t projectIDX, size_t levelObjectsIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "levelObjects");
	// <mt>

	// register member methods
	dub::fregister(L, levelObjects_member_methods);
	dub::setup(L, "levelObjects");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	idxUserData[0] = projectIDX;
	idxUserData[1] = levelObjectsIDX;

	luaL_getmetatable(L, "levelObjects");
	lua_setmetatable(L, -2);
	return 1;
}
