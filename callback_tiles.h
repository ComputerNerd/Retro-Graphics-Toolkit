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
#include "includes.h"
void insertTileCB(Fl_Widget*,void*);
void delete_tile_at_location(Fl_Widget*, void* row);
void new_tile(Fl_Widget*,void*);
void update_truecolor(Fl_Widget* o,void* v);
void blank_tile(Fl_Widget*,void*);
void update_offset_tile_edit(Fl_Widget*,void*);
void set_tile_current(Fl_Widget* o,void*);
void set_tile_currentTP(Fl_Widget* o,void*);
void update_all_tiles(Fl_Widget*,void*);
void remove_duplicate_truecolor(Fl_Widget*,void*);
void remove_duplicate_tiles(Fl_Widget*,void*);
void fill_tile(Fl_Widget* o, void*);
