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
			{struct undoResize*um=(struct undoResize*)uptr->ptr;
			if(um->ptr){
				free(um->ptr);
				memUsed-=getSzResizeGeneric(um->w,um->h,um->wnew,um->hnew,4,1);
			}
			free(uptr->ptr);
			memUsed-=sizeof(struct undoResize);}
		break;
		case uPalette:
			{struct undoPalette*up=(struct undoPalette*)uptr->ptr;
			unsigned sz;
			switch(currentProject->gameSystem){
				case sega_genesis:
					sz=128;
				break;
				case NES:
					sz=32;
				break;
			}
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
void UndoRedo(bool redo){
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
			if(redo)
				currentProject->tileMapC->setRaw(um->x,um->y,um->valnew);
			else{
				um->valnew=currentProject->tileMapC->getRaw(um->x,um->y);
				currentProject->tileMapC->setRaw(um->x,um->y,um->val);
			}
			if(tileEditModePlace_G)
				window->updateTileMapGUI(um->x,um->y);}
		break;
		case uTilemap:
		case uTilemapattr:
			{struct undoTilemap*um=(struct undoTilemap*)uptr->ptr;
			if(redo){
				if(uptr->type==uTilemapattr)
					attrCpyU(currentProject->tileMapC->tileMapDat,(uint8_t*)um->ptrnew,um->wnew*um->hnew);
				else{
					currentProject->tileMapC->resize_tile_map(um->wnew,um->hnew);
					memcpy(currentProject->tileMapC->tileMapDat,um->ptrnew,um->wnew*um->hnew*4);
				}
			}else{
				if(!um->ptrnew){
					um->wnew=currentProject->tileMapC->mapSizeW;
					um->hnew=currentProject->tileMapC->mapSizeHA;
					if(uptr->type==uTilemapattr){
						um->ptrnew=malloc(um->wnew*um->hnew);
						attrCpy((uint8_t*)um->ptrnew,currentProject->tileMapC->tileMapDat,um->wnew*um->hnew);
					}else{
						um->ptrnew=malloc(um->wnew*um->hnew*4);
						memcpy(um->ptrnew,currentProject->tileMapC->tileMapDat,um->wnew*um->hnew*4);
					}
				}
				if(uptr->type==uTilemapattr){
					attrCpyU(currentProject->tileMapC->tileMapDat,(uint8_t*)um->ptr,um->w*um->h);
				}else{
					currentProject->tileMapC->resize_tile_map(um->w,um->h);
					memcpy(currentProject->tileMapC->tileMapDat,um->ptr,um->w*um->h*4);
				}
			}}
		break;
		case uTilemapResize:
			{struct undoResize*um=(struct undoResize*)uptr->ptr;
			if(redo)
				currentProject->tileMapC->resize_tile_map(um->wnew,um->hnew);
			else{
				currentProject->tileMapC->resize_tile_map(um->w,um->h);
				if(um->ptr)
					cpyResizeGeneric((uint8_t*)um->ptr,currentProject->tileMapC->tileMapDat,um->w,um->h,um->wnew,um->hnew,4,1,true);
			}
			window->updateMapWH();}

		break;
		case uPalette:
			{struct undoPalette*up=(struct undoPalette*)uptr->ptr;
			unsigned sz,el;
			switch(currentProject->gameSystem){
				case sega_genesis:
					sz=128;
					el=64;
				break;
				case NES:
					el=sz=32;
				break;
			}
			if(redo)
				memcpy(currentProject->palDat,up->ptrnew,sz);
			else{
				if(!up->ptrnew){
					up->ptrnew=malloc(sz);
					memUsed+=sz;
				}
				memcpy(up->ptrnew,currentProject->palDat,sz);
				memcpy(currentProject->palDat,up->ptr,sz);
			}
			for(unsigned i=0;i<el;++i)
				updateRGBindex(i);
			}
			palEdit.updateSlider();
			tileEdit_pal.updateSlider();
			tileMap_pal.updateSlider();
			spritePal.updateSlider();
		break;
		case uPaletteEntry:
			{struct undoPaletteEntry*up=(struct undoPaletteEntry*)uptr->ptr;
			switch(currentProject->gameSystem){
				case sega_genesis:
					{uint16_t*ptr=(uint16_t*)currentProject->palDat+up->id;
					if(redo)
						*ptr=up->valnew;
					else{
						up->valnew=*ptr;
						*ptr=up->val;
					}}
				break;
				case NES:
					if(redo)
						currentProject->palDat[up->id]=up->valnew;
					else{
						up->valnew=currentProject->palDat[up->id];
						currentProject->palDat[up->id]=up->val;
					}
				break;
			}
			updateRGBindex(up->id);
			switch (mode_editor){
				case pal_edit:
					palEdit.box_sel=up->id%palEdit.perRow;
					palEdit.changeRow(up->id/palEdit.perRow);
				break;
				case tile_edit:
					tileEdit_pal.box_sel=up->id%palEdit.perRow;
					tileEdit_pal.changeRow(up->id/tileEdit_pal.perRow);
					{unsigned focus=0;
					for(unsigned i=0;i<4;++i)
						focus|=Fl::focus()==window->palRTE[i];
					for(unsigned i=0;i<4;++i){
						if(focus&&(i==tileEdit_pal.theRow))
							Fl::focus(window->palRTE[i]);
						window->palRTE[i]->value(i==tileEdit_pal.theRow);
					}}
				break;
				case tile_place:
					tileMap_pal.box_sel=up->id%tileMap_pal.perRow;
					tileMap_pal.changeRow(up->id/tileMap_pal.perRow);
					{unsigned focus=0;
					for(unsigned i=0;i<4;++i)
						focus|=Fl::focus()==window->palRTE[i+4];
					for(unsigned i=0;i<4;++i){
						if(focus&&(i==tileMap_pal.theRow))
							Fl::focus(window->palRTE[i+4]);
						window->palRTE[i+4]->value(i==tileMap_pal.theRow);
					}}
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
			window->chunk_select->maximum(currentProject->Chunk->amt-1);
		break;
		case uChunkNew:
			if(redo)
				currentProject->Chunk->insert((uintptr_t)uptr->ptr);
			else
				currentProject->Chunk->removeAt((uintptr_t)uptr->ptr);
			window->chunk_select->maximum(currentProject->Chunk->amt-1);
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
			window->updateChunkSize();
			window->chunk_select->maximum(currentProject->Chunk->amt-1);}
		break;
		case uChunk:
		case uChunkDelete:
			{struct undoChunk*uc=(struct undoChunk*)uptr->ptr;

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
	}
	if(!redo)
		--pos;
	window->redraw();
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
	um->w=currentProject->tileMapC->mapSizeW;
	um->h=currentProject->tileMapC->mapSizeHA;
	um->ptrnew=0;
	if(attrOnly){
		um->ptr=malloc(um->w*um->h);
		attrCpy((uint8_t*)um->ptr,currentProject->tileMapC->tileMapDat,um->w*um->h);
	}else{
		um->ptr=malloc(um->w*um->h*4);
		memcpy(um->ptr,currentProject->tileMapC->tileMapDat,um->w*um->h*4);
	}
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
	um->val=currentProject->tileMapC->getRaw(x,y);
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
	pushResize(wnew,hnew,currentProject->tileMapC->mapSizeW,currentProject->tileMapC->mapSizeHA,currentProject->tileMapC->tileMapDat,uTilemapResize,4,1);
}
void pushPaletteAll(void){
	pushEventPrepare();
	struct undoEvent*uptr=undoBuf+pos;
	uptr->type=uPalette;
	uptr->ptr=malloc(sizeof(struct undoPalette));
	memUsed+=sizeof(struct undoPalette);
	struct undoPalette*up=(struct undoPalette*)uptr->ptr;
	switch(currentProject->gameSystem){
		case sega_genesis:
			up->ptr=malloc(128);
			memcpy(up->ptr,currentProject->palDat,128);
			memUsed+=128;
		break;
		case NES:
			up->ptr=malloc(32);
			memcpy(up->ptr,currentProject->palDat,32);
			memUsed+=32;
		break;
	}
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
	switch(currentProject->gameSystem){
		case sega_genesis:
			{uint16_t*ptr=(uint16_t*)currentProject->palDat+id;
			up->val=*ptr;}
		break;
		case NES:
			up->val=(int32_t)currentProject->palDat[id];
		break;
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
	uc->ptrnew=0;
	uc->ptr=(struct ChunkAttrs*)malloc(currentProject->Chunk->wi*currentProject->Chunk->hi*sizeof(struct ChunkAttrs));
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
			case uTilemapEdit:
				{struct undoTilemapEdit*um=(struct undoTilemapEdit*)uptr->ptr;
				snprintf(tmp,2048,"Edit tilemap X: %d Y: %d",um->x,um->y);}	
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
