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
