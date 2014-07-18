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
/*
Stuff related to tilemap operations goes here*/
#include "global.h"
#include "quant.h"
#include "color_compare.h"
#include "color_convert.h"
#include "dither.h"
#include "spatial_color_quant.h"
#include "NEUQUANT.H"
#include "palette.h"
#include "wu.h"
bool truecolor_to_image(uint8_t * the_image,int8_t useRow,bool useAlpha){
	/*!
	the_image pointer to image must be able to hold the image using rgba 32bit or rgb 24bit if not using alpha
	useRow what row to use or -1 for no row
	*/
	if (the_image == 0){
		fl_alert("Error malloc must be called before generating this image");
		return false;
	}
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeHA*8;
	uint16_t x_tile=0,y_tile=0;
	uint32_t truecolor_tile_ptr=0;
	uint8_t pixelSize,pSize2;
	if (useAlpha){
		pixelSize=4;
		pSize2=32;
	}else{
		pixelSize=3;
		pSize2=24;
	}
	if (useRow != -1){
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8){//a tiles y
			for (uint_fast32_t b=0;b<w*pixelSize;b+=pSize2){//b tiles x
				truecolor_tile_ptr=currentProject->tileMapC->get_tileRow(x_tile,y_tile,useRow)*256;
				if (truecolor_tile_ptr != -256){
					for (uint_fast32_t y=0;y<w*pSize2;y+=w*pixelSize){//pixels y
						if (useAlpha)
							memcpy(&the_image[a+b+y],&currentProject->tileC->truetileDat[truecolor_tile_ptr],32);
						else{
							unsigned xx=0;
							for (unsigned x=0;x<32;x+=4)//pixels x
							{
								the_image[a+b+y+xx]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x];
								the_image[a+b+y+xx+1]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+1];
								the_image[a+b+y+xx+2]=currentProject->tileC->truetileDat[truecolor_tile_ptr+x+2];
								xx+=3;
							}
						}
						truecolor_tile_ptr+=32;
					}
				}else{
					for (uint32_t y=0;y<w*pSize2;y+=w*pixelSize)//pixels y
						memset(&the_image[a+b+y],0,pSize2);
				}
				++x_tile;
			}
			x_tile=0;
			++y_tile;
		}
	}else{
		for (uint64_t a=0;a<(h*w*pixelSize)-w*pixelSize;a+=w*pixelSize*8){//a tiles y
			for (uint32_t b=0;b<w*pixelSize;b+=pSize2){//b tiles x
				truecolor_tile_ptr=currentProject->tileMapC->get_tile(x_tile,y_tile)*256;
				for (uint32_t y=0;y<w*pixelSize*8;y+=w*pixelSize){//pixels y
					if (useAlpha)
						memcpy(&the_image[a+b+y],&currentProject->tileC->truetileDat[truecolor_tile_ptr],32);
					else{
						unsigned xx=0;
						for (unsigned x=0;x<32;x+=4){//pixels x
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
	return true;
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
void rgbToHls(double r,double g,double b,double * hh,double * ll,double * ss){
	r /= 255.0;
	g /= 255.0;
	b /= 255.0;
	double max = max3(r, g, b);
	double min = min3(r, g, b);
	double h, s, l = (max + min) / 2.0;

	if(max == min){
		h = s = 0.0; // achromatic
	}else{
		double d = max - min;
		s = l > 0.5 ? d / (2.0 - max - min) : d / (max + min);
		if (max == r)
			h = (g - b) / d + (g < b ? 6 : 0);
		else if (max == g)
			h = (b - r) / d + 2.0;
		else
			h = (r - g) / d + 4.0;

		h /= 6.0;
	}
	*hh=h;
	*ll=l;
	*ss=s;
}
void rgbtohsv(uint8_t R,uint8_t G,uint8_t B,double * hh,double * ss,double * vv){
	double var_R =  R / 255.0;				//RGB from 0 to 255
	double var_G =  G / 255.0;
	double var_B =  B / 255.0;

	double var_Min = min3(var_R, var_G, var_B);		//Min. value of RGB
	double var_Max = max3(var_R, var_G, var_B);		//Max. value of RGB
	double del_Max = var_Max - var_Min;			//Delta RGB value 
	double V = var_Max;
	double H,S;
	if (del_Max == 0.0){					//This is a gray, no chroma...
		H = 0.0;					//HSV results from 0 to 1
		S = 0.0;
	}else{							//Chromatic data...
		S = del_Max / var_Max;
		double del_R = (((var_Max - var_R ) / 6.0 ) + (del_Max / 2.0 )) / del_Max;
		double del_G = (((var_Max - var_G ) / 6.0 ) + (del_Max / 2.0 )) / del_Max;
		double del_B = (((var_Max - var_B ) / 6.0 ) + (del_Max / 2.0 )) / del_Max;
		if (var_R == var_Max)
			H = del_B - del_G;
		else if (var_G == var_Max)
			H = (1.0 / 3.0) + del_R - del_B;
		else if (var_B == var_Max)
			H = (2.0 / 3.0) + del_G - del_R;
		if (H < 0.0)
			H += 1.0;
		if (H > 1.0)
			H -= 1.0;
	}
	*hh=H;
	*ss=S;
	*vv=V;
}
static double getHH(uint32_t cur_tile,int type){
	double hh=0.0;
	uint8_t * truePtr=&currentProject->tileC->truetileDat[cur_tile*256];
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
	double divide=(double)amount;//convert to double
	uint32_t x,y;
	double maxPal=divide;
	double divBy;
	unsigned addBy;
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem==NES2x2)){
		divBy=256.0;//8*8*4
		addBy=2;
	}else{
		divBy=64.0;//8*8
		addBy=1;
	}
	for (y=0;y<mapSizeHA;y+=addBy){
		for (x=0;x<mapSizeW;x+=addBy){
			double hh;
			if((currentProject->gameSystem==NES)&&(currentProject->subSystem==NES2x2)){
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
bool comparatorHLS ( const HLSpair& l, const HLSpair& r)
   { return l.first < r.first; }
void tileMap::pickRowDelta(bool showProgress,Fl_Progress *progress){
	int alg=MenuPopup("Select picking algorithm","Pick which method you think works better for this image.",6,"ciede2000","Weighted","Mean squared error","Hue difference","Saturation difference","Lightness difference");
	if(alg<0)
		return;
	if(fl_ask("Would you like the palette to be ordered by hue or light or saturation")){
		unsigned type=fl_choice("What do you want it ordered by","Hue","Light","Saturation");
		HLSpair* MapHLS=new HLSpair[palEdit.perRow*4];//Remember to change if there is a palete with a different amount than 4 rows
		for(unsigned x=0;x<palEdit.perRow*3*4;x+=3){
			double h,l,s,cmp,cmp2;
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
	uint32_t x,y;
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
	if((currentProject->gameSystem==NES)&&(currentProject->subSystem==NES2x2))
		per=2;
	else
		per=1;
	for (uint_fast32_t a=0;a<(h*w*4)-(w*4*per);a+=w*4*8*per){//a tiles y
		for (uint_fast32_t b=0;b<w*4;b+=32*per){//b tiles x
			if(alg>=2)
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
							if(imagein[a+b+y+x+3]!=0){//Avoid checking transperency
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
static void reduceImage(uint8_t * image,uint8_t * found_colors,int8_t row,uint8_t offsetPal,Fl_Progress *progress,uint8_t maxCol,unsigned yuv,unsigned alg){
	progress->maximum(1.0);
	uint8_t off2=offsetPal*2;
	uint8_t off3=offsetPal*3;
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
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeHA*8;
	truecolor_to_image(image,row,false);
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
		for (uint8_t x=0;x<colors_found;x++){
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
		}
		else
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
					printf("Trying again at %d needs more color\n",colorz);
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
			truecolorimageToTiles(output,row,false);
			free(output);
		}
		if(yuv)
			free(imageuse);
	}
}
void generate_optimal_palette(Fl_Widget*,void*){
	uint8_t perRow[4];
	char temp[4];
	uint8_t rowSize;
	uint8_t rows;
	switch (currentProject->gameSystem){
		case sega_genesis:
			strcpy(temp,"64");
			rowSize=16;
		break;
		case NES:
			strcpy(temp,"16");
			rowSize=4;
		break;
	}
	char * returned=(char *)fl_input("How many colors would you like?",temp);
	if(!returned)
		return;
	if(!verify_str_number_only(returned))
		return;
	int8_t colors=atoi(returned);
	int8_t colorstotal=colors;
	uint8_t asdf;
	for (asdf=0;asdf<4;asdf++) {
		perRow[asdf]=colors > rowSize ? rowSize:colors;
		colors-=rowSize;
		printf("Colors %d\n",colors);
		if (colors <= 0)
			break;
	}
	rows=asdf+1;
	printf("Using %d rows\n",rows);
	/*
	This function is one of the more importan features of the program
	This will look at the tile map and based on that find an optimal palette
	*/
	uint8_t * image;
	//uint8_t * colors;
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeHA*8;
	uint32_t colors_found;
	//uint8_t * found_colors;
	uint8_t found_colors[768];
	int rowAuto;
	if (rows==1)
		rowAuto = fl_ask("Would you like all tiles on the tilemap to be set to row 0? (This is where all generated colors will apear)");
	else{
		rowAuto = MenuPopup("Palette setting","How would you like the palette map to be handled",3,"Don't change anythin","Pick based on hue","Generate contiguous palette then pick based on delta");
		if(rowAuto<0)
			return;
	}
	uint8_t fun_palette;
	int alg=MenuPopup("Pick an algorithm","What color reduction algorithm would you like used?",5,"Dennis Lee v3","scolorq","Neuquant","Wu","Dennis Lee v1");
	if(alg<0)
		return;
	int yuv;
	yuv=MenuPopup("Color space selection","What color space would you like to use?",3,"rgb","yuv","YCbCr");
	if(yuv<0)
		return;
	Fl_Window *win;
	Fl_Progress *progress;
	win = new Fl_Window(250,45,"Progress");		// access parent window
	win->begin();					// add progress bar to it..
	progress = new Fl_Progress(25,7,200,30);
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
		reduceImage(image,found_colors,-1,0,progress,perRow[0],yuv,alg);
		window->damage(FL_DAMAGE_USER1);
		Fl::check();
	}else{
		if(rowAuto==2){
			reduceImage(image,found_colors,-1,0,progress,colorstotal,yuv,alg);
			currentProject->tileMapC->pickRowDelta(true,progress);
			window->damage(FL_DAMAGE_USER1);
			Fl::check();
		}else{
			if (rowAuto)
				currentProject->tileMapC->pickRow(rows);
			for (uint8_t nerdL=0;nerdL<rows;nerdL++){
				reduceImage(image,found_colors,nerdL,nerdL*fun_palette,progress,perRow[nerdL],yuv,alg);
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
	window->redraw();
	Fl::check();
}
void truecolorimageToTiles(uint8_t * image,int rowusage,bool useAlpha){
	uint8_t type_temp=palTypeGen;
	uint8_t tempSet=0;
	uint8_t truecolor_tile[256];
	uint_fast32_t x_tile=0;
	uint_fast32_t y_tile=0;
	uint_fast8_t pSize=useAlpha ? 4:3;
	uint_fast8_t pTile=useAlpha ? 32:24;
	uint32_t w=currentProject->tileMapC->mapSizeW*8;
	uint32_t h=currentProject->tileMapC->mapSizeHA*8;
	uint_fast32_t truecolor_tile_ptr;
	for (uint_fast32_t a=0;a<(h*w*pSize)-w*pSize;a+=w*pSize*8){//a tiles y
		for (uint_fast32_t b=0;b<w*pSize;b+=pTile){//b tiles x
			uint8_t temp;
			int32_t current_tile;
			if(rowusage==-1){
				current_tile=currentProject->tileMapC->get_tile(x_tile,y_tile);
			}else{
				current_tile=currentProject->tileMapC->get_tileRow(x_tile,y_tile,rowusage);
				if (current_tile == -1)
					goto dont_convert_tile;
			}
			truecolor_tile_ptr=0;
			for (uint32_t y=0;y<w*pSize*8;y+=w*pSize)//pixels y
			{
				if(useAlpha){
					memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y],32);
					truecolor_tile_ptr+=32;
				}else{
					for(uint8_t xx=0;xx<24;xx+=3){
						memcpy(&truecolor_tile[truecolor_tile_ptr],&image[a+b+y+xx],3);
						truecolor_tile_ptr+=3;
						truecolor_tile[truecolor_tile_ptr]=255;
						++truecolor_tile_ptr;
					}
				}
			}
			//convert back to tile
			uint8_t * TileTempPtr;
			if ((type_temp != 0) && (currentProject->gameSystem == sega_genesis)){
				tempSet=(currentProject->tileMapC->get_prio(x_tile,y_tile)^1)*8;
				set_palette_type(tempSet);
			}
			currentProject->tileC->truecolor_to_tile_ptr(currentProject->tileMapC->get_palette_map(x_tile,y_tile),current_tile,truecolor_tile,false);
dont_convert_tile:
		x_tile++;	
		}
	x_tile=0;
	y_tile++;
	}
	if (currentProject->gameSystem == sega_genesis)
		set_palette_type(type_temp);
}
