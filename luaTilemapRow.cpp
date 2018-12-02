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
#include "luaTilemapRow.hpp"
#include "luaTilemapEntry.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
static int tilemapRow__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);

		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects[idxPtr[0]]->tms->maps[idxPtr[1]].mapSizeW) {
			luaopen_TilemapEntry(L, idxPtr[0], idxPtr[1], idxPtr[2], k);
			return 1;
		}
	}

	return 0;
}

static int tilemapRow__len_(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t tilemapIDX = idxPtr[1];

	lua_pushinteger(L, projects[projectIDX]->tms->maps[tilemapIDX].mapSizeW);
	return 1;
}

static int tilemapRow___tostring(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	lua_pushfstring(L, "Tilemap row: %d from tilemap: %d from project: %d", idxPtr[2], idxPtr[1], idxPtr[0]);
	return 1;
}

static const struct luaL_Reg tilemapRow_member_methods[] = {
	{ "__index", tilemapRow__get_       },
	{ "__len", tilemapRow__len_       },
	{ "__tostring", tilemapRow___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_TilemapRow(lua_State *L, size_t projectIDX, size_t tilemapIDX, size_t rowIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tilemapRow");
	// <mt>

	// register member methods
	dub::fregister(L, tilemapRow_member_methods);
	dub::setup(L, "tilemapRow");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 3);
	idxUserData[0] = projectIDX;
	idxUserData[1] = tilemapIDX;
	idxUserData[2] = rowIDX;

	luaL_getmetatable(L, "tilemapRow");
	lua_setmetatable(L, -2);
	return 1;
}
