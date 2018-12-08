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
#include "luaChunkEntry.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"
#include "classChunks.h"


static int chunkEntry__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	getIdxPtrChk
	const size_t projectIDX = idxPtr[0];
	const size_t chunkIDX = idxPtr[1];
	const size_t columnIDX = idxPtr[2];
	const size_t entryIDX = idxPtr[3];
	class ChunkClass *chunk = projects[projectIDX]->Chunk;

	const char*k = luaL_checkstring(L, 2);

	if (!strcmp("hflip", k))
		chunk->setHflip(chunkIDX, entryIDX, columnIDX, lua_toboolean(L, 3));
	else if (!strcmp("vflip", k))
		chunk->setVflip(chunkIDX, entryIDX, columnIDX, lua_toboolean(L, 3));
	else if (!strcmp("priority", k))
		chunk->setPrio(chunkIDX, entryIDX, columnIDX, lua_toboolean(L, 3));
	else if (!strcmp("block", k))
		chunk->setBlock(chunkIDX, entryIDX, columnIDX, lua_tointeger(L, 3));
	else if (!strcmp("flag", k))
		chunk->setFlag(chunkIDX, entryIDX, columnIDX, lua_tointeger(L, 3));

	return 0;
}

static int lua_chunk_getTileRow(lua_State*L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	size_t projectIDX = *idxPtr;
	size_t entryIDX = idxPtr[1];
	lua_pushinteger(L, projects[projectIDX]->tms->maps[entryIDX].get_tileRow(luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0), luaL_optinteger(L, 4, 0)));
	return 1;
}

static int chunkEntry__get_(lua_State *L) {
	int type = lua_type(L, 2);

	if (type == LUA_TSTRING) {
		const char *key = luaL_checkstring(L, 2);
		getIdxPtrChk
		const size_t projectIDX = idxPtr[0];
		const size_t chunkIDX = idxPtr[1];
		const size_t columnIDX = idxPtr[2];
		const size_t entryIDX = idxPtr[3];
		const class ChunkClass *chunk = projects[projectIDX]->Chunk;

		const char*k = luaL_checkstring(L, 2);

		if (!strcmp("hflip", k)) {
			lua_pushboolean(L, chunk->getHflip(chunkIDX, entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("vflip", k)) {
			lua_pushboolean(L, chunk->getVflip(chunkIDX, entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("priority", k)) {
			lua_pushboolean(L, chunk->getPrio(chunkIDX, entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("block", k)) {
			lua_pushinteger(L, chunk->getBlock(chunkIDX, entryIDX, columnIDX));
			return 1;
		} else if (!strcmp("flag", k)) {
			lua_pushinteger(L, chunk->getFlag(chunkIDX, entryIDX, columnIDX));
			return 1;
		}

	}

	return 0;
}

static int chunkEntry___tostring(lua_State *L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	lua_pushfstring(L, "Chunk Entry (%d, %d) from chunk %d project %d", idxPtr[2], idxPtr[3], idxPtr[1], idxPtr[0]);
	return 1;
}
static const struct luaL_Reg chunkEntry_member_methods[] = {
	{ "__newindex", chunkEntry__set_       },
	{ "__index", chunkEntry__get_       },
	{ "__tostring", chunkEntry___tostring  },
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_ChunkEntry(lua_State *L, size_t projectIDX, size_t chunkIDX, size_t columnIDX, size_t entryIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "chunkEntry");
	// <mt>

	// register member methods
	dub::fregister(L, chunkEntry_member_methods);
	dub::setup(L, "chunkEntry");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 4);
	idxUserData[0] = projectIDX;
	idxUserData[1] = chunkIDX;
	idxUserData[2] = columnIDX;
	idxUserData[3] = entryIDX;

	luaL_getmetatable(L, "chunkEntry");
	lua_setmetatable(L, -2);
	return 1;
}
