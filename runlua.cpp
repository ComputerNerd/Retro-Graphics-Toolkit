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
#include "dither.h"
#include "CIE.h"
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
static int luafl_eventnames(lua_State*L){
	lua_pushstring(L,fl_eventnames[luaL_checkunsigned(L,1)]);
	return 1;
}
static int luafl_point(lua_State*L){
	fl_point(luaL_optint(L,1,0),luaL_optint(L,2,0));
	return 0;
}
static int luafl_rect(lua_State*L){
	int x=luaL_optint(L,1,0),y=luaL_optint(L,2,0),w=luaL_optint(L,3,0),h=luaL_optint(L,4,0);
	if(lua_type(L,5)==LUA_TNUMBER)
		fl_rect(x,y,w,h,luaL_optunsigned(L,5,0));
	else
		fl_rect(x,y,w,h);
	return 0;
}
static int luafl_color(lua_State*L){
	if(lua_type(L,2)==LUA_TNUMBER&&lua_type(L,3)==LUA_TNUMBER)
		fl_color(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0));
	else
		fl_color(luaL_optunsigned(L,1,0));
	return 0;
}
static int luafl_arc(lua_State*L){
	if(lua_type(L,6)==LUA_TNUMBER)
		fl_arc(luaL_optint(L,1,0),luaL_optint(L,2,0),luaL_optint(L,3,0),luaL_optint(L,4,0),luaL_optnumber(L,5,0),luaL_optnumber(L,6,0));
	else
		fl_arc(luaL_optnumber(L,1,0.0),luaL_optnumber(L,2,0.0),luaL_optnumber(L,3,0.0),luaL_optnumber(L,4,0.0),luaL_optnumber(L,5,0.0));
	return 0;
}
static int luafl_begin_complex_polygon(lua_State*L){
	fl_begin_complex_polygon();
	return 0;
}
static int luafl_begin_line(lua_State*L){
	fl_begin_line();
	return 0;
}
static int luafl_begin_loop(lua_State*L){
	fl_begin_loop();
	return 0;
}
static int luafl_begin_points(lua_State*L){
	fl_begin_points();
	return 0;
}
static int luafl_begin_polygon(lua_State*L){
	fl_begin_polygon();
	return 0;
}
static int luafl_circle(lua_State*L){
	fl_circle(luaL_optnumber(L,1,0.0),luaL_optnumber(L,2,0.0),luaL_optnumber(L,3,0.0));
	return 0;
}
static int luafl_end_complex_polygon(lua_State*L){
	fl_end_complex_polygon();
	return 0;
}
static int luafl_end_line(lua_State*L){
	fl_end_line();
	return 0;
}
static int luafl_end_loop(lua_State*L){
	fl_end_loop();
	return 0;
}
static int luafl_end_points(lua_State*L){
	fl_end_points();
	return 0;
}
static int luafl_end_polygon(lua_State*L){
	fl_end_polygon();
	return 0;
}
static int luafl_draw(lua_State*L){
	if(lua_type(L,1)==LUA_TNUMBER){
		int parm[3];
		parm[0]=luaL_optint(L,1,0);
		parm[1]=luaL_optint(L,3,0);
		parm[2]=luaL_optint(L,4,0);
		const char*str=lua_tostring(L,2);
		if(lua_type(L,5)==LUA_TNUMBER)//n specified
			fl_draw(parm[0],str,parm[1],parm[2],luaL_optint(L,5,0));
		else
			fl_draw(parm[0],str,parm[1],parm[2]);
			
	}else{
		const char*str=lua_tostring(L,1);
		int parm[2];
		parm[0]=luaL_optint(L,2,0);
		parm[1]=luaL_optint(L,3,0);
		if(lua_type(L,4)==LUA_TNUMBER)//n specified
			fl_draw(str,parm[0],parm[1],luaL_optint(L,4,0));
		else
			fl_draw(str,parm[0],parm[1]);
	}
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
	{"eventnames",luafl_eventnames},
	{"point",luafl_point},
	{"rect",luafl_rect},
	{"color",luafl_color},
	{"arc",luafl_arc},
	{"begin_complex_polygon",luafl_begin_complex_polygon},
	{"begin_line",luafl_begin_line},
	{"begin_loop",luafl_begin_loop},
	{"begin_points",luafl_begin_points},
	{"begin_polygon",luafl_begin_polygon},
	{"circle",luafl_circle},
	{"end_complex_polygon",luafl_end_complex_polygon},
	{"end_line",luafl_end_line},
	{"end_loop",luafl_end_loop},
	{"end_points",luafl_end_points},
	{"end_polygon",luafl_end_polygon},
	{"draw",luafl_draw},
	{0,0}
};
static int luaFl_check(lua_State*L){
	lua_pushinteger(L,Fl::check());
	return 1;
}
static int luaFl_wait(lua_State*L){
	if(lua_type(L,1)==LUA_TNUMBER)
		lua_pushinteger(L,Fl::wait(luaL_checknumber(L,1)));
	else
		lua_pushinteger(L,Fl::wait());
	return 1;
}
static int luaFl_event_x(lua_State*L){
	lua_pushinteger(L,Fl::event_x());
	return 1;
}
static int luaFl_event_y(lua_State*L){
	lua_pushinteger(L,Fl::event_y());
	return 1;
}
static const luaL_Reg lua_FlAPI[]={
	{"check",luaFl_check},
	{"wait",luaFl_wait},
	{"event_x",luaFl_event_x},
	{"event_y",luaFl_event_y},
	{0,0}
};
static void outofBoundsAlert(const char*what,unsigned val){
	fl_alert("Error tried to access out of bound %s %u",what,val);
}
static unsigned inRangeEnt(unsigned ent){
	if(ent>=(currentProject->pal->colorCnt+currentProject->pal->colorCntalt)){
		outofBoundsAlert("palette entry",ent);
		return 0;
	}
	return 1;
}
static int lua_palette_getRGB(lua_State*L){
	unsigned ent=luaL_optunsigned(L,1,0);
	if(inRangeEnt(ent)){
		ent*=3;
		lua_pushunsigned(L,currentProject->pal->rgbPal[ent]);
		lua_pushunsigned(L,currentProject->pal->rgbPal[ent+1]);
		lua_pushunsigned(L,currentProject->pal->rgbPal[ent+2]);
		return 3;
	}else
		return 0;
}
static int lua_palette_getRaw(lua_State*L){
	unsigned ent=luaL_optunsigned(L,1,0);
	if(inRangeEnt(ent)){
		switch(currentProject->gameSystem){
			case sega_genesis:
				ent*=2;
				lua_pushunsigned(L,currentProject->pal->palDat[ent]|(currentProject->pal->palDat[ent+1]<<8));
			break;
			case NES:
				lua_pushunsigned(L,currentProject->pal->palDat[ent]);
			break;
			default:
				show_default_error
				return 0;
		}
		return 1;
	}else
		return 0;
}
static int lua_palette_setRaw(lua_State*L){
	unsigned ent=luaL_optunsigned(L,1,0);
	if(inRangeEnt(ent)){
		unsigned val=luaL_optunsigned(L,2,0);
		switch(currentProject->gameSystem){
			case sega_genesis:
				currentProject->pal->palDat[ent*2]=val&255;
				currentProject->pal->palDat[ent*2+1]=val>>8;
			break;
			case NES:
				currentProject->pal->palDat[ent]=val;
			break;
			default:
				show_default_error
				return 0;
		}
		currentProject->pal->updateRGBindex(ent);
		return 1;
	}else
		return 0;
}
static int lua_palette_setRGB(lua_State*L){
	unsigned ent=luaL_optunsigned(L,1,0);
	if(inRangeEnt(ent))
		currentProject->pal->rgbToEntry(luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0),luaL_optunsigned(L,4,0),ent);
	return 0;
}
static int lua_palette_fixSliders(lua_State*L){
	set_mode_tabs(0,0);
	window->redraw();
	return 0;
}
static int lua_palette_maxInRow(lua_State*L){
	lua_pushunsigned(L,currentProject->pal->calMaxPerRow(luaL_optunsigned(L,1,0)));
	return 1;
}
static int lua_palette_getType(lua_State*L){
	lua_pushunsigned(L,currentProject->pal->palType[luaL_optunsigned(L,1,0)]);
	return 1;
}
static int lua_palette_sortByHSL(lua_State*L){
	sortBy(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0));
	return 0;
}
static const luaL_Reg lua_paletteAPI[]={
	{"getRGB",lua_palette_getRGB},
	{"setRGB",lua_palette_setRGB},
	{"getRaw",lua_palette_getRaw},
	{"setRaw",lua_palette_setRaw},
	{"fixSliders",lua_palette_fixSliders},
	{"maxInRow",lua_palette_maxInRow},
	{"getType",lua_palette_getType},
	{"sortByHSL",lua_palette_sortByHSL},
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
static int lua_tile_getTileRGBA(lua_State*L){
	unsigned tile=luaL_optunsigned(L,1,0);
	if(inRangeTile(tile)){
		uint8_t*tptr=((uint8_t*)currentProject->tileC->truetDat.data()+(tile*currentProject->tileC->tcSize));
		lua_newtable(L);
		for(unsigned i=1;i<=currentProject->tileC->tcSize;++i){
			lua_pushunsigned(L,*tptr++);
			lua_rawseti(L,-2,i);
		}
		return 1;
	}
	return 0;
}
static void fillucharFromTab(lua_State*L,unsigned index,unsigned len,unsigned sz,uint8_t*ptr){//len amount in table sz expected size
	unsigned to=std::min(len,sz);
	for(unsigned i=1;i<=to;++i){
		lua_rawgeti(L,index,i);
		int tmp=lua_tointeger(L,-1);
		if(tmp<0)
			tmp=0;
		if(tmp>255)
			tmp=255;
		*ptr++=tmp;
		lua_pop(L,1);
	}
	if(sz>len)
		memset(ptr,0,sz-len);
}
static int lua_tile_setTileRGBA(lua_State*L){
	unsigned tile=luaL_optunsigned(L,1,0);
	if(inRangeTile(tile)){
		uint8_t*tptr=((uint8_t*)currentProject->tileC->truetDat.data()+(tile*currentProject->tileC->tcSize));
		unsigned len=lua_rawlen(L,2);
		if(!len){
			fl_alert("setTileRGBA error: parameter 2 must be a table");
			return 0;
		}
		fillucharFromTab(L,2,len,currentProject->tileC->tcSize,tptr);
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
static void setUnsignedLua(lua_State*L,const char*tab,const char*var,unsigned val){
	lua_getglobal(L,tab);
	lua_pushstring(L,var);
	lua_pushunsigned(L,val);
	lua_rawset(L, -3);
}
static void syncTileAmt(lua_State*L){
	setUnsignedLua(L,"tile","amt",currentProject->tileC->amt);
	updateTileSelectAmt();
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
	{"getTileRGBA",lua_tile_getTileRGBA},
	{"setTileRGBA",lua_tile_setTileRGBA},
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
static int lua_tilemap_resize(lua_State*L){
	currentProject->tileMapC->resize_tile_map(luaL_optunsigned(L,1,1),luaL_optunsigned(L,2,1));
	setUnsignedLua(L,"tilemap","width",currentProject->tileMapC->mapSizeW);
	setUnsignedLua(L,"tilemap","height",currentProject->tileMapC->mapSizeH);
	setUnsignedLua(L,"tilemap","heightA",currentProject->tileMapC->mapSizeHA);
	return 0;
}
static int lua_tilemap_getHflip(lua_State*L){
	lua_pushboolean(L,currentProject->tileMapC->get_hflip(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0)));
	return 1;
}
static int lua_tilemap_getVflip(lua_State*L){
	lua_pushboolean(L,currentProject->tileMapC->get_vflip(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0)));
	return 1;
}
static int lua_tilemap_getPrio(lua_State*L){
	lua_pushboolean(L,currentProject->tileMapC->get_prio(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0)));
	return 1;
}
static int lua_tilemap_getTile(lua_State*L){
	lua_pushunsigned(L,currentProject->tileMapC->get_tile(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0)));
	return 1;
}
static int lua_tilemap_getTileRow(lua_State*L){
	lua_pushinteger(L,currentProject->tileMapC->get_tileRow(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0)));
	return 1;
}
static int lua_tilemap_getRow(lua_State*L){
	lua_pushunsigned(L,currentProject->tileMapC->get_palette_map(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0)));
	return 1;
}
static int lua_tilemap_setHflip(lua_State*L){
	currentProject->tileMapC->set_hflip(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0));
	return 0;
}
static int lua_tilemap_setVflip(lua_State*L){
	currentProject->tileMapC->set_vflip(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0));
	return 0;
}
static int lua_tilemap_setRow(lua_State*L){
	currentProject->tileMapC->set_pal_row(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0));
	return 0;
}
static int lua_tilemap_setFull(lua_State*L){
	currentProject->tileMapC->set_tile_full(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0),luaL_optunsigned(L,4,0),luaL_optunsigned(L,5,0),luaL_optunsigned(L,6,0),luaL_optunsigned(L,7,0));
	return 0;
}
static int lua_tilemap_setTile(lua_State*L){
	currentProject->tileMapC->set_tile(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0));
	return 0;
}
static int lua_tilemap_setPrio(lua_State*L){
	currentProject->tileMapC->set_prio(luaL_optunsigned(L,1,0),luaL_optunsigned(L,2,0),luaL_optunsigned(L,3,0));
	return 0;
}
static int lua_tilemap_allToRow(lua_State*L){
	currentProject->tileMapC->allRowSet(luaL_optunsigned(L,1,0));
	return 0;
}
static int lua_tilemap_toImage(lua_State*L){
	int row=luaL_optinteger(L,1,-1);
	bool useAlpha=luaL_optunsigned(L,2,0);
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*currentProject->tileC->sizew;
	h=currentProject->tileMapC->mapSizeHA*currentProject->tileC->sizeh;
	unsigned bpp=useAlpha+3;
	uint8_t*image=(uint8_t *)malloc(w*h*bpp);
	if(!image){
		show_malloc_error(w*h*bpp)
		return 0;
	}
	currentProject->tileMapC->truecolor_to_image(image,row,useAlpha);
	uint8_t*imgptr=image;
	lua_newtable(L);
	for(unsigned i=1;i<=w*h*bpp;++i){
		lua_pushunsigned(L,*imgptr++);
		lua_rawseti(L,-2,i);
	}
	free(image);
	return 1;
}
static int lua_tilemap_imageToTiles(lua_State*L){
	unsigned len=lua_rawlen(L,1);
	if(!len){
		fl_alert("imageToTiles error: parameter 1 must be a table");
		return 0;
	}
	int row=luaL_optinteger(L,2,-1);
	bool useAlpha=luaL_optunsigned(L,3,0);
	bool copyToTruecol=luaL_optunsigned(L,4,0);
	bool convert=luaL_optunsigned(L,4,1);
	unsigned bpp=useAlpha+3;
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*currentProject->tileC->sizew;
	h=currentProject->tileMapC->mapSizeHA*currentProject->tileC->sizeh;
	unsigned sz=w*h*bpp;
	uint8_t*image=(uint8_t*)malloc(sz);
	fillucharFromTab(L,1,len,sz,image);
	currentProject->tileMapC->truecolorimageToTiles(image,row,useAlpha,copyToTruecol,convert);
	free(image);
	return 0;
}
static const luaL_Reg lua_tilemapAPI[]={
	{"dither",lua_tilemap_dither},
	{"resize",lua_tilemap_resize},
	{"getHflip",lua_tilemap_getHflip},
	{"getVflip",lua_tilemap_getVflip},
	{"getPrio",lua_tilemap_getPrio},
	{"getTile",lua_tilemap_getTile},
	{"getTileRow",lua_tilemap_getTileRow},
	{"getRow",lua_tilemap_getRow},
	{"setHflip",lua_tilemap_setHflip},
	{"setVflip",lua_tilemap_setVflip},
	{"setRow",lua_tilemap_setRow},
	{"setFull",lua_tilemap_setFull},
	{"setTile",lua_tilemap_setTile},
	{"setPrio",lua_tilemap_setPrio},
	{"allToRow",lua_tilemap_allToRow},
	{"toImage",lua_tilemap_toImage},
	{"imageToTiles",lua_tilemap_imageToTiles},
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
static int lua_project_rgt_have(lua_State*L){
	lua_pushboolean(L,containsDataCurProj(luaL_optunsigned(L,1,pjHavePal)));
	return 1;
}
static int lua_project_rgt_haveOR(lua_State*L){
	lua_pushboolean(L,containsDataCurProjOR(luaL_optunsigned(L,1,pjHavePal)));
	return 1;
}
static int lua_project_rgt_haveMessage(lua_State*L){
	unsigned mask=luaL_optunsigned(L,1,pjHavePal);
	std::string msg="Current project:";
	for(unsigned x=0;x<=pjMaxMaskBit;++x){
		if(mask&(1<<x)){
			msg.push_back('\n');
			msg.append(containsDataCurProj(1<<x)?"has ":"does not have ");
			msg.append(maskToName(1<<x));
		}
	}
	fl_alert(msg.c_str());
	return 0;
}
static const luaL_Reg lua_projectAPI[]={
	{"have",lua_project_rgt_have},
	{"haveOR",lua_project_rgt_haveOR},
	{"haveMessage",lua_project_rgt_haveMessage},
	{0,0}
};
static int lua_rgt_redraw(lua_State*L){
	window->redraw();
	return 0;
}
static int lua_rgt_ditherImage(lua_State*L){
	unsigned len=lua_rawlen(L,1);
	if(!len){
		fl_alert("ditherImage error: parameter 1 must be a table");
		return 0;
	}
	unsigned w=luaL_optunsigned(L,2,0);
	unsigned h=luaL_optunsigned(L,3,0);
	if(!w||!h){
		fl_alert("Invalid width/height");
		return 0;
	}
	bool useAlpha=luaL_optunsigned(L,4,0);
	unsigned bpp=useAlpha+3;
	unsigned sz=w*h*bpp;
	uint8_t*image=(uint8_t*)malloc(sz);
	fillucharFromTab(L,1,len,sz,image);
	ditherImage(image,w,h,useAlpha,luaL_optunsigned(L,5,0),luaL_optunsigned(L,6,0),luaL_optunsigned(L,7,0),luaL_optunsigned(L,8,0),luaL_optunsigned(L,9,0),luaL_optunsigned(L,10,0));
	uint8_t*imgptr=image;
	for(unsigned i=1;i<=std::min(len,sz);++i){
		lua_pushunsigned(L,*imgptr++);
		lua_rawseti(L,1,i);
	}
	free(image);
	return 0;
}
static int lua_rgt_rgbToLab(lua_State*L){
	double l,a,b;
	Rgb2Lab(&l,&a,&b,luaL_optnumber(L,1,0.0),luaL_optnumber(L,2,0.0),luaL_optnumber(L,3,0.0));
	lua_pushnumber(L,l);
	lua_pushnumber(L,a);
	lua_pushnumber(L,b);
	return 3;
}
static int lua_rgt_labToRgb(lua_State*L){
	double r,g,b;
	Lab2Rgb(&r,&g,&b,luaL_optnumber(L,1,0.0),luaL_optnumber(L,2,0.0),luaL_optnumber(L,3,0.0));
	lua_pushnumber(L,r);
	lua_pushnumber(L,g);
	lua_pushnumber(L,b);
	return 3;
}
static int lua_rgt_rgbToLch(lua_State*L){
	double l,c,h;
	Rgb2Lch(&l,&c,&h,luaL_optnumber(L,1,0.0),luaL_optnumber(L,2,0.0),luaL_optnumber(L,3,0.0));
	lua_pushnumber(L,l);
	lua_pushnumber(L,c);
	lua_pushnumber(L,h);
	return 3;
}
static int lua_rgt_lchToRgb(lua_State*L){
	double r,g,b;
	Lch2Rgb(&r,&g,&b,luaL_optnumber(L,1,0.0),luaL_optnumber(L,2,0.0),luaL_optnumber(L,3,0.0));
	lua_pushnumber(L,r);
	lua_pushnumber(L,g);
	lua_pushnumber(L,b);
	return 3;
}
static int lua_rgt_rgbToHsl(lua_State*L){
	double h,s,l;
	rgbToHsl(luaL_optnumber(L,1,0.0),luaL_optnumber(L,2,0.0),luaL_optnumber(L,3,0.0),&h,&s,&l);
	lua_pushnumber(L,h);
	lua_pushnumber(L,s);
	lua_pushnumber(L,l);
	return 3;
}
static const luaL_Reg lua_rgtAPI[]={
	{"redraw",lua_rgt_redraw},
	{"ditherImage",lua_rgt_ditherImage},
	{"rgbToLab",lua_rgt_rgbToLab},
	{"labToRgb",lua_rgt_labToRgb},
	{"rgbToLch",lua_rgt_rgbToLch},
	{"lchToRgb",lua_rgt_lchToRgb},
	{"rgbToHsl",lua_rgt_rgbToHsl},
	{0,0}
};
static void runFunc(lua_State*L,unsigned args,unsigned results){
	if (lua_pcall(L, args, results, 0) != LUA_OK)
		luaL_error(L, "error: %s",lua_tostring(L, -1));
}
class Fl_Lua_Window : public Fl_Window{
public:
	char*drawFunc=0;
	char*handleFunc=0;
	lua_State*L;
	int baseHandle(int e){
		return Fl_Window::handle(e);
	}
	int handle(int e){
		if(handleFunc){
			lua_getglobal(L,handleFunc);
			lua_pushnumber(L,e);
			runFunc(L,1,1);
			int ret=luaL_checkint(L,-1);
			lua_pop(L,1);
			return ret;
		}else
			return baseHandle(e);
	}
	void baseDraw(){
		Fl_Window::draw();
	}
	void draw(){
		if(drawFunc){
			lua_getglobal(L,drawFunc);
			runFunc(L,0,0);
		}else
			baseDraw();
	}
	Fl_Lua_Window(int X, int Y, int W, int H, const char *L=0)
		: Fl_Window(X, Y, W, H, L){
		}
	Fl_Lua_Window(int W, int H, const char *L=0)
		: Fl_Window(W, H, L) {
		}
};
static int lua_Fl_Window_new(lua_State*L){
	unsigned width=luaL_checkunsigned(L,1);
	unsigned height=luaL_checkunsigned(L,2);
	bool useXY=false;
	unsigned x,y;
	const char*title;
	if(lua_type(L,3)==LUA_TNUMBER&&lua_type(L,4)==LUA_TNUMBER){
		unsigned x=luaL_checkunsigned(L,3);
		unsigned y=luaL_checkunsigned(L,4);
		useXY=true;
		title=luaL_optstring(L,5,0);
	}else
		title=luaL_optstring(L,3,0);
	void*ptr=lua_newuserdata(L,sizeof(void*));
	Fl_Lua_Window**win=(Fl_Lua_Window**)ptr;
	luaL_getmetatable(L, "FLTKmeta.Fl_Window");
	lua_setmetatable(L, -2);
	if(useXY)
		*win=new Fl_Lua_Window(x,y,width,height);
	else
		*win=new Fl_Lua_Window(width,height);
	if(title&&title[0])
		(*win)->copy_label(title);
	(*win)->L=L;
	return 1;
}
static Fl_Lua_Window*getWin(lua_State*L){
	Fl_Lua_Window *win = *(Fl_Lua_Window**)luaL_checkudata(L,1,"FLTKmeta.Fl_Window");
	if(!win)
		fl_alert("Fl_Window null");
	return win;
}
static int lua_Fl_Window_show(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	if(win)
		win->show();
	return 0;
}
static int lua_Fl_Window_shown(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	if(win)
		lua_pushinteger(L,win->shown());
	return 1;
}
static int lua_Fl_Window_gc(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	if(win){
		if(win->drawFunc)
			free(win->drawFunc);
		if(win->handleFunc)
			free(win->handleFunc);
		delete win;
	}
	return 0;
}
static int lua_Fl_Window_end(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	if(win)
		win->end();
	return 0;
}
static void cpyAndSet(lua_State*L,char**what){
	if(*what)
		free(*what);
	const char*str=luaL_optstring(L,2,0);
	if(str&&str[0]){
		*what=strdup(str);
	}else
		*what=0;
}
static int lua_Fl_Window_setDrawFunction(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	if(win)
		cpyAndSet(L,&win->drawFunc);
	return 0;
}
static int lua_Fl_Window_setHandleFunction(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	if(win)
		cpyAndSet(L,&win->handleFunc);
	return 0;
}
static int lua_Fl_Window_baseHandle(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	lua_pushinteger(L,win->baseHandle(luaL_checkint(L,2)));
	return 1;
}
static int lua_Fl_Window_baseDraw(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	win->baseDraw();
	return 0;
}
static int lua_Fl_Window_redraw(lua_State*L){
	Fl_Lua_Window*win=getWin(L);
	win->redraw();
	return 0;
}
static const luaL_Reg lua_Fl_Window[]={
	{"new",lua_Fl_Window_new},
	{"show",lua_Fl_Window_show},
	{"shown",lua_Fl_Window_shown},
	{"End",lua_Fl_Window_end},
	{"baseHandle",lua_Fl_Window_baseHandle},
	{"baseDraw",lua_Fl_Window_baseDraw},
	{"setDrawFunction",lua_Fl_Window_setDrawFunction},
	{"setHandleFunction",lua_Fl_Window_setHandleFunction},
	{"redraw",lua_Fl_Window_redraw},
	{0,0}
};
static void mkKeyunsigned(lua_State*L,const char*str,unsigned val){
	lua_pushstring(L,str);
	lua_pushunsigned(L,val);
	lua_rawset(L, -3);
}
void runLua(Fl_Widget*,void*){
	std::string scriptname;
	if(loadsavefile(scriptname,"Select a lua script")){
		lua_State *L = lua_newstate(l_alloc, NULL);
  		if(L){
			lua_atpanic(L, &panic);
			try{
				luaL_openlibs(L);
				//FLTK bindings
				luaL_newlib(L,lua_flAPI);
				lua_setglobal(L, "fl");

				luaL_newlib(L,lua_FlAPI);
				lua_setglobal(L, "Fl");

				lua_createtable(L,0,FL_FULLSCREEN);
				for(unsigned x=0;x<=FL_FULLSCREEN;++x)
					mkKeyunsigned(L,fl_eventnames[x]+3,x);
				lua_setglobal(L, "FL");

				luaL_newmetatable(L,"FLTKmeta.Fl_Window");
				lua_pushcfunction(L,lua_Fl_Window_gc);
				lua_setfield(L, -2, "__gc");
				luaL_newlib(L,lua_Fl_Window);
				lua_setglobal(L, "Fl_Window");

				//Retro Graphics Toolkit bindings
				if(containsDataCurProj(pjHavePal)){
					lua_createtable(L, 0,(sizeof(lua_paletteAPI)/sizeof((lua_paletteAPI)[0]) - 1)+5);
					luaL_setfuncs(L,lua_paletteAPI,0);

					mkKeyunsigned(L,"cnt",currentProject->pal->colorCnt);
					mkKeyunsigned(L,"cntAlt",currentProject->pal->colorCntalt);
					mkKeyunsigned(L,"rowCnt",currentProject->pal->rowCntPal);
					mkKeyunsigned(L,"rowCntAlt",currentProject->pal->rowCntPalalt);
					mkKeyunsigned(L,"haveAlt",currentProject->pal->haveAlt);

					lua_setglobal(L, "palette");
				}

				if(containsDataCurProj(pjHaveTiles)){
					lua_createtable(L, 0,(sizeof(lua_tileAPI)/sizeof((lua_tileAPI)[0]) - 1)+3);
					luaL_setfuncs(L,lua_tileAPI,0);

					mkKeyunsigned(L,"amt",currentProject->tileC->amt);
					mkKeyunsigned(L,"width",currentProject->tileC->sizew);
					mkKeyunsigned(L,"height",currentProject->tileC->sizeh);

					lua_setglobal(L, "tile");
				}

				if(containsDataCurProj(pjHaveMap)){
					lua_createtable(L, 0,(sizeof(lua_tilemapAPI)/sizeof((lua_tilemapAPI)[0]) - 1)+3);
					luaL_setfuncs(L,lua_tilemapAPI,0);

					mkKeyunsigned(L,"width",currentProject->tileMapC->mapSizeW);
					mkKeyunsigned(L,"height",currentProject->tileMapC->mapSizeH);
					mkKeyunsigned(L,"heightA",currentProject->tileMapC->mapSizeHA);

					lua_setglobal(L, "tilemap");
				}

				if(containsDataCurProj(pjHaveSprites)){
					lua_createtable(L, 0,(sizeof(lua_spriteAPI)/sizeof((lua_spriteAPI)[0]) - 1)+1);
					luaL_setfuncs(L,lua_spriteAPI,0);

					mkKeyunsigned(L,"amt",currentProject->spritesC->amt);

					lua_setglobal(L, "sprite");
				}


  				lua_createtable(L, 0,(sizeof(lua_projectAPI)/sizeof((lua_projectAPI)[0]) - 1)+10);
				luaL_setfuncs(L,lua_projectAPI,0);
				mkKeyunsigned(L,"palMask",pjHavePal);
				mkKeyunsigned(L,"tilesMask",pjHaveTiles);
				mkKeyunsigned(L,"mapMask",pjHaveMap);
				mkKeyunsigned(L,"chunksMask",pjHaveChunks);
				mkKeyunsigned(L,"spritesMask",pjHaveSprites);
				mkKeyunsigned(L,"levelMask",pjHaveLevel);
				mkKeyunsigned(L,"allMask",pjAllMask);
				mkKeyunsigned(L,"gameSystem",currentProject->gameSystem);
				mkKeyunsigned(L,"segaGenesis",sega_genesis);
				mkKeyunsigned(L,"NES",NES);
				lua_setglobal(L, "project");

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
