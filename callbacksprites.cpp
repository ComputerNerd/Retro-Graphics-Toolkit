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
#include <ctime>
#include "includes.h"
#include "classSprites.h"
#include "gui.h"
#include "project.h"
#include "gamedef.h"
#include "undo.h"
#include "dither.h"
#include "classpalettebar.h"
uint32_t curSprite;
uint32_t curSpritegroup;
int32_t spriteEndDraw[2];
bool centerSpriteDraw_G;
void palRowstCB(Fl_Widget*,void*){
	unsigned st=currentProject->spritesC->groups[curSpritegroup].list[curSprite].starttile;
	unsigned palrow=currentProject->spritesC->groups[curSpritegroup].list[curSprite].palrow;
	for(unsigned j=0;j<currentProject->spritesC->amt;++j){
		for(unsigned i=0;i<currentProject->spritesC->groups[j].list.size();++i){
			if(st==currentProject->spritesC->groups[j].list[i].starttile)
				currentProject->spritesC->groups[j].list[i].palrow=palrow;
		}
	}
	window->updateSpriteSliders();
	window->redraw();
}
void optimizeSpritesCB(Fl_Widget*,void*){
	for(unsigned i=0;i<currentProject->spritesC->amt;++i){
		currentProject->spritesC->freeOptmizations(i);
	}
	window->updateSpriteSliders();
	window->redraw();
}
void ditherSpriteAsImage(unsigned which){
	unsigned w,h;
	w=currentProject->spritesC->width(which);
	h=currentProject->spritesC->height(which);
	uint8_t*image=(uint8_t*)malloc(w*h*4);
	if (!image)
		show_malloc_error(w*h*4)
	pushTilesAll(tTypeTile);
	for (unsigned row=0;row<4;++row){
		currentProject->spritesC->spriteGroupToImage(image,which,row);
		ditherImage(image,w,h,true,true,false,0,false,0,true);
		ditherImage(image,w,h,true,false,true,row,false,0,true);
		currentProject->spritesC->spriteImageToTiles(image,which,row);
	}
	Fl::check();
	free(image);
}
void ditherSpriteAsImageAllCB(Fl_Widget*,void*){
	Fl_Window *winP;
	Fl_Progress *progress;
	mkProgress(&winP,&progress);
	progress->maximum(currentProject->spritesC->amt-1);
	time_t lasttime=time(NULL);
	Fl::check();
	for(unsigned i=0;i<currentProject->spritesC->amt;++i){
		ditherSpriteAsImage(i);
		if((time(NULL)-lasttime)>=1){
			lasttime=time(NULL);
			progress->value(i);
			Fl::check();
		}
	}
	winP->remove(progress);// remove progress bar from window
	delete(progress);// deallocate it
	delete winP;
	Fl::check();
	window->redraw();
}
void ditherSpriteAsImageCB(Fl_Widget*,void*){
	ditherSpriteAsImage(curSpritegroup);
	window->redraw();
}
void setDrawSpriteCB(Fl_Widget*,void*m){
	centerSpriteDraw_G=(m)?true:false;
	window->redraw();
}
void SpriteSheetimportCB(Fl_Widget*o,void*){
	currentProject->spritesC->importSpriteSheet();
	window->updateSpriteSliders();
	window->redraw();
}
void assignSpriteglobalnameCB(Fl_Widget*o,void*){
	Fl_Input*i=(Fl_Input*)o;
	currentProject->spritesC->name.assign(i->value());
	window->redraw();
}
void exportSonicDPLCCB(Fl_Widget*o,void*t){
	currentProject->spritesC->exportDPLC((gameType_t)(uintptr_t)t);
}
void alignSpriteCB(Fl_Widget*,void*t){
	uint32_t with;
	if(currentProject->spritesC->groups[curSpritegroup].list.size()<=1){
		fl_alert("You must have at least two sprites to align");
		return;
	}
	pushSpriteOffx();
	pushSpriteOffy();
	if(curSprite)
		with=curSprite-1;
	else
		with=curSprite+1;
	switch((uintptr_t)t){
		case 0://Left
			currentProject->spritesC->groups[curSpritegroup].offx[curSprite]=currentProject->spritesC->groups[curSpritegroup].offx[with]-(currentProject->spritesC->groups[curSpritegroup].list[with].w*8);
			currentProject->spritesC->groups[curSpritegroup].offy[curSprite]=currentProject->spritesC->groups[curSpritegroup].offy[with];
		break;
		case 1://Right
			currentProject->spritesC->groups[curSpritegroup].offx[curSprite]=currentProject->spritesC->groups[curSpritegroup].offx[with]+(currentProject->spritesC->groups[curSpritegroup].list[with].w*8);
			currentProject->spritesC->groups[curSpritegroup].offy[curSprite]=currentProject->spritesC->groups[curSpritegroup].offy[with];
		break;
		case 2://Top
			currentProject->spritesC->groups[curSpritegroup].offx[curSprite]=currentProject->spritesC->groups[curSpritegroup].offx[with];
			currentProject->spritesC->groups[curSpritegroup].offy[curSprite]=currentProject->spritesC->groups[curSpritegroup].offy[with]-(currentProject->spritesC->groups[curSpritegroup].list[with].h*8);
		break;
		case 3://Bottom
			currentProject->spritesC->groups[curSpritegroup].offx[curSprite]=currentProject->spritesC->groups[curSpritegroup].offx[with];
			currentProject->spritesC->groups[curSpritegroup].offy[curSprite]=currentProject->spritesC->groups[curSpritegroup].offy[with]+(currentProject->spritesC->groups[curSpritegroup].list[with].h*8);
		break;
	}
	window->updateSpriteSliders();
	window->redraw();
}
void importSonicDPLCCB(Fl_Widget*o,void*t){
	currentProject->spritesC->importDPLC((gameType_t)(uintptr_t)t);
	window->updateSpriteSliders();
	window->redraw();
}
void setoffspriteCB(Fl_Widget*o,void*y){
	Fl_Int_Input*i=(Fl_Int_Input*)o;
	int tmp=atoi(i->value());
	if(y){
		pushSpriteOffy();
		currentProject->spritesC->groups[curSpritegroup].offy[curSprite]=tmp;	
	}else{
		pushSpriteOffx();
		currentProject->spritesC->groups[curSpritegroup].offx[curSprite]=tmp;	
	}
	window->redraw();
}
void exportSonicMappingCB(Fl_Widget*o,void*t){
	currentProject->spritesC->exportMapping((gameType_t)(uintptr_t)t);
}
void importSonicMappingCB(Fl_Widget*o,void*t){
	currentProject->spritesC->importMapping((gameType_t)(uintptr_t)t);
	window->updateSpriteSliders();
	window->redraw();
}
void assignSpritegroupnameCB(Fl_Widget*o,void*){
	Fl_Input*i=(Fl_Input*)o;
	currentProject->spritesC->groups[curSpritegroup].name.assign(i->value());
	window->redraw();
}
void spritePrioCB(Fl_Widget*,void*){
	pushSpritePrio();
	currentProject->spritesC->groups[curSpritegroup].list[curSprite].prio^=true;
	window->redraw();
}
void spriteHflipCB(Fl_Widget*,void*){
	pushSpriteHflip();
	currentProject->spritesC->groups[curSpritegroup].list[curSprite].hflip^=true;
	window->redraw();
}
void spriteVflipCB(Fl_Widget*,void*){
	pushSpriteVflip();
	currentProject->spritesC->groups[curSpritegroup].list[curSprite].vflip^=true;
	window->redraw();
}
void SpriteimportCB(Fl_Widget*,void*append){
	if((uintptr_t)append)
		currentProject->spritesC->importImg(currentProject->spritesC->amt);//This works because amt counts from one and the function counts from zero
	else
		currentProject->spritesC->importImg(curSprite);
}
void selSpriteCB(Fl_Widget*w,void*){
	Fl_Slider*s=(Fl_Slider*)w;
	curSprite=s->value();
	window->updateSpriteSliders();
	window->redraw();
}
void appendSpriteCB(Fl_Widget*,void*g){
	if(g){
		pushSpriteAppendgroup();
		currentProject->spritesC->setAmt(currentProject->spritesC->amt+1);
	}else{
		pushSpriteAppend(curSpritegroup);
		currentProject->spritesC->setAmtingroup(curSpritegroup,currentProject->spritesC->groups[curSpritegroup].list.size()+1);
	}
	window->updateSpriteSliders();
	window->redraw();
}
void delSpriteCB(Fl_Widget*,void*group){
	if(group)
		currentProject->spritesC->del(curSpritegroup);
	else
		currentProject->spritesC->delingroup(curSpritegroup,curSprite);
	window->updateSpriteSliders();
	window->redraw();
}
void selspriteGroup(Fl_Widget*o,void*){
	Fl_Slider*s=(Fl_Slider*)o;
	curSpritegroup=s->value();
	window->updateSpriteSliders();
	window->redraw();
}
#define pushSpriteItem(fname) if(pushed_g||(Fl::event()==FL_KEYDOWN)){ \
		pushed_g=0; \
		pushSprite##fname(); \
	}
void setvalueSpriteCB(Fl_Widget*o,void*which){
	Fl_Slider*v=(Fl_Slider*)o;
	uint32_t val=v->value();
	switch((uintptr_t)which){
		case 0:
			pushSpriteItem(Starttile)
			currentProject->spritesC->groups[curSpritegroup].list[curSprite].starttile=val;
		break;
		case 1:
			pushSpriteItem(Width)
			currentProject->spritesC->groups[curSpritegroup].list[curSprite].w=val;
		break;
		case 2:
			pushSpriteItem(Height)
			currentProject->spritesC->groups[curSpritegroup].list[curSprite].h=val;
		break;
		case 3:
			pushSpriteItem(Palrow)
			palBar.changeRow(val,3);
			currentProject->spritesC->groups[curSpritegroup].list[curSprite].palrow=val;
		break;
		case 4:
			pushSpriteItem(Loadat)
			currentProject->spritesC->groups[curSpritegroup].loadat[curSprite]=val;
		break;
	}
	window->redraw();
}
