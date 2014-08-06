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
#include <stdint.h>
#include "classSprite.h"
#include "project.h"
#include "class_tiles.h"
sprite::sprite(){
	w=h=1;
	starttile=0;
	palrow=0;
	hflip=vflip=false;
}
sprite::sprite(uint32_t wi,uint32_t hi,uint32_t palrowset,uint32_t settile,bool hf,bool vf){
	w=wi;
	h=hi;
	palrow=palrowset;
	starttile=settile;
	hflip=hf;
	vflip=vf;
}
void sprite::draw(unsigned x,unsigned y,unsigned zoom){
	unsigned yy=y;
	int32_t t=starttile;
	if(hflip)
		t+=(w-1)*h;
	for(unsigned i=0;i<w;++i){//Width and height are swapped due to the way sega genesis stores sprites. The code is the same for NES because width will always be one
		if(vflip)
			t+=h-1;
		for(unsigned j=0;j<h;++j){
			currentProject->tileC->draw_tile(x,yy,t,zoom,palrow,hflip,vflip);
			yy+=currentProject->tileC->sizeh*zoom;
			if(vflip)
				--t;
			else
				++t;
		}
		if(vflip)
			t+=h+1;
		if(hflip)
			t-=h*2;
		x+=currentProject->tileC->sizew*zoom;
		yy=y;
	}
}
