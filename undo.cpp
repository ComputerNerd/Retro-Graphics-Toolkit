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
#include <cmath>//For some reason this is needed when compiling with mingw otherwise hypot error is encountered
#include <stdlib.h>
#include <FL/Fl_Browser.H>
#include "system.h"
#include "project.h"
#include "undo.h"
#include "color_convert.h"
#include "callback_chunk.h"
#include "callbacksprites.h"
#include "classpalettebar.h"
#include "callbacktilemaps.h"
enum undoTypes_t{
	uTile=0,
	uTileAll,
	uTileGroup,
	uTilePixel,
	uTileAppend,//No struct
	uTileAppendgroupdat,//uTileGroup could be adapted for this but using separate code ram is saved
	uTileNew,//No struct reuses ptr for insert tile after
	uTilemap,
	uTilemapattr,
	uTilemapEdit,
	uTilemapResize,
	uTilemapBlocksAmt,
	uTilemapPlaneDelete,
	uTilemapPlaneAdd,
	uPalette,
	uPaletteEntry,
	uChunk,
	uChunkDelete,
	uChunkAll,
	uChunkEdit,
	uChunkResize,
	uChunkAppend,
	uChunkNew,//No struct reuses ptr
	uSpriteNew,
	uSpriteNewgroup,//No struct reuses ptr
	uSpriteAppend,//No struct reuses ptr
	uSpriteAppendgroup,//No struct
	uSpriteWidth,
	uSpriteHeight,
	uSpritePalrow,
	uSpritestarttile,
	uSpriteloadat,
	uSpriteoffx,
	uSpriteoffy,
	uSpriteprio,
	uSpritehflip,
	uSpritevflip,
	uSpriteGroupDel,
	uSpriteDel,
	uSpriteAll,
	uCurProject,//For both system switches and project loads
	uSwitchPrj,//No struct reuses ptr
	ULoadPrjGroup,
};
struct undoEvent{//This struct merely holds which type of undo this is
	undoTypes_t type;
	void*ptr;//Can also be reused for information for example appendTile will store tile id if doing so limit yourself to 32bit values. Even if void* is 64bit on your system also can point to pointer created either by malloc or new
};
struct undoTile{//The purpose of this struct if to completely undo a tile
	tileTypeMask_t type;
	uint32_t id;
	void*ptrnew;
	void*ptr;//when type is both first the truecolor tile will be stored then the regular tile
};
struct undoTileAll{
	tileTypeMask_t type;
	uint32_t amt,amtnew;
	void*ptr;
	void*ptrnew;
};
struct undoTileGroup{
	tileTypeMask_t type;
	std::vector<uint32_t> lst;//Contains group of affect tiles
	std::vector<uint8_t> data;//Similar situation to other tile structs as in what this contains and what order
	std::vector<uint8_t> datanew;
};
struct undoAppendgroupdat{
	uint32_t amt;
	std::vector<uint8_t> dat;
	std::vector<uint8_t> truedat;
};
struct undoTilePixel{
	tileTypeMask_t type;
	uint32_t id,x,y,val,valnew;
};
struct undoTilemap{//For undoing the entire tilemap
	uint32_t plane;
	uint32_t w,h,wnew,hnew;//The width and height
	void*ptr;//Points to tilemap data that is w*h*4 bytes or attributes if so size is w*h
	void*ptrnew;
};
struct undoTilemapEdit{
	uint32_t plane;
	uint32_t x,y,val,valnew;
};
struct undoTilemapPlane{
	tileMap*old;
	tileMap*Tnew;
	std::string*oldStr;
	std::string*TnewStr;
	uint32_t plane;
};
struct undoResize{
	uint32_t plane;
	uint32_t w,h,wnew,hnew;//Old width and height
	void*ptr;//Contains a pointer ONLY TO LOST DATA
};
struct undoPalette{
	void*ptr;
	void*ptrnew;
};
struct undoPaletteEntry{
	uint32_t id,val,valnew;
};
struct undoChunkEdit{
	uint32_t id,x,y;
	struct ChunkAttrs valnew,val;
};
struct undoChunk{
	uint32_t id;
	struct ChunkAttrs*ptr,*ptrnew;
};
struct undoChunkAll{
	uint32_t w,h,wnew,hnew;//The width and height
	uint32_t amt,amtnew;
	struct ChunkAttrs*ptr,*ptrnew;
};
struct undoSpriteVal{
	uint32_t id,subid;
	uint32_t val,valnew;
};
struct undoSpriteValbool{
	uint32_t id,subid;
	bool val,valnew;
};
struct undoSpriteDel{
	uint32_t id,subid;
	class sprite sp;
	int32_t offx,offy;
	uint32_t loadat;
};
struct undoSpriteGroupDel{
	struct spriteGroup;
};
struct undoProject{
	struct Project*old,*pnew;
};
struct undoProjectSwitch{
	uint32_t oldID,newID;
};
static struct undoEvent*undoBuf;
static uint_fast32_t amount;
static uint_fast32_t memUsed;
static uint_fast32_t maxMen=16*1024*1024;//Limit undo buffer to 16Mb this is better than limiting by depth as each item varies in size
static int_fast32_t pos=-1;
void showMemUsageUndo(Fl_Widget*,void*){
	fl_alert("May not be accurate\nThe undo stack currently uses %u bytes of ram not including any overhead\nAmount of items %d",(unsigned)memUsed,(int)amount);
}
static void resizeArray(uint32_t amt){
	if(undoBuf){
		if(amt)
			undoBuf=(struct undoEvent*)realloc(undoBuf,amt*sizeof(struct undoEvent));
		else{
			free(undoBuf);
			undoBuf=0;
		}
	}else{
		if(amt)
			undoBuf=(struct undoEvent*)malloc(amt*sizeof(struct undoEvent));
	}
}
static unsigned getSzTile(tileTypeMask_t type){
	unsigned sz=0;
	if(type&tTypeTile)
		sz+=currentProject->tileC->tileSize;
	if(type&tTypeTruecolor)
		sz+=currentProject->tileC->tcSize;
	return sz;
}
static uint32_t getSzResizeGeneric(uint32_t w,uint32_t h,uint32_t wnew,uint32_t hnew,uint32_t szelm,uint32_t n){//szelm is the size per element of what you are resizing
	if((w>wnew)||(h>hnew)){
		uint32_t tmp=0;
		if(w>wnew)
			tmp=(w-wnew)*hnew*szelm*n;
		if(h>hnew)
			tmp+=(h-hnew)*w*szelm*n;
		return tmp;
	}else
		return 0;
}
static void cleanupEvent(uint32_t id){
	struct undoEvent*uptr=undoBuf+id;
	switch(uptr->type){
		case uTile:
			{struct undoTile*ut=(struct undoTile*)uptr->ptr;
			unsigned sz=getSzTile(ut->type);
			free(ut->ptr);
			memUsed-=sz;
			if(ut->ptrnew){
				free(ut->ptrnew);
				memUsed-=sz;
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTile);}
		break;
		case uTilePixel:
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTilePixel);
		break;
		case uTileAll:
			{struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
			unsigned sz=getSzTile(ut->type)*currentProject->tileC->amt;
			free(ut->ptr);
			memUsed-=sz;
			if(ut->ptrnew){
				free(ut->ptrnew);
				memUsed-=sz;
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTileAll);}
		break;
		case uTileNew:
		case uTileAppend:
		case uChunkAppend:
		case uChunkNew:
		case uSpriteNewgroup:
		case uSpriteAppend:
		case uSpriteAppendgroup:
			//Nothing to do here
		break;
		case uTileGroup:
			{struct undoTileGroup*ut=(struct undoTileGroup*)uptr->ptr;
			unsigned sz=getSzTile(ut->type)*ut->lst.size();
			ut->data.clear();
			memUsed-=sz;
			if(ut->datanew.size()){
				ut->datanew.clear();
				memUsed-=sz;
			}
			delete uptr->ptr;
			memUsed-=sizeof(struct undoTileGroup);}
		break;
		case uTileAppendgroupdat:
			{struct undoAppendgroupdat*ut=(struct undoAppendgroupdat*)uptr->ptr;
			unsigned sz=getSzTile(tTypeBoth)*ut->amt;
			ut->dat.clear();
			ut->truedat.clear();
			delete uptr->ptr;
			memUsed-=sizeof(struct undoAppendgroupdat);}
		break;
		case uTilemapEdit:
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTilemapEdit);
		break;
		case uTilemap:
		case uTilemapattr:
			{struct undoTilemap*um=(struct undoTilemap*)uptr->ptr;
			uint_fast32_t sz=um->w*um->h;
			if(uptr->type==uTilemap)
				sz*=4;
			free(um->ptr);
			memUsed-=sz;
			if(um->ptrnew){
				sz=um->wnew*um->hnew;
				if(uptr->type==uTilemap)
					sz*=4;
				free(um->ptrnew);
				memUsed-=sz;
			}
			free(uptr->ptr);
			memUsed-=sizeof(undoTilemap);}
		break;
		case uTilemapResize:
		case uTilemapBlocksAmt:
			{struct undoResize*um=(struct undoResize*)uptr->ptr;
			if(um->ptr){
				free(um->ptr);
				memUsed-=getSzResizeGeneric(um->w,um->h,um->wnew,um->hnew,4,1);
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoResize);}
		break;
		case uTilemapPlaneDelete:
		case uTilemapPlaneAdd:
			{struct undoTilemapPlane*um=(struct undoTilemapPlane*)uptr->ptr;
			if(um->old){
				memUsed-=um->old->mapSizeW*um->old->mapSizeHA*TileMapSizePerEntry;
				delete um->old;
			}
			if(um->oldStr){
				memUsed-=um->oldStr->length();
				delete um->oldStr;
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoTilemapPlane);}
		break;
		case uPalette:
			{struct undoPalette*up=(struct undoPalette*)uptr->ptr;
			unsigned sz=currentProject->pal->colorCnt+currentProject->pal->colorCntalt;
			sz*=currentProject->pal->esize;
			free(up->ptr);
			memUsed-=sz;
			if(up->ptrnew){
				free(up->ptrnew);
				memUsed-=sz;
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoPalette);}
		break;
		case uPaletteEntry:
			free(uptr->ptr);
			memUsed-=sizeof(struct undoPaletteEntry);
		break;
		case uChunkResize:
			{struct undoResize*um=(struct undoResize*)uptr->ptr;
			if(um->ptr){
				free(um->ptr);
				memUsed-=getSzResizeGeneric(um->w,um->h,um->wnew,um->hnew,sizeof(struct ChunkAttrs),currentProject->Chunk->amt);
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoResize);}
		break;
		case uChunkEdit:
			free(uptr->ptr);
			memUsed-=sizeof(struct undoChunkEdit);
		break;
		case uChunkAll:
			{struct undoChunkAll*uc=(struct undoChunkAll*)uptr->ptr;
			free(uc->ptr);
			memUsed-=uc->w*uc->h*uc->amt*sizeof(struct ChunkAttrs);
			if(uc->ptrnew){
				free(uc->ptrnew);
				memUsed-=uc->wnew*uc->hnew*uc->amtnew*sizeof(struct ChunkAttrs);
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoChunkAll);}
		break;
		case uChunk:
		case uChunkDelete:
			{struct undoChunk*uc=(struct undoChunk*)uptr->ptr;
			free(uc->ptr);
			memUsed-=currentProject->Chunk->wi*currentProject->Chunk->hi*sizeof(struct ChunkAttrs);
			if(uc->ptrnew){
				free(uc->ptrnew);
				memUsed-=currentProject->Chunk->wi*currentProject->Chunk->hi*sizeof(struct ChunkAttrs);
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoChunk);}
		break;
		case uSpriteWidth:
		case uSpriteHeight:
		case uSpritePalrow:
		case uSpritestarttile:
		case uSpriteloadat:
		case uSpriteoffx:
		case uSpriteoffy:
			{struct undoSpriteVal*uc=(struct undoSpriteVal*)uptr->ptr;
			free(uptr->ptr);
			memUsed-=sizeof(struct undoSpriteVal);}
		break;
		case uSpriteprio:
		case uSpritehflip:
		case uSpritevflip:
			{struct undoSpriteValbool*uc=(struct undoSpriteValbool*)uptr->ptr;
			free(uptr->ptr);
			memUsed-=sizeof(struct undoSpriteValbool);}
		break;
		case uCurProject:
			{struct undoProject*up=(struct undoProject*)uptr->ptr;
			if(up->old)
				delete up->old;
			if(up->pnew)
				delete up->pnew;
			memUsed-=sizeof(struct undoProject);}
		break;
	}
}
void clearUndoCB(Fl_Widget*,void*){
	if(fl_ask("Warning this action cannot be undone.\nAre you sure that you want to do this?")){
		for(uint_fast32_t i=0;i<amount;++i){
			cleanupEvent(i);
			memUsed-=sizeof(struct undoEvent);
		}
		amount=0;
		pos=-1;
		compactPrjMem();
	}
}
static void pushEventPrepare(void){
	++pos;
	if((pos<=amount)&&amount){
		for(uint_fast32_t i=pos;i<amount;++i){
			cleanupEvent(i);
			memUsed-=sizeof(struct undoEvent);
		}
	}
	amount=pos;
	resizeArray(++amount);
	memUsed+=sizeof(struct undoEvent);
}
static void tilesTo(uint8_t*ptr,uint32_t id,tileTypeMask_t type){
	if(type&tTypeTile){
		memcpy(ptr,currentProject->tileC->tDat.data()+(id*currentProject->tileC->tileSize),currentProject->tileC->tileSize);
		ptr+=currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(ptr,currentProject->tileC->truetDat.data()+(id*currentProject->tileC->tcSize),currentProject->tileC->tcSize);
}
static void tilesToU(uint8_t*ptr,uint32_t id,tileTypeMask_t type){
	if(type&tTypeTile){
		memcpy(currentProject->tileC->tDat.data()+(id*currentProject->tileC->tileSize),ptr,currentProject->tileC->tileSize);
		ptr+=currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(currentProject->tileC->truetDat.data()+(id*currentProject->tileC->tcSize),ptr,currentProject->tileC->tcSize);
}
static void cpyAllTiles(uint8_t*ptr,unsigned amt,tileTypeMask_t type){
	if(type&tTypeTile){
		memcpy(ptr,currentProject->tileC->tDat.data(),amt*currentProject->tileC->tileSize);
		ptr+=amt*currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(ptr,currentProject->tileC->truetDat.data(),amt*currentProject->tileC->tcSize);
}
static void cpyAllTilesU(uint8_t*ptr,unsigned amt,tileTypeMask_t type){
	if(amt!=currentProject->tileC->amt){
		currentProject->tileC->resizeAmt(amt);
		updateTileSelectAmt();
	}
	if(type&tTypeTile){
		memcpy(currentProject->tileC->tDat.data(),ptr,amt*currentProject->tileC->tileSize);
		ptr+=amt*currentProject->tileC->tileSize;
	}
	if(type&tTypeTruecolor)
		memcpy(currentProject->tileC->truetDat.data(),ptr,amt*currentProject->tileC->tcSize);
}
static void attrCpy(uint8_t*dst,uint8_t*src,uint_fast32_t n){
	while(n--){
		*dst++=*src;
		src+=4;
	}
}
static void attrCpyU(uint8_t*dst,uint8_t*src,uint_fast32_t n){
	while(n--){
		*dst=*src++;
		dst+=4;
	}
}
static void cpyResizeGeneric(uint8_t*dst,uint8_t*src,uint32_t w,uint32_t h,uint32_t wnew,uint32_t hnew,uint32_t szelm,uint32_t n,bool reverse){
	if((w>wnew)||(h>hnew)){
		while(n--){
			if(w>wnew){
				for(uint32_t i=0;i<std::min(hnew,h);++i){
					src+=wnew*szelm;
					if(reverse)
						memcpy(src,dst,(w-wnew)*szelm);
					else
						memcpy(dst,src,(w-wnew)*szelm);
					dst+=(w-wnew)*szelm;
					src+=(w-wnew)*szelm;
				}
			}else
				src+=w*std::min(hnew,h)*szelm;
			if(h>hnew){
				for(uint32_t i=hnew;i<h;++i){
					if(reverse)
						memcpy(src,dst,w*szelm);
					else
						memcpy(dst,src,w*szelm);
					dst+=w*szelm;
					src+=w*szelm;
				}
			}
		}
	}
}
#define mkSpritePop(which) {struct undoSpriteVal*us=(struct undoSpriteVal*)uptr->ptr; \
	if(redo) \
		currentProject->spritesC->groups[us->id].list[us->subid].which=us->valnew; \
	else{ \
		us->valnew=currentProject->spritesC->groups[us->id].list[us->subid].which; \
		currentProject->spritesC->groups[us->id].list[us->subid].which=us->val; \
	} \
	window->updateSpriteSliders();}
#define mkSpritePop2(which) {struct undoSpriteVal*us=(struct undoSpriteVal*)uptr->ptr; \
	if(redo) \
		currentProject->spritesC->groups[us->id].which[us->subid]=us->valnew; \
	else{ \
		us->valnew=currentProject->spritesC->groups[us->id].which[us->subid]; \
		currentProject->spritesC->groups[us->id].which[us->subid]=us->val; \
	} \
	window->updateSpriteSliders();}
#define mkSpritePopbool(which) {struct undoSpriteValbool*us=(struct undoSpriteValbool*)uptr->ptr; \
	if(redo) \
		currentProject->spritesC->groups[us->id].list[us->subid].which=us->valnew; \
	else{ \
		us->valnew=currentProject->spritesC->groups[us->id].list[us->subid].which; \
		currentProject->spritesC->groups[us->id].list[us->subid].which=us->val; \
	} \
	window->updateSpriteSliders();}

static void isCorrectPlane(uint32_t plane){
	if(plane!=currentProject->curPlane){
		window->planeSelect->value(plane);
		setCurPlaneTilemaps(0,(void*)(uintptr_t)plane);
		window->redraw();
	}
}
static void removePlane(uint32_t plane){
	currentProject->tms->removePlane(plane);
	if(currentProject->curPlane==plane){
		if(currentProject->curPlane)
			--currentProject->curPlane;
		else
			++currentProject->curPlane;
		window->planeSelect->value(currentProject->curPlane);
		updatePlaneTilemapMenu();
		setCurPlaneTilemaps(0,(void*)(uintptr_t)currentProject->curPlane);
	}else
		updatePlaneTilemapMenu();
}
static void UndoRedo(bool redo){
	if((pos<0)&&(!redo))
		return;
	if(!amount)
		return;
	if(redo&&(pos>=(int_fast32_t(amount)-1)))
		return;
	if(redo&&(pos<=int_fast32_t(amount)))
		++pos;
	struct undoEvent*uptr=undoBuf+pos;
	switch(uptr->type){
		case uTile:
			{struct undoTile*ut=(struct undoTile*)uptr->ptr;
			if(redo){
				if(ut->type&tTypeDeleteFlag){
					currentProject->tileC->remove_tile_at(ut->id);
					updateTileSelectAmt();
				}else
					tilesToU((uint8_t*)ut->ptrnew,ut->id,ut->type);
			}else{
				if((!ut->ptrnew)&&(!(ut->type&tTypeDeleteFlag))){
					unsigned sz=getSzTile(ut->type);
					ut->ptrnew=malloc(sz);
					memUsed+=sz;
				}
				if(ut->type&tTypeDeleteFlag){
					currentProject->tileC->insertTile(ut->id);
					updateTileSelectAmt();
				}else
					tilesTo((uint8_t*)ut->ptrnew,ut->id,ut->type);
				tilesToU((uint8_t*)ut->ptr,ut->id,ut->type);
			}}
		break;
		case uTilePixel:
			{struct undoTilePixel*ut=(struct undoTilePixel*)uptr->ptr;
			if(ut->type==tTypeTruecolor){
				if(redo)
					currentProject->tileC->setPixelTc(ut->id,ut->x,ut->y,ut->valnew);
				else{
					ut->valnew=currentProject->tileC->getPixelTc(ut->id,ut->x,ut->y);
					currentProject->tileC->setPixelTc(ut->id,ut->x,ut->y,ut->val);
				}
			}else{
				if(redo)
					currentProject->tileC->setPixel(ut->id,ut->x,ut->y,ut->valnew);
				else{
					ut->valnew=currentProject->tileC->getPixel(ut->id,ut->x,ut->y);
					currentProject->tileC->setPixel(ut->id,ut->x,ut->y,ut->val);
				}
			}}
		break;
		case uTileAll:
			{struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
			if(redo)
				cpyAllTilesU((uint8_t*)ut->ptrnew,ut->amtnew,ut->type);
			else{
				if(!ut->ptrnew){
					ut->amtnew=currentProject->tileC->amt;
					unsigned sz=getSzTile(ut->type)*ut->amtnew;
					ut->ptrnew=malloc(sz);
					memUsed+=sz;
				}
				cpyAllTiles((uint8_t*)ut->ptrnew,ut->amtnew,ut->type);
				cpyAllTilesU((uint8_t*)ut->ptr,ut->amt,ut->type);
			}}
		break;
		case uTileGroup:
			{struct undoTileGroup*ut=(struct undoTileGroup*)uptr->ptr;
			unsigned sz=getSzTile(ut->type);
			if(redo){
				if(ut->type&tTypeDeleteFlag){
					std::vector<uint32_t> tmp=ut->lst;
					std::sort(tmp.begin(),tmp.end());
					for(int_fast32_t i=tmp.size();i--;)
						currentProject->tileC->remove_tile_at(tmp[i]);
					updateTileSelectAmt();
				}else{
					for(int_fast32_t i=ut->lst.size()-1;i>=0;--i)
						tilesToU(ut->datanew.data()+(i*sz),ut->lst[i],ut->type);
				}
			}else{
				if((!(ut->datanew.size()))&&(!(ut->type&tTypeDeleteFlag))){
					ut->datanew.resize(sz*ut->lst.size());
					memUsed+=sz*ut->lst.size();
					for(int_fast32_t i=ut->lst.size()-1;i>=0;--i)
						tilesTo(ut->datanew.data()+(i*sz),ut->lst[i],ut->type);
				}
				if(ut->type&tTypeDeleteFlag){
					uint32_t fullSize=currentProject->tileC->amt+ut->lst.size();
					std::vector<uint32_t> tmp=ut->lst;
					std::sort(tmp.begin(),tmp.end());
					for(int_fast32_t i=0;i<tmp.size();++i){
						if(tmp[i]<currentProject->tileC->amt){
							currentProject->tileC->insertTile(tmp[i]);
						}
					}
					currentProject->tileC->resizeAmt(fullSize);
					updateTileSelectAmt();
				}
				for(int_fast32_t i=ut->lst.size()-1;i>=0;--i){
					tilesToU(ut->data.data()+(i*sz),ut->lst[i],ut->type);
				}
			}}
		break;
		case uTileAppendgroupdat:
			{struct undoAppendgroupdat*ut=(struct undoAppendgroupdat*)uptr->ptr;
			if(redo){
				unsigned amtold=currentProject->tileC->amt;
				currentProject->tileC->resizeAmt(amtold+ut->amt);
				memcpy(currentProject->tileC->tDat.data()+((amtold*currentProject->tileC->tileSize)),ut->dat.data(),currentProject->tileC->tileSize*ut->amt);
				memcpy(currentProject->tileC->truetDat.data()+((amtold*currentProject->tileC->tcSize)),ut->truedat.data(),currentProject->tileC->tcSize*ut->amt);
			}else
				currentProject->tileC->resizeAmt(currentProject->tileC->amt-ut->amt);
			updateTileSelectAmt();
			}
		break;
		case uTileAppend:
			if(redo)
				currentProject->tileC->appendTile();
			else
				currentProject->tileC->resizeAmt(currentProject->tileC->amt-1);
			updateTileSelectAmt();
		break;
		case uTileNew:
			if(redo)
				currentProject->tileC->insertTile((uintptr_t)uptr->ptr);
			else
				currentProject->tileC->remove_tile_at((uintptr_t)uptr->ptr);
			updateTileSelectAmt();
		break;
		case uTilemapEdit:
			{struct undoTilemapEdit*um=(struct undoTilemapEdit*)uptr->ptr;
			isCorrectPlane(um->plane);
			if(redo)
				currentProject->tms->maps[um->plane].setRaw(um->x,um->y,um->valnew);
			else{
				um->valnew=currentProject->tms->maps[um->plane].getRaw(um->x,um->y);
				currentProject->tms->maps[um->plane].setRaw(um->x,um->y,um->val);
			}
			if(tileEditModePlace_G)
				window->updateTileMapGUI(um->x,um->y);}
		break;
		case uTilemap:
		case uTilemapattr:
			{struct undoTilemap*um=(struct undoTilemap*)uptr->ptr;
			isCorrectPlane(um->plane);
			if(redo){
				if(uptr->type==uTilemapattr)
					attrCpyU(currentProject->tms->maps[um->plane].tileMapDat,(uint8_t*)um->ptrnew,um->wnew*um->hnew);
				else{
					currentProject->tms->maps[um->plane].resize_tile_map(um->wnew,um->hnew);
					memcpy(currentProject->tms->maps[um->plane].tileMapDat,um->ptrnew,um->wnew*um->hnew*4);
				}
			}else{
				if(!um->ptrnew){
					um->wnew=currentProject->tms->maps[um->plane].mapSizeW;
					um->hnew=currentProject->tms->maps[um->plane].mapSizeHA;
					if(uptr->type==uTilemapattr){
						um->ptrnew=malloc(um->wnew*um->hnew);
						attrCpy((uint8_t*)um->ptrnew,currentProject->tms->maps[um->plane].tileMapDat,um->wnew*um->hnew);
					}else{
						um->ptrnew=malloc(um->wnew*um->hnew*4);
						memcpy(um->ptrnew,currentProject->tms->maps[um->plane].tileMapDat,um->wnew*um->hnew*4);
					}
				}
				if(uptr->type==uTilemapattr){
					attrCpyU(currentProject->tms->maps[um->plane].tileMapDat,(uint8_t*)um->ptr,um->w*um->h);
				}else{
					currentProject->tms->maps[um->plane].resize_tile_map(um->w,um->h);
					memcpy(currentProject->tms->maps[um->plane].tileMapDat,um->ptr,um->w*um->h*4);
				}
			}}
		break;
		case uTilemapResize:
			{struct undoResize*um=(struct undoResize*)uptr->ptr;
			isCorrectPlane(um->plane);
			if(redo)
				currentProject->tms->maps[um->plane].resize_tile_map(um->wnew,um->hnew);
			else{
				currentProject->tms->maps[um->plane].resize_tile_map(um->w,um->h);
				if(um->ptr)
					cpyResizeGeneric((uint8_t*)um->ptr,currentProject->tms->maps[um->plane].tileMapDat,um->w,um->h,um->wnew,um->hnew,4,1,true);
			}}
		break;
		case uTilemapBlocksAmt:
			{struct undoResize*um=(struct undoResize*)uptr->ptr;
			isCorrectPlane(um->plane);
			if(redo){
				currentProject->tms->maps[um->plane].blockAmt(um->hnew/currentProject->tms->maps[um->plane].mapSizeH);
				char tmp[16];
				snprintf(tmp,16,"%u",um->hnew/currentProject->tms->maps[um->plane].mapSizeH);
			}else{
				currentProject->tms->maps[um->plane].blockAmt(um->h/currentProject->tms->maps[um->plane].mapSizeH);
				if(um->ptr)
					cpyResizeGeneric((uint8_t*)um->ptr,currentProject->tms->maps[um->plane].tileMapDat,um->w,um->h,um->wnew,um->hnew,4,1,true);
				char tmp[16];
				snprintf(tmp,16,"%u",um->h/currentProject->tms->maps[um->plane].mapSizeH);
			}}
		break;
		case uTilemapPlaneDelete:
			{struct undoTilemapPlane*um=(struct undoTilemapPlane*)uptr->ptr;
			if(redo){
				removePlane(um->plane);
			}else{
				currentProject->tms->maps.insert(currentProject->tms->maps.begin()+um->plane,tileMap(*um->old));
				currentProject->tms->planeName.insert(currentProject->tms->planeName.begin()+um->plane,*um->oldStr);
				updatePlaneTilemapMenu();
				if(um->plane==currentProject->curPlane)
					setCurPlaneTilemaps(0,(void*)(uintptr_t)um->plane);
			}}
		break;
		case uTilemapPlaneAdd:
			{struct undoTilemapPlane*um=(struct undoTilemapPlane*)uptr->ptr;
			if(redo){
				currentProject->tms->maps.insert(currentProject->tms->maps.begin()+um->plane,tileMap());
				char tmp[16];
				snprintf(tmp,16,"%u",um->plane);
				currentProject->tms->planeName.insert(currentProject->tms->planeName.begin()+um->plane,1,tmp);
				updatePlaneTilemapMenu();
				if(um->plane==currentProject->curPlane)
					setCurPlaneTilemaps(0,(void*)(uintptr_t)um->plane);
			}else{
				removePlane(um->plane);
			}}
		break;
		case uPalette:
			{struct undoPalette*up=(struct undoPalette*)uptr->ptr;
			unsigned sz,el=currentProject->pal->colorCnt+currentProject->pal->colorCntalt;
			sz=el*currentProject->pal->esize;
			if(redo)
				memcpy(currentProject->pal->palDat,up->ptrnew,sz);
			else{
				if(!up->ptrnew){
					up->ptrnew=malloc(sz);
					memUsed+=sz;
				}
				memcpy(up->ptrnew,currentProject->pal->palDat,sz);
				memcpy(currentProject->pal->palDat,up->ptr,sz);
			}
			for(unsigned i=0;i<el;++i)
				currentProject->pal->updateRGBindex(i);
			}
			palBar.updateSliders();
		break;
		case uPaletteEntry:
			{struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
			switch(currentProject->pal->esize){
				case 1:
					if(redo)
						currentProject->pal->palDat[up->id]=up->valnew;
					else{
						up->valnew=currentProject->pal->palDat[up->id];
						currentProject->pal->palDat[up->id]=up->val;
					}
				break;
				case 2:
					{uint16_t*ptr=(uint16_t*)currentProject->pal->palDat+up->id;
					if(redo)
						*ptr=up->valnew;
					else{
						up->valnew=*ptr;
						*ptr=up->val;
					}}
				break;
			}
			currentProject->pal->updateRGBindex(up->id);
			switch (mode_editor){
				case pal_edit:
					palBar.selBox[0]=up->id%currentProject->pal->perRow;
					palBar.changeRow(up->id/currentProject->pal->perRow,0);
				break;
				case tile_edit:
					palBar.selBox[1]=up->id%currentProject->pal->perRow;
					palBar.changeRow(up->id/currentProject->pal->perRow,1);
					{unsigned focus=0;
					for(unsigned i=0;i<currentProject->pal->rowCntPal;++i)
						focus|=Fl::focus()==window->palRTE[i];
					for(unsigned i=0;i<currentProject->pal->rowCntPal;++i){
						if(focus&&(i==palBar.selRow[1]))
							Fl::focus(window->palRTE[i]);
						window->palRTE[i]->value(i==palBar.selRow[1]);
					}}
				break;
				case tile_place:
					palBar.selBox[2]=up->id%currentProject->pal->perRow;
					palBar.changeRow(up->id/currentProject->pal->perRow,2);
					{unsigned focus=0;
					for(unsigned i=0;i<currentProject->pal->rowCntPal;++i)
						focus|=Fl::focus()==window->palRTE[i+4];
					for(unsigned i=0;i<currentProject->pal->rowCntPal;++i){
						if(focus&&(i==palBar.selRow[2]))
							Fl::focus(window->palRTE[i+4]);
						window->palRTE[i+4]->value(i==palBar.selRow[2]);
					}}
				break;
				case spriteEditor:
					palBar.selBox[3]=up->id%currentProject->pal->perRow;
					palBar.changeRow(up->id/currentProject->pal->perRow,3);
					window->spritepalrow->value(palBar.selRow[3]);
				break;
			}}
		break;
		case uChunkResize:
			{struct undoResize*um=(struct undoResize*)uptr->ptr;
			if(redo)
				currentProject->Chunk->resize(um->wnew,um->hnew);
			else{
				currentProject->Chunk->resize(um->w,um->h);
				if(um->ptr)
					cpyResizeGeneric((uint8_t*)um->ptr,(uint8_t*)currentProject->Chunk->chunks.data(),um->w,um->h,um->wnew,um->hnew,sizeof(struct ChunkAttrs),currentProject->Chunk->amt,true);
			}
			window->updateChunkSize();}
		break;
		case uChunkEdit:
			{struct undoChunkEdit*uc=(struct undoChunkEdit*)uptr->ptr;
			if(redo){
				currentProject->Chunk->setElm(uc->id,uc->x,uc->y,uc->valnew);
			}else{
				uc->valnew=currentProject->Chunk->getElm(uc->id,uc->x,uc->y);
				currentProject->Chunk->setElm(uc->id,uc->x,uc->y,uc->val);
			}
			if(tileEditModeChunk_G)
				window->updateChunkGUI(uc->x,uc->y);
			}
		break;
		case uChunkAppend:
			if(redo)
				currentProject->Chunk->resizeAmt(currentProject->Chunk->amt+1);
			else
				currentProject->Chunk->resizeAmt(currentProject->Chunk->amt-1);
			window->updateChunkSel();
		break;
		case uChunkNew:
			if(redo)
				currentProject->Chunk->insert((uintptr_t)uptr->ptr);
			else
				currentProject->Chunk->removeAt((uintptr_t)uptr->ptr);
			window->updateChunkSel();
		break;
		case uChunkAll:
			{struct undoChunkAll*uc=(struct undoChunkAll*)uptr->ptr;
			if(redo){
				currentProject->Chunk->resize(uc->wnew,uc->hnew);
				currentProject->Chunk->resizeAmt(uc->amtnew);
				memcpy(currentProject->Chunk->chunks.data(),uc->ptrnew,uc->wnew*uc->hnew*uc->amtnew*sizeof(struct ChunkAttrs));
			}else{
				if(!uc->ptrnew){
					uc->wnew=currentProject->Chunk->wi;
					uc->hnew=currentProject->Chunk->hi;
					uc->amtnew=currentProject->Chunk->amt;
					uc->ptrnew=(struct ChunkAttrs*)malloc(uc->wnew*uc->hnew*uc->amtnew*sizeof(struct ChunkAttrs));
					memcpy(uc->ptrnew,currentProject->Chunk->chunks.data(),uc->wnew*uc->hnew*uc->amtnew*sizeof(struct ChunkAttrs));
				}
				currentProject->Chunk->resize(uc->w,uc->h);
				currentProject->Chunk->resizeAmt(uc->amt);
				memcpy(currentProject->Chunk->chunks.data(),uc->ptr,uc->w*uc->h*uc->amt*sizeof(struct ChunkAttrs));
			}
			window->updateChunkSize();}
		break;
		case uChunk:
			fl_alert("TODO");
		break;
		case uChunkDelete:
			{struct undoChunk*uc=(struct undoChunk*)uptr->ptr;
			if(redo)
				currentProject->Chunk->removeAt(uc->id);
			else{
				currentProject->Chunk->insert(uc->id);
				memcpy(currentProject->Chunk->chunks.data()+(currentProject->Chunk->wi*currentProject->Chunk->hi*uc->id),uc->ptr,currentProject->Chunk->wi*currentProject->Chunk->hi*sizeof(struct ChunkAttrs));
			}
			window->updateChunkSel();
			}
		break;
		case uSpriteAppend:
			{uint32_t id=(uintptr_t)uptr->ptr;
			if(redo)
				currentProject->spritesC->setAmtingroup(id,currentProject->spritesC->groups[id].list.size()+1);
			else
				currentProject->spritesC->delingroup(id,currentProject->spritesC->groups[id].list.size()-1);
			window->updateSpriteSliders();}
		break;
		case uSpriteAppendgroup:
			if(redo)
				currentProject->spritesC->setAmt(currentProject->spritesC->amt+1);
			else
				currentProject->spritesC->del(currentProject->spritesC->amt-1);
			window->updateSpriteSliders();
		break;
		case uSpriteWidth:
			mkSpritePop(w)
		break;
		case uSpriteHeight:
			mkSpritePop(h)
		break;
		case uSpritePalrow:
			mkSpritePop(palrow)
		break;
		case uSpritestarttile:
			mkSpritePop(starttile)
		break;
		case uSpriteloadat:
			mkSpritePop2(loadat)
		break;
		case uSpriteoffx:
			mkSpritePop2(offx)
		break;
		case uSpriteoffy:
			mkSpritePop2(offy)
		break;
		case uSpriteprio:
			mkSpritePopbool(prio)
		break;
		case uSpritehflip:
			mkSpritePopbool(hflip)
		break;
		case uSpritevflip:
			mkSpritePopbool(vflip)
		break;
		case uCurProject:
			{struct undoProject*up=(struct undoProject*)uptr->ptr;
			if(redo){
				up->old=new Project(*currentProject);
				delete currentProject;
				currentProject=new Project(*up->pnew);
				delete up->pnew;
				up->pnew=0;
			}else{
				up->pnew=new Project(*currentProject);
				delete currentProject;
				currentProject=new Project(*up->old);
				delete up->old;
				up->old=0;
			}
			switchProject(curProjectID);}
		break;
	}
	if(!redo)
		--pos;
	window->redraw();
}
void undoCB(Fl_Widget*,void*){
	UndoRedo(false);
}
void redoCB(Fl_Widget*,void*){
	UndoRedo(true);
}
void pushTile(uint32_t id,tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTile;
	uptr->ptr=malloc(sizeof(struct undoTile));
	memUsed+=sizeof(struct undoTile);
	unsigned sz=getSzTile(type);
	struct undoTile*ut=(struct undoTile*)uptr->ptr;
	ut->ptr=malloc(sz);
	ut->ptrnew=0;
	ut->id=id;
	ut->type=type;
	memUsed+=sz;
	tilesTo((uint8_t*)ut->ptr,id,type);
}
void pushTilenew(uint32_t id){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTileNew;
	uptr->ptr=(void*)(uintptr_t)id;
}
void pushTilePixel(uint32_t id,uint32_t x,uint32_t y,tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTilePixel;
	uptr->ptr=malloc(sizeof(struct undoTilePixel));
	memUsed+=sizeof(struct undoTilePixel);
	struct undoTilePixel*ut=(struct undoTilePixel*)uptr->ptr;
	ut->id=id;
	ut->x=x;
	ut->y=y;
	ut->type=type;
	if(type==tTypeTruecolor)
		ut->val=currentProject->tileC->getPixelTc(id,x,y);
	else
		ut->val=currentProject->tileC->getPixel(id,x,y);
}
void pushTilesAll(tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTileAll;
	uptr->ptr=malloc(sizeof(struct undoTileAll));
	memUsed+=sizeof(struct undoTileAll);
	struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
	ut->amt=currentProject->tileC->amt;
	unsigned sz=getSzTile(type)*ut->amt;
	ut->ptr=malloc(sz);
	memUsed+=sz;
	ut->type=type;
	ut->ptrnew=0;
	cpyAllTiles((uint8_t*)ut->ptr,ut->amt,type);
}
void pushTileAppend(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTileAppend;
}
void pushTileGroupPrepare(tileTypeMask_t type){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTileGroup;
	uptr->ptr=new struct undoTileGroup;
	memUsed+=sizeof(struct undoTileGroup);
	struct undoTileGroup*ut=(struct undoTileGroup*)uptr->ptr;
	ut->type=type;
}
void addTileGroup(uint32_t tile,int32_t forceid){
	struct undoEvent*uptr=undoBuf+pos;
	struct undoTileGroup*ut=(struct undoTileGroup*)uptr->ptr;
	if(forceid>0)
		ut->lst.push_back(forceid);
	else
		ut->lst.push_back(tile);
	unsigned sz=getSzTile(ut->type);
	ut->data.resize(sz*ut->lst.size());
	tilesTo(ut->data.data()+(sz*(ut->lst.size()-1)),tile,ut->type);
}
void pushTileappendGroupPrepare(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTileAppendgroupdat;
	uptr->ptr=new struct undoAppendgroupdat;
	memUsed+=sizeof(struct undoAppendgroupdat);
	struct undoAppendgroupdat*ut=(struct undoAppendgroupdat*)uptr->ptr;
	ut->amt=0;
}
void addTileappendGroup(uint8_t*tdat,uint8_t*truetdat){
	struct undoEvent*uptr=undoBuf+pos;
	struct undoAppendgroupdat*ut=(struct undoAppendgroupdat*)uptr->ptr;
	++ut->amt;
	ut->dat.resize(ut->amt*currentProject->tileC->tileSize);
	ut->truedat.resize(ut->amt*currentProject->tileC->tcSize);
	memcpy(ut->dat.data()+((ut->amt-1)*currentProject->tileC->tileSize),tdat,currentProject->tileC->tileSize);
	memcpy(ut->truedat.data()+((ut->amt-1)*currentProject->tileC->tcSize),truetdat,currentProject->tileC->tcSize);
}
void pushTilemapAll(bool attrOnly){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	if(attrOnly)
		uptr->type=uTilemapattr;
	else
		uptr->type=uTilemap;
	uptr->ptr=malloc(sizeof(struct undoTilemap));
	memUsed+=sizeof(struct undoTilemap);
	struct undoTilemap*um=(struct undoTilemap*)uptr->ptr;
	um->plane=currentProject->curPlane;
	um->w=currentProject->tms->maps[currentProject->curPlane].mapSizeW;
	um->h=currentProject->tms->maps[currentProject->curPlane].mapSizeHA;
	um->ptrnew=0;
	if(attrOnly){
		um->ptr=malloc(um->w*um->h);
		attrCpy((uint8_t*)um->ptr,currentProject->tms->maps[currentProject->curPlane].tileMapDat,um->w*um->h);
	}else{
		um->ptr=malloc(um->w*um->h*4);
		memcpy(um->ptr,currentProject->tms->maps[currentProject->curPlane].tileMapDat,um->w*um->h*4);
	}
}
struct undoTilemapPlane*pushTilemapPlaneComm(uint32_t plane){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->ptr=malloc(sizeof(struct undoTilemapPlane));
	memUsed+=sizeof(struct undoTilemapPlane);
	struct undoTilemapPlane*um=(struct undoTilemapPlane*)uptr->ptr;
	um->plane=plane;
	um->old=0;
	um->oldStr=0;
	return um;
}
void pushTilemapPlaneDelete(uint32_t plane){
	struct undoTilemapPlane*um=pushTilemapPlaneComm(plane);
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTilemapPlaneDelete;
	um->old=new tileMap(currentProject->tms->maps[plane]);
	um->oldStr=new std::string(currentProject->tms->planeName[um->plane].c_str());
	memUsed+=um->oldStr->length();
	memUsed+=um->old->mapSizeW*um->old->mapSizeHA*TileMapSizePerEntry;
}
void pushTilemapPlaneAdd(uint32_t plane){
	pushTilemapPlaneComm(plane);
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTilemapPlaneAdd;
}
void pushTilemapEdit(uint32_t x,uint32_t y){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uTilemapEdit;
	uptr->ptr=malloc(sizeof(struct undoTilemapEdit));
	memUsed+=sizeof(struct undoTilemapEdit);
	struct undoTilemapEdit*um=(struct undoTilemapEdit*)uptr->ptr;
	um->x=x;
	um->y=y;
	um->plane=currentProject->curPlane;
	um->val=currentProject->tms->maps[currentProject->curPlane].getRaw(x,y);
}
static void pushResize(uint32_t wnew,uint32_t hnew,uint32_t w,uint32_t h,uint8_t*ptr,undoTypes_t type,uint32_t szelm,uint32_t n){
	if((wnew==w)&&(hnew==h))
		return;
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=type;
	uptr->ptr=malloc(sizeof(struct undoResize));
	memUsed+=sizeof(struct undoResize);
	struct undoResize*um=(struct undoResize*)uptr->ptr;
	um->plane=currentProject->curPlane;
	um->w=w;
	um->h=h;
	um->wnew=wnew;
	um->hnew=hnew;
	uint32_t sz=getSzResizeGeneric(um->w,um->h,wnew,hnew,szelm,n);
	if(sz){
		um->ptr=malloc(sz);
		memUsed+=sz;
		cpyResizeGeneric((uint8_t*)um->ptr,ptr,um->w,um->h,wnew,hnew,szelm,n,false);
	}else
		um->ptr=0;
}
void pushTilemapResize(uint32_t wnew,uint32_t hnew){
	pushResize(wnew,hnew,currentProject->tms->maps[currentProject->curPlane].mapSizeW,currentProject->tms->maps[currentProject->curPlane].mapSizeHA,currentProject->tms->maps[currentProject->curPlane].tileMapDat,uTilemapResize,TileMapSizePerEntry,1);
}
void pushTilemapBlocksAmt(uint32_t amtnew){
	pushResize(currentProject->tms->maps[currentProject->curPlane].mapSizeW,currentProject->tms->maps[currentProject->curPlane].mapSizeH*amtnew,currentProject->tms->maps[currentProject->curPlane].mapSizeW,currentProject->tms->maps[currentProject->curPlane].mapSizeHA,currentProject->tms->maps[currentProject->curPlane].tileMapDat,uTilemapBlocksAmt,TileMapSizePerEntry,1);
}
void pushPaletteAll(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uPalette;
	uptr->ptr=malloc(sizeof(struct undoPalette));
	memUsed+=sizeof(struct undoPalette);
	struct undoPalette*up=(struct undoPalette*)uptr->ptr;
	unsigned sz=currentProject->pal->colorCnt+currentProject->pal->colorCntalt;
	sz*=currentProject->pal->esize;
	up->ptr=malloc(sz);
	memcpy(up->ptr,currentProject->pal->palDat,sz);
	memUsed+=sz;
	up->ptrnew=0;
}
void pushPaletteEntry(uint32_t id){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uPaletteEntry;
	uptr->ptr=malloc(sizeof(struct undoPaletteEntry));
	memUsed+=sizeof(struct undoPaletteEntry);
	struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
	up->id=id;
	switch(currentProject->pal->esize){
		case 1:
			up->val=(int32_t)currentProject->pal->palDat[id];
		break;
		case 2:
			{uint16_t*ptr=(uint16_t*)currentProject->pal->palDat+id;
			up->val=*ptr;}
		break;
		default:
			show_default_error
	}
}
void pushChunkResize(uint32_t wnew,uint32_t hnew){
	pushResize(wnew,hnew,currentProject->Chunk->wi,currentProject->Chunk->hi,(uint8_t*)currentProject->Chunk->chunks.data(),uChunkResize,sizeof(struct ChunkAttrs),currentProject->Chunk->amt);
}
void pushChunkEdit(uint32_t id,uint32_t x,uint32_t y){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uChunkEdit;
	uptr->ptr=malloc(sizeof(struct undoChunkEdit));
	memUsed+=sizeof(struct undoChunkEdit);
	struct undoChunkEdit*uc=(struct undoChunkEdit*)uptr->ptr;
	uc->x=x;
	uc->y=y;
	uc->id=id;
	uc->val=currentProject->Chunk->getElm(id,x,y);
}
void pushChunkNew(uint32_t id){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uChunkNew;
	uptr->ptr=(void*)(uintptr_t)id;
}
void pushChunkAppend(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uChunkAppend;
}
void pushChunk(uint32_t id,bool rm){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	if(rm)
		uptr->type=uChunkDelete;
	else
		uptr->type=uChunk;
	uptr->ptr=malloc(sizeof(struct undoChunk));
	memUsed+=sizeof(struct undoChunk);
	struct undoChunk*uc=(struct undoChunk*)uptr->ptr;
	uc->id=id;
	uc->ptrnew=0;
	uc->ptr=(struct ChunkAttrs*)malloc(currentProject->Chunk->wi*currentProject->Chunk->hi*sizeof(struct ChunkAttrs));
	memcpy(uc->ptr,currentProject->Chunk->chunks.data()+(currentProject->Chunk->wi*currentProject->Chunk->hi*id),currentProject->Chunk->wi*currentProject->Chunk->hi*sizeof(struct ChunkAttrs));
}
void pushChunksAll(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uChunkAll;
	uptr->ptr=malloc(sizeof(struct undoChunkAll));
	memUsed+=sizeof(struct undoChunkAll);
	struct undoChunkAll*uc=(struct undoChunkAll*)uptr->ptr;
	uc->ptrnew=0;
	uc->w=currentProject->Chunk->wi;
	uc->h=currentProject->Chunk->hi;
	uc->amt=currentProject->Chunk->amt;
	uc->ptr=(struct ChunkAttrs*)malloc(uc->w*uc->h*uc->amt*sizeof(struct ChunkAttrs));
	memcpy(uc->ptr,currentProject->Chunk->chunks.data(),uc->w*uc->h*uc->amt*sizeof(struct ChunkAttrs));
}
void pushSpriteAppend(uint32_t id){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uSpriteAppend;
	uptr->ptr=(void*)(uintptr_t)id;
}
void pushSpriteAppendgroup(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uSpriteAppendgroup;
}
#define mkSpritePush(thetype,which) pushEventPrepare(); \
	struct undoEvent*uptr=undoBuf+pos; \
	uptr->type=thetype; \
	uptr->ptr=malloc(sizeof(struct undoSpriteVal)); \
	memUsed+=sizeof(struct undoSpriteVal); \
	struct undoSpriteVal*us=(struct undoSpriteVal*)uptr->ptr; \
	us->id=curSpritegroup; \
	us->subid=curSprite; \
	us->val=currentProject->spritesC->groups[us->id].list[us->subid].which
void pushSpriteWidth(void){
	mkSpritePush(uSpriteWidth,w);
}
void pushSpriteHeight(void){
	mkSpritePush(uSpriteHeight,h);
}
void pushSpritePalrow(void){
	mkSpritePush(uSpritePalrow,palrow);
}
void pushSpriteStarttile(void){
	mkSpritePush(uSpritestarttile,starttile);
}
#define mkSpritePush2(thetype,which) pushEventPrepare(); \
	struct undoEvent*uptr=undoBuf+pos; \
	uptr->type=thetype; \
	uptr->ptr=malloc(sizeof(struct undoSpriteVal)); \
	memUsed+=sizeof(struct undoSpriteVal); \
	struct undoSpriteVal*us=(struct undoSpriteVal*)uptr->ptr; \
	us->id=curSpritegroup; \
	us->subid=curSprite; \
	us->val=currentProject->spritesC->groups[us->id].which[us->subid]
void pushSpriteLoadat(void){
	mkSpritePush2(uSpriteloadat,loadat);
}
void pushSpriteOffx(void){
	mkSpritePush2(uSpriteoffx,offx);
}
void pushSpriteOffy(void){
	mkSpritePush2(uSpriteoffy,offy);
}
#define mkSpritePushbool(thetype,which) pushEventPrepare(); \
	struct undoEvent*uptr=undoBuf+pos; \
	uptr->type=thetype; \
	uptr->ptr=malloc(sizeof(struct undoSpriteValbool)); \
	memUsed+=sizeof(struct undoSpriteValbool); \
	struct undoSpriteValbool*us=(struct undoSpriteValbool*)uptr->ptr; \
	us->id=curSpritegroup; \
	us->subid=curSprite; \
	us->val=currentProject->spritesC->groups[us->id].list[us->subid].which
void pushSpriteHflip(void){
	mkSpritePushbool(uSpritehflip,hflip);
}
void pushSpriteVflip(void){
	mkSpritePushbool(uSpritevflip,vflip);
}
void pushSpritePrio(void){
	mkSpritePushbool(uSpriteprio,prio);
}
void pushProject(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uCurProject;
	uptr->ptr=malloc(sizeof(struct undoProject));
	memUsed+=sizeof(struct undoProject);
	struct undoProject*up=(struct undoProject*)uptr->ptr;
	up->old=new Project(*currentProject);
	up->pnew=0;
}
static Fl_Window * win;
static void closeHistory(Fl_Widget*,void*){
	win->hide();
}
void historyWindow(Fl_Widget*,void*){
	win=new Fl_Window(350,450,"History");
	win->begin();
	Fl_Button * Close=new Fl_Button(143,418,64,24,"Close");
	Close->callback(closeHistory);
	Fl_Browser*hist=new Fl_Browser(8,32,336,386);
	char tmp[2048];
	snprintf(tmp,2048,"%u items sorted from oldest to newest\nPosition selected: %d (can be -1)",(unsigned)amount,(int)pos);
	hist->copy_label(tmp);
	hist->align(FL_ALIGN_TOP);
	for(unsigned n=0;n<amount;++n){
		struct undoEvent*uptr=undoBuf+n;
		switch(uptr->type){
			case uTile:
				{struct undoTile*ut=(struct undoTile*)uptr->ptr;
				if(ut->type&tTypeDeleteFlag)
					snprintf(tmp,2048,"Delete tile %d",ut->id);
				else
					snprintf(tmp,2048,"Change tile %d",ut->id);
				}
			break;
			case uTilePixel:
				{struct undoTilePixel*ut=(struct undoTilePixel*)uptr->ptr;
				if(ut->type&tTypeTruecolor)
					snprintf(tmp,2048,"Edit truecolor tile pixel X: %d Y: %d",ut->x,ut->y);
				else
					snprintf(tmp,2048,"Edit tile pixel X: %d Y: %d",ut->x,ut->y);
				}
			break;
			case uTileAll:
				{struct undoTileAll*ut=(struct undoTileAll*)uptr->ptr;
				snprintf(tmp,2048,"Change all tiles amount: %u",ut->amt);}
			break;
			case uTileAppend:
				strcpy(tmp,"Append tile");
			break;
			case uTileNew:
				snprintf(tmp,2048,"Insert tile at %u",unsigned(uintptr_t(uptr->ptr)));
			case uTileGroup:
				{struct undoTileGroup*ut=(struct undoTileGroup*)uptr->ptr;
				snprintf(tmp,2048,"Tile group tiles affected: %u",(unsigned)ut->lst.size());}
			break;
			case uTileAppendgroupdat:
				{struct undoAppendgroupdat*ut=(struct undoAppendgroupdat*)uptr->ptr;
				snprintf(tmp,2048,"Append %u tiles with data",ut->amt);}
			break;
			case uTilemap:
				strcpy(tmp,"Change tilemap");
			break;
			case uTilemapattr:
				strcpy(tmp,"Change tilemap attributes");
			break;
			case uChunkResize:
			case uTilemapResize:
				{struct undoResize*um=(struct undoResize*)uptr->ptr;
				snprintf(tmp,2048,"Resize from w: %d h: %d to w: %d h: %d",um->w,um->h,um->wnew,um->hnew);}
			break;
			case uTilemapBlocksAmt:
				{struct undoResize*um=(struct undoResize*)uptr->ptr;
				snprintf(tmp,2048,"Change blocks amount from %u on plane %u",um->h/currentProject->tms->maps[um->plane].mapSizeH,um->plane);}
			break;
			case uTilemapEdit:
				{struct undoTilemapEdit*um=(struct undoTilemapEdit*)uptr->ptr;
				snprintf(tmp,2048,"Edit tilemap X: %d Y: %d",um->x,um->y);}
			break;
			case uTilemapPlaneDelete:
				{struct undoTilemapPlane*um=(struct undoTilemapPlane*)uptr->ptr;
				snprintf(tmp,2048,"Delete tilemap %u",um->plane);}
			break;
			case uTilemapPlaneAdd:
				{struct undoTilemapPlane*um=(struct undoTilemapPlane*)uptr->ptr;
				snprintf(tmp,2048,"Add tilemap %u",um->plane);}
			break;
			case uPalette:
				strcpy(tmp,"Change entire palette");
			break;
			case uPaletteEntry:
				{struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
				snprintf(tmp,2048,"Change palette entry: %d",up->id);}
			break;
			case uChunkEdit:
				{struct undoChunkEdit*uc=(struct undoChunkEdit*)uptr->ptr;
				snprintf(tmp,2048,"Edit Chunk ID: %d X: %d Y: %d",uc->id,uc->x,uc->y);}
			break;
			case uChunk:
				{struct undoChunk*uc=(struct undoChunk*)uptr->ptr;
				snprintf(tmp,2048,"Change chunk: %d",uc->id);}
			break;
			case uChunkDelete:
				{struct undoChunk*uc=(struct undoChunk*)uptr->ptr;
				snprintf(tmp,2048,"Delete chunk: %d",uc->id);}
			break;
			case uChunkAppend:
				strcpy(tmp,"Append chunk");
			break;
			case uChunkNew:
				snprintf(tmp,2048,"Insert chunk at %u",unsigned(uintptr_t(uptr->ptr)));
			break;
			case uChunkAll:
				strcpy(tmp,"Change all chunks");
			break;
			case uSpriteAppend:
				snprintf(tmp,2048,"Append sprite to group: %u",unsigned(uintptr_t(uptr->ptr)));
			break;
			case uSpriteAppendgroup:
				strcpy(tmp,"Append sprite group");
			break;
			case uSpriteWidth:
				strcpy(tmp,"Change sprite width");
			break;
			case uSpriteHeight:
				strcpy(tmp,"Change sprite height");
			break;
			case uSpritePalrow:
				strcpy(tmp,"Change sprite palette row");
			break;
			case uSpritestarttile:
				strcpy(tmp,"Change sprite start tile");
			break;
			case uSpriteloadat:
				strcpy(tmp,"Change sprite load at");
			break;
			case uSpriteoffx:
				strcpy(tmp,"Change sprite offset x");
			break;
			case uSpriteoffy:
				strcpy(tmp,"Change sprite offset y");
			break;
			case uSpriteprio:
				strcpy(tmp,"Change sprite priority");
			break;
			case uSpritehflip:
				strcpy(tmp,"Change sprite hflip");
			break;
			case uSpritevflip:
				strcpy(tmp,"Change sprite vflip");
			break;
			case uCurProject:
				strcpy(tmp,"Change current project");
			break;
			default:
				snprintf(tmp,2048,"TODO unhandled %d",uptr->type);
		}
		hist->add(tmp);
	}
	hist->select(pos+1);
	win->end();
	win->set_modal();
	win->show();
	while(win->shown())
		Fl::wait();
	delete win;
}
