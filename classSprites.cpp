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
#include "classSprite.h"
#include "classSprites.h"
sprites::sprites(){
	amt=1;
	spriteslist=(sprite**)malloc(sizeof(sprite*));
	spriteslist[0]=new sprite;
}
sprites::~sprites(){
	for(uint32_t x=0;x<amt;++x)
		delete spriteslist[x];
	free(spriteslist);
}
		
