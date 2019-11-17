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
#include "luaChunk.hpp"
#include "luaChunkRow.hpp"
#include "luaHelpers.hpp"
#include "project.h"
#include "dub/dub.h"



static int lua_chunk_draw(lua_State*L) {
	getProjectIDX
	size_t chunk = idxPtr[1];

	projects->at(projectIDX).Chunk->drawChunk(chunk,
	                                      luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0), // X offset, Y offset.
	                                      luaL_optinteger(L, 4, 1), // Zoom
	                                      luaL_optinteger(L, 5, 0), luaL_optinteger(L, 6, 0) // Scroll X, scroll Y
	                                     );
	return 0;
}

static int lua_chunk_remove(lua_State*L) {
	getProjectIDX
	size_t chunkIDX = idxPtr[1];

	projects->at(projectIDX).Chunk->removeAt(chunkIDX);
	return 0;
}

static int chunk__get_(lua_State *L) {
	checkAlreadyExists
	int type = lua_type(L, 2);

	getProjectIDX

	size_t chunkIDX = idxPtr[1];

	if (type == LUA_TNUMBER) {
		int k = luaL_checkinteger(L, 2) - 1;

		if (k >= 0 && k < projects->at(projectIDX).Chunk->amt) {
			luaopen_ChunkRow(L, projectIDX, chunkIDX, k);
			return 1;
		}
	}
}

static int chunk___tostring(lua_State * L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);

	if (idxPtr) {
		lua_pushfstring(L, "Chunk: %d from project: %d", idxPtr[0], idxPtr[1]);
		return 1;
	} else
		return 0;
}

static int chunk__len(lua_State *L) {
	getProjectIDX
	lua_pushinteger(L, projects->at(projectIDX).Chunk->hi);
	return 1;
}

static const struct luaL_Reg chunk_member_methods[] = {
	{ "__index", chunk__get_       },
	{ "__tostring", chunk___tostring  },
	{ "__len", chunk__len  },
	{ "draw", lua_chunk_draw},
	{ "remove", lua_chunk_remove},
	{ "deleted", dub::isDeleted    },
	{ NULL, NULL},
};

int luaopen_Chunk(lua_State * L, size_t projectIDX, size_t chunkIDX) {
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "chunk");
	// <mt>

	// register member methods
	dub::fregister(L, chunk_member_methods);
	dub::setup(L, "chunk");
	// setup meta-table

	size_t* idxUserData = (size_t*)lua_newuserdata(L, sizeof(size_t) * 2);
	idxUserData[0] = projectIDX;
	idxUserData[1] = chunkIDX;

	luaL_getmetatable(L, "chunk");
	lua_setmetatable(L, -2);
	return 1;
}
