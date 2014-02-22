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
#include "project.h"
#include "includes.h"
uint32_t currentChunck=0;
void ImportS1CBChuncks(Fl_Widget*,void*a){
	bool append=(uintptr_t)a?true:false;
	if(load_file_generic("Pick chunck data from sonic 1")){
		currentProject->Chunck->importSonic1(the_file.c_str(),append);
		window->redraw();
	}
}
void currentChunckCB(Fl_Widget*,void*){
	currentChunck=window->chunck_select->value();
	window->redraw();
}
void useBlocksCB(Fl_Widget*o,void*){
	Fl_Check_Button*b=(Fl_Check_Button*)o;
	bool use=b->value()?true:false;
	if((use)&&(!(currentProject->tileMapC->isBlock))){
		fl_alert("You cannot use blocks without blocks");
		b->value(0);
		window->redraw();
		return;
	}
	currentProject->Chunck->useBlocks=use;
	window->redraw();
}
void scrollChunckCB(Fl_Widget*,void*){
	currentProject->Chunck->scrollChuncks();
	window->redraw();
}
void scrollChunckX(Fl_Widget*,void*){
	scrollChunks[0]=window->chunckX->value();
	window->redraw();
}
void scrollChunckY(Fl_Widget*,void*){
	scrollChunks[1]=window->chunckY->value();
	window->redraw();
}
