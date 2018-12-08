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
#include <algorithm>
#include <cstring>
#include <FL/fl_ask.H>
#include "luaHelpers.hpp"
void mkKeyunsigned(lua_State*L, const char*str, unsigned val) {
	lua_pushstring(L, str);
	lua_pushinteger(L, val);
	lua_rawset(L, -3);
}
void mkKeyint(lua_State*L, const char*str, int val) {
	lua_pushstring(L, str);
	lua_pushinteger(L, val);
	lua_rawset(L, -3);
}
void mkKeybool(lua_State*L, const char*str, bool val) {
	lua_pushstring(L, str);
	lua_pushboolean(L, val);
	lua_rawset(L, -3);
}
size_t getSizeTUserData(lua_State*L) {
	const size_t *idxPtr = (const size_t*)lua_touserdata(L, 1);
	return *idxPtr;
}
bool luaL_optboolean (lua_State *L, int narg, bool def) {
	return lua_isboolean(L, narg) ? lua_toboolean(L, narg) : def;
}
void fillucharFromTab(lua_State*L, unsigned index, unsigned len, unsigned sz, uint8_t*ptr) { //len amount in table sz expected size
	unsigned to = std::min(len, sz);

	for (unsigned i = 1; i <= to; ++i) {
		lua_rawgeti(L, index, i);
		int tmp = lua_tointeger(L, -1);

		if (tmp < 0)
			tmp = 0;

		if (tmp > 255)
			tmp = 255;

		*ptr++ = tmp;
		lua_pop(L, 1);
	}

	if (sz > len)
		std::memset(ptr, 0, sz - len);
}
void outofBoundsAlert(const char*what, unsigned val) {
	fl_alert("Error tried to access out of bound %s %u", what, val);
}
void noUserDataError() {
	fl_alert("Lua code error: No user data");
}
