/*
	This file is part of Retro Graphics Toolkit

	Retro Graphics Toolkit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or any later version.

	Retro Graphics Toolkit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
	Copyright Sega16 (or whatever you wish to call me) (2012-2016)
*/
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
#include "palette.h"
#include "classtilemap.h"
#include "classpalette.h"
#include "classpalettebar.h"
#include "nearestColor.h"
#include "gui.h"
#include "class_global.h"
static void addHist(uint32_t cur_tile,int type,uint32_t*hist,unsigned sz){
	double szz=(double)sz;
	uint8_t * truePtr=&currentProject->tileC->truetDat[cur_tile*256];
	double h,l,s;
	for(unsigned z=0;z<256;z+=4){
		rgbToHsl255(truePtr[0],truePtr[1],truePtr[2],&h,&s,&l);
		truePtr+=4;
		switch(type){
			case 0:
				++hist[unsigned(h*szz)];
			break;
			case 1:
				++hist[unsigned(s*szz)];
			break;
			case 2:
				++hist[unsigned(l*szz)];
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
				szz/=2.0;
				++hist[unsigned((h+s)*szz)];
			break;
			case 7:
				szz/=2.0;
				++hist[unsigned((h+l)*szz)];
			break;
			case 8:
				szz/=2.0;
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
		rgbToHsl255(truePtr[0],truePtr[1],truePtr[2],&h,&s,&l);
		truePtr+=4;
		switch(type){
			case 0:
				hh+=h;
			break;
			case 1:
				hh+=s;
			break;
			case 2:
				hh+=l;
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
static const Fl_Menu_Item hueChoices[]={
	{"Hue",0,0,0},
	{"Saturation",0,0,0},
	{"Lightness",0,0,0},
	{"Hue*saturation",0,0,0},
	{"Hue*Lightness",0,0,0},
	{"Lightness*saturation",0,0,0},
	{"Hue+saturation",0,0,0},
	{"Hue+lightness",0,0,0},
	{"Lightness+saturation",0,0,0},
	{0}
};
static const Fl_Menu_Item hueMethodChoices[]={
	{"Average",0,0,0},
	{"Histogram section with most occurrences (stretch)",0,0,0},
	{"Histogram section with most occurrences",0,0,0},
	{"Histogram peak (stretch)",0,0,0},
	{"Histogram peak",0,0,0},
	{0}
};
static const char*hueLblSel="Pick tile row based on";
static const char*hueTooltip="Please pick what most defines the image";
static const char*hueMethodLbl="Select a method";
static const char*hueMethodTooltip="This depends on the image";
void tileMap::pickRow(unsigned amount,int type,int method){
	if(type<0){
		type=menuPopupArray(hueLblSel,hueTooltip,0,hueChoices);
		if(type<0)
			return;
	}
	if(method<0){
		method=menuPopupArray(hueMethodLbl,hueMethodTooltip,0,hueMethodChoices);
		if(method<0)
			return;
	}
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
		stretch=method&1;
		method=(method+1)>>1;
		if(type>=6)
			sz=2000;
		else
			sz=1000;
		hist=(uint32_t*)calloc(sz,sizeof(uint32_t));
		if(stretch){
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
			uint32_t*histp=hist;
			while(!(*histp++));//At least one entry in the array will contain a nonzero value
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
template <typename Type>
static unsigned pick4Delta(Type*ptr,unsigned amt){
	Type minVal=ptr[0];
	unsigned which=0;
	for(unsigned i=1;i<amt;++i){
		if(ptr[i]<minVal){
			minVal=ptr[i];
			which=i;
		}
	}
	return which;
}
static inline uint32_t sqri(int x){
	return x*x;
}
static const Fl_Menu_Item deltaChoices[]={
	{"ciede2000",0,0,0},
	{"Weighted",0,0,0},
	{"Mean squared error",0,0,0},
	{"Hue difference",0,0,0},
	{"Saturation difference",0,0,0},
	{"Lightness difference",0,0,0},
	{0}
};
static const char*deltaAlgLbl="Select picking algorithm";
static const char*deltaAlgTooltip="Pick which method you think works better for this image.";
static const Fl_Menu_Item deltaOrderChoices[]={
	{"Don't sort",0,0,0},
	{"Hue -- per row",0,0,0},
	{"Saturation -- per row",0,0,0},
	{"Lightness -- per row",0,0,0},
	{"Hue -- globally",0,0,0},
	{"Saturation -- globally",0,0,0},
	{"Lightness -- globally",0,0,0},
	{0}
};
static const char*deltaOrderLbl="Sort palette by:";
static const char*deltaOrderTooltip="Select an option other than Don't sort to sort the palette by least to most using the select component.";
void tileMap::pickRowDelta(bool showProgress,Fl_Progress *progress,int alg,int order){
	if(currentProject->pal->rowCntPal<=1){
		fl_alert("This function needs more than one palette row to work");
		return;
	}
	if(alg<0){
		alg=menuPopupArray(deltaAlgLbl,deltaAlgTooltip,0,deltaChoices);
		if(alg<0)
			return;
	}
	pushTilemapAll(true);
	pushTilesAll(tTypeTile);
	if(order<0){
		order=menuPopupArray(deltaAlgLbl,deltaAlgTooltip,0,deltaOrderChoices);
		if(order<0)
			return;
	}
	if(order>0){
		pushPaletteAll();
		sortBy(order>3?order-4:order-1,order<4);
	}
	unsigned type_temp=palTypeGen;
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
	uint8_t **imageout=(uint8_t**)malloc(currentProject->pal->rowCntPal*sizeof(void*));
	uint32_t xtile=0,ytile=0;
	if(showProgress){
		progress->maximum(currentProject->pal->rowCntPal*3);
		progress->value(0);
	}
	for(x=0;x<currentProject->pal->rowCntPal;++x){//This function has too many hard coded values The four should be a variable with the amount of palette rows
		if(showProgress){
			snprintf((char*)temp,256,"Dithering %d",x);
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
			if(alg==2||alg==1)
				memset(di,0,currentProject->pal->rowCntPal*sizeof(uint32_t));
			else{
				std::fill(d,d+currentProject->pal->rowCntPal,0.);
			}
			if ((type_temp != 0) && (currentProject->gameSystem == segaGenesis)){
				tempSet=(currentProject->tms->maps[currentProject->curPlane].get_prio(xtile,ytile)^1)*8;
				set_palette_type_force(tempSet);
			}
			for(t=0;t<currentProject->pal->rowCntPal;++t){
				for(unsigned c=0;c<per*w*4*8;c+=w*4*8){
					for(uint32_t y=0;y<w*4*8;y+=w*4){//pixels y
						for(unsigned e=0;e<per*32;e+=32){
							if(imagein[a+b+y+x+3+c+e]!=0){//Avoid checking transparency
								switch(alg){
									case 0:
										for(x=0;x<32;x+=4)
											d[t]+=std::abs(ciede2000rgb(imagein[a+b+y+x+c+e],imagein[a+b+y+x+1+c+e],imagein[a+b+y+x+2+c+e],imageout[t][a+b+y+x+c+e],imageout[t][a+b+y+x+1+c+e],imageout[t][a+b+y+x+2+c+e]));
									break;
									case 1:
										for(x=0;x<32;x+=4)
											di[t]+=ColourDistance(imagein[a+b+y+x+c+e],imagein[a+b+y+x+1+c+e],imagein[a+b+y+x+2+c+e],imageout[t][a+b+y+x+c+e],imageout[t][a+b+y+x+1+c+e],imageout[t][a+b+y+x+2+c+e]);
									break;
									case 3:
									case 4:
									case 5:
										for(x=0;x<32;x+=4){
											double h[2],l[2],s[2];
											rgbToHsl255(imagein[a+b+y+x+c+e],imagein[a+b+y+x+1+c+e],imagein[a+b+y+x+2+c+e],h,s,l);
											rgbToHsl255(imageout[t][a+b+y+x+c+e],imageout[t][a+b+y+x+1+c+e],imageout[t][a+b+y+x+2+c+e],h+1,s+1,l+1);
											d[t]+=std::abs(pickIt(h[0],s[0],l[0],alg-3)-pickIt(h[1],s[1],l[1],alg-3));
										}
									break;
									default://Usually case 2
										for(x=0;x<32;x+=4)
											di[t]+=sqri(imagein[a+b+y+x+c+e]-imageout[t][a+b+y+x+c+e])+sqri(imagein[a+b+y+x+1+c+e]-imageout[t][a+b+y+x+1+c+e])+sqri(imagein[a+b+y+x+2+c+e]-imageout[t][a+b+y+x+2+c+e]);
								}
							}
						}
					}
				}
			}
			unsigned sillyrow;
			if(alg==2||alg==1)
				sillyrow=pick4Delta(di,currentProject->pal->rowCntPal);
			else
				sillyrow=pick4Delta(d,currentProject->pal->rowCntPal);
			set_pal_row(xtile,ytile,sillyrow);
			for(unsigned c=0,i=0;c<per*w*4*8;c+=w*4*8,++i){
				for(unsigned e=0,j=0;e<per*32;e+=32,++j){
					uint_fast32_t truecolor_tile_ptr=0;
					for (uint_fast32_t y=0;y<w*4*8;y+=w*4){//pixels y
						memcpy(&temp[truecolor_tile_ptr],&imageout[sillyrow][a+b+y+c+e],32);
						truecolor_tile_ptr+=32;
					}
					currentProject->tileC->truecolor_to_tile_ptr(sillyrow,get_tile(xtile+j,ytile+i),temp,false,false);
				}
			}
			xtile+=per;
		}
		if(showProgress){
			if((a%(w*4*8*16))==0){
				progress->value(ytile);
				if(window){
					window->redraw();
					Fl::check();
				}
			}
		}
		xtile=0;
		ytile+=per;
	}
	free(imagein);
	for(unsigned i=0;i<currentProject->pal->rowCntPal;++i)
		free(imageout[i]);
	free(imageout);
	if(currentProject->gameSystem == segaGenesis)
		set_palette_type();
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
static void colorAmtExceed(void){
	fl_alert("No more room for colors\nYou should not be seeing this message please report this.");
}
static void reduceImage(uint8_t * image,uint8_t * found_colors,int row,unsigned offsetPal,Fl_Progress *progress,Fl_Window*pwin,unsigned maxCol,unsigned yuv,unsigned alg,bool isSprite=false,bool ditherBefore=true){
	if(progress)
		progress->maximum(1.0);
	unsigned off2=offsetPal*2;
	unsigned off3=offsetPal*3;
	unsigned colors_found;
	unsigned w,h;
	unsigned maxPal=maxCol;
	unsigned msprt=curSpritemeta;
	if(isSprite){
		w=currentProject->ms->sps[msprt].width(curSpritegroup);
		h=currentProject->ms->sps[msprt].height(curSpritegroup);
		currentProject->ms->sps[msprt].spriteGroupToImage(image,curSpritegroup,row,false);
	}else{
		w=currentProject->tms->maps[currentProject->curPlane].mapSizeW;
		h=currentProject->tms->maps[currentProject->curPlane].mapSizeHA;
		w*=currentProject->tileC->sizew;
		h*=currentProject->tileC->sizeh;
		currentProject->tms->maps[currentProject->curPlane].truecolor_to_image(image,row,false);
	}
	if(progress){
		progress->label("Dithering to colorspace");
		Fl::check();
	}
	if((!yuv)&&ditherBefore)
		ditherImage(image,w,h,false,true);
	if(progress){
		progress->label("Quantizing image");
		Fl::check();
	}
	colors_found=count_colors(image,w,h,&found_colors[0],false);
	printf("Unique colors %d\n",colors_found);
	if (colors_found <= maxCol){
		printf("%d colors\n",colors_found);
		unsigned offsetTmp=offsetPal;
		for (unsigned x=0;x<colors_found;x++){
			uint_fast8_t r,g,b;
againFun:
			if (currentProject->pal->palType[offsetTmp]){
				++offsetTmp;
				if(offsetTmp>=maxPal)
					goto actullyNeededReduction;
				goto againFun;
			}
			r=found_colors[(x*3)];
			g=found_colors[(x*3)+1];
			b=found_colors[(x*3)+2];
			printf("R=%d G=%d B=%d\n",r,g,b);
			if(currentProject->pal->shouldAddCol(offsetTmp,r,g,b,isSprite)){
				currentProject->pal->rgbToEntry(r,g,b,offsetTmp);
				currentProject->pal->updateRGBindex(offsetTmp);
				++offsetTmp;
			}
		}
		if(currentProject->gameSystem==NES)
			updateEmphesis();
		if(window)
			window->redraw();
	}else{
actullyNeededReduction:
		printf("More than %d colors reducing to %d colors\n",maxCol,maxCol);
		uint8_t user_pal[3][256];			
		uint8_t rgb_pal2[768];
		uint8_t rgb_pal3[768];
		unsigned colorz=maxCol;
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
				dl3quant(imageuse,w,h,colorz,user_pal,true,progress);/*this uses denesis lee's v3 color quant which is found at http://www.gnu-darwin.org/www001/ports-1.5a-CURRENT/graphics/mtpaint/work/mtpaint-3.11/src/quantizer.c*/
		}
		for (unsigned x=0;x<colorz;x++){
			unsigned r,g,b;
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
				case segaGenesis:
					r=nearest_color_index(r);
					g=nearest_color_index(g);
					b=nearest_color_index(b);
					rgb_pal2[(x*3)]=palTab[r];
					rgb_pal2[(x*3)+1]=palTab[g];
					rgb_pal2[(x*3)+2]=palTab[b];
				break;
				case NES:
					{uint8_t temp=currentProject->pal->to_nes_color_rgb(r,g,b);
					uint32_t temp_rgb = nesPalToRgb(temp);
					rgb_pal2[(x*3)]=(temp_rgb>>16)&255;
					rgb_pal2[(x*3)+1]=(temp_rgb>>8)&255;
					rgb_pal2[(x*3)+2]=temp_rgb&255;}
				break;
				case masterSystem:
				case gameGear:
					{const uint8_t*palUseTab=currentProject->gameSystem==gameGear?palTabGameGear:palTabMasterSystem;
					unsigned colsTab=currentProject->gameSystem==gameGear?16:4;
					r=nearestOneChannel(r,palUseTab,colsTab);
					g=nearestOneChannel(g,palUseTab,colsTab);
					b=nearestOneChannel(b,palUseTab,colsTab);
					rgb_pal2[(x*3)]=palUseTab[r];
					rgb_pal2[(x*3)+1]=palUseTab[g];
					rgb_pal2[(x*3)+2]=palUseTab[b];}
				break;
				default:
					show_default_error
			}
		}
		unsigned new_colors = count_colors(rgb_pal2,colorz,1,rgb_pal3);
		printf("Unique colors in palette %u\n",new_colors);
			if (new_colors < maxCol){
				if (can_go_again == true){
					if (colorz != 512)
						colorz++;
					else
						can_go_again=false;
					char tmp[1024];
					snprintf(tmp,1024,"Found only %d colors trying again with %d",new_colors,colorz);
					tmp[sizeof(tmp)-1]=0;
					if(pwin){
						pwin->copy_label(tmp);
						Fl::check();
					}
					puts(tmp);
					goto try_again_color;
				}
			}
		if (new_colors > maxCol){
			can_go_again=false;
			if(pwin)
				pwin->label("Too many colors");
			colorz--;
			goto try_again_color;
		}
		unsigned offsetTmp=offsetPal;
		for (unsigned x=0;x<maxCol;x++){
againNerd:
			if (currentProject->pal->palType[offsetTmp]){
				++offsetTmp;
				if(offsetTmp>(maxPal+offsetPal)){
					if(maxCol>1){
						--colorz;
						printf("Needed to reduce colors generated due to locked colors %u\n",maxCol);
					}else{
						fl_alert("Cannot reduce maximum colors to make this happen...aborting");
						return;	
					}
					goto try_again_color;
				}
				goto againNerd;
			}
			unsigned r=rgb_pal3[x*3],g=rgb_pal3[x*3+1],b=rgb_pal3[x*3+2];
			if(currentProject->pal->shouldAddCol(offsetTmp,r,g,b,isSprite)){
				memcpy(currentProject->pal->rgbPal+(offsetTmp*3),rgb_pal3+(x*3),3);
				currentProject->pal->rgbToEntry(r,g,b,offsetTmp);
				++offsetTmp;
			}
		}
		if(currentProject->gameSystem==NES)
			updateEmphesis();
		if(alg==1){
			if(isSprite)
				currentProject->ms->sps[msprt].spriteImageToTiles(output,curSpritegroup,row,false);
			else
				currentProject->tms->maps[currentProject->curPlane].truecolorimageToTiles(output,row,false);
			free(output);
		}
		if(yuv)
			free(imageuse);
	}
}
void generate_optimal_paletteapply(Fl_Widget*,void*s){
	struct settings*set=(struct settings*)s;
	uint8_t * image;
	uint32_t w,h;
	if(set->sprite){
		unsigned msprt=curSpritemeta;
		w=currentProject->ms->sps[msprt].width(curSpritegroup);
		h=currentProject->ms->sps[msprt].height(curSpritegroup);
	}else{
		w=currentProject->tms->maps[currentProject->curPlane].mapSizeW;
		h=currentProject->tms->maps[currentProject->curPlane].mapSizeHA;
		w*=currentProject->tileC->sizew;
		h*=currentProject->tileC->sizeh;
	}

	unsigned maxRows,firstRow;
	int spRow=currentProject->fixedSpirtePalRow();
	if(spRow>=0&&set->sprite){
		maxRows=spRow+1;
		firstRow=spRow;
		if(!set->useRow[firstRow]){
			fl_alert("No rows specified");
			return;
		}
	}else{
		maxRows=currentProject->pal->getMaxRows(set->sprite);
		for(firstRow=0;(firstRow<maxRows)&&(!set->useRow[firstRow]);++firstRow);
		if(firstRow>=maxRows){
			fl_alert("No rows specified");
			return;
		}
	}
	unsigned rows=0;
	for(unsigned i=firstRow;i<maxRows;++i){
		if(set->useRow[i])
			++rows;
	}
	printf("First row: %u\n",firstRow);
	uint8_t found_colors[768];
	int rowAuto;
	if((rows==1)&&(!set->sprite))
		rowAuto = set->rowAuto;
	else if(!set->sprite){
		rowAuto = set->rowAuto;
		if(rowAuto<0)
			return;
	}else{
		puts("rowAuto forced to zero");
		rowAuto=0;
	}
	pushPaletteAll();//Save the old palette
	Fl_Window *win=0;
	Fl_Progress *progress=0;
	mkProgress(&win,&progress);
	image = (uint8_t *)malloc(w*h*3);
	if (rows==1){
		if (rowAuto)
			currentProject->tms->maps[currentProject->curPlane].allRowSet(firstRow);
		if(set->sprite){
			unsigned off=set->off[firstRow]+(firstRow*currentProject->pal->perRow);
			if(currentProject->gameSystem==NES)
				off+=16;
			reduceImage(image,found_colors,-1,off,progress,win,set->perRow[firstRow],set->colSpace,set->alg,true);
		}else
			reduceImage(image,found_colors,-1,firstRow*currentProject->pal->perRow+set->off[firstRow],progress,win,set->perRow[firstRow],set->colSpace,set->alg);
		if(window){
			window->damage(FL_DAMAGE_USER1);
			Fl::check();
		}
	}else{
		if(rowAuto==2){
			unsigned coltarget=0;
			for(unsigned i=0;i<currentProject->pal->getMaxRows(set->sprite);++i)
				coltarget+=set->perRow[i];	
			reduceImage(image,found_colors,-1,0,progress,win,coltarget,set->colSpace,set->alg);
			currentProject->tms->maps[currentProject->curPlane].pickRowDelta(true,progress,set->rowAutoEx[0],set->rowAutoEx[1]);
			if(window){
				window->damage(FL_DAMAGE_USER1);
				Fl::check();
			}
		}else{
			if (rowAuto==1)
				currentProject->tms->maps[currentProject->curPlane].pickRow(maxRows-firstRow,set->rowAutoEx[0],set->rowAutoEx[1]);
			else if(rowAuto==3)
				currentProject->tms->maps[currentProject->curPlane].pickTileRowQuantChoice(maxRows-firstRow);
			for (unsigned i=firstRow;i<maxRows;++i){
				if(set->useRow[i]){
					reduceImage(image,found_colors,i,(i*currentProject->pal->perRow)+set->off[i],progress,win,set->perRow[i],set->colSpace,set->alg);
					if(window){
						window->damage(FL_DAMAGE_USER1);
						Fl::check();
					}
				}
			}
		}
	}
	free(image);
	if(progress){
		win->remove(progress);// remove progress bar from window
		delete(progress);// deallocate it
	}
	if(win)
		delete win;
	palBar.updateSliders();
	if(set->ditherAfter){
		if(set->sprite){
			ditherSpriteAsImage(window->metaspritesel->value(),curSpritegroup);
		}else{
			pushTilesAll(tTypeTile);
			currentProject->tms->maps[currentProject->curPlane].ditherAsImage(set->entireRow);
		}
	}
	if(window){
		window->redraw();
		Fl::check();
	}
}
static Fl_Window*winG;
struct settings*setG;
static Fl_Int_Input*perrow[MAX_ROWS_PALETTE];
static Fl_Int_Input*perrowoffset[MAX_ROWS_PALETTE];
static Fl_Check_Button*useRowCbtn[MAX_ROWS_PALETTE];
static void setValInt(Fl_Int_Input*i,unsigned val){
	char tmp[16];
	snprintf(tmp,16,"%d",val);
	i->value(tmp);
	winG->redraw();
}
static void setPerRow(Fl_Widget*w,void*x){
	uintptr_t which=(uintptr_t)x;
	unsigned val=SafeTxtInput((Fl_Int_Input*)w,false);
	if((val+setG->off[which])>currentProject->pal->getPerRow(setG->sprite)){
		val=currentProject->pal->getPerRow(setG->sprite)-setG->off[which];
		setValInt((Fl_Int_Input*)w,val);
	}
	setG->perRow[which]=val;
}
static void setPerRowoff(Fl_Widget*w,void*x){
	uintptr_t which=(uintptr_t)x;
	unsigned val=SafeTxtInputZeroAllowed((Fl_Int_Input*)w,false);
	if(val>=currentProject->pal->getPerRow(setG->sprite)){
		val=currentProject->pal->getPerRow(setG->sprite)-1;
		setValInt((Fl_Int_Input*)w,val);
	}
	setG->off[which]=val;
	if((val+setG->perRow[which])>currentProject->pal->getPerRow(setG->sprite)){
		setG->perRow[which]=currentProject->pal->getPerRow(setG->sprite)-val;
		setValInt(perrow[which],setG->perRow[which]);
	}
}
static void doneCB(Fl_Widget*,void*){
	winG->hide();
}
static void toggleBoolCB(Fl_Widget*o,void*ptr){
	bool*val=(bool*)ptr;
	Fl_Check_Button*c=(Fl_Check_Button*)o;
	*val=!!c->value();
}
static Fl_Choice*palchoice;
static Fl_Choice*deltaalgchoice,*huealgchoice;
static Fl_Choice*deltaorderchoice,*huemethodchoice;
static Fl_Check_Button*forceBtn;
static void rowShowCB(Fl_Widget*,void*x){
	uintptr_t which=(uintptr_t)x;
	setG->useRow[which]^=true;
	if(setG->useRow[which]){
		perrow[which]->show();
		perrowoffset[which]->show();
	}else{
		perrow[which]->hide();
		perrowoffset[which]->hide();
	}
	unsigned rowCnt=0;
	int spRow=setG->sprite?currentProject->fixedSpirtePalRow():-1;
	for(unsigned i=0;i<currentProject->pal->getMaxRows(setG->sprite);++i){
		if(spRow>=0&&i!=spRow)
			continue;
		if(setG->useRow[i])
			++rowCnt;
	}
	if(rowCnt<=1){
		palchoice->hide();
		forceBtn->show();
		setG->rowAuto=!!forceBtn->value();
	}else{
		palchoice->show();
		forceBtn->hide();
		setG->rowAuto=palchoice->value();
	}
}
static const Fl_Menu_Item colorReducationChoices[]={
	{"Dennis Lee v3",0,0,0},
	{"Scolorq",0,0,0},
	{"Neuquant",0,0,0},
	{"Wu",0,0,0},
	{"Dennis Lee v1",0,0,0},
	{0}
};
static const Fl_Menu_Item colorSpaceChoices[]={
	{"RGB",0,0,0},
	{"YUV",0,0,0},
	{"YCbCr",0,0,0},
	{0}
};
static const Fl_Menu_Item palChoices[]={
	{"Don't change anything",0,0,0},
	{"Pick based on hue",0,0,0},
	{"Generate contiguous palette then pick based on delta",0,0,0},
	{"Quantizer's choice",0,0,0},
	{0}
};
static void setParmChoiceCB(Fl_Widget*w,void*in){
	unsigned*val=(unsigned*)in;
	Fl_Choice*c=(Fl_Choice*)w;
	*val=c->value();
}
static void setPalChoiceCB(Fl_Widget*w,void*in){
	unsigned*val=(unsigned*)in;
	Fl_Choice*c=(Fl_Choice*)w;
	*val=c->value();
	if(*val==1){
		huealgchoice->show();
		huealgchoice->value(0);
		huemethodchoice->show();
		huemethodchoice->value(0);
	}else{
		huealgchoice->hide();
		huemethodchoice->hide();
	}
	if(*val==2){
		deltaalgchoice->show();
		deltaalgchoice->value(0);
		deltaorderchoice->show();
		deltaorderchoice->value(0);
	}else{
		deltaalgchoice->hide();
		deltaorderchoice->hide();
	}
	if(*val==1||*val==2)
		memset(setG->rowAutoEx,0,sizeof(setG->rowAutoEx));
	
}
void generate_optimal_palette(Fl_Widget*,void*sprite){
	if(!currentProject->containsData(pjHavePal)){
		currentProject->haveMessage(pjHavePal);
		return;
	}
	static bool openAlready;
	if(openAlready){
		fl_alert("Window already open");
		return;
	}
	openAlready=true;
	struct settings set;
	setG=&set;
	memset(&set,0,sizeof(struct settings));
	set.sprite=!!sprite;
	set.ditherAfter=set.entireRow=true;
	winG = new Fl_Window(450,300,"Palette generation settings");
	winG->begin();
	Fl_Box*rowlabel1=new Fl_Box(24,8,96,12,"Colors per row:");
	rowlabel1->labelsize(12);
	Fl_Box*rowlabel2=new Fl_Box(120,8,96,12,"Offset per row:");
	rowlabel2->labelsize(12);
	int spRow=sprite?currentProject->fixedSpirtePalRow():-1;
	unsigned rowCnt=0;
	for(unsigned i=0;i<currentProject->pal->getMaxRows(!!sprite);++i){
		if(spRow>=0&&i!=spRow)
			continue;
		set.useRow[i]=true;
		char tmp[16];
		useRowCbtn[i]=new Fl_Check_Button(8,28+(i*24),24,16);
		useRowCbtn[i]->set();
		useRowCbtn[i]->callback(rowShowCB,(void*)(uintptr_t)i);
		perrow[i]=new Fl_Int_Input(40,24+(i*24),64,24);
		perrowoffset[i]=new Fl_Int_Input(136,24+(i*24),64,24);
		snprintf(tmp,sizeof(tmp),"%d",i);
		perrow[i]->copy_label(tmp);
		perrowoffset[i]->copy_label(tmp);
		perrow[i]->callback(setPerRow,(void*)(uintptr_t)i);
		perrowoffset[i]->callback(setPerRowoff,(void*)(uintptr_t)i);
		snprintf(tmp,sizeof(tmp),"%d",currentProject->pal->getPerRow(set.sprite));
		set.perRow[i]=currentProject->pal->getPerRow(set.sprite);
		perrow[i]->value(tmp);
		perrowoffset[i]->value("0");
		perrow[i]->when(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY);
		perrowoffset[i]->when(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY);
		++rowCnt;
	}
	Fl_Button*preview=new Fl_Button(86,268,56,24,"Preview");
	preview->callback(generate_optimal_paletteapply,(void*)&set);
	Fl_Button*done=new Fl_Button(264,268,48,24,"Done");
	done->callback(doneCB,(void*)&set);
	Fl_Check_Button*dit=new Fl_Check_Button(216,2,176,24,"Dither after completion");
	dit->set();
	dit->callback(toggleBoolCB,&set.ditherAfter);

	Fl_Choice*algsel=new Fl_Choice(240,40,176,24,"Color reduction algorithm:");
	algsel->align(FL_ALIGN_TOP);
	algsel->copy(colorReducationChoices);
	algsel->callback(setParmChoiceCB,(void*)&set.alg);

	Fl_Choice*colsel=new Fl_Choice(240,80,176,24,"Using colorspace:");
	colsel->align(FL_ALIGN_TOP);
	colsel->copy(colorSpaceChoices);
	colsel->callback(setParmChoiceCB,(void*)&set.colSpace);

	palchoice=new Fl_Choice(240,120,176,24,"Palette setting:");
	palchoice->tooltip("How would you like the palette map to be handled");
	palchoice->align(FL_ALIGN_TOP);
	palchoice->copy(palChoices);
	palchoice->callback(setPalChoiceCB,(void*)&set.rowAuto);
	if(rowCnt<=1)
		palchoice->hide();

	forceBtn=new Fl_Check_Button(216,104,176,24,"For all tiles to selected row");
	forceBtn->callback(toggleBoolCB,&set.rowAuto);
	if(rowCnt>1)
		forceBtn->hide();

	huealgchoice=new Fl_Choice(240,160,176,24,hueLblSel);
	huealgchoice->tooltip(hueTooltip);
	huealgchoice->align(FL_ALIGN_TOP);
	huealgchoice->copy(hueChoices);
	huealgchoice->callback(setParmChoiceCB,(void*)&set.rowAutoEx[0]);
	huealgchoice->hide();

	huemethodchoice=new Fl_Choice(240,200,176,24,hueMethodLbl);
	huemethodchoice->tooltip(hueMethodTooltip);
	huemethodchoice->align(FL_ALIGN_TOP);
	huemethodchoice->copy(hueMethodChoices);
	huemethodchoice->callback(setParmChoiceCB,(void*)&set.rowAutoEx[1]);
	huemethodchoice->hide();
	
	deltaalgchoice=new Fl_Choice(240,160,176,24,deltaAlgLbl);
	deltaalgchoice->tooltip(deltaAlgTooltip);
	deltaalgchoice->align(FL_ALIGN_TOP);
	deltaalgchoice->copy(deltaChoices);
	deltaalgchoice->callback(setParmChoiceCB,(void*)&set.rowAutoEx[0]);
	deltaalgchoice->hide();

	deltaorderchoice=new Fl_Choice(240,200,176,24,deltaOrderLbl);
	deltaorderchoice->tooltip(deltaOrderTooltip);
	deltaorderchoice->align(FL_ALIGN_TOP);
	deltaorderchoice->copy(deltaOrderChoices);
	deltaorderchoice->callback(setParmChoiceCB,(void*)&set.rowAutoEx[1]);
	deltaorderchoice->hide();

	winG->end();
	winG->show();
	while(winG->shown())
		Fl::wait();
	delete winG;
	openAlready=false;
}
