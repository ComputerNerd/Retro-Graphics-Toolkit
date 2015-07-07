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
	Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#include <string>
#include <FL/Fl_Color_Chooser.H>
#include <cmath>//Mingw workaround
#include <FL/Fl_File_Chooser.H>
#include <libgen.h>
#ifdef __MINGW32__
#include <direct.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "runlua.h"
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
#include "class_global.h"
#include "palette.h"
#include "callback_project.h"
#include "lua_zlib.h"
#include "dub/dub.h"
#include "FLTK_Fl_Box.h"
#include "FLTK_Fl_Button.h"
#include "FLTK_Fl_Chart.h"
#include "FLTK_Fl_Choice.h"
#include "FLTK_Fl_Double_Window.h"
#include "FLTK_Fl_Float_Input.h"
#include "FLTK_Fl_Group.h"
#include "FLTK_Fl_Input.h"
#include "FLTK_Fl_Input_Choice.h"
#include "FLTK_Fl_Int_Input.h"
#include "FLTK_Fl_Progress.h"
#include "FLTK_Fl_Scrollbar.h"
#include "FLTK_Fl_Slider.h"
#include "FLTK_Fl_Spinner.h"
#include "FLTK_Fl_Tree.h"
#include "FLTK_Fl_Tree_Item.h"
#include "FLTK_Fl_Tree_Item_Array.h"
#include "FLTK_Fl_Tree_Prefs.h"
#include "FLTK_Fl_Value_Slider.h"
#include "undoLua.h"
static int panic(lua_State *L){
	fl_alert("PANIC: unprotected error in call to Lua API (%s)\n",lua_tostring(L, -1));
	throw 0;//Otherwise abort() would be called when not needed
}
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize){
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
	r=luaL_optnumber(L,3,0.);
	g=luaL_optnumber(L,4,0.);
	b=luaL_optnumber(L,5,0.);
	int ret=fl_color_chooser(luaL_optstring(L,1,"Select a color"),r,g,b,luaL_optinteger(L,2,-1));
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
	lua_pushstring(L,fl_dir_chooser(luaL_optstring(L,1,"Choose a directory"),luaL_optstring(L,2,NULL),luaL_optinteger(L,3,0)));
	return 1;
}
static int luafl_file_chooser(lua_State*L){
	lua_pushstring(L,fl_file_chooser(luaL_optstring(L,1,"Choose a file"),luaL_optstring(L,2,NULL),luaL_optstring(L,3,0),luaL_optinteger(L,4,0)));
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
	lua_pushstring(L,fl_eventnames[luaL_checkinteger(L,1)]);
	return 1;
}
static int luafl_point(lua_State*L){
	fl_point(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0));
	return 0;
}
static int luafl_rect(lua_State*L){
	int x=luaL_optinteger(L,1,0),y=luaL_optinteger(L,2,0),w=luaL_optinteger(L,3,0),h=luaL_optinteger(L,4,0);
	if(lua_type(L,5)==LUA_TNUMBER)
		fl_rect(x,y,w,h,luaL_optinteger(L,5,0));
	else
		fl_rect(x,y,w,h);
	return 0;
}
static int luafl_color(lua_State*L){
	if(lua_type(L,2)==LUA_TNUMBER&&lua_type(L,3)==LUA_TNUMBER)
		fl_color(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0));
	else
		fl_color((unsigned)luaL_optinteger(L,1,0));
	return 0;
}
static int luafl_arc(lua_State*L){
	if(lua_type(L,6)==LUA_TNUMBER)
		fl_arc(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0),luaL_optnumber(L,5,0),luaL_optnumber(L,6,0));
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
		parm[0]=luaL_optinteger(L,1,0);
		parm[1]=luaL_optinteger(L,3,0);
		parm[2]=luaL_optinteger(L,4,0);
		const char*str=lua_tostring(L,2);
		if(lua_type(L,5)==LUA_TNUMBER)//n specified
			fl_draw(parm[0],str,parm[1],parm[2],luaL_optinteger(L,5,0));
		else
			fl_draw(parm[0],str,parm[1],parm[2]);
			
	}else{
		const char*str=lua_tostring(L,1);
		int parm[2];
		parm[0]=luaL_optinteger(L,2,0);
		parm[1]=luaL_optinteger(L,3,0);
		if(lua_type(L,4)==LUA_TNUMBER)//n specified
			fl_draw(str,parm[0],parm[1],luaL_optinteger(L,4,0));
		else
			fl_draw(str,parm[0],parm[1]);
	}
	return 0;
}
/** void fl_rectf(int x, int y, int w, int h)
 * inc/fl_draw.h:206
 */
static int FLTK_fl_rectf(lua_State *L) {
	try {
		int top__ = lua_gettop(L);
		if (top__ >= 7) {
			int x = dub::checkinteger(L, 1);
			int y = dub::checkinteger(L, 2);
			int w = dub::checkinteger(L, 3);
			int h = dub::checkinteger(L, 4);
			unsigned char r = dub::checkinteger(L, 5);
			unsigned char g = dub::checkinteger(L, 6);
			unsigned char b = dub::checkinteger(L, 7);
			fl_rectf(x, y, w, h, r, g, b);
			return 0;
		} else if (top__ >= 5) {
			int x = dub::checkinteger(L, 1);
			int y = dub::checkinteger(L, 2);
			int w = dub::checkinteger(L, 3);
			int h = dub::checkinteger(L, 4);
			Fl_Color *c = *((Fl_Color **)dub::checksdata(L, 5, "Fl_Color"));
			fl_rectf(x, y, w, h, *c);
			return 0;
		} else {
			int x = dub::checkinteger(L, 1);
			int y = dub::checkinteger(L, 2);
			int w = dub::checkinteger(L, 3);
			int h = dub::checkinteger(L, 4);
			fl_rectf(x, y, w, h);
			return 0;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "FLTK.fl_rectf: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "FLTK.fl_rectf: Unknown exception");
	}
	return lua_error(L);
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
	{"rectf",FLTK_fl_rectf},
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
static int luaFl_event_key(lua_State*L){
	if(lua_type(L,1)==LUA_TNUMBER)
		lua_pushinteger(L,Fl::event_key(luaL_checknumber(L,1)));
	else
		lua_pushinteger(L,Fl::event_key());
	return 1;
}
static int luaFl_event_ctrl(lua_State*L){
	lua_pushinteger(L,Fl::event_ctrl());
	return 1;
}
static int luaFl_event_alt(lua_State*L){
	lua_pushinteger(L,Fl::event_alt());
	return 1;
}
static int luaFl_event_shift(lua_State*L){
	lua_pushinteger(L,Fl::event_shift());
	return 1;
}
static int luaFl_event_length(lua_State*L){
	lua_pushinteger(L,Fl::event_length());
	return 1;
}
static int luaFl_event_text(lua_State*L){
	lua_pushlstring(L,Fl::event_text(),Fl::event_length());
	return 1;
}
static const luaL_Reg lua_FlAPI[]={
	{"check",luaFl_check},
	{"wait",luaFl_wait},
	{"event_x",luaFl_event_x},
	{"event_y",luaFl_event_y},
	{"event_key",luaFl_event_key},
	{"event_ctrl",luaFl_event_ctrl},
	{"event_alt",luaFl_event_alt},
	{"event_shift",luaFl_event_shift},
	{"event_length",luaFl_event_length},
	{"event_text",luaFl_event_text},
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
	unsigned ent=luaL_optinteger(L,1,0);
	if(inRangeEnt(ent)){
		ent*=3;
		lua_pushinteger(L,currentProject->pal->rgbPal[ent]);
		lua_pushinteger(L,currentProject->pal->rgbPal[ent+1]);
		lua_pushinteger(L,currentProject->pal->rgbPal[ent+2]);
		return 3;
	}else
		return 0;
}
static int lua_palette_getRaw(lua_State*L){
	unsigned ent=luaL_optinteger(L,1,0);
	if(inRangeEnt(ent)){
		switch(currentProject->pal->esize){
			case 1:
				lua_pushinteger(L,currentProject->pal->palDat[ent]);
			break;
			case 2:
				ent*=2;
				lua_pushinteger(L,currentProject->pal->palDat[ent]|(currentProject->pal->palDat[ent+1]<<8));
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
	unsigned ent=luaL_optinteger(L,1,0);
	if(inRangeEnt(ent)){
		unsigned val=luaL_optinteger(L,2,0);
		switch(currentProject->pal->esize){
			case 1:
				currentProject->pal->palDat[ent]=val;
			break;
			case 2:
				currentProject->pal->palDat[ent*2]=val&255;
				currentProject->pal->palDat[ent*2+1]=val>>8;
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
	unsigned ent=luaL_optinteger(L,1,0);
	if(inRangeEnt(ent))
		currentProject->pal->rgbToEntry(lua_tointeger(L,2),lua_tointeger(L,3),lua_tointeger(L,4),ent);
	return 0;
}
static int lua_palette_fixSliders(lua_State*L){
	set_mode_tabs(0,0);
	window->redraw();
	return 0;
}
static int lua_palette_maxInRow(lua_State*L){
	lua_pushinteger(L,currentProject->pal->calMaxPerRow(luaL_optinteger(L,1,0)));
	return 1;
}
static int lua_palette_getType(lua_State*L){
	lua_pushinteger(L,currentProject->pal->palType[luaL_optinteger(L,1,0)]);
	return 1;
}
static int lua_palette_sortByHSL(lua_State*L){
	sortBy(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0));
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
	unsigned tile=luaL_optinteger(L,1,0);
	unsigned x=luaL_optinteger(L,2,0);
	unsigned y=luaL_optinteger(L,3,0);
	if(inRangeTile(tile)){
		if(inXYbound(x,y)){
			uint8_t*tptr=((uint8_t*)currentProject->tileC->truetDat.data()+(tile*currentProject->tileC->tcSize));
			tptr+=(y*currentProject->tileC->sizew+x)*4;
			for(unsigned i=0;i<4;++i)
				lua_pushinteger(L,*tptr++);
			return 4;
		}
	}
	return 0;
}
static int lua_tile_setPixelRGBA(lua_State*L){
	unsigned tile=luaL_optinteger(L,1,0);
	unsigned x=luaL_optinteger(L,2,0);
	unsigned y=luaL_optinteger(L,3,0);
	if(inRangeTile(tile)){
		if(inXYbound(x,y)){
			uint8_t*tptr=((uint8_t*)currentProject->tileC->truetDat.data()+(tile*currentProject->tileC->tcSize));
			tptr+=(y*currentProject->tileC->sizew+x)*4;
			for(unsigned i=4;i<8;++i){
				unsigned tmp=luaL_optinteger(L,i,0);
				if(tmp>255)
					tmp=255;
				*tptr++=tmp;
			}
		}
	}
	return 0;
}
static int lua_tile_getTileRGBA(lua_State*L){
	unsigned tile=luaL_optinteger(L,1,0);
	if(inRangeTile(tile)){
		uint8_t*tptr=((uint8_t*)currentProject->tileC->truetDat.data()+(tile*currentProject->tileC->tcSize));
		lua_newtable(L);
		for(unsigned i=1;i<=currentProject->tileC->tcSize;++i){
			lua_pushinteger(L,*tptr++);
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
	unsigned tile=luaL_optinteger(L,1,0);
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
	unsigned tile=luaL_optinteger(L,1,0);
	unsigned row=luaL_optinteger(L,2,0);
	bool useAlt=luaL_optinteger(L,3,0);
	if(inRangeTile(tile))
		currentProject->tileC->truecolor_to_tile(row,tile,useAlt);
	return 0;
}
static void setUnsignedLua(lua_State*L,const char*tab,const char*var,unsigned val){
	lua_getglobal(L,tab);
	lua_pushstring(L,var);
	lua_pushinteger(L,val);
	lua_rawset(L, -3);
	lua_pop(L,1);
}
static void setTableUnsignedLua(lua_State*L,const char*tab,const char*subtab,unsigned idx,unsigned val){
	lua_getglobal(L,tab);
	lua_pushstring(L,subtab);
	lua_gettable(L,-2);
	lua_pushinteger(L,val);
	lua_rawseti(L,-2,idx);
	lua_pop(L,1);
}
static lua_Integer getLuaInt(lua_State*L,const char*tab,const char*var){
	lua_getglobal(L,tab);
	lua_pushstring(L,var);
	lua_rawget(L, -2);
	lua_Integer l=luaL_checkinteger(L,-1);
	lua_pop(L,2);
	return l;
}
static void syncTileAmt(lua_State*L){
	setUnsignedLua(L,"tile","amt",currentProject->tileC->amt);
	updateTileSelectAmt();
}
static int lua_tile_append(lua_State*L){
	currentProject->tileC->appendTile(luaL_optinteger(L,1,1));
	syncTileAmt(L);
	return 0;
}
static int lua_tile_resize(lua_State*L){
	currentProject->tileC->resizeAmt(luaL_optinteger(L,1,1));
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
static unsigned getPlane(lua_State*L){
	return unsigned(luaL_optinteger(L,1,0))%currentProject->tms->maps.size();
}
static int lua_tilemap_dither(lua_State*L){
	unsigned method=luaL_optinteger(L,2,1);
	currentProject->tms->maps[getPlane(L)].ditherAsImage(method);
	return 0;
}
static int lua_tilemap_resize(lua_State*L){
	unsigned plane=getPlane(L);
	currentProject->tms->maps[plane].resize_tile_map(luaL_optinteger(L,1,1),luaL_optinteger(L,2,1));
	setTableUnsignedLua(L,"tilemaps","width",plane+1,currentProject->tms->maps[getPlane(L)].mapSizeW);
	setTableUnsignedLua(L,"tilemaps","height",plane+1,currentProject->tms->maps[getPlane(L)].mapSizeH);
	setTableUnsignedLua(L,"tilemaps","heightA",plane+1,currentProject->tms->maps[getPlane(L)].mapSizeHA);
	return 0;
}
static int lua_tilemap_getHflip(lua_State*L){
	lua_pushboolean(L,currentProject->tms->maps[getPlane(L)].get_hflip(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_tilemap_getVflip(lua_State*L){
	lua_pushboolean(L,currentProject->tms->maps[getPlane(L)].get_vflip(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_tilemap_getPrio(lua_State*L){
	lua_pushboolean(L,currentProject->tms->maps[getPlane(L)].get_prio(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_tilemap_getTile(lua_State*L){
	lua_pushinteger(L,currentProject->tms->maps[getPlane(L)].get_tile(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_tilemap_getTileRow(lua_State*L){
	lua_pushinteger(L,currentProject->tms->maps[getPlane(L)].get_tileRow(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0)));
	return 1;
}
static int lua_tilemap_getRow(lua_State*L){
	lua_pushinteger(L,currentProject->tms->maps[getPlane(L)].getPalRow(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_tilemap_setHflip(lua_State*L){
	currentProject->tms->maps[getPlane(L)].set_hflip(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_tilemap_setVflip(lua_State*L){
	currentProject->tms->maps[getPlane(L)].set_vflip(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_tilemap_setRow(lua_State*L){
	currentProject->tms->maps[getPlane(L)].set_pal_row(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_tilemap_setFull(lua_State*L){
	currentProject->tms->maps[getPlane(L)].set_tile_full(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0),luaL_optinteger(L,5,0),luaL_optinteger(L,6,0),luaL_optinteger(L,7,0),luaL_optinteger(L,8,0));
	return 0;
}
static int lua_tilemap_setTile(lua_State*L){
	currentProject->tms->maps[getPlane(L)].set_tile(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_tilemap_setPrio(lua_State*L){
	currentProject->tms->maps[getPlane(L)].set_prio(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_tilemap_allToRow(lua_State*L){
	currentProject->tms->maps[getPlane(L)].allRowSet(luaL_optinteger(L,2,0));
	return 0;
}
static int lua_tilemap_toImage(lua_State*L){
	int row=luaL_optinteger(L,1,-1);
	bool useAlpha=luaL_optinteger(L,2,0);
	uint32_t w,h;
	w=currentProject->tms->maps[getPlane(L)].mapSizeW*currentProject->tileC->sizew;
	h=currentProject->tms->maps[getPlane(L)].mapSizeHA*currentProject->tileC->sizeh;
	unsigned bpp=useAlpha+3;
	uint8_t*image=(uint8_t *)malloc(w*h*bpp);
	if(!image){
		show_malloc_error(w*h*bpp)
		return 0;
	}
	currentProject->tms->maps[getPlane(L)].truecolor_to_image(image,row,useAlpha);
	uint8_t*imgptr=image;
	lua_newtable(L);
	for(unsigned i=1;i<=w*h*bpp;++i){
		lua_pushinteger(L,*imgptr++);
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
	bool useAlpha=luaL_optinteger(L,3,0);
	bool copyToTruecol=luaL_optinteger(L,4,0);
	bool convert=luaL_optinteger(L,4,1);
	unsigned bpp=useAlpha+3;
	uint32_t w,h;
	w=currentProject->tms->maps[getPlane(L)].mapSizeW*currentProject->tileC->sizew;
	h=currentProject->tms->maps[getPlane(L)].mapSizeHA*currentProject->tileC->sizeh;
	unsigned sz=w*h*bpp;
	uint8_t*image=(uint8_t*)malloc(sz);
	fillucharFromTab(L,1,len,sz,image);
	currentProject->tms->maps[getPlane(L)].truecolorimageToTiles(image,row,useAlpha,copyToTruecol,convert);
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
	ditherSpriteAsImage(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0));
	return 0;
}
static int lua_sprite_ditherAll(lua_State*L){
	ditherGroupAsImage(luaL_optinteger(L,1,0));
	return 0;
}
static const luaL_Reg lua_spriteAPI[]={
	{"dither",lua_sprite_dither},
	{"ditherAll",lua_sprite_ditherAll},
	{0,0}
};
static int lua_project_rgt_have(lua_State*L){
	lua_pushboolean(L,currentProject->containsData(luaL_optinteger(L,1,pjHavePal)));
	return 1;
}
static int lua_project_rgt_haveOR(lua_State*L){
	lua_pushboolean(L,currentProject->containsDataOR(luaL_optinteger(L,1,pjHavePal)));
	return 1;
}
static int lua_project_rgt_haveMessage(lua_State*L){
	unsigned mask=luaL_optinteger(L,1,pjHavePal);
	std::string msg="Current project:";
	for(unsigned x=0;x<=pjMaxMaskBit;++x){
		if(mask&(1<<x)){
			msg.append(currentProject->containsData(1<<x)?"\nhas ":"\ndoes not have ");
			msg.append(maskToName(1<<x));
		}
	}
	fl_alert(msg.c_str());
	return 0;
}
static void mkKeyunsigned(lua_State*L,const char*str,unsigned val){
	lua_pushstring(L,str);
	lua_pushinteger(L,val);
	lua_rawset(L, -3);
}
static int lua_project_set(lua_State*L);
static int lua_project_getSettings(lua_State*L){
	lua_pushinteger(L,currentProject->luaSettings);
	return 1;
}
static int lua_project_setSettings(lua_State*L){
	currentProject->luaSettings=luaL_optinteger(L,1,0);
	return 0;
}
static const luaL_Reg lua_projectAPI[]={
	{"have",lua_project_rgt_have},
	{"haveOR",lua_project_rgt_haveOR},
	{"haveMessage",lua_project_rgt_haveMessage},
	{"set",lua_project_set},
	{"getSettings",lua_project_getSettings},
	{"setSettings",lua_project_setSettings},
	{0,0}
};
#define arLen(ar) (sizeof(ar)/sizeof(ar[0]))
void updateProjectTablesLua(lua_State*L){
	//Retro Graphics Toolkit bindings
	lua_pushnil(L);
	lua_setglobal(L, "palette");
	if(currentProject->containsData(pjHavePal)){
		lua_createtable(L, 0,(sizeof(lua_paletteAPI)/sizeof((lua_paletteAPI)[0]) - 1)+5);
		luaL_setfuncs(L,lua_paletteAPI,0);

		mkKeyunsigned(L,"cnt",currentProject->pal->colorCnt);
		mkKeyunsigned(L,"cntAlt",currentProject->pal->colorCntalt);
		mkKeyunsigned(L,"perRow",currentProject->pal->perRow);
		mkKeyunsigned(L,"perRowAlt",currentProject->pal->perRowalt);
		mkKeyunsigned(L,"rowCnt",currentProject->pal->rowCntPal);
		mkKeyunsigned(L,"rowCntAlt",currentProject->pal->rowCntPalalt);
		mkKeyunsigned(L,"haveAlt",currentProject->pal->haveAlt);
		mkKeyunsigned(L,"esize",currentProject->pal->esize);

		lua_setglobal(L, "palette");
	}
	lua_pushnil(L);
	lua_setglobal(L, "tile");
	if(currentProject->containsData(pjHaveTiles)){
		lua_createtable(L, 0,(sizeof(lua_tileAPI)/sizeof((lua_tileAPI)[0]) - 1)+3);
		luaL_setfuncs(L,lua_tileAPI,0);

		mkKeyunsigned(L,"amt",currentProject->tileC->amt);
		mkKeyunsigned(L,"width",currentProject->tileC->sizew);
		mkKeyunsigned(L,"height",currentProject->tileC->sizeh);

		lua_setglobal(L, "tile");
	}

	lua_pushnil(L);
	lua_setglobal(L, "tilemaps");
	if(currentProject->containsData(pjHaveMap)){
		lua_createtable(L, 0,(sizeof(lua_tilemapAPI)/sizeof((lua_tilemapAPI)[0]) - 1)+3);
		luaL_setfuncs(L,lua_tilemapAPI,0);

		lua_pushstring(L,"width");
		lua_createtable(L,currentProject->tms->maps.size(),0);
		for(unsigned i=0;i<currentProject->tms->maps.size();++i){
			lua_pushinteger(L,currentProject->tms->maps[i].mapSizeW);
			lua_rawseti(L,-2,i+1);
		}
		lua_rawset(L,-3);
		lua_pushstring(L,"height");
		lua_createtable(L,currentProject->tms->maps.size(),0);
		for(unsigned i=0;i<currentProject->tms->maps.size();++i){
			lua_pushinteger(L,currentProject->tms->maps[i].mapSizeH);
			lua_rawseti(L,-2,i+1);
		}
		lua_rawset(L,-3);
		lua_pushstring(L,"heightA");
		lua_createtable(L,currentProject->tms->maps.size(),0);
		for(unsigned i=0;i<currentProject->tms->maps.size();++i){
			lua_pushinteger(L,currentProject->tms->maps[i].mapSizeHA);
			lua_rawseti(L,-2,i+1);
		}
		mkKeyunsigned(L,"current",currentProject->curPlane);
		lua_rawset(L,-3);

		lua_setglobal(L, "tilemaps");
	}

	lua_pushnil(L);
	lua_setglobal(L, "metasprites");
	if(currentProject->containsData(pjHaveSprites)){
		lua_createtable(L, 0,(arLen(lua_spriteAPI)-1)+1);
		luaL_setfuncs(L,lua_spriteAPI,0);

		lua_pushstring(L,"amt");
		lua_createtable(L,currentProject->ms->sps.size(),0);
		for(unsigned i=0;i<currentProject->ms->sps.size();++i){
			lua_pushinteger(L,currentProject->ms->sps[i].amt);
			lua_rawseti(L,-2,i+1);
		}
		lua_rawset(L,-3);
		lua_setglobal(L, "metasprites");
	}

	lua_pushnil(L);
	lua_setglobal(L, "project");
	lua_createtable(L, 0,(sizeof(lua_projectAPI)/sizeof((lua_projectAPI)[0]) - 1)+12);
	luaL_setfuncs(L,lua_projectAPI,0);
	mkKeyunsigned(L,"palMask",pjHavePal);
	mkKeyunsigned(L,"tilesMask",pjHaveTiles);
	mkKeyunsigned(L,"mapMask",pjHaveMap);
	mkKeyunsigned(L,"chunksMask",pjHaveChunks);
	mkKeyunsigned(L,"spritesMask",pjHaveSprites);
	mkKeyunsigned(L,"levelMask",pjHaveLevel);
	mkKeyunsigned(L,"allMask",pjAllMask);
	mkKeyunsigned(L,"gameSystem",currentProject->gameSystem);
	mkKeyunsigned(L,"segaGenesis",segaGenesis);
	mkKeyunsigned(L,"NES",NES);
	mkKeyunsigned(L,"count",projects_count);
	mkKeyunsigned(L,"curProjectID",curProjectID);
	lua_setglobal(L, "project");
}
static int lua_project_set(lua_State*L){
	unsigned off=luaL_optinteger(L,1,0);
	printf("off = %u\n",off);
	if((off>=projects_count)||(off==curProjectID))
		lua_pushboolean(L,false);//Failure
	else{
		switchProjectSlider(off);
		updateProjectTablesLua(L);
		lua_pushboolean(L,true);
	}
	return 1;
}
static int lua_rgt_redraw(lua_State*L){
	window->redraw();
	return 0;
}
static int lua_rgt_damage(lua_State*L){
	window->damage(FL_DAMAGE_USER1);
	return 0;
}
static int lua_rgt_ditherImage(lua_State*L){
	unsigned len=lua_rawlen(L,1);
	if(!len){
		fl_alert("ditherImage error: parameter 1 must be a table");
		return 0;
	}
	unsigned w=luaL_optinteger(L,2,0);
	unsigned h=luaL_optinteger(L,3,0);
	if(!w||!h){
		fl_alert("Invalid width/height");
		return 0;
	}
	bool useAlpha=luaL_optinteger(L,4,0);
	unsigned bpp=useAlpha+3;
	unsigned sz=w*h*bpp;
	uint8_t*image=(uint8_t*)malloc(sz);
	fillucharFromTab(L,1,len,sz,image);
	ditherImage(image,w,h,useAlpha,luaL_optinteger(L,5,0),luaL_optinteger(L,6,0),luaL_optinteger(L,7,0),luaL_optinteger(L,8,0),luaL_optinteger(L,9,0),luaL_optinteger(L,10,0));
	uint8_t*imgptr=image;
	for(unsigned i=1;i<=std::min(len,sz);++i){
		lua_pushinteger(L,*imgptr++);
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
static int lua_rgt_setTab(lua_State*L){
	int idx=luaL_checkinteger(L,1);
	if(idx<0)
		idx=window->tabsMain.size()-1;
	idx%=window->tabsMain.size();
	window->the_tabs->value(window->tabsMain[idx]);
	set_mode_tabs(nullptr,nullptr);
}
struct keyPair{
	const char*key;
	unsigned pair;
};
static int lua_rgt_syncProject(lua_State*L){
	updateProjectTablesLua(L);
}
static int lua_rgt_w(lua_State*L){
	lua_pushinteger(L,window->w());
	return 1;
}
static int lua_rgt_h(lua_State*L){
	lua_pushinteger(L,window->h());
	return 1;
}
static const luaL_Reg lua_rgtAPI[]={
	{"redraw",lua_rgt_redraw},
	{"damage",lua_rgt_damage},
	{"ditherImage",lua_rgt_ditherImage},
	{"rgbToLab",lua_rgt_rgbToLab},
	{"labToRgb",lua_rgt_labToRgb},
	{"rgbToLch",lua_rgt_rgbToLch},
	{"lchToRgb",lua_rgt_lchToRgb},
	{"rgbToHsl",lua_rgt_rgbToHsl},
	{"setTab",lua_rgt_setTab},
	{"syncProject",lua_rgt_syncProject},
	{"w",lua_rgt_w},
	{"h",lua_rgt_h},
	{0,0}
};
static const struct keyPair rgtConsts[]={
	{"paletteTab",pal_edit},
	{"tileTab",tile_edit},
	{"planeTab",tile_place},
	{"chunkTab",chunkEditor},
	{"spritesTab",spriteEditor},
	{"levelTab",levelEditor},
	{"settingsTab",settingsTab},
	{"luaTab",luaTab},
};
static int lua_tabs_append(lua_State*L){
	int rx,ry,rw,rh;
	window->the_tabs->client_area(rx,ry,rw,rh);
	window->tabsMain.emplace_back(new Fl_Group(rx, ry, rw, rh, "Lua scripting"));
}
static int lua_tabs_endAppend(lua_State*L){
	window->tabsMain[window->tabsMain.size()-1]->end();
}
static const luaL_Reg lua_tabAPI[]={
	{"appendTab",lua_tabs_append},
	{"endAppendTab",lua_tabs_endAppend},
	{"deleteTab",lua_tabs_append},
	{"getTabs",lua_tabs_append},
	{0,0}
};
void runLuaFunc(lua_State*L,unsigned args,unsigned results){
	try{
		if (lua_pcall(L, args, results, 0) != LUA_OK){
			luaL_error(L, "error: %s",lua_tostring(L, -1));
		}
	}catch(...){
		fl_alert("Lua error while running function\nthrow was called");
	}
}
void runLua(lua_State*L,const char*str,bool isFile){
	try{
		int s;
		if(isFile)
			s=luaL_loadfile(L, str);
		else
			s=luaL_loadstring(L, str);
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
}
static const keyPair FLconsts[]={
	{"MENU_INACTIVE",FL_MENU_INACTIVE},
	{"MENU_TOGGLE",FL_MENU_TOGGLE},
	{"MENU_VALUE",FL_MENU_VALUE},
	{"MENU_RADIO",FL_MENU_RADIO},
	{"MENU_INVISIBLE",FL_MENU_INVISIBLE},
	{"SUBMENU_POINTER",FL_SUBMENU_POINTER},
	{"SUBMENU",FL_SUBMENU},
	{"MENU_DIVIDER",FL_MENU_DIVIDER},
	{"MENU_HORIZONTAL",FL_MENU_HORIZONTAL},
	{"SHIFT",FL_SHIFT},
	{"CAPS_LOCK",FL_CAPS_LOCK},
	{"CTRL",FL_CTRL},
	{"ALT",FL_ALT},
	{"NUM_LOCK",FL_NUM_LOCK},
	{"META",FL_META},
	{"SCROLL_LOCK",FL_SCROLL_LOCK},
	{"BUTTON1",FL_BUTTON1},
	{"BUTTON2",FL_BUTTON2},
	{"BUTTON3",FL_BUTTON3},
	{"BUTTONS",FL_BUTTONS},
	{"Button",FL_Button},
	{"BackSpace",FL_BackSpace},
	{"Tab",FL_Tab},
	{"Iso_Key",FL_Iso_Key},
	{"Enter",FL_Enter},
	{"Pause",FL_Pause},
	{"Scroll_Lock",FL_Scroll_Lock},
	{"Escape",FL_Escape},
	{"Kana",FL_Kana},
	{"Eisu",FL_Eisu},
	{"Yen",FL_Yen},
	{"JIS_Underscore",FL_JIS_Underscore},
	{"Home",FL_Home},
	{"Left",FL_Left},
	{"Up",FL_Up},
	{"Right",FL_Right},
	{"Down",FL_Down},
	{"Page_Up",FL_Page_Up},
	{"Page_Down",FL_Page_Down},
	{"End",FL_End},
	{"Print",FL_Print},
	{"Insert",FL_Insert},
	{"Menu",FL_Menu},
	{"Help",FL_Help},
	{"Num_Lock",FL_Num_Lock},
	{"KP",FL_KP},
	{"KP_Enter",FL_KP_Enter},
	{"KP_Last",FL_KP_Last},
	{"F",FL_F},
	{"F_Last",FL_F_Last},
	{"Shift_L",FL_Shift_L},
	{"Shift_R",FL_Shift_R},
	{"Control_L",FL_Control_L},
	{"Control_R",FL_Control_R},
	{"Caps_Lock",FL_Caps_Lock},
	{"Meta_L",FL_Meta_L},
	{"Meta_R",FL_Meta_R},
	{"Alt_L",FL_Alt_L},
	{"Alt_R",FL_Alt_R},
	{"Delete",FL_Delete},
	{"NORMAL_BUTTON",FL_NORMAL_BUTTON},
	{"TOGGLE_BUTTON",FL_TOGGLE_BUTTON},
	{"RADIO_BUTTON",FL_RADIO_BUTTON},
	{"HIDDEN_BUTTON",FL_HIDDEN_BUTTON},
	{"VERT_SLIDER",FL_VERT_SLIDER},
	{"HOR_SLIDER",FL_HOR_SLIDER},
	{"VERT_FILL_SLIDER",FL_VERT_FILL_SLIDER},
	{"HOR_FILL_SLIDER",FL_HOR_FILL_SLIDER},
	{"VERT_NICE_SLIDER",FL_VERT_NICE_SLIDER},
	{"HOR_NICE_SLIDER",FL_HOR_NICE_SLIDER},
	{"VERTICAL",FL_VERTICAL},
	{"HORIZONTAL",FL_HORIZONTAL},
	{"LEFT_MOUSE",FL_LEFT_MOUSE},
	{"MIDDLE_MOUSE",FL_MIDDLE_MOUSE},
	{"RIGHT_MOUSE",FL_RIGHT_MOUSE},
};
lua_State*createLuaState(void){
	lua_State *L = lua_newstate(l_alloc, NULL);
	if(L){
		lua_atpanic(L, &panic);
		luaL_openlibs(L);
		//FLTK bindings
		luaL_newlib(L,lua_flAPI);
		lua_setglobal(L, "fl");

		luaL_newlib(L,lua_FlAPI);
		lua_setglobal(L, "Fl");

		lua_createtable(L,0,FL_FULLSCREEN+arLen(FLconsts));
		for(unsigned x=0;x<=FL_FULLSCREEN;++x)
			mkKeyunsigned(L,fl_eventnames[x]+3,x);
		for(unsigned x=0;x<arLen(FLconsts);++x)
			mkKeyunsigned(L,FLconsts[x].key,FLconsts[x].pair);
		lua_setglobal(L, "FL");


		updateProjectTablesLua(L);

		lua_createtable(L,0,arLen(rgtConsts)+arLen(lua_rgtAPI)-1);
		luaL_setfuncs(L,lua_rgtAPI,0);
		for(unsigned x=0;x<arLen(rgtConsts);++x)
			mkKeyunsigned(L,rgtConsts[x].key,rgtConsts[x].pair);
		lua_setglobal(L, "rgt");

		luaopen_zlib(L);
		lua_setglobal(L, "zlib");

		luaopen_FLTK_Fl_Box(L);
		lua_setglobal(L, "Fl_Box");
		luaopen_FLTK_Fl_Button(L);
		lua_setglobal(L, "Fl_Button");
		luaopen_FLTK_Fl_Chart(L);
		lua_setglobal(L, "Fl_Chart");
		luaopen_FLTK_Fl_Choice(L);
		lua_setglobal(L, "Fl_Choice");
		luaopen_FLTK_Fl_Double_Window(L);
		lua_setglobal(L, "Fl_Window");
		luaopen_FLTK_Fl_Float_Input(L);
		lua_setglobal(L, "Fl_Float_Input");
		luaopen_FLTK_Fl_Group(L);
		lua_setglobal(L, "Fl_Group");
		luaopen_FLTK_Fl_Input(L);
		lua_setglobal(L, "Fl_Input");
		luaopen_FLTK_Fl_Input_Choice(L);
		lua_setglobal(L, "Fl_Input_Choice");
		luaopen_FLTK_Fl_Int_Input(L);
		lua_setglobal(L, "Fl_Int_Input");
		luaopen_FLTK_Fl_Progress(L);
		lua_setglobal(L, "Fl_Progress");
		luaopen_FLTK_Fl_Scrollbar(L);
		lua_setglobal(L, "Fl_Scrollbar");
		luaopen_FLTK_Fl_Slider(L);
		lua_setglobal(L, "Fl_Slider");
		luaopen_FLTK_Fl_Spinner(L);
		lua_setglobal(L, "Fl_Spinner");
		luaopen_FLTK_Fl_Tree(L);
		lua_setglobal(L, "Fl_Tree");
		luaopen_FLTK_Fl_Tree_Item(L);
		lua_setglobal(L, "Fl_Tree_Item");
		luaopen_FLTK_Fl_Tree_Item_Array(L);
		lua_setglobal(L, "Fl_Tree_Item_Array");
		luaopen_FLTK_Fl_Tree_Prefs(L);
		lua_setglobal(L, "Fl_Tree_Prefs");
		luaopen_FLTK_Fl_Value_Slider(L);
		lua_setglobal(L, "Fl_Value_Slider");
		luaopen_undoLua(L);
		lua_setglobal(L, "undo");
	}else
		fl_alert("lua_newstate failed");
	return L;
}
void runLuaCB(Fl_Widget*,void*){
	char*st;
	if(st=loadsavefile("Select a Lua script",false)){
		char*dup=strdup(st);
#ifdef _WIN32
		_chdir(dirname(dup));
#else
		chdir(dirname(dup));
#endif
		lua_State*L=createLuaState();
		runLua(L,st);
		lua_close(L);
		free(st);
		free(dup);
	}
}
