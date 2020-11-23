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
#include "luaTilemapBlock.hpp"
#include "luaTilemapRow.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"

static int tilemapBlock__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectRef

	if (!strcmp(key, "data")) {
		tileMap&tm = prj.tms->maps[idxPtr[1]];
		uint32_t* tmPtr = (uint32_t*)tm.tileMapDat;
		tmPtr += idxPtr[2] * tm.mapSizeW * tm.mapSizeH;
		std::vector<uint8_t> tmp;
		size_t blockSize = tm.mapSizeW * tm.mapSizeH * sizeof(uint32_t);
		luaStringToVector(L, 3, tmp, blockSize, true, -1);
		memcpy(tmPtr, tmp.data(), std::min(blockSize, tmp.size()));
	}
}

static int tilemapBlock__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TNUMBER) {
		const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);

		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects->at(idxPtr[0]).tms->maps[idxPtr[1]].mapSizeH) {
			// luaopen_TilemapRow(L, idxPtr[0], idxPtr[1], idxPtr[2], k);
			//return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("data", k)) {
			getProjectRef
			tileMap&tm = prj.tms->maps[idxPtr[1]];
			const uint32_t* tmPtr = (const uint32_t*)tm.tileMapDat;
			tmPtr += idxPtr[2] * tm.mapSizeW * tm.mapSizeH;
			lua_pushlstring(L, (const char*)tmPtr, tm.mapSizeW * tm.mapSizeH * sizeof(uint32_t));
			return 1;

		}
	}

	return 0;
}

static int tilemapBlock__len_(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t tilemapIDX = idxPtr[1];

	lua_pushinteger(L, projects->at(projectIDX).tms->maps[tilemapIDX].mapSizeH);
	return 1;
}

static int tilemapBlock___tostring(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	lua_pushfstring(L, "Tilemap block: %d from tilemap: %d from project: %d", idxPtr[2], idxPtr[1], idxPtr[0]);
	return 1;
}

static const struct luaL_Reg tilemapBlock_member_methods[] = {
	{ "__newindex", tilemapBlock__set_},
	{ "__index", tilemapBlock__get_       },
	{ "__len", tilemapBlock__len_       },
	{ "__tostring", tilemapBlock___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_TilemapBlock(lua_State *L, size_t projectIDX, size_t tilemapIDX, size_t blockIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tilemapBlock");
	// <mt>

	// register member methods
	dub::fregister(L, tilemapBlock_member_methods);
	dub::setup(L, "tilemapBlock");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 3);
	idxUserData[0] = projectIDX;
	idxUserData[1] = tilemapIDX;
	idxUserData[2] = blockIDX;

	luaL_getmetatable(L, "tilemapBlock");
	lua_setmetatable(L, -2);
	return 1;
}
