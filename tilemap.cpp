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
/* Stuff related to tilemap operations goes here*/
#include "global.h"
#include "quant.h"
#include "color_compare.h"
#include "color_convert.h"
#include "dither.h"
#include "spatial_color_quant.h"
#include "NEUQUANT.H"
#include "palette.h"
#include "wu.h"
#include "callbacksprites.h"
#include "undo.h"
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
static void rgbToHls(double r,double g,double b,double * hh,double * ll,double * ss){
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
static void addHist(uint32_t cur_tile,int type,uint32_t*hist,unsigned sz){
	double szz=(double)sz;
	uint8_t * truePtr=&currentProject->tileC->truetDat[cur_tile*256];
	double h,l,s;
	for(unsigned z=0;z<256;z+=4){
		rgbToHls(truePtr[0],truePtr[1],truePtr[2],&h,&l,&s);
		truePtr+=4;
		switch(type){
			case 0:
				++hist[unsigned(h*szz)];
			break;
			case 1:
				++hist[unsigned(l*szz)];
			break;
			case 2:
				++hist[unsigned(s*szz)];
			break;
			case 3:
				++hist[unsigned(h*s*szz)];
			break;
			case 4:
				++hist[unsigned(h*l*szz)];
			break;
			case 5:
				++hist[unsigned(s*l*szz)];
			break;
			case 6:
				++hist[unsigned((h+s)*szz)];
			break;
			case 7:
				++hist[unsigned((h+l)*szz)];
			break;
			case 8:
				++hist[unsigned((s+l)*szz)];
			break;
		}
	}
}
static double getHH(uint32_t cur_tile,int type){
	double hh=0.0;
	uint8_t * truePtr=&currentProject->tileC->truetDat[cur_tile*256];
	double h,l,s;
	for(unsigned z=0;z<256;z+=4){
		rgbToHls(truePtr[0],truePtr[1],truePtr[2],&h,&l,&s);
		truePtr+=4;
		switch(type){
			case 0:
				hh+=h;
			break;
			case 1:
				hh+=l;
			break;
			case 2:
				hh+=s;
			break;
			case 3:
				hh+=h*s;
			break;
			case 4:
				hh+=h*l;
			break;
			case 5:
				hh+=s*l;
			break;
			case 6:
				hh+=h+s;
			break;
			case 7:
				hh+=h+l;
			break;
			case 8:
				hh+=s+l;
			break;
		}
	}
	return hh;
}
void tileMap::pickRow(uint8_t amount){
	int type=MenuPopup("Pick tile row based on...","Please pick what most defines the image",9,"Hue","Brightness","Saturation","Hue*satuaration","Hue*Brightness","Brightness*saturation","Hue+satuaration","Hue+Brightness","Brightness+saturation");
	if(type<0)
		return;
	int method=MenuPopup("Select a method","This depends on the image",3,"Average","Histogram section with most occurances","Histogram peak");
	if(method<0)
		return;
	pushTilemapAll(true);
	double divide=(double)amount;//convert to double
	uint32_t x,y;
	double maxPal=divide;
	double divBy;
	unsigned addBy;
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
		divBy=256.0;//8*8*2*2
		addBy=2;
	}else{
		divBy=64.0;//8*8
		addBy=1;
	}
	uint32_t*hist,sz,stretch,maxh,minh;
	if(method){
		stretch=fl_ask("Stretch histogram?");
		if(type>=6)
			sz=2000;
		else
			sz=1000;
		hist=(uint32_t*)calloc(sz,sizeof(uint32_t));
		for (y=0;y<mapSizeHA;y+=addBy){
			for (x=0;x<mapSizeW;x+=addBy){
				if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
					addHist(get_tile(x,y),type,hist,sz);
					addHist(get_tile(x+1,y),type,hist,sz);
					addHist(get_tile(x,y+1),type,hist,sz);
					addHist(get_tile(x+1,y+1),type,hist,sz);
				}else
					addHist(get_tile(x,y),type,hist,sz);
			}
		}
		if(stretch){
			uint32_t*histp=hist;
			while(!(*histp++));//Atleast one entry in the array will contain a nonzero value
			minh=(histp-hist);
			--minh;
			histp=hist+sz-1;
			while(!(*histp--));
			maxh=(histp-hist);
			maxh+=2;
			printf("Histogram stretched to [%d,%d)\n",minh,maxh);
		}else{
			minh=0;
			maxh=sz;
		}
	}
	for (y=0;y<mapSizeHA;y+=addBy){
		for (x=0;x<mapSizeW;x+=addBy){
			if(method){
				std::fill(hist,hist+sz,0);
				if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
					addHist(get_tile(x,y),type,hist,sz);
					addHist(get_tile(x+1,y),type,hist,sz);
					addHist(get_tile(x,y+1),type,hist,sz);
					addHist(get_tile(x+1,y+1),type,hist,sz);
				}else
					addHist(get_tile(x,y),type,hist,sz);
				//Find min and max of the histogram
				uint32_t*histp;
				unsigned divH=(maxh-minh)/amount;
				if(!divH)
					divH=1;
				if(method==2){
					uint32_t maxv,i,ent;
					histp=hist+minh;
					for(i=maxv=ent=0;i<maxh;++i){
						if(maxv<*histp){
							maxv=*histp;
							ent=i;
						}
						++histp;
					}
					set_pal_row(x,y,ent/divH);
				}else if(method==1){
					uint32_t*sums=(uint32_t*)alloca(amount*sizeof(uint32_t)),i,maxv,ent;
					histp=hist+minh;
					std::fill(sums,sums+amount,0);
					for(i=ent=0;i<maxh;++i)
						sums[i/divH]+=*histp++;
					maxv=sums[0];
					for(i=1;i<amount;++i){
						if(maxv<sums[i]){
							maxv=sums[i];
							ent=i;
						}
					}
					set_pal_row(x,y,ent);
				}
			}else{
				double hh;
				if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2)){
					hh=getHH(get_tile(x,y),type);
					hh+=getHH(get_tile(x+1,y),type);
					hh+=getHH(get_tile(x,y+1),type);
					hh+=getHH(get_tile(x+1,y+1),type);
				}else
					hh=getHH(get_tile(x,y),type);
				hh/=divBy/divide;
				if (hh >= maxPal){
					printf("hh >= %f %f %d\n",maxPal,hh,(int)hh);
					hh=divide-0.5;
				}
				set_pal_row(x,y,hh);
			}
		}
	}
	if(method)
		free(hist);
}
void tileMap::allRowZero(void){
	uint32_t x,y;
	for (y=0;y<mapSizeHA;++y){
		for (x=0;x<mapSizeW;++x)
			set_pal_row(x,y,0);
	}
}
static inline uint8_t pick4Delta(double * d){
	if ((d[0] <= d[1]) && (d[0] <= d[2]) && (d[0] <= d[3]))
		return 0;
	if ((d[1] <= d[2]) && (d[1] <= d[3]))
		return 1;
	if (d[2] <= d[3])
		return 2;
	return 3;
}
static inline uint8_t pick4Deltai(uint32_t * d){
	if ((d[0] <= d[1]) && (d[0] <= d[2]) && (d[0] <= d[3]))
		return 0;
	if ((d[1] <= d[2]) && (d[1] <= d[3]))
		return 1;
	if (d[2] <= d[3])
		return 2;
	return 3;
}
static inline uint32_t sqri(int x){
	return x*x;
}
static inline double pickIt(double h,double l,double s,unsigned type){
	switch(type){
		case 0:
			return h;
		break;
		case 1:
			return l;
		break;
		case 2:
			return s;
		break;
	}
}
typedef std::pair<double,int> HLSpair;
bool comparatorHLS(const HLSpair& l,const HLSpair& r)
   { return l.first < r.first; }
void tileMap::pickRowDelta(bool showProgress,Fl_Progress *progress){
	int alg=MenuPopup("Select picking algorithm","Pick which method you think works better for this image.",6,"ciede2000","Weighted","Mean squared error","Hue difference","Saturation difference","Lightness difference");
	if(alg<0)
		return;
	pushTilemapAll(true);
	pushTilesAll(tTypeTile);
	if(fl_ask("Would you like the palette to be ordered by hue or light or saturation")){
		unsigned type=fl_choice("What do you want it ordered by","Hue","Light","Saturation");
		pushPaletteAll();
		HLSpair* MapHLS=new HLSpair[palEdit.perRow*4];//Remember to change if there is a palete with a different amount than 4 rows
		for(unsigned x=0;x<palEdit.perRow*3*4;x+=3){
			double h,l,s;
			rgbToHls(currentProject->rgbPal[x],currentProject->rgbPal[x+1],currentProject->rgbPal[x+2],&h,&l,&s);
			MapHLS[x/3].first=pickIt(h,l,s,type);
			MapHLS[x/3].second=x/3;
		}
		std::sort(MapHLS,MapHLS+(palEdit.perRow*4),comparatorHLS);
		unsigned eSize;
		switch(currentProject->gameSystem){
			case sega_genesis:
				eSize=2;
			break;
			case NES:
				eSize=1;
			break;
		}
		uint8_t* newPal=(uint8_t*)alloca(palEdit.perRow*4*eSize);
		uint8_t* newPalRgb=(uint8_t*)alloca(palEdit.perRow*4*eSize*3);
		uint8_t* newPalType=(uint8_t*)alloca(palEdit.perRow*4);
		for(unsigned x=0;x<palEdit.perRow*4;++x){
			printf("%d with %d\n",MapHLS[x].second,x);
			memcpy(newPal+(x*eSize),currentProject->palDat+(MapHLS[x].second*eSize),eSize);
			memcpy(newPalRgb+(x*3),currentProject->rgbPal+(MapHLS[x].second*3),3);
			newPalType[x]=currentProject->palType[MapHLS[x].second];
		}
		memcpy(currentProject->palDat,newPal,palEdit.perRow*4*eSize);
		memcpy(currentProject->rgbPal,newPalRgb,palEdit.perRow*4*3);
		memcpy(currentProject->palType,newPalType,palEdit.perRow*4);
		delete[] MapHLS;
	}
	uint8_t type_temp=palTypeGen;
	uint8_t tempSet=0;
	double d[4];//Delta
	uint32_t di[4];//Delta integer
	uint32_t x;
	uint8_t t;
	uint8_t temp[256];//Just as a word of caution this is used for both sprintf temporary buffer and temporary truecolor tile buffer
	uint32_t w,h;
	w=mapSizeW*8;
	h=mapSizeHA*8;
	uint8_t * imagein=(uint8_t*)malloc(w*h*4);
	truecolor_to_image(imagein,-1);
	uint8_t **imageout=(uint8_t**)malloc(4*sizeof(void*));
	uint32_t xtile=0,ytile=0;
	if(showProgress){
		progress->maximum(12);
		progress->value(0);
	}
	for(x=0;x<4;++x){//This function has too many hard coded values The four should be a variable with the amount of palette rows
		if(showProgress){
			sprintf((char*)temp,"Dithering %d",x);
			progress->label((char*)temp);
			Fl::check();
		}
		imageout[x]=(uint8_t*)malloc(w*h*4);
		truecolor_to_image(imageout[x],-1);
		if(showProgress){
			progress->value((x*3)+1);
			Fl::check();
		}
		ditherImage(imageout[x],w,h,true,true,true,x);
		if(showProgress){
			progress->value((x*3)+2);
			Fl::check();
		}
		ditherImage(imageout[x],w,h,true,false,true,x);
		if(showProgress){
			progress->value((x*3)+3);
			Fl::check();
		}
	}
	if(showProgress){
		progress->maximum(mapSizeHA);
		progress->label("Picking tiles based on delta");
	}
	unsigned per;
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem&NES2x2))
		per=2;
	else
		per=1;
	for (uint_fast32_t a=0;a<(h*w*4)-(w*4*per);a+=w*4*8*per){//a tiles y
		for (uint_fast32_t b=0;b<w*4;b+=32*per){//b tiles x
			if(alg==2)
				memset(di,0,4*sizeof(uint32_t));
			else{
				for (t=0;t<4;t++)
					d[t]=0.0;
			}
			if ((type_temp != 0) && (currentProject->gameSystem == sega_genesis)){
				tempSet=(currentProject->tileMapC->get_prio(xtile,ytile)^1)*8;
				set_palette_type(tempSet);
			}
			for(t=0;t<4;++t){
				for(unsigned c=0;c<per*w*4*8;c+=w*4*8){
					for(uint32_t y=0;y<w*4*8;y+=w*4){//pixels y
						for(unsigned e=0;e<per*32;e+=32){
							if(imagein[a+b+y+x+3+c+e]!=0){//Avoid checking transperency
								switch(alg){
									case 0:
										for(x=0;x<32;x+=4)
											d[t]+=std::abs(ciede2000rgb(imagein[a+b+y+x+c+e],imagein[a+b+y+x+1+c+e],imagein[a+b+y+x+2+c+e],imageout[t][a+b+y+x+c+e],imageout[t][a+b+y+x+1+c+e],imageout[t][a+b+y+x+2+c+e]));
										break;
									case 1:
										for(x=0;x<32;x+=4)
											d[t]+=std::abs(ColourDistance(imagein[a+b+y+x+c+e],imagein[a+b+y+x+1+c+e],imagein[a+b+y+x+2+c+e],imageout[t][a+b+y+x+c+e],imageout[t][a+b+y+x+1+c+e],imageout[t][a+b+y+x+2+c+e]));
										break;
									case 3:
									case 4:
									case 5:
										for(x=0;x<32;x+=4){
											double h[2],l[2],s[2];
											rgbToHls(imagein[a+b+y+x+c+e],imagein[a+b+y+x+1+c+e],imagein[a+b+y+x+2+c+e],h,l,s);
											rgbToHls(imageout[t][a+b+y+x+c+e],imageout[t][a+b+y+x+1+c+e],imageout[t][a+b+y+x+2+c+e],h+1,l+1,s+1);
											d[t]+=std::abs(pickIt(h[0],l[0],s[0],alg-3)-pickIt(h[1],l[1],s[1],alg-3));
										}
										//printf("d[%d]=%f\n",t,d[t]);
										break;
									default://Usally case 2
										for(x=0;x<32;x+=4)
											di[t]+=sqri(imagein[a+b+y+x+c+e]-imageout[t][a+b+y+x+c+e])+sqri(imagein[a+b+y+x+1+c+e]-imageout[t][a+b+y+x+1+c+e])+sqri(imagein[a+b+y+x+2+c+e]-imageout[t][a+b+y+x+2+c+e]);
								}
							}
						}
					}
				}
			}
			uint8_t sillyrow;
			if(alg==2)
				sillyrow=pick4Deltai(di);
			else
				sillyrow=pick4Delta(d);
			set_pal_row(xtile,ytile,sillyrow);
			for(unsigned c=0,i=0;c<per*w*4*8;c+=w*4*8,++i){
				for(unsigned e=0,j=0;e<per*32;e+=32,++j){
					uint_fast32_t truecolor_tile_ptr=0;
					for (uint_fast32_t y=0;y<w*4*8;y+=w*4){//pixels y
						memcpy(&temp[truecolor_tile_ptr],&imageout[sillyrow][a+b+y+c+e],32);
						truecolor_tile_ptr+=32;
					}
					currentProject->tileC->truecolor_to_tile_ptr(sillyrow,get_tile(xtile+j,ytile+i),temp,false);
				}
			}
			xtile+=per;
		}
		if(showProgress){
			if((a%(w*4*8*16))==0){
				progress->value(ytile);
				window->redraw();
				Fl::check();
			}
		}
		xtile=0;
		ytile+=per;
	}
	free(imagein);
	free(imageout[0]);
	free(imageout[1]);
	free(imageout[2]);
	free(imageout[3]);
	free(imageout);
	if (currentProject->gameSystem == sega_genesis)
		set_palette_type(type_temp);
}
#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// RGB -> YUV
#define RGB2Y(R, G, B) CLIP(( (  66 * (R) + 129 * (G) +  25 * (B) + 128) >> 8) +  16)
#define RGB2U(R, G, B) CLIP(( ( -38 * (R) -  74 * (G) + 112 * (B) + 128) >> 8) + 128)
#define RGB2V(R, G, B) CLIP(( ( 112 * (R) -  94 * (G) -  18 * (B) + 128) >> 8) + 128)

// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)

// RGB -> YCbCr
#define CRGB2Y(R, G, B) CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16)
#define CRGB2Cb(R, G, B) CLIP((36962 * (B - CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16) ) >> 16) + 128)
#define CRGB2Cr(R, G, B) CLIP((46727 * (R - CLIP((19595 * R + 38470 * G + 7471 * B ) >> 16) ) >> 16) + 128)

// YCbCr -> RGB
#define CYCbCr2R(Y, Cb, Cr) CLIP( Y + ( 91881 * Cr >> 16 ) - 179 )
#define CYCbCr2G(Y, Cb, Cr) CLIP( Y - (( 22544 * Cb + 46793 * Cr ) >> 16) + 135)
#define CYCbCr2B(Y, Cb, Cr) CLIP( Y + (116129 * Cb >> 16 ) - 226 )
static void reduceImage(uint8_t * image,uint8_t * found_colors,int row,uint8_t offsetPal,Fl_Progress *progress,Fl_Window*pwin,uint8_t maxCol,unsigned yuv,unsigned alg,bool isSprite=false){
	progress->maximum(1.0);
	unsigned off2=offsetPal*2;
	unsigned off3=offsetPal*3;
	uint32_t colors_found;
	uint32_t w,h;
	uint8_t maxPal;
	switch(currentProject->gameSystem){
		case sega_genesis:
			maxPal=64;
		break;
		case NES:
			maxPal=16;
		break;
	}
	if(isSprite){
		w=currentProject->spritesC->width(curSpritegroup);
		h=currentProject->spritesC->height(curSpritegroup);
		currentProject->spritesC->spriteGroupToImage(image,curSpritegroup,row,false);
	}else{
		w=currentProject->tileMapC->mapSizeW;
		h=currentProject->tileMapC->mapSizeHA;
		w*=currentProject->tileC->sizew;
		h*=currentProject->tileC->sizeh;
		currentProject->tileMapC->truecolor_to_image(image,row,false);
	}
	progress->label("Dithering to colorspace");
	Fl::check();
	if(!yuv)
		ditherImage(image,w,h,false,true);
	progress->label("Quantizing image");
	Fl::check();
	colors_found=count_colors(image,w,h,&found_colors[0],false);
	printf("Unique colors %d\n",colors_found);
	if (colors_found <= maxCol){
		printf("%d colors\n",colors_found);
		for (unsigned x=0;x<colors_found;x++){
			uint8_t r,g,b;
againFun:
			if (currentProject->palType[x+offsetPal]){
				offsetPal++;
				off3+=3;
				off2+=2;
				if (offsetPal >= maxPal)
					break;
				goto againFun;
			}
			r=found_colors[(x*3)];
			g=found_colors[(x*3)+1];
			b=found_colors[(x*3)+2];
			switch(currentProject->gameSystem){
				case sega_genesis:
					printf("R=%d G=%d B=%d\n",r,g,b);
					r=nearest_color_index(r);
					g=nearest_color_index(g);
					b=nearest_color_index(b);
					currentProject->rgbPal[(x*3)+off3]=palTab[r];
					currentProject->rgbPal[(x*3)+1+off3]=palTab[g];
					currentProject->rgbPal[(x*3)+2+off3]=palTab[b];
					//bgr
					r-=palTypeGen;
					g-=palTypeGen;
					b-=palTypeGen;
					currentProject->palDat[(x*2)+off2]=b<<1;
					currentProject->palDat[(x*2)+1+off2]=(r<<1)|(g<<5);
				break;
				case NES:
					printf("R=%d G=%d B=%d\n",r,g,b);
					{uint8_t temp = to_nes_color_rgb(r,g,b);
					currentProject->palDat[x+offsetPal]=temp;}
				break;
				default:
					show_default_error
				break;
			}
		}
		if(currentProject->gameSystem==NES)
			update_emphesis(0,0);
		window->redraw();
	}else{
		printf("More than %d colors reducing to %d colors\n",maxCol,maxCol);
		uint8_t user_pal[3][256];			
		uint8_t rgb_pal2[768];
		uint8_t rgb_pal3[768];
		uint16_t colorz=maxCol;
		bool can_go_again=true;
		uint8_t*imageuse;
		uint8_t*output;
		if(alg==1)
			output=(uint8_t*)malloc(w*h*3);
		if(yuv){
			imageuse=(uint8_t*)malloc(w*h*3);
			uint32_t x,y;
			uint8_t*imageptr=image;
			uint8_t*outptr=imageuse;
			for(y=0;y<h;y++){
				for(x=0;x<w;x++){
					if(yuv==2){
						outptr[0]=CRGB2Y(imageptr[0],imageptr[1],imageptr[2]);
						outptr[1]=CRGB2Cb(imageptr[0],imageptr[1],imageptr[2]);
						outptr[2]=CRGB2Cr(imageptr[0],imageptr[1],imageptr[2]);
					}else{
						outptr[0]=RGB2Y(imageptr[0],imageptr[1],imageptr[2]);
						outptr[1]=RGB2U(imageptr[0],imageptr[1],imageptr[2]);
						outptr[2]=RGB2V(imageptr[0],imageptr[1],imageptr[2]);
					}
					imageptr+=3;
					outptr+=3;
				}
			}
		}else
			imageuse=image;
try_again_color:
		switch(alg){
			case 4:
				dl1quant(imageuse,w,h,colorz,user_pal);
			break;
			case 3:
				wu_quant(imageuse,w,h,colorz,user_pal);
			break;
			case 2:
				NEU_wrapper(w,h,imageuse,colorz,user_pal);
			break;
			case 1:
				scolorq_wrapper(imageuse,output,user_pal,w,h,colorz);
			break;
			default:
				dl3quant(imageuse,w,h,colorz,user_pal,true,progress);/*this uses denesis lee's v3 color quant which is fonund at http://www.gnu-darwin.org/www001/ports-1.5a-CURRENT/graphics/mtpaint/work/mtpaint-3.11/src/quantizer.c*/
		}
		for (uint16_t x=0;x<colorz;x++){
			uint8_t r,g,b;
			if(yuv){
				if(yuv==2){
					r=CYCbCr2R(user_pal[0][x],user_pal[1][x],user_pal[2][x]);
					g=CYCbCr2G(user_pal[0][x],user_pal[1][x],user_pal[2][x]);
					b=CYCbCr2B(user_pal[0][x],user_pal[1][x],user_pal[2][x]);
				}else{
					r=YUV2R(user_pal[0][x],user_pal[1][x],user_pal[2][x]);
					g=YUV2G(user_pal[0][x],user_pal[1][x],user_pal[2][x]);
					b=YUV2B(user_pal[0][x],user_pal[1][x],user_pal[2][x]);
				}
			}else{
				r=user_pal[0][x];
				g=user_pal[1][x];
				b=user_pal[2][x];
			}
			switch(currentProject->gameSystem){
				case sega_genesis:
					r=nearest_color_index(r);
					g=nearest_color_index(g);
					b=nearest_color_index(b);
					rgb_pal2[(x*3)]=palTab[r];
					rgb_pal2[(x*3)+1]=palTab[g];
					rgb_pal2[(x*3)+2]=palTab[b];
				break;
				case NES:
					uint8_t temp=to_nes_color_rgb(r,g,b);
					uint32_t temp_rgb = MakeRGBcolor(temp);
					rgb_pal2[(x*3)]=(temp_rgb>>16)&255;
					rgb_pal2[(x*3)+1]=(temp_rgb>>8)&255;
					rgb_pal2[(x*3)+2]=temp_rgb&255;
				break;
			}
		}
		uint8_t new_colors = count_colors(rgb_pal2,colorz,1,&rgb_pal3[off3]);
		printf("Unique colors in palette %d\n",new_colors);
			if (new_colors < maxCol){
				if (can_go_again == true){
					if (colorz != 512)
						colorz++;
					else
						can_go_again=false;
					char tmp[1024];
					snprintf(tmp,1024,"Found only %d colors trying again with %d",new_colors,colorz);
					pwin->copy_label(tmp);
					puts(tmp);
					Fl::check();
					goto try_again_color;
				}
			}
		if (new_colors > maxCol){
			can_go_again=false;
			puts("Woops too many colors");
			colorz--;
			goto try_again_color;
		}
		uint8_t off3o=off3;
		for (uint8_t x=0;x<maxCol;x++){
			uint8_t r,g,b;
againNerd:
			if (currentProject->palType[x+offsetPal]){
				offsetPal++;
				off3+=3;
				off2+=2;
				if (offsetPal >= maxPal)
					break;
				//printf("%d %d %d\n",offsetPal,off2,off3);
				goto againNerd;
			}
			memcpy(currentProject->rgbPal+off3+(x*3),&rgb_pal3[off3o+(x*3)],3);
			switch(currentProject->gameSystem){
				case sega_genesis:
					r=currentProject->rgbPal[(x*3)+off3];
					g=currentProject->rgbPal[(x*3)+1+off3];
					b=currentProject->rgbPal[(x*3)+2+off3];
					r=nearest_color_index(r)-palTypeGen;
					g=nearest_color_index(g)-palTypeGen;
					b=nearest_color_index(b)-palTypeGen;
					currentProject->palDat[(x*2)+off2]=b<<1;
					currentProject->palDat[(x*2)+1+off2]=(r<<1)|(g<<5);
				break;
				case NES:
					currentProject->palDat[x+offsetPal]=to_nes_color(x+offsetPal);
				break;
			}
		}
		if(currentProject->gameSystem==NES)
			update_emphesis(0,0);
		if(alg==1){
			currentProject->tileMapC->truecolorimageToTiles(output,row,false);
			free(output);
		}
		if(yuv)
			free(imageuse);
	}
}
void generate_optimal_palette(Fl_Widget*,void*sprite){
	bool isSprite=((uintptr_t)sprite)?true:false;
	uint8_t perRow[4];
	char temp[4];
	unsigned rowSize;
	unsigned rows;
	switch (currentProject->gameSystem){
		case sega_genesis:
			if(isSprite)
				strcpy(temp,"16");
			else
				strcpy(temp,"64");
			rowSize=16;
		break;
		case NES:
			if(isSprite)
				strcpy(temp,"4");
			else
				strcpy(temp,"16");
			rowSize=4;
		break;
	}
	char * returned=(char *)fl_input("How many colors would you like?",temp);
	if(!returned)
		return;
	if(!verify_str_number_only(returned))
		return;
	int colors=atoi(returned);
	int colorstotal=colors;
	if(isSprite){
		rows=1;
		perRow[0]=colors > rowSize ? rowSize:colors;
	}else{
		uint8_t asdf;
		for(asdf=0;asdf<4;++asdf){
			perRow[asdf]=colors > rowSize ? rowSize:colors;
			colors-=rowSize;
			printf("Colors %d\n",colors);
			if (colors <= 0)
				break;
		}
		rows=asdf+1;
	}
	printf("Using %d rows\n",rows);
	/*
	This function is one of the more importan features of the program
	This will look at the tile map and based on that find an optimal palette
	*/
	uint8_t * image;
	//uint8_t * colors;
	uint32_t w,h;
	if(isSprite){
		w=currentProject->spritesC->width(curSpritegroup);
		h=currentProject->spritesC->height(curSpritegroup);
	}else{
		w=currentProject->tileMapC->mapSizeW;
		h=currentProject->tileMapC->mapSizeHA;
		w*=currentProject->tileC->sizew;
		h*=currentProject->tileC->sizeh;
	}
	uint32_t colors_found;
	//uint8_t * found_colors;
	uint8_t found_colors[768];
	int rowAuto;
	if((rows==1)&&(!isSprite))
		rowAuto = fl_ask("Would you like all tiles on the tilemap to be set to row 0? (This is where all generated colors will apear)");
	else if(!isSprite){
		rowAuto = MenuPopup("Palette setting","How would you like the palette map to be handled",3,"Don't change anythin","Pick based on hue","Generate contiguous palette then pick based on delta");
		if(rowAuto<0)
			return;
	}else
		rowAuto=0;
	uint8_t fun_palette;
	int alg=MenuPopup("Pick an algorithm","What color reduction algorithm would you like used?",5,"Dennis Lee v3","scolorq","Neuquant","Wu","Dennis Lee v1");
	if(alg<0)
		return;
	int yuv;
	yuv=MenuPopup("Color space selection","What color space would you like to use?",3,"rgb","yuv","YCbCr");
	if(yuv<0)
		return;
	pushPaletteAll();//Save the old palette
	Fl_Window *win;
	Fl_Progress *progress;
	win = new Fl_Window(400,45,"Progress");		// access parent window
	win->begin();					// add progress bar to it..
	progress = new Fl_Progress(25,7,350,30);
	progress->minimum(0.0);				// set progress range to be 0.0 ~ 1.0
	progress->maximum(1.0);
	progress->color(0x88888800);			// background color
	progress->selection_color(0x4444ff00);		// progress bar color
	progress->labelcolor(FL_WHITE);			// percent text color
	win->end();					// end adding to window
	win->show();
	switch (currentProject->gameSystem){
		case sega_genesis:
			fun_palette=16;
		break;
		case NES:
			fun_palette=4;
		break;
		default:
			show_default_error
		break;
	}
	image = (uint8_t *)malloc(w*h*3);
	if (rows==1){
		if (rowAuto)
			currentProject->tileMapC->allRowZero();
		if(isSprite)
			reduceImage(image,found_colors,-1,0,progress,win,perRow[0],yuv,alg,true);
		else
			reduceImage(image,found_colors,-1,0,progress,win,perRow[0],yuv,alg);
		window->damage(FL_DAMAGE_USER1);
		Fl::check();
	}else{
		if(rowAuto==2){
			reduceImage(image,found_colors,-1,0,progress,win,colorstotal,yuv,alg);
			currentProject->tileMapC->pickRowDelta(true,progress);
			window->damage(FL_DAMAGE_USER1);
			Fl::check();
		}else{
			if (rowAuto)
				currentProject->tileMapC->pickRow(rows);
			for (uint8_t nerdL=0;nerdL<rows;nerdL++){
				reduceImage(image,found_colors,nerdL,nerdL*fun_palette,progress,win,perRow[nerdL],yuv,alg);
				window->damage(FL_DAMAGE_USER1);
				Fl::check();
			}
		}
	}
	free(image);
	win->remove(progress);// remove progress bar from window
	delete(progress);// deallocate it
	//w->draw();
	delete win;
	palEdit.updateSlider();
	tileEdit_pal.updateSlider();
	tileMap_pal.updateSlider();
	spritePal.updateSlider();
	window->redraw();
	Fl::check();
}
