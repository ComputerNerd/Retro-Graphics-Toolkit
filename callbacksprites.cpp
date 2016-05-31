/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2016)
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
#include "class_global.h"
uint32_t curSprite;
uint32_t curSpritegroup;
uint32_t curSpritemeta;
int32_t spriteEndDraw[2];
bool centerSpriteDraw_G;
void palRowstCB(Fl_Widget*,void*){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	unsigned st=currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].starttile;
	unsigned palrow=currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].palrow;
	for(unsigned j=0;j<currentProject->ms->sps[msprt].amt;++j){
		for(unsigned i=0;i<currentProject->ms->sps[msprt].groups[j].list.size();++i){
			if(st==currentProject->ms->sps[msprt].groups[j].list[i].starttile)
				currentProject->ms->sps[msprt].groups[j].list[i].palrow=palrow;
		}
	}
	window->updateSpriteSliders();
	window->redraw();
}
void optimizeSpritesCB(Fl_Widget*,void*){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	for(unsigned i=0;i<currentProject->ms->sps[msprt].amt;++i){
		currentProject->ms->sps[msprt].freeOptmizations(i);
	}
	window->updateSpriteSliders();
	window->redraw();
}
void ditherSpriteAsImage(unsigned msprt,unsigned which){
	unsigned w,h;
	w=currentProject->ms->sps[msprt].width(which);
	h=currentProject->ms->sps[msprt].height(which);
	uint8_t*image=(uint8_t*)malloc(w*h*4);
	if (!image)
		show_malloc_error(w*h*4)
	pushTilesAll(tTypeTile);
	for(unsigned row=0;row<(currentProject->pal->haveAlt?currentProject->pal->rowCntPalalt:currentProject->pal->rowCntPal);++row){
		currentProject->ms->sps[msprt].spriteGroupToImage(image,which,row);
		ditherImage(image,w,h,true,true,false,0,false,0,true);
		void*indexPtr=ditherImage(image,w,h,true,false,true,row,false,0,true,true);
		currentProject->ms->sps[msprt].spriteImageToTiles((uint8_t*)indexPtr,which,row,true,true);
		free(indexPtr);
	}
	Fl::check();
	free(image);
}
void ditherGroupAsImage(unsigned msprt){
	Fl_Window *winP;
	Fl_Progress *progress;
	mkProgress(&winP,&progress);
	progress->maximum(currentProject->ms->sps[msprt].amt-1);
	time_t lasttime=time(nullptr);
	Fl::check();
	for(unsigned i=0;i<currentProject->ms->sps[msprt].amt;++i){
		ditherSpriteAsImage(msprt,i);
		if((time(nullptr)-lasttime)>=1){
			lasttime=time(nullptr);
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
void ditherSpriteAsImageAllCB(Fl_Widget*,void*){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	ditherGroupAsImage(window->metaspritesel->value());
}
void ditherSpriteAsImageCB(Fl_Widget*,void*){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	ditherSpriteAsImage(msprt,curSpritegroup);
	window->redraw();
}
void setDrawSpriteCB(Fl_Widget*,void*m){
	centerSpriteDraw_G=(m)?true:false;
	window->redraw();
}
void spriteSheetimportCB(Fl_Widget*o,void*){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	currentProject->ms->sps[msprt].importSpriteSheet();
	window->updateSpriteSliders();
	window->redraw();
}
void assignSpriteAllMetanameCB(Fl_Widget*o,void*){
	Fl_Input*i=(Fl_Input*)o;
	currentProject->ms->name.assign(i->value());
	window->redraw();
}
void assignSpritemetaNameCB(Fl_Widget*o,void*){
	unsigned msprt=window->metaspritesel->value();
	Fl_Input*i=(Fl_Input*)o;
	currentProject->ms->sps[msprt].name.assign(i->value());
	window->redraw();
}
void exportSonicDPLCCB(Fl_Widget*o,void*t){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	currentProject->ms->sps[msprt].exportDPLC((gameType_t)(uintptr_t)t);
}
void alignSpriteCB(Fl_Widget*,void*t){
	unsigned msprt=window->metaspritesel->value();
	uint32_t with;
	if(currentProject->ms->sps[msprt].groups[curSpritegroup].list.size()<=1){
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
			currentProject->ms->sps[msprt].groups[curSpritegroup].offx[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offx[with]-(currentProject->ms->sps[msprt].groups[curSpritegroup].list[with].w*8);
			currentProject->ms->sps[msprt].groups[curSpritegroup].offy[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offy[with];
		break;
		case 1://Right
			currentProject->ms->sps[msprt].groups[curSpritegroup].offx[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offx[with]+(currentProject->ms->sps[msprt].groups[curSpritegroup].list[with].w*8);
			currentProject->ms->sps[msprt].groups[curSpritegroup].offy[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offy[with];
		break;
		case 2://Top
			currentProject->ms->sps[msprt].groups[curSpritegroup].offx[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offx[with];
			currentProject->ms->sps[msprt].groups[curSpritegroup].offy[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offy[with]-(currentProject->ms->sps[msprt].groups[curSpritegroup].list[with].h*8);
		break;
		case 3://Bottom
			currentProject->ms->sps[msprt].groups[curSpritegroup].offx[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offx[with];
			currentProject->ms->sps[msprt].groups[curSpritegroup].offy[curSprite]=currentProject->ms->sps[msprt].groups[curSpritegroup].offy[with]+(currentProject->ms->sps[msprt].groups[curSpritegroup].list[with].h*8);
		break;
	}
	window->updateSpriteSliders();
	window->redraw();
}
void importSonicDPLCCB(Fl_Widget*o,void*t){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	currentProject->ms->sps[msprt].importDPLC((gameType_t)(uintptr_t)t);
	window->updateSpriteSliders();
	window->redraw();
}
void setoffspriteCB(Fl_Widget*o,void*y){
	unsigned msprt=window->metaspritesel->value();
	Fl_Int_Input*i=(Fl_Int_Input*)o;
	int tmp=atoi(i->value());
	if(y){
		pushSpriteOffy();
		currentProject->ms->sps[msprt].groups[curSpritegroup].offy[curSprite]=tmp;	
	}else{
		pushSpriteOffx();
		currentProject->ms->sps[msprt].groups[curSpritegroup].offx[curSprite]=tmp;	
	}
	window->redraw();
}
void exportSonicMappingCB(Fl_Widget*o,void*t){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	currentProject->ms->sps[msprt].exportMapping((gameType_t)(uintptr_t)t);
}
void importSonicMappingCB(Fl_Widget*o,void*t){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	currentProject->ms->sps[msprt].importMapping((gameType_t)(uintptr_t)t);
	window->updateSpriteSliders();
	window->redraw();
}
void assignSpritegroupnameCB(Fl_Widget*o,void*){
	unsigned msprt=window->metaspritesel->value();
	Fl_Input*i=(Fl_Input*)o;
	currentProject->ms->sps[msprt].groups[curSpritegroup].name.assign(i->value());
	window->redraw();
}
void spritePrioCB(Fl_Widget*,void*){
	unsigned msprt=window->metaspritesel->value();
	pushSpritePrio();
	currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].prio^=true;
	window->redraw();
}
void spriteHflipCB(Fl_Widget*,void*){
	unsigned msprt=window->metaspritesel->value();
	pushSpriteHflip();
	currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].hflip^=true;
	window->redraw();
}
void spriteVflipCB(Fl_Widget*,void*){
	unsigned msprt=window->metaspritesel->value();
	pushSpriteVflip();
	currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].vflip^=true;
	window->redraw();
}
void SpriteimportCB(Fl_Widget*,void*append){
	if(!currentProject->containsData(pjHaveSprites)){
		currentProject->haveMessage(pjHaveSprites);
		return;
	}
	unsigned msprt=window->metaspritesel->value();
	if((uintptr_t)append)
		currentProject->ms->sps[msprt].importImg(currentProject->ms->sps[msprt].amt);//This works due to the fact that amt counts from one and the function counts from zero
	else
		currentProject->ms->sps[msprt].importImg(curSprite);
}
void selSpriteCB(Fl_Widget*w,void*){
	Fl_Slider*s=(Fl_Slider*)w;
	curSprite=s->value();
	window->updateSpriteSliders();
	window->redraw();
}
void appendSpriteCB(Fl_Widget*,void*g){
	unsigned msprt=window->metaspritesel->value();
	intptr_t typ=(intptr_t)g;
	if(typ==1){
		pushSpriteAppendgroup();
		currentProject->ms->sps[msprt].setAmt(currentProject->ms->sps[msprt].amt+1);
	}else if(typ==2){
		pushSpriteAppendmeta();
		currentProject->ms->sps.emplace_back(sprites(currentProject));
	}else{
		pushSpriteAppend(curSpritegroup);
		currentProject->ms->sps[msprt].setAmtingroup(curSpritegroup,currentProject->ms->sps[msprt].groups[curSpritegroup].list.size()+1);
	}
	window->updateSpriteSliders();
	window->redraw();
}
void delSpriteCB(Fl_Widget*,void*group){
	unsigned msprt=window->metaspritesel->value();
	if(group)
		currentProject->ms->sps[msprt].del(curSpritegroup);
	else
		currentProject->ms->sps[msprt].delingroup(curSpritegroup,curSprite);
	window->updateSpriteSliders();
	window->redraw();
}
void selspriteGroup(Fl_Widget*o,void*){
	Fl_Slider*s=(Fl_Slider*)o;
	curSpritegroup=s->value();
	window->updateSpriteSliders();
	window->redraw();
}
void selspriteMeta(Fl_Widget*o,void*){
	Fl_Slider*s=(Fl_Slider*)o;
	curSpritemeta=s->value();
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
	unsigned msprt=window->metaspritesel->value();
	switch((uintptr_t)which){
		case 0:
			pushSpriteItem(Starttile)
			currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].starttile=val;
		break;
		case 1:
			pushSpriteItem(Width)
			currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].w=val;
		break;
		case 2:
			pushSpriteItem(Height)
			currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].h=val;
		break;
		case 3:
			pushSpriteItem(Palrow)
			palBar.changeRow(val,3);
			currentProject->ms->sps[msprt].groups[curSpritegroup].list[curSprite].palrow=val;
		break;
		case 4:
			pushSpriteItem(Loadat)
			currentProject->ms->sps[msprt].groups[curSpritegroup].loadat[curSprite]=val;
		break;
	}
	window->redraw();
}
