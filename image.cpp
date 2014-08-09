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
#include <FL/Fl_Color_Chooser.H>
void getMaskColorImg(Fl_Shared_Image*loaded_image,bool grayscale,unsigned*remap,uint8_t*palMap,uint8_t*mask){
	unsigned depth=loaded_image->d();
	uint64_t*histr=0,*histg=0,*histb=0,*hista=0;
	unsigned w=loaded_image->w(),h=loaded_image->h();
	unsigned imgb=w*h;
	uint8_t*imgptr;
	double rc,gc,bc;
	uint8_t r=0,g=0,b=0;
	histr=(uint64_t*)calloc(sizeof(uint64_t),256);
	if(((depth==1)&&(!grayscale))||(depth==4))
		hista=(uint64_t*)calloc(sizeof(uint64_t),256);
	if(depth>1){
		histg=(uint64_t*)calloc(sizeof(uint64_t),256);
		histb=(uint64_t*)calloc(sizeof(uint64_t),256);
	}
	bool maskTrans=false;//Detect if the image is using a mask color or transparent data.
	switch(depth){
		case 4:
			imgptr=(uint8_t*)loaded_image->data()[0];
			for(unsigned i=0;i<imgb;++i){
				++histr[*imgptr++];
				++histg[*imgptr++];
				++histb[*imgptr++];
				++hista[*imgptr++];
			}
			{uint64_t maxr=*histr,maxg=*histg,maxb=*histb,*ptrr=histr,*ptrg=histg,*ptrb=histb;
			for(unsigned i=0;i<255;++i,++ptrr,++ptrg,++ptrb){
				if(maxr<*ptrr){
					maxr=*ptrr;
					r=i;
				}
				if(maxg<*ptrg){
					maxg=*ptrg;
					g=i;
				}
				if(maxb<*ptrb){
					maxb=*ptrb;
					b=i;
				}
			}
			}
		break;
		case 3:
			imgptr=(uint8_t*)loaded_image->data()[0];
			for(unsigned i=0;i<imgb;++i){
				++histr[*imgptr++];
				++histg[*imgptr++];
				++histb[*imgptr++];
			}
			{uint64_t maxr=*histr,maxg=*histg,maxb=*histb,maxa=*hista,*ptrr=histr,*ptrg=histg,*ptrb=histb,*ptra=hista;
			unsigned alpha=0;
			for(unsigned i=0;i<255;++i,++ptrr,++ptrg,++ptrb,++ptra){
				if(maxr<*ptrr){
					maxr=*ptrr;
					r=i;
				}
				if(maxg<*ptrg){
					maxg=*ptrg;
					g=i;
				}
				if(maxb<*ptrb){
					maxb=*ptrb;
					b=i;
				}
				if(maxa<*ptra){
					maxa=*ptra;
					alpha=i;
				}
			}
			maskTrans=(alpha)?false:true;
			}
		break;
		case 1:
			if(grayscale){
				imgptr=(uint8_t*)loaded_image->data()[0];
				for(unsigned i=0;i<imgb;++i)
					++histr[*imgptr++];
			}else{
				for(unsigned y=0;y<h;++y){
					imgptr=(uint8_t*)loaded_image->data()[y+2];
					for(unsigned x=0;x<w;++x){
						if(*imgptr==' '){
							++hista[0];
						}else{
							++histr[*imgptr++];
							++hista[255];
						}
					}
				}
			}
			{uint64_t maxv=*histr,*ptrv=histr;
			unsigned ent=0;
			if(grayscale){
				for(unsigned i=0;i<255;++i,++ptrv){
					if(maxv<*ptrv){
						maxv=*ptrv;
						ent=i;
					}
				}
				r=g=b=ent;
			}else{
				uint64_t maxa=*hista,*ptra=hista;
				unsigned alpha=0;
				for(unsigned i=0;i<255;++i,++ptrv,++ptra){
					if(maxv<*ptrv){
						maxv=*ptrv;
						ent=i;
					}
					if(maxa<*ptra){
						maxa=*ptra;
						alpha=i;
					}
				}
				maskTrans=(alpha)?false:true;
				r=palMap[remap[ent]+1];
				g=palMap[remap[ent]+2];
				b=palMap[remap[ent]+3];
			}
			}
		break;
	}
	rc=(double)r/255.0;
	gc=(double)g/255.0;
	bc=(double)b/255.0;
	if(maskTrans)
		maskTrans=fl_ask("This image was detected to be using alpha instead of mask color is it true?");
	fl_color_chooser("Background color",rc,gc,bc,0);
	mask[0]=r;
	mask[1]=g;
	mask[2]=b;
	if(histr)
		free(histr);
	if(histg)
		free(histg);
	if(histb)
		free(histb);
	if(hista)
		free(hista);
}
bool handle1byteImg(Fl_Shared_Image*loaded_image,unsigned*remap){
	char*timgptr=(char*)loaded_image->data()[0];
	//See if grayscale or colormapped xpm
	if(isdigit(*timgptr)){
		/*Checking to see if the first byte is a digit is not enough.
		  What if the first pixel just happen to fall in digit range?
		  Avoid this by verifing width and height*/
		if(strtol(timgptr,&timgptr,10)==loaded_image->w()){
			if(strtol(timgptr,&timgptr,10)==loaded_image->h()){
				int numcolors;
				numcolors=abs(strtol(timgptr,&timgptr,10));
				uint8_t*palMap=(uint8_t*)loaded_image->data()[1];
				std::fill(remap,remap+256,0);
				for(unsigned xx=0;xx<numcolors*4;xx+=4)
					remap[palMap[xx]]=xx;
				return false;
			}else
				return true;
		}else
			return true;

	}else
		return true;
}
