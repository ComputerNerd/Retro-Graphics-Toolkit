#include "includes.h"
#include "class_global.h"
#include "errorMsg.h"
#include "dither.h"
#include "class_tiles.h"
#include "color_convert.h"
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
uint8_t truecolor_temp[4];/*!< This stores the rgba data selected with the truecolor sliders*/
std::string the_file;//this is for tempory use only
uint8_t mode_editor;//this is used to determin which thing to draw
uint8_t ditherAlg;
#define PIf 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989f
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989
uint8_t palTypeGen=0;
bool showTrueColor=false;
bool rowSolo=false;
uint8_t nearest_color_index(uint8_t val)
{
	//returns closest value
	//palette_muliplier
	uint8_t i;
    int32_t distanceSquared, minDistanceSquared, bestIndex = 0;
    minDistanceSquared = 255*255 + 1;
	if (game_system!=sega_genesis) {
		fl_alert("This function is for use with sega genesis/mega drive only");
		return 0;
	}
    for (i=palTypeGen; i<8+palTypeGen; i++) {
        int32_t Rdiff = (int) val - (int)palTab[i];
        distanceSquared = Rdiff*Rdiff;
        if (distanceSquared < minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            bestIndex = i;
        }
    }
    return bestIndex;
}
bool saveBinAsText(void * ptr,size_t sizeBin,FILE * myfile)
{
	/*!
	This function saves binary data as plain text useful for c headers each byte is seperated by a comma
	Returns True on sucess false on error
	*/
	uint8_t * dat=(uint8_t *)ptr;
	char str[16];
	for (size_t x=0;x<sizeBin-1;x++){
		sprintf(str,"%d",*dat);
		if (fputs(str,myfile)==0)
			return false;
		if (fputc(',',myfile)==0)
			return false;
		if ((x&63)==63){
			if (fputc('\n',myfile)==0)
				return false;
		}
		dat++;
	}
	sprintf(str,"%d",*dat);
	if (fputs(str,myfile)==0)
		return false;
	return true;
}
void tileToTrueCol(uint8_t * input,uint8_t * output,uint8_t row,bool useAlpha)
{
	switch (game_system)
	{
		case sega_genesis:
			row*=48;
			for (uint8_t y=0;y<8;++y){
				for (uint8_t x=0;x<4;++x){
					//even,odd
					uint8_t temp=*input++;
					uint8_t temp_1,temp_2;
					temp_1=temp>>4;//first pixel
					temp_2=temp&15;//second pixel
					*output++=currentProject->rgbPal[row+(temp_1*3)];
					*output++=currentProject->rgbPal[row+(temp_1*3)+1];
					*output++=currentProject->rgbPal[row+(temp_1*3)+2];
					if(useAlpha)
						*output++=255;
					*output++=currentProject->rgbPal[row+(temp_2*3)];
					*output++=currentProject->rgbPal[row+(temp_2*3)+1];
					*output++=currentProject->rgbPal[row+(temp_2*3)+2];
					if(useAlpha)
						*output++=255;
				}
			}
		break;
		case NES:
			row*=12;
			for (uint8_t y=0;y<8;++y)
			{
				for (int8_t x=7;x>=0;--x)
				{
					uint8_t temp;
					temp=(input[y]>>x)&1;
					temp|=((input[y+8]>>x)&1)<<1;
					*output++=currentProject->rgbPal[row+(temp*3)];
					*output++=currentProject->rgbPal[row+(temp*3)+1];
					*output++=currentProject->rgbPal[row+(temp*3)+2];
					if(useAlpha)
						*output++=255;
				}
			}
		break;
	}
	
}
bool verify_str_number_only(char * str)
{
/*!
Fltk provides an input text box that makes it easy for the user to type text however as a side effect they can accidently enter non number characters that may be handled weird by atoi()
this function address that issue by error checking the string and it also gives the user feedback so they are aware that the input box takes only numbers
this function returns true when the string contains only numbers 0-9 and false when there is other stuff
it will also allow the use of the - symbol as negative
*/
	while(*str++)
	{
		if (*str != 0 && *str != '-')
		{
			if (*str < '0')
			{
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
			if (*str > '9')
			{
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
		}
	}
	return true;
}
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
//uint32_t MakeRGBcolor(uint32_t pixel,float saturation = 1.1f, float hue_tweak = 0.0f,float contrast = 1.0f, float brightness = 1.0f,float gamma = 2.2f)
uint32_t MakeRGBcolor(uint32_t pixel,float saturation, float hue_tweak,float contrast, float brightness,float gamma)
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
			i += v * std::cos( (PIf/6.f) * (p+hue_tweak) );
			q += v * std::sin( (PIf/6.f) * (p+hue_tweak) );
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
const uint8_t palTab[]=   {0,49,87,119,146,174,206,255,0,27,49,71,87,103,119,130,130,146,157,174,190,206,228,255};//from http://gendev.spritesmind.net/forum/viewtopic.php?t=1389
const uint8_t palTabEmu[]={0,36,72,108,144,180,216,252,0,18,36,54,72, 90,108,126,126,144,162,180,198,216,234,252};
void set_palette_type(uint8_t type)
{
	palTypeGen=type;
	//now reconvert all the colors
	for (uint8_t pal=0; pal < 128;pal+=2){
		//to convert to rgb first get value of color then multiply it by 16 to get rgb
		//first get blue value
		//the rgb array is in rgb format and the genesis palette is bgr format
		uint8_t rgb_array = pal+(pal/2);//multiply pal by 1.5
		uint8_t temp_var = currentProject->palDat[pal];
		temp_var>>=1;
		currentProject->rgbPal[rgb_array+2]=palTab[temp_var+type];
		//seperating the gr values will require some bitwise operations
		//to get g shift to the right by 4
		temp_var = currentProject->palDat[pal+1];
		temp_var>>=5;
		currentProject->rgbPal[rgb_array+1]=palTab[temp_var+type];
		//to get r value apply the and opperation by 0xF or 15
		temp_var = currentProject->palDat[pal+1];
		temp_var&=0xF;
		temp_var>>=1;
		currentProject->rgbPal[rgb_array]=palTab[temp_var+type];
	}
	//window->redraw();
}
void set_tile_full(uint32_t tile,uint16_t x,uint16_t y,uint8_t palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio)
{
	if (currentProject->tileMapC->mapSizeW < x || currentProject->tileMapC->mapSizeH < y) {
		fl_alert("Error tried to set a non existen tile on the map");
		return;
	}
	uint32_t selected_tile=((y*currentProject->tileMapC->mapSizeW)+x)*4;
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
	currentProject->tileMapC->tileMapDat[selected_tile]=flags;
	//in little endain the least sigficant byte is stored in the lowest address
	currentProject->tileMapC->tileMapDat[selected_tile+1]=(tile>>16)&255;
	currentProject->tileMapC->tileMapDat[selected_tile+2]=(tile>>8)&255;
	currentProject->tileMapC->tileMapDat[selected_tile+3]=tile&255;
}
void set_tile(uint32_t tile,uint16_t x,uint16_t y)
{
	//we must split into two varibles
	if (currentProject->tileMapC->mapSizeW < x || currentProject->tileMapC->mapSizeH < y) {
		fl_alert("Error: Tried to set a non existent tile on the tile map");
		return;
	}
	uint32_t selected_tile=((y*currentProject->tileMapC->mapSizeW)+x)*4;
	currentProject->tileMapC->tileMapDat[selected_tile+1]=(tile>>16)&255;
	currentProject->tileMapC->tileMapDat[selected_tile+2]=(tile>>8)&255;
	currentProject->tileMapC->tileMapDat[selected_tile+3]=tile&255;
}
void set_tile_clear_flags(uint32_t tile,uint16_t x,uint16_t y)
{
	uint32_t selected_tile=((y*currentProject->tileMapC->mapSizeW)+x)*4;
	currentProject->tileMapC->tileMapDat[selected_tile]=0;
	currentProject->tileMapC->tileMapDat[selected_tile+1]=(tile>>16)&255;
	currentProject->tileMapC->tileMapDat[selected_tile+2]=(tile>>8)&255;
	currentProject->tileMapC->tileMapDat[selected_tile+3]=tile&255;
}
void invert_vflip(uint16_t x,uint16_t y)
{
	currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]^=16;
}
void invert_hflip(uint16_t x,uint16_t y)
{
	currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]^=8;
}
void invert_prio(uint16_t x,uint16_t y)
{
	currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]^=128;
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
	int bestIndex = 0;
	double minerror=9001.0;
	uint8_t max_rgb=palEdit.perRow*3;
	row*=max_rgb;
	for (i=row; i<max_rgb+row; i+=3) {
		double distance=ciede2000rgb(r,g,b,currentProject->rgbPal[i],currentProject->rgbPal[i+1],currentProject->rgbPal[i+2]);
		if (distance <= minerror) {
			if (currentProject->palType[i/3]!=2) {
				minerror = distance;
				bestIndex = i;
			}
		}
	}
    return bestIndex;
}
uint8_t find_near_color_from_row(uint8_t row,uint8_t r,uint8_t g,uint8_t b)
{
	return (find_near_color_from_row_rgb(row,r,g,b)/3)-(row*palEdit.perRow);
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
		currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]|= 1 << 3;
	else
		currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]&= ~(1 << 3);
}
void set_vflip(uint16_t x,uint16_t y,bool vflip_set)
{
	if (vflip_set == true)
		currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]|= 1 << 4;
	else
		currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4]&= ~(1 << 4);
}
void set_prio(uint16_t x,uint16_t y,bool prio_set)
{
	if (prio_set == true)
		currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4] |= 1 << 7;
	else
		currentProject->tileMapC->tileMapDat[((y*currentProject->tileMapC->mapSizeW)+x)*4] &= ~(1 << 7);
}
void resize_tile_map(uint16_t new_x,uint16_t new_y)
{
	//currentProject->tileMapC->mapSizeW and currentProject->tileMapC->mapSizeH hold old map size
	if (new_x == currentProject->tileMapC->mapSizeW && new_y == currentProject->tileMapC->mapSizeH)
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
			if (x < currentProject->tileMapC->mapSizeW && y < currentProject->tileMapC->mapSizeH){
				uint32_t sel_map_old=((y*currentProject->tileMapC->mapSizeW)+x)*4;
				/*temp[sel_map]=currentProject->tileMapC->tileMapDat[sel_map_old];
				temp[sel_map+1]=currentProject->tileMapC->tileMapDat[sel_map_old+1];
				temp[sel_map+2]=currentProject->tileMapC->tileMapDat[sel_map_old+2];
				temp[sel_map+3]=currentProject->tileMapC->tileMapDat[sel_map_old+3];*/
				memcpy(&temp[sel_map],&currentProject->tileMapC->tileMapDat[sel_map_old],4);
			}else{
				/*temp[sel_map]=0;
				temp[sel_map+1]=0;
				temp[sel_map+2]=0;
				temp[sel_map+3]=0;*/
				memset(&temp[sel_map],0,4);
			}
		}
	}
	currentProject->tileMapC->tileMapDat = (uint8_t *)realloc(currentProject->tileMapC->tileMapDat,(new_x*new_y)*4);
	if (currentProject->tileMapC->tileMapDat == 0){
		show_realloc_error((new_x*new_y)*4)
		return;
	}
	/*for (uint32_t c=0;c<(new_x*new_y)*4;c++)
	{
		currentProject->tileMapC->tileMapDat[c]=temp[c];
	}*/
	memcpy(currentProject->tileMapC->tileMapDat,temp,(new_x*new_y)*4);
	free(temp);
	currentProject->tileMapC->mapSizeW=new_x;
	//calulate new scroll size
	uint16_t old_scroll=window->map_x_scroll->value();
	uint8_t tile_size_placer=window->place_tile_size->value();
	int32_t map_scroll=((tile_size_placer*8)*currentProject->tileMapC->mapSizeW)-map_off_x;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
		map_scroll=0;
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
		old_scroll=map_scroll;
	window->map_x_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));//the reason for adding map_scroll/2 to map_scroll is because without it the user will not be able to scroll the tilemap all the way
	currentProject->tileMapC->mapSizeH=new_y;
	old_scroll=window->map_y_scroll->value();
	tile_size_placer=window->place_tile_size->value();
	map_scroll=((tile_size_placer*8)*currentProject->tileMapC->mapSizeH)-map_off_y;//size of all offscreen tiles in pixels
	//map_scroll-=(tile_size_placer*8);
	if (map_scroll < 0)
		map_scroll=0;
	map_scroll/=tile_size_placer*8;//now size of all tiles
	//cout << "tiles off screen: " << map_scroll << endl;
	if (old_scroll > map_scroll)
		old_scroll=map_scroll;
	window->map_y_scroll->value(old_scroll,(map_scroll/2),0,map_scroll+(map_scroll/2));
}
//bool load_file_generic(const char * the_tile="Pick a file",bool save_file=false)
bool load_file_generic(const char * the_tile,bool save_file)
{	
	// Create native chooser
	Fl_Native_File_Chooser native;
	native.title(the_tile);
	if (save_file == false)
		native.type(Fl_Native_File_Chooser::BROWSE_FILE);
	else
		native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
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
