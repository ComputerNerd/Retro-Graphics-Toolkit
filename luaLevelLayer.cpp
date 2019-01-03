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

#include "luaLevelLayer.hpp"
#include "luaLevelLayerRow.hpp"
#include "luaLevelObjects.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "level_levelInfo.h"

static int lua_levelLayer_remove(lua_State*L) {
	getProjectIDX
	size_t levelLayerIDX = idxPtr[1];

	projects[projectIDX].lvl->removeLayer(levelLayerIDX);
	return 0;
}

static int lua_levelLayer_resize(lua_State*L) {
	getProjectIDX
	size_t levelLayerIDX = idxPtr[1];

	projects[projectIDX].lvl->resizeLayer(levelLayerIDX, luaL_optinteger(L, 2, 1), luaL_optinteger(L, 3, 1));
	return 0;
}

static int levelLayer__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectIDX
	const size_t idx2 = idxPtr[1];

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		struct levelInfo* info = projects[projectIDX].lvl->getInfo(projectIDX);

		if (k >= 0 && k < info->h) {
			luaopen_LevelLayerRow(L, projectIDX, idx2, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("info", k)) {
			luaopen_level_levelInfo(L, projects[projectIDX].lvl->getInfo(idx2));
			return 1;
		} else if (!strcmp("objects", k)) {
			luaopen_LevelObjects(L, projectIDX, idx2);
			return 1;
		} else if (!strcmp("name", k)) {
			lua_pushstring(L, projects[projectIDX].lvl->layernames[idx2].c_str());
			return 1;
		}
	}

	return 0;
}

static int levelLayer___tostring(lua_State *L) {
	getIdxPtrChk
	lua_pushfstring(L, "LevelLayer: %d from project: %d", idxPtr[0], idxPtr[1]);
	return 1;
}

static int levelLayer__len_(lua_State *L) {
	getProjectIDX
	size_t levelLayerIDX = idxPtr[1];
	struct levelInfo* info = projects[projectIDX].lvl->getInfo(levelLayerIDX);

	lua_pushinteger(L, info->h);
	return 1;
}

static int levelLayer__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectIDX
	const size_t levelLayerIDX = idxPtr[1];

	const char*k = luaL_checkstring(L, 2);

	if (!strcmp("name", k))
		projects[projectIDX].lvl->layernames[levelLayerIDX].assign(lua_tostring(L, 3));

	return 0;
}

static const struct luaL_Reg levelLayer_member_methods[] = {
	{ "__newindex", levelLayer__set_       },
	{ "__index", levelLayer__get_       },
	{ "__len", levelLayer__len_       },
	{ "__tostring", levelLayer___tostring  },
	{ "remove", lua_levelLayer_remove},
	{ "resize", lua_levelLayer_resize},
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_LevelLayer(lua_State *L, size_t projectIDX, size_t levelLayerIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "levelLayer");
	// <mt>

	// register member methods
	dub::fregister(L, levelLayer_member_methods);
	dub::setup(L, "levelLayer");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	idxUserData[0] = projectIDX;
	idxUserData[1] = levelLayerIDX;

	luaL_getmetatable(L, "levelLayer");
	lua_setmetatable(L, -2);
	return 1;
}

