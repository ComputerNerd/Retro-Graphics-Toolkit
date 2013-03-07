#include "global.h"
#include "class_global.h"
#include "tilemap.h"
void delete_tile_at_location(Fl_Widget*, void* row)
{
	//this function will delete the tile that the user has selected
	//remeber both current_tile and tiles_amount are counting from zero that means that a value of zero means one tile
	//in this case we will still need a temp buffer even after switching to malloc because it can not delete part of it
	tiles_main.remove_tile_at(tiles_main.current_tile);
	window->redraw();
}
void new_tile(Fl_Widget*,void*)
{
	tiles_main.tiles_amount++;
	tiles_main.tileDat=(unsigned char *)realloc(tiles_main.tileDat,(tiles_main.tiles_amount+1)*tiles_main.tileSize);
	if (tiles_main.tileDat == 0)
	{
		show_realloc_error((tiles_main.tiles_amount+1)*tiles_main.tileSize);
		exit(1);
	}
	tiles_main.truetileDat=(uint8_t *)realloc(tiles_main.truetileDat,(tiles_main.tiles_amount+1)*256);
	if (tiles_main.truetileDat == 0)
	{
		show_realloc_error((tiles_main.tiles_amount+1)*256)
		exit(1);
	}
	/*uint32_t x;
	for (x=tiles_main.tiles_amount*tiles_main.tileSize;x<(tiles_main.tiles_amount+1)*tiles_main.tileSize;x++)
	{
		tiles_main.tileDat[x]=0;
	}*/
	memset(&tiles_main.tileDat[tiles_main.tiles_amount*tiles_main.tileSize],0,tiles_main.tileSize);
	/*for (x=tiles_main.tiles_amount*192;x<(tiles_main.tiles_amount+1)*192;x++)
	{
		tiles_main.truetileDat[x]=0;
	}*/
	memset(&tiles_main.truetileDat[tiles_main.tiles_amount*256],0,256);
	
	//set the new maximum for slider
	window->tile_select->maximum(tiles_main.tiles_amount);
	window->tile_select_2->maximum(tiles_main.tiles_amount);
	//redraw so the user knows that there is another tile
	window->redraw();
}
