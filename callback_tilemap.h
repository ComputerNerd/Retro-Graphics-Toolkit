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
void setTmapOffsetCB(Fl_Widget*,void*);
void resizeBlocksCB(Fl_Widget*o,void*);
void blocksAmtCB(Fl_Widget*o,void*);
void toggleBlocksCB(Fl_Widget*o,void*);
void FixOutOfRangeCB(Fl_Widget*,void*);
void callback_resize_map(Fl_Widget* o,void*);
void set_grid(Fl_Widget*,void*);
void set_grid_placer(Fl_Widget*,void*);
void save_tilemap_as_image(Fl_Widget*,void*);
void save_tilemap_as_colspace(Fl_Widget*,void*);
void load_tile_map(Fl_Widget*,void*);
void save_map(Fl_Widget*,void*);
void fill_tile_map_with_tile(Fl_Widget*,void*);
void dither_tilemap_as_image(Fl_Widget*,void*);
void load_image_to_tilemap(Fl_Widget*,void*o);
void set_prioCB(Fl_Widget*,void*);
void set_hflipCB(Fl_Widget*,void*);
void set_vflipCB(Fl_Widget*,void*);
void update_map_scroll_x(Fl_Widget*,void*);
void update_map_scroll_y(Fl_Widget*,void*);
void update_map_size(Fl_Widget*,void*);
void tilemap_remove_callback(Fl_Widget*,void*);
void shadow_highligh_findout(Fl_Widget*,void*);
void tileDPicker(Fl_Widget*,void*);
