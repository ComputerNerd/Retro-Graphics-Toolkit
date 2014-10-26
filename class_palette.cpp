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
#include "global.h"
#include "callbacks_palette.h"
#include "class_palette.h"
#include "color_convert.h"
#include "errorMsg.h"
palette_bar palEdit;
palette_bar tileEdit_pal;
palette_bar tileMap_pal;
palette_bar spritePal;
unsigned palette_bar::getEntry(void){
	return box_sel+(theRow*perRow);
}
void palette_bar::setSysColCnt(void){
	switch (currentProject->gameSystem){
		case sega_genesis:
			perRow=16;
			currentProject->colorCnt=64;
			currentProject->colorCntalt=currentProject->rowCntPalalt=0;
			currentProject->rowCntPal=4;
			currentProject->haveAltspritePal=false;
		break;
		case NES:
			perRow=4;
			currentProject->colorCnt=currentProject->colorCntalt=16;
			currentProject->rowCntPal=currentProject->rowCntPalalt=4;
			currentProject->haveAltspritePal=true;
		break;
		default:
			show_default_error
	}
}
void palette_bar::more_init(uint8_t x,uint16_t offsetx,uint16_t offsety,bool altset,unsigned ln,bool tiny){
	alt=altset;
	int sz;
	if(tiny)
		sz=22;
	else
		sz=24;
	sysCache=currentProject->gameSystem;
	setSysColCnt();
	rows=x;
	offxx=offsetx;
	offyy=offsety;
	offx=offsetx;
	offy=offsety;
	if(tiny)
		offsety+=6;
	else
		offsety+=8;
	pal_r = new Fl_Hor_Value_Slider(offx+32,offsety+(rows*32),ln,sz,"Red");
	pal_r->minimum(0); pal_r->maximum(0x0E);
	pal_r->step(2);
	pal_r->value(0);
	pal_r->align(FL_ALIGN_LEFT);
	pal_r->callback(update_palette, (void*)0);

	if(tiny)
		offsety+=26;
	else
		offsety+=32;

	pal_g = new Fl_Hor_Value_Slider(offx+32,offsety+(rows*32),ln,sz,"Green");
	pal_g->minimum(0); pal_g->maximum(0x0E);
	pal_g->step(2);
	pal_g->value(0);
	pal_g->align(FL_ALIGN_LEFT);
	pal_g->callback(update_palette, (void*)1);
	pal_g->labelsize(13);

	if(tiny)
		offsety+=26;
	else
		offsety+=32;

	pal_b = new Fl_Hor_Value_Slider(offx+32,offsety+(rows*32),ln,sz,"Blue");
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
void palette_bar::check_box(int x,int y){
	/*!
	This function is in charge of seeing if the mouse click is on a box and what box it is
	for x and y pass the mouser cordinace
	*/
	unsigned boxSize=window->pal_size->value();
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
void palette_bar::draw_boxes(void){
	unsigned box_size=window->pal_size->value();
	unsigned x,y,a;
	a=perRow*3;
	if (rows!=1){
		uint16_t loc_x,loc_y;
		loc_x=(double)((double)window->w()/800.0)*(double)palette_preview_box_x;
		loc_y=(double)((double)window->h()/600.0)*(double)palette_preview_box_y;
		fl_rectf(loc_x,loc_y,box_size*4,box_size*4,currentProject->rgbPal[(box_sel*3)+(theRow*a)],currentProject->rgbPal[(box_sel*3)+(theRow*a)+1],currentProject->rgbPal[(box_sel*3)+(theRow*a)+2]);//this will show larger preview of current color
	}
	if (theRow >= rows){
		uint8_t*rgbPtr=currentProject->rgbPal+(a*theRow);
		if(alt&&(currentProject->gameSystem==NES))
			rgbPtr+=currentProject->colorCnt*3;
		for (x=0;x<perRow;++x){
			fl_rectf(offx+(x*box_size),offy,box_size,box_size,*rgbPtr,*(rgbPtr+1),*(rgbPtr+2));
			rgbPtr+=3;
		}
		fl_draw_box(FL_EMBOSSED_FRAME,box_sel*box_size+offx,offy,box_size,box_size,0);
	}else{
		uint8_t*rgbPtr=currentProject->rgbPal;
		if(alt&&(currentProject->gameSystem==NES))
			rgbPtr+=currentProject->colorCnt*3;
		for (y=0;y<rows;++y){
			for (x=0;x<perRow;++x){
				fl_rectf(offx+(x*box_size),offy+(y*box_size),box_size,box_size,*rgbPtr,*(rgbPtr+1),*(rgbPtr+2));
				rgbPtr+=3;
			}
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
				if(alt){
					pal_r->value(currentProject->palDat[box_sel+(theRow*4)+16]&15);
					pal_g->value((currentProject->palDat[box_sel+(theRow*4)+16]>>4)&3);
				}else{
					pal_r->value(currentProject->palDat[box_sel+(theRow*4)]&15);
					pal_g->value((currentProject->palDat[box_sel+(theRow*4)]>>4)&3);
				}
			break;
			default:
				show_default_error
		}
	}
	window->palType[currentProject->palType[box_sel+(theRow*perRow)]]->setonly();
	window->palType[currentProject->palType[box_sel+(theRow*perRow)]+3]->setonly();
	window->palType[currentProject->palType[box_sel+(theRow*perRow)]+6]->setonly();
}
void palette_bar::changeSystem(){
	if(sysCache!=currentProject->gameSystem){
		setSysColCnt();
		switch (currentProject->gameSystem){
			case sega_genesis:
				pal_r->label("Red");
				pal_g->label("Green");
				pal_g->labelsize(13);
				pal_b->label("Blue");
				pal_b->labelsize(14);
				pal_b->resize(pal_b->x()-16,pal_b->y(),pal_b->w()+16,pal_b->h());
				pal_r->step(2);
				pal_g->step(2);
				pal_b->step(2);
				pal_r->maximum(14);
				pal_g->maximum(14);
				pal_b->maximum(14);
				pal_b->callback(update_palette, (void*)2);
			break;
			case NES:
				pal_r->label("Hue");
				pal_g->label("Value");
				pal_g->labelsize(14);
				pal_b->label("Emphasis");
				pal_b->labelsize(12);
				pal_b->resize(pal_b->x()+16,pal_b->y(),pal_b->w()-16,pal_b->h());
				pal_r->maximum(15);
				pal_g->maximum(3);
				pal_b->maximum(7);
				pal_b->value(0);
				pal_r->step(1);
				pal_g->step(1);
				pal_b->step(1);
				pal_b->callback(update_emphesis);
			break;
			default:
				show_default_error
		}
	}
	updateSlider();
	box_sel%=perRow;
	sysCache=currentProject->gameSystem;
}
