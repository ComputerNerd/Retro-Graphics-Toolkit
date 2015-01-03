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
#include "project.h"
#include "classtilemaps.h"
#include "callbacktilemaps.h"
#include "undo.h"
void setCurPlaneTilemaps(Fl_Widget*,void*val){
	currentProject->curPlane=(uintptr_t)val;
	window->BlocksCBtn->value(currentProject->tms->maps[currentProject->curPlane].isBlock);
	currentProject->tms->maps[currentProject->curPlane].toggleBlocks(currentProject->tms->maps[currentProject->curPlane].isBlock);
	window->updateTileMapGUI(selTileE_G[0],selTileE_G[1]);
	window->curPlaneName->value(currentProject->tms->planeName[currentProject->curPlane].c_str());
	window->redraw();
}
void removeTilemapsPlane(Fl_Widget*,void*){
	if(currentProject->tms->maps.size()>1){
		pushTilemapPlaneDelete(currentProject->curPlane);
		currentProject->tms->removePlane(currentProject->curPlane);
		if(currentProject->curPlane)
			--currentProject->curPlane;
		window->planeSelect->value(currentProject->curPlane);
		updatePlaneTilemapMenu();
		setCurPlaneTilemaps(0,(void*)(uintptr_t)currentProject->curPlane);
	}else
		fl_alert("This is the improper way to disable planes");
}
void updateNameTilemaps(Fl_Widget*w,void*){
	Fl_Input*wi=(Fl_Input*)w;
	currentProject->tms->planeName[currentProject->curPlane].assign(wi->value());
	window->planeSelect->replace(currentProject->curPlane,currentProject->tms->planeName[currentProject->curPlane].c_str());
	window->redraw();
}
void updatePlaneTilemapMenu(uint32_t id,Fl_Choice*plM){
	if(plM->size())
		plM->clear();
	for(uintptr_t i=0;i<projects[id]->tms->maps.size();++i)
		plM->add(projects[id]->tms->planeName[i].c_str(),0,setCurPlaneTilemaps,(void*)i,0);
	plM->value(projects[id]->curPlane);
}
void updatePlaneTilemapMenu(void){
	updatePlaneTilemapMenu(curProjectID,window->planeSelect);
}
void addPlaneTilemap(Fl_Widget*,void*val){
	pushTilemapPlaneAdd(currentProject->tms->maps.size());
	currentProject->tms->setPlaneCnt(currentProject->tms->maps.size()+1);
	uintptr_t i=currentProject->tms->maps.size()-1;
	window->planeSelect->add(currentProject->tms->planeName[i].c_str(),0,setCurPlaneTilemaps,(void*)i,0);
	window->redraw();
}
