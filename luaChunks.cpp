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
#include "luaChunks.hpp"
#include "luaChunk.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "gui.h"

static int chunk__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getProjectIDX

	class ChunkClass *chunk = projects[projectIDX]->Chunk;

	const char*k = luaL_checkstring(L, 2);

	if (!strcmp("useBlocks", k))
		chunk->useBlocks = lua_toboolean(L, 3);
	else if (!strcmp("usePlane", k))
		chunk->useBlocks = lua_tointeger(L, 3) % chunk->amt;

	return 0;
}

static int lua_chunks_importSonic1(lua_State*L) {
	getProjectIDX
	projects[projectIDX]->Chunk->importSonic1(luaL_optboolean(L, 2, false));
	return 0;
}
static int lua_chunks_exportSonic1(lua_State*L) {
	getProjectIDX
	projects[projectIDX]->Chunk->exportSonic1();
	return 0;
}

static int lua_chunks_append(lua_State*L) {
	getProjectIDX
	projects[projectIDX]->Chunk->append();

	if (window && curProjectID == projectIDX)
		window->updateChunkSel();

	return 0;
}

static int lua_chunks_setAmt(lua_State*L) {
	getProjectIDX
	projects[projectIDX]->Chunk->resizeAmt(luaL_optinteger(L, 2, 1));

	if (window && curProjectID == projectIDX)
		window->updateChunkSel();

	return 0;
}

static int lua_chunks_setWH(lua_State*L) {
	getProjectIDX
	projects[projectIDX]->Chunk->resize(luaL_optinteger(L, 2, 1), luaL_optinteger(L, 3, 1));

	if (window && curProjectID == projectIDX)
		window->updateChunkSize();

	return 0;
}

static int lua_chunks_subBlock(lua_State*L) {
	getProjectIDX
	projects[projectIDX]->Chunk->subBlock(luaL_optinteger(L, 2, 1), luaL_optinteger(L, 3, 1));

	if (window && curProjectID == projectIDX)
		window->updateChunkSize();

	return 0;
}

static int chunks__get_(lua_State *L) {
	checkAlreadyExists

	int type = lua_type(L, 2);
	getProjectIDX

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects[projectIDX]->Chunk->amt) {
			luaopen_Chunk(L, projectIDX, k);
			return 1;
		}
	} else if (type == LUA_TSTRING) {
		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("useBlocks", k)) {
			lua_pushboolean(L, projects[projectIDX]->Chunk->useBlocks);
			return 1;
		} else if (!strcmp("width", k)) {
			lua_pushinteger(L, projects[projectIDX]->Chunk->wi);
			return 1;
		} else if (!strcmp("height", k)) {
			lua_pushinteger(L, projects[projectIDX]->Chunk->hi);
			return 1;
		} else if (!strcmp("usePlane", k)) {
			lua_pushinteger(L, projects[projectIDX]->Chunk->usePlane);
			return 1;
		}
	}

	return 0;
}

static int chunks__len_(lua_State *L) {
	getProjectIDX
	lua_pushinteger(L, projects[projectIDX]->Chunk->amt);
	return 1;
}

static int chunks___tostring(lua_State *L) {
	getProjectIDX
	lua_pushfstring(L, "Chunks table: %p", projects[projectIDX]->Chunk);
	return 1;
}

static const struct luaL_Reg chunks_member_methods[] = {
	{ "__newindex", chunk__set_       },
	{ "__index", chunks__get_       },
	{ "__len", chunks__len_       },
	{ "__tostring", chunks___tostring  },
	{ "deleted", dub::isDeleted    },
	{ "importSonic1", lua_chunks_importSonic1},
	{ "exportSonic1", lua_chunks_exportSonic1},
	{ "append", lua_chunks_append},
	{ "setAmt", lua_chunks_setAmt},
	{ "setWH", lua_chunks_setWH},
	{ "subBlock", lua_chunks_subBlock},
	{ NULL, NULL},
};

int luaopen_Chunks(lua_State *L, unsigned projectIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "chunks");
	// <mt>

	// register member methods
	dub::fregister(L, chunks_member_methods);
	dub::setup(L, "chunks");
	// setup meta-table
	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t));
	luaL_getmetatable(L, "chunks");
	*idxUserData = projectIDX;
	lua_setmetatable(L, -2);
	return 1;
}
