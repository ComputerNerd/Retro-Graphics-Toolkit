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
#include "project.h"
#include "callback_tilemap.h"
#include "filemisc.h"
#include "classtilemap.h"
#include "compressionWrapper.h"
#include "quant.h"
#include "dither.h"
tileMap::tileMap(){
	amt=1;
	mapSizeHA=mapSizeW=mapSizeH=2;
	isBlock=false;
	tileMapDat=(uint8_t *)calloc(4,TileMapSizePerEntry);
	offset=0;
}
tileMap::tileMap(uint32_t w,uint32_t h){
	amt=1;
	mapSizeW=w;
	mapSizeHA=mapSizeH=h;
	isBlock=false;
	tileMapDat=(uint8_t*)malloc(TileMapSizePerEntry*w*h);
	offset=0;
}
tileMap::tileMap(const tileMap& other){
	mapSizeW=other.mapSizeW;
	mapSizeH=other.mapSizeH;
	mapSizeHA=other.mapSizeHA;
	isBlock=other.isBlock;
	offset=other.offset;
	if(isBlock){
		amt=other.amt;
		mapSizeHA=mapSizeH*amt;
		tileMapDat=(uint8_t*)malloc(mapSizeW*mapSizeHA*TileMapSizePerEntry);
		memcpy(tileMapDat,other.tileMapDat,mapSizeW*mapSizeHA*TileMapSizePerEntry);
		printf("Copied map of size %d\n",mapSizeW*mapSizeHA*TileMapSizePerEntry);
	}else{
		amt=1;
		tileMapDat=(uint8_t*)malloc(mapSizeW*mapSizeH*TileMapSizePerEntry);
		memcpy(tileMapDat,other.tileMapDat,mapSizeW*mapSizeH*TileMapSizePerEntry);
		printf("Copied map of size %d\n",mapSizeW*mapSizeH*TileMapSizePerEntry);
	}
}
tileMap::~tileMap(){
	free(tileMapDat);
}

void tileMap::ditherAsImage(bool entire){
	uint8_t*image;
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW;
	h=currentProject->tileMapC->mapSizeHA;
	w*=currentProject->tileC->sizew;
	h*=currentProject->tileC->sizeh;
	image = (uint8_t *)malloc(w*h*4);
	if(!image)
		show_malloc_error(w*h*4)
	if(entire){
		currentProject->tileMapC->truecolor_to_image(image,-1);
		ditherImage(image,w,h,true,true);
		ditherImage(image,w,h,true,false);
		currentProject->tileMapC->truecolorimageToTiles(image,-1);
	}else{
		for(unsigned row=0;row<4;++row){
			printf("Row: %u\n",row);
			currentProject->tileMapC->truecolor_to_image(image,row);
			ditherImage(image,w,h,true,true);
			ditherImage(image,w,h,true,false);
			//convert back to tiles
			currentProject->tileMapC->truecolorimageToTiles(image,row);
		}
	}
	free(image);
}
void tileMap::allRowSet(unsigned row){
	uint32_t x,y;
	for (y=0;y<mapSizeHA;++y){
		for (x=0;x<mapSizeW;++x)
			set_pal_row(x,y,row);
	}
}
static void sumTile(uint8_t*tilePtr,uint32_t*sums){
	uint32_t sum[3];//In hopes that the compiler is smart enough to keep these in registers
	memset(sum,0,sizeof(sum));
	for(unsigned j=0;j<256;++j){
		if(tilePtr[3]){
			sum[0]+=tilePtr[0];
			sum[1]+=tilePtr[1];
			sum[2]+=tilePtr[2];
		}
		tilePtr+=4;
	}
	sums[0]=sum[0];
	sums[1]=sum[1];
	sums[2]=sum[2];
}
bool tileMap::pickTileRowQuantChoice(unsigned rows){
	unsigned w=mapSizeW,h=mapSizeHA;
	unsigned char userpal[3][256];
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
		w/=2;
		h/=2;
	}
	uint8_t*imgin=(uint8_t*)malloc(w*h*3);
	uint8_t*imgout=(uint8_t*)malloc(w*h);
	uint32_t sums[3];
	uint8_t*imgptr=imgin;
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
		for(unsigned y=0;y<mapSizeHA;y+=2){
			for(unsigned x=0;x<mapSizeW;x+=2){
				memset(sums,0,sizeof(sums));
				for(unsigned i=0;i<4;++i){
					uint32_t sumtmp[3];
					sumTile(currentProject->tileC->truetDat.data()+(get_tile(x+(i&1),y+(i/2))*currentProject->tileC->tcSize),sumtmp);
					sums[0]+=sumtmp[0];
					sums[1]+=sumtmp[1];
					sums[2]+=sumtmp[2];
				}
				*imgptr++=sums[0]/256;
				*imgptr++=sums[1]/256;
				*imgptr++=sums[2]/256;
			}
		}
	}else{
		for(unsigned y=0;y<mapSizeHA;++y){
			for(unsigned x=0;x<mapSizeW;++x){
				sumTile(currentProject->tileC->truetDat.data()+(get_tile(x,y)*currentProject->tileC->tcSize),sums);
				*imgptr++=sums[0]/64;
				*imgptr++=sums[1]/64;
				*imgptr++=sums[2]/64;
			}
		}
	}
	dl3quant(imgin,w,h,rows,userpal,false,0);
	dl3floste(imgin,imgout,w,h,rows,0,userpal);
	imgptr=imgout;
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
		for(unsigned y=0;y<mapSizeHA;y+=2){
			for(unsigned x=0;x<mapSizeW;x+=2){
				set_pal_row(x,y,(*imgptr)%rows);
				set_pal_row(x+1,y,(*imgptr)%rows);
				set_pal_row(x,y+1,(*imgptr)%rows);
				set_pal_row(x+1,y+1,(*imgptr++)%rows);
			}
		}
	}else{
		for(unsigned y=0;y<mapSizeHA;++y){
			for(unsigned x=0;x<mapSizeW;++x)
				set_pal_row(x,y,(*imgptr++)%rows);
		}
	}
	free(imgin);
	free(imgout);
}
bool tileMap::inRange(uint32_t x,uint32_t y){
	if (mapSizeW < x || mapSizeHA < y){
		printf("Out of range %u %u\n",x,y);
		return false;
	}else
		return true;
}
void tileMap::setRaw(uint32_t x,uint32_t y,uint32_t val){
	uint32_t*tptr=(uint32_t*)tileMapDat;
	tptr+=(y*mapSizeW)+x;
	*tptr=val;
}
uint32_t tileMap::getRaw(uint32_t x,uint32_t y){
	uint32_t*tptr=(uint32_t*)tileMapDat;
	tptr+=(y*mapSizeW)+x;
	return*tptr;
}
void tileMap::resizeBlocks(uint32_t wn,uint32_t hn){
	uint32_t amtTemp=mapSizeW*mapSizeH*amt;
	amtTemp/=(wn*hn);
	if(amtTemp){
		amt=amtTemp;
		mapSizeW=wn;
		mapSizeH=hn;
		mapSizeHA=mapSizeH*amt;
		char tmp[16];
		snprintf(tmp,16,"%u",amt);
		window->map_amt->value(tmp);
	}else{
		window->updateMapWH();
	}
	ScrollUpdate();
}
void tileMap::blockAmt(uint32_t newAmt){
	if(newAmt==amt)
		return;
	tileMapDat=(uint8_t*)realloc(tileMapDat,TileMapSizePerEntry*mapSizeW*mapSizeH*newAmt);
	if(newAmt>amt)
		memset(tileMapDat+(amt*TileMapSizePerEntry*mapSizeW*mapSizeH),0,(newAmt-amt)*mapSizeW*mapSizeH*TileMapSizePerEntry);
	amt=newAmt;
	mapSizeHA=mapSizeH*amt;
	ScrollUpdate();
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
				for(uint_fast32_t y=0;y<mapSizeH;y+=h){
					for(uint_fast32_t x=0;x<mapSizeW;x+=w){
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
			mapSizeH*=amt;
			amt=1;//amt must = 1 when blocks are not in use
			mapSizeHA=mapSizeH;
		}
	}
	window->updateMapWH();
	if(set){
		window->map_w->label("Block width");
		window->map_w->callback(resizeBlocksCB);
		window->map_h->label("Block height");
		window->map_h->callback(resizeBlocksCB);
		window->map_amt->show();
		char tmp[16];
		snprintf(tmp,16,"%u",amt);
		window->map_amt->value(tmp);
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
	if(inRange(x,y))
		return (tileMapDat[((y*mapSizeW)+x)*4]>>3)&1;
	else
		return false;
}
bool tileMap::get_vflip(uint32_t x,uint32_t y){
	if(inRange(x,y))
		return (tileMapDat[((y*mapSizeW)+x)*4]>>4)&1;
	else
		return false;
}
bool tileMap::get_prio(uint32_t x,uint32_t y){
	if(inRange(x,y))
		return (tileMapDat[((y*mapSizeW)+x)*4]>>7)&1;
	else
		return false;
}
uint8_t tileMap::get_palette_map(uint32_t x,uint32_t y){
	if(inRange(x,y)){
			if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
				x&=~1;
				y&=~1;
			}
			return (tileMapDat[((y*mapSizeW)+x)*4]>>5)&3;
	}else
		return 0;
}
void tileMap::set_pal_row(uint32_t x,uint32_t y,uint8_t row){
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
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
	uint32_t x,y;
	FILE * myfile;
	size_t fileSize;
	int compression;
	uint8_t* mapptr;
	fileType_t type=askSaveType();
	int clipboard;
	if(type){
		clipboard=clipboardAsk();
		if(clipboard==2)
			return true;
	}else
		clipboard=0;
	bool pickedFile;
	if(clipboard)
		pickedFile=true;
	else
		pickedFile=load_file_generic("Save tilemap to",true);
	if(pickedFile){
		compression=compressionAsk();
		if(compression<0)
			return true;
		if(clipboard)
			myfile=0;
		else if(type)
			myfile = fopen(the_file.c_str(),"w");
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (likely(myfile||clipboard)){
			switch (currentProject->gameSystem){
				case sega_genesis:
					{uint16_t * TheMap;
					fileSize=(mapSizeW*mapSizeH*amt)*2;
					TheMap = (uint16_t*)malloc(fileSize);
					for (y=0;y<mapSizeH*amt;++y){
						for (x=0;x<mapSizeW;++x){
							int tile=get_tile(x,y);
							tile+=offset;
							if(tile<0)
								tile=0;
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
					mapptr=(uint8_t*)TheMap;}
				break;
				case NES:
					{uint8_t * TheMap;
					fileSize=mapSizeW*mapSizeH*amt;
					TheMap = (uint8_t *)malloc(fileSize);
					for (y=0;y<mapSizeH*amt;++y){
						for (x=0;x<mapSizeW;++x){
							int tile=get_tile(x,y);
							tile+=offset;
							if(tile<0)
								tile=0;
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
			if(compression){
				void*TheMap=mapptr;
				mapptr=(uint8_t*)encodeType(TheMap,fileSize,fileSize,compression);
				free(TheMap);
			}
			if(type){
				char temp[2048];
				snprintf(temp,2048,"Width %d Height %d %s",mapSizeW,mapSizeH*amt,typeToText(compression));
				int bits;
				if((currentProject->gameSystem==sega_genesis)&&(!compression))
					bits=16;
				else
					bits=8;
				if(!saveBinAsText(mapptr,fileSize,myfile,type,temp,"mapDat",bits)){
					free(mapptr);
					return false;
				}
			}else
				fwrite(mapptr,1,fileSize,myfile);
			free(mapptr);
			if(myfile)
				fclose(myfile);
			puts("File Saved");
		}else
			return false;
	}
	if (currentProject->gameSystem == NES){
		if(clipboard)
			fl_alert("Copy the data to clipboard and press okay after this tilemap attributes will be copied to clipboard");
		else
			pickedFile=load_file_generic("Save attributes to",true);
		if(pickedFile){
			if(clipboard)
				myfile=0;
			else if(type)
				myfile = fopen(the_file.c_str(),"w");
			else
				myfile = fopen(the_file.c_str(),"wb");
			if (likely(myfile||clipboard)) {
				uint8_t * AttrMap = (uint8_t *)malloc(((mapSizeW+2)/4)*((mapSizeHA+2)/4));
				uint8_t * freeAttrMap=AttrMap;
				for (y=0;y<mapSizeHA;y+=4){
					for (x=0;x<mapSizeW;x+=4){
						*AttrMap++ = get_palette_map(x,y) | (get_palette_map(x+2,y)<<2) | (get_palette_map(x,y+2) << 4) | (get_palette_map(x+2,y+2) << 6);
					}
				}
				if(type){
					if(saveBinAsText(freeAttrMap,((mapSizeW+2)/4)*((mapSizeHA+2)/4),myfile,type,0,"mapDatAttr",8)==false)
						return false;
				}else
					fwrite(freeAttrMap,1,((mapSizeW+2)/4)*((mapSizeHA+2)/4),myfile);		
				free(freeAttrMap);
				if(myfile)
					fclose(myfile);
				puts("File Saved");
			}else
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
	if (load_file_generic("Load tile map data")){
		int compression=compressionAsk();
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
		if (currentProject->gameSystem == NES && (w & 1)&&(currentProject->subSystem&NES2x2)){
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
		if (currentProject->gameSystem == NES && (h & 1)&&(currentProject->subSystem&NES2x2)){
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height the be a multiple to 2");
			return true;
		}
		//we can now load the map
		int32_t offset;
		str_ptr=(char *)fl_input("Enter offset\nThis number will be subtracted from the tilemap's tile value can be positive or negative");
		if(!str_ptr)
			return true;
		if (!verify_str_number_only(str_ptr))
			return true;
		offset=atoi(str_ptr);
		window->tmapOffset->value(str_ptr);
		currentProject->tileMapC->offset=offset;
		window->BlocksCBtn->value(blocksLoad?1:0);
		FILE*fp;
		if(!compression){
			fp=fopen(tilemap_file.c_str(),"rb");
			fseek(fp,0,SEEK_END);
			file_size = ftell(fp);
			rewind(fp);
		}
		size_t size_temp;
		std::string output;
		if(compression)
			output=decodeTypeStr(tilemap_file.c_str(),file_size,compression);
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
		window->updateMapWH();
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
		tileMapDat = (uint8_t *)realloc(tileMapDat,(w*h)*4*amt);
		uint8_t * tempMap = (uint8_t *) malloc(size_temp);
		if (unlikely(!tileMapDat))
			show_malloc_error(size_temp)
		if (compression)
			output.copy((char *)tempMap, file_size);
		else if (!compression){
			fread((char *)tempMap,size_temp,1,fp);
			fclose(fp);
		}
		uint32_t x,y;
		switch (currentProject->gameSystem){
			case sega_genesis:
				for (y=0;y<mapSizeH*amt;++y){
					for (x=0;x<mapSizeW;++x){
						if (((x+(y*w)+1)*2) <= file_size){
							int temp=*tempMap++;
							//set attributes
							tileMapDat[((y*mapSizeW)+x)*4]=(uint8_t)temp&0xF8;
							temp&=7;
							temp<<=8;
							temp|=*tempMap++;
							if (temp-offset > 0)
								set_tile((int32_t)temp-offset,x,y);
							else
								set_tile(0,x,y);
						}else
							set_tile(0,x,y);
					}
				}
			break;
			case NES:
				for (y=0;y<mapSizeH*amt;++y){
					for (x=0;x<mapSizeW;++x){
						if ((x+(y*w)+1) <= file_size){
							int temp=*tempMap++;
							if (temp-offset > 0)
								set_tile((int32_t)temp-offset,x,y);
							else
								set_tile(0,x,y);
						}else
							set_tile(0,x,y);
					}
				}
				//now load attributes
				if (load_file_generic("Load Attribtues")){
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
	return true;
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
	if (mapSizeW < x || (mapSizeHA) < y) {
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
bool tileMap::truecolor_to_image(uint8_t * the_image,int8_t useRow,bool useAlpha){
	/*!
	the_image pointer to image must be able to hold the image using rgba 32bit or rgb 24bit if not using alpha
	useRow what row to use or -1 for no row
	*/
	if (the_image == 0){
		fl_alert("Error malloc must be called before generating this image");
		return false;
	}
	uint32_t w,h;
	w=mapSizeW*currentProject->tileC->sizew;
	h=mapSizeHA*currentProject->tileC->sizeh;
	uint16_t x_tile=0,y_tile=0;
	uint32_t truecolor_tile_ptr=0;
	uint8_t pixelSize,pSize2;
	if (useAlpha){
		pixelSize=4;
		pSize2=32;
	}else{
		pixelSize=3;
		pSize2=24;
	}
	if (useRow != -1){
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8){//a tiles y
			for (uint_fast32_t b=0;b<w*pixelSize;b+=pSize2){//b tiles x
				truecolor_tile_ptr=get_tileRow(x_tile,y_tile,useRow)*256;
				if((truecolor_tile_ptr != -256)&&(truecolor_tile_ptr<(currentProject->tileC->amt*256))){
					for (uint_fast32_t y=0;y<w*pSize2;y+=w*pixelSize){//pixels y
						if (useAlpha)
							memcpy(&the_image[a+b+y],&currentProject->tileC->truetDat[truecolor_tile_ptr],32);
						else{
							unsigned xx=0;
							for (unsigned x=0;x<32;x+=4){//pixels x
								the_image[a+b+y+xx]=currentProject->tileC->truetDat[truecolor_tile_ptr+x];
								the_image[a+b+y+xx+1]=currentProject->tileC->truetDat[truecolor_tile_ptr+x+1];
								the_image[a+b+y+xx+2]=currentProject->tileC->truetDat[truecolor_tile_ptr+x+2];
								xx+=3;
							}
						}
						truecolor_tile_ptr+=32;
					}
				}else{
					for (uint32_t y=0;y<w*pSize2;y+=w*pixelSize)//pixels y
						memset(&the_image[a+b+y],0,pSize2);
				}
				++x_tile;
			}
			x_tile=0;
			++y_tile;
		}
	}else{
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8){//a tiles y
			for (uint32_t b=0;b<w*pixelSize;b+=pSize2){//b tiles x
				truecolor_tile_ptr=get_tile(x_tile,y_tile)*256;
				if(truecolor_tile_ptr<(currentProject->tileC->amt*256)){
					for (uint32_t y=0;y<w*pixelSize*8;y+=w*pixelSize){//pixels y
						if (useAlpha)
							memcpy(&the_image[a+b+y],&currentProject->tileC->truetDat[truecolor_tile_ptr],32);
						else{
							unsigned xx=0;
							for (unsigned x=0;x<32;x+=4){//pixels x
								the_image[a+b+y+xx]=currentProject->tileC->truetDat[truecolor_tile_ptr+x];
								the_image[a+b+y+xx+1]=currentProject->tileC->truetDat[truecolor_tile_ptr+x+1];
								the_image[a+b+y+xx+2]=currentProject->tileC->truetDat[truecolor_tile_ptr+x+2];
								xx+=3;
							}
						}
						truecolor_tile_ptr+=32;
					}
				}else{
					for (uint32_t y=0;y<w*pSize2;y+=w*pixelSize)//pixels y
						memset(&the_image[a+b+y],0,pSize2);
				}
				++x_tile;
			}
			x_tile=0;
			++y_tile;
		}
	}
	return true;
}
void tileMap::truecolorimageToTiles(uint8_t * image,int rowusage,bool useAlpha){
	uint8_t type_temp=palTypeGen;
	uint8_t tempSet=0;
	uint8_t truecolor_tile[256];
	uint_fast32_t x_tile=0;
	uint_fast32_t y_tile=0;
	uint_fast8_t pSize=useAlpha ? 4:3;
	uint_fast8_t pTile=useAlpha ? 32:24;
	uint32_t w=mapSizeW*8;
	uint32_t h=mapSizeHA*8;
	uint_fast32_t truecolor_tile_ptr;
	for (uint_fast32_t a=0;a<(h*w*pSize)-w*pSize;a+=w*pSize*8){//a tiles y
		for (uint_fast32_t b=0;b<w*pSize;b+=pTile){//b tiles x
			int32_t current_tile;
			if(rowusage==-1){
				current_tile=get_tile(x_tile,y_tile);
				if(current_tile>=currentProject->tileC->amt)
					goto dont_convert_tile;
			}else{
				current_tile=get_tileRow(x_tile,y_tile,rowusage);
				if(current_tile>=currentProject->tileC->amt)
					goto dont_convert_tile;
				if (current_tile == -1)
					goto dont_convert_tile;
			}
			truecolor_tile_ptr=0;
			for (uint32_t y=0;y<w*pSize*8;y+=w*pSize)//pixels y
			{
				if(useAlpha){
					memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y],32);
					truecolor_tile_ptr+=32;
				}else{
					for(uint8_t xx=0;xx<24;xx+=3){
						memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y+xx],3);
						truecolor_tile_ptr+=3;
						truecolor_tile[truecolor_tile_ptr]=255;
						++truecolor_tile_ptr;
					}
				}
			}
			//convert back to tile
			if ((type_temp != 0) && (currentProject->gameSystem == sega_genesis)){
				tempSet=(get_prio(x_tile,y_tile)^1)*8;
				set_palette_type_force(tempSet);
			}
			currentProject->tileC->truecolor_to_tile_ptr(get_palette_map(x_tile,y_tile),current_tile,truecolor_tile,false,false);
dont_convert_tile:
		++x_tile;
		}
	x_tile=0;
	++y_tile;
	}
	if(currentProject->gameSystem==sega_genesis)
		set_palette_type();
}
