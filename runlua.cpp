/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
*/
#include <string>
#include <FL/Fl_Color_Chooser.H>
#include "includes.h"
#include "gui.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
static int panic(lua_State *L){
	fl_alert("PANIC: unprotected error in call to Lua API (%s)\n",lua_tostring(L, -1));
	throw 0;//Otherwise abort() would be called when not needed
}
static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize){
	if(nsize)
		return realloc(ptr, nsize);
	else{
		free(ptr);
		return 0;
	}
}
static int luafl_alert(lua_State*L){
	fl_alert(luaL_optstring(L,1,"Default message"));
	return 0;
}
static int luafl_ask(lua_State*L){
	lua_pushinteger(L,fl_ask(luaL_optstring(L,1,"Default message")));
	return 1;
}
static int luafl_beep(lua_State*L){
	fl_beep();
	return 0;
}
static int luafl_choice(lua_State*L){
	lua_pushinteger(L,fl_choice(luaL_optstring(L,1,"Default message"),luaL_optstring(L,2,NULL),luaL_optstring(L,3,NULL),luaL_optstring(L,4,NULL)));
	return 1;
}
static int luafl_color_chooser(lua_State*L){
	double r,g,b;
	r=luaL_optnumber(L,3,0.0);
	g=luaL_optnumber(L,4,0.0);
	b=luaL_optnumber(L,5,0.0);
	int ret=fl_color_chooser(luaL_optstring(L,1,"Select a color"),r,g,b,luaL_optint(L,2,-1));
	if(ret){
		lua_pushinteger(L,ret);
		lua_pushnumber(L,r);
		lua_pushnumber(L,g);
		lua_pushnumber(L,b);
		return 4;
	}else{
		lua_pushinteger(L,ret);
		return 1;
	}
}
static int luafl_dir_chooser(lua_State*L){
	lua_pushstring(L,fl_dir_chooser(luaL_optstring(L,1,"Choose a directory"),luaL_optstring(L,2,NULL),luaL_optint(L,3,0)));
	return 1;
}
static int luafl_file_chooser(lua_State*L){
	lua_pushstring(L,fl_file_chooser(luaL_optstring(L,1,"Choose a file"),luaL_optstring(L,2,NULL),luaL_optstring(L,3,0),luaL_optint(L,4,0)));
	return 1;
}
static int luafl_input(lua_State*L){
	lua_pushstring(L,fl_input(luaL_optstring(L,1,"Enter text"),luaL_optstring(L,2,NULL)));
	return 1;
}
static int luafl_message(lua_State*L){
	fl_message(luaL_optstring(L,1,"Default message"));
	return 0;
}
static int luafl_password(lua_State*L){
	fl_password(luaL_optstring(L,1,"Enter text"),luaL_optstring(L,2,NULL));
	return 0;
}
static const luaL_Reg lua_flAPI[]={
	{"alert",luafl_alert},
	{"ask",luafl_ask},
	{"beep",luafl_beep},
	{"choice",luafl_choice},
	{"color_chooser",luafl_color_chooser},
	{"dir_chooser",luafl_dir_chooser},
	{"file_chooser",luafl_file_chooser},
	{"input",luafl_input},
	{"message",luafl_message},
	{"password",luafl_password},
	{0,0}
};
void runLua(Fl_Widget*,void*){
	std::string scriptname;
	if(loadsavefile(scriptname,"Select a lua script")){
		lua_State *L = lua_newstate(l_alloc, NULL);
  		if(L){
			lua_atpanic(L, &panic);
			try{
				luaL_openlibs(L);
				luaL_newlib(L,lua_flAPI);
				lua_setglobal(L, "fl");
				int s = luaL_loadfile(L, scriptname.c_str());
				if(s != LUA_OK && !lua_isnil(L, -1)){
					const char *msg = lua_tostring(L, -1);
					if (msg == NULL) msg = "(error object is not a string)";
					fl_alert(msg);
					lua_pop(L, 1);
				}else{
					// execute Lua program
					s = lua_pcall(L, 0, LUA_MULTRET, 0);
					if (s != LUA_OK){
						const char *msg = (lua_type(L, -1) == LUA_TSTRING) ? lua_tostring(L, -1)
							: NULL;
						if (msg == NULL) msg = "(error object is not a string)";
						fl_alert(msg);
						lua_pop(L, 1);
					}
				}
			}catch(...){
				fl_alert("Lua error while running script\nthrow was called");
			}
			lua_close(L);
		}else
			fl_alert("lua_newstate failed");
	}
}
