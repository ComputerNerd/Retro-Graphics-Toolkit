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
#include "global.h"
#include "color_compare.h"
#include "color_convert.h"
#include "system.h"
#include "palette.h"
#include "nearestColor.h"
uint8_t nespaltab[64*3];
uint8_t nespaltab_alt[64*3];


uint8_t nearest_color_index(uint8_t val,unsigned startindex){
	int32_t distanceSquared, minDistanceSquared, bestIndex = 0;
	minDistanceSquared = 255*255 + 1;
	if (currentProject->gameSystem!=sega_genesis){
		fl_alert("This function is for use with sega genesis/mega drive only");
		return 0;
	}
	for(unsigned i=startindex;i<8+startindex;++i){
		int32_t Rdiff = (int) val - (int)palTab[i];
		distanceSquared = Rdiff*Rdiff;
		if(distanceSquared<minDistanceSquared){
			minDistanceSquared = distanceSquared;
			bestIndex = i;
		}
	}
	return bestIndex;
}
uint8_t nearest_color_index(uint8_t val){
	return nearest_color_index(val,palTypeGen);
}
static double max3(double a,double b,double c){
	if ((a > b) && (a > c))
		return a;
	if (b > c)
		return b;
	return c;
}
static double min3(double a,double b,double c){
	if ((a < b) && (a < c))
		return a;
	if (b < c)
		return b;
	return c;
}
/**
 * Converts an RGB color value to HSL. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes r, g, and b are contained in the set [0, 255] and
 * returns h, s, and l in the set [0, 1].
 *
 * @param   Number  r       The red color value
 * @param   Number  g       The green color value
 * @param   Number  b       The blue color value
 * @return  Array           The HSL representation
 */
void rgbToHsl255(unsigned r,unsigned g,unsigned b,double * hh,double * ss,double * ll){
	double rd=double(r)/255.0,gd=double(g)/255.0,bd=double(b)/255.0;
	rgbToHsl(rd,gd,bd,hh,ss,ll);
}
void rgbToHsl(double r,double g,double b,double * hh,double * ss,double * ll){
	double max = max3(r, g, b);
	double min = min3(r, g, b);
	double h, s, l = (max + min) / 2.0;

	if(max == min)
		h = s = 0.0; // achromatic
	else{
		double d = max - min;
		s = l > 0.5 ? d / (2.0 - max - min) : d / (max + min);
		/*if (max == r)
			h = (g - b) / d + (g < b ? 6 : 0);
		else if (max == g)
			h = (b - r) / d + 2.0;
		else
			h = (r - g) / d + 4.0;
		h /= 6.0;*/

		//From: http://easyrgb.com/index.php?X=MATH&H=18#text18
		double del_R = ((( max - r )/6.0) + (d/2.0)) / d;
		double del_G = ((( max - g )/6.0) + (d/2.0)) / d;
		double del_B = ((( max - b )/6.0) + (d/2.0)) / d;

		if      (r == max ) h = del_B - del_G;
		else if (g == max ) h = (1.0/3.0) + del_R - del_B;
		else if (b == max ) h = (2.0/3.0) + del_G - del_R;

		if (h < 0.0) h += 1.0;
		if (h > 1.0) h -= 1.0;
	}
	if(h>1.0)
		printf("Warning %f\n",h);
	*hh=h;
	*ll=l;
	*ss=s;
}
static inline uint32_t sq(uint32_t x){
	return x*x;
}
uint32_t count_colors(uint8_t * image_ptr,uint32_t w,uint32_t h,uint8_t *colors_found,bool useAlpha){
	/*!
	Scans for colors in an image stops at over 256 as if there is an excess of 256 colors there is no reason to countinue
	*/
	uint32_t colors_amount=3;
	colors_found[0]=*image_ptr++;
	colors_found[1]=*image_ptr++;
	colors_found[2]=*image_ptr++;
	if (useAlpha)
		++image_ptr;
	uint8_t start=1;
	uint32_t y;
	for (y=0;y<h;++y){
		for (uint32_t x=start;x<w;++x){
			start=0;
			uint8_t r,g,b;
			r=*image_ptr++;
			g=*image_ptr++;
			b=*image_ptr++;
			if (useAlpha)
				image_ptr++;
			bool new_col=true;
			for (uint32_t c=0;c<colors_amount;c+=3){
				if (r == colors_found[c] && g == colors_found[c+1] && b == colors_found[c+2]){
					new_col=false;
					break;//exit loop
				}
			}
			if (new_col){
				colors_found[colors_amount]=r;
				colors_found[colors_amount+1]=g;
				colors_found[colors_amount+2]=b;
				colors_amount+=3;
			}
			if (colors_amount >= 765){
				printf("\nOver 255 colors timing out no need for operation to countinue.\n");
				return colors_amount/3;
			}
		}
			//update progress
			//printf("counting colors %% %f Colors Found: %d\r",((float)y/(float)h)*100.0,colors_amount/3);
	}
	putchar('\n');
	return colors_amount/3;
}
void updateNesTab(unsigned emps,bool alt){
	uint32_t rgb_out;
	if(alt){
		for(unsigned temp=0;temp<64;++temp){
			rgb_out=MakeRGBcolor(temp|emps);
			nespaltab_alt[temp*3]=(rgb_out>>16)&255;//red
			nespaltab_alt[temp*3+1]=(rgb_out>>8)&255;//green
			nespaltab_alt[temp*3+2]=rgb_out&255;//blue
		}
	}else{
		for(unsigned temp=0;temp<64;++temp){
			rgb_out=MakeRGBcolor(temp|emps);
			nespaltab[temp*3]=(rgb_out>>16)&255;//red
			nespaltab[temp*3+1]=(rgb_out>>8)&255;//green
			nespaltab[temp*3+2]=rgb_out&255;//blue
		}
	}
}
void update_emphesis(Fl_Widget*,void*){
	unsigned emps;
	unsigned empsSprite;
	switch (mode_editor){
		case pal_edit:
			emps=palEdit.pal_b->value();
		break;
		case tile_edit:
			emps=tileEdit_pal.pal_b->value();
		break;
		case tile_place:
			emps=tileMap_pal.pal_b->value();
		break;
		case spriteEditor:
			empsSprite=spritePal.pal_b->value();
		break;
	}
	/*76543210
	  ||||||||
	  ||||++++- Hue (phase)
	  ||++----- Value (voltage)
	  ++------- Unimplemented, reads back as 0*/
	emps<<=6;
	uint32_t rgb_out;
	updateNesTab(emps,false);
	updateNesTab(empsSprite,true);
	for(unsigned c=0;c<48;c+=3){
		rgb_out=MakeRGBcolor(currentProject->pal->palDat[c/3]|emps);
		currentProject->pal->rgbPal[c]=(rgb_out>>16)&255;//red
		currentProject->pal->rgbPal[c+1]=(rgb_out>>8)&255;//green
		currentProject->pal->rgbPal[c+2]=rgb_out&255;//blue
	}
	for(unsigned c=48;c<96;c+=3){
		rgb_out=MakeRGBcolor(currentProject->pal->palDat[c/3]|empsSprite);
		currentProject->pal->rgbPal[c]=(rgb_out>>16)&255;//red
		currentProject->pal->rgbPal[c+1]=(rgb_out>>8)&255;//green
		currentProject->pal->rgbPal[c+2]=rgb_out&255;//blue
	}
	window->redraw();
}
