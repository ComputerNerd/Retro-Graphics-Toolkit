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
#include "global.h"
#include "savepng.h"
#include "dither.h"
#include "callback_chunk.h"
#include "callbacksprites.h"
void tileDPicker(Fl_Widget*,void*){
	Fl_Window *win;
	Fl_Progress *progress;
	win = new Fl_Window(250,45,"Progress");		// access parent window
	win->begin();                                // add progress bar to it..
	progress = new Fl_Progress(25,7,200,30);
	progress->minimum(0.0);                      // set progress range to be 0.0 ~ 1.0
	progress->maximum(1.0);
	progress->color(0x88888800);               // background color
	progress->selection_color(0x4444ff00);     // progress bar color
	progress->labelcolor(FL_WHITE);            // percent text color
	win->end();                                  // end adding to window
	win->show();
	currentProject->tileMapC->pickRowDelta(true,progress);
	win->remove(progress);// remove progress bar from window
	delete(progress);// deallocate it
	//w->draw();
	delete win;
	window->damage(FL_DAMAGE_USER1);
}

void setMapW(Fl_Widget*,void*){
	char * str_ptr;
	str_ptr=(char *)fl_input("Enter Width");
	if (!str_ptr)
		return;
	if (!verify_str_number_only(str_ptr))
		return;
	uint32_t wTemp=atoi(str_ptr);
	if(currentProject->tileMapC->isBlock)
		currentProject->tileMapC->resizeBlocks(wTemp,window->map_h->value());
	else
		currentProject->tileMapC->resize_tile_map(wTemp,window->map_h->value());
	window->map_w->value(wTemp);
	window->redraw();
}
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
	bool Toggle=b->value()?true:false;
	currentProject->tileMapC->toggleBlocks(Toggle);
	if(!Toggle){
		currentProject->Chunk->useBlocks=false;
		window->useBlocksChunkCBtn->value(0);
	}
	window->redraw();
}
void FixOutOfRangeCB(Fl_Widget*,void*){
	//use current attributes
	for(int y=0;y<currentProject->tileMapC->mapSizeHA;++y){
		for(int x=0;x<currentProject->tileMapC->mapSizeW;++x){
			if(currentProject->tileMapC->get_tile(x,y)>currentProject->tileC->tiles_amount)
				currentProject->tileMapC->set_tile_full(currentProject->tileC->current_tile,x,y,tileMap_pal.theRow,G_hflip[0],G_vflip[0],G_highlow_p[0]);
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
		uint32_t h=currentProject->tileMapC->mapSizeHA*8;
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
		uint32_t h=currentProject->tileMapC->mapSizeHA*8;
		uint8_t * image=(uint8_t*)malloc(w*h*3);
		currentProject->tileMapC->truecolor_to_image(image,-1,false);
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
		for (uint32_t y=0;y<currentProject->tileMapC->mapSizeHA;++y){
			for (uint32_t x=0;x<currentProject->tileMapC->mapSizeW;++x)
				currentProject->tileMapC->set_tile_full(currentProject->tileC->current_tile,x,y,tileMap_pal.theRow,G_hflip[0],G_vflip[0],G_highlow_p[0]);
		}
		window->damage(FL_DAMAGE_USER1);
	}
}
void dither_tilemap_as_image(Fl_Widget*,void*sprite){
	//normally this program dithers all tiles individully this is not always desirable
	//to fix this I created this function It convertes the tilemap to image and dithers all tiles
	//so first create ram for image
	bool isSprite=((uintptr_t)sprite)?true:false;
	uint8_t * image;
	uint32_t w,h;
	if(isSprite){
		w=currentProject->spritesC->spriteslist[curSprite]->w;
		h=currentProject->spritesC->spriteslist[curSprite]->h;
	}else{
		w=currentProject->tileMapC->mapSizeW;
		h=currentProject->tileMapC->mapSizeHA;
	}
	w*=currentProject->tileC->sizex;
	h*=currentProject->tileC->sizey;
	unsigned method;
	if(isSprite)
		method=1;
	else
		method=fl_choice("How would you like this tilemap dithered?","Dither each palette row separately","Dither entire image at once","Cancel");
	if(method==2)
		return;
	image = (uint8_t *)malloc(w*h*4);
	if (!image)
		show_malloc_error(w*h*4)
	if(method==1){
		if(isSprite){
			tileMap*spriteMap=new tileMap(w/currentProject->tileC->sizex,h/currentProject->tileC->sizey);
			//make verticle tile map
			unsigned t=currentProject->spritesC->spriteslist[curSprite]->starttile;
			for(unsigned i=0;i<w/currentProject->tileC->sizex;++i){
				for(unsigned j=0;j<h/currentProject->tileC->sizey;++j){
					//void set_tile_full(uint32_t tile,uint32_t x,uint32_t y,uint8_t palette_row,bool use_hflip,bool use_vflip,bool highorlow_prio);
					spriteMap->set_tile_full(t++,i,j,currentProject->spritesC->spriteslist[curSprite]->palrow,false,false,false);
				}
			}
			spriteMap->truecolor_to_image(image,-1);
			ditherImage(image,w,h,true,true,true,currentProject->spritesC->spriteslist[curSprite]->palrow);
			ditherImage(image,w,h,true,false,true,currentProject->spritesC->spriteslist[curSprite]->palrow);
			spriteMap->truecolorimageToTiles(image,-1);
			delete spriteMap;
		}else{
			currentProject->tileMapC->truecolor_to_image(image,-1);
			ditherImage(image,w,h,true,true);
			ditherImage(image,w,h,true,false);
			currentProject->tileMapC->truecolorimageToTiles(image,-1);
		}
	}else{
		for (uint8_t rowz=0;rowz<4;++rowz){
			printf("Row %d\n",rowz);
			currentProject->tileMapC->truecolor_to_image(image,rowz);
			ditherImage(image,w,h,true,true);
			ditherImage(image,w,h,true,false);
			//convert back to tiles
			currentProject->tileMapC->truecolorimageToTiles(image,rowz);
		}
	}
	puts("Done with image");
	window->damage(FL_DAMAGE_USER1);
	Fl::check();
	free(image);
	window->redraw();
}
void load_image_to_tilemap(Fl_Widget*,void*o){
	Fl_Shared_Image * loaded_image;
	bool over=(uintptr_t)o?1:0;
	if (load_file_generic("Load image")){
		loaded_image=Fl_Shared_Image::get(the_file.c_str());
		if(!loaded_image){
			fl_alert("Error loading image");
			return;
		}
		unsigned tilebitw,tilebith;
		tilebitw=currentProject->tileC->sizex;
		tilebith=currentProject->tileC->sizey;
		if((currentProject->subSystem&NES2x2)&&(currentProject->gameSystem==NES)){
			tilebitw*=2;
			tilebith*=2;
		}
		uint32_t w,h;
		w=loaded_image->w();
		h=loaded_image->h();
		printf("image width: %d image height: %d\n",w,h);
		uint32_t w8,h8;
		uint32_t wt,ht,wr,hr;
		wr=w%tilebitw;
		hr=h%tilebith;
		w8=w/currentProject->tileC->sizex;
		h8=h/currentProject->tileC->sizey;
		if (wr)
			++w8;
		if (hr)
			++h8;
		if((currentProject->gameSystem==NES)&&(currentProject->subSystem=NES2x2)){
			if((wr-8)>0)
				++w8;
			if((hr-8)>0)
				++h8;
		}
		wt=w8*currentProject->tileC->sizex;
		ht=h8*currentProject->tileC->sizey;
		if(wr)
			fl_alert("Warning width is not a multiple of %d",tilebitw);
		if(hr)
			fl_alert("Warning height is not a multiple of %d",tilebith);
		//start by copying the data
		uint8_t * imgptr=(uint8_t *)loaded_image->data()[0];
		//now we can convert to tiles
		if (unlikely(loaded_image->d() != 3 && loaded_image->d() != 4)){
			fl_alert("Please use color depth of 3 or 4\nYou Used %d",loaded_image->d());
			loaded_image->release();
			return;
		}else
			printf("Image depth %d\n",loaded_image->d());
		uint64_t truecolor_tile_ptr=0;
		if(!over){
			currentProject->tileC->truetileDat = (uint8_t *)realloc(currentProject->tileC->truetileDat,w8*h8*256);
			currentProject->tileC->tileDat = (uint8_t *)realloc(currentProject->tileC->tileDat,w8*h8*currentProject->tileC->tileSize);
			currentProject->tileC->tiles_amount=(w8*h8)-1;
			updateTileSelectAmt();
		}
		uint32_t tx,ty=0;
		unsigned center[3];
		center[0]=(wt-w)/2;
		center[1]=(ht-h)/2;
		center[2]=wt-w-center[0];
		unsigned depth=loaded_image->d();
		for(uint32_t y=0,tcnt=0;y<ht;++y,++ty){
			if(y%currentProject->tileC->sizey)
				tcnt-=wt/currentProject->tileC->sizex;
			tx=0;
			for(uint32_t x=0;x<wt;x+=currentProject->tileC->sizex,++tcnt,++tx){
				uint32_t ctile;
				if(over)
					ctile=currentProject->tileMapC->get_tile(tx,ty);
				else
					ctile=tcnt;
				uint8_t*ttile=currentProject->tileC->truetileDat+((ctile*currentProject->tileC->sizex*currentProject->tileC->sizey*4)+((y%currentProject->tileC->sizey)*currentProject->tileC->sizex*4));
				//First take care of border
				unsigned line=currentProject->tileC->sizex;
				if((y<center[1])||(y>=(h+center[1])))
					memset(ttile,0,line*4);
				else{
					if(x<center[0]){
						memset(ttile,0,center[0]*4);
						line-=center[0];
						ttile+=center[0]*4;
					}else if(x>=(wt-currentProject->tileC->sizex))
						line-=center[2];
					switch (depth){
						case 3:
							for(unsigned xx=0;xx<line;++xx){
								*ttile++=*imgptr++;
								*ttile++=*imgptr++;
								*ttile++=*imgptr++;
								*ttile++=255;
							}
						break;
						case 4:
							memcpy(ttile,imgptr,line*4);
							imgptr+=line*4;
							ttile+=line*4;
						break;
					}
					if(x>=(wt-currentProject->tileC->sizex))
						memset(ttile,0,center[2]*4);
				}
			}
		}
		loaded_image->release();
		if(!over){
			currentProject->tileMapC->resize_tile_map(w8,h8);
			window->map_w->value(w8);
			window->map_h->value(h8);
			uint32_t tilecounter=0;
			for (uint32_t y=0;y<h8;++y){
				for (uint32_t x=0;x<w8;++x){
					currentProject->tileMapC->set_tile_full(tilecounter,x,y,0,false,false,false);
					++tilecounter;
				}
			}
		}
		window->redraw();
	}
}
void set_prioCB(Fl_Widget*,void*o){
	unsigned off=(uintptr_t)o;
	G_highlow_p[off]^=true;
	if((tileEditModeChunk_G)&&(off==1))
		currentProject->Chunk->setPrio(currentChunk,editChunk_G[0],editChunk_G[1],G_highlow_p[off]);
	if(tileEditModePlace_G)
		currentProject->tileMapC->set_prio(selTileE_G[0],selTileE_G[1],G_highlow_p[off]);
	window->redraw();
}
void set_hflipCB(Fl_Widget*,void*o){
	unsigned off=(uintptr_t)o;
	G_hflip[off]^=true;
	if((tileEditModeChunk_G)&&(off==1))
		currentProject->Chunk->setHflip(currentChunk,editChunk_G[0],editChunk_G[1],G_hflip[off]);
	else if(tileEditModePlace_G&&(off==0))
		currentProject->tileMapC->set_hflip(selTileE_G[0],selTileE_G[1],G_hflip[off]);
	window->redraw();
}
void set_vflipCB(Fl_Widget*,void*o){
	unsigned off=(uintptr_t)o;
	G_vflip[off]^=true;
	if((tileEditModeChunk_G)&&(off==1))
		currentProject->Chunk->setVflip(currentChunk,editChunk_G[0],editChunk_G[1],G_vflip[off]);
	else if(tileEditModePlace_G&&(off==0))
		currentProject->tileMapC->set_vflip(selTileE_G[0],selTileE_G[1],G_vflip[off]);
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
		for (y=0;y<currentProject->tileMapC->mapSizeHA;++y){
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
		for (y=0;y<currentProject->tileMapC->mapSizeHA;++y){
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
