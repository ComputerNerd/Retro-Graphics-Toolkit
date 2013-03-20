//this is were all the callbacks for palette realted functions go
#include "global.h"
#include "class_global.h"
void save_palette(Fl_Widget*, void* start_end)
{
	if (load_file_generic("Save palette",true) == true)
	{
		unsigned char start,end;
		//split the varible into two
		//varible format start,end
		end=(uintptr_t)start_end&0xFF;
		start=(uintptr_t)start_end>>8;
		//open the file
		cout << "entry start: " << start/2 << " entry end: " << end/2 << endl;
		cout << "saving the palette to " << the_file << endl;
		ofstream myfile;
		myfile.open(the_file.c_str(),ios::binary | ios::trunc);
		if (myfile.is_open())
		{
			//save the palette
			myfile.write((char *)currentProject->palDat+start,end-start);
			cout << "Great Success! File saved!" << endl;
		}
		else
		{
			cout << "myfile.is_open() returned false that means there was an error in creating the file" << endl;
		}
		myfile.close();
	}

}
void update_palette(Fl_Widget* o, void* v)
{
	//first get the color and draw the box
	Fl_Slider* s = (Fl_Slider*)o;
	//now we need to update the entrie we are editing
	if (game_system == sega_genesis)
	{
		
		//rgb_temp[fl_intptr_t(v)] = s->value();
		uint8_t temp_var;
		uint8_t temp_entry;
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
				temp_var+=(unsigned char)s->value();//value() returns a double so we need to cast it to unsigned char
				currentProject->palDat[(temp_entry*2)+1]=temp_var;
				//now convert the new red value
				currentProject->rgbPal[temp_entry*3]=(unsigned char)palette_adder+s->value()*palette_muliplier;
			break;
			case 1://green
				//this is very similar to what I just did above
				temp_var=currentProject->palDat[(temp_entry*2)+1];
				temp_var&=15;//get only the red value
				//now add the new green value to it
				temp_var+=(unsigned char)s->value()<<4;
				currentProject->palDat[(temp_entry*2)+1]=temp_var;
				//now convert the new green value
				currentProject->rgbPal[(temp_entry*3)+1]=(unsigned char)palette_adder+s->value()*palette_muliplier;
			break;
			case 2:
				//blue takes the least commands
				currentProject->palDat[temp_entry*2]=(unsigned char)s->value();
				currentProject->rgbPal[(temp_entry*3)+2]=(unsigned char)palette_adder+s->value()*palette_muliplier;
			break;
		}
	}
	else if (game_system == NES)
	{
		uint8_t pal;
//		unsigned short temp;
		unsigned int rgb_out;
		uint8_t temp_entry;
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
void Butt_CB(Fl_Widget*, void* offset)
{
	if(load_file_generic("Load palette") == true)
	{
		cout << "offset=" << (uintptr_t)offset << endl;
		cout << "loading file" << the_file << endl;
		ifstream file (the_file.c_str(), ios::in|ios::binary|ios::ate);
		if (file.is_open())
		{
			//copy 32 bytes to the palette buffer
			file_size = file.tellg();
			if (file_size > 128-(uintptr_t)offset)
			{
				//cout << "error file size is bigger than " << 128-(unsigned short)offset << " bytes not a valid palette?" << endl;
				fl_alert("Error: The file size is bigger than %d bytes it is not a valid palette",(int)128-(int)(uintptr_t)offset);
				file.close();
				return;//end function due to errrors
			}
			if (offset == 0 && file_size != 32)
			{
				cout << "warning file size not 32 bytes file size is: " << file_size << "\nthis should be normal if all your colors are stored in one file" << endl;
			}
			//read the palette to the buffer
			file.seekg (0, ios::beg);
			//cout << "reading to buffer" << endl; Who is gonna care about that???
			file.read ((char *)currentProject->palDat+(uintptr_t)offset, file_size);
			file.close();
			//now convert each value to rgb
			for (unsigned char pal=(uintptr_t)offset; pal < file_size+(uintptr_t)offset;pal+=2)
			{
				//to convert to rgb first get value of color then multiply it by 16 to get rgb
				//first get blue value
				//the rgb array is in rgb format and the genesis palette is bgr format
				//cout << "converting palette number " << (unsigned short)pal/2 << endl;//cout does not print char or unsigned char so it has to be cast to something else
				printf("Converting palette number: %d\n",pal/2);
				unsigned char rgb_array = pal+(pal/2);//multiply pal by 1.5
				unsigned char temp_var = currentProject->palDat[pal];
				temp_var*=palette_muliplier;
				temp_var+=palette_adder;
				currentProject->rgbPal[rgb_array+2]=temp_var;
				//<< = left bitshift >> = right bitshifts
				//seperating the gr values will require some bitwise operations
				//to get g shift to the right by 4
				temp_var = currentProject->palDat[pal+1];
				temp_var>>=4;
				temp_var*=palette_muliplier;
				temp_var+=palette_adder;
				currentProject->rgbPal[rgb_array+1]=temp_var;
				//to get r value apply the and opperation by 0xF or 15
				temp_var = currentProject->palDat[pal+1];
				temp_var&=0xF;
				temp_var*=palette_muliplier;
				temp_var+=palette_adder;
				currentProject->rgbPal[rgb_array]=temp_var;
				//now tell us the rgb values
				//cout << "Red: " << (unsigned short) currentProject->rgbPal[rgb_array] << " Green: " << (unsigned short) currentProject->rgbPal[rgb_array+1] << " Blue: " << (unsigned short) currentProject->rgbPal[rgb_array+2] << endl;
				printf("Red: %d Green: %d Blue: %d\n",currentProject->rgbPal[rgb_array],currentProject->rgbPal[rgb_array+1],currentProject->rgbPal[rgb_array+2]);
			}
			//mode_editor=pal_edit;
			window->redraw();
		}
		else
		{
			cout << "file.is_open() returned false meaning there was some trouble reading the file." << endl;
		}
	}
}
