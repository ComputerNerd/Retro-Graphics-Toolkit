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
#include "savepng.h"
#include "dither.h"
void resizeBlocksCB(Fl_Widget*o,void*){
	currentProject->tileMapC->resizeBlocks(window->map_w->value(),window->map_h->value());
	window->redraw();
}
void blocksAmtCB(Fl_Widget*o,void*){
	Fl_Slider*s=(Fl_Slider*)o;
	currentProject->tileMapC->blockAmt(s->value());
	window->redraw();
}
void toggleBlocksCB(Fl_Widget*o,void*){
	Fl_Check_Button* b=(Fl_Check_Button*)o;
	currentProject->tileMapC->toggleBlocks(b->value()?true:false);
	window->redraw();
}
void FixOutOfRangeCB(Fl_Widget*,void*){
	//use current attributes
	for(int y=0;y<currentProject->tileMapC->mapSizeH;++y){
		for(int x=0;x<currentProject->tileMapC->mapSizeW;++x){
			if(currentProject->tileMapC->get_tile(x,y)>currentProject->tileC->tiles_amount)
				currentProject->tileMapC->set_tile_full(currentProject->tileC->current_tile,x,y,tileMap_pal.theRow,G_hflip,G_vflip,G_highlow_p);
		}
	}
	window->damage(FL_DAMAGE_USER1);
}
void callback_resize_map(Fl_Widget* o,void*){
	uint32_t w,h;
	w=window->map_w->value();
	h=window->map_h->value();
	currentProject->tileMapC->resize_tile_map(w,h);
	window->redraw();
}
void set_grid(Fl_Widget*,void*){
	/*this function will only be trigger when the check button is pressed
	so we just need to invert the bool using xor to avoid if statments*/
	show_grid^=true;
	window->redraw();//redraw to reflect the updated statues of the grid
}
void set_grid_placer(Fl_Widget*,void*){
	show_grid_placer^=true;
	window->redraw();//redraw to reflect the updated statues of the grid
}

void save_tilemap_as_image(Fl_Widget*,void*){
	if(load_file_generic("Save png as",true)==true){
		uint32_t w=currentProject->tileMapC->mapSizeW*8;
		uint32_t h=currentProject->tileMapC->mapSizeH*8;
		uint8_t * image=(uint8_t*)malloc(w*h*3);
		uint8_t * imageold=image;
		if(image==0)
			show_malloc_error(w*h*3)
		uint8_t temptile[192];
		uint32_t x,y;
		uint32_t w3=w*3;//do this once instead of thousands of times in the loop
		uint32_t w21=w*21;
		uint32_t w24_24=(w*24)-24;
		uint8_t * tempptr,yy;
		for(y=0;y<h;y+=8){
			for(x=0;x<w;x+=8){
				tileToTrueCol(currentProject->tileC->tileDat+(currentProject->tileMapC->get_tile(x/8,y/8)*currentProject->tileC->tileSize),temptile,currentProject->tileMapC->get_palette_map(x/8,y/8),false);
				tempptr=temptile;
				for(yy=0;yy<8;++yy){
					memcpy(image,tempptr,24);
					image+=w3;
					tempptr+=24;
				}
				image-=w24_24;
			}
			image+=w21;
		}
		savePNG(the_file.c_str(),w,h,(void*)imageold);
		free(imageold);
	}
}
void save_tilemap_as_colspace(Fl_Widget*,void*){
	if(load_file_generic("Save png as",true)==true){
		uint32_t w=currentProject->tileMapC->mapSizeW*8;
		uint32_t h=currentProject->tileMapC->mapSizeH*8;
		uint8_t * image=(uint8_t*)malloc(w*h*3);
		truecolor_to_image(image,-1,false);
		ditherImage(image,w,h,false,true);
		savePNG(the_file.c_str(),w,h,(void*)image);
		free(image);
	}
}
void load_tile_map(Fl_Widget*,void*){
	if(unlikely(!currentProject->tileMapC->loadFromFile()))
		fl_alert("Error: Cannot load file %s",the_file.c_str());
}
void save_map(Fl_Widget*,void*){
	if(unlikely(!currentProject->tileMapC->saveToFile()))
		fl_alert("Error: can not save file %s\nTry making sure that you have permission to save the file here",the_file.c_str());
}
void fill_tile_map_with_tile(Fl_Widget*,void*){
	if (mode_editor != tile_place){
		fl_alert("To prevent accidental modification to the tile map be in plane editing mode");
		return;
	}
	if(fl_ask("This will erase the entire tilemap and fill it with the currently selected tile\nAre you sure you want to do this?")){
		for (uint16_t y=0;y<currentProject->tileMapC->mapSizeH;++y) {
			for (uint16_t x=0;x<currentProject->tileMapC->mapSizeW;++x)
				currentProject->tileMapC->set_tile_full(currentProject->tileC->current_tile,x,y,tileMap_pal.theRow,G_hflip,G_vflip,G_highlow_p);
		}
		window->damage(FL_DAMAGE_USER1);
	}
}
void dither_tilemap_as_image(Fl_Widget*,void*){
	//normally this program dithers all tiles individully this is not always desirable
	//to fix this I created this function It convertes the tilemap to image and dithers all tiles
	//so first create ram for image
	uint8_t * image;
	uint32_t w,h;
	w=currentProject->tileMapC->mapSizeW*8;
	h=currentProject->tileMapC->mapSizeH*8;
	uint8_t method=fl_choice("How would you like this tilemap dithered?","Dither each palette row separately","Dither entire image at once","cancel");
	if(method==2)
		return;
	image = (uint8_t *)malloc(w*h*4);
	if (!image)
		show_malloc_error(w*h*4)
	if(method==1){
		truecolor_to_image(image,-1);
		ditherImage(image,w,h,true,true);
		ditherImage(image,w,h,true,false);
		truecolorimageToTiles(image,-1);
	}else{
		for (uint8_t rowz=0;rowz<4;rowz++){
			printf("Row %d\n",rowz);
			truecolor_to_image(image,rowz);
			ditherImage(image,w,h,true,true);
			ditherImage(image,w,h,true,false);
			//convert back to tiles
			truecolorimageToTiles(image,rowz);
		}
	}
	putchar('\n');
	puts("Done with image");
	window->damage(FL_DAMAGE_USER1);
	Fl::check();
	free(image);
	window->redraw();
}
void load_image_to_tilemap(Fl_Widget*,void*){
	Fl_Shared_Image * loaded_image;
	if (load_file_generic("Load image") == true){
		loaded_image=Fl_Shared_Image::get(the_file.c_str());
		if (loaded_image == 0){
			fl_alert("Error loading image");
			return;
		}
		uint8_t tilebit;
		switch(currentProject->gameSystem){
			case sega_genesis:
				tilebit=7;
			break;
			case NES:
				tilebit=15;
			break;
			default:
				show_default_error
			break;
		}
		uint32_t w,h;
		w=loaded_image->w();
		h=loaded_image->h();
		printf("image width: %d image height: %d\n",w,h);
		uint32_t w8,h8;
		uint32_t wt,ht;
		uint8_t wr,hr;
		wt=w&(~(uint32_t)tilebit);
		ht=h&(~(uint32_t)tilebit);
		wr=w&tilebit;
		hr=h&tilebit;
		w8=w/8;
		h8=h/8;
		if (wr!=0)
			w8++;
		if (hr!=0)
			h8++;
		if(currentProject->gameSystem==NES){
			if((wr-8)>0)
				w8++;
			if((hr-8)>0)
				h8++;
		}
		if ((wr != 0) && (hr != 0))
			fl_alert("Warning both width and height are not a multiple of 8");
		else if (wr != 0)
			fl_alert("Warning width is not a multiple of 8");
		else if (hr != 0)
			fl_alert("Warning height is not a multiple of 8");
		printf("w %d h %d wt %d ht %d wr %d hr %d w8 %d h8 %d\n",w,h,wt,ht,wr,hr,w8,h8);
		//start by copying the data
		uint8_t * img_ptr=(uint8_t *)loaded_image->data()[0];
		//printf("First Pixel Red: %d Green: %d Blue: %d\n",img_ptr[0],img_ptr[1],img_ptr[2]);
		//now we can convert to tiles
		if (unlikely(loaded_image->d() != 3 && loaded_image->d() != 4)){
			fl_alert("Please use color depth of 3 or 4\nYou Used %d",loaded_image->d());
			loaded_image->release();
			return;
		}else
			printf("Image depth %d\n",loaded_image->d());
		uint64_t truecolor_tile_ptr=0;
		currentProject->tileC->truetileDat = (uint8_t *)realloc(currentProject->tileC->truetileDat,w8*h8*256);
		currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,w8*h8*currentProject->tileC->tileSize);
		currentProject->tileC->tiles_amount=(w8*h8)-1;
		window->tile_select->maximum(currentProject->tileC->tiles_amount);
		window->tile_select_2->maximum(currentProject->tileC->tiles_amount);
		//uint8_t sizeTemp,sizeTemp2;
		uint64_t a;
		uint32_t b,y,x=0;
		uint8_t xx;
		switch (loaded_image->d()){
			case 3:
				for (a=0;a<(ht*wt*3)-wt*3;a+=w*3*8){//a tiles y
					for (b=0;b<wt*3;b+=24){//b tiles x
						for (y=0;y<wt*3*8;y+=w*3){//pixels y
							xx=0;
							for (x=0;x<32;x+=4){//pixels x
								memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x],&img_ptr[a+b+y+xx],3);
								currentProject->tileC->truetileDat[truecolor_tile_ptr+x+3]=255;//solid
								xx+=3;
							}
							truecolor_tile_ptr+=32;
						}
					}
					if (wr!=0){//handle borders
						b+=24;
						uint32_t yy=0;
						for (y=0;y<8;++y){
							xx=0;
							for (x=0;x<wr*4;x+=4){
								memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x],&img_ptr[a+b+yy+xx],3);
								currentProject->tileC->truetileDat[truecolor_tile_ptr+x+3]=255;//solid
								xx+=3;
							}
							memset(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x+4],0,32-xx-4);
							truecolor_tile_ptr+=32;
							yy+=w*3;
						}
					}
				}
			break;
			case 4:
				for (a=0;a<(ht*wt*4)-wt*4;a+=w*4*8){//a tiles y
					for (b=0;b<wt*4;b+=32){//b tiles x
						for (y=0;y<wt*4*8;y+=w*4){//pixels y
							memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr],&img_ptr[a+b+y],32);
							truecolor_tile_ptr+=32;
						}
					}
					if (wr!=0){//handle borders
						b+=32;
						uint32_t yy=wt*4*8;
						for (y=0;y<8;++y){
							memcpy(&currentProject->tileC->truetileDat[truecolor_tile_ptr],&img_ptr[a+b+y],wr*4);
							memset(&currentProject->tileC->truetileDat[truecolor_tile_ptr+x+4],0,32-(wr*4)-4);
							truecolor_tile_ptr+=32;
							yy+=w*3;
						}
					}
				}
			break;
		}
		loaded_image->release();
		currentProject->tileMapC->resize_tile_map(w8,h8);
		window->map_w->value(w8);
		window->map_h->value(h8);
		uint32_t tilecounter=0;
		for (y=0;y<h8;++y){
			for (x=0;x<w8;++x){
				currentProject->tileMapC->set_tile_full(tilecounter,x,y,0,false,false,false);
				tilecounter++;
			}
		}
		window->redraw();
	}
}
void set_prioCB(Fl_Widget*,void*){
	G_highlow_p^=true;
	if(tileEditModePlace_G)
		currentProject->tileMapC->set_prio(selTileE_G[0],selTileE_G[1],G_highlow_p);
	window->redraw();
}
void set_hflipCB(Fl_Widget*,void*){
	G_hflip^=true;
	if(tileEditModePlace_G)
		currentProject->tileMapC->set_hflip(selTileE_G[0],selTileE_G[1],G_hflip);
	window->redraw();
}
void set_vflipCB(Fl_Widget*,void*){
	G_vflip^=true;
	if(tileEditModePlace_G)
		currentProject->tileMapC->set_vflip(selTileE_G[0],selTileE_G[1],G_vflip);
	window->redraw();
}
void update_map_scroll_x(Fl_Widget*,void*){
	map_scroll_pos_x=window->map_x_scroll->value();
	window->redraw();
}
void update_map_scroll_y(Fl_Widget*,void*){
	map_scroll_pos_y=window->map_y_scroll->value();
	window->redraw();
}
void update_map_size(Fl_Widget*,void*){
	currentProject->tileMapC->ScrollUpdate();
	window->redraw();
}
void tilemap_remove_callback(Fl_Widget*,void*){
	char * str_ptr;
	str_ptr=(char *)fl_input("Enter Tile");
	if (!str_ptr)
		return;
	if (!verify_str_number_only(str_ptr))
		return;
	int32_t tile=atoi(str_ptr);
	if (tile < 0){
		fl_alert("You must enter a number greater than or equal to 0 however you entered %d\n",tile);
		return;
	}
	if(tile)
		currentProject->tileMapC->sub_tile_map(tile,tile-1,false,false);
	else
		currentProject->tileMapC->sub_tile_map(0,0,false,false);
	window->damage(FL_DAMAGE_USER1);
}
void shadow_highligh_findout(Fl_Widget*,void*){
	if (unlikely(currentProject->gameSystem != sega_genesis)){
		fl_alert("Only the Sega Genesis/Mega Drive supports shadow highligh mode\n");
		return;
	}
	uint8_t type=fl_choice("How will it be determined if the tile is shadowed or not?","Tile brightness","Delta",0);
	//this function will see if 3 or less pixels are above 125 and if so set prioity to low or set priority to high if bright tile
	uint16_t x,y;
	uint32_t xx;
	if (type==0){
		for (y=0;y<currentProject->tileMapC->mapSizeH;++y){
			for (x=0;x<currentProject->tileMapC->mapSizeW;++x){
				uint32_t cur_tile=currentProject->tileMapC->get_tile(x,y);
				uint8_t over=0;
				for (xx=cur_tile*256;xx<cur_tile*256+256;xx+=4){
					if ((currentProject->tileC->truetileDat[xx] > 130) || (currentProject->tileC->truetileDat[xx+1] > 130) || (currentProject->tileC->truetileDat[xx+2] > 130))
						over++;
				}
				if (over > 4)
					currentProject->tileMapC->set_prio(x,y,true);//normal
				else
					currentProject->tileMapC->set_prio(x,y,false);//shadowed
			}
		}
	}else{
		uint8_t temp[256];
		//uint8_t useHiL=palette_muliplier;
		uint8_t type_temp=palTypeGen;
		for (y=0;y<currentProject->tileMapC->mapSizeH;++y){
			for (x=0;x<currentProject->tileMapC->mapSizeW;++x){
				uint32_t cur_tile=currentProject->tileMapC->get_tile(x,y);
				uint32_t errorSh=0,errorNorm=0;
				uint8_t * ptrorgin=&currentProject->tileC->truetileDat[(cur_tile*256)];
				set_palette_type(0);//normal
				currentProject->tileC->truecolor_to_tile(currentProject->tileMapC->get_palette_map(x,y),cur_tile);
				tileToTrueCol(&currentProject->tileC->tileDat[(cur_tile*currentProject->tileC->tileSize)],temp,currentProject->tileMapC->get_palette_map(x,y));
				for (xx=0;xx<256;xx+=4){
					errorNorm+=abs(temp[xx]-ptrorgin[xx]);
					errorNorm+=abs(temp[xx+1]-ptrorgin[xx+1]);
					errorNorm+=abs(temp[xx+2]-ptrorgin[xx+2]);
				}
				set_palette_type(8);//shadow
				currentProject->tileC->truecolor_to_tile(currentProject->tileMapC->get_palette_map(x,y),cur_tile);
				tileToTrueCol(&currentProject->tileC->tileDat[(cur_tile*currentProject->tileC->tileSize)],temp,currentProject->tileMapC->get_palette_map(x,y));
				for (xx=0;xx<256;xx+=4){
					errorSh+=abs(temp[xx]-ptrorgin[xx]);
					errorSh+=abs(temp[xx+1]-ptrorgin[xx+1]);
					errorSh+=abs(temp[xx+2]-ptrorgin[xx+2]);
				}
				if (errorSh < errorNorm)
					currentProject->tileMapC->set_prio(x,y,false);//shadowed
				else
					currentProject->tileMapC->set_prio(x,y,true);//normal
			}
		}
		set_palette_type(type_temp);//0 normal 8 shadow 16 highlight		
	}
	window->redraw();
}
