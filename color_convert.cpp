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
uint8_t nespaltab_r[64];
uint8_t nespaltab_g[64];
uint8_t nespaltab_b[64];
uint8_t nespaltab_r_alt[64];
uint8_t nespaltab_g_alt[64];
uint8_t nespaltab_b_alt[64];

void rgbToEntry(unsigned r,unsigned g,unsigned b,unsigned ent){
	unsigned maxent=currentProject->colorCnt+currentProject->colorCntalt;
	if(ent>maxent){
		fl_alert("Attempted access for color %d but there is only %d colors",ent,maxent);
		return;
	}
	switch(currentProject->gameSystem){
		case sega_genesis:
			{uint16_t temp=to_sega_genesis_colorRGB(r,g,b,ent);
			ent*=2;
			currentProject->palDat[ent]=temp>>8;
			currentProject->palDat[ent+1]=temp&255;}
		break;
		case NES:
			currentProject->palDat[ent]=to_nes_color_rgb(r,g,b);
			updateRGBindex(ent);
		break;
		default:
			show_default_error
	}
}

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
void rgbToHsl(double r,double g,double b,double * hh,double * ss,double * ll){
	r /= 255.0;
	g /= 255.0;
	b /= 255.0;
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
void updateRGBindex(unsigned index){
	switch(currentProject->gameSystem){
		case sega_genesis:
			{uint16_t*ptr=(uint16_t*)currentProject->palDat+index;
			currentProject->rgbPal[index*3+2]=palTab[((*ptr>>1)&7)+palTypeGen];//Blue note that bit shifting is different due to little endian
			currentProject->rgbPal[index*3+1]=palTab[((*ptr>>13)&7)+palTypeGen];//Green
			currentProject->rgbPal[index*3]=palTab[((*ptr>>9)&7)+palTypeGen];//Red
			}
		break;
		case NES:
			{uint32_t rgb_out=MakeRGBcolor(currentProject->palDat[index]);
			currentProject->rgbPal[index*3+2]=rgb_out&255;//blue
			currentProject->rgbPal[index*3+1]=(rgb_out>>8)&255;//green
			currentProject->rgbPal[index*3]=(rgb_out>>16)&255;//red
			}
		break;
	}
}
static inline uint32_t sq(uint32_t x){
	return x*x;
}
uint8_t to_nes_color_rgb(uint8_t red,uint8_t green,uint8_t blue){
	//this function does not set any values to global palette it is done in other functions
	uint32_t minerrori =(255*255) +(255*255) +(255*255) +1;
	double minerrord=100000.0;
	unsigned bestcolor=0;
	for (unsigned temp=0;temp<64;++temp){
		switch(nearestAlg){
		case 0:
			{double distance=ciede2000rgb(red,green,blue,nespaltab_r[temp],nespaltab_g[temp],nespaltab_b[temp]);
			if (distance <= minerrord){
				minerrord = distance;
				bestcolor = temp;
			}}
		break;
		case 1:
			{uint32_t distance=ColourDistance(red,green,blue,nespaltab_r[temp],nespaltab_g[temp],nespaltab_b[temp]);
			if (distance <= minerrori){
				minerrori = distance;
				bestcolor = temp;
			}}
		break;
		default:
			{uint32_t distance=sq(nespaltab_r[temp]-red)+sq(nespaltab_g[temp]-green)+sq(nespaltab_b[temp]-blue);
			if (distance <= minerrori){
				minerrori = distance;
				bestcolor = temp;
			}}
		}
	}
	return bestcolor;
}
uint8_t to_nes_color(uint8_t pal_index){
	//this function does not set any values to global palette it is done in other functions
	pal_index*=3;
	return to_nes_color_rgb(currentProject->rgbPal[pal_index],currentProject->rgbPal[pal_index+1],currentProject->rgbPal[pal_index+2]);
}
uint8_t toNesChan(uint8_t ri,uint8_t gi,uint8_t bi,uint8_t chan){
	uint32_t rgb_out=toNesRgb(ri,gi,bi);
	uint8_t b=rgb_out&255;
	uint8_t g=(rgb_out>>8)&255;
	uint8_t r=(rgb_out>>16)&255;
	switch (chan){
		case 0:
			return r;
		break;
		case 1:
			return g;
		break;
		case 2:
			return b;
		break;
	}
	return 0;
}
uint16_t to_sega_genesis_colorRGB(uint8_t r,uint8_t g,uint8_t b,uint16_t pal_index){
	//note this function only set the new rgb colors not the outputed sega genesis palette format
	pal_index*=3;
	r=nearest_color_index(r,0);
	g=nearest_color_index(g,0);
	b=nearest_color_index(b,0);
	currentProject->rgbPal[pal_index]=palTab[r];
	currentProject->rgbPal[pal_index+1]=palTab[g];
	currentProject->rgbPal[pal_index+2]=palTab[b];
	//bgr format
	return (r<<1)|(g<<5)|(b<<9);
}
uint16_t to_sega_genesis_color(uint16_t pal_index){
	//note this function only set the new rgb colors not the outputed sega genesis palette format
	pal_index*=3;
	uint8_t r,g,b;
	r=nearest_color_index(currentProject->rgbPal[pal_index]);
	g=nearest_color_index(currentProject->rgbPal[pal_index+1]);
	b=nearest_color_index(currentProject->rgbPal[pal_index+2]);
	currentProject->rgbPal[pal_index]=palTab[r];
	currentProject->rgbPal[pal_index+1]=palTab[g];
	currentProject->rgbPal[pal_index+2]=palTab[b];
	//bgr format
	return ((r-palTypeGen)<<1)|((g-palTypeGen)<<5)|((b-palTypeGen)<<9);
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
	printf("\n");
	return colors_amount/3;
}
void updateNesTab(unsigned emps,bool alt){
	uint32_t rgb_out;
	if(alt){
		for(unsigned temp=0;temp<64;++temp){
			rgb_out=MakeRGBcolor(temp|emps);
			nespaltab_r_alt[temp]=(rgb_out>>16)&255;//red
			nespaltab_g_alt[temp]=(rgb_out>>8)&255;//green
			nespaltab_b_alt[temp]=rgb_out&255;//blue
		}
	}else{
		for(unsigned temp=0;temp<64;++temp){
			rgb_out=MakeRGBcolor(temp|emps);
			nespaltab_r[temp]=(rgb_out>>16)&255;//red
			nespaltab_g[temp]=(rgb_out>>8)&255;//green
			nespaltab_b[temp]=rgb_out&255;//blue
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
		rgb_out=MakeRGBcolor(currentProject->palDat[c/3]|emps);
		currentProject->rgbPal[c]=(rgb_out>>16)&255;//red
		currentProject->rgbPal[c+1]=(rgb_out>>8)&255;//green
		currentProject->rgbPal[c+2]=rgb_out&255;//blue
	}
	for(unsigned c=48;c<96;c+=3){
		rgb_out=MakeRGBcolor(currentProject->palDat[c/3]|empsSprite);
		currentProject->rgbPal[c]=(rgb_out>>16)&255;//red
		currentProject->rgbPal[c+1]=(rgb_out>>8)&255;//green
		currentProject->rgbPal[c+2]=rgb_out&255;//blue
	}
	window->redraw();
}
