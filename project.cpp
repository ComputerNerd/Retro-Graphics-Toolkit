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
#include "project.h"
#include <FL/Fl.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include "color_convert.h"
#include "zlibwrapper.h"
struct Project ** projects;
uint32_t projects_count;//holds how many projects there are this is needed for realloc when adding or removing function
struct Project * currentProject;
Fl_Slider* curPrj;
static const char * defaultName="Add a description here";
uint32_t curProjectID=0;
void initProject(void){
	projects = (struct Project **) malloc(sizeof(void *));
	projects[0] = new struct Project;
	currentProject=projects[0];
	projects_count=1;
	currentProject->tileC = new tiles;
	currentProject->tileMapC = new tileMap;
	currentProject->Name.assign(defaultName);
	memset(currentProject->palDat,0,128);
	memset(currentProject->rgbPal,0,192);
	memset(currentProject->palType,0,64);
	currentProject->sharePalette=-1;//Note always check to see if less than 0 do not use == -1
	currentProject->shareTiles=-1;
	currentProject->shareTileMap=-1;
	currentProject->gameSystem=sega_genesis;
}
bool appendProject(void){
	projects = (struct Project **) realloc(projects,(projects_count+1)*sizeof(void *));
	if (!projects){
		show_realloc_error((projects_count+1)*sizeof(void *))
		return false;
	}
	projects[projects_count] = new struct Project;
	projects[projects_count]->tileC = new tiles;
	projects[projects_count]->tileMapC = new tileMap;
	projects[projects_count]->Name.assign(defaultName);
	memset(projects[projects_count]->palDat,0,128);
	memset(projects[projects_count]->rgbPal,0,192);
	memset(projects[projects_count]->palType,0,64);
	projects[projects_count]->gameSystem=sega_genesis;
	projects[projects_count]->sharePalette=-1;
	projects[projects_count]->shareTiles=-1;
	projects[projects_count]->shareTileMap=-1;
	++projects_count;
	//Realloc could have changed address
	currentProject=projects[curProjectID];
	return true;
}
bool removeProject(uint32_t id){
	//removes selected project
	if (projects_count<=1){
		fl_alert("You must have atleast one project.");
		return false;
	}
	delete projects[id]->tileC;
	delete projects[id]->tileMapC;
	delete projects[id];
	if((id+1)!=projects_count)//Are we not removing the project last on the list?
		memmove(projects+id,projects+id+1,sizeof(void*)*(projects_count-id-1));
	projects_count--;
	projects = (struct Project **) realloc(projects,projects_count*sizeof(void *));
	return true;
}
static void invaildProject(void){
	fl_alert("This is not a vaild Retro Graphics Toolkit project");
}
void switchProject(uint32_t id){
	window->TxtBufProject->text(projects[id]->Name.c_str());//Make editor display new text
	window->GameSys[projects[id]->gameSystem]->setonly();
	switch(projects[id]->gameSystem){
		case sega_genesis:
			projects[id]->tileC->tileSize=32;
			shadow_highlight_switch->show();
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();
			set_palette_type(0);
			window->map_w->step(1);
			window->map_h->step(1);
		break;
		case NES:
			projects[id]->tileC->tileSize=16;
			shadow_highlight_switch->hide();
			updateNesTab(0);
			for(int temp_entry=0;temp_entry<64;++temp_entry){
				uint32_t rgb_out;
				uint8_t pal;
				pal=projects[id]->palDat[temp_entry];
				rgb_out=MakeRGBcolor(pal);
				projects[id]->rgbPal[temp_entry*3+2]=rgb_out&255;//blue
				projects[id]->rgbPal[temp_entry*3+1]=(rgb_out>>8)&255;//green
				projects[id]->rgbPal[temp_entry*3]=(rgb_out>>16)&255;//red
			}
			palEdit.changeSystem();
			tileEdit_pal.changeSystem();
			tileMap_pal.changeSystem();
			update_emphesis(0,0);
			window->map_w->step(2);
			window->map_h->step(2);
		break;
	}
	//Make sure sliders have correct values
	window->map_w->value(projects[id]->tileMapC->mapSizeW);
	window->map_h->value(projects[id]->tileMapC->mapSizeH);
	window->tile_select->maximum(projects[id]->tileC->tiles_amount);
	window->tile_select_2->maximum(projects[id]->tileC->tiles_amount);
	window->redraw();
}
bool loadProject(uint32_t id){
	if(!load_file_generic("Load project",false))
		return true;
	FILE * fi=fopen(the_file.c_str(),"rb");
	if(fgetc(fi)!='R'){
		invaildProject();
		fclose(fi);
		return false;
	}
	if(fgetc(fi)!='P'){
		invaildProject();
		fclose(fi);
		return false;
	}
	char d=fgetc(fi);
	if(d){
		char tmp[2];
		tmp[0]=d;
		tmp[1]=0;
		projects[id]->Name.assign(tmp);
		while(d!=0){
			d=fgetc(fi);
			projects[id]->Name.append(1,d);
		}
	}else
		projects[id]->Name.assign(defaultName);
	uint32_t version;
	fread(&version,1,sizeof(uint32_t),fi);
	fread(&projects[id]->gameSystem,1,sizeof(uint32_t),fi);
	int entries,eSize,tSize;
	switch(projects[id]->gameSystem){
		case sega_genesis:
			entries=64;
			eSize=2;
			tSize=32;
		break;
		case NES:
			entries=16;
			eSize=1;
			tSize=16;
		break;
	}
	fread(projects[id]->palDat,eSize,entries,fi);
	fread(projects[id]->palType,1,entries,fi);
	fread(&projects[id]->tileC->tiles_amount,1,sizeof(uint32_t),fi);
	projects[id]->tileC->tileDat=(uint8_t*)realloc(projects[id]->tileC->tileDat,projects[id]->tileC->tileSize*(projects[id]->tileC->tiles_amount+1));
	decompressFromFile(projects[id]->tileC->tileDat,projects[id]->tileC->tileSize*(projects[id]->tileC->tiles_amount+1),fi);
	projects[id]->tileC->truetileDat=(uint8_t*)realloc(projects[id]->tileC->truetileDat,256*(projects[id]->tileC->tiles_amount+1));
	decompressFromFile(projects[id]->tileC->truetileDat,256*(projects[id]->tileC->tiles_amount+1),fi);
	fread(&projects[id]->tileMapC->mapSizeW,1,sizeof(uint32_t),fi);
	fread(&projects[id]->tileMapC->mapSizeH,1,sizeof(uint32_t),fi);
	projects[id]->tileMapC->tileMapDat=(uint8_t*)realloc(projects[id]->tileMapC->tileMapDat,4*projects[id]->tileMapC->mapSizeW*projects[id]->tileMapC->mapSizeH);
	decompressFromFile(projects[id]->tileMapC->tileMapDat,4*projects[id]->tileMapC->mapSizeW*projects[id]->tileMapC->mapSizeH,fi);
	fclose(fi);
	return true;
}
static bool saveProjectFile(uint32_t id,FILE * fo){
	fputc('R',fo);
	fputc('P',fo);
	if(strcmp(projects[id]->Name.c_str(),defaultName)!=0)
		fputs(projects[id]->Name.c_str(),fo);
	fputc(0,fo);
	uint32_t version=currentProjectVersionNUM;
	fwrite(&version,sizeof(uint32_t),1,fo);
	fwrite(&projects[id]->gameSystem,sizeof(uint32_t),1,fo);
	int entries,eSize,tSize;
	switch(projects[id]->gameSystem){
		case sega_genesis:
			entries=64;
			eSize=2;
			tSize=32;
		break;
		case NES:
			entries=16;
			eSize=1;
			tSize=16;
		break;
	}
	fwrite(projects[id]->palDat,eSize,entries,fo);
	fwrite(projects[id]->palType,1,entries,fo);
	fwrite(&projects[id]->tileC->tiles_amount,1,sizeof(uint32_t),fo);
	compressToFile(projects[id]->tileC->tileDat,tSize*(projects[id]->tileC->tiles_amount+1),fo);
	compressToFile(projects[id]->tileC->truetileDat,256*(projects[id]->tileC->tiles_amount+1),fo);
	fwrite(&projects[id]->tileMapC->mapSizeW,1,sizeof(uint32_t),fo);
	fwrite(&projects[id]->tileMapC->mapSizeH,1,sizeof(uint32_t),fo);
	compressToFile(projects[id]->tileMapC->tileMapDat,4*projects[id]->tileMapC->mapSizeW*projects[id]->tileMapC->mapSizeH,fo);
	return true;
}
bool saveProject(uint32_t id){
	/*!
	File format
	char R
	char P
	Null terminated project description or just 0 if default string
	uint32_t version the reason this is stored is for backwards compability if I change the file format stats at version 0
	if (version >= 1) uint32_t have mask
	The format is
	bit 0 have map
	bit 1 have tiles
	if these bits are zero skip it 
	uint32_t game system
	palette data (128 bytes if sega genesis or 16 bytes if NES)
	Free locked reserved data 64 bytes if sega genesis or 16 if NES
	uint32_t tile count
	uint32_t compressed size tiles
	tile data will decompress to either 32 bytes * tile count if sega genesis or 16 bytes * tile count if NES and is compressed with zlib
	uint32_t compressed size truecolor tiles
	true color tile data always decompresses to 256 bytes * tile count and is compressed with zlib
	uint32_t map size w
	uint32_t map size h
	uint32_t compressed size map
	map data will decompress to map size w * map size h * 4 and is compressed with zlib
	*/
	//Start by creating a save file dialog
	if(!load_file_generic("Save project as...",true))
		return true;
	FILE * fo=fopen(the_file.c_str(),"wb");
	saveProjectFile(id,fo);
	fclose(fo);
	return true;
}
bool saveAllProjects(void){
	/*!The format is the same except it begins with
	 char R
	 char G
	 uint32_t amount of projects stored
	 (format described in saveProject is repeated n amount of times let n = amount of projects stored)
	*/
	if(!load_file_generic("Save projects grupt as...",true))
		return true;
	FILE * fo=fopen(the_file.c_str(),"wb");
	fputc('R',fo);
	fputc('G',fo);
	fwrite(&projects_count,1,sizeof(uint32_t),fo);
	for(int s=0;s<projects_count;++s)
		saveProjectFile(s,fo);
	fclose(fo);
	return true;
}
