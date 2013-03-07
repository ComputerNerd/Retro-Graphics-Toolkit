//and of cource we "get" to define the functions twice once in the header and then include the actual code in the cpp file
#if _WIN32
#include <Windows.h>
#endif
#include <FL/Fl.H>
#pragma once
void save_palette(Fl_Widget*, void* start_end);
void update_palette(Fl_Widget* o, void* v);
void Butt_CB(Fl_Widget*, void* offset);
//void set_palette_type(Fl_Widget*, void* type); Moved to global
