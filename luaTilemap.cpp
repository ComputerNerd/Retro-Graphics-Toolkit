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
#include "luaTilemap.hpp"
#include "luaTilemapRow.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "callback_tilemap.h"
static int lua_tilemap_dither(lua_State*L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t tilemapIDX = idxPtr[1];

	unsigned method = luaL_optinteger(L, 2, 1);

	projects[projectIDX]->tms->maps[tilemapIDX].ditherAsImage(method);
	return 0;
}

static int lua_tilemap_loadImage(lua_State*L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t tilemapIDX = idxPtr[1];

	load_image_to_tilemap_project_ptr(projects[projectIDX], lua_tostring(L, 2), luaL_optboolean(L, 3, false), luaL_optboolean(L, 4, false), luaL_optboolean(L, 5, false), tilemapIDX);
	return 0;
}

static int lua_tilemap_imageToTiles(lua_State*L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t tilemapIDX = idxPtr[1];

	unsigned len = lua_rawlen(L, 2);

	if (!len) {
		fl_alert("imageToTiles error: parameter 1 must be a table");
		return 0;
	}

	int row = luaL_optinteger(L, 3, -1);
	bool useAlpha = luaL_optboolean(L, 4, false);
	bool copyToTruecol = luaL_optboolean(L, 5, false);
	bool convert = luaL_optinteger(L, 6, 1);
	unsigned bpp = useAlpha + 3;
	uint32_t w, h;
	w = projects[projectIDX]->tms->maps[tilemapIDX].mapSizeW * projects[projectIDX]->tileC->sizew;
	h = projects[projectIDX]->tms->maps[tilemapIDX].mapSizeHA * projects[projectIDX]->tileC->sizeh;
	unsigned sz = w * h * bpp;
	uint8_t*image = (uint8_t*)malloc(sz);
	fillucharFromTab(L, 2, len, sz, image);
	projects[projectIDX]->tms->maps[tilemapIDX].truecolorimageToTiles(image, row, useAlpha, copyToTruecol, convert);
	free(image);
	return 0;
}

static int lua_tilemaps_removePlane(lua_State*L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t idx = *idxPtr;
	size_t idx2 = idxPtr[1];
	projects[idx]->tms->removePlane(idx2);
	return 0;
}

static int tilemap__get_(lua_State *L) {
	int type = lua_type(L, 2);
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t idx = *idxPtr;
	size_t idx2 = idxPtr[1];

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects[idx]->tms->maps[idx].mapSizeHA) {
			luaopen_TilemapRow(L, idx, idx2, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("w", k)) {
			lua_pushinteger(L, projects[idx]->tms->maps[idx2].mapSizeW);
			return 1;
		} else if (!strcmp("h", k)) {
			lua_pushinteger(L, projects[idx]->tms->maps[idx2].mapSizeH);
			return 1;
		} else if (!strcmp("hAll", k)) {
			lua_pushinteger(L, projects[idx]->tms->maps[idx2].mapSizeHA);
			return 1;
		} else if (!strcmp("useBlocks", k)) {
			lua_pushboolean(L, projects[idx]->tms->maps[idx2].isBlock);
			return 1;
		} else if (!strcmp("name", k)) {
			lua_pushstring(L, projects[idx]->tms->planeName[idx2].c_str());
			return 1;
		} else if (!strcmp("dither", k)) {
			lua_pushcfunction(L, &lua_tilemap_dither);
			return 1;
		} else if (!strcmp("loadImage", k)) {
			lua_pushcfunction(L, &lua_tilemap_loadImage);
			return 1;
		} else if (!strcmp("imageToTiles", k)) {
			lua_pushcfunction(L, &lua_tilemap_imageToTiles);
			return 1;
		} else if (!strcmp("remove", k)) {
			lua_pushcfunction(L, &lua_tilemaps_removePlane);
			return 1;
		}
	}

	return 0;
}

static int tilemap___tostring(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	lua_pushfstring(L, "Tilemap: %d from project: %d", idxPtr[0], idxPtr[1]);
	return 1;
}

static int tilemap__len_(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t tilemapIDX = idxPtr[2];

	lua_pushinteger(L, projects[projectIDX]->tms->maps[tilemapIDX].mapSizeHA);
	return 1;
}

static const struct luaL_Reg tilemap_member_methods[] = {
	{ "__index", tilemap__get_       },
	{ "__len", tilemap__len_       },
	{ "__tostring", tilemap___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_Tilemap(lua_State *L, size_t projectIDX, size_t tilemapIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "tilemap");
	// <mt>

	// register member methods
	dub::fregister(L, tilemap_member_methods);
	dub::setup(L, "tilemap");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	idxUserData[0] = projectIDX;
	idxUserData[1] = tilemapIDX;

	luaL_getmetatable(L, "tilemap");
	lua_setmetatable(L, -2);
	return 1;
}
