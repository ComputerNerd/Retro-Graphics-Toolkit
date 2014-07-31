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
#include <cstdarg>
#include <stdio.h>
#include "gui.h"
#include "includes.h"
#include "project.h"
uint32_t map_scroll_pos_x;
uint32_t map_scroll_pos_y;
uint32_t map_off_x,map_off_y;
uint16_t tile_edit_offset_x;
uint16_t tile_edit_offset_y;
uint16_t tile_placer_tile_offset_y;
uint16_t tile_edit_truecolor_off_x,tile_edit_truecolor_off_y;
uint16_t true_color_box_x,true_color_box_y;
static int returnVal=0;
static Fl_Choice*PopC;
static Fl_Window * winP;
unsigned SpriteOff[2];
static void zeroError(const char*more){
	fl_alert("Please enter a value greater than zero\n%s",more);
}
int SafeTxtInput(Fl_Int_Input*in){
	int val=atoi(in->value());
	if(val<=0){
		if(val==0){
			zeroError("Defaulting to one");
			val=1;
		}else{
			zeroError("Defaulting to absoulute value");
			val=-val;
		}
		char tmp[16];
		snprintf(tmp,16,"%d",val);
		in->value(tmp);
		window->redraw();
	}
	return val;
}
void updateTileSelectAmt(uint32_t newMax){
	--newMax;
	window->tile_select->maximum(newMax);
	window->tile_select_2->maximum(newMax);
	if(currentProject->Chunk->useBlocks)
		window->tile_select_3->maximum(currentProject->tileMapC->amt-1);
	else
		window->tile_select_3->maximum(newMax);
	window->spritest->maximum(newMax);
	char tmp[128];
	snprintf(tmp,128,"Total tiles: %d",newMax+1);
	window->totalTiles->copy_label(tmp);
}
void updateTileSelectAmt(void){
	updateTileSelectAmt(currentProject->tileC->amt);
}
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
		if(text)
			new Fl_Box(FL_NO_BOX,8,8,464,88,text);
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
bool load_file_generic(const char * the_tile,bool save_file){//Warning this function sets global variable string the_file
	// Create native chooser
	Fl_Native_File_Chooser native;
	native.title(the_tile);
	if(save_file)
		native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	else
		native.type(Fl_Native_File_Chooser::BROWSE_FILE);
	// Show native chooser
	switch (native.show()){
		case -1:
			//fprintf(stderr, "ERROR: %s\n", native.errmsg());
			fl_alert("Error %s",native.errmsg());
		break;	// ERROR
		case  1:
			fprintf(stderr, "*** CANCEL\n");
			//fl_beep();
		break;		// CANCEL
		default:// Picked File
			if (native.filename()){
				the_file=native.filename();
				return true;//the only way this this function will return true is the user picked a file
			}
		break;
	}
	return false;//if an error happened or the user did not pick a file the function returns false
}
