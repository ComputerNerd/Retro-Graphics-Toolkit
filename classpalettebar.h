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
#pragma once
#define tabsWithPalette 4
class paletteBar{
private:
	unsigned ox[tabsWithPalette],oy[tabsWithPalette];
	unsigned baseOffx[tabsWithPalette],baseOffy[tabsWithPalette];
	bool tiny[tabsWithPalette];
	bool all[tabsWithPalette];
	bool alt[tabsWithPalette];
	uint32_t sysCache;
public:
	Fl_Slider*slide[tabsWithPalette][3];
	unsigned selRow[tabsWithPalette];
	unsigned selBox[tabsWithPalette];
	inline unsigned getEntry(unsigned tab) const{
		return currentProject->pal->perRow*selRow[tab]+selBox[tab];
	}
	void addTab(unsigned tab,bool all=false,bool tiny=false,bool alt=false);
	void setSys(bool upSlide=true);
	void updateSize(unsigned tab);
	void updateSlider(unsigned tab);
	void updateSliders(void){
		for(unsigned i=0;i<tabsWithPalette;++i)
			updateSlider(i);
	}
	unsigned toTab(unsigned realtab);
	void changeRow(unsigned row,unsigned tab){
		selRow[tab]=row;
		updateSlider(tab);
	}
	void checkBox(int x,int y,unsigned tab);
	void drawBoxes(unsigned tab);
};
extern paletteBar palBar;
