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
#include "luaTile.hpp"
#include "luaTileRGBArow.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"

/* static int lua_tile_setTileRGBA(lua_State*L) {
	unsigned tile = luaL_optinteger(L, 1, 0);

	if (inRangeTile(tile)) {
		uint8_t*tptr = ((uint8_t*)currentProject->tileC->truetDat.data() + (tile * currentProject->tileC->tcSize));
		unsigned len = lua_rawlen(L, 2);

		if (!len) {
			fl_alert("setTileRGBA error: parameter 2 must be a table");
			return 0;
		}

		fillucharFromTab(L, 2, len, currentProject->tileC->tcSize, tptr);
	}

	return 0;
}*/

static int tile__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
		size_t idx = *idxPtr;
		size_t tileIDX = idxPtr[1];

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
		} else if (!strcmp("rgba", k)) {
			luaopen_TileRGBArow(L, idx, tileIDX);
			return 1;
		}

		return 0;
	}
}

static int tile___tostring(lua_State * L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	lua_pushfstring(L, "Tile: %d from project: %d", idxPtr[0], idxPtr[1]);
	return 1;
}
static const struct luaL_Reg tile_member_methods[] = {
	{ "__index", tile__get_       },
	{ "__tostring", tile___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_Tile(lua_State * L, size_t projectIDX, size_t tileIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tile");
	// <mt>

	// register member methods
	dub::fregister(L, tile_member_methods);
	dub::setup(L, "tile");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	idxUserData[0] = projectIDX;
	idxUserData[1] = tileIDX;

	luaL_getmetatable(L, "tile");
	lua_setmetatable(L, -2);
	return 1;
}
