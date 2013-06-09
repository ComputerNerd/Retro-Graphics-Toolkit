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
*/
#include "global.h"
#include "callbacks_palette.h"
#include "callback_tiles.h"
#include "tilemap.h"
#include "color_convert.h"
#include "errorMsg.h"
#include "dither.h"
#include <zlib.h>
#include <png.h>
void fill_tile(Fl_Widget* o, void*)
{
	//fills tile with currently selected color
	if (mode_editor == tile_place)
	{
		uint8_t color;
		color=tileMap_pal.box_sel;
		uint8_t * tile_ptr_temp;
		switch (game_system)
		{
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
				for (uint8_t y=0;y<8;y++)
				{
					for (x=0;x<8;x++)
					{
						tile_ptr_temp[y]|=col_1<<x;
						tile_ptr_temp[y+8]|=col_2<<x;
					}
				}
			break;
		}
	}
	else if (mode_editor == tile_edit)
	{
		for (uint32_t x=currentProject->tileC->current_tile*256;x<(currentProject->tileC->current_tile*256)+256;x+=4)
		{
			currentProject->tileC->truetileDat[x]=truecolor_temp[0];//red
			currentProject->tileC->truetileDat[x+1]=truecolor_temp[1];//green
			currentProject->tileC->truetileDat[x+2]=truecolor_temp[2];//blue
			currentProject->tileC->truetileDat[x+3]=truecolor_temp[3];//alpha
		}
		currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile);
	}
	else
		fl_alert("To prevent accidental modification be in the Tile editor or Tile map editor to use this");
	window->damage(FL_DAMAGE_USER1);
}
void update_truecolor(Fl_Widget* o, void* v)
{
	Fl_Slider* s = (Fl_Slider*)o;
	truecolor_temp[fl_intptr_t(v)] = s->value();
	window->redraw();
}
void blank_tile(Fl_Widget* o, void*)
{
	//this will fill the current tile with zeros
	currentProject->tileC->blank_tile(currentProject->tileC->current_tile);
	window->damage(FL_DAMAGE_USER1);
}
void callback_resize_map(Fl_Widget* o, void*)
{
	uint8_t w,h;
	w=window->map_w->value();
	h=window->map_h->value();
	resize_tile_map(w,h);
	window->redraw();
}
void update_offset_tile_edit(Fl_Widget* o, void*)
{
	tile_zoom_edit=window->tile_size->value();
	tile_edit_offset_x=16+(tile_zoom_edit*8);
	window->redraw();
}
void set_mode_tabs(Fl_Widget* o, void*)
{
	intptr_t val=(intptr_t)(Fl_Tabs*)window->the_tabs->value();
	if (val==pal_id)
	{
		mode_editor=pal_edit;
		palEdit.updateSlider();
	}
	else if (val==tile_edit_id)
	{
		mode_editor=tile_edit;
		tileEdit_pal.updateSlider();
	}
	else if (val==tile_place_id)
	{
		mode_editor=tile_place;
		tileMap_pal.updateSlider();
	}
}
void set_ditherAlg(Fl_Widget*, void* typeset)
{
	if ((uintptr_t)typeset == 0)
		window->ditherPower->show();
	else
		window->ditherPower->hide();//imagine the user trying to change the power and nothing happening not fun at all
	ditherAlg=(uintptr_t)typeset;
}
void set_tile_row(Fl_Widget*, void* row)
{
	uint8_t selrow=(uintptr_t)row;
	switch (mode_editor) {
		case tile_edit:
			tileEdit_pal.changeRow(selrow);
			currentProject->tileC->truecolor_to_tile(selrow,currentProject->tileC->current_tile);
		break;
		case tile_place:
			tileMap_pal.changeRow(selrow);
		break;
	}
	window->redraw();//trigger a redraw so that the new row is displayed
}
void update_box_size(Fl_Widget*, void* )
{
	window->redraw();
}
void set_tile_current(Fl_Widget* o, void* )
{
	Fl_Slider* s = (Fl_Slider*)o;
	currentProject->tileC->current_tile=s->value();
	window->redraw();
}
void set_grid(Fl_Widget*,void*)
{
	//this function will only be trigger when the check button is pressed
	//so we just need to invert the bool using xor to avoid if statments
	show_grid=show_grid^true;
	window->redraw();//redraw to reflect the updated statues of the grid
}
void set_grid_placer(Fl_Widget*,void*)
{
	show_grid_placer=show_grid_placer^true;
	window->redraw();//redraw to reflect the updated statues of the grid
}
void set_prio_callback(Fl_Widget*,void*)
{
	G_highlow_p=G_highlow_p^true;
	//window->redraw();
}
void set_hflip(Fl_Widget*,void*)
{
	G_hflip=G_hflip^true;
	window->redraw();
}
void set_vflip(Fl_Widget*,void*)
{
	G_vflip=G_vflip^true;
	window->redraw();
}
void update_map_scroll_x(Fl_Widget*,void*)
{
	map_scroll_pos_x=window->map_x_scroll->value();
	//cout << "map scroll pos x = " << (short)map_scroll_pos_x << endl;//chars needed to casted to something else
	window->redraw();
}
void update_map_scroll_y(Fl_Widget*,void*)
{
	map_scroll_pos_y=window->map_y_scroll->value();
	//cout << "map scroll pos x = " << (short)map_scroll_pos_x << endl;//chars needed to casted to something else
	window->redraw();
}
void update_map_size(Fl_Widget*,void*)
{
	uint16_t old_scroll=window->map_x_scroll->value();
	uint8_t tile_size_placer=window->place_tile_size->value();
	int32_t map_scroll=((tile_size_placer*8)*currentProject->tileMapC->mapSizeW)-map_off_x;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
		map_scroll=0;
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll){
		old_scroll=map_scroll;
		map_scroll_pos_x=map_scroll;
	}
	window->map_x_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));//the reason for adding map_scroll/2 to map_scroll is because without it the user will not be able to scroll the tilemap all the way
	old_scroll=window->map_y_scroll->value();
	tile_size_placer=window->place_tile_size->value();
	map_scroll=((tile_size_placer*8)*currentProject->tileMapC->mapSizeH)-map_off_y;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
		map_scroll=0;
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll){
		old_scroll=map_scroll;
		map_scroll_pos_y=map_scroll;
	}
	window->map_y_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));
	window->redraw();
}
void save_tiles_truecolor(Fl_Widget*,void*)
{
	if (load_file_generic("Save truecolor tiles",true) == true){
		FILE * myfile;
		myfile = fopen(the_file.c_str(),"wb");
		if (myfile!=0){
			fwrite(currentProject->tileC->truetileDat,1,(currentProject->tileC->tiles_amount+1)*256,myfile);
			puts("Great Sucess File Saved!");
			fclose(myfile);
		}
		else
			fl_alert("Error: can not save file %s",the_file.c_str());
	}
}
void save_tiles(Fl_Widget*,void*)
{
	if (load_file_generic("Pick a location to save tiles",true) == true){
		uint8_t type=fl_choice("How would like this file saved?","Binary","C header",0);
		uint8_t compression=fl_choice("What kind of compression do you want used?","Uncompressed","Nemesis","Kosinski");
		FILE* myfile;
		uint8_t* compdat;
		uint32_t compsize;
		if (type==1)
			myfile = fopen(the_file.c_str(),"w");
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (likely(myfile!=0)){
			if (type == 1){
				char temp[2048];
				sprintf(temp,"//%d tiles",currentProject->tileC->tiles_amount+1);
				fputs((const char *)temp,myfile);
				if (compression==1)
					fputs(" nemesis compressed",myfile);
				else
					fputc('\n',myfile);
				fputs("const uint8_t tileDat[]={",myfile);
			}
			if(compression){
				string input;
				input.assign((const char *)currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize);
				std::istringstream iss(input);
				std::ostringstream outfun;
				if(compression==2){
						kosinski comp;
						comp.encode(iss,outfun);
				}else{
					nemesis comp;
					comp.encode(iss,outfun);
				}
				compsize=outfun.str().length();
				compdat=(uint8_t*)malloc(compsize);
				if (compdat==0)
					show_malloc_error(compsize)
				string output=outfun.str();
				output.copy((char *)compdat,compsize);
				printf("Compressed to %d uncompressed would be %d so therefore the file the ratio is %f\n",compsize,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize,(double)compsize/(double)((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize)*100.0);
			}
			if (type == 1){
				if (compression){
					if(saveBinAsText(compdat,compsize,myfile)==false){
						free(compdat);
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}else{
					if (saveBinAsText(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize,myfile)==false) {
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}
				fputs("};",myfile);
			}else{
				if(compression)
					fwrite(compdat,1,compsize,myfile);
				else
					fwrite(currentProject->tileC->tileDat,currentProject->tileC->tileSize,(currentProject->tileC->tiles_amount+1),myfile);
			}
			if (compression==1)
				free(compdat);
		}else
			fl_alert("Error: can not save file %s",the_file.c_str());
		fclose(myfile);
	}
}
void save_map(Fl_Widget*,void*)
{
	if (currentProject->tileMapC->saveToFile() == false)
		fl_alert("Error: can not save file %s\nTry making sure that you have permission to save the file here",the_file.c_str());
}
void load_tiles(Fl_Widget*,void* split)
{
	//if append==1 then we will append data but if not it will erase over current tiles
	//format row,append
	uint32_t file_size;
	uint8_t append=(uintptr_t)split&0xFF;
		char * returned=(char *)fl_input("What row should these tiles use?\nEnter 0 to 3 to selected a row or -1 to -4 to auto determin based on tilemap\n The number after the negative symbol is the default row +1 if not tile is found","-1");
	if (returned==0)
		return;
	if (verify_str_number_only(returned) == false)
			return;
	int8_t row=atoi(returned);
	if (unlikely((row > 3) || (row < -4))) {
		fl_alert("You entered %d which is out of range it must be in range of -4 to 3",row);
		return;
	}
	uint8_t defaultRow=row >= 0 ? row:abs(row)-1;
	uint8_t compression = fl_choice("What format is the tile?","Uncompressed","Nemesis Compressed","Kosinski");
	if (load_file_generic() == true) {
		FILE * myfile;
		std::stringstream outDecomp;
		myfile = fopen(the_file.c_str(),"rb");
		if (likely(myfile!=0)) {
			fseek(myfile, 0L, SEEK_END);
			file_size = ftell(myfile);//file.tellg();
			rewind(myfile);
			uint8_t truecolor_multiplier;
			truecolor_multiplier=256/currentProject->tileC->tileSize;
			if(compression){
				uint8_t * datcmp=(uint8_t *)malloc(file_size);
				if (unlikely(datcmp==0))
					show_malloc_error(file_size)
				fread(datcmp,1,file_size,myfile);
				fclose(myfile);
				std::string input;
				input.assign((const char *)datcmp,file_size);
				free(datcmp);
				std::istringstream iss(input);
				if (compression==2){
					kosinski decomp;
					decomp.decode(iss,outDecomp);
				}else{
					nemesis decomp;
					decomp.decode(iss,outDecomp);
				}
				file_size=outDecomp.str().length();
				printf("Decompressed to %d bytes\n",file_size);
			}else{
				if ((file_size/currentProject->tileC->tileSize)*currentProject->tileC->tileSize != file_size) {
					fl_alert("Error: This is not a valid tile file each tile is %d bytes and this file is not a multiple of %d so it is not a valid tile file",currentProject->tileC->tileSize,currentProject->tileC->tileSize);
					fclose(myfile);
					return;//return so that the file does not get loaded
				}
			}
			uint32_t offset_tiles;
			uint32_t offset_tiles_bytes;
			if (append == 1) {
				offset_tiles=currentProject->tileC->tiles_amount+1;
				offset_tiles_bytes=offset_tiles*currentProject->tileC->tileSize;
				currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,file_size+((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize));
				if (currentProject->tileC->tileDat == 0) {
					if (compression==0)
						fclose(myfile);
					show_realloc_error(file_size)
				}
			}else{
				free(currentProject->tileC->tileDat);
				currentProject->tileC->tileDat = (uint8_t *)malloc(file_size);
				if (currentProject->tileC->tileDat == 0) {
					if (compression==0)
						fclose(myfile);
					show_malloc_error(file_size)
				}
				offset_tiles=0;
				offset_tiles_bytes=0;
			}
			if (compression) {
				string output=outDecomp.str();
				output.copy((char *)currentProject->tileC->tileDat+offset_tiles_bytes,file_size);
			}else{
				fread(currentProject->tileC->tileDat+offset_tiles_bytes,1,file_size,myfile);
				fclose(myfile);
			}
			currentProject->tileC->truetileDat = (uint8_t *)realloc(currentProject->tileC->truetileDat,(file_size*truecolor_multiplier)+(offset_tiles_bytes*truecolor_multiplier));
			if (currentProject->tileC->truetileDat == 0)
				show_malloc_error(file_size*truecolor_multiplier)
			for (uint32_t c=offset_tiles;c<(file_size/currentProject->tileC->tileSize)+offset_tiles;c++) {
				if (row < 0) {
					uint32_t x,y;
					uint8_t foundRow=defaultRow;
					for (y=0;y<currentProject->tileMapC->mapSizeH;y++) {
						for (x=0;x<currentProject->tileMapC->mapSizeW;x++) {
							if (currentProject->tileMapC->get_tile(x,y) == c) {
								foundRow=currentProject->tileMapC->get_palette_map(x,y);
								goto doTile;
							}
						}
					}
doTile:
					tileToTrueCol(&currentProject->tileC->tileDat[(c*currentProject->tileC->tileSize)],&currentProject->tileC->truetileDat[(c*256)],foundRow);
				}
				else
					tileToTrueCol(&currentProject->tileC->tileDat[(c*currentProject->tileC->tileSize)],&currentProject->tileC->truetileDat[(c*256)],defaultRow);
			}
			currentProject->tileC->tiles_amount=(file_size/currentProject->tileC->tileSize)-1;
			currentProject->tileC->tiles_amount+=offset_tiles > 1 ? offset_tiles-1:0;
			window->tile_select->maximum(currentProject->tileC->tiles_amount);
			window->tile_select->value(0);
			window->tile_select_2->maximum(currentProject->tileC->tiles_amount);
			window->tile_select_2->value(0);
			window->redraw();
		}
		else
			fl_alert("The file %s Cannot be loaded",the_file.c_str());
	}
}
void update_all_tiles(Fl_Widget*,void*)
{
	uint8_t sel_pal;
	if (mode_editor == tile_place)
		sel_pal=tileMap_pal.theRow;
	else
		sel_pal=tileEdit_pal.theRow;
	if (currentProject->tileC->tiles_amount > 63)
		putchar('\n');
	for (uint32_t x=0;x<currentProject->tileC->tiles_amount+1;x++) {
		currentProject->tileC->truecolor_to_tile(sel_pal,x);
		if ((x % 64) == 0)
			printf("Progress: %f\r",((float)x/(float)currentProject->tileC->tiles_amount)*100.0);
	}
	window->redraw();
}
void load_truecolor_tiles(Fl_Widget*,void*)
{
	//start by loading the file
	uint32_t file_size;
	if (load_file_generic() == true)
	{
		//ios::ifstream file (the_file.c_str(), ios::in|ios::binary|ios::ate);
		FILE * myfile;
		myfile = fopen(the_file.c_str(),"rb");
		fseek(myfile, 0L, SEEK_END);
		file_size = ftell(myfile);
		if ((file_size/256)*256 != file_size) {
			fl_alert("Error: this file is not a multiple of 256 it is not a valid truecolor tiles. The file size is: %d",file_size);
			fclose(myfile);
			return;
		}
		free(currentProject->tileC->truetileDat);
		free(currentProject->tileC->tileDat);
		currentProject->tileC->truetileDat = (uint8_t *)malloc(file_size);
		if (currentProject->tileC->truetileDat == 0)
			show_malloc_error(file_size)
		switch (game_system) {
			case sega_genesis:
				currentProject->tileC->tileDat = (uint8_t *)malloc(file_size/6);
			break;
			case NES:
				currentProject->tileC->tileDat = (uint8_t *)malloc(file_size/12);
			break;
		}
		if (currentProject->tileC->tileDat == 0)
			show_malloc_error(file_size/6)
		//file.seekg (0, ios::beg);//return to the beginning of the file
		rewind(myfile);
		//file.read ((char *)currentProject->tileC->truetileDat, file_size);
		fread(currentProject->tileC->truetileDat,file_size,1,myfile);
		//file.close();
		fclose(myfile);
		currentProject->tileC->tiles_amount=file_size/256;
		currentProject->tileC->tiles_amount--;
		window->tile_select->maximum(currentProject->tileC->tiles_amount);
		window->tile_select_2->maximum(currentProject->tileC->tiles_amount);
		window->redraw();
	}
}
void fill_tile_map_with_tile(Fl_Widget*,void*)
{
	if (mode_editor != tile_place) {
		fl_alert("To prevent aciddental modifaction to the tile map be in plane editing mode");
		return;
	}
	for (uint16_t y=0;y<currentProject->tileMapC->mapSizeH;y++) {
		for (uint16_t x=0;x<currentProject->tileMapC->mapSizeW;x++)
			set_tile_full(currentProject->tileC->current_tile,x,y,tileMap_pal.theRow,G_hflip,G_vflip,G_highlow_p);
	}
	window->damage(FL_DAMAGE_USER1);
}
void load_tile_map(Fl_Widget*,void*)
{
	if (unlikely(currentProject->tileMapC->loadFromFile() == false))
		fl_alert("Error: Cannot load file %s",the_file.c_str());
}
void shadow_highligh_findout(Fl_Widget*,void*)
{
	if (unlikely(game_system != sega_genesis)) {
		fl_alert("Only the Sega Genesis/Mega Drive supports shadow highligh mode\n");
		return;
	}
	uint8_t type=fl_choice("How will it be determined if the tile is shadowed or not?","Tile brightness","Delta",0);
	//this function will see if 3 or less pixels are above 125 and if so set prioity to low or set priority to high if bright tile
	uint16_t x,y;
	uint32_t xx;
	if (type==0) {
		for (y=0;y<currentProject->tileMapC->mapSizeH;y++) {
			for (x=0;x<currentProject->tileMapC->mapSizeW;x++) {
				uint32_t cur_tile=currentProject->tileMapC->get_tile(x,y);
				uint8_t over=0;
				for (xx=cur_tile*256;xx<cur_tile*256+256;xx+=4) {
					if ((currentProject->tileC->truetileDat[xx] > 130) || (currentProject->tileC->truetileDat[xx+1] > 130) || (currentProject->tileC->truetileDat[xx+2] > 130))
						over++;
				}
				if (over > 4)
					set_prio(x,y,true);//normal
				else
					set_prio(x,y,false);//shadowed
			}
		}
	}
	else {
		uint8_t temp[256];
		//uint8_t useHiL=palette_muliplier;
		uint8_t type_temp=palTypeGen;
		for (y=0;y<currentProject->tileMapC->mapSizeH;y++) {
			for (x=0;x<currentProject->tileMapC->mapSizeW;x++) {
				uint32_t cur_tile=currentProject->tileMapC->get_tile(x,y);
				uint32_t errorSh=0,errorNorm=0;
				uint8_t * ptrorgin=&currentProject->tileC->truetileDat[(cur_tile*256)];
				set_palette_type(0);//normal
				currentProject->tileC->truecolor_to_tile(currentProject->tileMapC->get_palette_map(x,y),cur_tile);
				tileToTrueCol(&currentProject->tileC->tileDat[(cur_tile*currentProject->tileC->tileSize)],temp,currentProject->tileMapC->get_palette_map(x,y));
				for (xx=0;xx<256;xx+=4) {
					errorNorm+=abs(temp[xx]-ptrorgin[xx]);
					errorNorm+=abs(temp[xx+1]-ptrorgin[xx+1]);
					errorNorm+=abs(temp[xx+2]-ptrorgin[xx+2]);
				}
				set_palette_type(8);//shadow
				currentProject->tileC->truecolor_to_tile(currentProject->tileMapC->get_palette_map(x,y),cur_tile);
				tileToTrueCol(&currentProject->tileC->tileDat[(cur_tile*currentProject->tileC->tileSize)],temp,currentProject->tileMapC->get_palette_map(x,y));
				for (xx=0;xx<256;xx+=4) {
					errorSh+=abs(temp[xx]-ptrorgin[xx]);
					errorSh+=abs(temp[xx+1]-ptrorgin[xx+1]);
					errorSh+=abs(temp[xx+2]-ptrorgin[xx+2]);
				}
				if (errorSh < errorNorm)
					set_prio(x,y,false);//shadowed
				else
					set_prio(x,y,true);//normal
			}
		}
		set_palette_type(type_temp);//0 normal 8 shadow 16 highlight		
	}
	window->redraw();
}
void dither_tilemap_as_image(Fl_Widget*,void*)
{
	//normally this program dithers all tiles individully this is not always desirable
	//to fix this I created this function It convertes the tilemap to image and dithers all tiles
	//so first create ram for image
	uint8_t * image;
	uint32_t w,h;
	uint8_t type_temp=palTypeGen;
	uint8_t tempSet=0;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeH*8;
	image = (uint8_t *)malloc(w*h*4);
	if (image==0)
		show_malloc_error(w*h*4)
	uint32_t truecolor_tile_ptr=0;
	uint32_t x_tile=0,y_tile=0;
	uint8_t truecolor_tile[256];
	for (uint8_t rowz=0;rowz<4;rowz++){
		printf("Row %d\n",rowz);
		puts("Starting");
		printf("Stage 1 %%: 0\n");
		truecolor_to_image(image,rowz);
		printf("Stage 1 %%: %f\n",(1.0f/3.0f)*100.0f);
		ditherImage(image,w,h,true,true);
		ditherImage(image,w,h,true,false);
		printf("Stage 2 %%: %f\n",(2.0f/3.0f)*100.0f);
		//convert back to tiles
		x_tile=0;
		y_tile=0;
		puts("Stage 3 starting");
		for (uint64_t a=0;a<(h*w*4)-w*4;a+=w*4*8)//a tiles y
		{
			for (uint32_t b=0;b<w*4;b+=32)//b tiles x
			{	
				uint8_t temp;
				int32_t current_tile=currentProject->tileMapC->get_tileRow(x_tile,y_tile,rowz);
				if (current_tile == -1)
					goto dont_convert_tile;
				truecolor_tile_ptr=0;
				for (uint32_t y=0;y<w*4*8;y+=w*4)//pixels y
				{
					memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y],32);
					truecolor_tile_ptr+=32;
				}
				//convert back to tile
				uint8_t * TileTempPtr;
				switch (game_system){
					case sega_genesis:
						current_tile*=32;
						if (type_temp != 0){
							tempSet=(currentProject->tileMapC->get_prio(x_tile,y_tile)^1)*8;
							set_palette_type(tempSet);
						}
						TileTempPtr=&currentProject->tileC->tileDat[current_tile];
						for (uint16_t y=0;y<256;y+=32) {
							for (uint8_t x=0;x<32;x+=4) {
								//even,odd
								if (x & 4) {
									//odd
									if (truecolor_tile[y+x+3] != 0) {
										temp=find_near_color_from_row(currentProject->tileMapC->get_palette_map(x_tile,y_tile),truecolor_tile[y+x],truecolor_tile[y+x+1],truecolor_tile[y+x+2]);
										*TileTempPtr++|=temp;
									}
									else
										TileTempPtr++;
								}
								else {
									//even
									if (likely(truecolor_tile[y+x+3] != 0)) {
										temp=find_near_color_from_row(currentProject->tileMapC->get_palette_map(x_tile,y_tile),truecolor_tile[y+x],truecolor_tile[y+x+1],truecolor_tile[y+x+2]);
										*TileTempPtr=temp<<4;
									}
									else
										*TileTempPtr=0;
								}
							}
						}
					break;
					case NES:
						current_tile*=16;
						TileTempPtr=&currentProject->tileC->tileDat[current_tile];
						for (uint8_t x=0;x<16;x++)
						{
							currentProject->tileC->tileDat[current_tile+x]=0;
						}
						for (uint8_t y=0;y<8;y++)
						{
							for (uint8_t x=0;x<8;x++)
							{
								if (truecolor_tile[(y*32)+(x*4)+3] != 0)
								{
									temp=find_near_color_from_row(currentProject->tileMapC->get_palette_map(x_tile,y_tile),truecolor_tile[(y*32)+(x*4)],truecolor_tile[(y*32)+(x*4)+1],truecolor_tile[(y*32)+(x*4)+2]);
									TileTempPtr[y]|=(temp&1)<<(7-x);
									TileTempPtr[y+8]|=((temp>>1)&1)<<(7-x);
								}
							}
						}
					break;
				}
	dont_convert_tile:
			x_tile++;	
			}
		x_tile=0;
		y_tile++;
		//update progress currentProject->tileMapC->mapSizeH
		printf("Stage 3 %%: %f\r",(((float)y_tile/(float)currentProject->tileMapC->mapSizeH/3.0f)+(2.0f/3.0f))*100.0f);
		}
		putchar('\n');
		puts("Done with image");
		window->damage(FL_DAMAGE_USER1);
		Fl::check();
	}
	free(image);
	if (game_system == sega_genesis)
		set_palette_type(type_temp);
	window->redraw();
}
void load_image_to_tilemap(Fl_Widget*,void*)
{
	Fl_Shared_Image * loaded_image;
	if (load_file_generic("Load image") == true){
		loaded_image=Fl_Shared_Image::get(the_file.c_str());
		if (loaded_image == 0){
			fl_alert("Error loading image");
			return;
		}
		uint32_t w,h;
		w=loaded_image->w();
		h=loaded_image->h();
		printf("image width: %d image height: %d\n",w,h);
		uint16_t w8,h8;
		uint32_t wt,ht;
		uint8_t wr,hr;
		wt=w&(~7U);
		ht=h&(~7U);
		wr=w&7;
		hr=h&7;
		w8=w/8;
		h8=h/8;
		if (wr!=0)
			w8++;
		if (hr!=0)
			h8++;
		if ((wr != 0) && (hr != 0))
			fl_alert("Warning both width and height are not a multiple of 8");
		else if (wr != 0)
			fl_alert("Warning width is not a multiple of 8");
		else if (hr != 0)
			fl_alert("Warning height is not a multiple of 8");
		printf("w %d h %d wt %d ht %d wr %d hr %d w8 %d h8 %d\n",w,h,wt,ht,wr,hr,w8,h8);
		//start by copying the data
		uint8_t * img_ptr=(uint8_t *)loaded_image->data()[0];
		//printf("First Pixel Red: %d Green: %d Blue: %d\n",img_ptr[0],img_ptr[1],img_ptr[2]);
		//now we can convert to tiles
		if (unlikely(loaded_image->d() != 3 && loaded_image->d() != 4)){
			fl_alert("Please use color depth of 3 or 4\nYou Used %d",loaded_image->d());
			loaded_image->release();
			return;
		}else
			printf("Image depth %d\n",loaded_image->d());
		uint64_t truecolor_tile_ptr=0;
		currentProject->tileC->truetileDat = (uint8_t *)realloc(currentProject->tileC->truetileDat,w8*h8*256);
		currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,w8*h8*currentProject->tileC->tileSize);
		currentProject->tileC->tiles_amount=(w8*h8)-1;
		window->tile_select->maximum(currentProject->tileC->tiles_amount);
		window->tile_select_2->maximum(currentProject->tileC->tiles_amount);
		//uint8_t sizeTemp,sizeTemp2;
		uint64_t a;
		uint32_t b,y,x;
		uint8_t xx;
		switch (loaded_image->d()){
			case 3:
				for (a=0;a<(ht*wt*3)-wt*3;a+=w*3*8)//a tiles y
				{
					for (b=0;b<wt*3;b+=24)//b tiles x
					{
						for (y=0;y<wt*3*8;y+=w*3)//pixels y
						{
							xx=0;
							for (x=0;x<32;x+=4)//pixels x
							{
								memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x],&img_ptr[a+b+y+xx],3);
								currentProject->tileC->truetileDat[truecolor_tile_ptr+x+3]=255;//solid
								xx+=3;
							}
							truecolor_tile_ptr+=32;
						}
					}
					if (wr!=0)
					{//handle borders
						b+=24;
						uint32_t yy=wt*3*8;
						for (y=0;y<8;y++){
							xx=0;
							for (x=0;x<wr*4;x+=4){
								memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x],&img_ptr[a+b+yy+xx],3);
								currentProject->tileC->truetileDat[truecolor_tile_ptr+x+3]=255;//solid
								xx+=3;
							}
							memset(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x+4],0,32-xx-4);
							truecolor_tile_ptr+=32;
							yy+=w*3;
						}
					}
				}
			break;
			case 4:
				for (a=0;a<(ht*wt*4)-wt*4;a+=w*4*8)//a tiles y
				{
					for (b=0;b<wt*4;b+=32)//b tiles x
					{
						for (y=0;y<wt*4*8;y+=w*4)//pixels y
						{
							memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr],&img_ptr[a+b+y],32);
							truecolor_tile_ptr+=32;
						}
					}
					if (wr!=0)
					{//handle borders
						b+=24;
						uint32_t yy=wt*4*8;
						for (y=0;y<8;y++)
						{
							memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr],&img_ptr[a+b+y],wr*4);
							memset(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x+4],0,32-(wr*4)-4);
							truecolor_tile_ptr+=32;
							yy+=w*3;
						}
					}
				}
			break;
			default:
				fl_alert("HUGE ERROR ENDING FUNCTION\n");
				loaded_image->release();
				return;
			break;
		}
		loaded_image->release();
		resize_tile_map(w8,h8);
		window->map_w->value(w8);
		window->map_h->value(h8);
		uint32_t tilecounter=0;
		for (y=0;y<h8;y++){
			for (x=0;x<w8;x++){
				set_tile_full(tilecounter,x,y,0,false,false,false);
				tilecounter++;
			}
		}
		window->redraw();
	}
}
void set_palette_type_callback(Fl_Widget*,void* type)
{
	set_palette_type((uintptr_t)type);
	window->redraw();
}
void remove_duplicate_tiles(Fl_Widget*,void*)
{
	currentProject->tileC->remove_duplicate_tiles();
}
void remove_duplicate_truecolor(Fl_Widget*,void*)
{
	//sub_tile_map
	uint32_t tile_remove_c=0;
	int32_t cur_tile,curT;
	puts("Pass 1");
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++)
	{
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--)
		{
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
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++)
	{
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--)
		{
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
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++)
	{
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--)
		{
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
	for (cur_tile=0;cur_tile<=currentProject->tileC->tiles_amount;cur_tile++)
	{
		for (curT=currentProject->tileC->tiles_amount;curT>=0;curT--)
		{
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
void rgb_pal_to_entry(Fl_Widget*,void*)
{
	//this function will convert a rgb value to the nearst palette entry
	if (mode_editor != tile_edit) {
		fl_alert("Be in Tile editor to use this");
		return;
	}
}
void set_game_system(Fl_Widget*,void* selection)
{
	uint8_t sel8=(uintptr_t)selection;
	if (unlikely(sel8 == game_system)){
		fl_alert("You are already in that mode");
		return;
	}
	switch(sel8){
		case sega_genesis:
			//fl_alert("Sega genesis Mode");
			game_system=sega_genesis;
			currentProject->tileC->tileSize=32;
			//create_shadow_highlight_buttons();
			shadow_highlight_switch->show();
			{//for varibles to be declared inside of switch statment I must put brackes around so the compiler knows when to free them
				uint8_t pal_temp[128];
				uint8_t c;
				for (c=0;c<128;c+=2){
					uint16_t temp=to_sega_genesis_color(c/2);
					pal_temp[c]=temp>>8;
					pal_temp[c+1]=temp&255;
				}
				memcpy(currentProject->palDat,pal_temp,128);
			}
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();
			currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*32);
		break;
		case NES:
			game_system=NES;
			currentProject->tileC->tileSize=16;
			shadow_highlight_switch->hide();
			for (uint8_t c=0;c<16;c++)
				currentProject->palDat[c]=to_nes_color(c);
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();
			currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*16);
		break;
		default:
			show_default_error
			return;
		break;
	}
	window->redraw();
}
void tilemap_remove_callback(Fl_Widget*,void*)
{
		char str[16];
		char * str_ptr;
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Tile");
		if (str_ptr == 0)
			return;
		if (verify_str_number_only(str_ptr) == false)
			return;
		int32_t tile=atoi(str_ptr);
		if (tile < 0) {
			fl_alert("You must enter a value equal to or about 0 but you entered %d\n",tile);
			return;
		}
		currentProject->tileMapC->sub_tile_map(tile,tile-1,false,false);
		window->damage(FL_DAMAGE_USER1);
}
void trueColTileToggle(Fl_Widget*,void*)
{
	showTrueColor^=1;
	window->damage(FL_DAMAGE_USER1);
}
void tileDPicker(Fl_Widget*,void*)
{
	currentProject->tileMapC->pickRowDelta();
	window->damage(FL_DAMAGE_USER1);
}
void showAbout(Fl_Widget*,void*)
{
	fl_alert("Retro Graphics Toolkit is written by sega16/nintendo8/sonic master or whatever you want to call me\nThis program was build on %s %s\nTechiclly speaking this date was the last time that main.cpp was updated.",__DATE__,__TIME__);
}
void toggleRowSolo(Fl_Widget*,void*)
{
	rowSolo^=true;
	window->redraw();
}
void clearPalette(Fl_Widget*,void*)
{
	if (fl_ask("This will set all colors to 0 are you sure you want to do this?")){
		memset(currentProject->palDat,0,128);
		memset(currentProject->rgbPal,0,192);
		window->damage(FL_DAMAGE_USER1);
		palEdit.updateSlider();
		tileEdit_pal.updateSlider();
		tileMap_pal.updateSlider();
	}
}
const char * freeDes="This sets the currently selected palette entry to free meaning that this color can be changed";
const char * lockedDes="This sets the currently selected palette entry to locked meaning that this color cannot be changed but tiles can still use it";
const char * reservedDes="This sets the currently selected palette entry to reserved meaning that this color cannot be changed or used in tiles note that you may need make sure all tiles get re-dithered to ensure that this rule is enforced";
void setPalType(Fl_Widget*,void* type)
{
	switch (mode_editor)
	{
		case pal_edit:
			currentProject->palType[palEdit.getEntry()]=(uintptr_t)type;
			palEdit.updateSlider();
		break;
		case tile_edit:
			currentProject->palType[tileEdit_pal.getEntry()]=(uintptr_t)type;
			tileEdit_pal.updateSlider();
		break;
		case tile_place:
			currentProject->palType[tileMap_pal.getEntry()]=(uintptr_t)type;
			tileMap_pal.updateSlider();
		break;
		default:
			show_default_error
		break;
	}
	window->redraw();
}
int savePNG(const char * fileName,uint32_t width,uint32_t height,void * ptr)
{
	//saves a 24bit png with rgb byte order
	png_byte * dat=(png_byte*)ptr;//convert to uint8_t
	FILE * fp=fopen(fileName,"wb");
	if (fp==0)
		return 1;
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)0,0,0);
	if (!png_ptr)
		return 1;
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		return 1;
	}
	if (setjmp(png_jmpbuf(png_ptr))){
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return 1;
	}
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height,8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);//must be called before other png_set_*() functions
	png_set_compression_level(png_ptr,Z_BEST_COMPRESSION);
	uint32_t y;
	png_set_user_limits(png_ptr, width, height);
	png_write_info(png_ptr, info_ptr);
	for (y=0;y<height;y++)
		png_write_row(png_ptr, &dat[(y*width*3)]);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);//done with file
	return 0;//will return 0 on success non-zero in error
}
void save_tilemap_as_image(Fl_Widget*,void*)
{
	if(load_file_generic("Save png",true)==true){
		uint32_t w=currentProject->tileMapC->mapSizeW*8;
		uint32_t h=currentProject->tileMapC->mapSizeH*8;
		uint8_t * image=(uint8_t*)malloc(w*h*3);
		uint8_t * imageold=image;
		if(image==0)
			show_malloc_error(w*h*3)
		uint8_t temptile[192];
		uint32_t x,y;
		uint32_t w3=w*3;//do this once instead of thousands of times in the loop
		uint32_t h3=h*3;
		uint32_t w21=w*21;
		uint32_t w24_24=(w*24)-24;
		uint8_t * tempptr,yy;
		for(y=0;y<h;y+=8){
			for(x=0;x<w;x+=8){
				tileToTrueCol(currentProject->tileC->tileDat+(currentProject->tileMapC->get_tile(x/8,y/8)*currentProject->tileC->tileSize),temptile,currentProject->tileMapC->get_palette_map(x/8,y/8),false);
				tempptr=temptile;
				for(yy=0;yy<8;++yy){
					memcpy(image,tempptr,24);
					image+=w3;
					tempptr+=24;
				}
				image-=w24_24;
			}
			image+=w21;
		}
		savePNG(the_file.c_str(),w,h,(void*)imageold);
		free(imageold);
	}
}
void editor::_editor()
{
	//create the window
	menu = new Fl_Menu_Bar(0,0,800,24);		// Create menubar, items..
	menu->add("&File/&Open palette",(int)0, loadPalette,(void *)0,(int)0);
	menu->add("&File/&Open tiles",(int)0,load_tiles,(void*)0,(int)0);
	menu->add("&File/&Open Truecolor Tiles",(int)0,load_truecolor_tiles,0,(int)0);
	menu->add("&File/&Append tiles",(int)0,load_tiles,(void*)1,(int)0);
	menu->add("&File/&Open tile map and if NES attrabiuts",(int)0,load_tile_map,(void *)0,(int)0);
	menu->add("&File/&import image to tilemap",(int)0,load_image_to_tilemap,(void *)0,(int)0);
	menu->add("&File/&save tilemap as image",(int)0,save_tilemap_as_image,(void *)0,(int)0);
	menu->add("&File/&Save Palette",  0, save_palette,(void*)0);
	menu->add("&File/&Save tiles",0,save_tiles,0,0);
	menu->add("&File/&Save truecolor tiles",0,save_tiles_truecolor,0,0);
	menu->add("&File/&Save tile map and if nes attributes",0,save_map,0,0);
	menu->add("&Palette Actions/&generate optimal palette with x amount of colors",0,generate_optimal_palette,(void *)0,(int)0);
	menu->add("&Palette Actions/&Clear entire Palette",0,clearPalette,(void *)0,(int)0);
	menu->add("&Tile Actions/&Append blank tile to end of buffer",0,new_tile,0,0);
	menu->add("&Tile Actions/&Fill tile with selected color",0,fill_tile,(void *)0,(int)0);
	menu->add("&Tile Actions/&Fill tile with color 0",0,blank_tile,0,0);
	menu->add("&Tile Actions/&Remove duplicate truecolor tiles",0,remove_duplicate_truecolor,0,0);
	menu->add("&Tile Actions/&Remove duplicate tiles",0,remove_duplicate_tiles,0,0);
	menu->add("&Tile Actions/&update dither all tiles",0,update_all_tiles,0,0);
	menu->add("&Tile Actions/&Delete currently selected tile",0,delete_tile_at_location,(void *)0,(int)0);
	menu->add("&TileMap Actions/&Remove tiles after x",0,tilemap_remove_callback,0,0);
	menu->add("&TileMap Actions/&Toggle TrueColor Viewing (defaults to off)",0,trueColTileToggle,0,0);
	menu->add("&TileMap Actions/&Pick Tile row based on delta",0,tileDPicker,0,0);
	menu->add("&TileMap Actions/&Auto determin if use shadow highlight",0,shadow_highligh_findout,(void *)0,(int)0);
	menu->add("&TileMap Actions/&Dither tilemap as image",0,dither_tilemap_as_image,(void *)0,(int)0);
	menu->add("&TileMap Actions/&File tile map with selection includeing attrabutes",0,fill_tile_map_with_tile,(void *)0,(int)0);
	menu->add("&Help/&About",0,showAbout,0,0);
	tile_placer_tile_offset_y=default_tile_placer_tile_offset_y;
	true_color_box_x=default_true_color_box_x;
	true_color_box_y=default_true_color_box_y;
	tile_edit_truecolor_off_x=default_tile_edit_truecolor_off_x;
	tile_edit_truecolor_off_y=default_tile_edit_truecolor_off_y;
	{ /*Fl_Tabs**/ the_tabs = new Fl_Tabs(0, 24, 800, 576);
	the_tabs->callback(set_mode_tabs);
	int rx,ry,rw,rh;
	the_tabs->client_area(rx,ry,rw,rh);
		{ Fl_Group* o = new Fl_Group(rx, ry, rw, rh, "palette editor");
			//cout << "palette editor as group: " << o->as_group() << endl;
			pal_id=(intptr_t)o->as_group();
			//stuff realed to this group should go here
			palEdit.more_init(4);
			pal_size = new Fl_Hor_Value_Slider(128,384,320,24,"Palette box size");
			pal_size->minimum(1); pal_size->maximum(42);
			pal_size->step(1);
			pal_size->value(32);
			pal_size->align(FL_ALIGN_LEFT);
			pal_size->callback(update_box_size);
			ditherPower = new Fl_Hor_Value_Slider(128,416,320,24,"Dither Power");
			ditherPower->tooltip("A lower value resualts in more dithering artifacts a higer value resualts in less artifacts");
			ditherPower->minimum(1); ditherPower->maximum(255);
			ditherPower->step(1);
			ditherPower->value(16);
			ditherPower->align(FL_ALIGN_LEFT);
	{ shadow_highlight_switch = new Fl_Group(112, 288, 800, 480);
		{ Fl_Round_Button* o = new Fl_Round_Button(96, 280, 64, 32, "Normal");
		o->type(FL_RADIO_BUTTON);
		o->tooltip("This is the default sega genesis color.When shadow/highlight mode is disabled all tiles will look like this however when enabling shadow higligh mode and a tile is set to high prioraty you will the tile will use these set of colors");
		o->callback((Fl_Callback*) set_palette_type_callback,(void *)0);
		o->set();
		} // Fl_Round_Button* o
		{ Fl_Round_Button* o = new Fl_Round_Button(164, 280, 64, 32, "Shadow");
		o->tooltip("This mode uses the color sets that the vdp uses when shadow highlight mode is enabled by setting bit 3 (the LSB being bit 0) to 1 in the vdp register 0C also for the tile to be shadowed the tile's priority must be set at 0 or low priority");
		o->type(FL_RADIO_BUTTON);
       o->callback((Fl_Callback*) set_palette_type_callback,(void *)8);
      } // Fl_Round_Button* o
      { Fl_Round_Button* o = new Fl_Round_Button(240, 280, 64, 32, "Highlight");
	  o->tooltip("This mode uses the color sets that a highlighted sprite or tile uses to make a tile highlighted use a mask sprite");
        o->type(FL_RADIO_BUTTON);
        o->callback((Fl_Callback*) set_palette_type_callback,(void *)16);
      } // Fl_Round_Button* o
      shadow_highlight_switch->end();
		} // Fl_Group* o
			{
				Fl_Group *o = new Fl_Group(96, 312, 800, 480);
				{
					Fl_Round_Button* o = new Fl_Round_Button(96, 312, 96, 32, "Sega Genesis");
					o->tooltip("Sets the editing mode to Sega Genesis or Sega Mega Drive");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_game_system,(void *)sega_genesis);
					o->set();
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(224, 312, 64, 32, "NES");
					o->tooltip("Sets the editing mode to Nintendo Entertamint System or Famicon");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_game_system,(void *)NES);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons
			{
				Fl_Group *o = new Fl_Group(0, 0, 800, 500);
				{
					Fl_Round_Button* o = new Fl_Round_Button(128, 440, 96, 32, "Floyd Steinberg");
					o->type(FL_RADIO_BUTTON);
					o->set();
					o->callback((Fl_Callback*) set_ditherAlg,(void *)0);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(256, 440, 64, 32, "Reimesha");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)1);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(384, 440, 64, 32, "Nearest Color");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_ditherAlg,(void *)2);
				} // Fl_Round_Button* o
				o->end();
			} // End of buttons
			{ Fl_Group *o = new Fl_Group(304, 192, 88, 96);
				{
					palType[0] = new Fl_Round_Button(304, 192, 64, 32, "Free");
					palType[0]->type(FL_RADIO_BUTTON);
					palType[0]->set();
					palType[0]->callback((Fl_Callback*) setPalType,(void *)0);
					palType[0]->tooltip(freeDes);
					palType[1] = new Fl_Round_Button(304, 224, 72, 32, "Locked");
					palType[1]->type(FL_RADIO_BUTTON);
					palType[1]->callback((Fl_Callback*) setPalType,(void *)1);
					palType[1]->tooltip(lockedDes);
					palType[2] = new Fl_Round_Button(304, 256, 88, 32, "Reserved");
					palType[2]->type(FL_RADIO_BUTTON);
					palType[2]->callback((Fl_Callback*) setPalType,(void *)2);
					palType[2]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
      			o->end();
			} // Fl_Group* o
		{ Fl_Group* o = new Fl_Group(5, 48, 800, 567, "Tile Editor");
			//stuff realed to this group should go here
			tile_edit_id=(intptr_t)o->as_group();
		//o->callback(set_mode_tabs);
			{ Fl_Group* o = new Fl_Group(0, 0, 800, 567);
				{ Fl_Round_Button* o = new Fl_Round_Button(384, default_palette_bar_offset_y+40, 56, 32, "Row 0");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(FL_RADIO_BUTTON);
					o->set();
					o->callback((Fl_Callback*) set_tile_row,(void *)0);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(448, default_palette_bar_offset_y+40, 56, 32, "Row 1");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)1);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(512, default_palette_bar_offset_y+40, 56, 32, "Row 2");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)2);
				} // Fl_Round_Button* o
				{ Fl_Round_Button* o = new Fl_Round_Button(576, default_palette_bar_offset_y+40, 56, 32, "Row 3");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)3);
				} // Fl_Round_Button* o
			o->end();
		} // Fl_Group* o
			{ Fl_Check_Button* o = new Fl_Check_Button(640,default_palette_bar_offset_y+40,120,32,"Show grid?");
				o->callback(set_grid);
				o->tooltip("This button Toggles wheater or not you which to see a grid while editing your tiles. A grid can help you see the spacing betwen each pixel.");
			}
			{ Fl_Button *o = new Fl_Button(540, default_palette_bar_offset_y, 120, 32, "New Tile");//these button should be inline with the palette bar
				o->tooltip("This will append a blank tile to the tile buffer in the ram.");
				o->callback(new_tile);
			}
			{ Fl_Button *o = new Fl_Button(668, default_palette_bar_offset_y, 128, 32, "Delete Selected Tile");
				o->tooltip("This button will delete the curretly selected tile");
				o->callback(delete_tile_at_location);
			}
			tileEdit_pal.more_init();
			rgb_red = new Fl_Hor_Value_Slider(64,default_palette_bar_offset_y+136,128,24,"RGB red");
			rgb_red->minimum(0);
			rgb_red->maximum(255);
			rgb_red->step(1);
			rgb_red->value(0);
			rgb_red->align(FL_ALIGN_LEFT);
			rgb_red->callback(update_truecolor,(void *)0);
			rgb_green = new Fl_Hor_Value_Slider(240,default_palette_bar_offset_y+136,128,24,"Green");
			rgb_green->minimum(0);
			rgb_green->maximum(255);
			rgb_green->step(1);
			rgb_green->value(0);
			rgb_green->align(FL_ALIGN_LEFT);
			rgb_green->callback(update_truecolor,(void *)1);
			rgb_blue = new Fl_Hor_Value_Slider(402,default_palette_bar_offset_y+136,128,24,"Blue");
			rgb_blue->minimum(0);
			rgb_blue->maximum(255);
			rgb_blue->step(1);
			rgb_blue->value(0);
			rgb_blue->align(FL_ALIGN_LEFT);
			rgb_blue->callback(update_truecolor,(void *)2);
			rgb_alpha = new Fl_Hor_Value_Slider(576,default_palette_bar_offset_y+136,128,24,"Alpha");
			rgb_alpha->minimum(0);
			rgb_alpha->maximum(255);
			rgb_alpha->step(1);
			rgb_alpha->value(0);
			rgb_alpha->align(FL_ALIGN_LEFT);
			rgb_alpha->callback(update_truecolor,(void *)3);
			{ Fl_Group *o = new Fl_Group(304, 96, 88, 96);
				{
					palType[3] = new Fl_Round_Button(304, 96, 64, 32, "Free");
					palType[3]->type(FL_RADIO_BUTTON);
					palType[3]->set();
					palType[3]->callback((Fl_Callback*) setPalType,(void *)0);
					palType[3]->tooltip(freeDes);
					palType[4] = new Fl_Round_Button(304, 128, 72, 32, "Locked");
					palType[4]->type(FL_RADIO_BUTTON);
					palType[4]->callback((Fl_Callback*) setPalType,(void *)1);
					palType[4]->tooltip(lockedDes);
					palType[5] = new Fl_Round_Button(304, 160, 88, 32, "Reserved");
					palType[5]->type(FL_RADIO_BUTTON);
					palType[5]->callback((Fl_Callback*) setPalType,(void *)2);
					palType[5]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
			tile_edit_offset_x=default_tile_edit_offset_x;
			tile_edit_offset_y=default_tile_edit_offset_y;
			tile_size = new Fl_Hor_Value_Slider(496,default_palette_bar_offset_y+72,304,24,"Tile Zoom Factor");
			tile_size->tooltip("This slider sets magnification a value of 10 would mean the image is being displayed 10 times larger");
			tile_size->minimum(1);
			tile_size->maximum(64);
			tile_size->step(1);
			tile_size->value(46);
			tile_size->align(FL_ALIGN_LEFT);
			tile_size->callback(update_offset_tile_edit);
			//now for the tile select slider
			tile_select = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+104,320,24,"Tile Select");
			tile_select->tooltip("This slider selects which tile that you are editing the first tile is zero");
			tile_select->minimum(0);
			tile_select->maximum(0);
			tile_select->step(1);
			tile_select->value(0);
			tile_select->align(FL_ALIGN_LEFT);
			tile_select->callback(set_tile_current);
			o->end();
		}
		{ Fl_Group* o = new Fl_Group(rx,ry,rw,rh,"Plane Mapping Editor");
			//o->callback(set_mode_tabs);
			tile_place_id=(intptr_t)o->as_group();
			{
				Fl_Group* o = new Fl_Group(tile_place_buttons_x_off, 208, 60, 128);
				{
					Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 208, 60, 32, "Row 0");
					o->type(FL_RADIO_BUTTON);
					o->set();
					o->callback((Fl_Callback*) set_tile_row,(void *)0);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 240, 60, 32, "Row 1");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)1);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 272, 60, 32, "Row 2");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)2);
				} // Fl_Round_Button* o
				{
					Fl_Round_Button* o = new Fl_Round_Button(tile_place_buttons_x_off, 304, 60, 32, "Row 3");
					//o->tooltip("Radio button, only one button is set at a time, in the corresponding group.");
					o->type(FL_RADIO_BUTTON);
					o->callback((Fl_Callback*) set_tile_row,(void *)3);
				} // Fl_Round_Button* o
				o->end();
			} // Fl_Group* o
			map_w = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+72,312,24,"Map width");
			map_w->minimum(1);
			map_w->maximum(0xFFFF);
			map_w->step(1);
			map_w->value(2);
			map_w->align(FL_ALIGN_LEFT);
			map_w->callback(callback_resize_map);
			map_h = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+104,312,24,"Map height");
			map_h->minimum(1);
			map_h->maximum(0xFFFF);
			map_h->step(1);
			map_h->value(2);
			map_h->align(FL_ALIGN_LEFT);
			map_h->callback(callback_resize_map);
			map_x_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y-42, 800-8-default_map_off_x, 24);
			map_x_scroll->value(0,0,0,0);
			map_x_scroll->type(FL_HORIZONTAL);
			map_x_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map left and right.");
			map_x_scroll->callback(update_map_scroll_x);
			map_y_scroll = new Fl_Scrollbar(default_map_off_x-32, default_map_off_y, 24, 600-8-default_map_off_y);
			map_y_scroll->value(0,0,0,0);
			//map_x_scroll->type(FL_HORIZONTAL);
			map_y_scroll->tooltip("Use this scroll bar to move around the tile map if you are zoomed in and there is not enough room to display the entire tilemap at once. This scroll bar will move the map up and down.");
			map_y_scroll->callback(update_map_scroll_y);
			//now for the tile select slider
			tile_select_2 = new Fl_Hor_Value_Slider(480,default_palette_bar_offset_y+40,312,24,"Tile Select");
			tile_select_2->tooltip("This slider allows you to choice which tile you would like to place on the map remember you can both horizontally and vertically flip the tile once placed on the map and select which row the tile uses");
			tile_select_2->minimum(0);
			tile_select_2->maximum(0);
			tile_select_2->step(1);
			tile_select_2->value(0);
			tile_select_2->align(FL_ALIGN_LEFT);
			tile_select_2->callback(set_tile_current);
			tileMap_pal.more_init();
			//buttons for tile settings
			{ Fl_Group *o = new Fl_Group(304, 96, 88, 96);
				{
					palType[6] = new Fl_Round_Button(304, 96, 64, 32, "Free");
					palType[6]->type(FL_RADIO_BUTTON);
					palType[6]->set();
					palType[6]->callback((Fl_Callback*) setPalType,(void *)0);
					palType[6]->tooltip(freeDes);
					palType[7] = new Fl_Round_Button(304, 128, 72, 32, "Locked");
					palType[7]->type(FL_RADIO_BUTTON);
					palType[7]->callback((Fl_Callback*) setPalType,(void *)1);
					palType[7]->tooltip(lockedDes);
					palType[8] = new Fl_Round_Button(304, 160, 88, 32, "Reserved");
					palType[8]->type(FL_RADIO_BUTTON);
					palType[8]->callback((Fl_Callback*) setPalType,(void *)2);
					palType[8]->tooltip(reservedDes);
					o->end();
				} // End of buttons
			}//end of group
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,184,64,32,"Show only selected row");
				o->callback(toggleRowSolo);
				o->tooltip("When checked Tiles that do not use the selected row will not be drawn");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,336,64,32,"hflip");
				o->callback(set_hflip);
				o->tooltip("This sets whether or not the tile is flipped horizontally");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,368,64,32,"vflip");
				o->callback(set_vflip);
				o->tooltip("This sets whether or not the tile is flipped vertically");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,400,72,32,"priority");
				o->callback(set_prio_callback);
				o->tooltip("If checked tile is high priority");
			}
			{ Fl_Check_Button* o = new Fl_Check_Button(tile_place_buttons_x_off,432,96,32,"Show grid?");
				o->callback(set_grid_placer);
				o->tooltip("This button Toggles whether or not a grid is visible over the tilemap this will allow you to easily see were each tile is");
			}
			place_tile_size = new Fl_Hor_Value_Slider(tile_place_buttons_x_off+120,496,128,24,"Tile Zoom Factor");
			place_tile_size->minimum(1);
			place_tile_size->maximum(16);
			place_tile_size->step(1);
			place_tile_size->value(12);
			place_tile_size->align(FL_ALIGN_LEFT);
			place_tile_size->callback(update_map_size);
			place_tile_size->tooltip("By changing this slider you are changing the magnification of the tile for example if this slider was set to 10 that would mean that the tile is magnified by a factor of 10");
			o->end();
		}
	}
}
int main(int argc, char **argv)
{
	printf("Welcome to Retro graphics Toolkit\nWritten by sega16/nintendo8\nBuild %s %s\n",__DATE__,__TIME__);
	initProject();
	window->resizable(window);
	Fl::scheme("plastic");
	fl_register_images();
	showTrueColor=false;
	//// For a nicer looking browser under linux, call Fl_File_Icon::load_system_icons();
	//// (If you do this, you'll need to link with fltk_images)
	//// NOTE: If you do not load the system icons, the file chooser will still work, but
	////       no icons will be shown. However, this means you do not need to link in the
	////       fltk_images library, potentially reducing the size of your executable.
	//// Loading the system icons is not required by the OSX or Windows native file choosers.

#if !defined(WIN32) && !defined(__APPLE__)
  Fl_File_Icon::load_system_icons();
#endif

 /* int argn = 1;
#ifdef __APPLE__
  // OS X may add the process number as the first argument - ignore
  if (argc>argn && strncmp(argv[1], "-psn_", 5)==0)
    argn++;
#endif*/

 // window.end();
  window->show(argc,argv);
	return Fl::run();
}
