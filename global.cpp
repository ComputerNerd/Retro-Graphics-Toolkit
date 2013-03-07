#include "includes.h"
#include "class_global.h"
#include "errorMsg.h"
#include "dither.h"
#include "class_tiles.h"
//fltk widget realted stuff
Fl_Group * shadow_highlight_switch;
//bools used to toggle stuff
bool shadow_highlight;
bool show_grid;
bool G_hflip;
bool G_vflip;
bool G_highlow_p;
bool show_grid_placer;
//tabs group id
intptr_t pal_id;
intptr_t tile_edit_id;
intptr_t tile_place_id;
//moveable offsets
uint16_t map_scroll_pos_x;
uint16_t map_scroll_pos_y;
uint16_t map_off_x,map_off_y;
uint16_t tile_edit_offset_x;
uint16_t tile_edit_offset_y;
//uint8_t palette_bar_offset_y;
uint16_t tile_placer_tile_offset_y;
uint16_t tile_edit_truecolor_off_x,tile_edit_truecolor_off_y;
uint16_t true_color_box_x,true_color_box_y;
uint8_t tile_zoom_edit;
uint8_t game_system;
uint8_t palette[128];
uint8_t truecolor_temp[4];/*!< This stores the rgba data selected with the truecolor sliders*/
string the_file;//this is for tempory use only
uint8_t mode_editor;//this is used to determin which thing to draw
uint8_t palette_muliplier;
uint8_t palette_adder;
uint8_t rgb_pal[192];
//uint8_t palette_entry;
//uint8_t rgb_temp[3];
//uint8_t tile_palette_row;//sets which palette row the tile displays
//uint8_t tile_palette_row_placer;//sets which palette row the tile placer displays
//uint8_t * tiles;
uint8_t * tile_map;
//uint32_t tiles_amount;
//uint32_t current_tile;//current tile that we are editing minus one
//uint32_t current_tile_placer;
uint32_t file_size;
uint16_t map_size_x;
uint16_t map_size_y;
uint8_t * attr_nes;
uint8_t ditherAlg;
#define PI 3.141592653589793238462643383279
inline int wave(int p,int color)
{
	return (color+p+8)%12 < 6;
}
inline float gammafix(float f,float gamma)
{
	return f < 0.f ? 0.f : std::pow(f, 2.2f / gamma);
}
inline int clamp(int v)
{
	return v<0 ? 0 : v>255 ? 255 : v;
}
 uint32_t MakeRGBcolor(uint32_t pixel,
                          float saturation = 1.1f, float hue_tweak = 0.0f,
                          float contrast = 1.0f, float brightness = 1.0f,
                          float gamma = 2.2f)
    {
        /*!
	 The input value is a NES color index (with de-emphasis bits).
         We need RGB values. Convert the index into RGB.
         For most part, this process is described at:
            http://wiki.nesdev.com/w/index.php/NTSC_video
	*/
        // Decode the color index
        int color = (pixel & 0x0F), level = color<0xE ? (pixel>>4) & 3 : 1;
            // Voltage levels, relative to synch voltage
            static const float black=.518f, white=1.962f, attenuation=.746f,
              levels[8] = {.350f, .518f, .962f,1.550f,  // Signal low
                          1.094f,1.506f,1.962f,1.962f}; // Signal high

        float lo_and_hi[2] = { levels[level + 4 * (color == 0x0)],
                               levels[level + 4 * (color <  0xD)] };

        // Calculate the luma and chroma by emulating the relevant circuits:
        float y=0.f, i=0.f, q=0.f;
        //auto wave = [](int p, int color) { return (color+p+8)%12 < 6; };
        for(int p=0; p<12; ++p) // 12 clock cycles per pixel.
        {
            // NES NTSC modulator (square wave between two voltage levels):
            float spot = lo_and_hi[wave(p,color)];

            // De-emphasis bits attenuate a part of the signal:
            if(((pixel & 0x40) && wave(p,12))
            || ((pixel & 0x80) && wave(p, 4))
            || ((pixel &0x100) && wave(p, 8))) spot *= attenuation;

            // Normalize:
            float v = (spot - black) / (white-black);
			//float v = (spot - black) / (white-black) / 12.f;
            // Ideal TV NTSC demodulator:
            // Apply contrast/brightness
            v = (v - .5f) * contrast + .5f;
            v *= brightness / 12.f;

            y += v;
            i += v * std::cos( (PI/6.f) * (p+hue_tweak) );
            q += v * std::sin( (PI/6.f) * (p+hue_tweak) );
        }   

        i *= saturation;
        q *= saturation;

        // Convert YIQ into RGB according to FCC-sanctioned conversion matrix.
        //auto gammafix = [=](float f) { return f < 0.f ? 0.f : std::pow(f, 2.2f / gamma); };
        //auto clamp = [](int v) { return v<0 ? 0 : v>255 ? 255 : v; };
        uint32_t rgb = 0x10000*clamp(255 * gammafix(y +  0.946882f*i +  0.623557f*q,gamma))
                     + 0x00100*clamp(255 * gammafix(y + -0.274788f*i + -0.635691f*q,gamma))
                     + 0x00001*clamp(255 * gammafix(y + -1.108545f*i +  1.709007f*q,gamma));
        return rgb;
    }
 //the game system defines are also defined in global.h
#define sega_genesis 0
#define NES 1
/*
from genvdp.txt

----------------------------------------------------------------------------
 13.) Background Layers
 ----------------------------------------------------------------------------

 The VDP manages two background layers, called plane A and plane B.

 Name Tables
 -----------

 There are three tables stored in video RAM that define the layout for
 planes A, B, and W.

 Each table is a matrix of 16-bit words. Each word has the following format:
  I aded the numbering not in origonal gendvp.txt
    15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
    p  c   c  v  h  n n n n n n n n n n n

    p = Priority flag
    c = Palette select
    v = Vertical flip
    h = Horizontal flip
    n = Pattern name
 
 The pattern name is the upper 11 bits of the physical address of pattern
 in video RAM. Bit zero of the name is ignored in interlace mode 2.

 The vertical and horizontal flip flags tell the VDP to draw the pattern
 flipped in either direction.

 The palette select allows the pattern to be shown in one of four
 16-color palettes.

 The priority flag is described later.

 The name tables for plane A and B share the same dimensions. The name
 table size cannot exceed 8192 bytes, so while a 64x64 or 128x32 name
 table is allowed, a size of 128x128 or 64x128 is invalid.

 The name table for plane W is 32x32 in 32-cell mode, and 64x32 in 40-cell
 mode. This size is fixed and is entirely dependant on the display width.

*/


uint32_t get_tile(uint16_t x,uint16_t y)
{
	//first calulate which tile we want
	if (map_size_x < x || map_size_y < y)
	{
		fl_alert("Error tried to get a non existen tile on the map");
		return 0;
	}
	uint32_t selected_tile=((y*map_size_x)+x)*4;
	//get both bytes
	uint8_t temp_1,temp_2,temp_3;
	temp_1=tile_map[selected_tile+1];//least sigficant is stored in the lowest address
	temp_2=tile_map[selected_tile+2];
	temp_3=tile_map[selected_tile+3];//most sigficant
	//printf("Returned %d\n",(temp_1<<16)+(temp_2<<8)+temp_3);
	return (temp_1<<16)+(temp_2<<8)+temp_3;
}
int32_t get_tileRow(uint16_t x,uint16_t y,uint8_t useRow)
{
	//first calulate which tile we want
	uint32_t selected_tile=((y*map_size_x)+x)*4;
	//get both bytes
	if (((tile_map[selected_tile]>>5)&3) == useRow)
	{
		uint8_t temp_1,temp_2,temp_3;
		temp_1=tile_map[selected_tile+1];//least sigficant is stored in the lowest address
		temp_2=tile_map[selected_tile+2];
		temp_3=tile_map[selected_tile+3];//most sigficant
		return (temp_1<<16)+(temp_2<<8)+temp_3;
	}
	else
	{
		return -1;
	}
}
void set_palette_type(uint8_t type)
{
	//get the type
	switch (type)
	{
		case 0://normal
			palette_muliplier=18;
			palette_adder=0;
		break;
		case 1:
			palette_muliplier=9;
			palette_adder=0;
		break;
		case 2:
			palette_muliplier=9;
			palette_adder=126;
		break;
		default:
			show_default_error
		break;
	}
	//now reconvert all the colors
	for (uint8_t pal=0; pal < 128;pal+=2)
	{
		//to convert to rgb first get value of color then multiply it by 16 to get rgb
		//first get blue value
		//the rgb array is in rgb format and the genesis palette is bgr format
		uint8_t rgb_array = pal+(pal/2);//multiply pal by 1.5
		uint8_t temp_var = palette[pal];
		temp_var*=palette_muliplier;
		temp_var+=palette_adder;
		rgb_pal[rgb_array+2]=temp_var;
		//seperating the gr values will require some bitwise operations
		//to get g shift to the right by 4
		temp_var = palette[pal+1];
		temp_var>>=4;
		temp_var*=palette_muliplier;
		temp_var+=palette_adder;
		rgb_pal[rgb_array+1]=temp_var;
		//to get r value apply the and opperation by 0xF or 15
		temp_var = palette[pal+1];
		temp_var&=0xF;
		temp_var*=palette_muliplier;
		temp_var+=palette_adder;
		rgb_pal[rgb_array]=temp_var;
	}
	//window->redraw();
}
uint8_t get_palette_map(uint16_t x,uint16_t y)
{
	return (tile_map[((y*map_size_x)+x)*4]>>5)&3;
}

bool get_hflip(uint16_t x,uint16_t y)
{
	return (tile_map[((y*map_size_x)+x)*4]>>3)&1;
}

bool get_vflip(uint16_t x,uint16_t y)
{
	return (tile_map[((y*map_size_x)+x)*4]>>4)&1;
}

bool get_prio(uint16_t x,uint16_t y)
{
	return (tile_map[((y*map_size_x)+x)*4]>>7)&1;
}

void set_tile_full(uint32_t tile,uint16_t x,uint16_t y,uint8_t palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio)
{
	if (map_size_x < x || map_size_y < y)
	{
		fl_alert("Error tried to set a non existen tile on the map");
		return;
	}
	uint32_t selected_tile=((y*map_size_x)+x)*4;
	uint8_t flags;
	uint8_t the_tiles;
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
	tile_map[selected_tile]=flags;
	//in little endain the least sigficant byte is stored in the lowest address
	tile_map[selected_tile+1]=(tile>>16)&255;
	tile_map[selected_tile+2]=(tile>>8)&255;
	tile_map[selected_tile+3]=tile&255;
}

void set_tile(uint32_t tile,uint16_t x,uint16_t y)
{
	//we must split into two varibles
	if (map_size_x < x || map_size_y < y)
	{
		fl_alert("Error: Tried to set a non existent tile on the tile map");
		return;
	}
	uint32_t selected_tile=((y*map_size_x)+x)*4;
	tile_map[selected_tile+1]=(tile>>16)&255;
	tile_map[selected_tile+2]=(tile>>8)&255;
	tile_map[selected_tile+3]=tile&255;
}
void set_tile_clear_flags(uint32_t tile,uint16_t x,uint16_t y)
{
	uint32_t selected_tile=((y*map_size_x)+x)*4;
	tile_map[selected_tile]=0;
	tile_map[selected_tile+1]=(tile>>16)&255;
	tile_map[selected_tile+2]=(tile>>8)&255;
	tile_map[selected_tile+3]=tile&255;
}
void invert_vflip(uint16_t x,uint16_t y)
{
	tile_map[((y*map_size_x)+x)*4]^=16;
}
void invert_hflip(uint16_t x,uint16_t y)
{
	tile_map[((y*map_size_x)+x)*4]^=8;
}
void invert_prio(uint16_t x,uint16_t y)
{
	tile_map[((y*map_size_x)+x)*4]^=128;
}
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

uint8_t find_near_color_from_row_rgb(uint8_t row,uint8_t r,uint8_t g,uint8_t b)
{
	
	uint8_t i;
	int distanceSquared, minDistanceSquared, bestIndex = 0;
	minDistanceSquared = 255*255 + 255*255 + 255*255 + 1;
	uint8_t max_rgb;
	switch (game_system)
	{
		case sega_genesis:
			max_rgb=48;//16*3=48
		break;
		case NES:
			max_rgb=12;//4*3=12
		break;
	}
	row*=max_rgb;
    for (i=row; i<max_rgb+row; i+=3)
	{
        int Rdiff = (int) r - (int)rgb_pal[i];
        int Gdiff = (int) g - (int)rgb_pal[i+1];
        int Bdiff = (int) b - (int)rgb_pal[i+2];
        distanceSquared = Rdiff*Rdiff + Gdiff*Gdiff + Bdiff*Bdiff;
        if (distanceSquared <= minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            bestIndex = i;
        }
    }
    return bestIndex;
}

uint8_t find_near_color_from_row(uint8_t row,uint8_t r,uint8_t g,uint8_t b)
{
	
	uint8_t i;
    int distanceSquared, minDistanceSquared, bestIndex = 0;
    minDistanceSquared = 255*255 + 255*255 + 255*255 + 1;
	uint8_t max_rgb;
	switch (game_system)
	{
		case sega_genesis:
			max_rgb=48;//16*3=48
		break;
		case NES:
			max_rgb=12;//4*3=12
		break;
	}
	row*=max_rgb;
    for (i=0; i<max_rgb; i+=3)
	{
        int Rdiff = (int) r - (int)rgb_pal[i+row];
        int Gdiff = (int) g - (int)rgb_pal[i+row+1];
        int Bdiff = (int) b - (int)rgb_pal[i+row+2];
        distanceSquared = Rdiff*Rdiff + Gdiff*Gdiff + Bdiff*Bdiff;
        if (distanceSquared <= minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            bestIndex = i;
        }
    }
    return bestIndex/3;
}

inline uint16_t add_pixel(int16_t a,int16_t b)
{
	if (a+b > 255)
	{
		a=255;
	}
	else if (a+b < 0)
	{
		a=0;
	}
	else
	{
		a+=b;
	}
	return a;
}

uint32_t cal_offset_truecolor(uint16_t x,uint16_t y,uint16_t rgb,uint32_t tile)
{
	/*
	cal_offset_truecolor is made to help when accesing a true color tile array
	an example of it would be
	red_temp=truecolor_data[cal_offset_truecolor(0,0,0,0)]//get the red pixel at pixel 0,0 at tile 0
	green_temp=truecolor_data[cal_offset_truecolor(0,0,1,0)]//get the green pixel at pixel 0,0 at tile 0
	blue_temp=truecolor_data[cal_offset_truecolor(0,0,2,0)]//get the blue pixel at pixel 0,0 at tile 0
	*/
	return (x*4)+(y*32)+rgb+(tile*256);
}

void set_hflip(uint16_t x,uint16_t y,bool hflip_set)
{
	if (hflip_set == true)
	{
		tile_map[((y*map_size_x)+x)*4]|= 1 << 3;
	}
	else
	{
		tile_map[((y*map_size_x)+x)*4]&= ~(1 << 3);
	}
}
void set_vflip(uint16_t x,uint16_t y,bool vflip_set)
{
	if (vflip_set == true)
	{
		tile_map[((y*map_size_x)+x)*4]|= 1 << 4;
	}
	else
	{
		tile_map[((y*map_size_x)+x)*4]&= ~(1 << 4);
	}
}
void set_prio(uint16_t x,uint16_t y,bool prio_set)
{
	if (prio_set == true)
	{
		tile_map[((y*map_size_x)+x)*4] |= 1 << 7;
	}
	else
	{
		tile_map[((y*map_size_x)+x)*4] &= ~(1 << 7);
	}
}

void resize_tile_map(uint16_t new_x,uint16_t new_y)
{
	//map_size_x and map_size_y hold old map size
	if (new_x == map_size_x && new_y == map_size_y) return;
	//now create a temp buffer to hold the old data
	uint16_t x,y;//needed for loop varibles to copy data
	//uint8_t * temp = new uint8_t [(new_x*new_y)*2];
	uint8_t * temp=0;
	temp=(uint8_t *)malloc((new_x*new_y)*4);
	if (temp == 0)
	{
		//cout << "error" << endl;
		show_malloc_error((new_x*new_y)*4)
		return;
	}
	//now copy old data to temp
	uint32_t sel_map;
	for (y=0;y<new_y;y++)
	{
		for (x=0;x<new_x;x++)
		{
			if (x < map_size_x && y < map_size_y)
			{	
				sel_map=((y*new_x)+x)*4;
				uint32_t sel_map_old=((y*map_size_x)+x)*4;
				temp[sel_map]=tile_map[sel_map_old];
				temp[sel_map+1]=tile_map[sel_map_old+1];
				temp[sel_map+2]=tile_map[sel_map_old+2];
				temp[sel_map+3]=tile_map[sel_map_old+3];
			}
			else
			{
				sel_map=((y*new_x)+x)*4;
				temp[sel_map]=0;
				temp[sel_map+1]=0;
				temp[sel_map+2]=0;
				temp[sel_map+3]=0;
			}
		}
	}
	tile_map = (uint8_t *)realloc(tile_map,(new_x*new_y)*4);
	if (tile_map == 0)
	{
		//fl_alert("An error occured while realloc tile_map");
		show_realloc_error((new_x*new_y)*4)
		return;
	}
	for (uint32_t c=0;c<(new_x*new_y)*4;c++)
	{
		tile_map[c]=temp[c];
	}
	free(temp);
	map_size_x=new_x;
	//calulate new scroll size
	uint16_t old_scroll=window->map_x_scroll->value();
	uint8_t tile_size_placer=window->place_tile_size->value();
	int32_t map_scroll=((tile_size_placer*8)*map_size_x)-map_off_x;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
	{
		map_scroll=0;
	}
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
	{
		old_scroll=map_scroll;
	}
	window->map_x_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));//the reason for adding map_scroll/2 to map_scroll is because without it the user will not be able to scroll the tilemap all the way
	map_size_y=new_y;
	old_scroll=window->map_y_scroll->value();
	tile_size_placer=window->place_tile_size->value();
	map_scroll=((tile_size_placer*8)*map_size_y)-map_off_y;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
	{
		map_scroll=0;
	}
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
	{
		old_scroll=map_scroll;
	}
	window->map_y_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));

}

bool load_file_generic(const char * the_tile="Pick a file",bool save_file=false)
{	
	// Create native chooser
	Fl_Native_File_Chooser native;
	native.title(the_tile);
	if (save_file == false)
	{
		native.type(Fl_Native_File_Chooser::BROWSE_FILE);
	}
	else
	{
		native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	}
	// Show native chooser
	switch ( native.show() )
	{
	case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
	case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
	default:// PICKED FILE
		if (native.filename())
		{
			the_file=native.filename();
			//native.~Fl_Native_File_Chooser();//sementation fault
			return true;//the only way this this function will return true is the user picked a file
		}
		break;
	}
	//native.~Fl_Native_File_Chooser();
	return false;//if an error happened or the user did not pick a file the function returns false
}
