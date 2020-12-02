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
#include <FL/fl_ask.H>

#include "luaTilemap.hpp"
#include "luaTilemapRow.hpp"
#include "luaTilemapBlocks.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "callback_tilemap.h"
#include "class_global.h" // For window.
#include "errorMsg.h"
extern "C" {
#include "compat-5.3.h"
}

static int lua_tilemap_dither(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	unsigned method = luaL_optinteger(L, 2, 1);

	projects->at(projectIDX).tms->maps[tilemapIDX].ditherAsImage(method);
	return 0;
}

static int lua_tilemap_loadImage(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	load_image_to_tilemap_project_ptr(&projects->at(projectIDX), lua_tostring(L, 2), luaL_optboolean(L, 3, false), luaL_optboolean(L, 4, false), luaL_optboolean(L, 5, false), tilemapIDX);
	return 0;
}

static int lua_tilemap_imageToTiles(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	unsigned len = lua_rawlen(L, 2);

	if (!len) {
		fl_alert("imageToTiles error: parameter 1 must be a table");
		return 0;
	}

	int row = luaL_optinteger(L, 3, -1);
	bool useAlpha = luaL_optboolean(L, 4, false);
	bool copyToTruecol = luaL_optboolean(L, 5, false);
	bool convert = luaL_optboolean(L, 6, false);
	unsigned bpp = useAlpha + 3;
	uint32_t w, h;
	w = projects->at(projectIDX).tms->maps[tilemapIDX].mapSizeW * projects->at(projectIDX).tileC->width();
	h = projects->at(projectIDX).tms->maps[tilemapIDX].mapSizeHA * projects->at(projectIDX).tileC->height();
	unsigned sz = w * h * bpp;
	uint8_t*image = (uint8_t*)malloc(sz);
	fillucharFromTab(L, 2, len, sz, image);
	projects->at(projectIDX).tms->maps[tilemapIDX].truecolorimageToTiles(image, row, useAlpha, copyToTruecol, convert);
	free(image);
	return 0;
}

static int lua_tilemaps_removePlane(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	projects->at(projectIDX).tms->removePlane(tilemapIDX);
	return 0;
}

static int lua_tilemaps_resize(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	projects->at(projectIDX).tms->maps[tilemapIDX].resize_tile_map(luaL_optinteger(L, 2, 1), luaL_optinteger(L, 3, 1));
	return 0;
}
static void updateMapAmt(size_t projectIDX, size_t tilemapIDX) {
	if (tilemapIDX == projects->at(projectIDX).curPlane && curProjectID == projectIDX && window)
		window->map_amt->value(std::to_string(projects->at(projectIDX).tms->maps[tilemapIDX].amt).c_str());
}

static int lua_tilemaps_setBlocksAmt(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	projects->at(projectIDX).tms->maps[tilemapIDX].blockAmt(luaL_optinteger(L, 2, 1));

	updateMapAmt(projectIDX, tilemapIDX);
	return 0;
}

static int lua_tilemaps_removeBlock(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	projects->at(projectIDX).tms->maps[tilemapIDX].removeBlock(luaL_optinteger(L, 2, 0));

	updateMapAmt(projectIDX, tilemapIDX);
	return 0;
}

static int lua_tilemaps_setBlocksEnabled(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	projects->at(projectIDX).tms->maps[tilemapIDX].toggleBlocks(lua_toboolean(L, 2));

	updateMapAmt(projectIDX, tilemapIDX);
	return 0;
}

static int lua_tilemap_drawBlock(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];
	projects->at(projectIDX).tms->maps[tilemapIDX].drawBlock(luaL_optinteger(L, 2, 1) - 1, luaL_optinteger(L, 3, 0), luaL_optinteger(L, 4, 0), luaL_optinteger(L, 5, 0), luaL_optinteger(L, 6, 0));
	return 0;
}
static int lua_tilemap_subTile(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];
	projects->at(projectIDX).tms->maps[tilemapIDX].sub_tile_map(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_toboolean(L, 4), lua_toboolean(L, 5));
	return 0;
}

static int lua_tilemap_save(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];
	projects->at(projectIDX).tms->maps[tilemapIDX].saveToFile(lua_tostring(L, 2), (fileType_t)lua_tointeger(L, 3), lua_toboolean(L, 4), (CompressionType)lua_tointeger(L, 5), lua_tostring(L, 6), luaL_optstring(L, 7, nullptr), luaL_optstring(L, 8, nullptr));
	return 0;
}
static int lua_tilemap_pickRowDelta(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];
	projects->at(projectIDX).tms->maps[tilemapIDX].pickRowDelta(false, nullptr, lua_tointeger(L, 2), lua_tointeger(L, 3));
	return 0;
}
static int lua_tilemap_pickRow(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];
	projects->at(projectIDX).tms->maps[tilemapIDX].pickRow(lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
	return 0;
}
static int lua_tilemap_allToRow(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];
	projects->at(projectIDX).tms->maps[tilemapIDX].allRowSet(luaL_optinteger(L, 2, 1) - 1);
	return 0;
}

static int lua_tilemap_toImage(lua_State*L) {
	getProjectIDX
	size_t tilemapIDX = idxPtr[1];

	int row = luaL_optinteger(L, 2, -1);
	bool useAlpha = luaL_optboolean(L, 3, false);
	uint32_t w, h;
	w = projects->at(projectIDX).tms->maps[tilemapIDX].mapSizeW * projects->at(projectIDX).tileC->width();
	h = projects->at(projectIDX).tms->maps[tilemapIDX].mapSizeHA * projects->at(projectIDX).tileC->height();
	unsigned bpp = useAlpha + 3;
	uint8_t*image = (uint8_t *)malloc(w * h * bpp);

	if (!image) {
		show_malloc_error(w * h * bpp)
		return 0;
	}

	projects->at(projectIDX).tms->maps[tilemapIDX].truecolor_to_image(image, row, useAlpha);
	uint8_t*imgptr = image;
	lua_newtable(L);

	for (unsigned i = 1; i <= w * h * bpp; ++i) {
		lua_pushinteger(L, *imgptr++);
		lua_rawseti(L, -2, i);
	}

	free(image);
	return 1;
}

static int tilemap__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectRef
	size_t idx2 = idxPtr[1];

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < prj.tms->maps[idx2].mapSizeHA) {
			luaopen_TilemapRow(L, projectIDX, idx2, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("width", k)) {
			lua_pushinteger(L, prj.tms->maps[idx2].mapSizeW);
			return 1;
		} else if (!strcmp("height", k)) {
			lua_pushinteger(L, prj.tms->maps[idx2].mapSizeH);
			return 1;
		} else if (!strcmp("hAll", k)) {
			lua_pushinteger(L, prj.tms->maps[idx2].mapSizeHA);
			return 1;
		} else if (!strcmp("useBlocks", k)) {
			lua_pushboolean(L, prj.tms->maps[idx2].isBlock);
			return 1;
		} else if (!strcmp("name", k)) {
			lua_pushstring(L, prj.tms->maps[idx2].planeName.c_str());
			return 1;
		} else if (!strcmp("blocks", k)) {
			if (!prj.tms->maps[idx2].isBlock)
				luaL_error(L, "Blocks must be enabled to use this.");

			luaopen_TilemapBlocks(L, projectIDX, idx2);
			return 1;
		}
	}

	return 0;
}

static int tilemap___tostring(lua_State *L) {
	getIdxPtrChk
	lua_pushfstring(L, "Tilemap: %d from project: %d", idxPtr[0], idxPtr[1]);
	return 1;
}

static int tilemap__len_(lua_State *L) {
	getProjectRef
	size_t tilemapIDX = idxPtr[1];

	lua_pushinteger(L, prj.tms->maps[tilemapIDX].mapSizeHA);
	return 1;
}

static int tilemap__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectIDX
	const size_t tilemapIDX = idxPtr[1];
	class tileMap *tm = &projects->at(projectIDX).tms->maps[tilemapIDX];

	const char*k = luaL_checkstring(L, 2);

	if (!strcmp("useBlocks", k))
		tm->isBlock = lua_toboolean(L, 3);

	return 0;
}

static const struct luaL_Reg tilemap_member_methods[] = {
	{ "__newindex", tilemap__set_       },
	{ "__index", tilemap__get_       },
	{ "__len", tilemap__len_       },
	{ "__tostring", tilemap___tostring  },
	{ "allToRow", lua_tilemap_allToRow},
	{ "drawBlock", lua_tilemap_drawBlock},
	{ "pickRow", lua_tilemap_pickRow},
	{ "pickRowDelta", lua_tilemap_pickRowDelta},
	{ "save", lua_tilemap_save},
	{ "subTile", lua_tilemap_subTile},
	{ "toImage", lua_tilemap_toImage},
	{ "dither", lua_tilemap_dither},
	{ "loadImage", lua_tilemap_loadImage},
	{ "imageToTiles", lua_tilemap_imageToTiles},
	{ "remove", lua_tilemaps_removePlane},
	{ "resize", lua_tilemaps_resize},
	{ "setBlocksAmt", lua_tilemaps_setBlocksAmt},
	{ "removeBlock", lua_tilemaps_removeBlock},
	{ "setBlocksEnabled", lua_tilemaps_setBlocksEnabled},
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
