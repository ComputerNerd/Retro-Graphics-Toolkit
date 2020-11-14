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
#include "luaHelpers.hpp"
#include "luaProjects.hpp"
#include "luaProject.hpp"
#include "dub/dub.h"
#include "project.h"

static int projects__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2);

		if (k > 0 && k <= projects->size()) {
			luaopen_Project(L, k - 1);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("currentIdx", k)) {
			lua_pushinteger(L, curProjectID + 1);
			return 1;
		} else if (!strcmp("current", k)) {
			luaopen_Project(L, curProjectID);
			return 1;
		}
	}

	return 0;
}

static int projects__len_(lua_State *L) {
	lua_pushinteger(L, projects->size());
	return 1;
}

static int projects___tostring(lua_State *L) {
	lua_pushfstring(L, "projects table: %p Entries: %d", projects, projects->size());
	return 1;
}

static int lua_projects_load(lua_State*L) {
	lua_pushboolean(L, loadAllProjects(lua_tostring(L, 1)));
	switchProject(curProjectID, curProjectID);
	return 1;
}

static int lua_projects_save(lua_State*L) {
	lua_pushboolean(L, saveAllProjects(lua_tostring(L, 1)));
	return 1;
}

static const struct luaL_Reg projects_member_methods[] = {
	{ "__index", projects__get_       },
	{ "__len", projects__len_       },
	{ "__tostring", projects___tostring  },
	{ "deleted", dub::isDeleted       },
	{ "load", lua_projects_load},
	{ "save", lua_projects_save},
	{ NULL, NULL},
};
void luaCreateProjectsTable(lua_State* L) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "projects");
	// <mt>

	// register member methods
	dub::fregister(L, projects_member_methods);
	// setup meta-table
	dub::setup(L, "projects");
	// <mt>
	dub::pushudata(L, &projects, "projects", true);
	lua_setglobal(L, "projects");
}
