#include "global.h"
unsigned char to_nes_color_rgb(unsigned char red,unsigned char green,unsigned char blue)
{
	//this function does not set any values to global palette it is done in other functions
	int min_error =(255*255) +(255*255) +(255*255) +1;
	unsigned char bestcolor=0;
	for (unsigned char a=0;a<16;a++)
	{
		for (unsigned char c=0;c<4;c++)//c++ haha
		{
			unsigned int rgb_out=MakeRGBcolor(a+(c<<4));
			unsigned char b=rgb_out&255;
			unsigned char g=(rgb_out>>8)&255;
			unsigned char r=(rgb_out>>16)&255;
			int rdiff= (int)r - (int)red;
			int gdiff= (int)g - (int)green;
			int bdiff= (int)b - (int)blue;
			int dist = (rdiff*rdiff) + (gdiff*gdiff) + (bdiff*bdiff);
			if (dist < min_error)
			{
				min_error = dist;
				bestcolor=a+(c<<4);
			}
		}
	}
	return bestcolor;
}
unsigned char to_nes_color(unsigned char pal_index)
{
	//this function does not set any values to global palette it is done in other functions
	int min_error =(255*255) +(255*255) +(255*255) +1;
	unsigned char bestcolor=0;
	for (unsigned char a=0;a<16;a++)
	{
		for (unsigned char c=0;c<4;c++)//c++ haha
		{
			unsigned int rgb_out=MakeRGBcolor(a+(c<<4));
			unsigned char b=rgb_out&255;
			unsigned char g=(rgb_out>>8)&255;
			unsigned char r=(rgb_out>>16)&255;
			int rdiff= (int)r - (int)currentProject->rgbPal[pal_index*3];
			int gdiff= (int)g - (int)currentProject->rgbPal[(pal_index*3)+1];
			int bdiff= (int)b - (int)currentProject->rgbPal[(pal_index*3)+2];
			int dist = (rdiff*rdiff) + (gdiff*gdiff) + (bdiff*bdiff);
			if (dist < min_error)
			{
				min_error = dist;
				bestcolor=a+(c<<4);
			}
		}
	}
	return bestcolor;
}
unsigned short to_sega_genesis_color(unsigned char pal_index)
{
	//note this function only set the new rgb colors not the outputed sega genesis palette format
	pal_index*=3;
	unsigned char r=currentProject->rgbPal[pal_index];
	unsigned char g=currentProject->rgbPal[pal_index+1];
	unsigned char b=currentProject->rgbPal[pal_index+2];
	r=(r+18)/36;
	g=(g+18)/36;
	b=(b+18)/36;
	r+=r;//multiply by 2 in the sega genesis hardware all palette have 3 valid bits but bit zero is always set to zero
	g+=g;
	b+=b;
	currentProject->rgbPal[pal_index]=r*18;
	currentProject->rgbPal[pal_index+1]=g*18;
	currentProject->rgbPal[pal_index+2]=b*18;
	//bgr format
	return r+(g<<4)+(b<<8);
}

uint32_t count_colors(uint8_t * image_ptr,uint32_t w,uint32_t h,uint8_t *colors_found,bool useAlpha=false)
{
	/*!
	Scans for colors in an image stops at over 256 as if there is an excess of 256 colors there is no reason to countinue
	*/
	//memset(colors_found,0,w*h*3);
	uint32_t colors_amount=3;
	colors_found[0]=*image_ptr++;
	colors_found[1]=*image_ptr++;
	colors_found[2]=*image_ptr++;
	if (useAlpha)
		image_ptr++;
	uint8_t start=1;
	unsigned int y;

	for (y=0;y<h;y++)
	{
		for (uint32_t x=start;x<w;x++)
		{
			start=0;
			unsigned char r;
			unsigned char g;
			unsigned char b;
			r=*image_ptr++;
			g=*image_ptr++;
			b=*image_ptr++;
			if (useAlpha)
				image_ptr++;
			bool new_col=true;
			for (unsigned int c=0;c<colors_amount;c+=3)
			{
				if (r == colors_found[c] && g == colors_found[c+1] && b == colors_found[c+2])
				{
					new_col=false;
					break;//exit loop
				}
			}
			if (new_col==true)
			{
				colors_found[colors_amount]=r;
				colors_found[colors_amount+1]=g;
				colors_found[colors_amount+2]=b;
				colors_amount+=3;
			}
			if (colors_amount >= 765)
			{
				printf("\nOver 255 colors timing out no need for operation to countinue.\n");
				return colors_amount/3;//to save on multiplication we have it times 3
			}
		}
			//update progress bar
			printf("counting colors %% %f Colors Found: %d\r",((float)y/(float)h)*100.0,colors_amount/3);
			
	}
	printf("\n");
	return colors_amount/3;
}
void update_emphesis(Fl_Widget*,void*)
{
	uint8_t emps;
	switch (mode_editor)
	{
		case pal_edit:
			emps=palEdit.pal_b->value();
		break;
		case tile_edit:
			emps=tileEdit_pal.pal_b->value();
		break;
		case tile_place:
			emps=tileMap_pal.pal_b->value();
		break;
	}
	for (uint8_t c=0;c<48;c+=3)
	{
		unsigned int rgb_out;
		rgb_out=MakeRGBcolor(currentProject->palDat[c/3]+(emps<<6));
		currentProject->rgbPal[c]=(rgb_out>>16)&255;//red
		currentProject->rgbPal[c+1]=(rgb_out>>8)&255;//green
		currentProject->rgbPal[c+2]=rgb_out&255;//blue
	}
	window->redraw();
}
