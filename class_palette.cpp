#include "global.h"
#include "callbacks_palette.h"
#include "class_palette.h"
#include "color_convert.h"
palette_bar palEdit;
palette_bar tileEdit_pal;
palette_bar tileMap_pal;
void palette_bar::more_init(uint8_t x,uint16_t offsetx,uint16_t offsety)
{
	rows=x;
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
void palette_bar::check_box(int16_t x,int16_t y)
{
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
	uint8_t z;
	switch (game_system)
	{
		case sega_genesis:
			z=16;
		break;
		case NES:
			z=4;
		break;
	}
	x/=boxSize;
	if (x >= z)
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
void palette_bar::draw_boxes()
{
	uint8_t box_size=window->pal_size->value();
	uint8_t x,y,z,a;
	switch (game_system)
	{
		case sega_genesis:
			z=16;
			a=48;
		break;
		case NES:
			z=4;
			a=12;
		break;
	}
	if (rows!=1)
	{
		uint16_t loc_x,loc_y;
		loc_x=(double)((double)window->w()/800.0)*(double)palette_preview_box_x;
		loc_y=(double)((double)window->h()/600.0)*(double)palette_preview_box_y;
		fl_rectf(loc_x,loc_y,box_size*4,box_size*4,rgb_pal[(box_sel*3)+(theRow*a)],rgb_pal[(box_sel*3)+(theRow*a)+1],rgb_pal[(box_sel*3)+(theRow*a)+2]);//this will show larger preview of current color
	}
	if (theRow >= rows)
	{
		for (x=0;x<z;x++)
			fl_rectf(offx+(x*box_size),offy,box_size,box_size,rgb_pal[(x*3)+(a*theRow)],rgb_pal[(x*3)+1+(a*theRow)],rgb_pal[(x*3)+2+(a*theRow)]);
		fl_draw_box(FL_EMBOSSED_FRAME,box_sel*box_size+offx,offy,box_size,box_size,0);
	}
	else
	{
		for (y=0;y<rows;y++)
		{
			for (x=0;x<z;x++)
				fl_rectf(offx+(x*box_size),offy+(y*box_size),box_size,box_size,rgb_pal[(x*3)+(y*a)],rgb_pal[(x*3)+(y*a)+1],rgb_pal[(x*3)+(y*a)+2]);
		}
		fl_draw_box(FL_EMBOSSED_FRAME,box_sel*box_size+offx,theRow*box_size+offy,box_size,box_size,0);
	}
	
}
void palette_bar::changeRow(uint8_t r)
{
	theRow=r;
	updateSlider();
	
}
void palette_bar::updateSlider()
{
	switch (game_system)
	{
		case sega_genesis:
			pal_b->value(palette[(box_sel*2)+(theRow*32)]);
			pal_g->value(palette[1+(box_sel*2)+(theRow*32)]>>4);
			pal_r->value(palette[1+(box_sel*2)+(theRow*32)]&15);
		break;
		case NES:
			pal_r->value(palette[box_sel+(theRow*4)]&15);
			pal_g->value((palette[box_sel+(theRow*4)]>>4)&3);
		break;
	}
}
void palette_bar::changeSystem()
{
	switch (game_system)
	{
		case sega_genesis:
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
			update_emphesis(0,0);
		break;
	}
}
