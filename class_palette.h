/*I have noticed that in the palette bar I am doing a lot of copy paste programming this is very bad so I am refactoring*/
#pragma once
#include "global.h"
class palette_bar
{
	uint8_t rows;//tells how many rows there are in the palette
	uint16_t offx,offy;//the offset in which the palette selection boxes will appear
	//uint8_t rgb_temp[3]; no need for this
public:
	Fl_Slider* pal_r;
	Fl_Slider* pal_g;
	Fl_Slider* pal_b;
	uint8_t box_sel;//tells what palette entry is selected
	uint8_t theRow;//tells what row in the palette is selected
	//palette_bar(uint16_t,uint16_t);
	//palette_bar(uint16_t,uint16_t,uint8_t);
	void check_box(int16_t,int16_t);
	void draw_boxes();
	void more_init(uint8_t x=1,uint16_t offsetx=16,uint16_t offsety=56);//this one should be called in a function that creates the gui elements
	void changeRow(uint8_t);
	void changeSystem();
	void updateSlider();
}
extern palEdit,tileEdit_pal,tileMap_pal;
