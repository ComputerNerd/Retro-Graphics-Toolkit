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
#include "callbacks_palette.h"
#include "class_palette.h"
#include "color_convert.h"
palette_bar palEdit;
palette_bar tileEdit_pal;
palette_bar tileMap_pal;
uint8_t palette_bar::getEntry(void){
	return box_sel+(theRow*perRow);
}
void palette_bar::more_init(uint8_t x,uint16_t offsetx,uint16_t offsety){
	switch (currentProject->gameSystem){
		case sega_genesis:
			perRow=16;
		break;
		case NES:
			perRow=4;
		break;
	}
	rows=x;
	offxx=offsetx;
	offyy=offsety;
	offx=offsetx;
	offy=offsety;
	pal_r = new Fl_Hor_Value_Slider(offx+32,offy+8+(rows*32),256,24,"Red");
	pal_r->minimum(0); pal_r->maximum(0x0E);
	pal_r->step(2);
	pal_r->value(0);
	pal_r->align(FL_ALIGN_LEFT);
	pal_r->callback(update_palette, (void*)0);

	pal_g = new Fl_Hor_Value_Slider(offx+32,offy+40+(rows*32),256,24,"Green");
	pal_g->minimum(0); pal_g->maximum(0x0E);
	pal_g->step(2);
	pal_g->value(0);
	pal_g->align(FL_ALIGN_LEFT);
	pal_g->callback(update_palette, (void*)1);

	pal_b = new Fl_Hor_Value_Slider(offx+32,offy+72+(rows*32),256,24,"Blue");
	pal_b->minimum(0); pal_b->maximum(0x0E);
	pal_b->step(2);
	pal_b->value(0);
	pal_b->align(FL_ALIGN_LEFT);
	pal_b->callback(update_palette, (void*)2);
}
void palette_bar::updateSize(void){
	offx=(double)((double)window->w()/800.0)*(double)offxx;
	offy=(double)((double)window->h()/600.0)*(double)offyy;
}
void palette_bar::check_box(int16_t x,int16_t y){
	/*!
	This function is in charge of seeing if the mouse click is on a box and what box it is
	for x and y pass the mouser cordinace
	*/
	uint8_t boxSize=window->pal_size->value();
	x-=offx;
	y-=offy;
	if (x < 0)
		return;
	if (y < 0)
		return;
	x/=boxSize;
	if (x >= perRow)
		return;
	y/=boxSize;
	if (y >= rows)
		return;
	box_sel=x;
	if (rows != 1)
		changeRow(y);
	else
		updateSlider();
	window->redraw();
	
}
void palette_bar::draw_boxes(){
	uint8_t box_size=window->pal_size->value();
	uint8_t x,y,a;
	a=perRow*3;
	if (rows!=1){
		uint16_t loc_x,loc_y;
		loc_x=(double)((double)window->w()/800.0)*(double)palette_preview_box_x;
		loc_y=(double)((double)window->h()/600.0)*(double)palette_preview_box_y;
		fl_rectf(loc_x,loc_y,box_size*4,box_size*4,currentProject->rgbPal[(box_sel*3)+(theRow*a)],currentProject->rgbPal[(box_sel*3)+(theRow*a)+1],currentProject->rgbPal[(box_sel*3)+(theRow*a)+2]);//this will show larger preview of current color
	}
	if (theRow >= rows){
		for (x=0;x<perRow;x++)
			fl_rectf(offx+(x*box_size),offy,box_size,box_size,currentProject->rgbPal[(x*3)+(a*theRow)],currentProject->rgbPal[(x*3)+1+(a*theRow)],currentProject->rgbPal[(x*3)+2+(a*theRow)]);
		fl_draw_box(FL_EMBOSSED_FRAME,box_sel*box_size+offx,offy,box_size,box_size,0);
	}else{
		for (y=0;y<rows;y++){
			for (x=0;x<perRow;x++)
				fl_rectf(offx+(x*box_size),offy+(y*box_size),box_size,box_size,currentProject->rgbPal[(x*3)+(y*a)],currentProject->rgbPal[(x*3)+(y*a)+1],currentProject->rgbPal[(x*3)+(y*a)+2]);
		}
		fl_draw_box(FL_EMBOSSED_FRAME,box_sel*box_size+offx,theRow*box_size+offy,box_size,box_size,0);
	}
	
}
void palette_bar::changeRow(uint8_t r){
	theRow=r;
	updateSlider();
	
}
void palette_bar::updateSlider(){
	if (currentProject->palType[box_sel+(theRow*perRow)]){
		pal_b->hide();
		pal_g->hide();
		pal_r->hide();
	}else{
		pal_b->show();
		pal_g->show();
		pal_r->show();
		switch (currentProject->gameSystem){
			case sega_genesis:
				pal_b->value(currentProject->palDat[(box_sel*2)+(theRow*32)]);
				pal_g->value(currentProject->palDat[1+(box_sel*2)+(theRow*32)]>>4);
				pal_r->value(currentProject->palDat[1+(box_sel*2)+(theRow*32)]&15);		
			break;
			case NES:
				pal_r->value(currentProject->palDat[box_sel+(theRow*4)]&15);
				pal_g->value((currentProject->palDat[box_sel+(theRow*4)]>>4)&3);
			break;
		}
	}
	window->palType[currentProject->palType[box_sel+(theRow*perRow)]]->setonly();
	window->palType[currentProject->palType[box_sel+(theRow*perRow)]+3]->setonly();
	window->palType[currentProject->palType[box_sel+(theRow*perRow)]+6]->setonly();
}
void palette_bar::changeSystem(){
	switch (currentProject->gameSystem){
		case sega_genesis:
			perRow=16;
			pal_r->label("Red");
			pal_g->label("Green");
			pal_b->label("Blue");
			pal_r->step(2);
			pal_g->step(2);
			pal_b->step(2);
			pal_r->maximum(14);
			pal_g->maximum(14);
			pal_b->maximum(14);
			pal_b->callback(update_palette, (void*)2);
			updateSlider();
		break;
		case NES:
			perRow=4;
			if (box_sel > 3)
				box_sel=3;//box_sel starts at zero yes there are 4 colors per row
			pal_r->label("Hue");
			pal_g->label("Value");
			pal_b->label("Emphasis");
			pal_r->maximum(15);
			pal_g->maximum(3);
			pal_b->maximum(7);
			pal_b->value(0);
			pal_r->step(1);
			pal_g->step(1);
			pal_b->step(1);
			pal_b->callback(update_emphesis);
			updateSlider();
		break;
	}
}
