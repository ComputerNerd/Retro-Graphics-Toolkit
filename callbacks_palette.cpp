//this is were all the callbacks for palette realted functions go
#include "global.h"
#include "class_global.h"
#include "color_convert.h"
void save_palette(Fl_Widget*, void* start_end)
{
	char temp[4];
	switch (game_system)
	{
		case sega_genesis:
			strcpy(temp,"63");
		break;
		case NES:
			strcpy(temp,"15");
		break;
	}
	char * returned=(char *)fl_input("Counting from zero enter the first entry that you want saved","0");
	if (returned==0)
		return;
	if (verify_str_number_only(returned) == false)
			return;
	uint8_t start = atoi(returned);
	returned=(char *)fl_input("Counting from zero enter the last entry that you want saved",temp);
	if (returned==0)
		return;
	if (verify_str_number_only(returned) == false)
			return;
	uint8_t end = atoi(returned);
	if (game_system==sega_genesis)
	{
		start*=2;
		end *=2;
	}
	uint8_t type=fl_choice("How would like this file saved?","Binary","C header",0);
	if (load_file_generic("Save palette",true) == true)
	{
		//unsigned char start,end;
		//split the varible into two
		//varible format start,end
		//end=(uintptr_t)start_end&0xFF;
		//start=(uintptr_t)start_end>>8;
		//open the file
		//cout << "entry start: " << start/2 << " entry end: " << end/2 << endl;
		//cout << "saving the palette to " << the_file << endl;
		FILE * myfile=0;
		if (type == 1)
		{
			myfile = fopen(the_file.c_str(),"w");
			fputs("const uint8_t palDat[]={",myfile);
		}
		else
			myfile = fopen(the_file.c_str(),"wb");
		if (myfile!=0)
		{
			//save the palette
			if (type == 1)
			{
				if (saveBinAsText(currentProject->palDat+start,end-start,myfile)==false)
				{
					fl_alert("Error: can not save file %s",the_file.c_str());
					return;
				}
				fputs("};",myfile);
			}
			else
			{
				if (fwrite(currentProject->palDat+start,1,end-start,myfile)==0)
				{
					fl_alert("Error: can not save file %s",the_file.c_str());
					return;
				}
			}
			puts("Great Success! File saved!");
		}
		else
		{
			cout << "myfile.is_open() returned false that means there was an error in creating the file" << endl;
		}
		fclose(myfile);
	}

}
void update_palette(Fl_Widget* o, void* v)
{
	//first get the color and draw the box
	Fl_Slider* s = (Fl_Slider*)o;
	//now we need to update the entrie we are editing
	if (game_system == sega_genesis)
	{
		uint8_t temp_var=0;
		uint8_t temp2=(uint8_t)s->value();
		uint8_t temp_entry=0;
		switch (mode_editor)
		{
			case pal_edit:
				temp_entry=palEdit.box_sel+(palEdit.theRow*16);
			break;
			case tile_edit:
				temp_entry=tileEdit_pal.box_sel+(tileEdit_pal.theRow*16);
			break;
			case tile_place:
				temp_entry=tileMap_pal.box_sel+(tileMap_pal.theRow*16);
			break;
		}
		switch ((uintptr_t)v)
		{
			case 0://red
				temp_var=currentProject->palDat[(temp_entry*2)+1];//get the green value we need to save it for later
				//temp_var>>=4;
				//temp_var<<=4;//put the green value back in proper place
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
				//now add the new green value to it
				temp_var+=temp2<<4;
				currentProject->palDat[(temp_entry*2)+1]=temp_var;
				//now convert the new green value
				currentProject->rgbPal[(temp_entry*3)+1]=palTab[(temp2>>1)+palTypeGen];
			break;
			case 2:
				//blue takes the least commands
				currentProject->palDat[temp_entry*2]=temp2;
				currentProject->rgbPal[(temp_entry*3)+2]=palTab[(temp2>>1)+palTypeGen];
			break;
		}
	}
	else if (game_system == NES)
	{
		uint8_t pal;
//		unsigned short temp;
		unsigned int rgb_out;
		uint8_t temp_entry=0;
		switch (mode_editor)
		{
			case pal_edit:
				temp_entry=palEdit.box_sel+(palEdit.theRow*4);
			break;
			case tile_edit:
				temp_entry=tileEdit_pal.box_sel+(tileEdit_pal.theRow*4);
			break;
			case tile_place:
				temp_entry=tileMap_pal.box_sel+(tileMap_pal.theRow*4);
			break;
		}
		switch ((uintptr_t)v)
		{
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
			default://I am not sure why I include a default error handeler the chances of this happening are pretty much zero
				show_default_error
				return;
			break;
		}
		currentProject->palDat[temp_entry]=pal;
		rgb_out=MakeRGBcolor(pal);
		currentProject->rgbPal[temp_entry*3+2]=rgb_out&255;//blue
		currentProject->rgbPal[temp_entry*3+1]=(rgb_out>>8)&255;//green
		currentProject->rgbPal[temp_entry*3]=(rgb_out>>16)&255;//red
	}
	if (mode_editor == tile_edit)
	{
		currentProject->tileC->truecolor_to_tile(tileEdit_pal.theRow,currentProject->tileC->current_tile);//update tile
	}
	window->redraw();//update the palette
}
void loadPalette(Fl_Widget*, void*)
{
	uint32_t file_size;
	uint8_t offset;
	char * inputTemp=(char *)fl_input("Counting from zero enter the first entry that you want the palette to start at","0");
	if (inputTemp==0)
		return;
	if (verify_str_number_only(inputTemp) == false)
			return;
	offset=atoi(inputTemp);
	uint8_t palSize;
	switch (game_system) {
		case sega_genesis:
			offset*=2;
			palSize=128;
		break;
		case NES:
			palSize=16;
		break;
	}
	if(load_file_generic("Load palette") == true) {
		//cout << "offset=" << (uintptr_t)offset << endl;
		//cout << "loading file" << the_file << endl;
		ifstream file (the_file.c_str(), ios::in|ios::binary|ios::ate);
		if (file.is_open()) {
			//copy 32 bytes to the palette buffer
			file_size = file.tellg();
			if (file_size > palSize-offset) {
				fl_alert("Error: The file size is bigger than %d (%d-%d) bytes it is not a valid palette",palSize-offset,palSize,offset);
				file.close();
				return;//end function due to errrors
			}
			//read the palette to the buffer
			file.seekg (0, ios::beg);
			file.read ((char *)currentProject->palDat+offset, file_size);
			file.close();
			//now convert each value to rgb
			switch (game_system) {
				case sega_genesis:
					set_palette_type(palTypeGen);
				break;
				case NES:
					update_emphesis(0,0);
				break;
			}
			window->redraw();
		}
		else
			fl_alert("file.is_open() returned false meaning there was some trouble reading the file.");
	}
}
