#if _WIN32
#include <windows.h>
#endif
#include <FL/Fl.H>
#pragma once
void save_palette(Fl_Widget*, void* start_end);
void update_palette(Fl_Widget* o, void* v);
void loadPalette(Fl_Widget*, void* offset);
