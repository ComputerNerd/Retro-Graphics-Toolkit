/*
Stuff related to tilemap operations goes here*/
#include "global.h"
#include "quant.h"
#include "color_convert.h"
tileMap::tileMap()
{
	mapSizeW=2;
	mapSizeH=2;
	tileMapDat=(uint8_t *)calloc(16,1);
}
tileMap::~tileMap()
{
	free(tileMapDat);
}
bool tileMap::get_hflip(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>3)&1;
}

bool tileMap::get_vflip(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>4)&1;
}

bool tileMap::get_prio(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>7)&1;
}
uint8_t tileMap::get_palette_map(uint16_t x,uint16_t y)
{
	return (tileMapDat[((y*mapSizeW)+x)*4]>>5)&3;
}
#if _WIN32
inline uint16_t swap_word(uint16_t w)
{
	uint8_t a,b;
	a=w&255;
	b=w>>8;
	return (a<<8)|b;
}
#endif
bool tileMap::saveToFile()
{
	/*!
	Saves tilemap to file returns true on success or cancelation
	returns false if there was an error but remeber if the user cancles this it is not an error
	*/
	//first see how this file should be saved
	uint8_t type=fl_choice("How would like this file saved?","Binary","C header",0);
	uint16_t x,y;
	FILE * myfile;
	if (load_file_generic("Save tilemap to",true) == true)
	{
		if (type == 1)
		{
			char temp[2048];
			myfile = fopen(the_file.c_str(),"w");
			sprintf(temp,"//Width %d Height %d\n",mapSizeW,mapSizeH);
			fputs((const char *)temp,myfile);
			fputs("const uint8_t mapDat[]={",myfile);
		}
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (myfile!=0)
		{
			switch (game_system)
			{
				case sega_genesis:
				{
					uint16_t * TheMap;
					TheMap = (uint16_t *)malloc((mapSizeW*mapSizeH)*2);

					for (y=0;y<mapSizeH;y++)
					{
						for (x=0;x<mapSizeW;x++)
						{
							uint32_t tile=get_tile(x,y);
							if (tile > 2047)
							{
								printf("Warning tile value %d exceeded 2047 at x: %d y: %d\n",tile,x,y);
								tile=2047;
							}
							#if _WIN32
							tile=swap_word(tile);
							#else
							tile=htobe16(tile);//needs to be big endian
							#endif
							*TheMap=(uint16_t)tileMapDat[((y*mapSizeW)+x)*4]<<8;//get attributes
							*TheMap++|=(uint16_t)tile;//add tile
						}
					}
					//TheMap--;
					TheMap-=mapSizeW*mapSizeH;//return to begining so it can be freeded and the file saved
					if (type == 1)
					{
						if (saveBinAsText(TheMap,mapSizeW*mapSizeH*2,myfile)==false)
							return false;
						fputs("};",myfile);
					}
					else
						fwrite(TheMap,2,mapSizeW*mapSizeH,myfile);
					free(TheMap);
				}//brackets used to prevent TheMap conflict
				break;
				case NES:
				{
					uint8_t * TheMap;
					TheMap = (uint8_t *)malloc(mapSizeW*mapSizeH);
					for (y=0;y<mapSizeH;y++)
					{
						for (x=0;x<mapSizeW;x++)
						{
							uint32_t tile=get_tile(x,y);
							if (tile > 255)
							{
								printf("Warning tile value %d exceeded 255 at x: %d y: %d\n",tile,x,y);
								tile=255;
							}
							*TheMap++=tile;
							
						}
					}
					//TheMap--;
					TheMap-=mapSizeW*mapSizeH;//return to begining so it can be freeded and the file sized
					if (type == 1)
					{
						if (saveBinAsText(TheMap,mapSizeW*mapSizeH,myfile)==false)
							return false;
						fputs("};",myfile);
					}
					else
						fwrite(TheMap,1,mapSizeW*mapSizeH,myfile);
					free(TheMap);
				}
				break;
			}
			fclose(myfile);
			puts("File Saved");
		}
		else
		{
			return false;
		}
		
	}
	if (game_system == NES)
	{
		if (load_file_generic("Save attributes to",true) == true)
		{
			if (type == 1)
			{
				myfile = fopen(the_file.c_str(),"w");
				fputs("const uint8_t attrDat[]={",myfile);
			}
			else
				myfile = fopen(the_file.c_str(),"wb");
			if (myfile!=0)
			{
				uint8_t * AttrMap = (uint8_t *)malloc((mapSizeW/4)*(mapSizeH/4));
				uint8_t * freeAttrMap=AttrMap;
				for (y=0;y<mapSizeH;y+=4)
				{
					for (x=0;x<mapSizeW;x+=4)
					{
						*AttrMap++ = get_palette_map(x,y) | (get_palette_map(x+2,y)<<4) | (get_palette_map(x,y+2) << 4) | (get_palette_map(x+2,y+2) << 6);
						printf("x: %d y: %d\n",x,y);
					}
				}
				//AttrMap-=(mapSizeW/4)*(mapSizeH/4);
				printf("%d %d\n",AttrMap,freeAttrMap);
				if (type == 1)
				{
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
			{
				return false;
			}
		}
	}
	return true;
}
void zero_error_tile_map(int32_t x)
{/*! this is a long string I do not want it stored more than once*/
	fl_alert("Please enter value greater than zero you on the other hand entered %d",x);
}
bool tileMap::loadFromFile()
{
//start by loading the file
	/*Only will return false when there is a error malloc of file error
	the file saving user cancalation and not entering the number correctly return true*/
	if (load_file_generic("Load tile map data") == true)
	{
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
		if (w <= 0)
		{
			zero_error_tile_map(w);
			return true;
		}
		if (game_system == NES && (w & 1))
		{
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
		if (h <= 0)
		{
			zero_error_tile_map(h);
			return true;
		}
		if (game_system == NES && (h & 1))
		{
			fl_alert("Error unlike in sega genesis mode NES mode needs the width and height the be a multiple to 2");
			return true;
		}

		//we can now load the map

		str_ptr=&str[0];
		str_ptr=(char *)fl_input("Enter offset\nIf the tile map is generated by this program enter zero\nIf not then the first tile may not be zero\nLets say that the first tile is 200 you would enter -200 that makes the first tile zero\nIf you want the first tile to have an offset enter a positive number\nFor example if the tilemap has the first tile set to zero and you enter 5 the first tile will be tile 5");
		if (str_ptr == 0)
		{
			//cout << "canceled by user" << endl;
			return true;
		}
		if (verify_str_number_only(str_ptr) == false)
		{
			return true;
		}
		int32_t offset=atoi(str_ptr);
		ifstream file (tilemap_file.c_str(), ios::in|ios::binary|ios::ate);
		file_size = file.tellg();
		uint32_t size_temp;
		switch (game_system)
		{
			case sega_genesis:
				size_temp=(w*h)*2;
			break;
			case NES:
				size_temp=w*h;
			break;
		}
		if (size_temp > file_size)
		{
			fl_alert("Error: The Tile map that you are attempting to load is smaller than a tile map that has the width and height that you specified");
			file.close();//even though there was an error the file was open so it still needs to be closed
			return true;//return so that the file does not get loaded
		}
		file.seekg (0, ios::beg);//return to the beginning of the file
		//start converting to tile
		//free(tile_map);
		tileMapDat = (uint8_t *) realloc(tileMapDat,(w*h)*4);
		uint8_t * tempMap = (uint8_t *) malloc(size_temp);
		if (tileMapDat == 0)
		{
			show_malloc_error(size_temp)
		}
		file.read ((char *)tempMap, size_temp);
		file.close();
		window->map_w->value(w);
		window->map_h->value(h);
		mapSizeW=w;
		mapSizeH=h;
		uint16_t x,y;
		switch (game_system)
		{
			case sega_genesis:
				for (y=0;y<h;y++)
				{
					for (x=0;x<w;x++)
					{
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
						
					}
				}
			break;
			case NES:
				for (y=0;y<h;y++)
				{
					for (x=0;x<w;x++)
					{
						uint8_t temp=*tempMap++;
						if (temp+offset > 0)
							set_tile((int32_t)temp+offset,x,y);
						else
							set_tile(0,x,y);
						printf("Tile value %d at %d %d\n",temp+offset,x,y);
					}
				}
				//now load attributes
				if (load_file_generic("Load Attribtues") == true)
				{
					puts("done");
				}
			break;
		}
		tempMap-=size_temp;
		free(tempMap);
		window->redraw();
	}
}
void tileMap::sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip)
{
	uint16_t x,y;
	int32_t temp;
	for (y=0;y<mapSizeH;y++)
	{
		for (x=0;x<mapSizeW;x++)
		{
			temp=get_tile(x,y);
			if (temp == oldTile)
			{
				set_tile(newTile,x,y);
				if (hflip == true)
					set_hflip(x,y,true);
				if (vflip == true)
					set_vflip(x,y,true);
			}
			else if (temp > oldTile)
			{
				temp--;
				if (temp < 0)
					temp=0;
				set_tile(temp,x,y);
			}
		}
	}
}

bool truecolor_to_image(uint8_t * the_image,int8_t useRow,bool useAlpha)
{
	/*!
	the_image pointer to image must be able to hold the image using rgba 32bit
	useRow what row to use or -1 for no row
	*/
	if (the_image == 0)
	{
		fl_alert("Error malloc must be called before generating this image");
		return false;
	}
	puts("Truecolor to image starting");
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeH*8;
	uint16_t x_tile=0,y_tile=0;
	uint32_t truecolor_tile_ptr=0;
	uint8_t pixelSize,pSize2;
	if (useAlpha)
	{
		pixelSize=4;
		pSize2=32;
	}
	else
	{
		pixelSize=3;
		pSize2=24;
	}
	if (useRow != -1)
	{
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8)//a tiles y
		{
			for (uint32_t b=0;b<w*pixelSize;b+=pSize2)//b tiles x
			{
				truecolor_tile_ptr=get_tileRow(x_tile,y_tile,useRow)*256;
				if (truecolor_tile_ptr != -256)
				{
					for (uint32_t y=0;y<w*pixelSize*8;y+=w*pixelSize)//pixels y
					{
						if (useAlpha)
							memcpy(&the_image[a+b+y],&currentProject->tileC->truetileDat[truecolor_tile_ptr],32);
						else
						{
							uint8_t xx=0;
							for (uint8_t x=0;x<32;x+=4)//pixels x
							{
								the_image[a+b+y+xx]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x];
								the_image[a+b+y+xx+1]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+1];
								the_image[a+b+y+xx+2]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+2];
								xx+=3;
							}
						}
						truecolor_tile_ptr+=32;
					}
				}
				x_tile++;
			}
			x_tile=0;
			y_tile++;
		}
	}
	else
	{
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8)//a tiles y
		{
			for (uint32_t b=0;b<w*pixelSize;b+=pSize2)//b tiles x
			{				
				truecolor_tile_ptr=get_tile(x_tile,y_tile)*256;
				for (uint32_t y=0;y<w*pixelSize*8;y+=w*pixelSize)//pixels y
				{
					if (useAlpha)
						memcpy(&the_image[a+b+y],&currentProject->tileC->truetileDat[truecolor_tile_ptr],32);
					else
					{
						uint8_t xx=0;
						for (uint8_t x=0;x<32;x+=4)//pixels x
						{
							the_image[a+b+y+xx]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x];
							the_image[a+b+y+xx+1]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+1];
							the_image[a+b+y+xx+2]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+2];
							xx+=3;
						}
					}
					truecolor_tile_ptr+=32;
				}
				x_tile++;
			}
			x_tile=0;
			y_tile++;
		}
	}
	puts("Done");
	return true;
}

void generate_optimal_palette(Fl_Widget*,void * row)
{
	/*
	This function is one of the more importan features of the program
	This will look at the tile map and based on that find an optimal palette
	*/
	uint8_t * image;
	//uint8_t * colors;
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeH*8;
	uint32_t colors_found;
	uint8_t * found_colors;
	switch (game_system)
	{
		case sega_genesis:
			switch((uintptr_t)row)
			{
				case 0:
					//this is easy we just convert tilemap to image count unique colors if less than 16 then just use that else reduce palete
					image = (uint8_t *)malloc(w*h*3);
					found_colors = (uint8_t *)malloc(w*3+3);
					truecolor_to_image(image,-1,false);
					colors_found=count_colors(image,w,h,&found_colors[0],false);
					printf("Unique colors %d\n",colors_found);
					if (colors_found < 17)
					{
						printf("16 or less colors\n");
						for (uint8_t x=0;x<colors_found;x++)
						{
							uint8_t r,g,b;
							r=found_colors[(x*3)];
							g=found_colors[(x*3)+1];
							b=found_colors[(x*3)+2];
							printf("R=%d G=%d B=%d\n",r,g,b);
							r=(r+18)/36;
							g=(g+18)/36;
							b=(b+18)/36;
							r*=2;
							g*=2;
							b*=2;
							//bgr
							currentProject->palDat[x*2]=b;
							currentProject->palDat[(x*2)+1]=r+(g<<4);
							currentProject->rgbPal[(x*3)]=r*18;
							currentProject->rgbPal[(x*3)+1]=g*18;
							currentProject->rgbPal[(x*3)+2]=b*18;
						}

						window->redraw();
					}
					else
					{
						printf("More than 16 colors reducing to 16 colors\n");
						/*this uses denesis lee's v3 color quant which is fonund at http://www.gnu-darwin.org/www001/ports-1.5a-CURRENT/graphics/mtpaint/work/mtpaint-3.11/src/quantizer.c*/
						uint8_t user_pal[3][256];
						
						uint8_t rgb_pal2[768];
						uint8_t colorz=16;
						bool can_go_again=true;
try_again_color:
						dl3quant(image,w,h,colorz,user_pal);
						for (uint16_t x=0;x<colorz;x++)
						{
							uint8_t r=0,g=0,b=0;
							
							r=user_pal[0][x];
							g=user_pal[1][x];
							b=user_pal[2][x];
							//printf("R=%d G=%d B=%d\n",r,g,b);
							r=(int16_t)(r+18)/36;//prevents overflow glitch
							g=(int16_t)(g+18)/36;
							b=(int16_t)(b+18)/36;
							//r*=2;
							//g*=2;
							//b*=2;
							//bgr
							//currentProject->palDat[x*2]=b;
							//currentProject->palDat[(x*2)+1]=r+(g<<4);
							rgb_pal2[(x*3)]=r*36;
							rgb_pal2[(x*3)+1]=g*36;
							rgb_pal2[(x*3)+2]=b*36;
						}
						uint8_t new_colors = count_colors(rgb_pal2,colorz,1,currentProject->rgbPal);
						printf("Unique colors in palette %d\n",new_colors);
						if (new_colors < 16)
						{
							if (can_go_again == true)
							{
								printf("Trying again needs more color\n");
								if (colorz != 255)
								{
									colorz++;
								}
								else
								{
									can_go_again=false;
								}
								goto try_again_color;
							}
						}
						if (new_colors > 16)
						{
							can_go_again=false;
							printf("Woops too many colors\n");
							colorz--;
							goto try_again_color;
						}
						for (uint8_t x=0;x<16;x++)
						{
							uint8_t r=0,g=0,b=0;
							
							r=currentProject->rgbPal[x*3];
							g=currentProject->rgbPal[(x*3)+1];
							b=currentProject->rgbPal[(x*3)+2];
							r/=36;
							g/=36;
							b/=36;
							r*=2;
							g*=2;
							b*=2;
							currentProject->palDat[x*2]=b;
							currentProject->palDat[(x*2)+1]=r+(g<<4);
						}
						
						//free(image_2);
					}
					free(image);
					free(found_colors);
					window->redraw();
				break;

				default:
					show_default_error
				break;
			}
		break;
		case NES:
			switch((uintptr_t)row)
			{
				case 0:
					image = (uint8_t *)malloc(w*h*3);
					found_colors = (uint8_t *)malloc(w*3+3);
					truecolor_to_image(image,-1,false);
					colors_found=count_colors(image,w,h,found_colors);
					if (colors_found < 5)
					{
						printf("4 or less colors\n");
						for (uint8_t x=0;x<colors_found;x++)
						{
							uint8_t r,g,b;
							r=found_colors[(x*3)];
							g=found_colors[(x*3)+1];
							b=found_colors[(x*3)+2];
							printf("R=%d G=%d B=%d\n",r,g,b);
							uint8_t temp = to_nes_color_rgb(r,g,b);
							currentProject->palDat[x]=temp;
							
						}
						update_emphesis(NULL,NULL);
					}
					else
					{
						printf("more than 4 colors\n");
						uint8_t user_pal[3][256];
						
						uint8_t rgb_pal2[768];
						uint8_t colorz=4;
						bool can_go_again=true;
try_again_nes_color:
						dl3quant(image,w,h,colorz,user_pal);
						for (uint16_t x=0;x<colorz;x++)
						{
							uint8_t r=0,g=0,b=0;
							
							r=user_pal[0][x];
							g=user_pal[1][x];
							b=user_pal[2][x];
							//printf("R=%d G=%d B=%d\n",r,g,b);
							uint8_t temp=to_nes_color_rgb(r,g,b);
							//r*=2;
							//g*=2;
							//b*=2;
							//bgr
							//currentProject->palDat[x*2]=b;
							//currentProject->palDat[(x*2)+1]=r+(g<<4);
							uint32_t temp_rgb = MakeRGBcolor(temp);
							rgb_pal2[(x*3)]=(temp_rgb>>16)&255;
							rgb_pal2[(x*3)+1]=(temp_rgb>>8)&255;
							rgb_pal2[(x*3)+2]=temp_rgb&255;
						}
						uint8_t new_colors = count_colors(rgb_pal2,colorz,1,currentProject->rgbPal);
						printf("Unique colors in palette %d\n",new_colors);
						if (new_colors < 4)
						{
							if (can_go_again == true)
							{
								printf("Trying again needs more color\n");
								if (colorz != 255)
								{
									colorz++;
								}
								else
								{
									can_go_again=false;
								}
								goto try_again_nes_color;
							}
						}
						if (new_colors > 4)
						{
							can_go_again=false;
							printf("Woops too many colors\n");
							colorz--;
							goto try_again_nes_color;
						}
						for (uint8_t x=0;x<4;x++)
						{
							currentProject->palDat[x]=to_nes_color(x);
						}
						update_emphesis(NULL,NULL);
					}
					free(image);
					free(found_colors);
					window->redraw();
				break;
				default:
					show_default_error
				break;
			}
			break;
		default:
			show_default_error
		break;
	}
}
