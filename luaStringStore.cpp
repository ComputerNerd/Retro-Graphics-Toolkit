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
	Copyright Sega16 (or whatever you wish to call me) (2012-2020)
*/

#include "dub/dub.h"
#include "luaHelpers.hpp"
#include "project.h"

static int stringStore__set_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		getProjectRef

		std::vector<uint8_t> vKey;
		luaStringToVector(L, 2, vKey, 1, false, -1);

		std::vector<uint8_t> vVal;
		luaStringToVector(L, 3, vVal, 1, false, -1);

		prj.luaStringStore[vKey] = vVal;
	}

	return 0;
}

static int stringStore__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		getProjectRef

		std::vector<uint8_t> vKey;
		luaStringToVector(L, 2, vKey, 1, false, -1);

		auto keySearch = prj.luaStringStore.find(vKey);

		if (keySearch != prj.luaStringStore.end()) {
			lua_pushlstring(L, (const char*)keySearch->second.data(), keySearch->second.size());
			return 1;
		}
	}

	return 0;
}

static int stringStore___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "stringStore table: %p", &projects->at(projectIDX).luaStringStore);
	return 1;
}

static const struct luaL_Reg stringStore_member_methods[] = {
	{ "__newindex", stringStore__set_},
	{ "__index", stringStore__get_       },
	{ "__tostring", stringStore___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_StringStore(lua_State *L, size_t projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "stringStore");
	// <mt>

	// register member methods
	dub::fregister(L, stringStore_member_methods);
	dub::setup(L, "stringStore");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "stringStore");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}
