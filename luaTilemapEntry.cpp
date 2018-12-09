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
#include "luaTilemapEntry.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "classtilemap.h"

static int lua_tilemap_setFull(lua_State*L) {
	getProjectIDX
	const size_t tilemapIDX = idxPtr[1];
	const size_t columnIDX = idxPtr[2];
	const size_t entryIDX = idxPtr[3];
	projects[projectIDX]->tms->maps[tilemapIDX].set_tile_full(entryIDX, // x
	        columnIDX, // y
	        luaL_optinteger(L, 2, 0), // tile
	        luaL_optboolean(L, 3, false), // palette_row
	        luaL_optboolean(L, 4, false), // hflip
	        luaL_optboolean(L, 5, false), // vflip
	        luaL_optboolean(L, 6, false)); // priority
	return 0;
}

static int tilemapEntry__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectIDX
	const size_t tilemapIDX = idxPtr[1];
	const size_t columnIDX = idxPtr[2];
	const size_t entryIDX = idxPtr[3];
	class tileMap *tm = &projects[projectIDX]->tms->maps[tilemapIDX];

	const char*k = luaL_checkstring(L, 2);

	if (!strcmp("hflip", k))
		tm->set_hflip(entryIDX, columnIDX, lua_toboolean(L, 3));
	else if (!strcmp("vflip", k))
		tm->set_vflip(entryIDX, columnIDX, lua_toboolean(L, 3));
	else if (!strcmp("priority", k))
		tm->set_prio(entryIDX, columnIDX, lua_toboolean(L, 3));
	else if (!strcmp("tile", k))
		tm->set_tile(entryIDX, columnIDX, lua_tointeger(L, 3));
	else if (!strcmp("row", k))
		tm->set_pal_row(entryIDX, columnIDX, lua_tointeger(L, 3));

	return 0;
}

static int lua_tilemap_getTileRow(lua_State*L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t entryIDX = idxPtr[1];
	lua_pushinteger(L, projects[projectIDX]->tms->maps[entryIDX].get_tileRow(luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0), luaL_optinteger(L, 4, 0)));
	return 1;
}

static int tilemapEntry__get_(lua_State *L) {
	checkAlreadyExists
	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		const char *key = luaL_checkstring(L, 2);
		getProjectIDX
		const size_t tilemapIDX = idxPtr[1];
		const size_t columnIDX = idxPtr[2];
		const size_t entryIDX = idxPtr[3];
		const class tileMap *tm = &projects[projectIDX]->tms->maps[tilemapIDX];

		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("hflip", k)) {
			lua_pushboolean(L, tm->get_hflip(entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("vflip", k)) {
			lua_pushboolean(L, tm->get_vflip(entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("priority", k)) {
			lua_pushboolean(L, tm->get_prio(entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("tile", k)) {
			lua_pushinteger(L, tm->get_tile(entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("row", k)) {
			lua_pushinteger(L, tm->getPalRow(entryIDX, columnIDX));
			return 1;
		}
	}

	return 0;
}

static int tilemapEntry___tostring(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	lua_pushfstring(L, "Tilemap Entry (%d, %d) from tilemap %d project %d", idxPtr[2], idxPtr[3], idxPtr[1], idxPtr[0]);
	return 1;
}
static const struct luaL_Reg tilemapEntry_member_methods[] = {
	{ "__newindex", tilemapEntry__set_       },
	{ "__index", tilemapEntry__get_       },
	{ "setFull", lua_tilemap_setFull       },
	{ "getTileRow", lua_tilemap_getTileRow       },
	{ "__tostring", tilemapEntry___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_TilemapEntry(lua_State *L, size_t projectIDX, size_t tilemapIDX, size_t columnIDX, size_t entryIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tilemapEntry");
	// <mt>

	// register member methods
	dub::fregister(L, tilemapEntry_member_methods);
	dub::setup(L, "tilemapEntry");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 4);
	idxUserData[0] = projectIDX;
	idxUserData[1] = tilemapIDX;
	idxUserData[2] = columnIDX;
	idxUserData[3] = entryIDX;

	luaL_getmetatable(L, "tilemapEntry");
	lua_setmetatable(L, -2);
	return 1;
}


