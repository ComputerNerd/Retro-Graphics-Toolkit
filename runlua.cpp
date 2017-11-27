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

/** @file runlua.cpp
 * This file runlua.cpp contains various functions related to the Lua bindings and for running Lua code
 *  @author Sega16*/
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
#include <sstream>
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
#include "comper.h"
#include "enigma.h"
#include "nemesis.h"
#include "kosinski.h"
#include "saxman.h"
#include "filemisc.h"
#include "level_levDat.h"
#include "level_levelInfo.h"
#include "level_levobjDat.h"
#include "undoLua.h"
#include "posix.h"
#include "callback_tilemap.h"
#include "luafltk.hpp"
#include "luaHelpers.hpp"

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
static int luaL_optboolean (lua_State *L, int narg, int def){
	return lua_isboolean(L, narg) ? lua_toboolean(L, narg) : def;
}
static int lua_palette_sortByHSL(lua_State*L){
	sortBy(luaL_optinteger(L,1,0),luaL_optboolean(L,2,true));
	return 0;
}
static int lua_palette_paletteToRgb(lua_State*L){
	currentProject->pal->paletteToRgb();
	return 0;
}
static int lua_palette_save(lua_State*L){
	//void savePalette(const char*fname,unsigned start,unsigned end,bool skipzero,fileType_t type,int clipboard,const char*label="palDat");
	currentProject->pal->savePalette(lua_tostring(L,1),lua_tointeger(L,2),lua_tointeger(L,3),lua_toboolean(L,4),(fileType_t)lua_tointeger(L,5),lua_toboolean(L,6),lua_tostring(L,7));
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
	{"toRgbAll",lua_palette_paletteToRgb},
	{"save",lua_palette_save},
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
static int lua_tile_compareTileRGBA(lua_State*L){
	unsigned tile1=luaL_optinteger(L,1,0);
	unsigned tile2=luaL_optinteger(L,2,0);
	if(inRangeTile(tile1)&&inRangeTile(tile2)&&(tile1!=tile2)){
		unsigned diffSum=0;
		uint8_t*off1=currentProject->tileC->truetDat.data()+(tile1*currentProject->tileC->tcSize);
		uint8_t*off2=currentProject->tileC->truetDat.data()+(tile2*currentProject->tileC->tcSize);
		for(unsigned i=0;i<currentProject->tileC->tcSize;i+=4){
			int tmp=0;
			for(unsigned j=0;j<3;++j)
				tmp+=*off1++-*off2++;
			++off1;
			++off2;
			diffSum+=tmp*tmp;
		}
		lua_pushinteger(L,diffSum);
		return 1;
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
static int lua_tile_draw(lua_State*L){
	currentProject->tileC->draw_tile(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0),luaL_optinteger(L,5,0),lua_toboolean(L,6),lua_toboolean(L,7),lua_toboolean(L,8),(const uint8_t*)luaL_optinteger(L,9,0),luaL_optinteger(L,10,0),lua_toboolean(L,11));
	return 0;
}
static int lua_tile_remove(lua_State*L){
	currentProject->tileC->remove_tile_at(luaL_optinteger(L,1,0));
	return 0;
}
static int lua_tile_removeDuplicate(lua_State*L){
	currentProject->tileC->remove_duplicate_tiles(lua_toboolean(L,1));
	return 0;
}
static int lua_tile_save(lua_State*L){
	//void save(const char*fname,fileType_t type,bool clipboard,int compression);
	currentProject->tileC->save(lua_tostring(L,1),(fileType_t)lua_tointeger(L,2),lua_toboolean(L,3),lua_tointeger(L,4),lua_tostring(L,5));
	return 0;
}
static const luaL_Reg lua_tileAPI[]={
	{"getPixelRGBA",lua_tile_getPixelRGBA},
	{"setPixelRGBA",lua_tile_setPixelRGBA},
	{"getTileRGBA",lua_tile_getTileRGBA},
	{"setTileRGBA",lua_tile_setTileRGBA},
	{"compareTileRGBA",lua_tile_compareTileRGBA},
	{"dither",lua_tile_dither},
	{"append",lua_tile_append},
	{"resize",lua_tile_resize},
	{"draw",lua_tile_draw},
	{"remove",lua_tile_remove},
	{"removeDuplicate",lua_tile_removeDuplicate},
	{"save",lua_tile_save},
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
static void fixTilemapSizes(lua_State*L,unsigned plane){
	setTableUnsignedLua(L,"tilemaps","width",plane+1,currentProject->tms->maps[plane].mapSizeW);
	setTableUnsignedLua(L,"tilemaps","height",plane+1,currentProject->tms->maps[plane].mapSizeH);
	setTableUnsignedLua(L,"tilemaps","heightA",plane+1,currentProject->tms->maps[plane].mapSizeHA);
}
static int lua_tilemap_resize(lua_State*L){
	unsigned plane=getPlane(L);
	currentProject->tms->maps[plane].resize_tile_map(luaL_optinteger(L,1,1),luaL_optinteger(L,2,1));
	fixTilemapSizes(L,plane);
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
	int row=luaL_optinteger(L,2,-1);
	bool useAlpha=luaL_optinteger(L,3,0);
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
	unsigned len=lua_rawlen(L,2);
	if(!len){
		fl_alert("imageToTiles error: parameter 2 must be a table");
		return 0;
	}
	int row=luaL_optinteger(L,3,-1);
	bool useAlpha=luaL_optinteger(L,4,0);
	bool copyToTruecol=luaL_optinteger(L,5,0);
	bool convert=luaL_optinteger(L,6,1);
	unsigned bpp=useAlpha+3;
	uint32_t w,h;
	w=currentProject->tms->maps[getPlane(L)].mapSizeW*currentProject->tileC->sizew;
	h=currentProject->tms->maps[getPlane(L)].mapSizeHA*currentProject->tileC->sizeh;
	unsigned sz=w*h*bpp;
	uint8_t*image=(uint8_t*)malloc(sz);
	fillucharFromTab(L,2,len,sz,image);
	currentProject->tms->maps[getPlane(L)].truecolorimageToTiles(image,row,useAlpha,copyToTruecol,convert);
	free(image);
	return 0;
}
static int lua_tilemap_drawBlock(lua_State*L){
	currentProject->tms->maps[luaL_optinteger(L,1,0)].drawBlock(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0),luaL_optinteger(L,5,0),luaL_optinteger(L,6,0));
	return 0;
}
static int lua_tilemap_getRaw(lua_State*L){
	lua_pushinteger(L,currentProject->tms->maps[getPlane(L)].getRaw(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_tilemap_setRaw(lua_State*L){
	currentProject->tms->maps[getPlane(L)].setRaw(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_tilemap_removeBlock(lua_State*L){
	unsigned pl=getPlane(L);
	currentProject->tms->maps[pl].removeBlock(luaL_optinteger(L,2,0));
	fixTilemapSizes(L,pl);
	setTableUnsignedLua(L,"tilemaps","amt",pl+1,currentProject->tms->maps[pl].amt);
	if(pl==currentProject->curPlane)
		window->map_amt->value(std::to_string(currentProject->tms->maps[pl].amt).c_str());
	return 0;
}
static int lua_tilemap_subTile(lua_State*L){
	currentProject->tms->maps[getPlane(L)].sub_tile_map(lua_tointeger(L,2),lua_tointeger(L,3),lua_toboolean(L,4),lua_toboolean(L,5));
	return 0;
}
static int lua_tilemap_loadImage(lua_State*L){
	load_image_to_tilemap(lua_tostring(L,1),luaL_optboolean(L,2,false),luaL_optboolean(L,3,false),luaL_optboolean(L,4,false));
	fixTilemapSizes(L,0);
	syncTileAmt(L);
	return 0;
}
static int lua_tilemap_generate_optimal_paletteapply(lua_State *L) {
	try {
		settings *s = *((settings **)dub::checksdata(L, 1, "settings"));
		generate_optimal_paletteapply(nullptr, s);
		return 0;
	} catch (std::exception &e) {
		lua_pushfstring(L, "generate_optimal_paletteapply: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "generate_optimal_paletteapply: Unknown exception");
	}
	return dub::error(L);
}
static int lua_tilemap_save(lua_State*L){
	//bool tileMap::saveToFile(const char*fname,fileType_t type,int clipboard,int compression,const char*nesFname){
	currentProject->tms->maps[getPlane(L)].saveToFile(lua_tostring(L,2),(fileType_t)lua_tointeger(L,3),lua_toboolean(L,4),lua_tointeger(L,5),lua_tostring(L,6),luaL_optstring(L,7,nullptr),luaL_optstring(L,8,nullptr));
	return 0;
}
static int lua_tilemap_pickRowDelta(lua_State*L){
	currentProject->tms->maps[getPlane(L)].pickRowDelta(false,nullptr,lua_tointeger(L,2),lua_tointeger(L,3));
	return 0;
}
static int lua_tilemap_pickRow(lua_State*L){
	currentProject->tms->maps[getPlane(L)].pickRow(lua_tointeger(L,2),lua_tointeger(L,3),lua_tointeger(L,4));
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
	{"drawBlock",lua_tilemap_drawBlock},
	{"getRaw",lua_tilemap_getRaw},
	{"setRaw",lua_tilemap_setRaw},
	{"removeBlock",lua_tilemap_removeBlock},
	{"subTile",lua_tilemap_subTile},
	{"loadImage",lua_tilemap_loadImage},
	{"generatePalette",lua_tilemap_generate_optimal_paletteapply},
	{"save",lua_tilemap_save},
	{"pickRowDelta",lua_tilemap_pickRowDelta},
	{"pickRow",lua_tilemap_pickRow},
	{0,0}
};
/** Set attributes (key, value)
 * 
 */
static int settings__set_(lua_State *L) {

	settings *self = *((settings **)dub::checksdata_n(L, 1, "settings"));
	const char *key = luaL_checkstring(L, 2);
	int key_h = dub::hash(key, 10);
	switch(key_h) {
		case 9: {
				if (DUB_ASSERT_KEY(key, "sprite")) break;
				self->sprite = luaL_checkboolean(L, 3);
				return 0;
			}
		case 0: {
				if (DUB_ASSERT_KEY(key, "alg")) break;
				self->alg = luaL_checkinteger(L, 3);
				return 0;
			}
		case 2: {
				if (DUB_ASSERT_KEY(key, "ditherAfter")) break;
				self->ditherAfter = luaL_checkboolean(L, 3);
				return 0;
			}
		case 5: {
				if (DUB_ASSERT_KEY(key, "entireRow")) break;
				self->entireRow = luaL_checkboolean(L, 3);
				return 0;
			}
		case 8: {
				if (DUB_ASSERT_KEY(key, "colSpace")) break;
				self->colSpace = luaL_checkinteger(L, 3);
				return 0;
			}
		case 1: {
				if (DUB_ASSERT_KEY(key, "rowAuto")) break;
				self->rowAuto = luaL_checkinteger(L, 3);
				return 0;
			}
	}
	if (lua_istable(L, 1)) {
		lua_rawset(L, 1);
	} else {
		luaL_error(L, KEY_EXCEPTION_MSG, key);
	}
	return 0;
}

/** Get attributes (key)
 * 
 */
static int settings__get_(lua_State *L) {

	settings *self = *((settings **)dub::checksdata_n(L, 1, "settings", true));
	const char *key = luaL_checkstring(L, 2);
	puts(lua_typename(L,lua_type(L,0)));
	puts(lua_typename(L,lua_type(L,1)));
	puts(lua_typename(L,lua_type(L,2)));
	puts(lua_typename(L,lua_type(L,3)));
	puts(lua_typename(L,lua_type(L,4)));
	puts("__END__");
	// <self> "key" <mt>
	// rawget(mt, key)
	lua_pushvalue(L, 2);
	// <self> "key" <mt> "key"
	lua_rawget(L, -2);
	if (!lua_isnil(L, -1)) {
		// Found method.
		return 1;
	} else {
		// Not in mt = attribute access.
		lua_pop(L, 2);
	}
	int key_h = dub::hash(key, 10);
	switch(key_h) {
		case 9: {
				if (DUB_ASSERT_KEY(key, "sprite")) break;
				lua_pushboolean(L, self->sprite);
				return 1;
			}
		case 0: {
				if (DUB_ASSERT_KEY(key, "alg")) break;
				lua_pushinteger(L, self->alg);
				return 1;
			}
		case 2: {
				if (DUB_ASSERT_KEY(key, "ditherAfter")) break;
				lua_pushboolean(L, self->ditherAfter);
				return 1;
			}
		case 5: {
				if (DUB_ASSERT_KEY(key, "entireRow")) break;
				lua_pushboolean(L, self->entireRow);
				return 1;
			}
		case 8: {
				if (DUB_ASSERT_KEY(key, "colSpace")) break;
				lua_pushinteger(L, self->colSpace);
				return 1;
			}
		case 1: {
				if (DUB_ASSERT_KEY(key, "rowAuto")) break;
				lua_pushinteger(L, self->rowAuto);
				return 1;
			}
	}
	return 0;
}

/** settings()
 * 
 */
static int settings_settings(lua_State *L) {
	try {
		settings *retval__ = new settings();
		dub::pushudata(L, retval__, "settings", true);
		return 1;
	} catch (std::exception &e) {
		lua_pushfstring(L, "new: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "new: Unknown exception");
	}
	return dub::error(L);
}

/** Read off(size_t i)
 * 
 */
static int settings_off(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);
		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			unsigned v = dub::checkinteger(L, 3);
			if (!i || i > MAX_ROWS_PALETTE) return 0;
			self->off[i-1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);
			if (!i || i > MAX_ROWS_PALETTE) return 0;
			lua_pushinteger(L, self->off[i-1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "off: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "off: Unknown exception");
	}
	return dub::error(L);
}

/** Read perRow(size_t i)
 * 
 */
static int settings_perRow(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);
		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			unsigned v = dub::checkinteger(L, 3);
			if (!i || i > MAX_ROWS_PALETTE) return 0;
			self->perRow[i-1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);
			if (!i || i > MAX_ROWS_PALETTE) return 0;
			lua_pushinteger(L, self->perRow[i-1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "perRow: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "perRow: Unknown exception");
	}
	return dub::error(L);
}

/** Read useRow(size_t i)
 * 
 */
static int settings_useRow(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);
		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			bool v = dub::checkboolean(L, 3);
			if (!i || i > MAX_ROWS_PALETTE) return 0;
			self->useRow[i-1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);
			if (!i || i > MAX_ROWS_PALETTE) return 0;
			lua_pushboolean(L, self->useRow[i-1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "useRow: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "useRow: Unknown exception");
	}
	return dub::error(L);
}

/** Read rowAutoEx(size_t i)
 * 
 */
static int settings_rowAutoEx(lua_State *L) {
	try {
		settings *self = *((settings **)dub::checksdata(L, 1, "settings"));
		int top__ = lua_gettop(L);
		if (top__ >= 3) {
			size_t i = dub::checkinteger(L, 2);
			int v = dub::checkinteger(L, 3);
			if (!i || i > 2) return 0;
			self->rowAutoEx[i-1] = v;
			return 0;
		} else {
			size_t i = dub::checkinteger(L, 2);
			if (!i || i > 2) return 0;
			lua_pushinteger(L, self->rowAutoEx[i-1]);
			return 1;
		}
	} catch (std::exception &e) {
		lua_pushfstring(L, "rowAutoEx: %s", e.what());
	} catch (...) {
		lua_pushfstring(L, "rowAutoEx: Unknown exception");
	}
	return dub::error(L);
}



// --=============================================== __tostring
static int settings___tostring(lua_State *L) {
  settings *self = *((settings **)dub::checksdata_n(L, 1, "settings"));
  lua_pushfstring(L, "settings: %p", self);
  
  return 1;
}

// --=============================================== METHODS

static const struct luaL_Reg settings_member_methods[] = {
	{ "__newindex"   , settings__set_       },
	{ "__index"      , settings__get_       },
	{ "new"          , settings_settings    },
	{ "off"          , settings_off         },
	{ "perRow"       , settings_perRow      },
	{ "useRow"       , settings_useRow      },
	{ "rowAutoEx"    , settings_rowAutoEx   },
	{ "__tostring"   , settings___tostring  },
	{ "deleted"      , dub::isDeleted       },
	{ NULL, NULL},
};

int luaopen_settings(lua_State *L)
{
	// Create the metatable which will contain all the member methods
	luaL_newmetatable(L, "settings");
	// <mt>

	// register member methods
	dub::fregister(L, settings_member_methods);
	// setup meta-table
	dub::setup(L, "settings");
	// <mt>
	return 1;
}

static int sprite__set_(lua_State *L) {
	sprite *self = *((sprite **)dub::checksdata(L, 1, "sprite"));
	const char *key = luaL_checkstring(L, 2);
	if(!strcmp(key,"w")){
		self->w=luaL_checkinteger(L,3);
		return 0;
	}
	if(!strcmp(key,"h")){
		self->h=luaL_checkinteger(L,3);
		return 0;
	}
	if(!strcmp(key,"starttile")){
		self->starttile=luaL_checkinteger(L,3);
		return 0;
	}
	if(!strcmp(key,"offx")){
		self->offx=luaL_checkinteger(L,3);
		return 0;
	}
	if(!strcmp(key,"offy")){
		self->offy=luaL_checkinteger(L,3);
		return 0;
	}
	if(!strcmp(key,"loadat")){
		self->loadat=luaL_checkinteger(L,3);
		return 0;
	}
	if(!strcmp(key,"palrow")){
		self->palrow=luaL_checkinteger(L,3);
		return 0;
	}
	if(!strcmp(key,"hflip")){
		self->hflip=lua_toboolean(L,3);
		return 0;
	}
	if(!strcmp(key,"vflip")){
		self->vflip=lua_toboolean(L,3);
		return 0;
	}
	if(!strcmp(key,"prio")){
		self->prio=lua_toboolean(L,3);
		return 0;
	}
	return 0;
}
static int sprite__get_(lua_State *L) {
	sprite *self = *((sprite **)dub::checksdata_n(L, 1, "sprite", true));
	int type=lua_type(L,2);
	if(type==LUA_TNUMBER){
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n",k);
	}else if(type==LUA_TSTRING){
		const char*k=luaL_checkstring(L, 2);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if (!lua_isnil(L, -1)) {
			return 1;
		} else {
			lua_pop(L, 2);
		}
		printf("k=%s\n",k);
		if(!strcmp("w",k)){
			lua_pushinteger(L,self->w);
			return 1;
		}
		if(!strcmp("h",k)){
			lua_pushinteger(L,self->h);
			return 1;
		}
		if(!strcmp("palrow",k)){
			lua_pushinteger(L,self->palrow);
			return 1;
		}
		if(!strcmp("starttile",k)){
			lua_pushinteger(L,self->starttile);
			return 1;
		}
		if(!strcmp("offx",k)){
			lua_pushinteger(L,self->offx);
			return 1;
		}
		if(!strcmp("offy",k)){
			lua_pushinteger(L,self->offy);
			return 1;
		}
		if(!strcmp("loadat",k)){
			lua_pushinteger(L,self->loadat);
			return 1;
		}
		if(!strcmp("prio",k)){
			lua_pushboolean(L,self->prio);
			return 1;
		}
		if(!strcmp("hflip",k)){
			lua_pushboolean(L,self->hflip);
			return 1;
		}
		if(!strcmp("vflip",k)){
			lua_pushboolean(L,self->vflip);
			return 1;
		}
	}
	return 0;
}
static int sprite___tostring(lua_State *L) {
	sprite *self = *((sprite **)dub::checksdata(L, 1, "sprite"));
	lua_pushfstring(L, "sprite table: %p\n\tWidth: %d\n\tHeight: %d\n\tPalette row: %d\n\tHorizontal flip: %s\n\tVertical flip: %s\n\tHigh priority: %s",self,self->w,self->h,self->palrow,(self->hflip?"true":"false"),(self->vflip?"true":"false"),(self->prio?"true":"false"));
	return 1;
}
static const struct luaL_Reg sprite_member_methods[] = {
  //{ "new"          , settings_settings    },
  { "__newindex"   , sprite__set_       },
  { "__index"      , sprite__get_       },
  { "__tostring"   , sprite___tostring  },
  { "deleted"      , dub::isDeleted       },
  { NULL, NULL},
};
int luaopen_sprite(lua_State *L,class spriteGroup*self,unsigned idx)
{
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "sprite");
  // <mt>

  // register member methods
  dub::fregister(L, sprite_member_methods);
  // setup meta-table
  dub::setup(L, "sprite");
  // <mt>
  dub::pushudata(L,&self->list[idx], "sprite", true);
  return 1;
}



static int spriteGroup__set_(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata(L, 1, "spriteGroup"));
	const char *key = luaL_checkstring(L, 2);
	if(!strcmp(key,"name"))
		self->name.assign(luaL_checkstring(L,3));
	return 0;
}
static int spriteGroup__get_(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata_n(L, 1, "spriteGroup",true));
	int type=lua_type(L,2);
	if(type==LUA_TNUMBER){
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n",k);
		if(k>=1&&k<=self->list.size()){
			luaopen_sprite(L,self,k-1);
			return 1;
		}
	}else if(type==LUA_TSTRING){
		const char*k=luaL_checkstring(L, 2);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if (!lua_isnil(L, -1)) {
			return 1;
		} else {
			lua_pop(L, 2);
		}
		printf("k=%s\n",k);
		if(!strcmp("name",k)){
			lua_pushstring(L,self->name.c_str());
			return 1;
		}
	}
	return 0;
}
static int spriteGroup__len_(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata(L, 1, "spriteGroup"));
	lua_pushinteger(L,self->list.size());
	return 1;
}
static int spriteGroup___tostring(lua_State *L) {
	spriteGroup *self = *((spriteGroup **)dub::checksdata(L, 1, "spriteGroup"));
	lua_pushfstring(L, "sprite group table: %p\nNamed: %s",self,self->name.c_str());
	return 1;
}
static const struct luaL_Reg spriteGroup_member_methods[] = {
  //{ "new"          , settings_settings    },
  { "__newindex"   , spriteGroup__set_       },
  { "__index"      , spriteGroup__get_       },
  { "__len"      , spriteGroup__len_       },
  { "__tostring"   , spriteGroup___tostring  },
  { "deleted"      , dub::isDeleted       },
  { NULL, NULL},
};
int luaopen_spriteGroup(lua_State *L,class sprites*self,unsigned idx)
{
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "spriteGroup");
  // <mt>

  // register member methods
  dub::fregister(L, spriteGroup_member_methods);
  // setup meta-table
  dub::setup(L, "spriteGroup");
  // <mt>
  dub::pushudata(L,&self->groups[idx], "spriteGroup", true);
  return 1;
}


static int spriteGroups__set_(lua_State *L) {
	sprites *self = *((sprites **)dub::checksdata(L, 1, "spriteGroups"));
	const char *key = luaL_checkstring(L, 2);
	if(!strcmp(key,"name"))
		self->name.assign(luaL_checkstring(L,3));
	return 0;
}
static int spriteGroups__get_(lua_State *L) {
	sprites *self = *((sprites **)dub::checksdata_n(L, 1, "spriteGroups", true));
	int type=lua_type(L,2);
	if(type==LUA_TNUMBER){
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n",k);
		if(k>=1&&k<=self->groups.size()){
			luaopen_spriteGroup(L,self,k-1);
			return 1;
		}
	}else if(type==LUA_TSTRING){
		const char*k=luaL_checkstring(L, 2);
		puts(lua_typename(L,lua_type(L,0)));
		puts(lua_typename(L,lua_type(L,1)));
		puts(lua_typename(L,lua_type(L,2)));
		puts(lua_typename(L,lua_type(L,3)));
		puts(lua_typename(L,lua_type(L,4)));
		puts("__END__");
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if (!lua_isnil(L, -1)) {
			return 1;
		} else {
			lua_pop(L, 2);
		}
		printf("k=%s\n",k);
		if(!strcmp("name",k)){
			lua_pushstring(L,self->name.c_str());
			return 1;
		}
	}
	return 0;
}
static int spriteGroups__len_(lua_State *L) {
	class sprites *self = *((class sprites **)dub::checksdata(L, 1, "spriteGroups"));
	lua_pushinteger(L,self->groups.size());
	return 1;
}
static int spriteGroups___tostring(lua_State *L) {
	class sprites *self = *((class sprites **)dub::checksdata(L, 1, "spriteGroups"));
	lua_pushfstring(L, "sprite groups table: %p\nNamed: %s",self,self->name.c_str());
	return 1;
}
static int spriteGroups_importSpriteSheet(lua_State *L) {
	class sprites *self = *((class sprites **)dub::checksdata(L, 1, "spriteGroups"));
	self->importSpriteSheet(luaL_checkstring(L,2));
	return 0;
}
static const struct luaL_Reg spriteGroups_member_methods[] = {
  //{ "new"          , settings_settings    },
  { "__newindex"   , spriteGroups__set_       },
  { "__index"      , spriteGroups__get_       },
  { "__len"      , spriteGroups__len_       },
  { "__tostring"   , spriteGroups___tostring  },
  { "importSpriteSheet", spriteGroups_importSpriteSheet       },
  { "deleted"      , dub::isDeleted       },
  { NULL, NULL},
};
int luaopen_spriteGroups(lua_State *L,unsigned idx)
{
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "spriteGroups");
  // <mt>

  // register member methods
  dub::fregister(L, spriteGroups_member_methods);
  // setup meta-table
  dub::setup(L, "spriteGroups");
  // <mt>
  dub::pushudata(L,&currentProject->ms->sps[idx], "spriteGroups", true);
  return 1;
}


static int sprites__set_(lua_State *L) {
	const char *key = luaL_checkstring(L, 2);
	if(!strcmp(key,"name"))
		currentProject->ms->name.assign(luaL_checkstring(L,3));
	return 0;
}
static int sprites__get_(lua_State *L) {
	int type=lua_type(L,2);
	if(type==LUA_TNUMBER){
		int k = luaL_checkinteger(L, 2);
		printf("k=%d\n",k);
		if(k>=1&&k<=currentProject->ms->sps.size()){
			luaopen_spriteGroups(L,k-1);
			return 1;
		}
	}else if(type==LUA_TSTRING){
		const char*k=luaL_checkstring(L, 2);
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if (!lua_isnil(L, -1)) {
			return 1;
		} else {
			lua_pop(L, 2);
		}
		printf("k=%s\n",k);
		if(!strcmp("name",k)){
			lua_pushstring(L,currentProject->ms->name.c_str());
			return 1;
		}
	}
	return 0;
}
static int sprites__len_(lua_State *L) {
	lua_pushinteger(L,currentProject->ms->sps.size());
	return 1;
}
static int sprites___tostring(lua_State *L) {
  lua_pushfstring(L, "metasprite table: %p\nNamed: %s",currentProject->ms,currentProject->ms->name.c_str());
  
  return 1;
}
static const struct luaL_Reg sprites_member_methods[] = {
  //{ "new"          , settings_settings    },
  { "__newindex"   , sprites__set_       },
  { "__index"      , sprites__get_       },
  { "__len"      , sprites__len_       },
  { "__tostring"   , sprites___tostring  },
  { "deleted"      , dub::isDeleted       },
  { NULL, NULL},
};
int luaopen_sprites(lua_State *L)
{
  // Create the metatable which will contain all the member methods
  luaL_newmetatable(L, "sprites");
  // <mt>

  // register member methods
  dub::fregister(L, sprites_member_methods);
  // setup meta-table
  dub::setup(L, "sprites");
  // <mt>
  dub::pushudata(L, currentProject->ms, "sprites", true);
  return 1;
}
static int lua_chunk_draw(lua_State*L){
	currentProject->Chunk->drawChunk(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0),luaL_optinteger(L,5,0),luaL_optinteger(L,6,0));
	return 0;
}
static int lua_chunk_getBlocks(lua_State*L){
	lua_pushinteger(L,currentProject->Chunk->getBlock(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_chunk_setBlocks(lua_State*L){
	currentProject->Chunk->setBlock(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_chunk_getFlags(lua_State*L){
	lua_pushinteger(L,currentProject->Chunk->getFlag(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_chunk_setFlags(lua_State*L){
	currentProject->Chunk->setFlag(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0),luaL_optinteger(L,4,0));
	return 0;
}
static int lua_chunk_setUseBlocks(lua_State*L){
	currentProject->Chunk->useBlocks=lua_toboolean(L,1);
	return 0;
}
static int lua_chunk_getUseBlocks(lua_State*L){
	lua_pushinteger(L,currentProject->Chunk->useBlocks);
	return 1;
}
static int lua_chunk_setBlockUse(lua_State*L){
	unsigned tmp=luaL_optinteger(L,1,0);
	size_t sz=currentProject->tms->maps.size();
	if(tmp>=sz)
		tmp=sz-1;
	currentProject->Chunk->usePlane=tmp;
	return 0;
}
static int lua_chunk_getBlockUse(lua_State*L){
	lua_pushinteger(L,currentProject->Chunk->usePlane);
	return 1;
}
static int lua_chunk_resize(lua_State*L){
	currentProject->Chunk->resize(luaL_optinteger(L,1,1),luaL_optinteger(L,2,1));
	return 0;
}
static int lua_chunk_setAmt(lua_State*L){
	currentProject->Chunk->resizeAmt(luaL_optinteger(L,1,1));
	return 0;
}
static int lua_chunk_subBlock(lua_State*L){
	currentProject->Chunk->subBlock(lua_tointeger(L,1),lua_tointeger(L,2));
	return 0;
}
static int lua_chunk_removeAt(lua_State*L){
	currentProject->Chunk->removeAt(lua_tointeger(L,1));
	return 0;
}
static const luaL_Reg lua_chunkAPI[]={
	{"getBlocks",lua_chunk_getBlocks},
	{"getFlags",lua_chunk_getFlags},
	{"setBlocks",lua_chunk_setBlocks},
	{"setFlags",lua_chunk_setFlags},
	{"setUseBlocks",lua_chunk_setUseBlocks},
	{"getUseBlocks",lua_chunk_getUseBlocks},
	{"setBlockUse",lua_chunk_setBlockUse},
	{"getBlockUse",lua_chunk_getBlockUse},
	{"resize",lua_chunk_resize},
	{"setAmt",lua_chunk_setAmt},
	{"draw",lua_chunk_draw},
	{"subBlock",lua_chunk_subBlock},
	{"removeAt",lua_chunk_removeAt},
	{0,0}
};
static int lua_sprite_ditherGroup(lua_State*L){
	ditherSpriteAsImage(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0));
	return 0;
}
static int lua_sprite_ditherGroupAll(lua_State*L){
	ditherGroupAsImage(luaL_optinteger(L,1,0));
	return 0;
}
static int lua_sprite_ditherAll(lua_State*L){
	for(unsigned i=0;i<currentProject->ms->sps.size();++i)
		ditherGroupAsImage(i);
	return 0;
}
static int lua_sprite_draw(lua_State*L){
	int32_t outx,outy;
	projects[luaL_optinteger(L,1,0)]->ms->sps[luaL_optinteger(L,2,0)].draw(luaL_optinteger(L,3,0),luaL_optinteger(L,4,0),luaL_optinteger(L,5,0),luaL_optinteger(L,6,0),lua_toboolean(L,7),&outx,&outy);
	lua_pushinteger(L,outx);
	lua_pushinteger(L,outy);
	return 2;
}
static const luaL_Reg lua_spriteAPI[]={
	{"ditherGroup",lua_sprite_ditherGroup},
	{"ditherGroupAll",lua_sprite_ditherGroupAll},
	{"ditherAll",lua_sprite_ditherAll},
	{"draw",lua_sprite_draw},
	{0,0}
};
static void updateLevelTable(lua_State*L);
static int lua_level_setLayerAmt(lua_State*L){
	currentProject->lvl->setlayeramt(luaL_optinteger(L,1,0),lua_toboolean(L,2));
	updateLevelTable(L);
	return 0;
}
static int lua_level_setLayerName(lua_State*L){
	currentProject->lvl->layernames[luaL_optinteger(L,1,0)].assign(luaL_optstring(L,2,""));
	updateLevelTable(L);
	return 0;
}
static int lua_level_getInfo(lua_State*L){
	luaopen_level_levelInfo(L,currentProject->lvl->getInfo(luaL_optinteger(L,1,0)));
	return 1;
}
static int lua_level_getObj(lua_State*L){
	luaopen_level_levobjDat(L,currentProject->lvl->getObjDat(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0)));
	return 1;
}
static int lua_level_getXY(lua_State*L){
	luaopen_level_levDat(L,currentProject->lvl->getlevDat(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)));
	return 1;
}
static int lua_level_appendObj(lua_State*L){
	currentProject->lvl->odat[luaL_optinteger(L,1,0)]->emplace_back();
	updateLevelTable(L);
	return 0;
}
static int lua_level_delObj(lua_State*L){
	unsigned idx=luaL_optinteger(L,1,0);
	currentProject->lvl->odat[idx]->erase(currentProject->lvl->odat[idx]->begin()+luaL_optinteger(L,2,0));
	updateLevelTable(L);
	return 0;
}
static int lua_level_removeLayer(lua_State*L){
	currentProject->lvl->removeLayer(luaL_optinteger(L,1,0));
	updateLevelTable(L);
	return 0;
}
static int lua_level_resizeLayer(lua_State*L){
	currentProject->lvl->resizeLayer(luaL_optinteger(L,1,0),luaL_optinteger(L,2,0),luaL_optinteger(L,3,0));
	return 0;
}
static int lua_level_subType(lua_State*L){
	currentProject->lvl->subType(lua_tointeger(L,1),lua_tointeger(L,2),(enum source)lua_tointeger(L,3),lua_tointeger(L,4));
	return 0;
}
static const luaL_Reg lua_levelAPI[]={
	{"setLayerAmt",lua_level_setLayerAmt},
	{"setLayerName",lua_level_setLayerName},
	{"getInfo",lua_level_getInfo},
	{"getObj",lua_level_getObj},
	{"getXY",lua_level_getXY},
	{"appendObj",lua_level_appendObj},
	{"delObj",lua_level_delObj},
	{"removeLayer",lua_level_removeLayer},
	{"resizeLayer",lua_level_resizeLayer},
	{"subType",lua_level_subType},
	{0,0}
};
static const struct dub::const_Reg level_const[] = {
	{ "TILES"        , ::TILES              },
	{ "BLOCKS"       , ::BLOCKS             },
	{ "CHUNKS"       , ::CHUNKS             },
	{ NULL, 0},
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
	currentProject->haveMessage(mask);
	return 0;
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
static int lua_project_update(lua_State*L){
	switchProjectSlider(curProjectID);
	updateProjectTablesLua(L);
	return 0;
}
static int lua_project_getPalTab(lua_State*L){
	lua_pushinteger(L,currentProject->getPalTab());
	return 1;
}
static int lua_project_setPalTab(lua_State*L){
	currentProject->setPalTab(luaL_optinteger(L,1,0));
	return 0;
}
static int lua_project_load(lua_State*L){
	loadProject(lua_tointeger(L,2),lua_tostring(L,1));
	return 0;
}
static int lua_project_save(lua_State*L){
	saveProject(lua_tointeger(L,2),lua_tostring(L,1));
	return 0;
}
static int lua_project_append(lua_State*L){
	appendProject();
	return 0;
}
static int lua_project_remove(lua_State*L){
	removeProject(lua_tointeger(L,1));
	return 0;
}
static int lua_project_setSystem(lua_State*L){
	set_game_system(nullptr,(void*)(uintptr_t)lua_tointeger(L,1));
	return 0;
}
static const luaL_Reg lua_projectAPI[]={/*!This is the project table. The global project contains the following functions*/
	{"have",lua_project_rgt_have},
	{"haveOR",lua_project_rgt_haveOR},
	{"haveMessage",lua_project_rgt_haveMessage},
	{"set",lua_project_set},
	{"update",lua_project_update},
	{"getSettings",lua_project_getSettings},
	{"setSettings",lua_project_setSettings},
	{"getPalTab",lua_project_getPalTab},
	{"setPalTab",lua_project_setPalTab},
	{"load",lua_project_load},
	{"save",lua_project_save},
	{"append",lua_project_append},
	{"remove",lua_project_remove},
	{"setSystem",lua_project_setSystem},
	{0,0}
};
static void updateLevelTable(lua_State*L){
	lua_pushnil(L);
	lua_setglobal(L,"level");
	if(currentProject->containsData(pjHaveLevel)){
		lua_createtable(L, 0,(arLen(lua_levelAPI)-1)+(arLen(level_const)-1)+3);
		luaL_setfuncs(L,lua_levelAPI,0);
		dub::register_const(L, level_const);
		lua_pushstring(L,"names");
		lua_createtable(L,currentProject->lvl->layernames.size(),0);
		for(unsigned i=0;i<currentProject->lvl->layernames.size();++i){
			lua_pushstring(L,currentProject->lvl->layernames[i].c_str());
			lua_rawseti(L,-2,i+1);
		}
		lua_rawset(L,-3);
		mkKeyunsigned(L,"amt",currentProject->lvl->layeramt);
		lua_pushstring(L,"objamt");
		lua_createtable(L,currentProject->lvl->layeramt,0);
		for(unsigned i=0;i<currentProject->lvl->layernames.size();++i){
			lua_pushinteger(L,currentProject->lvl->odat[i]->size());
			lua_rawseti(L,-2,i+1);
		}
		lua_rawset(L,-3);
		lua_setglobal(L,"level");
	}
}
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
		lua_createtable(L, 0,(sizeof(lua_tilemapAPI)/sizeof((lua_tilemapAPI)[0]) - 1)+5);
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
		lua_rawset(L,-3);

		lua_pushstring(L,"useBlocks");
		lua_createtable(L,currentProject->tms->maps.size(),0);
		for(unsigned i=0;i<currentProject->tms->maps.size();++i){
			lua_pushboolean(L,currentProject->tms->maps[i].isBlock);
			lua_rawseti(L,-2,i+1);
		}
		lua_rawset(L,-3);

		lua_pushstring(L,"amt");
		lua_createtable(L,currentProject->tms->maps.size(),0);
		for(unsigned i=0;i<currentProject->tms->maps.size();++i){
			lua_pushinteger(L,currentProject->tms->maps[i].amt);
			lua_rawseti(L,-2,i+1);
		}
		lua_rawset(L,-3);

		mkKeyunsigned(L,"current",currentProject->curPlane);

		lua_setglobal(L, "tilemaps");
	}
	lua_pushnil(L);
	lua_setglobal(L, "chunks");
	if(currentProject->containsData(pjHaveChunks)){
		lua_createtable(L, 0,arLen(lua_chunkAPI)-1+5);
		luaL_setfuncs(L,lua_chunkAPI,0);
		mkKeyunsigned(L,"amt",currentProject->Chunk->amt);
		mkKeyunsigned(L,"width",currentProject->Chunk->wi);
		mkKeyunsigned(L,"height",currentProject->Chunk->hi);
		mkKeyunsigned(L,"usePlane",currentProject->Chunk->usePlane);
		mkKeybool(L,"useBlocks",currentProject->Chunk->useBlocks);
		lua_setglobal(L, "chunks");
	}
	lua_pushnil(L);
	lua_setglobal(L, "metasprites");
	lua_pushnil(L);
	lua_setglobal(L, "sprites");
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
		luaopen_sprites(L);
		lua_setglobal(L, "sprites");
	}

	updateLevelTable(L);

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
	mkKeyunsigned(L,"gameGear",gameGear);
	mkKeyunsigned(L,"masterSystem",masterSystem);
	mkKeyunsigned(L,"count",projects_count);
	mkKeyunsigned(L,"curProjectID",curProjectID);
	lua_setglobal(L, "project");
}
static int lua_project_set(lua_State*L){
	unsigned off=luaL_optinteger(L,1,0);
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
static int lua_rgt_syncProject(lua_State*L){
	updateProjectTablesLua(L);
	return 0;
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
static const struct keyPairi rgtConsts[]={
	{"paletteTab",pal_edit},
	{"tileTab",tile_edit},
	{"planeTab",tile_place},
	{"chunkTab",chunkEditor},
	{"spritesTab",spriteEditor},
	{"levelTab",levelEditor},
	{"settingsTab",settingsTab},
	{"luaTab",luaTab},
	{"tCancle",tCancle},
	{"tBinary",tBinary},
	{"tCheader",tCheader},
	{"tASM",tASM},
	{"tBEX",tBEX}
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
static void tableToSS(lua_State*L,unsigned idx,std::stringstream&ss){
	int len=lua_rawlen(L,idx);
	for(int i=1;i<=len;++i){
		lua_rawgeti(L,idx,i);
		int tmp=lua_tointeger(L,-1);
		if(tmp<0)
			tmp=0;
		if(tmp>255)
			tmp=255;
		lua_pop(L,1);
		unsigned char tmpc=tmp;
		ss<<tmpc;
	}
	ss.seekp(0,ss.beg);
}
static void SStoTable(lua_State*L,std::stringstream&ss){
	lua_newtable(L);
	int idx=0;
	ss.seekg(0,ss.beg);
	while(!ss.eof()){
		unsigned char c;
		ss>>c;
		lua_pushinteger(L,c);
		lua_rawseti(L,-2,++idx);
	}
}
#define mkDecompressEx(name,ex) static int lua_##name##Decompress(lua_State*L){ \
	std::stringstream ss; \
	if(lua_type(L,1)==LUA_TSTRING){ \
		std::ifstream ifs (lua_tostring(L,1), std::ifstream::in|std::ifstream::binary); \
		name decoder; \
		decoder.decode(ifs,ss,ex); \
		SStoTable(L,ss); \
		return 1; \
	}else if(lua_type(L,1)==LUA_TTABLE){ \
		std::stringstream ss_src; \
		tableToSS(L,1,ss_src); \
		name decoder; \
		decoder.decode(ss_src,ss,ex); \
		SStoTable(L,ss); \
		return 1; \
	}else \
		return 0; \
}
#define SINGLE_ARG(...) __VA_ARGS__
mkDecompressEx(comper,luaL_optinteger(L,2,0))
mkDecompressEx(enigma,SINGLE_ARG(luaL_optinteger(L,2,0),lua_toboolean(L,3)))
mkDecompressEx(nemesis,luaL_optinteger(L,2,0))
mkDecompressEx(kosinski,SINGLE_ARG(luaL_optinteger(L,2,0),lua_toboolean(L,3),luaL_optinteger(L,4,16)))
mkDecompressEx(saxman,SINGLE_ARG(luaL_optinteger(L,2,0),luaL_optinteger(L,3,0)))

#define mkCompress(name) static int lua_##name##Compress(lua_State*L){ \
	if(lua_type(L,1)==LUA_TTABLE){ \
		std::stringstream ss_src; \
		tableToSS(L,1,ss_src); \
		name encoder; \
		if(lua_type(L,2)==LUA_TSTRING){ \
			std::ofstream ofs(lua_tostring(L,2), std::ofstream::out|std::ofstream::binary); \
			encoder.encode(ss_src,ofs); \
			return 0; \
		}else{ \
			std::stringstream ss; \
			encoder.encode(ss_src,ss); \
			SStoTable(L,ss); \
			return 1; \
		} \
	}else \
		return 0; \
}
#define mkCompressEx(name,ex) static int lua_##name##Compress(lua_State*L){ \
	if(lua_type(L,1)==LUA_TTABLE){ \
		std::stringstream ss_src; \
		tableToSS(L,1,ss_src); \
		name encoder; \
		if(lua_type(L,2)==LUA_TSTRING){ \
			std::ofstream ofs(lua_tostring(L,2), std::ofstream::out|std::ofstream::binary); \
			encoder.encode(ss_src,ofs,ex); \
			return 0; \
		}else{ \
			std::stringstream ss; \
			encoder.encode(ss_src,ss,ex); \
			SStoTable(L,ss); \
			return 1; \
		} \
	}else \
		return 0; \
}
mkCompress(comper)
mkCompressEx(enigma,lua_toboolean(L,2))
mkCompressEx(kosinski,SINGLE_ARG(luaL_optinteger(L,2,8192),luaL_optinteger(L,3,256),lua_toboolean(L,4),luaL_optinteger(L,5,0x1000),luaL_optinteger(L,6,16)))
mkCompress(nemesis)
mkCompressEx(saxman,lua_toboolean(L,2))
static const luaL_Reg lua_kensAPI[]={
	{"comperDecompress",lua_comperDecompress},
	{"comperCompress",lua_comperCompress},
	{"enigmaDecompress",lua_enigmaDecompress},
	{"enigmaCompress",lua_enigmaCompress},
	{"nemesisDecompress",lua_nemesisDecompress},
	{"nemesisCompress",lua_nemesisCompress},
	{"kosinskiCcompress",lua_kosinskiCompress},
	{"kosinskiDecompress",lua_kosinskiDecompress},
	{"saxmanDecompress",lua_saxmanDecompress},
	{"saxmanCompress",lua_saxmanCompress},
	{0,0}
};
const char*getTraceback(lua_State*L){
	
}
void runLuaFunc(lua_State*L,unsigned args,unsigned results){
	try{
		if (lua_pcall(L, args, results, 0) != LUA_OK){
			luaL_error(L, "error: %s",lua_tostring(L, -1));
		}
	}catch(std::exception &e){
		fl_alert("Lua error: %s\nlua_tostring(): \%s", e.what(),lua_tostring(L,-1));
	}catch(...){
		fl_alert("Lua error while running script\nthrow was called and the exception is unknown\nlua_tostring(): %s",lua_tostring(L,-1));
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
	}catch(std::exception &e){
		fl_alert("Lua error: %s\nlua_tostring(): \%s", e.what(),lua_tostring(L,-1));
	}catch(...){
		fl_alert("Lua error while running script\nthrow was called and the exception is unknown\nlua_tostring(): %s",lua_tostring(L,-1));
	}
}
lua_State*createLuaState(void){
	lua_State *L = lua_newstate(l_alloc, NULL);
	if(L){
		lua_atpanic(L, &panic);
		luaL_openlibs(L);
		
		createFLTKbindings(L);

		updateProjectTablesLua(L);

		lua_createtable(L,0,arLen(rgtConsts)+arLen(lua_rgtAPI)-1);
		luaL_setfuncs(L,lua_rgtAPI,0);
		for(unsigned x=0;x<arLen(rgtConsts);++x)
			mkKeyint(L,rgtConsts[x].key,rgtConsts[x].pair);
		mkKeyunsigned(L,"Binary",tBinary);
		mkKeyunsigned(L,"Cheader",tCheader);
		mkKeyunsigned(L,"ASM",tASM);
		mkKeyunsigned(L,"BEX",tBEX);
		lua_setglobal(L, "rgt");

		luaL_newlib(L,lua_kensAPI);
		lua_setglobal(L, "kens");

		luaopen_zlib(L);
		lua_setglobal(L, "zlib");

		luaopen_undoLua(L);
		lua_setglobal(L, "undo");

#ifndef __MINGW32__
		luaopen_posix_sys_time(L);
		lua_setglobal(L, "time");
		luaopen_posix_sys_msg(L);
		lua_setglobal(L, "msg");
		luaopen_posix_sys_times(L);
		lua_setglobal(L, "times");
		luaopen_posix_sys_resource(L);
		lua_setglobal(L, "resource");
		luaopen_posix_sys_utsname(L);
		lua_setglobal(L, "utsname");
		luaopen_posix_sys_wait(L);
		lua_setglobal(L, "wait");
		luaopen_posix_sys_stat(L);
		lua_setglobal(L, "stat");
		luaopen_posix_sys_socket(L);
		lua_setglobal(L, "socket");
		//luaopen_posix_sys_statvfs(L);
		luaopen_posix_grp(L);
		lua_setglobal(L, "grp");
		luaopen_posix_time(L);
		lua_setglobal(L, "time");
		luaopen_posix_dirent(L);
		lua_setglobal(L, "dirent");
		luaopen_posix_glob(L);
		lua_setglobal(L, "glob");
		luaopen_posix_syslog(L);
		lua_setglobal(L, "syslog");
		luaopen_posix_stdlib(L);
		lua_setglobal(L, "stdlib");
		luaopen_posix_termio(L);
		lua_setglobal(L, "termio");
		luaopen_posix_ctype(L);
		lua_setglobal(L, "ctype");
		luaopen_posix_fcntl(L);
		lua_setglobal(L, "fcntl");
		luaopen_posix_poll(L);
		lua_setglobal(L, "poll");
		luaopen_posix_signal(L);
		lua_setglobal(L, "signal");
		luaopen_posix_utime(L);
		lua_setglobal(L, "utime");
		luaopen_posix_pwd(L);
		lua_setglobal(L, "pwd");
		luaopen_posix_errno(L);
		lua_setglobal(L, "errno");
		luaopen_posix_stdio(L);
		lua_setglobal(L, "stdio");
		luaopen_posix_sched(L);
		lua_setglobal(L, "sched");
		luaopen_posix_fnmatch(L);
		lua_setglobal(L, "fnmatch");
#endif
		luaopen_posix_libgen(L);
		lua_setglobal(L, "libgen");
		luaopen_posix_unistd(L);
		lua_setglobal(L, "unistd");
		luaopen_settings(L);
		lua_setglobal(L, "settings");
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
