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
void shareProjectCB(Fl_Widget*o,void*mask){
	Fl_Check_Button* b=(Fl_Check_Button*)o;
	if(projects_count<=1){
		fl_alert("Cannot share when there is only one project");
		b->value(0);
		window->redraw();
		return;
	}
	if(b->value()){
		if(!fl_ask("Warning this will delete this project's data\nDo you wish to countinue?")){
			b->value(0);
			window->redraw();
			return;
		}
	}
	uint32_t m=(uintptr_t)mask;
	//printf("%d %d\n",m,__builtin_ctz(m));
	shareProject(curProjectID,window->shareWith[__builtin_ctz(m)]->value(),m,b->value()?true:false);
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
	window->projectSelect->maximum(projects_count-1);
	if(curProjectID){
		curProjectID--;
		window->projectSelect->value(curProjectID);
	}
	currentProject=projects[curProjectID];
	switchProject(curProjectID);
}
