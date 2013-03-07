#pragma once
class editor : public Fl_Double_Window
{
	private:
	Fl_Menu_Bar *menu;
	void _editor();
	void draw_non_gui();
	public:
	unsigned char mouse_x,mouse_y;
	//Fl_Tabs* o;
	/*Fl_Slider* s_r;
	Fl_Slider* s_g;
	Fl_Slider* s_b;*/
	//used s_red,s_green,s_blue are used for tile editor s_r,s_g,s_b are used for palette editor
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
	void draw();
	editor(int X, int Y, int W, int H, const char *L = 0);
	editor(int W, int H, const char *L = 0);
	int handle(int);
};
extern editor *window; //= new editor(800,600,"Sega Genesis Toolkit");
