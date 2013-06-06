#pragma once
#include <inttypes.h>
class editor : public Fl_Double_Window
{
	private:
	Fl_Menu_Bar *menu;
	void _editor();
	void draw_non_gui();
	public:
	uint8_t mouse_x,mouse_y;
	Fl_Scrollbar * map_x_scroll;
	Fl_Scrollbar * map_y_scroll;
	Fl_Slider* map_w;
	Fl_Slider* map_h;
	Fl_Slider* rgb_red;
	Fl_Slider* rgb_green;
	Fl_Slider* rgb_blue;
	Fl_Slider* rgb_alpha;
	Fl_Slider * ditherPower;
	Fl_Slider* pal_size;
	Fl_Slider* tile_size;
	Fl_Slider * place_tile_size;
	Fl_Slider* tile_select;
	Fl_Slider* tile_select_2;
	Fl_Tabs* the_tabs;
	Fl_Round_Button * palType[9];
	void draw();
	editor(int X, int Y, int W, int H, const char *L = 0);
	editor(int W, int H, const char *L = 0);
	int handle(int);
};
extern editor *window;
