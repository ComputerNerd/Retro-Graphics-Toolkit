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
#include "includes.h"
#include "gui.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
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
void runLua(Fl_Widget*,void*){
	std::string scriptname;
	if(loadsavefile(scriptname,"Select a lua script")){
		lua_State *L = lua_newstate(l_alloc, NULL);
  		if(L){
			lua_atpanic(L, &panic);
			luaL_openlibs(L);
			try{
				int s = luaL_loadfile(L, scriptname.c_str());
				if(s){
					if (s != LUA_OK && !lua_isnil(L, -1)) {
						const char *msg = lua_tostring(L, -1);
						if (msg == NULL) msg = "(error object is not a string)";
						fl_alert(msg);
						lua_pop(L, 1);
						/* force a complete garbage collection in case of errors */
						lua_gc(L, LUA_GCCOLLECT, 0);
					}
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
				fl_alert("Lua error while running script");
			}
			lua_close(L);
		}else
			fl_alert("lua_newstate failed");
	}
}
