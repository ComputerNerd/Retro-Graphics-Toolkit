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

#include "luaTile.hpp"
#include "luaTileRGBArow.hpp"
#include "luaTilePixels.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "CIE.h"

static unsigned inRangeTile(unsigned tile, size_t projectIDX) {
	if (tile >= projects->at(projectIDX).tileC->amount()) {
		outofBoundsAlert("tile", tile);
		return 0;
	}

	return 1;
}
static int lua_tile_setTileRGBA(lua_State*L) {
	getProjectRef
	const size_t tile = idxPtr[1];

	if (inRangeTile(tile, projectIDX)) {
		uint8_t*tptr = ((uint8_t*)prj.tileC->truetDat.data() + (tile * prj.tileC->tcSize));
		unsigned len = lua_rawlen(L, 2);

		if (!len) {
			fl_alert("setTileRGBA error: parameter 2 must be a table");
			return 0;
		}

		fillucharFromTab(L, 2, len, prj.tileC->tcSize, tptr);
	}

	return 0;
}

static int lua_tile_compareTileRGBA(lua_State*L) {
	getProjectRef
	const size_t tile1 = idxPtr[1];
	unsigned tile2 = luaL_optinteger(L, 2, 0);

	if (inRangeTile(tile1, projectIDX) && inRangeTile(tile2, projectIDX) && (tile1 != tile2)) {
		unsigned diffSum = 0;
		uint8_t*off1 = prj.tileC->truetDat.data() + (tile1 * prj.tileC->tcSize);
		uint8_t*off2 = prj.tileC->truetDat.data() + (tile2 * prj.tileC->tcSize);

		for (unsigned i = 0; i < prj.tileC->tcSize; i += 4) {
			int tmp = 0;

			for (unsigned j = 0; j < 3; ++j)
				tmp += *off1++ -*off2++;

			++off1;
			++off2;
			diffSum += tmp * tmp;
		}

		lua_pushinteger(L, diffSum);
		return 1;
	}

	return 0;
}
static int lua_tile_dither(lua_State*L) {
	getIdxPtrChk
	size_t projectIDX = *idxPtr;
	size_t tile = idxPtr[1];

	unsigned row = luaL_optinteger(L, 2, 0);
	bool useAlt = luaL_optboolean(L, 3, false);

	if (inRangeTile(tile, projectIDX))
		projects->at(projectIDX).tileC->truecolor_to_tile(row - 1, tile, useAlt);

	return 0;
}

static int lua_tile_draw(lua_State*L) {
	getIdxPtrChk
	size_t projectIDX = *idxPtr;
	size_t tile = idxPtr[1];

	projects->at(projectIDX).tileC->draw_tile(luaL_optinteger(L, 2, 0), // X
	        luaL_optinteger(L, 3, 0), // Y
	        tile,
	        luaL_optinteger(L, 4, 0), // Zoom
	        luaL_optinteger(L, 5, 0), // Palette row
	        lua_toboolean(L, 6), // Hflip
	        lua_toboolean(L, 7), // Vflip
	        lua_toboolean(L, 8), // Is sprite?
	        lua_toboolean(L, 9)); // Draw with alpha?
	return 0;
}

static int lua_tile_remove(lua_State*L) {
	getIdxPtrChk
	size_t projectIDX = *idxPtr;
	size_t tileIDX = idxPtr[1];

	projects->at(projectIDX).tileC->remove_tile_at(tileIDX);
	return 0;
}

static int lua_tile_toPlanar(lua_State*L) {
	getProjectRef
	size_t tileIDX = idxPtr[1];

	prj.tileC->toPlanar((tileType)luaL_checkinteger(L, 2), tileIDX, tileIDX + 1);
	return 0;
}

static int lua_tile_convertToRGBA(lua_State*L) {
	getProjectRef
	size_t tileIDX = idxPtr[1];

	prj.tileC->tileToTrueCol(&prj.tileC->tDat.at(tileIDX * prj.tileC->tileSize), (uint8_t*)prj.tileC->getPixelPtrTC(tileIDX, 0, 0), luaL_checkinteger(L, 2) - 1, true, luaL_checkboolean(L, 3));
	return 0;
}

void rgbaToGrayscale(std::vector<uint8_t>& finalTile, const uint8_t*tilePtr, unsigned backgroundValue, const tiles* tileC) {
	finalTile.reserve(tileC->tileSize);
	for (unsigned y = 0; y < tileC->width(); ++y) {
		for (unsigned x = 0; x < tileC->height(); ++x) {
			double L,a,b;
			Rgb2Lab255(&L, &a, &b, tilePtr[0], tilePtr[1], tilePtr[2]);
			a = 0.0;
			b = 0.0;
			uint8_t redVal, greenUnused, blueUnused; // r, g and b will all be the same because a and b are zero.
			Lab2Rgb255(&redVal, &greenUnused, &blueUnused, L, a, b);
			
			unsigned alphaVal = tilePtr[3];
			unsigned v = ((redVal * alphaVal) + (backgroundValue * (255 - alphaVal))) / 255;
			finalTile.emplace_back(v);
			tilePtr += 4;
		}
	}
}

static int lua_tile_asGrayscale(lua_State*L) {
	getProjectRef
	size_t tileIDX = idxPtr[1];

	unsigned backgroundValue = luaL_checkinteger(L, 2);

	std::vector<uint8_t> finalTile;

	const uint8_t* tilePtr = (const uint8_t*)prj.tileC->getPixelPtrTC(tileIDX, 0, 0);
	rgbaToGrayscale(finalTile, tilePtr, backgroundValue, prj.tileC);

	lua_pushlstring(L, (const char*)finalTile.data(), finalTile.size());
	return 1;
}

static int tile__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectRef

	size_t tileIDX = idxPtr[1];

	if (!strcmp(key, "data"))
		luaStringToVector(L, 3, prj.tileC->tDat, prj.tileC->tileSize, true, tileIDX * prj.tileC->tileSize);

	else if (!strcmp(key, "rgbaData"))
		luaStringToVector(L, 3, prj.tileC->truetDat, prj.tileC->tcSize, true, tileIDX * prj.tileC->tcSize);

	return 0;
}

static int tile__get_(lua_State *L) {
	checkAlreadyExists
	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		getProjectRef
		size_t tileIDX = idxPtr[1];

		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("rgba", k)) {
			luaopen_TileRGBArow(L, projectIDX, tileIDX);
			return 1;
		} else if (!strcmp("pixels", k)) {
			luaopen_TilePixels(L, projectIDX, tileIDX);
			return 1;
		} else if (!strcmp("data", k)) {
			lua_pushlstring(L, (const char*)&prj.tileC->tDat.at(tileIDX * prj.tileC->tileSize), prj.tileC->tileSize);
			return 1;
		} else if (!strcmp("rgbaData", k)) {
			lua_pushlstring(L, (const char*)&prj.tileC->truetDat.at(tileIDX * prj.tileC->tcSize), prj.tileC->tcSize);
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
	{ "__newindex", tile__set_},
	{ "__index", tile__get_},
	{ "__tostring", tile___tostring},
	{ "compareTileRGBA", lua_tile_compareTileRGBA},
	{ "setTileRGBA", lua_tile_setTileRGBA},
	{ "dither", lua_tile_dither},
	{ "draw", lua_tile_draw},
	{ "remove", lua_tile_remove},
	{ "toPlanar", lua_tile_toPlanar},
	{ "convertToRGBA", lua_tile_convertToRGBA},
	{ "asGrayscale", lua_tile_asGrayscale},
	{ "deleted", dub::isDeleted},
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
