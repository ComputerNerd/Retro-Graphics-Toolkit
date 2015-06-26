#pragma once
#include <stdint.h>
#include "project.h"
void setGameSysTMS9918(Project*prj);
void setSubditherSetting(Fl_Widget*w, void*);
void redrawOnlyCB(Fl_Widget*, void*);
void set_mode_tabs(Fl_Widget* o, void*);
void showAbout(Fl_Widget*,void*);
void set_game_system(Fl_Widget*,void* selection);
void trueColTileToggle(Fl_Widget*,void*);
void toggleRowSolo(Fl_Widget*,void*);
