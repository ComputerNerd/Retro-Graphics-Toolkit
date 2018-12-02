/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2017)
   */
#if _WIN32
#include <windows.h>
#endif
#include <FL/Fl.H>
#pragma once
void sortRowbyCB(Fl_Widget*, void*);
void save_palette(Fl_Widget*, void* start_end);
void update_palette(Fl_Widget* o, void* v);
void loadPalette(Fl_Widget*, void* offset);
void set_ditherAlg(Fl_Widget*, void* typeset);
void set_tile_row(Fl_Widget*, void* row);
void setPalType(Fl_Widget*, void*type);
void pickNearAlg(Fl_Widget*, void*);
void rgb_pal_to_entry(Fl_Widget*, void*);
void entryToRgb(Fl_Widget*, void*);
void clearPalette(Fl_Widget*, void*);
