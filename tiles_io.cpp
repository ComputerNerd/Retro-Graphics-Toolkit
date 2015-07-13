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
#include "project.h"
#include "macros.h"
#include "filemisc.h"
#include "compressionWrapper.h"
#include "gui.h"
#include "class_global.h"
void save_tiles(Fl_Widget*,void*){
	fileType_t type=askSaveType();
	int clipboard;
	if(type){
		clipboard=clipboardAsk();
		if(clipboard==2)
			return;
	}else
		clipboard=0;
	bool pickedFile;
	if(clipboard)
		pickedFile=true;
	else
		pickedFile=load_file_generic("Pick a location to save tiles",true);
	if(pickedFile){
		uint8_t*savePtr;
		enum tileType tt=currentProject->getTileType();
		switch(tt){
			case LINEAR:
				savePtr=(uint8_t*)currentProject->tileC->toLinear();
			break;
			case PLANAR_LINE:
				savePtr=(uint8_t*)currentProject->tileC->toLinePlanar();
			break;
			case PLANAR_TILE:
				savePtr=currentProject->tileC->tDat.data();
			break;
		}
		int compression=compressionAsk();
		if(compression<0)
			return;
		FILE* myfile;
		uint8_t* compdat;
		size_t compsize;
		if(clipboard)
			myfile=0;
		else if(type)
			myfile = fopen(the_file.c_str(),"w");
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (likely(myfile||clipboard)){
			if(compression)
				compdat=(uint8_t*)encodeType(savePtr,currentProject->tileC->tileSize*(currentProject->tileC->amt),compsize,compression);
			if (type){
				char comment[2048];
				snprintf(comment,2048,"%d tiles %s",currentProject->tileC->amt,typeToText(compression));
				if (compression){
					if(!saveBinAsText(compdat,compsize,myfile,type,comment,"tileDat",8)){
						free(compdat);
						fl_alert("Error: can not save file %s",the_file.c_str());
						if(tt!=PLANAR_TILE)
							free(savePtr);
						return;
					}
				}else{
					if(!saveBinAsText(savePtr,(currentProject->tileC->amt)*currentProject->tileC->tileSize,myfile,type,comment,"tileDat",32)){
						fl_alert("Error: can not save file %s",the_file.c_str());
						if(tt!=PLANAR_TILE)
							free(savePtr);
						return;
					}
				}
			}else{
				if(compression)
					fwrite(compdat,1,compsize,myfile);
				else
					fwrite(savePtr,currentProject->tileC->tileSize,(currentProject->tileC->amt),myfile);
			}
			if(compression)
				free(compdat);
			if(tt!=PLANAR_TILE)
				free(savePtr);
		}else
			fl_alert("Error: can not save file %s",the_file.c_str());
		if(myfile)
			fclose(myfile);
	}
}
void load_tiles(Fl_Widget*,void*o){
	//if o=0 load if o=1 append if o=2 load at
	size_t file_size;
	unsigned mode=(uintptr_t)o;
	char * returned=(char*)fl_input("What row should these tiles use?\nEnter 0 to 3 to selected a row or -1 to -4 to auto determine based on tilemap\nWhen specifing a negative number to figure out what the default will be use this formula abs(row)-1","-1");
	if (unlikely(!returned))
		return;
	if (unlikely(!verify_str_number_only(returned)))
		return;
	int row=atoi(returned);
	if (unlikely((row > 3) || (row < -4))){
		fl_alert("You entered %d which is out of range it must be in range of -4 to 3",row);
		return;
	}
	uint8_t defaultRow=row >= 0 ? row:abs(row)-1;
	int compression=compressionAsk();
	bool alphaZero=fl_ask("Set color #0 to alpha 0 instead of 255")?true:false;
	if (load_file_generic()){
		FILE * myfile;
		std::string output;
		myfile = fopen(the_file.c_str(),"rb");
		if (likely(myfile!=0)){
			fseek(myfile, 0L, SEEK_END);
			file_size = ftell(myfile);
			rewind(myfile);
			unsigned truecolor_multiplier;
			truecolor_multiplier=256/currentProject->tileC->tileSize;
			if(compression)
				output=decodeTypeStr(the_file.c_str(),file_size,compression);
			else{
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
				}else{
					fl_alert("You must enter a number greater than or equal to zero");
					fclose(myfile);
					return;
				}
			}else if(mode==1){
				offset_tiles=currentProject->tileC->amt;
				offset_tiles_bytes=offset_tiles*currentProject->tileC->tileSize;
			}else{
				offset_tiles=0;
				offset_tiles_bytes=0;
			}
			if(mode==2){
				if(offset_tiles+(file_size/currentProject->tileC->tileSize)>=currentProject->tileC->amt)
					currentProject->tileC->tDat.resize(offset_tiles_bytes+file_size);
			}else
				currentProject->tileC->tDat.resize(offset_tiles_bytes+file_size);
			if(compression)
				output.copy((char *)currentProject->tileC->tDat.data()+offset_tiles_bytes,file_size);
			else{
				fread(currentProject->tileC->tDat.data()+offset_tiles_bytes,1,file_size,myfile);
				fclose(myfile);
			}
			if(currentProject->getTileType()!=PLANAR_TILE)
				currentProject->tileC->toPlanar(currentProject->getTileType(),offset_tiles,offset_tiles+(file_size/currentProject->tileC->tileSize));
			if(mode==2){
				if(offset_tiles+(file_size/currentProject->tileC->tileSize)>=currentProject->tileC->amt)
					currentProject->tileC->truetDat.resize((file_size*truecolor_multiplier)+(offset_tiles_bytes*truecolor_multiplier));
			}else
				currentProject->tileC->truetDat.resize((file_size*truecolor_multiplier)+(offset_tiles_bytes*truecolor_multiplier));
			for(uint32_t c=offset_tiles;c<(file_size/currentProject->tileC->tileSize)+offset_tiles;c++) {
				if(row < 0){
					uint32_t x,y;
					uint8_t foundRow=defaultRow;
					for(y=0;y<currentProject->tms->maps[currentProject->curPlane].mapSizeHA;++y){
						for(x=0;x<currentProject->tms->maps[currentProject->curPlane].mapSizeW;++x){
							if(currentProject->tms->maps[currentProject->curPlane].get_tile(x,y) == c) {
								foundRow=currentProject->tms->maps[currentProject->curPlane].getPalRow(x,y);
								goto doTile;
							}
						}
					}
doTile:
					currentProject->tileC->tileToTrueCol(&currentProject->tileC->tDat[(c*currentProject->tileC->tileSize)],&currentProject->tileC->truetDat[(c*256)],foundRow,true,alphaZero);
				}else
					currentProject->tileC->tileToTrueCol(&currentProject->tileC->tDat[(c*currentProject->tileC->tileSize)],&currentProject->tileC->truetDat[(c*256)],defaultRow,true,alphaZero);
			}
			if(mode==2){
				if(offset_tiles+(file_size/currentProject->tileC->tileSize)>=currentProject->tileC->amt){
					currentProject->tileC->amt=(file_size/currentProject->tileC->tileSize);
					currentProject->tileC->amt+=offset_tiles;
				}
			}else{
				currentProject->tileC->amt=(file_size/currentProject->tileC->tileSize);
				currentProject->tileC->amt+=offset_tiles;
			}
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
		FILE * myfile;
		myfile = fopen(the_file.c_str(),"rb");
		fseek(myfile, 0L, SEEK_END);
		file_size = ftell(myfile);
		if (file_size&255){
			fl_alert("Error: this file is not a multiple of 256 so it is not a valid truecolor tiles. The file size is: %d",file_size);
			fclose(myfile);
			return;
		}
		currentProject->tileC->truetDat.resize(file_size);
		currentProject->tileC->tDat.resize(file_size*currentProject->tileC->tileSize/currentProject->tileC->tcSize);
		rewind(myfile);
		fread(currentProject->tileC->truetDat.data(),file_size,1,myfile);
		fclose(myfile);
		currentProject->tileC->amt=file_size/256;
		updateTileSelectAmt();
		window->redraw();
	}
}
void save_tiles_truecolor(Fl_Widget*,void*){
	if (load_file_generic("Save truecolor tiles",true)){
		FILE * myfile;
		myfile = fopen(the_file.c_str(),"wb");
		if (myfile){
			fwrite(currentProject->tileC->truetDat.data(),1,(currentProject->tileC->amt)*currentProject->tileC->tcSize,myfile);
			fclose(myfile);
		}else
			fl_alert("Error: can not save file %s",the_file.c_str());
	}
}
