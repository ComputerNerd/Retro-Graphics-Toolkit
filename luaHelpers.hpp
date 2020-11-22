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
#ifndef LUA_HELPERS_HPP
#define LUA_HELPERS_HPP
#include <vector>
#include <stdint.h>
#include "lua.hpp"

#define arLen(ar) (sizeof(ar)/sizeof(ar[0]))

struct keyPair {
	const char*key;
	unsigned pair;
};
struct keyPairi {
	const char*key;
	int pair;
};

void mkKeyunsigned(lua_State*L, const char*str, unsigned val);
void mkKeyint(lua_State*L, const char*str, int val);
void mkKeybool(lua_State*L, const char*str, bool val);
size_t getSizeTUserData(lua_State*L);
bool luaL_optboolean (lua_State *L, int narg, bool def);
void fillucharFromTab(lua_State*L, unsigned index, unsigned len, unsigned sz, uint8_t*ptr);
void outofBoundsAlert(const char*what, unsigned val);
void noUserDataError();
void luaStringToVector(lua_State*L, int index, std::vector<uint8_t>&v, unsigned sizeReq, bool isExactSize, int inplaceIdx);

#define checkAlreadyExists lua_getmetatable(L, 1); \
lua_pushvalue(L, 2); \
lua_rawget(L, -2); \
if (!lua_isnil(L, -1)) { \
	return 1; \
} else { \
	lua_pop(L, 2); \
}

#define getIdxPtrChk const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1); \
if(!idxPtr) { \
	noUserDataError(); \
	return 0; \
}

#define getProjectIDX getIdxPtrChk \
const size_t projectIDX = *idxPtr;

#define getProjectRef getIdxPtrChk \
const size_t projectIDX = *idxPtr; \
Project&prj = projects->at(projectIDX);
#endif
