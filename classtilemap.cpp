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
#include "callback_tilemap.h"
#include "kens.h"
tileMap::tileMap(){
	amt=1;
	mapSizeHA=mapSizeW=mapSizeH=2;
	isBlock=false;
	tileMapDat=(uint8_t *)calloc(16,1);
}
tileMap::tileMap(const tileMap& other){
	mapSizeW=other.mapSizeW;
	mapSizeH=other.mapSizeH;
	mapSizeHA=other.mapSizeHA;
	isBlock=other.isBlock;
	if(isBlock){
		amt=other.amt;
		tileMapDat=(uint8_t*)malloc(mapSizeW*mapSizeH*4*amt);
		memcpy(tileMapDat,other.tileMapDat,mapSizeW*mapSizeH*4*amt);
		printf("Copied map of size %d\n",mapSizeW*mapSizeH*4*amt);
	}else{
		tileMapDat=(uint8_t*)malloc(mapSizeW*mapSizeH*4);
		memcpy(tileMapDat,other.tileMapDat,mapSizeW*mapSizeH*4);
		printf("Copied map of size %d\n",mapSizeW*mapSizeH*4);
	}
}
tileMap::~tileMap(){
	free(tileMapDat);
}
void tileMap::resizeBlocks(uint32_t wn,uint32_t hn){
	uint32_t amtTemp=mapSizeW*mapSizeH*amt;
	amtTemp/=(wn*hn);
	if(amtTemp){
		amt=amtTemp;
		mapSizeW=wn;
		mapSizeH=hn;
		mapSizeHA=mapSizeH*amt;
		window->map_amt->value(amt);
	}else{
		window->map_w->value(mapSizeW);
		window->map_h->value(mapSizeH);
	}
	ScrollUpdate();
}
void tileMap::blockAmt(uint32_t newAmt){
	amt=newAmt;
	resize_tile_map(mapSizeW,mapSizeH*amt);
	mapSizeHA=mapSizeH*amt;
}
const char * MapWidthTxt="Map width";
const char * MapHeightTxt="Map height";
static void rect2rect(uint8_t*in,uint8_t*out,unsigned xin,unsigned yin,unsigned win,unsigned wout,unsigned hout){
	in+=(yin*win)+xin;
	while(hout--){
		memcpy(out,in,wout);
		in+=win;
		out+=wout;
	}
}
void tileMap::toggleBlocks(bool set){
	if(set!=isBlock){
		if(set){
			//First get user input on the demensions of each block
			char * str_ptr;
			str_ptr=(char *)fl_input("Enter block width");
			if (!str_ptr)
				return;
			if (!verify_str_number_only(str_ptr))
				return;
			uint32_t w,h;
			w=atoi(str_ptr);
			if(mapSizeW%w){
				fl_alert("You must enter a number that will result in an integer when divding by %d",mapSizeW);
				return;
			}
			str_ptr=(char *)fl_input("Enter block height");
			if (!str_ptr)
				return;
			if (!verify_str_number_only(str_ptr))
				return;
			h=atoi(str_ptr);
			if(mapSizeH%h){
				fl_alert("You must enter a number that will result in an integer when divding by %d",mapSizeH);
				return;
			}
			if((mapSizeW!=w)||(mapSizeH!=h)){
				//The tiles will need to be rearagned
				uint8_t*tmp=(uint8_t*)malloc(mapSizeW*mapSizeH*TileMapSizePerEntry);
				uint8_t*out=tmp;
				printf("w: %d h: %d %d %d\n",w,h,mapSizeW,mapSizeH);
				for(uint_fast32_t y=0;y<mapSizeH;y+=h){
					for(uint_fast32_t x=0;x<mapSizeW;x+=w){
						printf("%d %d\n",x,y);
						rect2rect(tileMapDat,out,x*TileMapSizePerEntry,y,mapSizeW*TileMapSizePerEntry,w*TileMapSizePerEntry,h);
						out+=w*h*TileMapSizePerEntry;
					}
				}
				memcpy(tileMapDat,tmp,mapSizeH*mapSizeW*TileMapSizePerEntry);
				free(tmp);
			}
			isBlock=true;
			amt=(mapSizeW*mapSizeH)/(w*h);
			mapSizeW=w;
			mapSizeH=h;
			mapSizeHA=mapSizeH*amt;
		}else{
			isBlock=false;
			amt=1;//amt must = 1 when blocks are not in use
			mapSizeH*=amt;
			mapSizeHA=mapSizeH;
		}
	}
	window->map_w->value(mapSizeW);
	window->map_h->value(mapSizeH);
	if(set){
		window->map_w->label("Block width");
		window->map_w->callback(resizeBlocksCB);
		window->map_h->label("Block height");
		window->map_h->callback(resizeBlocksCB);
		window->map_amt->show();
		window->map_amt->value(amt);
	}else{
		window->map_w->callback(callback_resize_map);
		window->map_w->label(MapWidthTxt);
		window->map_h->callback(callback_resize_map);
		window->map_h->label(MapHeightTxt);
		window->map_amt->hide();
	}
	updateTileSelectAmt();
	ScrollUpdate();
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
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem==NES2x2)){
		x&=~1;
		y&=~1;
	}
	return (tileMapDat[((y*mapSizeW)+x)*4]>>5)&3;
}
void tileMap::set_pal_row(uint32_t x,uint32_t y,uint8_t row){
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem==NES2x2)){
		//printf("%d %d %d %d\n",x&(~1),x|1,y&(~1),y|1);
		tileMapDat[((y*mapSizeW)+(x&(~1)))*4]&= ~(3 << 5);
		tileMapDat[((y*mapSizeW)+(x&(~1)))*4]|=row<<5;
		tileMapDat[((y*mapSizeW)+(x|1))*4]&= ~(3 << 5);
		tileMapDat[((y*mapSizeW)+(x|1))*4]|=row<<5;
		tileMapDat[(((y&(~1))*mapSizeW)+(x&(~1)))*4]&= ~(3 << 5);
		tileMapDat[(((y&(~1))*mapSizeW)+(x&(~1)))*4]|=row<<5;
		tileMapDat[(((y&(~1))*mapSizeW)+(x|1))*4]&= ~(3 << 5);
		tileMapDat[(((y&(~1))*mapSizeW)+(x|1))*4]|=row<<5;;
		tileMapDat[(((y|1)*mapSizeW)+(x&(~1)))*4]&= ~(3 << 5);
		tileMapDat[(((y|1)*mapSizeW)+(x&(~1)))*4]|=row<<5;;
		tileMapDat[(((y|1)*mapSizeW)+(x|1))*4]&= ~(3 << 5);
		tileMapDat[(((y|1)*mapSizeW)+(x|1))*4]|=row<<5;;
	}else{
		tileMapDat[((y*mapSizeW)+x)*4]&= ~(3 << 5);
		tileMapDat[((y*mapSizeW)+x)*4]|=row<<5;
	}
}
uint32_t tileMap::get_tile(uint32_t x,uint32_t y){
	//first calulate which tile we want
	if ((mapSizeW <= x) || (mapSizeHA) <= y){
		printf("Error tile (%d,%d) does not exist on this map\n",x,y);
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
	if (load_file_generic("Save tilemap to",true)){
		type=fl_choice("How would like this file saved?","Binary","C header",0);
		compression=fl_choice("In what format would you like this tilemap saved","Uncompressed","Enigma Compression",0);
		if (type == 1){
			char temp[2048];
			myfile = fopen(the_file.c_str(),"w");
			sprintf(temp,"//Width %d Height %d",mapSizeW,mapSizeH*amt);
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
					fileSize=(mapSizeW*mapSizeH*amt)*2;
					TheMap = (uint16_t*)malloc(fileSize);
					for (y=0;y<mapSizeH*amt;++y){
						for (x=0;x<mapSizeW;++x){
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
					TheMap-=mapSizeW*mapSizeH*amt;//return to begining so it can be freeded and the file saved
					mapptr=(uint8_t*)TheMap;
				}//brackets used to prevent TheMap conflict
				break;
				case NES:
				{uint8_t * TheMap;
					fileSize=mapSizeW*mapSizeH*amt;
					TheMap = (uint8_t *)malloc(fileSize);
					for (y=0;y<mapSizeH*amt;++y){
						for (x=0;x<mapSizeW;++x){
							uint32_t tile=get_tile(x,y);
							if (tile > 255) {
								printf("Warning tile value %d exceeded 255 at x: %d y: %d\n",tile,x,y);
								tile=255;
							}
							*TheMap++=tile;
						}
					}
					TheMap-=mapSizeW*mapSizeH*amt;//return to begining so it can be freeded and the file sized
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
				uint8_t * AttrMap = (uint8_t *)malloc((mapSizeW/4)*(mapSizeH*amt/4));
				uint8_t * freeAttrMap=AttrMap;
				for (y=0;y<mapSizeH*amt;y+=4){
					for (x=0;x<mapSizeW;x+=4){
						*AttrMap++ = get_palette_map(x,y) | (get_palette_map(x+2,y)<<2) | (get_palette_map(x,y+2) << 4) | (get_palette_map(x+2,y+2) << 6);
						printf("x: %d y: %d\n",x,y);
					}
				}
				//AttrMap-=(mapSizeW/4)*(mapSizeH/4);
				printf("%d %d\n",AttrMap,freeAttrMap);
				if (type == 1){
					if (saveBinAsText(freeAttrMap,(mapSizeW/4)*(mapSizeH*amt/4),myfile)==false)
							return false;
						fputs("};",myfile);
				}
				else
					fwrite(freeAttrMap,1,(mapSizeW/4)*(mapSizeH*amt/4),myfile);		
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
	size_t file_size;
	if (load_file_generic("Load tile map data") == true){
		uint8_t compression=fl_choice("What kind of compression is this tilemap?","Uncompressed","Enigma Compressed",0);
		//get width and height
		int blocksLoad=fl_ask("Are you loading blocks?");
		std::string tilemap_file=the_file;
		int32_t w,h;
		char * str_ptr;
		if(blocksLoad)
			str_ptr=(char *)fl_input("Enter block width");
		else
			str_ptr=(char *)fl_input("Enter width");
		if (!str_ptr)
			return true;
		if (!verify_str_number_only(str_ptr))
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
		if(blocksLoad)
			str_ptr=(char *)fl_input("Enter block height");
		else
			str_ptr=(char *)fl_input("Enter height");
		if (!str_ptr)
			return true;
		if (!verify_str_number_only(str_ptr))
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
		int32_t offset;
		str_ptr=(char *)fl_input("Enter offset\nIf the tile map is generated by this program enter zero\nIf not then the first tile may not be zero\nLets say that the first tile is 200 you would enter -200 that makes the first tile zero\nIf you want the first tile to have an offset enter a positive number\nFor example if the tilemap has the first tile set to zero and you enter 5 the first tile will be tile 5");
		if(!str_ptr)
			return true;
		if (!verify_str_number_only(str_ptr))
			return true;
		offset=atoi(str_ptr);
		window->BlocksCBtn->value(blocksLoad?1:0);
		std::ifstream file (tilemap_file.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
		file_size = file.tellg();
		file.seekg (0, std::ios::beg);//return to the beginning of the file
		uint32_t size_temp;
		std::ostringstream outDecomp;
		if (compression==1){
			enigma decomp;
			std::stringstream iss;
			iss << file.rdbuf();
			decomp.decode(iss,outDecomp);
			file_size=outDecomp.str().length();
			printf("Decompressed to %d bytes\n",file_size);
		}
		uint32_t blocksLoaded=file_size/w/h;
		switch (currentProject->gameSystem){
			case sega_genesis:
				if(blocksLoad)
					size_temp=blocksLoaded*w*h;
				else
					size_temp=w*h*2;//Size of data that is to be loaded
				blocksLoaded/=2;
			break;
			case NES:
				if(blocksLoad)
					size_temp=blocksLoaded*w*h;
				else
					size_temp=w*h;
			break;
		}
		printf("W %d H %d blocks loaded %d\n",w,h,blocksLoaded);
		window->map_w->value(w);
		window->map_h->value(h);
		mapSizeW=w;
		mapSizeH=h;
		if(blocksLoad)
			amt=blocksLoaded;
		else
			amt=1;
		mapSizeHA=mapSizeH*amt;
		if(size_temp>file_size)
			fl_alert("Warning: The Tile map that you are attempting to load is smaller than a tile map that has the width and height that you specified\nThis missing data will be padded with tile zero");
		//start converting to tile
		//free(tile_map);
		tileMapDat = (uint8_t *) realloc(tileMapDat,(w*h)*4*amt);
		uint8_t * tempMap = (uint8_t *) malloc(size_temp);
		if (unlikely(!tileMapDat))
			show_malloc_error(size_temp)
		if (compression==1){
			std::string output=outDecomp.str();
			output.copy((char *)tempMap, file_size);
		}else if (compression==0)
			file.read ((char *)tempMap, size_temp);
		file.close();
		uint16_t x,y;
		switch (currentProject->gameSystem){
			case sega_genesis:
				for (y=0;y<mapSizeH*amt;++y){
					for (x=0;x<mapSizeW;++x){
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
							//printf("Tile value %d at %d %d\n",temp+offset,x,y);
						}else
							set_tile(0,x,y);
					}
				}
			break;
			case NES:
				for (y=0;y<mapSizeH*amt;++y){
					for (x=0;x<mapSizeW;++x){
						if ((x+(y*w)+1) <= file_size){
							uint8_t temp=*tempMap++;
							if (temp+offset > 0)
								set_tile((int32_t)temp+offset,x,y);
							else
								set_tile(0,x,y);
							//printf("Tile value %d at %d %d\n",temp+offset,x,y);
						}else
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
					for (y=0;y<mapSizeH*amt;y+=4){
						for (x=0;x<mapSizeW;x+=4){
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
		isBlock=blocksLoad;
		toggleBlocks(blocksLoad);
		window->redraw();
	}
}
void tileMap::sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip){
	uint_fast32_t x,y;
	int_fast32_t temp;
	for (y=0;y<mapSizeHA;y++){
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
	if (mapSizeW < x || (mapSizeH*amt) < y) {
		printf("Error (%d,%d) cannot be set to a tile as it is not on the map",x,y);
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
	if (mapSizeW < x || (mapSizeH*amt) < y) {
		printf("Error (%d,%d) cannot be set to a tile as it is not on the map",x,y);
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
void tileMap::ScrollUpdate(void){
	uint32_t old_scroll=window->map_x_scroll->value();
	uint32_t tile_size_placer=window->place_tile_size->value();
	int32_t map_scroll=mapSizeW-((window->w()-map_off_x)/tile_size_placer/8);
	//printf("x: %d\n",map_scroll);
	if (map_scroll < 0)
		map_scroll=0;
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll){
		old_scroll=map_scroll;
		map_scroll_pos_x=map_scroll;
	}
	if(map_scroll){
		window->map_x_scroll->show();
		window->map_x_scroll->value(old_scroll,1,0,map_scroll+2);
	}else
		window->map_x_scroll->hide();
	old_scroll=window->map_y_scroll->value();
	map_scroll=(mapSizeHA)-((window->h()-map_off_y)/tile_size_placer/8);//size of all offscreen tiles in pixels
	if (map_scroll < 0)
		map_scroll=0;
	if (old_scroll > map_scroll){
		old_scroll=map_scroll;
		map_scroll_pos_y=map_scroll;
	}
	if(map_scroll){
		window->map_y_scroll->show();
		window->map_y_scroll->value(old_scroll,1,0,map_scroll+2);
	}else
		window->map_y_scroll->hide();
}
void tileMap::resize_tile_map(uint32_t new_x,uint32_t new_y){
	//mapSizeW and mapSizeH hold old map size
	if ((new_x==mapSizeW) && (new_y==mapSizeH))
		return;
	if(new_x==0||new_y==0){
		fl_alert("Cannot have a map of size 0");
		return;
	}
	//now create a temp buffer to hold the old data
	uint32_t x,y;//needed for loop varibles to copy data
	//uint8_t * temp = new uint8_t [(new_x*new_y)*2];
	uint8_t * temp=0;
	temp=(uint8_t *)malloc((new_x*new_y)*4);
	if (!temp){
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
				memcpy(&temp[sel_map],&tileMapDat[sel_map_old],4);
			}else{
				memset(&temp[sel_map],0,4);
			}
		}
	}
	tileMapDat = (uint8_t *)realloc(tileMapDat,(new_x*new_y)*4);
	if (!tileMapDat){
		show_realloc_error((new_x*new_y)*4)
		return;
	}
	memcpy(tileMapDat,temp,(new_x*new_y)*4);
	free(temp);
	mapSizeW=new_x;
	mapSizeH=new_y;
	if(selTileE_G[0]>=new_x)
		selTileE_G[0]=new_x-1;
	if(selTileE_G[1]>=new_y)
		selTileE_G[1]=new_y-1;
	mapSizeHA=mapSizeH;
	if(isBlock)
		mapSizeH/=amt;
	ScrollUpdate();
}
