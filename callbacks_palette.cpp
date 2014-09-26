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
//this is were all the callbacks for palette realted functions go
#include "global.h"
#include "class_global.h"
#include "color_convert.h"
#include "filemisc.h"
#include "undo.h"
#include "errorMsg.h"
void sortRowbyCB(Fl_Widget*,void*){
	unsigned type=fl_choice("Sort each row by","Hue","Lightness","Saturation");
	sortBy(type,true);
	switch (mode_editor){
		case pal_edit:
			palEdit.updateSlider();
		break;
		case tile_edit:
			tileEdit_pal.updateSlider();
		break;
		case tile_place:
			tileMap_pal.updateSlider();
		break;
		case spriteEditor:
			spritePal.updateSlider();
		break;
		default:
			show_default_error
	}
	window->redraw();
}
void save_palette(Fl_Widget*, void* start_end){
	char temp[4];
	switch (currentProject->gameSystem){
		case sega_genesis:
			strcpy(temp,"63");
		break;
		case NES:
			strcpy(temp,"15");
		break;
		default:
			show_default_error
	}
	char * returned=(char *)fl_input("Counting from zero enter the first entry that you want saved\nFor NES to save the sprite palette the first entry is 16","0");
	if(!returned)
		return;
	if(!verify_str_number_only(returned))
		return;
	unsigned start = atoi(returned);
	returned=(char *)fl_input("Counting from zero enter the last entry that you want saved",temp);
	if (!returned)
		return;
	if (!verify_str_number_only(returned))
		return;
	unsigned end = atoi(returned)+1;
	bool skipzero;
	uint8_t bufskip[32];
	unsigned szskip=0;
	if(currentProject->gameSystem==NES){
		skipzero=fl_ask("Would you like to skip saving color 0 for all rows except zero?");
		if(skipzero){
			uint8_t*bufptr=bufskip;
			for(unsigned i=start;i<end;++i){
				if((i&3)||(i==0)){
					*bufptr++=currentProject->palDat[i];
					++szskip;
				}
			}
		}
	}else
		skipzero=false;
	fileType_t type=askSaveType();
	int clipboard;
	if(type){
		clipboard=clipboardAsk();
		if(clipboard==2)
			return;
	}else
		clipboard=0;
	bool pickedFile;
	if(clipboard)
		pickedFile=true;
	else
		pickedFile=load_file_generic("Save palette",true);
	if(pickedFile){
		FILE * myfile;
		if(clipboard)
			myfile=0;//When file is null for the function saveBinAsText clipboard will be used
		else if(type)
			myfile = fopen(the_file.c_str(),"w");
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (likely(myfile||clipboard)){
			//save the palette
			if (type){
				char comment[512];
				snprintf(comment,512,"Colors %d-%d",start,end-1);
				int bits;
				if(currentProject->gameSystem==sega_genesis){
					start*=2;//Be sure to keep this in sync with the other if statment shortly below
					end*=2;
					bits=16;
				}else
					bits=8;
				if(skipzero){
					if (!saveBinAsText(bufskip,szskip,myfile,type,comment,"palDat",bits)){
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}else{
					if (!saveBinAsText(currentProject->palDat+start,end-start,myfile,type,comment,"palDat",bits)){
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}
			}else{
				if(currentProject->gameSystem==sega_genesis){
					start*=2;
					end*=2;
				}
				if(skipzero){
					if (fwrite(bufskip,1,szskip,myfile)==0){
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}else{
					if (fwrite(currentProject->palDat+start,1,end-start,myfile)==0){
						fl_alert("Error: can not save file %s",the_file.c_str());
						return;
					}
				}
			}
			if(myfile)
				fclose(myfile);
		}else
			fl_alert("Cannot open file %s",the_file.c_str());
	}
}
void update_palette(Fl_Widget* o, void* v){
	//first get the color and draw the box
	Fl_Slider* s = (Fl_Slider*)o;
	//now we need to update the entry we are editing
	unsigned temp_entry=0;
	switch (mode_editor){
		case pal_edit:
			temp_entry=palEdit.box_sel+(palEdit.theRow*palEdit.perRow);
		break;
		case tile_edit:
			temp_entry=tileEdit_pal.box_sel+(tileEdit_pal.theRow*tileEdit_pal.perRow);
		break;
		case tile_place:
			temp_entry=tileMap_pal.box_sel+(tileMap_pal.theRow*tileMap_pal.perRow);
		break;
		case spriteEditor:
			temp_entry=spritePal.box_sel+(spritePal.theRow*spritePal.perRow);
			if(currentProject->gameSystem==NES)
				temp_entry+=16;
		break;
		default:
			show_default_error
	}
	if(pushed_g||(Fl::event()==FL_KEYDOWN)){
		pushed_g=0;
		pushPaletteEntry(temp_entry);
	}
	if (currentProject->gameSystem == sega_genesis){
		uint8_t temp_var=0;
		uint8_t temp2=(uint8_t)s->value();
		switch ((uintptr_t)v){
			case 0://red
				temp_var=currentProject->palDat[(temp_entry*2)+1];//get the green value we need to save it for later
				temp_var&=0xF0;
				temp_var|=temp2;
				currentProject->palDat[(temp_entry*2)+1]=temp_var;
				//now convert the new red value
				currentProject->rgbPal[temp_entry*3]=palTab[(temp2>>1)+palTypeGen];
			break;
			case 1://green
				//this is very similar to what I just did above
				temp_var=currentProject->palDat[(temp_entry*2)+1];
				temp_var&=15;//get only the red value
				//now OR the new green value to it
				temp_var|=temp2<<4;
				currentProject->palDat[(temp_entry*2)+1]=temp_var;
				//now convert the new green value
				currentProject->rgbPal[(temp_entry*3)+1]=palTab[(temp2>>1)+palTypeGen];
			break;
			case 2:
				//blue is the most trival conversion to do
				currentProject->palDat[temp_entry*2]=temp2;
				currentProject->rgbPal[(temp_entry*3)+2]=palTab[(temp2>>1)+palTypeGen];
			break;
		}
	}
	else if (currentProject->gameSystem == NES){
		uint8_t pal;
		uint32_t rgb_out;
		switch ((uintptr_t)v){
			/*
			76543210
			||||||||
			||||++++- Hue (phase)
			||++----- Value (voltage)
			++------- Unimplemented, reads back as 0
			*/
			case 0://Hue
				//first read out value
				pal=currentProject->palDat[temp_entry];
				pal&=48;
				pal|=(uint8_t)s->value();
			break;
			case 1://Value
				pal=currentProject->palDat[temp_entry];
				pal&=15;
				pal|=((uint8_t)s->value())<<4;
			break;
		}
		currentProject->palDat[temp_entry]=pal;
		rgb_out=MakeRGBcolor(pal);
		currentProject->rgbPal[temp_entry*3+2]=rgb_out&255;//blue
		currentProject->rgbPal[temp_entry*3+1]=(rgb_out>>8)&255;//green
		currentProject->rgbPal[temp_entry*3]=(rgb_out>>16)&255;//red
	}
	if (mode_editor == tile_edit)
		currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile,false);//update tile
	window->redraw();//update the palette
}
void loadPalette(Fl_Widget*, void*){
	uint32_t file_size;
	uint8_t offset;
	char * inputTemp=(char *)fl_input("Counting from zero enter the first entry that you want the palette to start at\nFor NES to load a sprite palette enter 16 or greater","0");
	if (!inputTemp)
		return;
	if (!verify_str_number_only(inputTemp))
		return;
	offset=atoi(inputTemp);
	uint8_t palSize;
	switch (currentProject->gameSystem){
		case sega_genesis:
			offset*=2;
			palSize=128;
		break;
		case NES:
			palSize=32;
		break;
	}
	if(load_file_generic("Load palette") == true){
		FILE * fi=fopen(the_file.c_str(), "rb");
		if(fi){
			//copy 32 bytes to the palette buffer
			fseek(fi,0,SEEK_END);
			file_size = ftell(fi);
			if (file_size > palSize-offset){
				fl_alert("Error: The file size is bigger than %d (%d-%d) bytes\nMaybe there is extra data or you loaded the wrong file?",palSize-offset,palSize,offset);
				fclose(fi);
				return;//end function due to errrors
			}
			//read the palette to the buffer
			rewind(fi);
			fread(currentProject->palDat+offset,1,file_size,fi);
			fclose(fi);
			//now convert each value to rgb
			switch (currentProject->gameSystem){
				case sega_genesis:
					set_palette_type();
				break;
				case NES:
					update_emphesis(0,0);
				break;
			}
			window->redraw();
		}else
			fl_alert("Error opening file");
	}
}
void set_ditherAlg(Fl_Widget*,void* typeset){
	if ((uintptr_t)typeset==0)
		window->ditherPower->show();
	else
		window->ditherPower->hide();//imagine the user trying to change the power and nothing happening not fun at all
	currentProject->settings&=~settingsDitherMask;
	currentProject->settings|=(uintptr_t)typeset&settingsDitherMask;
}
void set_tile_row(Fl_Widget*,void* row){
	uint8_t selrow=(uintptr_t)row;
	switch (mode_editor){
		case tile_edit:
			tileEdit_pal.changeRow(selrow);
			currentProject->tileC->truecolor_to_tile(selrow,currentProject->tileC->current_tile,false);
		break;
		case tile_place:
			tileMap_pal.changeRow(selrow);
			if(tileEditModePlace_G){
				pushTilemapEdit(selTileE_G[0],selTileE_G[1]);
				currentProject->tileMapC->set_pal_row(selTileE_G[0],selTileE_G[1],selrow);
			}
		break;
	}
	window->redraw();//trigger a redraw so that the new row is displayed
}
void setPalType(Fl_Widget*,void*type){
	switch (mode_editor){
		case pal_edit:
			currentProject->palType[palEdit.getEntry()]=(uintptr_t)type;
			palEdit.updateSlider();
		break;
		case tile_edit:
			currentProject->palType[tileEdit_pal.getEntry()]=(uintptr_t)type;
			tileEdit_pal.updateSlider();
		break;
		case tile_place:
			currentProject->palType[tileMap_pal.getEntry()]=(uintptr_t)type;
			tileMap_pal.updateSlider();
		break;
		default:
			show_default_error
	}
	window->redraw();
}
void pickNearAlg(Fl_Widget*,void*){
	nearestAlg=fl_choice("Which nearest color algorithm would you like to use?","ciede2000","Weighted http://www.compuphase.com/cmetric.htm","Euclidean distance");
}
void rgb_pal_to_entry(Fl_Widget*,void*){
	//this function will convert a rgb value to the nearst palette entry
	if (mode_editor != tile_edit){
		fl_alert("Be in Tile editor to use this");
		return;
	}
	uint8_t rgb[3];
	rgb[0]=window->rgb_red->value();
	rgb[1]=window->rgb_green->value();
	rgb[2]=window->rgb_blue->value();
	switch(currentProject->gameSystem){
		case sega_genesis:
			{unsigned en=tileEdit_pal.getEntry();
			uint16_t temp=to_sega_genesis_colorRGB(rgb[0],rgb[1],rgb[2],en);
			en*=2;
			currentProject->palDat[en]=temp>>8;
			currentProject->palDat[en+1]=temp&255;}
		break;
		case NES:
			{uint8_t bestCol=to_nes_color_rgb(rgb[0],rgb[1],rgb[2]);
			unsigned en=tileEdit_pal.getEntry();
			currentProject->palDat[en]=bestCol;
			en*=3;
			uint32_t rgb_out=MakeRGBcolor(bestCol);
			currentProject->rgbPal[en+2]=rgb_out&255;
			currentProject->rgbPal[en+1]=(rgb_out>>8)&255;
			currentProject->rgbPal[en]=(rgb_out>>16)&255;}
		break;
	}
	tileEdit_pal.updateSlider();
	currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile,false);
	window->redraw();
}
void entryToRgb(Fl_Widget*,void*){
	unsigned en=tileEdit_pal.getEntry()*3;
	truecolor_temp[0]=currentProject->rgbPal[en];
	truecolor_temp[1]=currentProject->rgbPal[en+1];
	truecolor_temp[2]=currentProject->rgbPal[en+2];
	window->rgb_red->value(truecolor_temp[0]);
	window->rgb_green->value(truecolor_temp[1]);
	window->rgb_blue->value(truecolor_temp[2]);
	window->redraw();
}
void clearPalette(Fl_Widget*,void*){
	if(fl_ask("This will set all colors to 0 are you sure you want to do this?\nYou can undo this by pressing pressing CTRL+Z")){
		pushPaletteAll();
		memset(currentProject->palDat,0,128);
		memset(currentProject->rgbPal,0,192);
		window->damage(FL_DAMAGE_USER1);
		palEdit.updateSlider();
		tileEdit_pal.updateSlider();
		tileMap_pal.updateSlider();
		spritePal.updateSlider();
	}
}
