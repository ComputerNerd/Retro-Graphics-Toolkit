/*
Stuff related to tilemap operations goes here*/
#include "global.h"
#include "quant.h"
#include "color_convert.h"
void sub_tile_map(uint32_t oldTile,uint32_t newTile,bool hflip,bool vflip)
{
	uint16_t x,y;
	int32_t temp;
	for (y=0;y<map_size_y;y++)
	{
		for (x=0;x<map_size_x;x++)
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

void convertTilemaptofile(string filename)
{
	//creates a file that contains genesis tilemap
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
	w=map_size_x*8;
	h=map_size_y*8;
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
							memcpy(&the_image[a+b+y],&tiles_main.truetileDat[truecolor_tile_ptr],32);
						else
						{
							uint8_t xx=0;
							for (unsigned char x=0;x<32;x+=4)//pixels x
							{
								the_image[a+b+y+xx]=tiles_main.truetileDat[truecolor_tile_ptr+x];
								the_image[a+b+y+xx+1]=tiles_main.truetileDat[truecolor_tile_ptr+x+1];
								the_image[a+b+y+xx+2]=tiles_main.truetileDat[truecolor_tile_ptr+x+2];
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
						memcpy(&the_image[a+b+y],&tiles_main.truetileDat[truecolor_tile_ptr],32);
					else
					{
						uint8_t xx=0;
						for (unsigned char x=0;x<32;x+=4)//pixels x
						{
							the_image[a+b+y+xx]=tiles_main.truetileDat[truecolor_tile_ptr+x];
							the_image[a+b+y+xx+1]=tiles_main.truetileDat[truecolor_tile_ptr+x+1];
							the_image[a+b+y+xx+2]=tiles_main.truetileDat[truecolor_tile_ptr+x+2];
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
	unsigned char * image;
	//unsigned char * colors;
	unsigned int w,h;
	w=map_size_x*8;
	h=map_size_y*8;
	unsigned int colors_found;
	unsigned char * found_colors;
	switch (game_system)
	{
		case sega_genesis:
			switch((uintptr_t)row)
			{
				case 0:
					//this is easy we just convert tilemap to image count unique colors if less than 16 then just use that else reduce palete
					image = (unsigned char *)malloc(w*h*3);
					found_colors = (unsigned char *)malloc(w*3+3);
					truecolor_to_image(image,-1,false);
					colors_found=count_colors(image,w,h,found_colors);
					printf("Unique colors %d\n",colors_found);
					if (colors_found < 17)
					{
						printf("16 or less colors\n");
						for (unsigned char x=0;x<colors_found;x++)
						{
							unsigned char r,g,b;
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
							palette[x*2]=b;
							palette[(x*2)+1]=r+(g<<4);
							rgb_pal[(x*3)]=r*18;
							rgb_pal[(x*3)+1]=g*18;
							rgb_pal[(x*3)+2]=b*18;
						}

						window->redraw();
					}
					else
					{
						//unsigned char * image_2 = (unsigned char *)malloc(w*h*3);
						//memcpy(image_2,image,w*h*3);
						printf("More than 16 colors reducing to 16 colors\n");
						/*this uses denesis lee's v3 color quant which is fonund at http://www.gnu-darwin.org/www001/ports-1.5a-CURRENT/graphics/mtpaint/work/mtpaint-3.11/src/quantizer.c*/
						unsigned char user_pal[3][256];
						
						unsigned char rgb_pal2[768];
						unsigned char colorz=16;
						bool can_go_again=true;
try_again_color:
						dl3quant(image,w,h,colorz,user_pal);
						for (unsigned short x=0;x<colorz;x++)
						{
							unsigned char r=0,g=0,b=0;
							
							r=user_pal[0][x];
							g=user_pal[1][x];
							b=user_pal[2][x];
							//printf("R=%d G=%d B=%d\n",r,g,b);
							r=(short)(r+18)/36;//prevents overflow glitch
							g=(short)(g+18)/36;
							b=(short)(b+18)/36;
							//r*=2;
							//g*=2;
							//b*=2;
							//bgr
							//palette[x*2]=b;
							//palette[(x*2)+1]=r+(g<<4);
							rgb_pal2[(x*3)]=r*36;
							rgb_pal2[(x*3)+1]=g*36;
							rgb_pal2[(x*3)+2]=b*36;
						}
						unsigned char new_colors = count_colors(rgb_pal2,colorz,1,rgb_pal);
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
						for (unsigned char x=0;x<16;x++)
						{
							unsigned char r=0,g=0,b=0;
							
							r=rgb_pal[x*3];
							g=rgb_pal[(x*3)+1];
							b=rgb_pal[(x*3)+2];
							r/=36;
							g/=36;
							b/=36;
							r*=2;
							g*=2;
							b*=2;
							palette[x*2]=b;
							palette[(x*2)+1]=r+(g<<4);
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
					image = (unsigned char *)malloc(w*h*3);
					found_colors = (unsigned char *)malloc(w*3+3);
					truecolor_to_image(image,-1,false);
					colors_found=count_colors(image,w,h,found_colors);
					if (colors_found < 5)
					{
						printf("4 or less colors\n");
						for (unsigned char x=0;x<colors_found;x++)
						{
							unsigned char r,g,b;
							r=found_colors[(x*3)];
							g=found_colors[(x*3)+1];
							b=found_colors[(x*3)+2];
							printf("R=%d G=%d B=%d\n",r,g,b);
							unsigned char temp = to_nes_color_rgb(r,g,b);
							palette[x]=temp;
							
						}
						update_emphesis(NULL,NULL);
					}
					else
					{
						printf("more than 4 colors\n");
						unsigned char user_pal[3][256];
						
						unsigned char rgb_pal2[768];
						unsigned char colorz=4;
						bool can_go_again=true;
try_again_nes_color:
						dl3quant(image,w,h,colorz,user_pal);
						for (unsigned short x=0;x<colorz;x++)
						{
							unsigned char r=0,g=0,b=0;
							
							r=user_pal[0][x];
							g=user_pal[1][x];
							b=user_pal[2][x];
							//printf("R=%d G=%d B=%d\n",r,g,b);
							unsigned char temp=to_nes_color_rgb(r,g,b);
							//r*=2;
							//g*=2;
							//b*=2;
							//bgr
							//palette[x*2]=b;
							//palette[(x*2)+1]=r+(g<<4);
							unsigned int temp_rgb = MakeRGBcolor(temp);
							rgb_pal2[(x*3)]=(temp_rgb>>16)&255;
							rgb_pal2[(x*3)+1]=(temp_rgb>>8)&255;
							rgb_pal2[(x*3)+2]=temp_rgb&255;
						}
						unsigned char new_colors = count_colors(rgb_pal2,colorz,1,rgb_pal);
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
						for (unsigned char x=0;x<4;x++)
						{
							palette[x]=to_nes_color(x);
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
