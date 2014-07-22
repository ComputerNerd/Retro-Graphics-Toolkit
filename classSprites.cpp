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
void sprites::importImg(uint32_t to){
	
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
