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
#include <stdint.h>
#include "gui.h"
#include "includes.h"
#include <cstdarg>
uint32_t map_scroll_pos_x;
uint32_t map_scroll_pos_y;
uint32_t map_off_x,map_off_y;
uint16_t tile_edit_offset_x;
uint16_t tile_edit_offset_y;
uint16_t tile_placer_tile_offset_y;
uint16_t tile_edit_truecolor_off_x,tile_edit_truecolor_off_y;
uint16_t true_color_box_x,true_color_box_y;
unsigned ChunkOff[2]={DefaultChunkX,DefaultChunkY};
unsigned scrollChunks_G[2];
uint_fast32_t editChunk_G[3];//Id,x,y
static int returnVal=0;
static Fl_Choice*PopC;
static Fl_Window * winP;
void setRet(Fl_Widget*,void*r){
	bool Cancel=(uintptr_t)r?true:false;
	if(Cancel)
		returnVal=-1;
	else
		returnVal=PopC->value();
	winP->hide();
}
int MenuPopup(const char * title,const char * text,unsigned num,...){
	if(num){
		winP=new Fl_Window(480,128,title);
		winP->begin();
		Fl_Box * txt=new Fl_Box(FL_NO_BOX,8,8,464,88,text);
		PopC=new Fl_Choice(8,96,192,24);
		va_list arguments;
		va_start(arguments,num);	// Initializing arguments to store all values after num
		for(unsigned x=0;x<num;++x)	// Loop until all numbers are added
			PopC->add(va_arg(arguments, const char*),0,0,0,0);
		PopC->value(0);
		Fl_Button * Ok=new Fl_Button(200,96,64,24,"Okay");
		Ok->callback(setRet,0);
		Fl_Button * Cancel=new Fl_Button(264,96,64,24,"Cancel");
		Cancel->callback(setRet,(void*)1);
		winP->end();
		winP->set_modal();
		winP->show();
		va_end(arguments);		// Cleans up the list
		while(winP->shown())
			Fl::wait();
		delete winP;
		return returnVal;
	}
	return -1;
}
