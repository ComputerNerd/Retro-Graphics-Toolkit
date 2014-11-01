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
#include "palette.h"
#include "classtilemap.h"
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
void tileMap::pickRow(unsigned amount){
	int type=MenuPopup("Pick tile row based on...","Please pick what most defines the image",9,"Hue","Saturation","Lightness","Hue*satuaration","Hue*Lightness","Lightness*saturation","Hue+satuaration","Hue+lightness","Lightness+saturation");
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
void tileMap::pickRowDelta(bool showProgress,Fl_Progress *progress){
	int alg=MenuPopup("Select picking algorithm","Pick which method you think works better for this image.",6,"ciede2000","Weighted","Mean squared error","Hue difference","Saturation difference","Lightness difference");
	if(alg<0)
		return;
	pushTilemapAll(true);
	pushTilesAll(tTypeTile);
	if(fl_ask("Would you like the palette to be ordered by hue or light or saturation")){
		unsigned type=fl_choice("What do you want it ordered by","Hue","Saturation","Lightness");
		sortBy(type,false);
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
	uint8_t **imageout=(uint8_t**)malloc(4*sizeof(void*));
	uint32_t xtile=0,ytile=0;
	if(showProgress){
		progress->maximum(12);
		progress->value(0);
	}
	for(x=0;x<4;++x){//This function has too many hard coded values The four should be a variable with the amount of palette rows
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
			if(alg==2)
				memset(di,0,4*sizeof(uint32_t));
			else{
				for (t=0;t<4;t++)
					d[t]=0.0;
			}
			if ((type_temp != 0) && (currentProject->gameSystem == sega_genesis)){
				tempSet=(currentProject->tileMapC->get_prio(xtile,ytile)^1)*8;
				set_palette_type_force(tempSet);
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
											rgbToHsl255(imagein[a+b+y+x+c+e],imagein[a+b+y+x+1+c+e],imagein[a+b+y+x+2+c+e],h,s,l);
											rgbToHsl255(imageout[t][a+b+y+x+c+e],imageout[t][a+b+y+x+1+c+e],imageout[t][a+b+y+x+2+c+e],h+1,s+1,l+1);
											d[t]+=std::abs(pickIt(h[0],s[0],l[0],alg-3)-pickIt(h[1],s[1],l[1],alg-3));
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
					currentProject->tileC->truecolor_to_tile_ptr(sillyrow,get_tile(xtile+j,ytile+i),temp,false,false);
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
	if(currentProject->gameSystem == sega_genesis)
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
static void reduceImage(uint8_t * image,uint8_t * found_colors,int row,unsigned offsetPal,Fl_Progress *progress,Fl_Window*pwin,unsigned maxCol,unsigned yuv,unsigned alg,bool isSprite=false){
	progress->maximum(1.0);
	unsigned off2=offsetPal*2;
	unsigned off3=offsetPal*3;
	uint32_t colors_found;
	uint32_t w,h;
	unsigned maxPal=(1<<getBitdepthcurSys())*4;
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
				++offsetPal;
				off3+=3;
				off2+=2;
				if(offsetPal>=maxPal){
					colorAmtExceed();
					break;
				}
				goto againFun;
			}
			r=found_colors[(x*3)];
			g=found_colors[(x*3)+1];
			b=found_colors[(x*3)+2];
			printf("R=%d G=%d B=%d\n",r,g,b);
			rgbToEntry(r,g,b,x+offsetPal);
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
		unsigned new_colors = count_colors(rgb_pal2,colorz,1,&rgb_pal3[off3]);
		printf("Unique colors in palette %u\n",new_colors);
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
			pwin->label("Too many colors");
			colorz--;
			goto try_again_color;
		}
		unsigned off3o=off3;
		for (unsigned x=0;x<maxCol;x++){
			uint8_t r,g,b;
againNerd:
			if (currentProject->palType[x+offsetPal]){
				++offsetPal;
				off3+=3;
				off2+=2;
				if(offsetPal>=maxPal){
					colorAmtExceed();
					break;
				}
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
			if(isSprite)
				currentProject->spritesC->spriteImageToTiles(output,curSpritegroup,row,false);
			else
				currentProject->tileMapC->truecolorimageToTiles(output,row,false);
			free(output);
		}
		if(yuv)
			free(imageuse);
	}
}
struct settings{//TODO avoid hardcoding palette row amount
	bool sprite;//Are we generating the palette for a sprite
	unsigned off[4];//Offsets for each row
	unsigned col;//How many colors are to be generated
	unsigned alg;//Which algorithm should be used
	bool ditherAfter;//After color quanization should the image be dithered
	bool entireRow;//If true dither entire tilemap at once or false dither each row sepeartly
	unsigned colSpace;//Which colorspace should the image be quantized in
	unsigned perRow[4];//How many colors will be generated per row
	bool useRow[4];
};
static void generate_optimal_paletteapply(Fl_Widget*,void*s){
	struct settings*set=(struct settings*)s;
	char temp[16];
	uint8_t * image;
	//uint8_t * colors;
	uint32_t w,h;
	if(set->sprite){
		w=currentProject->spritesC->width(curSpritegroup);
		h=currentProject->spritesC->height(curSpritegroup);
	}else{
		w=currentProject->tileMapC->mapSizeW;
		h=currentProject->tileMapC->mapSizeHA;
		w*=currentProject->tileC->sizew;
		h*=currentProject->tileC->sizeh;
	}
	unsigned firstRow=0;
	for(;(firstRow<4)&&(!set->useRow[firstRow]);++firstRow);
	if(firstRow>=4){
		fl_alert("No rows specified");
		return;
	}
	unsigned rows=0;
	for(unsigned i=firstRow;i<4;++i){
		if(set->useRow[i])
			++rows;
	}
	printf("First row: %u\n",firstRow);
	uint32_t colors_found;
	//uint8_t * found_colors;
	uint8_t found_colors[768];
	int rowAuto;
	if((rows==1)&&(!set->sprite))
		rowAuto = fl_ask("Would you like all tiles on the tilemap to be set to row %d? (This is where all generated colors will apear)",firstRow);
	else if(!set->sprite){
		rowAuto = MenuPopup("Palette setting","How would you like the palette map to be handled",4,"Don't change anythin","Pick based on hue","Generate contiguous palette then pick based on delta","Quantizer's choice");
		if(rowAuto<0)
			return;
	}else
		rowAuto=0;
	pushPaletteAll();//Save the old palette
	Fl_Window *win;
	Fl_Progress *progress;
	mkProgress(&win,&progress);
	image = (uint8_t *)malloc(w*h*3);
	if (rows==1){
		if (rowAuto)
			currentProject->tileMapC->allRowSet(firstRow);
		if(set->sprite){
			unsigned off=set->off[firstRow]+(firstRow*palEdit.perRow);
			if(currentProject->gameSystem==NES)
				off+=16;
			reduceImage(image,found_colors,-1,off,progress,win,set->perRow[firstRow],set->colSpace,set->alg,true);
		}else
			reduceImage(image,found_colors,-1,firstRow*palEdit.perRow+set->off[firstRow],progress,win,set->perRow[firstRow],set->colSpace,set->alg);
		window->damage(FL_DAMAGE_USER1);
		Fl::check();
	}else{
		if(rowAuto==2){
			reduceImage(image,found_colors,-1,0,progress,win,set->perRow[0]+set->perRow[1]+set->perRow[2]+set->perRow[3],set->colSpace,set->alg);
			currentProject->tileMapC->pickRowDelta(true,progress);
			window->damage(FL_DAMAGE_USER1);
			Fl::check();
		}else{
			if (rowAuto==1)
				currentProject->tileMapC->pickRow(4-firstRow);
			else if(rowAuto==3)
				currentProject->tileMapC->pickTileRowQuantChoice(4-firstRow);
			for (unsigned i=firstRow;i<4;++i){
				if(set->useRow[i]){
					reduceImage(image,found_colors,i,(i*palEdit.perRow)+set->off[i],progress,win,set->perRow[i],set->colSpace,set->alg);
					window->damage(FL_DAMAGE_USER1);
					Fl::check();
				}
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
	if(set->ditherAfter){
		if(set->sprite){
			ditherSpriteAsImage(curSpritegroup);
		}else{
			pushTilesAll(tTypeTile);
			currentProject->tileMapC->ditherAsImage(set->entireRow);
		}
	}
	window->redraw();
	Fl::check();
}
static Fl_Window*winG;
struct settings*setG;
static Fl_Int_Input*perrow[4];
static Fl_Int_Input*perrowoffset[4];
static Fl_Check_Button*useRowCbtn[4];
static void setValInt(Fl_Int_Input*i,unsigned val){
	char tmp[16];
	snprintf(tmp,16,"%d",val);
	i->value(tmp);
	winG->redraw();
}
static void setPerRow(Fl_Widget*w,void*x){
	uintptr_t which=(uintptr_t)x;
	unsigned val=SafeTxtInput((Fl_Int_Input*)w,false);
	if((val+setG->off[which])>calMaxPerRow(which)){
		val=calMaxPerRow(which)-setG->off[which];
		setValInt((Fl_Int_Input*)w,val);
	}
	setG->perRow[which]=val;
}
static void setPerRowoff(Fl_Widget*w,void*x){
	uintptr_t which=(uintptr_t)x;
	unsigned val=SafeTxtInputZeroAllowed((Fl_Int_Input*)w,false);
	if(val>=calMaxPerRow(which)){
		val=calMaxPerRow(which)-1;
		setValInt((Fl_Int_Input*)w,val);
	}
	setG->off[which]=val;
	if((val+setG->perRow[which])>calMaxPerRow(which)){
		setG->perRow[which]=calMaxPerRow(which)-val;
		setValInt(perrow[which],setG->perRow[which]);
	}
}
static void doneCB(Fl_Widget*,void*){
	winG->hide();
}
static void toggleBoolCB(Fl_Widget*,void*ptr){
	bool*val=(bool*)ptr;
	*val^=true;
}
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
static void setParmChoiceCB(Fl_Widget*w,void*in){
	unsigned*val=(unsigned*)in;
	Fl_Choice*c=(Fl_Choice*)w;
	*val=c->value();
}
void generate_optimal_palette(Fl_Widget*,void*sprite){
	static bool openAlready;
	if(openAlready){
		fl_alert("Window already open");
		return;
	}
	openAlready=true;
	struct settings set;
	setG=&set;
	memset(&set,0,sizeof(struct settings));
	set.sprite=sprite?true:false;
	set.ditherAfter=set.entireRow=true;
	winG = new Fl_Window(400,300,"Palette generation settings");
	winG->begin();
	Fl_Box*rowlabel1=new Fl_Box(24,8,96,12,"Colors per row:");
	rowlabel1->labelsize(12);
	Fl_Box*rowlabel2=new Fl_Box(120,8,96,12,"Offset per row:");
	rowlabel2->labelsize(12);
	for(uintptr_t i=0;i<4;++i){
		set.useRow[i]=true;
		char tmp[16];
		useRowCbtn[i]=new Fl_Check_Button(8,28+(i*24),24,16);
		useRowCbtn[i]->set();
		useRowCbtn[i]->callback(rowShowCB,(void*)i);
		perrow[i]=new Fl_Int_Input(40,24+(i*24),64,24);
		perrowoffset[i]=new Fl_Int_Input(136,24+(i*24),64,24);
		snprintf(tmp,sizeof(tmp),"%d",i);
		perrow[i]->copy_label(tmp);
		perrowoffset[i]->copy_label(tmp);
		perrow[i]->callback(setPerRow,(void*)i);
		perrowoffset[i]->callback(setPerRowoff,(void*)i);
		snprintf(tmp,sizeof(tmp),"%d",calMaxPerRow(i));
		set.perRow[i]=calMaxPerRow(i);
		perrow[i]->value(tmp);
		perrowoffset[i]->value("0");
		perrow[i]->when(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY);
		perrowoffset[i]->when(FL_WHEN_RELEASE|FL_WHEN_ENTER_KEY);
	}
	Fl_Button*preview=new Fl_Button(86,268,56,24,"Preview");
	preview->callback(generate_optimal_paletteapply,(void*)&set);
	Fl_Button*done=new Fl_Button(264,268,48,24,"Done");
	done->callback(doneCB,(void*)&set);
	Fl_Check_Button*dit=new Fl_Check_Button(216,2,176,24,"Dither after completion");
	dit->set();
	dit->callback(toggleBoolCB,&set.ditherAfter);
	Fl_Choice*algsel=new Fl_Choice(240,40,128,24,"Color reducation algorithm:");
	algsel->align(FL_ALIGN_TOP);
	algsel->copy(colorReducationChoices);
	algsel->callback(setParmChoiceCB,(void*)&set.alg);
	Fl_Choice*colsel=new Fl_Choice(240,80,128,24,"Using colorspace:");
	colsel->align(FL_ALIGN_TOP);
	colsel->copy(colorSpaceChoices);
	colsel->callback(setParmChoiceCB,(void*)&set.colSpace);
	winG->end();
	winG->show();
	while(winG->shown())
		Fl::wait();
	delete winG;
	openAlready=false;
}
