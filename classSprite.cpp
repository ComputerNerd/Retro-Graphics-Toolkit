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
}
sprite::sprite(uint32_t wi,uint32_t hi,uint32_t palrowset,uint32_t settile){
	w=wi;
	h=hi;
	palrow=palrowset;
	starttile=settile;
}
void sprite::draw(unsigned x,unsigned y,unsigned zoom){
	unsigned yy=y;
	unsigned t=starttile;
	for(unsigned i=0;i<w;++i){//This is backwards due to the way sega genesis stores sprites. The code is the same for NES because height will always be one
		for(unsigned j=0;j<h;++j){
			currentProject->tileC->draw_tile(x,yy,t++,zoom,palrow,false,false);
			yy+=currentProject->tileC->sizey*zoom;
		}
		x+=currentProject->tileC->sizex*zoom;
		yy=y;
	}
}
