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
void save_tiles(Fl_Widget*,void*){
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
				std::string input;
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
				std::string output=outfun.str();
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
void load_tiles(Fl_Widget*,void* split){
	//if append==1 then we will append data but if not it will erase over current tiles
	//format row,append
	uint32_t file_size;
	uint8_t append=(uintptr_t)split&0xFF;
		char * returned=(char *)fl_input("What row should these tiles use?\nEnter 0 to 3 to selected a row or -1 to -4 to auto determine based on tilemap\n The number after the negative symbol is the default row +1 if not tile is found","-1");
	if (returned==0)
		return;
	if (verify_str_number_only(returned) == false)
			return;
	int8_t row=atoi(returned);
	if (unlikely((row > 3) || (row < -4))){
		fl_alert("You entered %d which is out of range it must be in range of -4 to 3",row);
		return;
	}
	uint8_t defaultRow=row >= 0 ? row:abs(row)-1;
	uint8_t compression = fl_choice("What format is the tile?","Uncompressed","Nemesis Compressed","Kosinski");
	uint8_t trans=fl_ask("Set color #0 to alpha 0 instead of 255");
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
				std::string output=outDecomp.str();
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
					for (y=0;y<currentProject->tileMapC->mapSizeH;++y) {
						for (x=0;x<currentProject->tileMapC->mapSizeW;++x) {
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
			currentProject->tileC->tiles_amount+=offset_tiles;
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
void load_truecolor_tiles(Fl_Widget*,void*){
	//start by loading the file
	uint32_t file_size;
	if (load_file_generic() == true){
		//ios::ifstream file (the_file.c_str(), ios::in|ios::binary|ios::ate);
		FILE * myfile;
		myfile = fopen(the_file.c_str(),"rb");
		fseek(myfile, 0L, SEEK_END);
		file_size = ftell(myfile);
		if ((file_size/256)*256 != file_size){
			fl_alert("Error: this file is not a multiple of 256 so it is not a valid truecolor tiles. The file size is: %d",file_size);
			fclose(myfile);
			return;
		}
		free(currentProject->tileC->truetileDat);
		free(currentProject->tileC->tileDat);
		currentProject->tileC->truetileDat = (uint8_t *)malloc(file_size);
		if (currentProject->tileC->truetileDat == 0)
			show_malloc_error(file_size)
		switch (currentProject->gameSystem){
			case sega_genesis:
				currentProject->tileC->tileDat = (uint8_t *)malloc(file_size/6);
			break;
			case NES:
				currentProject->tileC->tileDat = (uint8_t *)malloc(file_size/12);
			break;
		}
		if (currentProject->tileC->tileDat == 0)
			show_malloc_error(file_size/6)
		rewind(myfile);
		fread(currentProject->tileC->truetileDat,file_size,1,myfile);
		fclose(myfile);
		currentProject->tileC->tiles_amount=file_size/256;
		currentProject->tileC->tiles_amount--;
		window->tile_select->maximum(currentProject->tileC->tiles_amount);
		window->tile_select_2->maximum(currentProject->tileC->tiles_amount);
		window->redraw();
	}
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
