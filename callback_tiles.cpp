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
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#include "global.h"
#include "class_global.h"
#include "tilemap.h"
void delete_tile_at_location(Fl_Widget*, void* row){
	/* this function will delete the tile that the user has selected
	   remeber both current_tile and tiles_amount are counting from zero that means that a value of zero means one tile */
	currentProject->tileC->remove_tile_at(currentProject->tileC->current_tile);
	window->redraw();
}
void new_tile(Fl_Widget*,void*){
	currentProject->tileC->tiles_amount++;
	currentProject->tileC->tileDat=(uint8_t *)realloc(currentProject->tileC->tileDat,(currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize);
	if (currentProject->tileC->tileDat == 0){
		show_realloc_error((currentProject->tileC->tiles_amount+1)*currentProject->tileC->tileSize);
		exit(1);
	}
	currentProject->tileC->truetileDat=(uint8_t *)realloc(currentProject->tileC->truetileDat,(currentProject->tileC->tiles_amount+1)*256);
	if (currentProject->tileC->truetileDat == 0){
		show_realloc_error((currentProject->tileC->tiles_amount+1)*256)
		exit(1);
	}
	memset(&currentProject->tileC->tileDat[currentProject->tileC->tiles_amount*currentProject->tileC->tileSize],0,currentProject->tileC->tileSize);
	memset(&currentProject->tileC->truetileDat[currentProject->tileC->tiles_amount*256],0,256);
	//set the new maximum for slider
	window->tile_select->maximum(currentProject->tileC->tiles_amount);
	window->tile_select_2->maximum(currentProject->tileC->tiles_amount);
	//redraw so the user knows that there is another tile
	window->redraw();
}
