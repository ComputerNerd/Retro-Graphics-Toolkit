#include "global.h"
#include "class_global.h"
editor *window = new editor(800,600,"Retro Graphics Toolkit");//this creates the window
void rect_alpha_grid(uint8_t rgba[4],uint16_t x,uint16_t y)
{
	//the formula for alpha blending is simple
	uint8_t grid[32*32*3];
	//first generate grid
	uint8_t * ptr_grid=grid;
	for (uint8_t e=0;e<16;e++)
	{
		for (uint8_t c=0;c<16*3;c++)
			*ptr_grid++=255;
		for (uint8_t c=0;c<16*3;c++)
			*ptr_grid++=160;
	}
	for (uint8_t e=0;e<16;e++)
	{
		for (uint8_t c=0;c<16*3;c++)
			*ptr_grid++=160;
		for (uint8_t c=0;c<16*3;c++)
			*ptr_grid++=255;
	}
	if (rgba[3]==0)
	{
		//just draw grid and return
		fl_draw_image(grid,x,y,32,32,3);
		return;
	}
	ptr_grid=grid;
	double percent=rgba[3]/255.0;
	for (uint16_t c=0;c<32*32;c++)
	{
		for (uint8_t e=0;e<3;e++)
			*ptr_grid++=((double)rgba[e]*percent)+((double)*ptr_grid*(1.0-percent));
			
	}
	fl_draw_image(grid,x,y,32,32,3);
	
}
void editor::draw_non_gui()
{
	//When resizing the window things move around so we need to compensate for that
	unsigned char per_row;
	unsigned char per_row_rgb;
	switch (game_system)
	{
		case sega_genesis:
			per_row=16;
			per_row_rgb=48;
		break;
		case NES:
			per_row=4;
			per_row_rgb=12;
		break;
		default:
			show_default_error
		break;
	}
	//palette_bar_offset_y=(double)((double)h()/600.0)*(double)default_palette_bar_offset_y;//this command needs to be done for more than one window so there is no point of writting this more than once so we put it here
	

	short x,y;//we will need to reuse these later
	unsigned char box_size=pal_size->value();
	unsigned char tiles_size=tile_size->value();
	unsigned char placer_tile_size=place_tile_size->value();
	switch (mode_editor)
	{
		case pal_edit:
			palEdit.draw_boxes();
		break;
		case tile_edit:
			//draw truecolor preview box
			true_color_box_y=(double)((double)h()/600.0)*(double)default_true_color_box_y;
			true_color_box_x=(double)((double)w()/800.0)*(double)default_true_color_box_x;
			//fl_rectf(true_color_box_x,true_color_box_y,true_color_box_size,true_color_box_size,truecolor_temp[0],truecolor_temp[1],truecolor_temp[2]);
			rect_alpha_grid(truecolor_temp,true_color_box_x,true_color_box_y);
			tile_edit_truecolor_off_x=(double)((double)w()/800.0)*(double)default_tile_edit_truecolor_off_x;
			tile_edit_truecolor_off_y=(double)((double)h()/600.0)*(double)default_tile_edit_truecolor_off_y;
			tile_edit_offset_y=(double)((double)h()/600.0)*(double)default_tile_edit_offset_y;
			tile_edit_offset_x=(tiles_size*9)+tile_edit_truecolor_off_x;//I muliplyed it by 9 instead of 8 to give spacing between the tiles

			tiles_main.draw_truecolor(tiles_main.current_tile,tile_edit_truecolor_off_x,tile_edit_truecolor_off_y,false,false,tiles_size);
			//draw palette selection box
			tileEdit_pal.draw_boxes();
			tiles_main.draw_tile(tile_edit_offset_x,tile_edit_offset_y,tiles_main.current_tile,tiles_size,tileEdit_pal.theRow,false,false);
			if (show_grid == true)
			{
				//draw the grid
				if (tiles_size > 3)
				{
					for (y=0;y<8;y++)
					{
						for (x=0;x<8;x++)
						{
							fl_draw_box(FL_EMBOSSED_FRAME,(x*tiles_size)+tile_edit_offset_x,(y*tiles_size)+tile_edit_offset_y,tiles_size,tiles_size,NULL);
						}
					}
					for (y=0;y<8;y++)
					{
						for (x=0;x<8;x++)
						{
							fl_draw_box(FL_EMBOSSED_FRAME,(x*tiles_size)+tile_edit_truecolor_off_x,(y*tiles_size)+tile_edit_truecolor_off_y,tiles_size,tiles_size,NULL);
						}
					}
				}
			}
		break;
		case tile_place:
			tile_placer_tile_offset_y=(double)((double)h()/600.0)*(double)default_tile_placer_tile_offset_y;
			tileMap_pal.draw_boxes();
			//now draw the tile
			tiles_main.draw_tile(tile_placer_tile_offset_x,tile_placer_tile_offset_y,tiles_main.current_tile,placer_tile_size,tileMap_pal.theRow,G_hflip,G_vflip);
			//tiles_main.draw_truecolor(tiles_main.current_tile,tile_placer_tile_offset_x,tile_placer_tile_offset_y,G_hflip,G_vflip,placer_tile_size);
			//convert posistion
			map_off_y=(double)((double)h()/600.0)*(double)default_map_off_y;
			map_off_x=(double)((double)w()/800.0)*(double)default_map_off_x;
			//draw tile map
			unsigned char max_map_w,max_map_h;//used to calulate the displayable tiles
			max_map_w=((placer_tile_size*8)+w()-map_off_x)/(placer_tile_size*8);//this will puroposly allow one tile to go partly off screen that is normal I added that on purpose
			max_map_h=((placer_tile_size*8)+h()-map_off_y)/(placer_tile_size*8);
			//see if shadow highlight is enabled
			if (palette_muliplier==18 || game_system != sega_genesis)
			{
				//shadow highlight is disabled
				for (y=0;y<min((int)map_size_y-map_scroll_pos_y,(int)max_map_h);y++)
				{
					for (x=0;x<min((int)map_size_x-map_scroll_pos_x,(int)max_map_w);x++)
					{//remember to change back to drawtile
						uint32_t tempx,tempy;
						tempx=x+map_scroll_pos_x;
						tempy=y+map_scroll_pos_y;
						tiles_main.draw_tile(map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),get_tile(x+map_scroll_pos_x,y+map_scroll_pos_y),placer_tile_size,get_palette_map(x+map_scroll_pos_x,y+map_scroll_pos_y),get_hflip(x+map_scroll_pos_x,y+map_scroll_pos_y),get_vflip(x+map_scroll_pos_x,y+map_scroll_pos_y));
						//tiles_main.draw_truecolor(get_tile(tempx,tempy),map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),get_hflip(tempx,tempy),get_vflip(tempx,tempy),placer_tile_size);
					}
				}
			}
			else
			{
				unsigned char type_temp;
				if (palette_adder==0)
				{
					type_temp=1;
				}
				else
				{
					type_temp=2;
				}
				for (y=0;y<min((int)map_size_y-map_scroll_pos_y,(int)max_map_h);y++)
				{
					for (x=0;x<min((int)map_size_x-map_scroll_pos_x,(int)max_map_w);x++)
					{
						bool temp=get_prio(x+map_scroll_pos_x,y+map_scroll_pos_y)^true;
						set_palette_type(temp);
						tiles_main.draw_tile(map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),get_tile(x+map_scroll_pos_x,y+map_scroll_pos_y),placer_tile_size,get_palette_map(x+map_scroll_pos_x,y+map_scroll_pos_y),get_hflip(x+map_scroll_pos_x,y+map_scroll_pos_y),get_vflip(x+map_scroll_pos_x,y+map_scroll_pos_y));
					}
				}
				set_palette_type(type_temp);

			}
			if (show_grid_placer == true)
			{
				//draw box over tiles
				for (y=0;y<min((int)map_size_y-map_scroll_pos_y,(int)max_map_h);y++)
				{
					for (x=0;x<min((int)map_size_x-map_scroll_pos_x,(int)max_map_w);x++)
					{
						fl_draw_box(FL_EMBOSSED_FRAME,map_off_x+((x*8)*placer_tile_size),map_off_y+((y*8)*placer_tile_size),placer_tile_size*8,placer_tile_size*8,NULL);
					}
				}
				
			}
		break;
	}//end of switch statment
}

void editor::draw()
{
	//cout << "drawing" << endl;
	if (damage() == FL_DAMAGE_USER1)
	{
		draw_non_gui();
		return;
	}
	//menu->redraw();
	//the_tabs->redraw();
	//draw_children();
	Fl_Group::draw();
	draw_non_gui();
}

// Create a window at the specified position
editor::editor(int X, int Y, int W, int H, const char *L)
    : Fl_Double_Window(X, Y, W, H, L) {
	_editor();
}


// Create a block window
editor::editor(int W, int H, const char *L)
    : Fl_Double_Window(W, H, L) {
	_editor();
}
int editor::handle(int event)
{

	//printf("Event was %s (%d)\n", fl_eventnames[event], event);     // e.g. "Event was FL_PUSH (1)"
	if (Fl_Double_Window::handle(event)) return (1);
	//printf("Event was %s (%d)\n", fl_eventnames[event], event);     // e.g. "Event was FL_PUSH (1)"
	unsigned char tiles_size;
	switch(event)
	{
		case FL_PUSH:
			switch (mode_editor)
			{
				case pal_edit:
					palEdit.check_box(Fl::event_x(),Fl::event_y());
				break;
				case tile_edit:
					tileEdit_pal.check_box(Fl::event_x(),Fl::event_y());
				break;
				case tile_place:
					tileMap_pal.check_box(Fl::event_x(),Fl::event_y());
				break;
			}
			switch (mode_editor)
			{
				case tile_place:
					tiles_size=place_tile_size->value();
					//see if the user placed a tile on the map
					if (Fl::event_x() > map_off_x && Fl::event_y() > map_off_y && Fl::event_x() < map_off_x+((tiles_size*8)*map_size_x) && Fl::event_y() < map_off_y+((tiles_size*8)*map_size_y))
					{
						uint16_t temp_two,temp_one;
						temp_one=((Fl::event_x()-map_off_x)/tiles_size)/8;
						temp_two=((Fl::event_y()-map_off_y)/tiles_size)/8;
						temp_one+=+map_scroll_pos_x;
						temp_two+=+map_scroll_pos_y;
						if (Fl::event_button() == FL_LEFT_MOUSE)
						{
							
							//printf("Setting tile: %d to location x: %d y: %d\n",tiles_main.current_tile,temp_one+map_scroll_pos_x,temp_two+map_scroll_pos_y);
							set_tile_full(tiles_main.current_tile,temp_one,temp_two,tileMap_pal.theRow,G_hflip,G_vflip,G_highlow_p);
							//redraw();
							damage(FL_DAMAGE_USER1);
						}
						else
						{
							fl_alert("Tile attributes id: %d h-flip: %d v-flip %d priority: %d pal row: %d\nAt location x: %d y: %d",get_tile(temp_one,temp_two),get_hflip(temp_one,temp_two),get_vflip(temp_one,temp_two),get_prio(temp_one,temp_two),get_palette_map(temp_one,temp_two),temp_one,temp_two);
						}
					}

					if (Fl::event_x() > tile_placer_tile_offset_x && Fl::event_y() > tile_placer_tile_offset_y && Fl::event_x() < tile_placer_tile_offset_x+(tiles_size*8) && Fl::event_y() < tile_placer_tile_offset_y+(tiles_size*8))
					{
						
						unsigned char temp_two,temp_one;
						temp_one=(Fl::event_x()-tile_placer_tile_offset_x)/tiles_size;
						temp_two=(Fl::event_y()-tile_placer_tile_offset_y)/tiles_size;
						//temp_one+=temp_two;
						if (G_hflip == true)
						{
							temp_one=7-temp_one;
						}
						if (G_vflip == true)
						{
							temp_two=7-temp_two;
						}
						//now we know which pixel we are editing
						//see if it is even or odd
						unsigned char temp_1,temp_2;
						//if ((temp_one/2)*2 == temp_one)
						if (temp_one & 1)//faster
						{
							//odd
							//split pixels
							unsigned char temp=tiles_main.tileDat[(tiles_main.current_tile*32)+(temp_one/2)+(temp_two*4)];
							//first,second pixel
							temp_1=temp>>4;//first pixel
							temp_2=temp&15;//second pixel
							//put temp_1 back in proper place
							temp_1<<=4;
							temp_1+=tileMap_pal.box_sel;
							tiles_main.tileDat[(tiles_main.current_tile*32)+(temp_one/2)+(temp_two*4)]=temp_1;
						}
						else
						{
							//even
							//split pixels
							unsigned char temp=tiles_main.tileDat[(tiles_main.current_tile*32)+(temp_one/2)+(temp_two*4)];
							//first,second pixel
							temp_1=temp>>4;//first pixel
							temp_2=temp&15;//second pixel
							temp_2+=tileMap_pal.box_sel<<4;
							tiles_main.tileDat[(tiles_main.current_tile*32)+(temp_one/2)+(temp_two*4)]=temp_2;
						}
						//redraw();
						damage(FL_DAMAGE_USER1);//no need to redraw the gui
					}
				break;

				case tile_edit:
					//first see if we are in a "valid" range
					tiles_size=tile_size->value();
					//start by handiling true color
					if (Fl::event_x() > tile_edit_truecolor_off_x && Fl::event_y() > tile_edit_truecolor_off_y && Fl::event_x() < tile_edit_truecolor_off_x+(tiles_size*8)  && Fl::event_y() < tile_edit_truecolor_off_y+(tiles_size*8))
					{
						//if all conditions have been met that means we are able to edit the truecolor tile
						uint8_t temp_two,temp_one;//geting the mouse posision is the same as with tile editing just different varibles that happens alot in c++ the same thing just slightly different
						temp_one=(Fl::event_x()-tile_edit_truecolor_off_x)/tiles_size;
						temp_two=(Fl::event_y()-tile_edit_truecolor_off_y)/tiles_size;
						//true color tiles are slightly easier to edit
						//I now have a proper function to calulate the offset so I am using that
						tiles_main.truetileDat[cal_offset_truecolor(temp_one,temp_two,0,tiles_main.current_tile)]=truecolor_temp[0];//red
						tiles_main.truetileDat[cal_offset_truecolor(temp_one,temp_two,1,tiles_main.current_tile)]=truecolor_temp[1];//green
						tiles_main.truetileDat[cal_offset_truecolor(temp_one,temp_two,2,tiles_main.current_tile)]=truecolor_temp[2];//blue
						tiles_main.truetileDat[cal_offset_truecolor(temp_one,temp_two,3,tiles_main.current_tile)]=truecolor_temp[3];//alpha
						tiles_main.truecolor_to_tile(tileEdit_pal.theRow,tiles_main.current_tile);
						//redraw();
						damage(FL_DAMAGE_USER1);
					}

					if (Fl::event_x() > tile_edit_offset_x && Fl::event_y() > tile_edit_offset_y && Fl::event_x() < tile_edit_offset_x+(tiles_size*8) && Fl::event_y() < tile_edit_offset_y+(tiles_size*8))
					{
						
						uint8_t temp_two,temp_one;
						temp_one=(Fl::event_x()-tile_edit_offset_x)/tiles_size;
						temp_two=(Fl::event_y()-tile_edit_offset_y)/tiles_size;
						uint8_t get_pal=(tileEdit_pal.theRow*48)+(tileEdit_pal.box_sel*3);
						tiles_main.truetileDat[cal_offset_truecolor(temp_one,temp_two,0,tiles_main.current_tile)]=rgb_pal[get_pal];//red
						tiles_main.truetileDat[cal_offset_truecolor(temp_one,temp_two,1,tiles_main.current_tile)]=rgb_pal[get_pal+1];//green
						tiles_main.truetileDat[cal_offset_truecolor(temp_one,temp_two,2,tiles_main.current_tile)]=rgb_pal[get_pal+2];//blue
						tiles_main.truecolor_to_tile(tileEdit_pal.theRow,tiles_main.current_tile);
						//redraw();
						damage(FL_DAMAGE_USER1);
					}
				break;
			}
		break;
	}
	return 0;
}
