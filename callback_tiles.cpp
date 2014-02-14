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
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#include "global.h"
#include "class_global.h"
#include "tilemap.h"
void delete_tile_at_location(Fl_Widget*, void* row){
	/* this function will delete the tile that the user has selected
	   remeber both current_tile and tiles_amount are counting from zero that means that a value of zero means one tile */
	currentProject->tileC->remove_tile_at(currentProject->tileC->current_tile);
	window->redraw();
}
void new_tile(Fl_Widget*,void*){
	currentProject->tileC->tiles_amount++;
	currentProject->tileC->tileDat=(uint8_t *)realloc(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize);
	if (currentProject->tileC->tileDat == 0){
		show_realloc_error((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize);
		exit(1);
	}
	currentProject->tileC->truetileDat=(uint8_t *)realloc(currentProject->tileC->truetileDat,(currentProject->tileC->tiles_amount+1)*256);
	if (currentProject->tileC->truetileDat == 0){
		show_realloc_error((currentProject->tileC->tiles_amount+1)*256)
		exit(1);
	}
	memset(&currentProject->tileC->tileDat[currentProject->tileC->tiles_amount*currentProject->tileC->tileSize],0,currentProject->tileC->tileSize);
	memset(&currentProject->tileC->truetileDat[currentProject->tileC->tiles_amount*256],0,256);
	//set the new maximum for slider
	window->tile_select->maximum(currentProject->tileC->tiles_amount);
	window->tile_select_2->maximum(currentProject->tileC->tiles_amount);
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
	if(tileEditModePlace_G)
		currentProject->tileMapC->set_tile(currentProject->tileC->current_tile,selTileE_G[0],selTileE_G[1]);
	window->redraw();
}
void update_all_tiles(Fl_Widget*,void*){
	uint8_t sel_pal;
	if (mode_editor == tile_place)
		sel_pal=tileMap_pal.theRow;
	else
		sel_pal=tileEdit_pal.theRow;
	if (currentProject->tileC->tiles_amount > 63)
		putchar('\n');
	for (uint32_t x=0;x<currentProject->tileC->tiles_amount+1;++x) {
		currentProject->tileC->truecolor_to_tile(sel_pal,x);
		if ((x % 64) == 0)
			printf("Progress: %f\r",((float)x/(float)currentProject->tileC->tiles_amount)*100.0);
	}
	window->redraw();
}
void remove_duplicate_tiles(Fl_Widget*,void*){
	currentProject->tileC->remove_duplicate_tiles();
}
void remove_duplicate_truecolor(Fl_Widget*,void*){
	//sub_tile_map
	uint32_t tile_remove_c=0;
	int32_t cur_tile,curT;
	puts("Pass 1");
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++){
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--){
			if (cur_tile == curT)//dont compare it's self
				continue;
			#if __LP64__
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint64_t *)&currentProject->tileC->truetileDat[curT*256]))
			#else
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint32_t *)&currentProject->tileC->truetileDat[curT*256]))
			#endif
			{
				currentProject->tileMapC->sub_tile_map(curT,cur_tile,false,false);
				currentProject->tileC->remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
		//printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	puts("Pass 2 h-flip");
	uint8_t trueColTemp[256];
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++){
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--){
			if (cur_tile == curT)//dont compare it's self
				continue;
			currentProject->tileC->hflip_truecolor(curT,(uint32_t *)trueColTemp);
			#if __LP64__
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint64_t *)trueColTemp))
			#else
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint32_t *)trueColTemp))
			#endif
			{
				currentProject->tileMapC->sub_tile_map(curT,cur_tile,true,false);
				currentProject->tileC->remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
		//printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	puts("Pass 3 v-flip");
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++){
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--){
			if (cur_tile == curT)//dont compare it's self
				continue;
			currentProject->tileC->vflip_truecolor(curT,trueColTemp);
			#if __LP64__
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint64_t *)trueColTemp))
			#else
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint32_t *)trueColTemp))
			#endif
			{
				currentProject->tileMapC->sub_tile_map(curT,cur_tile,false,true);
				currentProject->tileC->remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
		//printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	puts("Pass 4 vh-flip");
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++){
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--){
			if (cur_tile == curT)//dont compare it's self
				continue;
			currentProject->tileC->hflip_truecolor(curT,(uint32_t *)trueColTemp);
			currentProject->tileC->vflip_truecolor_ptr(trueColTemp,trueColTemp);
			#if __LP64__
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint64_t *)trueColTemp))
			#else
			if (currentProject->tileC->cmp_trueC(cur_tile,(uint32_t *)trueColTemp))
			#endif
			{
				currentProject->tileMapC->sub_tile_map(curT,cur_tile,true,true);
				currentProject->tileC->remove_tile_at(curT);
				tile_remove_c++;
			}
		}
		//if ((cur_tile % 4096) == 0)
		//printf("On tile %d Removed %d\r",cur_tile,tile_remove_c);
	}
	printf("Removed %d tiles\n",tile_remove_c);
	tile_remove_c=0;
	window->redraw();
}
void fill_tile(Fl_Widget* o, void*){
	//fills tile with currently selected color
	if (mode_editor == tile_place){
		uint8_t color;
		color=tileMap_pal.box_sel;
		uint8_t * tile_ptr_temp;
		switch (currentProject->gameSystem){
			case sega_genesis:
				tile_ptr_temp = &currentProject->tileC->tileDat[currentProject->tileC->current_tile*32];
				color+=color<<4;
				memset(tile_ptr_temp,color,32);
			break;
			case NES:
				tile_ptr_temp = &currentProject->tileC->tileDat[currentProject->tileC->current_tile*16];
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
		for (uint32_t x=currentProject->tileC->current_tile*256;x<(currentProject->tileC->current_tile*256)+256;x+=4){
			currentProject->tileC->truetileDat[x]=truecolor_temp[0];//red
			currentProject->tileC->truetileDat[x+1]=truecolor_temp[1];//green
			currentProject->tileC->truetileDat[x+2]=truecolor_temp[2];//blue
			currentProject->tileC->truetileDat[x+3]=truecolor_temp[3];//alpha
		}
		currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile);
	}else
		fl_alert("To prevent accidental modification be in the Tile editor or Tile map editor to use this");
	window->damage(FL_DAMAGE_USER1);
}
void tileDPicker(Fl_Widget*,void*){
	Fl_Window *win;
	Fl_Progress *progress;
	win = new Fl_Window(250,45,"Progress");           // access parent window
	win->begin();                                // add progress bar to it..
	progress = new Fl_Progress(25,7,200,30);
	progress->minimum(0.0);                      // set progress range to be 0.0 ~ 1.0
	progress->maximum(1.0);
	progress->color(0x88888800);               // background color
	progress->selection_color(0x4444ff00);     // progress bar color
	progress->labelcolor(FL_WHITE);            // percent text color
	win->end();                                  // end adding to window
	win->show();
	currentProject->tileMapC->pickRowDelta(true,progress);
	win->remove(progress);// remove progress bar from window
	delete(progress);// deallocate it
	//w->draw();
	delete win;
	window->damage(FL_DAMAGE_USER1);
}
