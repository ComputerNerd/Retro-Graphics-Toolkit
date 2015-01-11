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
#include "project.h"
#include "includes.h"
#include "classpalettebar.h"
#include "color_convert.h"
#include "callbacks_palette.h"
#include <stdexcept>
static const char*namesGen[]={"Red","Green","Blue"};
static const char*namesNES[]={"Hue","Value","Emphasis"};
paletteBar palBar;
void paletteBar::addTab(unsigned tab,bool all,bool tiny,bool alt){
	this->all[tab]=all;
	this->tiny[tab]=tiny;
	this->alt[tab]=alt;
	unsigned offsetx=16;
	unsigned offsety=tiny?54:56;
	ox[tab]=baseOffx[tab]=offsetx;
	oy[tab]=baseOffy[tab]=offsety;
	sysCache=currentProject->gameSystem;
	offsety+=tiny?6:8;
	for(unsigned i=0;i<3;++i){
		slide[tab][i]=new Fl_Hor_Value_Slider(offsetx+32,offsety+((all?4:1)*(tiny?26:32)),tiny?128:256,tiny?22:24,namesGen[i]);
		slide[tab][i]->minimum(0);
		slide[tab][i]->maximum(7);
		slide[tab][i]->step(1);
		slide[tab][i]->value(0);
		slide[tab][i]->align(FL_ALIGN_LEFT);
		slide[tab][i]->callback(update_palette, (void*)i);
		offsety+=tiny?26:32;
	}
}
void paletteBar::setSys(bool upSlide){
	if(sysCache!=currentProject->gameSystem){
		currentProject->pal->setVars(currentProject->gameSystem);
		for(unsigned i=0;i<TABS_WITH_ROW_BUTTONS*MAX_ROWS_PALETTE;i+=MAX_ROWS_PALETTE){
			for(unsigned j=0;j<currentProject->pal->rowCntPal;++j)
				window->palRTE[i+j]->show();
			for(unsigned j=currentProject->pal->rowCntPal;j<MAX_ROWS_PALETTE;++j)
				window->palRTE[i+j]->hide();
		}
		for(unsigned j=0;j<tabsWithPalette;++j){
			for(unsigned i=0;i<3;++i){
				switch(currentProject->gameSystem){
					case segaGenesis:
						slide[j][i]->label(namesGen[i]);
						slide[j][i]->maximum(7);
					break;
					case masterSystem:
						slide[j][i]->label(namesGen[i]);
						slide[j][i]->maximum(3);
					break;
					case gameGear:
						slide[j][i]->label(namesGen[i]);
						slide[j][i]->maximum(15);
					break;
					case NES:
						slide[j][i]->label(namesNES[i]);
					break;
					default:
						show_default_error
				}
			}
			switch(currentProject->gameSystem){
				case segaGenesis:
				case masterSystem:
				case gameGear:
					if(sysCache==NES){
						slide[j][1]->labelsize(13);
						slide[j][2]->labelsize(14);
						slide[j][2]->resize(slide[j][2]->x()-16,slide[j][2]->y(),slide[j][2]->w()+16,slide[j][2]->h());
						slide[j][2]->callback(update_palette, (void*)2);
					}
				break;
				case NES:
					slide[j][0]->maximum(15);
					slide[j][1]->maximum(3);
					slide[j][1]->labelsize(14);
					slide[j][2]->labelsize(12);
					slide[j][2]->value(0);
					slide[j][2]->maximum(7);
					slide[j][2]->resize(slide[j][2]->x()+16,slide[j][2]->y(),slide[j][2]->w()-16,slide[j][2]->h());
					slide[j][2]->callback(updateEmphesisCB);
				break;
				default:
					show_default_error
			}
			selBox[j]%=currentProject->pal->perRow;
			if(alt[j]&&currentProject->pal->haveAlt){
				if(selRow[j]>=currentProject->pal->rowCntPalalt)
					selRow[j]=currentProject->pal->rowCntPalalt-1;
			}else{
				if(selRow[j]>=currentProject->pal->rowCntPal)
					selRow[j]=currentProject->pal->rowCntPal-1;
			}
		}
		sysCache=currentProject->gameSystem;
	}else
		puts("Warning: syscache is same as gameSystem");
	if(upSlide)
		updateSliders();
}
void paletteBar::updateSize(unsigned tab){
	ox[tab]=(float)((float)window->w()/800.f)*(float)baseOffx[tab];
	oy[tab]=(float)((float)window->h()/600.f)*(float)baseOffy[tab];
}
void paletteBar::updateSlider(unsigned tab){
	if(currentProject->pal->palType[selBox[tab]+(selRow[tab]*currentProject->pal->perRow)]){
		for(unsigned i=0;i<3;++i)
			slide[tab][i]->hide();
	}else{
		for(unsigned i=0;i<3;++i)
			slide[tab][i]->show();
		switch (currentProject->gameSystem){
			case segaGenesis:
				slide[tab][2]->value(currentProject->pal->palDat[(selBox[tab]*2)+(selRow[tab]*32)]>>1);
				slide[tab][1]->value(currentProject->pal->palDat[1+(selBox[tab]*2)+(selRow[tab]*32)]>>5);
				slide[tab][0]->value((currentProject->pal->palDat[1+(selBox[tab]*2)+(selRow[tab]*32)]&14)>>1);		
			break;
			case NES:
				if(alt[tab]){
					slide[tab][0]->value(currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)+16]&15);
					slide[tab][1]->value((currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)+16]>>4)&3);
				}else{
					slide[tab][0]->value(currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)]&15);
					slide[tab][1]->value((currentProject->pal->palDat[selBox[tab]+(selRow[tab]*4)]>>4)&3);
				}
			break;
			case masterSystem:
				slide[tab][0]->value(currentProject->pal->palDat[selBox[tab]+(selRow[tab]*16)]&3);
				slide[tab][1]->value((currentProject->pal->palDat[selBox[tab]+(selRow[tab]*16)]>>2)&3);
				slide[tab][2]->value((currentProject->pal->palDat[selBox[tab]+(selRow[tab]*16)]>>4)&3);
			break;
			case gameGear:
				{uint16_t*palDat=(uint16_t*)currentProject->pal->palDat+selBox[tab]+(selRow[tab]*16);
				slide[tab][0]->value(*palDat&15);
				slide[tab][1]->value((*palDat>>4)&15);
				slide[tab][2]->value((*palDat>>8)&15);}
			break;
			default:
				show_default_error
		}
	}
	window->palType[currentProject->pal->palType[getEntry(tab)]]->setonly();
	window->palType[currentProject->pal->palType[getEntry(tab)]+3]->setonly();
	window->palType[currentProject->pal->palType[getEntry(tab)]+6]->setonly();
}
void paletteBar::drawBoxes(unsigned tab){
	unsigned box_size=window->pal_size->value();
	unsigned x,y,a;
	a=currentProject->pal->perRow*3;
	if(all[tab]){
		unsigned loc_x,loc_y;
		loc_x=(float)((float)window->w()/800.f)*(float)palette_preview_box_x;
		loc_y=(float)((float)window->h()/600.f)*(float)palette_preview_box_y;
		fl_rectf(loc_x,loc_y,box_size*4,box_size*4,currentProject->pal->rgbPal[(selBox[tab]*3)+(selRow[tab]*a)],currentProject->pal->rgbPal[(selBox[tab]*3)+(selRow[tab]*a)+1],currentProject->pal->rgbPal[(selBox[tab]*3)+(selRow[tab]*a)+2]);//this will show larger preview of current color
	}
	if(!all[tab]){
		uint8_t*rgbPtr=currentProject->pal->rgbPal+(a*selRow[tab]);
		if(alt[tab]&&(currentProject->gameSystem==NES))
			rgbPtr+=currentProject->pal->colorCnt*3;
		for (x=0;x<currentProject->pal->perRow;++x){
			fl_rectf(ox[tab]+(x*box_size),oy[tab],box_size,box_size,*rgbPtr,*(rgbPtr+1),*(rgbPtr+2));
			rgbPtr+=3;
		}
		fl_draw_box(FL_EMBOSSED_FRAME,selBox[tab]*box_size+ox[tab],oy[tab],box_size,box_size,0);
	}else{
		uint8_t*rgbPtr=currentProject->pal->rgbPal;
		if(alt[tab]&&(currentProject->gameSystem==NES))
			rgbPtr+=currentProject->pal->colorCnt*3;
		for (y=0;y<currentProject->pal->rowCntPal;++y){
			for (x=0;x<currentProject->pal->perRow;++x){
				fl_rectf(ox[tab]+(x*box_size),oy[tab]+(y*box_size),box_size,box_size,*rgbPtr,*(rgbPtr+1),*(rgbPtr+2));
				rgbPtr+=3;
			}
		}
		fl_draw_box(FL_EMBOSSED_FRAME,selBox[tab]*box_size+ox[tab],selRow[tab]*box_size+oy[tab],box_size,box_size,0);
	}
}
unsigned paletteBar::toTab(unsigned realtab){
	static const int tabLut[]={0,1,2,-1,3,-1,-1};
	if(tabLut[realtab]<0)
		throw std::invalid_argument("Invalid tab");
	return tabLut[realtab];
}
void paletteBar::checkBox(int x,int y,unsigned tab){
	/*!
	This function is in charge of seeing if the mouse click is on a box and what box it is
	for x and y pass the mouser coordinate
	*/
	unsigned boxSize=window->pal_size->value();
	x-=ox[tab];
	y-=oy[tab];
	if (x < 0)
		return;
	if (y < 0)
		return;
	x/=boxSize;
	if (x >= currentProject->pal->perRow)
		return;
	y/=boxSize;
	if (y >= (all?currentProject->pal->rowCntPal:1))
		return;
	selBox[tab]=x;
	if(all[tab])
		changeRow(y,tab);
	else
		updateSlider(tab);
	window->redraw();
}
