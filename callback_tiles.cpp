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
#include "global.h"
#include "class_global.h"
#include "tilemap.h"
#include "undo.h"
void tilesnewfilppedCB(Fl_Widget*,void*){
	pushTilemapAll(false);
	pushTileappendGroupPrepare();
	uint32_t amt=currentProject->tileC->amt;
	uint32_t*hflip=(uint32_t*)malloc(amt*sizeof(uint32_t));
	uint32_t*vflip=(uint32_t*)malloc(amt*sizeof(uint32_t));
	uint32_t*hvflip=(uint32_t*)malloc(amt*sizeof(uint32_t));
	memset(hflip,0,amt*sizeof(uint32_t));
	memset(vflip,0,amt*sizeof(uint32_t));
	memset(hvflip,0,amt*sizeof(uint32_t));
	uint32_t acum=0;
	uint8_t * tileTemp=(uint8_t *)alloca(currentProject->tileC->tileSize);
	uint8_t * tcTemp=(uint8_t *)alloca(currentProject->tileC->tcSize);
	for(uint32_t y=0;y<currentProject->tileMapC->mapSizeHA;++y){
		for(uint32_t x=0;x<currentProject->tileMapC->mapSizeW;++x){
			bool hf=currentProject->tileMapC->get_hflip(x,y),vf=currentProject->tileMapC->get_vflip(x,y);
			uint32_t t=currentProject->tileMapC->get_tile(x,y);
			if(hf&&vf){
				if(!(hvflip[t])){
					currentProject->tileC->hflip_tile(t,tileTemp);
					currentProject->tileC->hflip_truecolor(t,(uint32_t*)tcTemp);
					currentProject->tileC->vflip_tile_ptr(tileTemp,tileTemp);
					currentProject->tileC->vflip_truecolor_ptr(tcTemp,tcTemp);
					hvflip[t]=amt+acum;
					addTileappendGroup(tileTemp,tcTemp);
					currentProject->tileC->appendTile();
					memcpy(currentProject->tileC->tDat.data()+((amt+acum)*currentProject->tileC->tileSize),tileTemp,currentProject->tileC->tileSize);
					memcpy(currentProject->tileC->truetDat.data()+((amt+acum)*currentProject->tileC->tcSize),tcTemp,currentProject->tileC->tcSize);
					++acum;
				}
				currentProject->tileMapC->set_tile(hvflip[t],x,y);
				currentProject->tileMapC->set_hflip(x,y,false);
				currentProject->tileMapC->set_vflip(x,y,false);
			}else if(hf){
				if(!(hflip[t])){
					currentProject->tileC->hflip_tile(t,tileTemp);
					currentProject->tileC->hflip_truecolor(t,(uint32_t*)tcTemp);
					hflip[t]=amt+acum;
					addTileappendGroup(tileTemp,tcTemp);
					currentProject->tileC->appendTile();
					memcpy(currentProject->tileC->tDat.data()+((amt+acum)*currentProject->tileC->tileSize),tileTemp,currentProject->tileC->tileSize);
					memcpy(currentProject->tileC->truetDat.data()+((amt+acum)*currentProject->tileC->tcSize),tcTemp,currentProject->tileC->tcSize);
					++acum;
				}
				currentProject->tileMapC->set_tile(hflip[t],x,y);
				currentProject->tileMapC->set_hflip(x,y,false);
			}else if(vf){
				if(!(vflip[t])){
					currentProject->tileC->vflip_tile(t,tileTemp);
					currentProject->tileC->vflip_truecolor(t,tcTemp);
					vflip[t]=amt+acum;
					addTileappendGroup(tileTemp,tcTemp);
					currentProject->tileC->appendTile();
					memcpy(currentProject->tileC->tDat.data()+((amt+acum)*currentProject->tileC->tileSize),tileTemp,currentProject->tileC->tileSize);
					memcpy(currentProject->tileC->truetDat.data()+((amt+acum)*currentProject->tileC->tcSize),tcTemp,currentProject->tileC->tcSize);
					++acum;
				}
				currentProject->tileMapC->set_tile(vflip[t],x,y);
				currentProject->tileMapC->set_vflip(x,y,false);
			}
		}
	}
	free(hflip);
	free(vflip);
	free(hvflip);
	updateTileSelectAmt();
}
void insertTileCB(Fl_Widget*,void*){
	pushTilenew(currentProject->tileC->current_tile+1);
	currentProject->tileC->insertTile(currentProject->tileC->current_tile+1);
	updateTileSelectAmt();
	window->redraw();
}
void delete_tile_at_location(Fl_Widget*, void* row){
	/* this function will delete the tile that the user has selected */
	pushTile(currentProject->tileC->current_tile,tTypeDelete);
	currentProject->tileC->remove_tile_at(currentProject->tileC->current_tile);
	window->redraw();
}
void new_tile(Fl_Widget*,void*){
	pushTileAppend();
	currentProject->tileC->appendTile();
	//set the new maximum for slider
	updateTileSelectAmt();
	//redraw so the user knows that there is another tile
	window->redraw();
}
void update_truecolor(Fl_Widget* o,void* v){
	Fl_Slider* s = (Fl_Slider*)o;
	truecolor_temp[fl_intptr_t(v)] = s->value();
	window->redraw();
}
void blank_tile(Fl_Widget*,void*){
	//this will fill the current tile with zeros
	pushTile(currentProject->tileC->current_tile,tTypeBoth);
	currentProject->tileC->blank_tile(currentProject->tileC->current_tile);
	window->damage(FL_DAMAGE_USER1);
}
void update_offset_tile_edit(Fl_Widget*,void*){
	tile_zoom_edit=window->tile_size->value();
	tile_edit_offset_x=16+(tile_zoom_edit*8);
	window->redraw();
}
void set_tile_current(Fl_Widget* o,void*){
	Fl_Slider* s = (Fl_Slider*)o;
	currentProject->tileC->current_tile=s->value();
	window->redraw();
}
void set_tile_currentTP(Fl_Widget* o,void*){
	Fl_Slider* s = (Fl_Slider*)o;
	currentProject->tileC->current_tile=s->value();
	if(tileEditModePlace_G){
		pushTilemapEdit(selTileE_G[0],selTileE_G[1]);
		currentProject->tileMapC->set_tile(currentProject->tileC->current_tile,selTileE_G[0],selTileE_G[1]);
	}
	window->redraw();
}
void update_all_tiles(Fl_Widget*,void*){
	uint8_t sel_pal;
	if (mode_editor == tile_place)
		sel_pal=tileMap_pal.theRow;
	else
		sel_pal=tileEdit_pal.theRow;
	if (currentProject->tileC->amt>=63)
		putchar('\n');

	pushTilesAll(tTypeTile);

	for (uint32_t x=0;x<currentProject->tileC->amt;++x){
		currentProject->tileC->truecolor_to_tile(sel_pal,x,mode_editor==spriteEditor);
		if ((!(x&63))&&x)
			printf("Progress: %f\r",((float)x/(float)currentProject->tileC->amt)*100.0);
	}
	window->redraw();
}
void remove_duplicate_tiles(Fl_Widget*,void*){
	currentProject->tileC->remove_duplicate_tiles(false);
}
void remove_duplicate_truecolor(Fl_Widget*,void*){
	currentProject->tileC->remove_duplicate_tiles(true);
}
void fill_tile(Fl_Widget* o, void*){
	//fills tile with currently selected color
	if (mode_editor == tile_place){
		pushTile(currentProject->tileC->current_tile,tTypeTile);
		uint8_t color;
		color=tileMap_pal.box_sel;
		uint8_t * tile_ptr_temp;
		switch (currentProject->gameSystem){
			case sega_genesis:
				tile_ptr_temp = &currentProject->tileC->tDat[currentProject->tileC->current_tile*32];
				color|=color<<4;
				memset(tile_ptr_temp,color,32);
			break;
			case NES:
				tile_ptr_temp = &currentProject->tileC->tDat[currentProject->tileC->current_tile*16];
				//for the NES it is different
				uint8_t col_1;
				uint8_t col_2;
				col_1=color&1;
				col_2=(color>>1)&1;
				uint32_t x;
				memset(tile_ptr_temp,0,16);
				for (uint8_t y=0;y<8;++y){
					for (x=0;x<8;++x){
						tile_ptr_temp[y]|=col_1<<x;
						tile_ptr_temp[y+8]|=col_2<<x;
					}
				}
			break;
		}
	}
	else if (mode_editor == tile_edit){
		pushTile(currentProject->tileC->current_tile,tTypeBoth);
		for (uint32_t x=currentProject->tileC->current_tile*currentProject->tileC->tcSize;x<(currentProject->tileC->current_tile*currentProject->tileC->tcSize)+currentProject->tileC->tcSize;x+=4){
			currentProject->tileC->truetDat[x]=truecolor_temp[0];//red
			currentProject->tileC->truetDat[x+1]=truecolor_temp[1];//green
			currentProject->tileC->truetDat[x+2]=truecolor_temp[2];//blue
			currentProject->tileC->truetDat[x+3]=truecolor_temp[3];//alpha
		}
		currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile,false);
	}else
		fl_alert("To prevent accidental modification be in the Tile editor or Tile map editor to use this");
	window->damage(FL_DAMAGE_USER1);
}
