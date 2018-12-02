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
#include "luaTiles.hpp"
#include "luaTile.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"

static int lua_tiles_save(lua_State*L) {
	//void save(const char*fname,fileType_t type,bool clipboard,int compression);
	projects[getSizeTUserData(L)]->tileC->save(lua_tostring(L, 2), (fileType_t)lua_tointeger(L, 3), lua_toboolean(L, 4), lua_tointeger(L, 5), lua_tostring(L, 6));
	return 0;
}

static int lua_tiles_removeDuplicate(lua_State*L) {
	projects[getSizeTUserData(L)]->tileC->remove_duplicate_tiles(lua_toboolean(L, 2));
	return 0;
}

static int lua_tiles_append(lua_State*L) {
	size_t idx = getSizeTUserData(L);
	projects[idx]->tileC->appendTile(luaL_optinteger(L, 2, 1));

	if (curProjectID == idx)
		updateTileSelectAmt();

	return 0;
}

static int lua_tiles_resize(lua_State*L) {
	puts(lua_typename(L, lua_type(L, 0)));
	puts(lua_typename(L, lua_type(L, 1)));
	puts(lua_typename(L, lua_type(L, 2)));
	puts(lua_typename(L, lua_type(L, 3)));
	puts(lua_typename(L, lua_type(L, 4)));
	size_t idx = getSizeTUserData(L);
	projects[idx]->tileC->resizeAmt(luaL_optinteger(L, 2, 1));

	if (curProjectID == idx)
		updateTileSelectAmt();

	return 0;
}

static int tiles__get_(lua_State *L) {
	int type = lua_type(L, 2);
	size_t idx = getSizeTUserData(L);

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects[idx]->tileC->amt) {
			luaopen_Tile(L, idx, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("current", k)) {
			lua_pushinteger(L, projects[idx]->tileC->current_tile + 1);
			return 1;
		} else if (!strcmp("tileSize", k)) {
			lua_pushinteger(L, projects[idx]->tileC->tileSize);
			return 1;
		} else if (!strcmp("tcSize", k)) {
			lua_pushinteger(L, projects[idx]->tileC->tcSize);
			return 1;
		} else if (!strcmp("w", k)) {
			lua_pushinteger(L, projects[idx]->tileC->sizew);
			return 1;
		} else if (!strcmp("h", k)) {
			lua_pushinteger(L, projects[idx]->tileC->sizeh);
			return 1;
		} else if (!strcmp("save", k)) {
			lua_pushcfunction(L, &lua_tiles_save);
			return 1;
		} else if (!strcmp("removeDuplicate", k)) {
			lua_pushcfunction(L, &lua_tiles_removeDuplicate);
			return 1;
		} else if (!strcmp("append", k)) {
			lua_pushcfunction(L, &lua_tiles_append);
			return 1;
		} else if (!strcmp("resize", k)) {
			lua_pushcfunction(L, &lua_tiles_resize);
			return 1;
		}
	}

	return 0;
}

static int tiles__len_(lua_State *L) {
	lua_pushinteger(L, projects[getSizeTUserData(L)]->tileC->amt);
	return 1;
}

static int tiles___tostring(lua_State *L) {
	lua_pushfstring(L, "tiles table: %p", projects[getSizeTUserData(L)]->tileC);
	return 1;
}

static const struct luaL_Reg tiles_member_methods[] = {
	{ "__index", tiles__get_       },
	{ "__len", tiles__len_       },
	{ "__tostring", tiles___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_Tiles(lua_State *L, unsigned idx) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tiles");
	// <mt>

	// register member methods
	dub::fregister(L, tiles_member_methods);
	dub::setup(L, "tiles");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "tiles");
	*idxUserData = idx;
	lua_setmetatable(L, -2);
	return 1;
}
