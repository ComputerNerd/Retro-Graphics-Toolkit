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
#include "project.h"
#include "undo.h"
static struct undoEvent*undoBuf;
static uint_fast32_t amount;
static uint_fast32_t memUsed;
static uint_fast32_t maxMen=16*1024*1024;//Limit undo buffer to 16Mb this is better than limiting by depth as each item varies in size
static void pushEventPrepare(void){
	++amount;
	if(undoBuf)
		undoBuf=(struct undoEvent*)realloc(undoBuf,amount*sizeof(struct undoEvent));
	else
		undoBuf=(struct undoEvent*)malloc(sizeof(struct undoEvent));
	memUsed+=sizeof(struct undoEvent);
}
void popUndo(void){
	struct undoEvent*uptr=undoBuf+amount-1;
	switch(uptr->type){
		case uTile:
			{struct undoTile*ut=(struct undoTile*)uptr->ptr;
			if(ut->type&tTypeDelete){

			}}
		break;

	}
}
void pushTile(uint32_t id,tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+amount-1;
	uptr->type=uTile;
	uptr->ptr=malloc(sizeof(struct undoTile));
	memUsed+=sizeof(struct undoTile);
	struct undoTile*ut=(struct undoTile*)uptr->ptr;
	uint32_t sz=0;
	if(type&tTypeTile)
		sz+=currentProject->tileC->tileSize;
	if(type&tTypeTruecolor)
		sz+=currentProject->tileC->tcSize;
	ut->ptr=malloc(sz);
	ut->id=id;
	memUsed+=sz;
}
void pushTilePixel(uint32_t tile,uint32_t x,uint32_t y,uint32_t type){

}
