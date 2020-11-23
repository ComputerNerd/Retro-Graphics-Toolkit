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
#include "luaTilemapBlocks.hpp"
#include "luaTilemapBlock.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
static int tilemapBlocks__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);

		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects->at(idxPtr[0]).tms->maps[idxPtr[1]].amt) {
			luaopen_TilemapBlock(L, idxPtr[0], idxPtr[1], k);
			return 1;
		}
	}

	return 0;
}

static int tilemapBlocks__len_(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t tilemapIDX = idxPtr[1];

	lua_pushinteger(L, projects->at(projectIDX).tms->maps[tilemapIDX].amt);
	return 1;
}

static int tilemapBlocks___tostring(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	lua_pushfstring(L, "Tilemap blocks from tilemap: %d from project: %d", idxPtr[1], idxPtr[0]);
	return 1;
}

static const struct luaL_Reg tilemapBlocks_member_methods[] = {
	{ "__index", tilemapBlocks__get_       },
	{ "__len", tilemapBlocks__len_       },
	{ "__tostring", tilemapBlocks___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_TilemapBlocks(lua_State *L, size_t projectIDX, size_t tilemapIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tilemapBlocks");
	// <mt>

	// register member methods
	dub::fregister(L, tilemapBlocks_member_methods);
	dub::setup(L, "tilemapBlocks");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	idxUserData[0] = projectIDX;
	idxUserData[1] = tilemapIDX;

	luaL_getmetatable(L, "tilemapBlocks");
	lua_setmetatable(L, -2);
	return 1;
}
