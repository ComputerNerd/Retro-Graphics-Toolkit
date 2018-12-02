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
#include "dub/dub.h"
#include "luaHelpers.hpp"
#include "luaPalette.hpp"
#include "luaProject.hpp"
#include "luaTilemaps.hpp"
#include "luaTiles.hpp"
#include "project.h"

static int project__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	size_t idx = getSizeTUserData(L);

	if (!strcmp(key, "name"))
		projects[idx]->Name.assign(luaL_checkstring(L, 3));
	else if (!strcmp("settings", key))
		projects[idx]->luaSettings = luaL_optinteger(L, 3, 0);

	return 0;
}
static int lua_project_rgt_have(lua_State*L) {
	lua_pushboolean(L, projects[getSizeTUserData(L)]->containsData(luaL_optinteger(L, 2, pjHavePal)));
	return 1;
}

static int lua_project_rgt_haveOR(lua_State*L) {
	lua_pushboolean(L, projects[getSizeTUserData(L)]->containsDataOR(luaL_optinteger(L, 2, pjHavePal)));
	return 1;
}

static int lua_project_getPalTab(lua_State*L) {
	lua_pushinteger(L, projects[getSizeTUserData(L)]->getPalTab());
	return 1;
}

static int lua_project_setPalTab(lua_State*L) {
	projects[getSizeTUserData(L)]->setPalTab(luaL_optinteger(L, 2, 0));
	return 0;
}

static int lua_project_load(lua_State*L) {
	loadProject(getSizeTUserData(L), lua_tostring(L, 2));
	return 0;
}

static int lua_project_save(lua_State*L) {
	saveProject(getSizeTUserData(L), lua_tostring(L, 2));
	return 0;
}

static int lua_project_rgt_haveMessage(lua_State*L) {
	unsigned mask = luaL_optinteger(L, 2, pjHavePal);
	projects[getSizeTUserData(L)]->haveMessage(mask);
	return 0;
}

static int project__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		size_t idx = getSizeTUserData(L);

		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("name", k)) {
			lua_pushstring(L, projects[idx]->Name.c_str());
			return 1;
		} else if (!strcmp("tiles", k)) {
			if (projects[idx]->containsData(pjHaveTiles)) {
				luaopen_Tiles(L, idx);
				return 1;
			}
		} else if (!strcmp("tilemaps", k)) {
			if (projects[idx]->containsData(pjHaveMap)) {
				luaopen_Tilemaps(L, idx);
				return 1;
			}
		} else if (!strcmp("palette", k)) {
			if (projects[idx]->containsData(pjHavePal)) {
				luaopen_Palette(L, idx);
				return 1;
			}
		} else if (!strcmp("settings", k)) {
			lua_pushinteger(L, projects[idx]->luaSettings);
			return 1;
		} else if (!strcmp("have", k)) {
			lua_pushcfunction(L, &lua_project_rgt_have);
			return 1;
		} else if (!strcmp("haveOR", k)) {
			lua_pushcfunction(L, &lua_project_rgt_haveOR);
			return 1;
		} else if (!strcmp("haveMessage", k)) {
			lua_pushcfunction(L, &lua_project_rgt_haveMessage);
			return 1;
		} else if (!strcmp("setPalType", k)) {
			lua_pushcfunction(L, &lua_project_setPalTab);
			return 1;
		} else if (!strcmp("getPalType", k)) {
			lua_pushcfunction(L, &lua_project_getPalTab);
			return 1;
		} else if (!strcmp("load", k)) {
			lua_pushcfunction(L, &lua_project_load);
			return 1;
		} else if (!strcmp("save", k)) {
			lua_pushcfunction(L, &lua_project_save);
			return 1;
		}
	}

	return 0;
}

static int project___tostring(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t idx = *idxPtr;
	lua_pushfstring(L, "project table: %p Entries: %d", projects[idx], projects_count);
	return 1;
}


static const struct luaL_Reg project_member_methods[] = {
	{ "__newindex", project__set_       },
	{ "__index", project__get_       },
	{ "__tostring", project___tostring  },
	{ "deleted", dub::isDeleted      },
	{ NULL, NULL},
};

int luaopen_Project(lua_State *L, unsigned idx) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "project");
	// <mt>

	// register member methods
	dub::fregister(L, project_member_methods);
	dub::setup(L, "project");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "project");
	*idxUserData = idx;
	lua_setmetatable(L, -2);
	return 1;
}
