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
#include "luaTilePixelsRow.hpp"
#include "luaTilemap.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"
static int tilePixelsRow__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		getProjectIDX
		const size_t tileIDX = idxPtr[1];
		const size_t yIDX = idxPtr[2];

		if (k >= 0 && k < projects->at(projectIDX).tileC->width()) {
			lua_pushinteger(L, projects->at(projectIDX).tileC->getPixel(tileIDX, k, yIDX));
			return 1;
		}
	}

	return 0;
}

static int tilePixelsRow__set_(lua_State *L) {
	getProjectIDX
	const size_t tileIDX = idxPtr[1];
	const size_t yIDX = idxPtr[2];
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects->at(projectIDX).tileC->width())
			projects->at(projectIDX).tileC->setPixel(tileIDX, k, yIDX, luaL_checkinteger(L, 3));
	}

	return 0;
}

static int tilePixelsRow__len_(lua_State *L) {
	getProjectIDX
	lua_pushinteger(L, projects->at(projectIDX).tileC->width());
	return 1;
}

static int tilePixelsRow___tostring(lua_State *L) {
	lua_pushstring(L, "tilePixelsRow table");
	return 1;
}

static const struct luaL_Reg tilePixelsRow_member_methods[] = {
	{ "__newindex", tilePixelsRow__set_       },
	{ "__index", tilePixelsRow__get_       },
	{ "__len", tilePixelsRow__len_       },
	{ "__tostring", tilePixelsRow___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_TilePixelsRow(lua_State *L, size_t projectIDX, size_t tileIDX, size_t yIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tilePixelsRow");
	// <mt>

	// register member methods
	dub::fregister(L, tilePixelsRow_member_methods);
	dub::setup(L, "tilePixelsRow");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 3);
	luaL_getmetatable(L, "tilePixelsRow");
	idxUserData[0] = projectIDX;
	idxUserData[1] = tileIDX;
	idxUserData[2] = yIDX;

	lua_setmetatable(L, -2);
	return 1;
}

