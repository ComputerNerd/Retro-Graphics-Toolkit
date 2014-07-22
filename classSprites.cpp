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
#include <stdlib.h>
#include <string.h>
#include "classSprite.h"
#include "classSprites.h"
#include "includes.h"
#include "callback_tiles.h"
#include "global.h"
sprites::sprites(){
	amt=1;
	spriteslist=(sprite**)malloc(sizeof(sprite*));
	spriteslist[0]=new sprite;
}
sprites::sprites(const sprites& other){
	spriteslist=(sprite**)malloc(other.amt*sizeof(sprite*));
	for(uint32_t i=0;i<other.amt;++i)
		spriteslist[i]=new sprite(other.spriteslist[i]->w,other.spriteslist[i]->h,other.spriteslist[i]->starttile,other.spriteslist[i]->palrow);
	amt=other.amt;
}
sprites::~sprites(){
	for(uint32_t x=0;x<amt;++x)
		delete spriteslist[x];
	free(spriteslist);
}
bool sprites::save(FILE*fp){
	/*Format
	 * uint32_t amount
	 * And for each sprite:
	 * uint32_t width
	 * uint32_t height
	 * uint32_t start tile
	 * uint32_t pal row*/
	fwrite(&amt,sizeof(uint32_t),1,fp);
	for(unsigned n=0;n<amt;++n){
		fwrite(&spriteslist[n]->w,sizeof(uint32_t),1,fp);
		fwrite(&spriteslist[n]->h,sizeof(uint32_t),1,fp);
		fwrite(&spriteslist[n]->starttile,sizeof(uint32_t),1,fp);
		fwrite(&spriteslist[n]->palrow,sizeof(uint32_t),1,fp);
	}
	return true;
}
void sprites::setAmt(uint32_t amtnew){
	if(amtnew>amt){
		//Create more sprites with default paramater
		spriteslist=(sprite**)realloc(spriteslist,amtnew*sizeof(sprite*));
		for(unsigned n=amt;n<amtnew;++n)
			spriteslist[n]=new sprite;
	}else if(amtnew<amt){
		for(unsigned n=amtnew;n<amt;++n)
			delete spriteslist[n];
		spriteslist=(sprite**)realloc(spriteslist,amtnew*sizeof(sprite*));
	}
	amt=amtnew;
}
bool sprites::load(FILE*fp){
	uint32_t amtnew;
	fread(&amtnew,sizeof(uint32_t),1,fp);
	setAmt(amtnew);
	for(unsigned n=0;n<amt;++n){
		fread(&spriteslist[n]->w,sizeof(uint32_t),1,fp);
		fread(&spriteslist[n]->h,sizeof(uint32_t),1,fp);
		fread(&spriteslist[n]->starttile,sizeof(uint32_t),1,fp);
		fread(&spriteslist[n]->palrow,sizeof(uint32_t),1,fp);
	}
	return true;
}
static uint8_t*rect2rect(uint8_t*in,uint8_t*out,unsigned xin,unsigned yin,unsigned win,unsigned wout,unsigned hout,bool alpha){
	if(alpha)
		in+=(yin*win*4)+(xin*4);
	else
		in+=(yin*win*3)+(xin*3);
	while(hout--){
		if(alpha){
			memcpy(out,in,wout*4);
			in+=win*4;
			out+=wout*4;
		}else{
			for(unsigned i=0;i<wout;++i){
				*out++=*in++;
				*out++=*in++;
				*out++=*in++;
				*out++=255;
			}
			in+=(win-wout)*3;
		}
	}
	return out;
}
static int numCmp(uint8_t*dat,unsigned n,uint8_t num){
	while(n--){
		if(*dat++!=num)
			return 0;
	}
	return 1;
}
void sprites::importImg(uint32_t to){
	if(load_file_generic()){
		Fl_Shared_Image * loaded_image=Fl_Shared_Image::get(the_file.c_str());
		if(!loaded_image){
			fl_alert("Error loading image");
			return;
		}
		uint32_t wnew,hnew;
		wnew=loaded_image->w();
		hnew=loaded_image->h();
		uint32_t wmax,hmax;
		switch(currentProject->gameSystem){
			case sega_genesis:
				wmax=hmax=32;
			break;
			case NES:
				wmax=8;
				hmax=16;
			break;
		}
		if((wnew&7)||(hnew&7)){
			fl_alert("%d or %d are not a multiple of 8",wnew,hnew);
			loaded_image->release();
			return;
		}
		//Determin how many sprites will be created
		unsigned spritesnew=((wnew+wmax-8)/wmax)*((hnew+hmax-8)/hmax);
		if((int)to>((int)amt-(int)spritesnew))
			setAmt(to+spritesnew);
		if((loaded_image->d() != 3 && loaded_image->d() != 4)){
			fl_alert("Please use color depth of 3 or 4\nYou Used %d",loaded_image->d());
			loaded_image->release();
			return;
		}else
			printf("Image depth %d\n",loaded_image->d());
		unsigned startTile=currentProject->tileC->tiles_amount;
		uint8_t*out=currentProject->tileC->truetileDat+(startTile*256);
		unsigned newTiles=(wnew/8)*(hnew/8);
		//See if tile is blank
		bool overwrite=false;//This is to avoid duplicate code otherwise there would be the need for two else statments with identical code
		if(numCmp(out,256,0)){
			if(fl_ask("Tile %d detected as blank overwrite?",startTile))
				overwrite=true;
		}
		if(overwrite){
			currentProject->tileC->tiles_amount+=newTiles-1;
		}else{
			currentProject->tileC->tiles_amount+=newTiles;
			++startTile;
		}
		//set new amount
		currentProject->tileC->tileDat=(uint8_t *)realloc(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize);
		currentProject->tileC->truetileDat=(uint8_t *)realloc(currentProject->tileC->truetileDat,(currentProject->tileC->tiles_amount+1)*256);
		out=currentProject->tileC->truetileDat+(startTile*256);
		uint8_t * img_ptr=(uint8_t *)loaded_image->data()[0];
		for(unsigned y=0,cnt=0,tilecnt=startTile;y<hnew;y+=hmax){
			for(unsigned x=0;x<wnew;x+=wmax,++cnt){
				unsigned dimx,dimy;
				dimx=((wnew-x)>=wmax)?wmax:(wnew-x)%wmax;
				dimy=((hnew-y)>=hmax)?hmax:(hnew-y)%hmax;
				spriteslist[to+cnt]->w=dimx/8;
				spriteslist[to+cnt]->h=dimy/8;
				spriteslist[to+cnt]->starttile=tilecnt;
				tilecnt+=(dimx/8)*(dimy/8);
				for(unsigned i=0;i<dimx;i+=8){
					for(unsigned j=0;j<dimy;j+=8)
						out=rect2rect(img_ptr,out,i+x,j+y,wnew,8,8,(loaded_image->d()==4)?true:false);
				}
			}
		}
		loaded_image->release();
		window->updateSpriteSliders();
		updateTileSelectAmt();
		window->redraw();
	}
}
void sprites::del(uint32_t id){
	if(amt<=1){
		fl_alert("If you want no sprites uncheck have sprites instead.");
		return;
	}
	if(id<amt){
		delete spriteslist[id];
		--amt;
		if(id<amt){
			//if not at the end of the list
			memmove(spriteslist+id,spriteslist+id+1,(amt-1)*sizeof(uint32_t));
		}
		spriteslist=(sprite**)realloc(spriteslist,amt*sizeof(sprite*));
	}
}
void sprites::enforceMax(unsigned wmax,unsigned hmax){
	for(unsigned n=0;n<amt;++n){
		if(spriteslist[n]->w>wmax)
			spriteslist[n]->w=wmax;
		if(spriteslist[n]->h>hmax)
			spriteslist[n]->h=hmax;
	}
}
