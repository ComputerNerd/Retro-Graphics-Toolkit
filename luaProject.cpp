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
	Copyright Sega16 (or whatever you wish to call me) (2012-2018)
*/
#include "dub/dub.h"
#include "luaHelpers.hpp"
#include "luaPalette.hpp"
#include "luaProject.hpp"
#include "luaTilemaps.hpp"
#include "luaTiles.hpp"
#include "luaChunks.hpp"
#include "luaSprites.hpp"
#include "project.h"

static int project__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectIDX

	if (!strcmp(key, "name"))
		projects[projectIDX].Name.assign(luaL_checkstring(L, 3));
	else if (!strcmp("luaSettings", key))
		projects[projectIDX].luaSettings = luaL_optinteger(L, 3, 0);

	return 0;
}
static int lua_project_have(lua_State*L) {
	getProjectIDX
	lua_pushboolean(L, projects[projectIDX].containsData(luaL_optinteger(L, 2, pjHavePal)));
	return 1;
}

static int lua_project_haveOR(lua_State*L) {
	getProjectIDX
	lua_pushboolean(L, projects[projectIDX].containsDataOR(luaL_optinteger(L, 2, pjHavePal)));
	return 1;
}

static int lua_project_getPalTab(lua_State*L) {
	getProjectIDX
	lua_pushinteger(L, projects[projectIDX].getPalTab());
	return 1;
}

static int lua_project_getSpriteSizeID(lua_State*L) {
	getProjectIDX
	lua_pushinteger(L, projects[projectIDX].getSpriteSizeID());
	return 1;
}

static int lua_project_setPalTab(lua_State*L) {
	getProjectIDX
	projects[projectIDX].setPalTab(luaL_optinteger(L, 2, 0));
	return 0;
}

static int lua_project_setSpriteSizeID(lua_State*L) {
	getProjectIDX
	projects[projectIDX].setSpriteSizeID(luaL_optinteger(L, 2, 0));
	return 0;
}

static int lua_project_load(lua_State*L) {
	getProjectIDX
	loadProject(projectIDX, lua_tostring(L, 2));
	return 0;
}

static int lua_project_save(lua_State*L) {
	getProjectIDX
	saveProject(projectIDX, lua_tostring(L, 2));
	return 0;
}

static int lua_project_haveMessage(lua_State*L) {
	unsigned mask = luaL_optinteger(L, 2, pjHavePal);
	getProjectIDX
	projects[projectIDX].haveMessage(mask);
	return 0;
}

static int project__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		getProjectIDX

		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("name", k)) {
			lua_pushstring(L, projects[projectIDX].Name.c_str());
			return 1;
		} else if (!strcmp("tiles", k)) {
			if (projects[projectIDX].containsData(pjHaveTiles)) {
				luaopen_Tiles(L, projectIDX);
				return 1;
			}
		} else if (!strcmp("tilemaps", k)) {
			if (projects[projectIDX].containsData(pjHaveMap)) {
				luaopen_Tilemaps(L, projectIDX);
				return 1;
			}
		} else if (!strcmp("palette", k)) {
			if (projects[projectIDX].containsData(pjHavePal)) {
				luaopen_Palette(L, projectIDX);
				return 1;
			}
		} else if (!strcmp("chunks", k)) {
			if (projects[projectIDX].containsData(pjHaveChunks)) {
				luaopen_Chunks(L, projectIDX);
				return 1;
			}
		} else if (!strcmp("settings", k)) {
			lua_pushinteger(L, projects[projectIDX].settings);
			return 1;
		} else if (!strcmp("luaSettings", k)) {
			lua_pushinteger(L, projects[projectIDX].luaSettings);
			return 1;
		} else if (!strcmp("gameSystem", k)) {
			lua_pushinteger(L, projects[projectIDX].gameSystem);
			return 1;
		} else if (!strcmp("metasprites", k)) {
			if (projects[projectIDX].containsData(pjHaveSprites)) {
				luaopen_MetaSprites(L, projectIDX);
				return 1;
			}
		}
	}

	return 0;
}

static int project___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "project table: %p", &projects[projectIDX]);
	return 1;
}


static const struct luaL_Reg project_member_methods[] = {
	{ "__newindex", project__set_       },
	{ "__index", project__get_       },
	{ "__tostring", project___tostring  },
	{ "deleted", dub::isDeleted      },
	{ "have", lua_project_have},
	{ "haveOR", lua_project_haveOR},
	{ "haveMessage", lua_project_haveMessage},
	{ "setPalType", lua_project_setPalTab},
	{ "getPalType", lua_project_getPalTab},
	{ "getSpriteSizeID", lua_project_getSpriteSizeID},
	{ "setSpriteSizeID", lua_project_setSpriteSizeID},
	{ "load", lua_project_load},
	{ "save", lua_project_save},
	{ NULL, NULL},
};

int luaopen_Project(lua_State *L, unsigned projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "project");
	// <mt>

	// register member methods
	dub::fregister(L, project_member_methods);
	dub::setup(L, "project");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "project");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}
