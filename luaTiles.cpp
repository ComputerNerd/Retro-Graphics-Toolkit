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
#include "luaTiles.hpp"
#include "luaTile.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"

static int lua_tiles_save(lua_State*L) {
	getProjectIDX
	//void save(const char*fname,fileType_t type,bool clipboard,int compression);
	projects->at(projectIDX).tileC->save(lua_tostring(L, 2), (fileType_t)lua_tointeger(L, 3), lua_toboolean(L, 4), (CompressionType)lua_tointeger(L, 5), lua_tostring(L, 6));
	return 0;
}

static int lua_tiles_saveExtAttrs(lua_State*L) {
	getProjectIDX
	projects->at(projectIDX).tileC->saveExtAttrs(lua_tostring(L, 2), (fileType_t)lua_tointeger(L, 3), lua_toboolean(L, 4), (CompressionType)lua_tointeger(L, 5), lua_tostring(L, 6));
	return 0;
}

static int lua_tiles_removeDuplicate(lua_State*L) {
	getProjectIDX
	projects->at(projectIDX).tileC->remove_duplicate_tiles(lua_toboolean(L, 2));
	return 0;
}

static int lua_tiles_append(lua_State*L) {
	getProjectIDX
	projects->at(projectIDX).tileC->appendTile(luaL_optinteger(L, 2, 1));

	if (curProjectID == projectIDX)
		updateTileSelectAmt();

	return 0;
}

static int lua_tiles_resize(lua_State*L) {
	getProjectIDX
	projects->at(projectIDX).tileC->resizeAmt(luaL_optinteger(L, 2, 1));

	if (curProjectID == projectIDX)
		updateTileSelectAmt();

	return 0;
}

static int lua_tiles_toPlanar(lua_State*L) {
	getProjectRef
	size_t tileIDX = idxPtr[1];

	prj.tileC->toPlanar((tileType)luaL_checkinteger(L, 2), luaL_optinteger(L, 3, 1) - 1, luaL_optinteger(L, 4, prj.tileC->amount()));
	return 0;
}

static int lua_tiles_assignData(lua_State*L) {
	getProjectRef
	tileType tt = (tileType)luaL_checkinteger(L, 2);
	unsigned firstTile = luaL_checkinteger(L, 3) - 1;
	unsigned offset = firstTile * prj.tileC->tileSize;

	size_t len = luaStringToVector(L, 4, prj.tileC->tDat, prj.tileC->tileSize, false, offset);
	prj.tileC->truetDat.resize(prj.tileC->amount() * prj.tileC->tcSize);
	size_t nTiles = len / prj.tileC->tileSize;
	prj.tileC->toPlanar((tileType)tt, firstTile, firstTile + nTiles);

	if (curProjectID == projectIDX)
		updateTileSelectAmt();
}

static int lua_tiles_tms9918Mode1RearrangeActions(lua_State*L) {
	try {
		getProjectRef

		bool forceTileToAttribute = dub::checkboolean(L, 2);
		unsigned int tile = dub::checkinteger(L, 3);
		unsigned char attr = dub::checkinteger(L, 4);
		prj.tileC->tms9918Mode1RearrangeActions(forceTileToAttribute, tile, attr);
		return 0;
	} catch (std::exception &e) {
		lua_pushfstring(L, "tms9918Mode1RearrangeActions: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "tms9918Mode1RearrangeActions: Unknown exception");
	}

	return dub::error(L);
}

static int tiles__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectRef

	if (!strcmp(key, "data")) {
		luaStringToVector(L, 3, prj.tileC->tDat, prj.tileC->tileSize, false, -1);
		prj.tileC->resizeAmt(prj.tileC->amount());

	} else if (!strcmp(key, "rgbaData")) {
		luaStringToVector(L, 3, prj.tileC->truetDat, prj.tileC->tcSize, false, -1);
		prj.tileC->resizeAmt(prj.tileC->truetDat.size() / prj.tileC->tcSize);
	} else if (!strcmp(key, "extAttrs"))
		luaStringToVector(L, 3, prj.tileC->extAttrs, prj.tileC->extAttrs.size(), true, 0);


	return 0;
}

static int tiles__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectRef

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < prj.tileC->amount()) {
			luaopen_Tile(L, projectIDX, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("tileSize", k)) {
			lua_pushinteger(L, prj.tileC->tileSize);
			return 1;
		} else if (!strcmp("tcSize", k)) {
			lua_pushinteger(L, prj.tileC->tcSize);
			return 1;
		} else if (!strcmp("width", k)) {
			lua_pushinteger(L, prj.tileC->width());
			return 1;
		} else if (!strcmp("height", k)) {
			lua_pushinteger(L, prj.tileC->height());
			return 1;
		} else if (!strcmp("data", k)) {
			lua_pushlstring(L, (const char*)prj.tileC->tDat.data(), prj.tileC->tDat.size());
			return 1;
		} else if (!strcmp("rgbaData", k)) {
			lua_pushlstring(L, (const char*)prj.tileC->truetDat.data(), prj.tileC->truetDat.size());
			return 1;
		} else if (!strcmp("extAttrs", k)) {
			lua_pushlstring(L, (const char*)prj.tileC->extAttrs.data(), prj.tileC->extAttrs.size());
			return 1;
		}
	}

	return 0;
}

static int tiles__len_(lua_State *L) {
	getProjectIDX
	lua_pushinteger(L, projects->at(projectIDX).tileC->amount());
	return 1;
}

static int tiles___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "tiles table: %p", projects->at(projectIDX).tileC);
	return 1;
}

static const struct luaL_Reg tiles_member_methods[] = {
	{ "__newindex", tiles__set_},
	{ "__index", tiles__get_},
	{ "__len", tiles__len_},
	{ "__tostring", tiles___tostring},
	{ "deleted", dub::isDeleted},
	{ "save", lua_tiles_save},
	{ "saveExtAttrs", lua_tiles_saveExtAttrs},
	{ "removeDuplicate", lua_tiles_removeDuplicate},
	{ "append", lua_tiles_append},
	{ "setAmt", lua_tiles_resize},
	{ "toPlanar", lua_tiles_toPlanar},
	{ "assignData", lua_tiles_assignData},
	{ "tms9918Mode1RearrangeActions", lua_tiles_tms9918Mode1RearrangeActions},
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
