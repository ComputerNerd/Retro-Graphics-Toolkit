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
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#include "global.h"
#include "project.h"
static const char * warningDelete="Warning this will delete this project's data\nDo you wish to countinue?";
void setSubSysCB(Fl_Widget*o,void*){
	Fl_Choice *c=(Fl_Choice*)o;
	currentProject->subSystem=c->value();
}
void saveAllProjectsCB(Fl_Widget*,void*){
	saveAllProjects();
}
void loadAllProjectsCB(Fl_Widget*,void*o){
	loadAllProjects((uintptr_t)o?true:false);
	switchProject(curProjectID);
}
void haveCB(Fl_Widget*o,void*mask){
	Fl_Check_Button* b=(Fl_Check_Button*)o;
	uint32_t m=(uintptr_t)mask;
	bool set=b->value()?true:false;
	if(!set){
		if(!fl_ask(warningDelete)){
			b->value(1);
			window->redraw();
			return;
		}
	}
	if((m&pjHavePal)&&(!set)){//cannot have tiles without a palette
		if(((currentProject->useMask&pjAllMask)&(~pjHavePal))){
			/*the (above) if statment is based on the observation that all other have settings involve tiles
			be sure to change this if another function not involving tiles arries*/
			if((currentProject->share[1]<0)&&(currentProject->share[2]<0)){
				fl_alert("You cannot have tiles without a palette");
				b->value(1);
				window->redraw();
				return;
			}
		}
	}
	//Can not have tilemap without tiles
	if((m&pjHaveTiles)&&(!set)){
		if(currentProject->useMask&pjHaveMap){
			if(currentProject->share[1]<0){
				fl_alert("You cannot have tile map without tiles.");
				b->value(1);
				window->redraw();
				return;
			}
		}
	}
	if((m&(~pjHavePal))&&set){//Ensure that a palette exists before enabling anything else
		if(!(currentProject->useMask&pjHavePal)){
			if(currentProject->share[0]<0){
				fl_alert("You need a palette to do this");
				b->value(0);
				window->redraw();
				return;
			}
		}
	}
	if((m&pjHaveMap)&&set){//Are we trying to enable tilemap?
		if(!(currentProject->useMask&pjHaveTiles)){
			if(currentProject->share[1]<0){
				fl_alert("You cannot have tile map without tiles.");
				b->value(0);
				window->redraw();
				return;
			}
		}
	}
	setHaveProject(curProjectID,m,set);
	unsigned off=__builtin_ctz(m);
	if(set){
		if(window->tabsHidden[off]){
			window->the_tabs->insert(*window->TabsMain[off],off);
			window->tabsHidden[off]=false;
		}
	}else{
		if(!window->tabsHidden[off]){
			if(currentProject->share[off]<0){
				window->the_tabs->remove(window->TabsMain[off]);
				window->tabsHidden[off]=true;
			}
		}
	}
	window->redraw();
}
static void updateShareHave(void){
	for(int x=0;x<shareAmtPj;++x){
		if(currentProject->share[x]<0)
			window->havePrj[x]->show();
		else
			window->havePrj[x]->hide();
	}
}
void switchShareCB(Fl_Widget*o,void*mask){
	Fl_Slider* s=(Fl_Slider*)o;
	uint32_t m=(uintptr_t)mask;
	bool share=window->sharePrj[__builtin_ctz(m)]->value()?true:false;
	printf("Change mask %d %d\n",m,share);
	if(share)
		shareProject(curProjectID,s->value(),m,true);
	updateShareHave();
}
void shareProjectCB(Fl_Widget*o,void*mask){
	Fl_Check_Button* b=(Fl_Check_Button*)o;
	if(projects_count<=1){
		fl_alert("Cannot share when there is only one project");
		b->value(0);
		window->redraw();
		return;
	}
	uint32_t m=(uintptr_t)mask;
	uint32_t with=window->shareWith[__builtin_ctz(m)]->value();
	if(curProjectID==with){
		fl_alert("One does not simply share with itself");
		b->value(0);
		window->redraw();
		return;
	}
	if(b->value()){
		if(!fl_ask(warningDelete)){
			b->value(0);
			window->redraw();
			return;
		}
	}
	printf("%d with %d %d %d\n",curProjectID,with,m,__builtin_ctz(m));
	shareProject(curProjectID,with,m,b->value()?true:false);
	updateShareHave();
	window->redraw();
}
void loadProjectCB(Fl_Widget*,void*){
	loadProject(curProjectID);
	switchProject(curProjectID);
}
void saveProjectCB(Fl_Widget*,void*){
	currentProject->Name.assign(window->TxtBufProject->text());//Make sure text is up to date
	saveProject(curProjectID);
}
void switchProjectCB(Fl_Widget*o,void*){
	Fl_Slider* s=(Fl_Slider*)o;
	currentProject->Name.assign(window->TxtBufProject->text());//Save text to old project
	curProjectID=s->value();
	currentProject=projects[curProjectID];
	switchProject(curProjectID);
}
void appendProjectCB(Fl_Widget*,void*){
	appendProject();
	window->redraw();
}
void deleteProjectCB(Fl_Widget*,void*){
	if (projects_count<=1){
		fl_alert("You must have atleast one project.");
		return;
	}
	removeProject(curProjectID);
	if(curProjectID){
		curProjectID--;
		window->projectSelect->value(curProjectID);
	}
	currentProject=projects[curProjectID];
	switchProject(curProjectID);
}
