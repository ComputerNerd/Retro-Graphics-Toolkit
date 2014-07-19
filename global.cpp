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
#include "includes.h"
#include "class_global.h"
#include "errorMsg.h"
#include "dither.h"
#include "class_tiles.h"
#include "color_convert.h"
#include "color_compare.h"
//fltk widget realted stuff
Fl_Group * shadow_highlight_switch;
//bools used to toggle stuff
bool show_grid;
bool G_hflip[2];
bool G_vflip[2];
bool G_highlow_p[2];
bool show_grid_placer;
//moveable offsets
uint8_t tile_zoom_edit;
uint8_t truecolor_temp[4];/*!< This stores the rgba data selected with the truecolor sliders*/
std::string the_file;//this is for tempory use only
uint8_t mode_editor;//this is used to determin which thing to draw
uint8_t ditherAlg;
#define PIf 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989f
bool showTrueColor;
bool rowSolo;
bool tileEditModePlace_G;
uint32_t selTileE_G[2];
uint8_t nearestAlg=1;
uint8_t nearest_color_index(uint8_t val){
	//returns closest value
	//palette_muliplier
	uint8_t i;
    int32_t distanceSquared, minDistanceSquared, bestIndex = 0;
    minDistanceSquared = 255*255 + 1;
	if (currentProject->gameSystem!=sega_genesis){
		fl_alert("This function is for use with sega genesis/mega drive only");
		return 0;
	}
    for (i=palTypeGen; i<8+palTypeGen;++i){
        int32_t Rdiff = (int) val - (int)palTab[i];
        distanceSquared = Rdiff*Rdiff;
        if (distanceSquared < minDistanceSquared){
            minDistanceSquared = distanceSquared;
            bestIndex = i;
        }
    }
    return bestIndex;
}
bool saveBinAsText(void * ptr,size_t sizeBin,FILE * myfile){
	/*!
	This function saves binary data as plain text useful for c headers each byte is seperated by a comma
	Returns True on sucess false on error
	*/
	uint8_t * dat=(uint8_t *)ptr;
	char str[16];
	for (size_t x=0;x<sizeBin-1;++x){
		sprintf(str,"%d,",*dat);
		if (fputs(str,myfile)==0)
			return false;
		if ((x&63)==63){
			if (fputc('\n',myfile)==0)
				return false;
		}
		++dat;
	}
	sprintf(str,"%d",*dat);
	if (fputs(str,myfile)==0)
		return false;
	return true;
}
void tileToTrueCol(uint8_t * input,uint8_t * output,uint8_t row,bool useAlpha,bool alphaZero){
	switch (currentProject->gameSystem){
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
					if(useAlpha){
						if(alphaZero){
							if(temp_1)
								*output++=255;
							else
								*output++=0;
						}else
							*output++=255;
					}
					*output++=currentProject->rgbPal[row+(temp_2*3)];
					*output++=currentProject->rgbPal[row+(temp_2*3)+1];
					*output++=currentProject->rgbPal[row+(temp_2*3)+2];
					if(useAlpha){
						if(alphaZero){
							if(temp_2)
								*output++=255;
							else
								*output++=0;
						}else
							*output++=255;
					}
				}
			}
		break;
		case NES:
			row*=12;
			for (uint8_t y=0;y<8;++y){
				for (int8_t x=7;x>=0;--x){
					uint8_t temp;
					temp=(input[y]>>x)&1;
					temp|=((input[y+8]>>x)&1)<<1;
					*output++=currentProject->rgbPal[row+(temp*3)];
					*output++=currentProject->rgbPal[row+(temp*3)+1];
					*output++=currentProject->rgbPal[row+(temp*3)+2];
					if(useAlpha){
						if(alphaZero){
							if(temp)
								*output++=255;
							else
								*output++=0;
						}else
							*output++=255;
					}
				}
			}
		break;
	}
	
}
bool verify_str_number_only(char * str){
/*!
Fltk provides an input text box that makes it easy for the user to type text however as a side effect they can accidently enter non number characters that may be handled weird by atoi()
this function address that issue by error checking the string and it also gives the user feedback so they are aware that the input box takes only numbers
this function returns true when the string contains only numbers 0-9 and false when there is other stuff
it will also allow the use of the - symbol as negative
*/
	while(*str++){
		if (*str != 0 && *str != '-'){
			if (*str < '0'){
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
			if (*str > '9'){
				fl_alert("Please enter only numbers in decimal format\nCharacter entered %c",*str);
				return false;
			}
		}
	}
	return true;
}
static inline int wave(int p,int color){
	return (color+p+8)%12 < 6;
}
static inline float gammafix(float f,float gamma){
	return f < 0.f ? 0.f : std::pow(f, 2.2f / gamma);
}
static inline int clamp(int v){
	return v<0 ? 0 : v>255 ? 255 : v;
}
//uint32_t MakeRGBcolor(uint32_t pixel,float saturation = 1.1f, float hue_tweak = 0.0f,float contrast = 1.0f, float brightness = 1.0f,float gamma = 2.2f)
uint32_t MakeRGBcolor(uint32_t pixel,float saturation, float hue_tweak,float contrast, float brightness,float gamma){
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
static inline uint32_t sq(uint32_t x){
	return x*x;
}
uint8_t find_near_color_from_row_rgb(uint8_t row,uint8_t r,uint8_t g,uint8_t b){
	unsigned i;
	int bestIndex = 0;
	unsigned max_rgb=palEdit.perRow*3;
	row*=max_rgb;
	uint32_t minerrori=(255*255)+(255*255)+(255*255)+1;
	double minerrord=100000.0;
	for (i=row; i<max_rgb+row; i+=3){
		switch(nearestAlg){
			case 0:
				{double distance=ciede2000rgb(r,g,b,currentProject->rgbPal[i],currentProject->rgbPal[i+1],currentProject->rgbPal[i+2]);
				if (distance <= minerrord){
					if (currentProject->palType[i/3]!=2){
						minerrord = distance;
						bestIndex = i;
					}
				}}
			break;
			case 1:
				{double distance=ColourDistance(r,g,b,currentProject->rgbPal[i],currentProject->rgbPal[i+1],currentProject->rgbPal[i+2]);
				if (distance <= minerrord){
					if (currentProject->palType[i/3]!=2){
						minerrord = distance;
						bestIndex = i;
					}
				}}
			break;
			default:
				{uint32_t distance=sq(currentProject->rgbPal[i]-r)+sq(currentProject->rgbPal[i+1]-g)+sq(currentProject->rgbPal[i+2]-b);
				if (distance <= minerrori){
					if (currentProject->palType[i/3]!=2){
						minerrori = distance;
						bestIndex = i;
					}
				}}
		}
	}
	return bestIndex;
}
uint8_t find_near_color_from_row(uint8_t row,uint8_t r,uint8_t g,uint8_t b){
	return (find_near_color_from_row_rgb(row,r,g,b)/3)-(row*palEdit.perRow);
}
uint32_t cal_offset_truecolor(uint16_t x,uint16_t y,uint16_t rgb,uint32_t tile){
	/*!<
	cal_offset_truecolor is made to help when accesing a true color tile array
	an example of it would be
	red_temp=truecolor_data[cal_offset_truecolor(0,0,0,0)]//get the red pixel at pixel 0,0 at tile 0
	green_temp=truecolor_data[cal_offset_truecolor(0,0,1,0)]//get the green pixel at pixel 0,0 at tile 0
	blue_temp=truecolor_data[cal_offset_truecolor(0,0,2,0)]//get the blue pixel at pixel 0,0 at tile 0
	*/
	return (x*4)+(y*32)+rgb+(tile*256);
}
//bool load_file_generic(const char * the_tile="Pick a file",bool save_file=false)
bool load_file_generic(const char * the_tile,bool save_file){
	// Create native chooser
	Fl_Native_File_Chooser native;
	native.title(the_tile);
	if (save_file == false)
		native.type(Fl_Native_File_Chooser::BROWSE_FILE);
	else
		native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	// Show native chooser
	switch (native.show())
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
