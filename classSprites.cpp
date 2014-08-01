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
static const char*spriteDefName="DefaultSpriteGroupLabel";
sprites::sprites(){
	amt=1;
	groups.push_back(spriteGroup());
	groups[0].list.push_back(sprite());
	groups[0].name.assign(spriteDefName);
	groups[0].offx.push_back(0);
	groups[0].offy.push_back(0);
	groups[0].loadat.push_back(0);
}
sprites::sprites(const sprites& other){
	groups.reserve(other.groups.size());
	for(uint32_t i=0;i<other.groups.size();++i)
		groups.push_back(spriteGroup());
	for(uint32_t j=0;j<other.groups.size();++j){
		groups[j].list.reserve(other.groups[j].list.size());
		for(uint32_t i=0;i<other.groups[j].list.size();++i)
			groups[j].list.push_back(sprite(other.groups[j].list[i].w,other.groups[j].list[i].h,other.groups[j].list[i].palrow,other.groups[j].list[i].starttile));
	}
	amt=other.amt;
}
sprites::~sprites(){
	for(uint32_t j=0;j<amt;++j){
		groups[j].list.clear();
		groups[j].offx.clear();
		groups[j].offy.clear();
		groups[j].list.clear();
		groups[j].name.clear();
	}
	groups.clear();
}

void sprites::spriteGroupToImage(uint8_t*img,uint32_t id,int row,bool alpha){
	fl_alert("sprite to image");
}
void sprites::spriteImageToTiles(uint8_t*img,uint32_t id,int rowUsage,bool alpha){
	fl_alert("image to sprite");
}
uint32_t sprites::width(uint32_t id){
	fl_alert("Width");
}
uint32_t sprites::height(uint32_t id){
	fl_alert("Height");
}
void sprites::draw(uint32_t id,uint32_t x,uint32_t y,int32_t zoom){
	for(uint32_t i=0;i<groups[id].list.size();++i){
		groups[id].list[i].draw(x+(groups[id].offx[i]*zoom),y+(groups[id].offy[i]),zoom);
	}
}
bool sprites::save(FILE*fp){
	/* Format:
	 * uint32_t group amount
	 * for each group
	 * uint32_t sprite amount
	 * Null terminated sprite group name or just 0 if default name
	 * for each sprite in group
	 * int32_t offset x
	 * int32_t offset y
	 * uint32_t loadat
	 * uint32_t w
	 * uint32_t h
	 * uint32_t starttile
	 * uint32_t pal row
	 */
	fwrite(&amt,sizeof(uint32_t),1,fp);
	for(unsigned n=0;n<amt;++n){
		uint32_t amtgroup=groups[n].list.size();
		fwrite(&amtgroup,sizeof(uint32_t),1,fp);
		if(strcmp(groups[n].name.c_str(),spriteDefName))
			fputs(groups[n].name.c_str(),0);
		fputc(0,fp);
		for(uint32_t i=0;i<amtgroup;++i){
			fwrite(&groups[n].offx[i],sizeof(int32_t),1,fp);
			fwrite(&groups[n].offy[i],sizeof(int32_t),1,fp);
			fwrite(&groups[n].loadat[i],sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].w,sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].h,sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].starttile,sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].palrow,sizeof(uint32_t),1,fp);
		}
	}
	return true;
}
void sprites::setAmt(uint32_t amtnew){
	if(amtnew>amt){
		//Create more sprites with default paramater
		groups.resize(amtnew);
		for(unsigned n=amt;n<amtnew;++n){
			groups[n].list.push_back(sprite());
			groups[n].name.assign(spriteDefName);
		}
	}else if(amtnew<amt){
		for(unsigned n=amtnew;n<amt;++n)
			groups[n].list.clear();
		groups.resize(amtnew);
	}
	amt=amtnew;
}
void sprites::setAmtgroup(uint32_t id,uint32_t amtnew){
	uint32_t amtold=groups[id].list.size();
	if(amtold==amtnew)
		return;
	groups[id].list.resize(amtnew);
	groups[id].offx.resize(amtnew);
	groups[id].offy.resize(amtnew);
	groups[id].loadat.resize(amtnew);
	if(amtnew>amtold){
		for(unsigned i=amtold;i<amtnew;++i)
			groups[id].name.assign(spriteDefName);
	}
}
bool sprites::load(FILE*fp,uint32_t version){
	if(version>=7){

	}else{
		/* Old format
		 * uint32_t amount
		 * And for each sprite:
		 * uint32_t width
		 * uint32_t height
		 * uint32_t start tile
		 * uint32_t pal row*/
		uint32_t amtnew;
		fread(&amtnew,sizeof(uint32_t),1,fp);
		setAmt(amtnew);
		for(unsigned n=0;n<amt;++n){
			fread(&groups[n].list[0].w,sizeof(uint32_t),1,fp);
			fread(&groups[n].list[0].h,sizeof(uint32_t),1,fp);
			fread(&groups[n].list[0].starttile,sizeof(uint32_t),1,fp);
			fread(&groups[n].list[0].palrow,sizeof(uint32_t),1,fp);
		}
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
		if(to>=amt)
			setAmt(to+1);
		if((loaded_image->d() != 3 && loaded_image->d() != 4)){
			fl_alert("Please use color depth of 3 or 4\nYou Used %d",loaded_image->d());
			loaded_image->release();
			return;
		}else
			printf("Image depth %d\n",loaded_image->d());
		unsigned startTile=currentProject->tileC->amt-1;
		uint8_t*out=currentProject->tileC->truetDat.data()+(startTile*256);
		unsigned newTiles=(wnew/8)*(hnew/8);
		//See if tile is blank
		bool overwrite=false;//This is to avoid duplicate code otherwise there would be the need for two else statments with identical code
		if(numCmp(out,256,0)){
			if(fl_ask("Tile %d detected as blank overwrite?",startTile))
				overwrite=true;
		}
		if(overwrite){
			currentProject->tileC->amt+=newTiles-1;
		}else{
			currentProject->tileC->amt+=newTiles;
			++startTile;
		}
		//set new amount
		currentProject->tileC->resizeAmt();
		out=currentProject->tileC->truetDat.data()+(startTile*currentProject->tileC->tcSize);
		uint8_t * img_ptr=(uint8_t *)loaded_image->data()[0];
		setAmtgroup(to,spritesnew);
		groups[to].name.assign(fl_filename_name(the_file.c_str()));
		for(unsigned y=0,cnt=0,tilecnt=startTile;y<hnew;y+=hmax){
			for(unsigned x=0;x<wnew;x+=wmax,++cnt){
				unsigned dimx,dimy;
				dimx=((wnew-x)>=wmax)?wmax:(wnew-x)%wmax;
				dimy=((hnew-y)>=hmax)?hmax:(hnew-y)%hmax;
				groups[to].list[cnt].w=dimx/8;
				groups[to].list[cnt].h=dimy/8;
				groups[to].list[cnt].starttile=tilecnt;
				groups[to].offx[cnt]=x;
				groups[to].offy[cnt]=y;
				groups[to].loadat[cnt]=tilecnt;
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
		groups[id].offx.erase(groups[id].offx.begin()+id);
		groups[id].offy.erase(groups[id].offy.begin()+id);
		groups[id].loadat.erase(groups[id].loadat.begin()+id);
		groups[id].name.erase(groups[id].name.begin()+id);
		groups[id].list.erase(groups[id].list.begin()+id);
		groups.erase(groups.begin()+id);
	}else
		fl_alert("You cannot delete what does not exist");
}
void sprites::enforceMax(unsigned wmax,unsigned hmax){
	for(unsigned j=0;j<amt;++j){
		for(unsigned i=0;i<groups[j].list.size();++i){
		if(groups[j].list[i].w>wmax)
			groups[j].list[i].w=wmax;
		if(groups[j].list[i].h>hmax)
			groups[j].list[i].h=hmax;
		}
	}
}
