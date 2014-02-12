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
tileMap::tileMap(){
	mapSizeW=2;
	mapSizeH=2;
	selection=false;
	tileMapDat=(uint8_t *)calloc(16,1);
}
tileMap::~tileMap(){
	free(tileMapDat);
}
bool tileMap::get_hflip(uint32_t x,uint32_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>3)&1;
}
bool tileMap::get_vflip(uint32_t x,uint32_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>4)&1;
}
bool tileMap::get_prio(uint32_t x,uint32_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>7)&1;
}
uint8_t tileMap::get_palette_map(uint32_t x,uint32_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>5)&3;
}
void tileMap::set_pal_row(uint32_t x,uint32_t y,uint8_t row){
	tileMapDat[((y*mapSizeW)+x)*4]&= ~(3 << 5);
	tileMapDat[((y*mapSizeW)+x)*4]|=row<<5;
}
uint32_t tileMap::get_tile(uint32_t x,uint32_t y){
	//first calulate which tile we want
	if (mapSizeW < x || mapSizeH < y){
		fl_alert("Error tried to get a non-existent tile on the map");
		return 0;
	}
	uint32_t selected_tile=((y*mapSizeW)+x)*4;
	//get both bytes
	uint8_t temp_1,temp_2,temp_3;
	temp_1=tileMapDat[selected_tile+1];//least sigficant is stored in the lowest address
	temp_2=tileMapDat[selected_tile+2];
	temp_3=tileMapDat[selected_tile+3];//most sigficant
	return (temp_1<<16)+(temp_2<<8)+temp_3;
}
int32_t tileMap::get_tileRow(uint32_t x,uint32_t y,uint8_t useRow){
	//first calulate which tile we want
	uint32_t selected_tile=((y*mapSizeW)+x)*4;
	//get both bytes
	if (((tileMapDat[selected_tile]>>5)&3) == useRow) {
		uint8_t temp_1,temp_2,temp_3;
		temp_1=tileMapDat[selected_tile+1];//least sigficant is stored in the lowest address
		temp_2=tileMapDat[selected_tile+2];
		temp_3=tileMapDat[selected_tile+3];//most sigficant
		return (temp_1<<16)+(temp_2<<8)+temp_3;
	}else
		return -1;
}

void tileMap::set_vflip(uint32_t x,uint32_t y,bool vflip_set){
	if (vflip_set)
		tileMapDat[((y*mapSizeW)+x)*4]|= 1 << 4;
	else
		tileMapDat[((y*mapSizeW)+x)*4]&= ~(1 << 4);
}


#if _WIN32
static inline uint16_t swap_word(uint16_t w){
	uint8_t a,b;
	a=w&255;
	b=w>>8;
	return (a<<8)|b;
}
#endif
bool tileMap::saveToFile(){
	/*!
	Saves tilemap to file returns true on success or cancelation
	returns false if there was an error but remeber if the user cancles this it is not an error
	*/
	//first see how this file should be saved
	uint16_t x,y;
	FILE * myfile;
	uint32_t fileSize;
	uint8_t type,compression, * mapptr;
	if (load_file_generic("Save tilemap to",true) == true){
		type=fl_choice("How would like this file saved?","Binary","C header",0);
		compression=fl_choice("In what format would you like this tilemap saved","Uncompressed","Enigma Compression",0);
		if (type == 1){
			char temp[2048];
			myfile = fopen(the_file.c_str(),"w");
			sprintf(temp,"//Width %d Height %d",mapSizeW,mapSizeH);
			fputs((const char *)temp,myfile);
			if(compression==1)
				fputs(" Enigma compressed",myfile);
			fputc('\n',myfile);
			fputs("const uint8_t mapDat[]={",myfile);
		}
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (likely(myfile!=0)){
			switch (currentProject->gameSystem){
				case sega_genesis:
					{
					uint16_t * TheMap;
					fileSize=(mapSizeW*mapSizeH)*2;
					TheMap = (uint16_t*)malloc(fileSize);
					for (y=0;y<mapSizeH;y++){
						for (x=0;x<mapSizeW;x++){
							uint32_t tile=get_tile(x,y);
							if (tile > 2047){
								printf("Warning tile value %d exceeded 2047 at x: %d y: %d\n",tile,x,y);
								tile=2047;
							}
							#if _WIN32
							tile=swap_word(tile);//mingw appears not to provide htobe16 function
							#else
							tile=htobe16(tile);//needs to be big endian
							#endif
							*TheMap=(uint16_t)tileMapDat[((y*mapSizeW)+x)*4];//get attributes
							*TheMap++|=(uint16_t)tile;//add tile
						}
					}
					TheMap-=mapSizeW*mapSizeH;//return to begining so it can be freeded and the file saved
					mapptr=(uint8_t*)TheMap;
				}//brackets used to prevent TheMap conflict
				break;
				case NES:
				{uint8_t * TheMap;
					fileSize=mapSizeW*mapSizeH;
					TheMap = (uint8_t *)malloc(fileSize);
					for (y=0;y<mapSizeH;y++){
						for (x=0;x<mapSizeW;x++){
							uint32_t tile=get_tile(x,y);
							if (tile > 255) {
								printf("Warning tile value %d exceeded 255 at x: %d y: %d\n",tile,x,y);
								tile=255;
							}
							*TheMap++=tile;
						}
					}
					TheMap-=mapSizeW*mapSizeH;//return to begining so it can be freeded and the file sized
					mapptr=TheMap;}
				break;
			}
			if(compression==1){
				std::string input,output;
				std::ostringstream outcomp;
				enigma ecomp;
				input.assign((const char*)mapptr,fileSize);
				std::stringstream iss(input);
				ecomp.encode(iss,outcomp);
				output=outcomp.str();
				fileSize=outcomp.str().length();
				mapptr=(uint8_t*)realloc(mapptr,fileSize);
				output.copy((char*)mapptr,fileSize);
				printf("compressed to %d bytes\n",fileSize);
			}
			if (type == 1){
				if (saveBinAsText(mapptr,fileSize,myfile)==false){
					free(mapptr);
					return false;
				}
				fputs("};",myfile);
			}else
				fwrite(mapptr,1,fileSize,myfile);
			free(mapptr);
			fclose(myfile);
			puts("File Saved");
		}
		else
			return false;
	}
	if (currentProject->gameSystem == NES){
		if (load_file_generic("Save attributes to",true) == true) {
			if (type == 1){
				myfile = fopen(the_file.c_str(),"w");
				fputs("const uint8_t attrDat[]={",myfile);
			}
			else
				myfile = fopen(the_file.c_str(),"wb");
			if (likely(myfile!=0)) {
				uint8_t * AttrMap = (uint8_t *)malloc((mapSizeW/4)*(mapSizeH/4));
				uint8_t * freeAttrMap=AttrMap;
				for (y=0;y<mapSizeH;y+=4) {
					for (x=0;x<mapSizeW;x+=4) {
						*AttrMap++ = get_palette_map(x,y) | (get_palette_map(x+2,y)<<2) | (get_palette_map(x,y+2) << 4) | (get_palette_map(x+2,y+2) << 6);
						printf("x: %d y: %d\n",x,y);
					}
				}
				//AttrMap-=(mapSizeW/4)*(mapSizeH/4);
				printf("%d %d\n",AttrMap,freeAttrMap);
				if (type == 1){
					if (saveBinAsText(freeAttrMap,(mapSizeW/4)*(mapSizeH/4),myfile)==false)
							return false;
						fputs("};",myfile);
				}
				else
					fwrite(freeAttrMap,1,(mapSizeW/4)*(mapSizeH/4),myfile);		
				free(freeAttrMap);
				fclose(myfile);
				puts("File Saved");
			}
			else
				return false;
		}
	}
	return true;
}
static void zero_error_tile_map(int32_t x){
	//this is a long string I do not want it stored more than once
	fl_alert("Please enter value greater than zero you on the other hand entered %d",x);
}
bool tileMap::loadFromFile(){
//start by loading the file
	/*Only will return false when there is a malloc error or file error
	the file saving user cancalation and not entering the number correctly return true*/
	uint32_t file_size;
	if (load_file_generic("Load tile map data") == true){
		uint8_t compression=fl_choice("What kind of compression is this tilemap?","Uncompressed","Enigma Compressed",0);
		//get width and height
		std::string tilemap_file=the_file;
		int32_t w,h;
		char str[16];
		char * str_ptr;
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Width");
		if (str_ptr == 0)
			return true;
		if (verify_str_number_only(str_ptr) == false)
			return true;
		w=atoi(str_ptr);
		if (w <= 0){
			zero_error_tile_map(w);
			return true;
		}
		if (currentProject->gameSystem == NES && (w & 1)){
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height to be a multiple to 2");
			return true;
		}
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter Height");
		if (str_ptr == 0)
			return true;
		if (verify_str_number_only(str_ptr) == false)
			return true;
		h=atoi(str_ptr);
		if (h <= 0){
			zero_error_tile_map(h);
			return true;
		}
		if (currentProject->gameSystem == NES && (h & 1)){
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height the be a multiple to 2");
			return true;
		}
		//we can now load the map
		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter offset\nIf the tile map is generated by this program enter zero\nIf not then the first tile may not be zero\nLets say that the first tile is 200 you would enter -200 that makes the first tile zero\nIf you want the first tile to have an offset enter a positive number\nFor example if the tilemap has the first tile set to zero and you enter 5 the first tile will be tile 5");
		if (str_ptr == 0)
			return true;
		if (verify_str_number_only(str_ptr) == false)
			return true;
		int32_t offset=atoi(str_ptr);
		std::ifstream file (tilemap_file.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
		file_size = file.tellg();
		file.seekg (0, std::ios::beg);//return to the beginning of the file
		uint32_t size_temp;
		switch (currentProject->gameSystem){
			case sega_genesis:
				size_temp=(w*h)*2;
			break;
			case NES:
				size_temp=w*h;
			break;
		}
		std::ostringstream outDecomp;
		if (compression==1){
			enigma decomp;
			std::stringstream iss;
			iss << file.rdbuf();
			decomp.decode(iss,outDecomp);
			file_size=outDecomp.str().length();
			printf("Decompressed to %d bytes\n",file_size);
		}
		if (size_temp > file_size)
			fl_alert("Warning: The Tile map that you are attempting to load is smaller than a tile map that has the width and height that you specified\nThis missing data will be padded with tile zero");
		//start converting to tile
		//free(tile_map);
		tileMapDat = (uint8_t *) realloc(tileMapDat,(w*h)*4);
		uint8_t * tempMap = (uint8_t *) malloc(size_temp);
		if (unlikely(tileMapDat == 0))
			show_malloc_error(size_temp)
		if (compression==1){
			std::string output=outDecomp.str();
			output.copy((char *)tempMap, file_size);
		}else if (compression==0)
			file.read ((char *)tempMap, size_temp);
		file.close();
		window->map_w->value(w);
		window->map_h->value(h);
		mapSizeW=w;
		mapSizeH=h;
		uint16_t x,y;
		switch (currentProject->gameSystem){
			case sega_genesis:
				for (y=0;y<h;y++){
					for (x=0;x<w;x++){
						if (((x+(y*w)+1)*2) <= file_size){
							uint16_t temp=*tempMap++;
							//set attributes
							tileMapDat[((y*mapSizeW)+x)*4]=(uint8_t)temp&0xF8;
							temp&=7;
							temp<<=8;
							temp|=(uint16_t)*tempMap++;
							if (temp+offset > 0)
								set_tile((int32_t)temp+offset,x,y);
							else
								set_tile(0,x,y);
							printf("Tile value %d at %d %d\n",temp+offset,x,y);
						}else
							set_tile(0,x,y);
					}
				}
			break;
			case NES:
				for (y=0;y<h;++y){
					for (x=0;x<w;++x){
						if ((x+(y*w)+1) <= file_size){
							uint8_t temp=*tempMap++;
							if (temp+offset > 0)
								set_tile((int32_t)temp+offset,x,y);
							else
								set_tile(0,x,y);
							printf("Tile value %d at %d %d\n",temp+offset,x,y);
						}
						else
							set_tile(0,x,y);
					}
				}
				//now load attributes
				if (load_file_generic("Load Attribtues") == true){
					FILE * fp=fopen(the_file.c_str(),"rb");
					fseek(fp, 0L, SEEK_END);
					uint32_t sz=ftell(fp);
					rewind(fp);
					uint8_t* tempbuf=(uint8_t*)alloca(sz);
					fread(tempbuf,1,sz,fp);
					for (y=0;y<h;y+=4){
						for (x=0;x<w;x+=4){
							set_pal_row(x,y,*tempbuf&3);
							set_pal_row(x,y+1,*tempbuf&3);
							set_pal_row(x+1,y,*tempbuf&3);
							set_pal_row(x+1,y+1,*tempbuf&3);
							
							set_pal_row(x+2,y,((*tempbuf)>>2)&3);
							set_pal_row(x+2,y+1,((*tempbuf)>>2)&3);
							set_pal_row(x+3,y,((*tempbuf)>>2)&3);
							set_pal_row(x+3,y+1,((*tempbuf)>>2)&3);

							++tempbuf;
						}
					}
					fclose(fp);
				}
			break;
		}
		tempMap-=file_size;
		free(tempMap);
		window->redraw();
	}
}
void tileMap::sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip){
	uint16_t x,y;
	int32_t temp;
	for (y=0;y<mapSizeH;y++){
		for (x=0;x<mapSizeW;x++){
			temp=get_tile(x,y);
			if (temp == oldTile){
				set_tile(newTile,x,y);
				if (hflip == true)
					set_hflip(x,y,true);
				if (vflip)
					set_vflip(x,y,true);
			}
			else if (temp > oldTile){
				temp--;
				if (temp < 0)
					temp=0;
				set_tile(temp,x,y);
			}
		}
	}
}
void tileMap::set_tile_full(uint32_t tile,uint32_t x,uint32_t y,uint8_t palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio){
	if (mapSizeW < x || mapSizeH < y) {
		fl_alert("Error tried to set a non existen tile on the map");
		return;
	}
	uint32_t selected_tile=((y*mapSizeW)+x)*4;
	uint8_t flags;
	//uint8_t the_tiles;
	/*
	7  6  5  4  3  2  1 0
	15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
        p  c   c  v  h  n n n n n n n n n n n

    p = Priority flag
    c = Palette select
    v = Vertical flip
    h = Horizontal flip
    n = Pattern name
	*/
	//the exteneded tile maping format is a generic format it goes like this
	//The first byte stores attributes in sega genesis format except with no tile data
	//the next two bytes store the tile number
	//the_tiles=tile&0xFF; //get lower part in litle edian least signficant
	//flags=tile>>8;   //get higher part of int16_t most signifactn in little endain
	flags=0;
	flags|=palette_row<<5;
	flags|=use_hflip<<3;
	flags|=use_vflip<<4;
	flags|=highorlow_prio<<7;
	tileMapDat[selected_tile]=flags;
	//in little endain the least sigficant byte is stored in the lowest address
	tileMapDat[selected_tile+1]=(tile>>16)&255;
	tileMapDat[selected_tile+2]=(tile>>8)&255;
	tileMapDat[selected_tile+3]=tile&255;
}
void tileMap::set_tile(uint32_t tile,uint32_t x,uint32_t y){
	//we must split into two varibles
	if (mapSizeW < x || mapSizeH < y) {
		fl_alert("Error: Tried to set a non existent tile on the tile map");
		return;
	}
	uint32_t selected_tile=((y*mapSizeW)+x)*4;
	tileMapDat[selected_tile+1]=(tile>>16)&255;
	tileMapDat[selected_tile+2]=(tile>>8)&255;
	tileMapDat[selected_tile+3]=tile&255;
}
void tileMap::set_hflip(uint32_t x,uint32_t y,bool hflip_set){
	if (hflip_set)
		tileMapDat[((y*mapSizeW)+x)*4]|= 1 << 3;
	else
		tileMapDat[((y*mapSizeW)+x)*4]&= ~(1 << 3);
}
void tileMap::set_prio(uint32_t x,uint32_t y,bool prio_set){
	if (prio_set)
		tileMapDat[((y*mapSizeW)+x)*4] |= 1 << 7;
	else
		tileMapDat[((y*mapSizeW)+x)*4] &= ~(1 << 7);
}
void tileMap::resize_tile_map(uint32_t new_x,uint32_t new_y){
	//mapSizeW and mapSizeH hold old map size
	if (new_x == mapSizeW && new_y == mapSizeH)
		return;
	//now create a temp buffer to hold the old data
	uint16_t x,y;//needed for loop varibles to copy data
	//uint8_t * temp = new uint8_t [(new_x*new_y)*2];
	uint8_t * temp=0;
	temp=(uint8_t *)malloc((new_x*new_y)*4);
	if (temp == 0){
		//cout << "error" << endl;
		show_malloc_error((new_x*new_y)*4)
		return;
	}
	//now copy old data to temp
	uint32_t sel_map;
	for (y=0;y<new_y;y++){
		for (x=0;x<new_x;x++){
			sel_map=((y*new_x)+x)*4;
			if (x < mapSizeW && y < mapSizeH){
				uint32_t sel_map_old=((y*mapSizeW)+x)*4;
				/*temp[sel_map]=tileMapDat[sel_map_old];
				temp[sel_map+1]=tileMapDat[sel_map_old+1];
				temp[sel_map+2]=tileMapDat[sel_map_old+2];
				temp[sel_map+3]=tileMapDat[sel_map_old+3];*/
				memcpy(&temp[sel_map],&tileMapDat[sel_map_old],4);
			}else{
				/*temp[sel_map]=0;
				temp[sel_map+1]=0;
				temp[sel_map+2]=0;
				temp[sel_map+3]=0;*/
				memset(&temp[sel_map],0,4);
			}
		}
	}
	tileMapDat = (uint8_t *)realloc(tileMapDat,(new_x*new_y)*4);
	if (tileMapDat == 0){
		show_realloc_error((new_x*new_y)*4)
		return;
	}
	/*for (uint32_t c=0;c<(new_x*new_y)*4;c++)
	{
		tileMapDat[c]=temp[c];
	}*/
	memcpy(tileMapDat,temp,(new_x*new_y)*4);
	free(temp);
	mapSizeW=new_x;
	//calulate new scroll size
	uint16_t old_scroll=window->map_x_scroll->value();
	uint8_t tile_size_placer=window->place_tile_size->value();
	int32_t map_scroll=((tile_size_placer*8)*mapSizeW)-map_off_x;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
		map_scroll=0;
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
		old_scroll=map_scroll;
	window->map_x_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));//the reason for adding map_scroll/2 to map_scroll is because without it the user will not be able to scroll the tilemap all the way
	mapSizeH=new_y;
	old_scroll=window->map_y_scroll->value();
	tile_size_placer=window->place_tile_size->value();
	map_scroll=((tile_size_placer*8)*mapSizeH)-map_off_y;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
		map_scroll=0;
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
		old_scroll=map_scroll;
	window->map_y_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));
}
