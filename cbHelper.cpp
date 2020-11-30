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
#include "runlua.h"
#include "cbHelper.h"
#include <stdexcept>
extern "C" {
#include "compat-5.3.h"
}

void luaWidgetCallbackHelper(Fl_Widget*, void*i) {
	struct cbInfo*c = (struct cbInfo*)i;

	if (c->cb) {
		if (lua_getglobal(c->L, c->cb) != LUA_TFUNCTION)
			throw std::invalid_argument("This is not a function.");

		lua_pushinteger(c->L, c->udat);
		runLuaFunc(c->L, 1, 0);
	}
}
