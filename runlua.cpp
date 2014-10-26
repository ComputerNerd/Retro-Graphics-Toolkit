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
#include <cmath>//Mingw workaround
#include <FL/Fl_File_Chooser.H>
#include <libgen.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "includes.h"
#include "gui.h"
#include "project.h"
#include "color_convert.h"
#include "callback_gui.h"
#include "callbacksprites.h"
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
static void outofBoundsAlert(const char*what,unsigned val){
	fl_alert("Error tried to access out of bound %s %u",what,val);
}
static unsigned inRangeEnt(unsigned ent){
	if(ent>=(currentProject->colorCnt+currentProject->colorCntalt)){
		outofBoundsAlert("palette entry",ent);
		return 0;
	}
	return 1;
}
static int lua_palette_getRGB(lua_State*L){
	unsigned ent=luaL_optunsigned(L,1,0);
	if(inRangeEnt(ent)){
		ent*=3;
		lua_pushunsigned(L,currentProject->rgbPal[ent]);
		lua_pushunsigned(L,currentProject->rgbPal[ent+1]);
		lua_pushunsigned(L,currentProject->rgbPal[ent+2]);
		return 3;
	}else
		return 0;
}
static int lua_palette_setRGB(lua_State*L){
	unsigned ent=luaL_optunsigned(L,1,0);
	if(inRangeEnt(ent))
		rgbToEntry(luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0),luaL_optunsigned(L,4,0),ent);
	return 0;
}
static int lua_palette_fixSliders(lua_State*L){
	set_mode_tabs(0,0);
	window->redraw();
	return 0;
}
static const luaL_Reg lua_paletteAPI[]={
	{"getRGB",lua_palette_getRGB},
	{"setRGB",lua_palette_setRGB},
	{"fixSliders",lua_palette_fixSliders},
	{0,0}
};
static unsigned inRangeTile(unsigned tile){
	if(tile>=currentProject->tileC->amt){
		outofBoundsAlert("tile",tile);
		return 0;
	}
	return 1;
}
static unsigned inXYbound(unsigned x,unsigned y){
	if(x>=currentProject->tileC->sizew){
		outofBoundsAlert("X",x);
		return 0;
	}
	if(y>=currentProject->tileC->sizeh){
		outofBoundsAlert("Y",y);
		return 0;
	}
	return 1;
}
static int lua_tile_getPixelRGBA(lua_State*L){
	unsigned tile=luaL_optunsigned(L,1,0);
	unsigned x=luaL_optunsigned(L,2,0);
	unsigned y=luaL_optunsigned(L,3,0);
	if(inRangeTile(tile)){
		if(inXYbound(x,y)){
			uint8_t*tptr=((uint8_t*)currentProject->tileC->truetDat.data()+(tile*currentProject->tileC->tcSize));
			tptr+=(y*currentProject->tileC->sizew+x)*4;
			for(unsigned i=0;i<4;++i)
				lua_pushunsigned(L,*tptr++);
			return 4;
		}
	}
	return 0;
}
static int lua_tile_setPixelRGBA(lua_State*L){
	unsigned tile=luaL_optunsigned(L,1,0);
	unsigned x=luaL_optunsigned(L,2,0);
	unsigned y=luaL_optunsigned(L,3,0);
	if(inRangeTile(tile)){
		if(inXYbound(x,y)){
			uint8_t*tptr=((uint8_t*)currentProject->tileC->truetDat.data()+(tile*currentProject->tileC->tcSize));
			tptr+=(y*currentProject->tileC->sizew+x)*4;
			for(unsigned i=4;i<8;++i){
				unsigned tmp=luaL_optunsigned(L,i,0);
				if(tmp>255)
					tmp=255;
				*tptr++=tmp;
			}
		}
	}
	return 0;
}
static int lua_tile_dither(lua_State*L){
	unsigned tile=luaL_optunsigned(L,1,0);
	unsigned row=luaL_optunsigned(L,2,0);
	bool useAlt=luaL_optunsigned(L,3,0);
	if(inRangeTile(tile))
		currentProject->tileC->truecolor_to_tile(row,tile,useAlt);
	return 0;
}
static void syncTileAmt(lua_State*L){
	lua_getglobal(L, "tile");
	lua_pushstring(L,"amt");
	lua_pushunsigned(L, currentProject->tileC->amt);
	lua_rawset(L, -3);
}
static int lua_tile_append(lua_State*L){
	currentProject->tileC->appendTile(luaL_optunsigned(L,1,1));
	syncTileAmt(L);
	return 0;
}
static int lua_tile_resize(lua_State*L){
	currentProject->tileC->resizeAmt(luaL_optunsigned(L,1,1));
	syncTileAmt(L);
	return 0;
}
static const luaL_Reg lua_tileAPI[]={
	{"getPixelRGBA",lua_tile_getPixelRGBA},
	{"setPixelRGBA",lua_tile_setPixelRGBA},
	{"dither",lua_tile_dither},
	{"append",lua_tile_append},
	{"resize",lua_tile_resize},
	{0,0}
};
static int lua_tilemap_dither(lua_State*L){
	unsigned method=luaL_optunsigned(L,1,1);
	currentProject->tileMapC->ditherAsImage(method);
	return 0;
}
static const luaL_Reg lua_tilemapAPI[]={
	{"dither",lua_tilemap_dither},
	{0,0}
};
static int lua_sprite_dither(lua_State*L){
	unsigned which=luaL_optunsigned(L,1,0);
	ditherSpriteAsImage(which);
	return 0;
}
static int lua_sprite_ditherAll(lua_State*L){
	ditherSpriteAsImageAllCB(0,0);
	return 0;
}
static const luaL_Reg lua_spriteAPI[]={
	{"dither",lua_sprite_dither},
	{"ditherAll",lua_sprite_ditherAll},
	{0,0}
};
static int lua_rgt_redraw(lua_State*L){
	window->redraw();
	return 0;
}
static const luaL_Reg lua_rgtAPI[]={
	{"redraw",lua_rgt_redraw},
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

  				lua_createtable(L, 0,(sizeof(lua_paletteAPI)/sizeof((lua_paletteAPI)[0]) - 1)+5);
				luaL_setfuncs(L,lua_paletteAPI,0);
				
				lua_pushstring(L,"cnt");
				lua_pushunsigned(L, currentProject->colorCnt);
				lua_rawset(L, -3);

				lua_pushstring(L,"cntAlt");
				lua_pushunsigned(L, currentProject->colorCntalt);
				lua_rawset(L, -3);

				lua_pushstring(L,"rowCnt");
				lua_pushunsigned(L, currentProject->rowCntPal);
				lua_rawset(L, -3);

				lua_pushstring(L,"rowCntAlt");
				lua_pushunsigned(L, currentProject->rowCntPalalt);
				lua_rawset(L, -3);

				lua_pushstring(L,"haveAlt");
				lua_pushboolean(L, currentProject->haveAltspritePal);
				lua_rawset(L, -3);

				lua_setglobal(L, "palette");


  				lua_createtable(L, 0,(sizeof(lua_paletteAPI)/sizeof((lua_paletteAPI)[0]) - 1)+1);
				luaL_setfuncs(L,lua_tileAPI,0);

				lua_pushstring(L,"amt");
				lua_pushunsigned(L, currentProject->tileC->amt);
				lua_rawset(L, -3);

				lua_setglobal(L, "tile");


  				lua_createtable(L, 0,(sizeof(lua_paletteAPI)/sizeof((lua_paletteAPI)[0]) - 1)+2);
				luaL_setfuncs(L,lua_tilemapAPI,0);

				lua_pushstring(L,"width");
				lua_pushunsigned(L, currentProject->tileMapC->mapSizeW);
				lua_rawset(L, -3);

				lua_pushstring(L,"height");
				lua_pushunsigned(L, currentProject->tileMapC->mapSizeH);
				lua_rawset(L, -3);

				lua_setglobal(L, "tilemap");

  				lua_createtable(L, 0,(sizeof(lua_paletteAPI)/sizeof((lua_paletteAPI)[0]) - 1)+1);
				luaL_setfuncs(L,lua_spriteAPI,0);

				lua_pushstring(L,"amt");
				lua_pushunsigned(L, currentProject->spritesC->amt);
				lua_rawset(L, -3);

				lua_setglobal(L, "sprite");


				luaL_newlib(L,lua_rgtAPI);
				lua_setglobal(L, "rgt");

				std::string scriptnamecopy=scriptname.c_str();
				chdir(dirname((char*)scriptnamecopy.c_str()));
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
