/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#pragma once
#define TABS_WITH_PALETTE 4
#include <FL/Fl_Hor_Value_Slider.H>

class paletteBar {
private:
	unsigned ox[TABS_WITH_PALETTE], oy[TABS_WITH_PALETTE];
	unsigned baseOffx[TABS_WITH_PALETTE], baseOffy[TABS_WITH_PALETTE];
	bool tiny[TABS_WITH_PALETTE];
	bool all[TABS_WITH_PALETTE];
	bool alt[TABS_WITH_PALETTE];
public:
	int32_t sysCache;
	Fl_Slider*slide[TABS_WITH_PALETTE][3];
	unsigned selRow[TABS_WITH_PALETTE];
	unsigned selBox[TABS_WITH_PALETTE];
	unsigned selBoxAlt[TABS_WITH_PALETTE];
	unsigned getEntry(unsigned tab)const;
	void addTab(unsigned tab, bool all = false, bool tiny = false, bool alt = false);
	void setSys(bool upSlide = true);
	void updateSize(unsigned tab);
	void updateSlider(unsigned tab);
	void updateSliders(void) {
		for (unsigned i = 0; i < TABS_WITH_PALETTE; ++i)
			updateSlider(i);
	}
	unsigned toTab(unsigned realtab);
	void changeRow(unsigned row, unsigned tab) {
		selRow[tab] = row;
		updateSlider(tab);
	}
	void checkBox(int x, int y, unsigned tab);
	void drawBoxes(unsigned tab);
};
extern paletteBar palBar;
