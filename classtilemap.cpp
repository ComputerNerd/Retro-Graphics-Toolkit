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
bool tileMap::get_hflip(uint16_t x,uint16_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>3)&1;
}
bool tileMap::get_vflip(uint16_t x,uint16_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>4)&1;
}
bool tileMap::get_prio(uint16_t x,uint16_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>7)&1;
}
uint8_t tileMap::get_palette_map(uint16_t x,uint16_t y){
	return (tileMapDat[((y*mapSizeW)+x)*4]>>5)&3;
}
uint32_t tileMap::get_tile(uint16_t x,uint16_t y){
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
int32_t tileMap::get_tileRow(uint16_t x,uint16_t y,uint8_t useRow){
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

void tileMap::set_vflip(uint16_t x,uint16_t y,bool vflip_set){
	if (vflip_set)
		tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]|= 1 << 4;
	else
		tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]&= ~(1 << 4);
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
		if (likely(myfile!=0)) {
			switch (game_system) {
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
				string input,output;
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
	if (game_system == NES){
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
		string tilemap_file=the_file;
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
		if (game_system == NES && (w & 1)){
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
		if (game_system == NES && (h & 1)){
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
		ifstream file (tilemap_file.c_str(), ios::in|ios::binary|ios::ate);
		file_size = file.tellg();
		file.seekg (0, ios::beg);//return to the beginning of the file
		uint32_t size_temp;
		switch (game_system){
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
			string output=outDecomp.str();
			output.copy((char *)tempMap, file_size);
		}else if (compression==0)
			file.read ((char *)tempMap, size_temp);
		file.close();
		window->map_w->value(w);
		window->map_h->value(h);
		mapSizeW=w;
		mapSizeH=h;
		uint16_t x,y;
		switch (game_system){
			case sega_genesis:
				for (y=0;y<h;y++) {
					for (x=0;x<w;x++) {
						if (((x+(y*w)+1)*2) <= file_size) {
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
					FILE * fp=fopen(the_file.c_str(),"wb");
					fseek(fp, 0L, SEEK_END);
					uint32_t sz=ftell(fp);
					rewind(fp);
					uint8_t* tempbuf=(uint8_t*)alloca(sz);
					fread(tempbuf,1,sz,fp);
					for (y=0;y<h;y+=4) {
						for (x=0;x<w;x+=4) {
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
