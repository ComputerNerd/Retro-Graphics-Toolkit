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
#pragma once
#include <vector>
#include <cstdint>
#include "lua.hpp"
#include <FL/Fl_Widget.H>
void updateProjectTablesLua(lua_State*L);
void runLuaFunc(lua_State*L, unsigned args, unsigned results);
void runLua(lua_State*L, const char*str, bool isFile = true);
lua_State*createLuaState(void);
void runLuaCB(Fl_Widget*, void*);
bool luaL_checkboolean(lua_State* L, int n);
void registerProjectTables(lua_State*L);
void setProjectConstants(lua_State*L);
void tableToVector(lua_State*L, unsigned idx, std::vector<uint8_t>&vu8);
void runLuaCD(const char*fname);
