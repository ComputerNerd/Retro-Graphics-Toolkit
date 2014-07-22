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
#include "kens.h"
#include "filemisc.h"
void save_tiles(Fl_Widget*,void*){
	if (load_file_generic("Pick a location to save tiles",true) == true){
		int type=askSaveType();
		uint8_t compression=fl_choice("What kind of compression do you want used?","Uncompressed","Nemesis","Kosinski");
		FILE* myfile;
		uint8_t* compdat;
		uint32_t compsize;
		if(type)
			myfile = fopen(the_file.c_str(),"w");
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (likely(myfile!=0)){
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
				if(!compdat)
					show_malloc_error(compsize)
				std::string output=outfun.str();
				output.copy((char *)compdat,compsize);
				printf("Compressed to %d uncompressed would be %d so therefore the file the ratio is %f\n",compsize,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize,(double)compsize/(double)((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize)*100.0);
			}
			if (type){
				char comment[2048];
				snprintf(comment,2048,"%d tiles",currentProject->tileC->tiles_amount+1);
				if (compression){
					if(saveBinAsText(compdat,compsize,myfile,type,comment,"tileDat")==false){
						free(compdat);
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}else{
					if (saveBinAsText(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize,myfile,type,comment,"tileDat")==false) {
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}
			}else{
				if(compression)
					fwrite(compdat,1,compsize,myfile);
				else
					fwrite(currentProject->tileC->tileDat,currentProject->tileC->tileSize,(currentProject->tileC->tiles_amount+1),myfile);
			}
			if(compression==1)
				free(compdat);
		}else
			fl_alert("Error: can not save file %s",the_file.c_str());
		fclose(myfile);
	}
}
void load_tiles(Fl_Widget*,void*o){
	//if o=0 load if o=1 append if o=2 load at
	//format row,append
	uint32_t file_size;
	int mode=(uintptr_t)o;
	char * returned=(char*)fl_input("What row should these tiles use?\nEnter 0 to 3 to selected a row or -1 to -4 to auto determine based on tilemap\nWhen specifing a negative number to figure out what the default will be use this formula abs(row)-1","-1");
	if (!returned)
		return;
	if (!verify_str_number_only(returned))
		return;
	int row=atoi(returned);
	if (unlikely((row > 3) || (row < -4))){
		fl_alert("You entered %d which is out of range it must be in range of -4 to 3",row);
		return;
	}
	uint8_t defaultRow=row >= 0 ? row:abs(row)-1;
	int compression=fl_choice("What format is the tile?","Uncompressed","Nemesis Compressed","Kosinski");
	bool alphaZero=fl_ask("Set color #0 to alpha 0 instead of 255")?true:false;
	if (load_file_generic()){
		FILE * myfile;
		std::stringstream outDecomp;
		myfile = fopen(the_file.c_str(),"rb");
		if (likely(myfile!=0)){
			fseek(myfile, 0L, SEEK_END);
			file_size = ftell(myfile);//file.tellg();
			rewind(myfile);
			uint8_t truecolor_multiplier;
			truecolor_multiplier=256/currentProject->tileC->tileSize;
			if(compression){
				uint8_t * datcmp=(uint8_t *)malloc(file_size);
				if (unlikely(!datcmp))
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
				if ((file_size/currentProject->tileC->tileSize)*currentProject->tileC->tileSize != file_size){
					fl_alert("Error: This is not a valid tile file each tile is %d bytes and this file is not a multiple of %d so it is not a valid tile file",currentProject->tileC->tileSize,currentProject->tileC->tileSize);
					fclose(myfile);
					return;//return so that the file does not get loaded
				}
			}
			uint32_t offset_tiles;
			uint32_t offset_tiles_bytes;
			if(mode==2){
				const char * str=fl_input("Counting from zero which tile should this start at?");
				if(!str){
					return;
					fclose(myfile);
				}
				if(!verify_str_number_only((char*)str)){
					return;
					fclose(myfile);
				}
				int off=atoi(str);
				if(off>=0){
					offset_tiles=off;
					offset_tiles_bytes=offset_tiles*currentProject->tileC->tileSize;
					//if(
					//currentProject->tileC->tileDat=(uint8_t*)realloc(currentProject->tileC->tileDat,(file_size+((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize)));
				}else{
					fl_alert("You must enter a number greater than or equal to zero");
					fclose(myfile);
					return;
				}
			}else if(mode==1){
				offset_tiles=currentProject->tileC->tiles_amount+1;
				offset_tiles_bytes=offset_tiles*currentProject->tileC->tileSize;
				currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,file_size+((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize));
				if(!currentProject->tileC->tileDat){
					if (compression==0)
						fclose(myfile);
					show_realloc_error(file_size+((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize))
				}
			}else{
				currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,file_size);
				if(!currentProject->tileC->tileDat){
					if (!compression)
						fclose(myfile);
					show_realloc_error(file_size)
				}
				offset_tiles=0;
				offset_tiles_bytes=0;
			}
			if(compression){
				std::string output=outDecomp.str();
				output.copy((char *)currentProject->tileC->tileDat+offset_tiles_bytes,file_size);
			}else{
				fread(currentProject->tileC->tileDat+offset_tiles_bytes,1,file_size,myfile);
				fclose(myfile);
			}
			currentProject->tileC->truetileDat = (uint8_t *)realloc(currentProject->tileC->truetileDat,(file_size*truecolor_multiplier)+(offset_tiles_bytes*truecolor_multiplier));
			if(!currentProject->tileC->truetileDat)
				show_malloc_error(file_size*truecolor_multiplier)
			for(uint32_t c=offset_tiles;c<(file_size/currentProject->tileC->tileSize)+offset_tiles;c++) {
				if(row < 0){
					uint32_t x,y;
					uint8_t foundRow=defaultRow;
					for(y=0;y<currentProject->tileMapC->mapSizeHA;++y){
						for(x=0;x<currentProject->tileMapC->mapSizeW;++x){
							if(currentProject->tileMapC->get_tile(x,y) == c) {
								foundRow=currentProject->tileMapC->get_palette_map(x,y);
								goto doTile;
							}
						}
					}
doTile:
					tileToTrueCol(&currentProject->tileC->tileDat[(c*currentProject->tileC->tileSize)],&currentProject->tileC->truetileDat[(c*256)],foundRow,true,alphaZero);
				}else
					tileToTrueCol(&currentProject->tileC->tileDat[(c*currentProject->tileC->tileSize)],&currentProject->tileC->truetileDat[(c*256)],defaultRow,true,alphaZero);
			}
			currentProject->tileC->tiles_amount=(file_size/currentProject->tileC->tileSize)-1;
			currentProject->tileC->tiles_amount+=offset_tiles;
			updateTileSelectAmt();
			window->tile_select->value(0);
			window->tile_select_2->value(0);
			window->redraw();
		}else
			fl_alert("The file %s Cannot be loaded",the_file.c_str());
	}
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
		if (file_size&255){
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
		updateTileSelectAmt();
		window->redraw();
	}
}
void save_tiles_truecolor(Fl_Widget*,void*){
	if (load_file_generic("Save truecolor tiles",true)){
		FILE * myfile;
		myfile = fopen(the_file.c_str(),"wb");
		if (myfile){
			fwrite(currentProject->tileC->truetileDat,1,(currentProject->tileC->tiles_amount+1)*256,myfile);
			puts("Great Sucess File Saved!");
			fclose(myfile);
		}else
			fl_alert("Error: can not save file %s",the_file.c_str());
	}
}
