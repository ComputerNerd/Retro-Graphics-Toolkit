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
#include "global.h"
#include "palette.h"
#include "color_compare.h"
uint8_t nespaltab_r[64];
uint8_t nespaltab_g[64];
uint8_t nespaltab_b[64];
uint8_t to_nes_color_rgb(uint8_t red,uint8_t green,uint8_t blue){
	//this function does not set any values to global palette it is done in other functions
	int min_error =(255*255) +(255*255) +(255*255) +1;
	uint8_t bestcolor=0;
	for (uint8_t a=0;a<16;a++){
		for (uint8_t c=0;c<4;c++){
			uint8_t temp=a|(c<<4);
			int rdiff= (int)nespaltab_r[temp] - (int)red;
			int gdiff= (int)nespaltab_g[temp] - (int)green;
			int bdiff= (int)nespaltab_b[temp] - (int)blue;
			int dist = (rdiff*rdiff) + (gdiff*gdiff) + (bdiff*bdiff);
			if (dist <= min_error){
				min_error = dist;
				bestcolor=a|(c<<4);
			}
		}
	}
	return bestcolor;
}
uint8_t to_nes_color(uint8_t pal_index){
	//this function does not set any values to global palette it is done in other functions
	pal_index*=3;
	int min_error =(255*255) +(255*255) +(255*255) +1;
	uint8_t bestcolor=0;
	for (uint8_t a=0;a<16;++a){
		for (uint8_t c=0;c<4;++c){
			uint8_t temp=a|(c<<4);
			int rdiff= (int)nespaltab_r[temp] - (int)currentProject->rgbPal[pal_index];
			int gdiff= (int)nespaltab_g[temp] - (int)currentProject->rgbPal[pal_index+1];
			int bdiff= (int)nespaltab_b[temp] - (int)currentProject->rgbPal[pal_index+2];
			int dist = (rdiff*rdiff) + (gdiff*gdiff) + (bdiff*bdiff);
			if (dist <= min_error){
				min_error = dist;
				bestcolor=a|(c<<4);
			}
		}
	}
	return bestcolor;
}
static inline int squarei(int x){
	return x*x;
}
uint32_t toNesRgb(uint8_t ri,uint8_t gi,uint8_t bi){
	uint8_t bestcolor=0;
	if(nearestAlg){
		uint32_t min_error=(255*255)+(255*255)+(255*255)+1;
		for (uint8_t a=0;a<16;a++){//hue
			for (uint8_t c=0;c<4;c++){//value
				uint8_t temp=a|(c<<4);
				uint32_t dist = squarei(nespaltab_r[temp]-ri)+squarei(nespaltab_g[temp]-gi)+squarei(nespaltab_b[temp]-bi);
				if (dist < min_error){
					min_error = dist;
					bestcolor=a|(c<<4);
				}
			}
		}
	}else{
		double min_error=9001.0;
		for (uint8_t a=0;a<16;a++){//hue
			for (uint8_t c=0;c<4;c++){//value
				uint8_t temp=a|(c<<4);
				double dist = ciede2000rgb(nespaltab_r[temp],nespaltab_g[temp],nespaltab_b[temp],ri,gi,bi);
				if (dist < min_error){
					min_error = dist;
					bestcolor=a|(c<<4);
				}
			}
		}
	}
	return MakeRGBcolor(bestcolor);
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
uint32_t count_colors(uint8_t * image_ptr,uint32_t w,uint32_t h,uint8_t *colors_found,bool useAlpha=false){
	/*!
	Scans for colors in an image stops at over 256 as if there is an excess of 256 colors there is no reason to countinue
	*/
	//memset(colors_found,0,w*h*3);
	uint32_t colors_amount=3;
	colors_found[0]=*image_ptr++;
	colors_found[1]=*image_ptr++;
	colors_found[2]=*image_ptr++;
	if (useAlpha)
		image_ptr++;
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
			if (new_col==true){
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
void updateNesTab(uint8_t emps){
	uint8_t c,v;
	uint32_t rgb_out;
	for(c=0;c<16;++c){
		for(v=0;v<4;++v){
			uint8_t temp=c|(v<<4);
			rgb_out=MakeRGBcolor(temp|emps);
			nespaltab_r[temp]=(rgb_out>>16)&255;//red
			nespaltab_g[temp]=(rgb_out>>8)&255;//green
			nespaltab_b[temp]=rgb_out&255;//blue
		}
	}
}
void update_emphesis(Fl_Widget*,void*){
	uint8_t emps;
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
	}
	/*76543210
	  ||||||||
	  ||||++++- Hue (phase)
	  ||++----- Value (voltage)
	  ++------- Unimplemented, reads back as 0*/
	emps<<=6;
	uint8_t c,v;
	uint32_t rgb_out;
	updateNesTab(emps);
	for (c=0;c<48;c+=3){
		rgb_out=MakeRGBcolor(currentProject->palDat[c/3]|emps);
		currentProject->rgbPal[c]=(rgb_out>>16)&255;//red
		currentProject->rgbPal[c+1]=(rgb_out>>8)&255;//green
		currentProject->rgbPal[c+2]=rgb_out&255;//blue
	}
	window->redraw();
}
