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
#include <FL/fl_ask.H>

#include <stdexcept>

#include "project.h"
#include "classpalettebar.h"
#include "color_convert.h"
#include "callbacks_palette.h"
#include "class_global.h"
#include "gui.h"
#include "errorMsg.h"
static const char*namesGen[] = {"Red", "Green", "Blue"};
static const char*namesNES[] = {"Hue", "Value", "Emphasis"};
paletteBar palBar;
void paletteBar::addTab(unsigned tab, bool all, bool tiny, bool alt) {
	this->all[tab] = all;
	this->tiny[tab] = tiny;
	this->alt[tab] = alt;
	unsigned offsetx = 16;
	unsigned offsety = tiny ? 54 : 56;
	ox[tab] = baseOffx[tab] = offsetx;
	oy[tab] = baseOffy[tab] = offsety;
	sysCache = currentProject->gameSystem;
	offsety += tiny ? 6 : 8;

	for (unsigned i = 0; i < 3; ++i) {
		slide[tab][i] = new Fl_Hor_Value_Slider(offsetx + 32, offsety + ((all ? 4 : 1) * (tiny ? 26 : 32)), tiny ? 128 : 256, tiny ? 22 : 24, namesGen[i]);
		slide[tab][i]->minimum(0);
		slide[tab][i]->maximum(7);
		slide[tab][i]->step(1);
		slide[tab][i]->value(0);
		slide[tab][i]->align(FL_ALIGN_LEFT);
		slide[tab][i]->callback(update_palette, (void*)(uintptr_t)i);
		offsety += tiny ? 26 : 32;
	}
}
unsigned paletteBar::getEntry(unsigned tab)const {
	return currentProject->pal->getIndexByRow(selRow[tab], selBox[tab], alt[tab] && currentProject->pal->haveAlt);
}

void paletteBar::setSys(bool upSlide, bool force) {
	if (force || sysCache != currentProject->gameSystem) {
		currentProject->pal->setVars(currentProject->gameSystem);

		if (window) {
			for (unsigned i = 0; i < TABS_WITH_ROW_BUTTONS * MAX_ROWS_PALETTE; i += MAX_ROWS_PALETTE) {
				for (unsigned j = 0; j < currentProject->pal->rowCntPal; ++j)
					window->palRTE[i + j]->show();

				for (unsigned j = currentProject->pal->rowCntPal; j < MAX_ROWS_PALETTE; ++j)
					window->palRTE[i + j]->hide();
			}

			for (unsigned j = 0; j < TABS_WITH_PALETTE; ++j) {
				for (unsigned i = 0; i < 3; ++i) {
					if (!currentProject->isFixedPalette())
						slide[j][i]->show();

					slide[j][i]->callback(update_palette, (void*)i);

					switch (currentProject->gameSystem) {
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

						case TMS9918: // No action needed.
							break;

						default:
							show_default_error
					}
				}

				switch (currentProject->gameSystem) {
					case segaGenesis:
					case masterSystem:
					case gameGear:
						if (sysCache == NES) {
							slide[j][1]->labelsize(13);
							slide[j][2]->labelsize(14);
							slide[j][2]->resize(slide[j][2]->x() - 16, slide[j][2]->y(), slide[j][2]->w() + 16, slide[j][2]->h());
						}

						break;

					case NES:
						slide[j][0]->maximum(15);
						slide[j][1]->maximum(3);
						slide[j][1]->labelsize(14);
						slide[j][2]->labelsize(12);
						slide[j][2]->value(0);
						slide[j][2]->maximum(7);
						slide[j][2]->resize(slide[j][2]->x() + 16, slide[j][2]->y(), slide[j][2]->w() - 16, slide[j][2]->h());
						slide[j][2]->callback(updateEmphesisCB);
						break;

					case TMS9918:
					{
						TMS9918SubSys subSys = currentProject->getTMS9918subSys();

						if (subSys == MODE_0) {
							memset(currentProject->pal->rgbPal, 0, 3);
							slide[j][0]->hide();
						} else {
							slide[j][0]->label("BG col");
							slide[j][0]->maximum(15);
							slide[j][0]->value(currentProject->getPalColTMS9918() & 15);
							slide[j][0]->show();
							slide[j][0]->callback(setBGcolorTMS9918);
						}

						if (subSys == MODE_2) {
							slide[j][1]->label("Y");
							slide[j][1]->maximum(7);
							slide[j][1]->show();
							slide[j][1]->callback(updateYselection, (void*)j);
						} else
							slide[j][1]->hide();

						slide[j][1]->value(0);
						slide[j][2]->hide();
					}
					break;

					default:
						show_default_error
				}

				selBox[j] %= currentProject->pal->perRow;

				if (alt[j] && currentProject->pal->haveAlt) {
					if (selRow[j] >= currentProject->pal->rowCntPalalt)
						selRow[j] = currentProject->pal->rowCntPalalt - 1;
				} else {
					if (selRow[j] >= currentProject->pal->rowCntPal)
						selRow[j] = currentProject->pal->rowCntPal - 1;
				}
			}
		}

		sysCache = currentProject->gameSystem;
	} else
		puts("Warning: syscache is same as gameSystem");

	if (upSlide && window)
		updateSliders();
}
void paletteBar::updateSize(unsigned tab) {
	if (window) {
		ox[tab] = (float)((float)window->w() / 800.f) * (float)baseOffx[tab];
		oy[tab] = (float)((float)window->h() / 600.f) * (float)baseOffy[tab];
	}
}
void paletteBar::updateSlider(unsigned tab) {
	if (currentProject->isFixedPalette() || (!window))
		return;

	unsigned idx = getEntry(tab);

	if (currentProject->pal->palType[idx]) {
		for (unsigned i = 0; i < 3; ++i)
			slide[tab][i]->hide();
	} else {
		for (unsigned i = 0; i < 3; ++i)
			slide[tab][i]->show();

		switch (currentProject->gameSystem) {
			case segaGenesis:
			{
				const uint16_t*palDatPtr = (uint16_t*)currentProject->pal->palDat + idx;
				const uint16_t palDat = *palDatPtr;
				slide[tab][2]->value((palDat >> 9) & 7);
				slide[tab][1]->value((palDat >> 5) & 7);
				slide[tab][0]->value((palDat >> 1) & 7);
			}
			break;

			case NES:
				slide[tab][0]->value(currentProject->pal->palDat[idx] & 15);
				slide[tab][1]->value((currentProject->pal->palDat[idx] >> 4) & 3);
				break;

			case masterSystem:
				slide[tab][0]->value(currentProject->pal->palDat[idx] & 3);
				slide[tab][1]->value((currentProject->pal->palDat[idx] >> 2) & 3);
				slide[tab][2]->value((currentProject->pal->palDat[idx] >> 4) & 3);
				break;

			case gameGear:
			{
				const uint16_t*palDatPtr = (uint16_t*)currentProject->pal->palDat + idx;
				const uint16_t palDat = *palDatPtr;
				slide[tab][0]->value(palDat & 15);
				slide[tab][1]->value((palDat >> 4) & 15);
				slide[tab][2]->value((palDat >> 8) & 15);
			}
			break;

			default:
				show_default_error
		}
	}

	window->palType[currentProject->pal->palType[getEntry(0)]]->setonly();
	window->palType[currentProject->pal->palType[getEntry(1)] + 3]->setonly();
	window->palType[currentProject->pal->palType[getEntry(2)] + 6]->setonly();
	window->palType[currentProject->pal->palType[getEntry(3)] + 9]->setonly();
}
void paletteBar::drawBoxes(unsigned tab) {
	unsigned box_size = window->pal_size->value();
	unsigned x, y, a;
	const uint8_t*rgbPtr = currentProject->pal->rgbPal;

	a = currentProject->pal->perRow * 3;

	if (all[tab]) {
		unsigned loc_x, loc_y;
		loc_x = (float)((float)window->w() / 800.f) * (float)palette_preview_box_x;
		loc_y = (float)((float)window->h() / 600.f) * (float)palette_preview_box_y;
		fl_rectf(loc_x, loc_y, box_size * 4, box_size * 4, currentProject->pal->rgbPal[(selBox[tab] * 3) + (selRow[tab]*a)], currentProject->pal->rgbPal[(selBox[tab] * 3) + (selRow[tab]*a) + 1], currentProject->pal->rgbPal[(selBox[tab] * 3) + (selRow[tab]*a) + 2]); //this will show larger preview of current color

		if (alt[tab] && (currentProject->gameSystem == NES))
			rgbPtr += currentProject->pal->colorCnt * 3;

		for (y = 0; y < currentProject->pal->rowCntPal; ++y) {
			for (x = 0; x < currentProject->pal->perRow; ++x) {
				fl_rectf(ox[tab] + (x * box_size), oy[tab] + (y * box_size), box_size, box_size, *rgbPtr, *(rgbPtr + 1), *(rgbPtr + 2));
				rgbPtr += 3;
			}
		}

		if (currentProject->pal->haveAlt) {
			for (y = 0; y < currentProject->pal->rowCntPalalt; ++y) {
				for (x = 0; x < currentProject->pal->perRowalt; ++x) {
					fl_rectf(ox[tab] + (x * box_size) + box_size + (currentProject->pal->perRow * box_size), oy[tab] + (y * box_size), box_size, box_size, *rgbPtr, *(rgbPtr + 1), *(rgbPtr + 2));
					rgbPtr += 3;
				}
			}
		}

		fl_draw_box(FL_EMBOSSED_FRAME, selBox[tab]*box_size + ox[tab], selRow[tab]*box_size + oy[tab], box_size, box_size, 0);
	} else {
		rgbPtr += a * selRow[tab];

		if (alt[tab] && (currentProject->gameSystem == NES))
			rgbPtr += currentProject->pal->colorCnt * 3;

		for (x = 0; x < currentProject->pal->perRow; ++x) {
			fl_rectf(ox[tab] + (x * box_size), oy[tab], box_size, box_size, *rgbPtr, *(rgbPtr + 1), *(rgbPtr + 2));
			rgbPtr += 3;
		}

		fl_draw_box(FL_EMBOSSED_FRAME, selBox[tab]*box_size + ox[tab], oy[tab], box_size, box_size, 0);

		if (hasAltSelection())
			fl_draw_box(FL_DOWN_FRAME, selBoxAlt[tab]*box_size + ox[tab], oy[tab], box_size, box_size, 0);
	}
}
unsigned paletteBar::toTab(unsigned realtab) {
	static const int tabLut[] = {0, 1, 2, -1, 3, -1, -1};

	if (tabLut[realtab] < 0) {
		fl_alert("Invalid tab using palette editor");
		return 0;
	}

	return tabLut[realtab];
}
void paletteBar::checkBox(int x, int y, unsigned tab) {
	/*!
	This function is in charge of seeing if the mouse click is on a box and what box it is
	for x and y pass the mouser coordinate
	*/
	unsigned boxSize = window->pal_size->value();
	x -= ox[tab];
	y -= oy[tab];

	if (x < 0)
		return;

	if (y < 0)
		return;

	x /= boxSize;

	if (x >= currentProject->pal->perRow)
		return;

	y /= boxSize;

	if (y >= (all[tab] ? currentProject->pal->rowCntPal : 1))
		return;

	if (tab != 0 && hasAltSelection() && Fl::event_button() == FL_RIGHT_MOUSE)
		selBoxAlt[tab] = x; // Background color
	else
		selBox[tab] = x;

	if (hasAltSelection() && tab != 0) {
		unsigned extAttrTmp = selBox[tab] << 4;
		extAttrTmp |= selBoxAlt[tab];

		if (currentProject->tileC) {
			currentProject->tileC->setExtAttr(currentProject->tileC->current_tile, slide[tab][1]->value(), extAttrTmp);
			window->damage(FL_DAMAGE_USER1);
		}
	}

	if (all[tab])
		changeRow(y, tab);
	else
		updateSlider(tab);

	window->redraw();
}

bool paletteBar::hasAltSelection() {
	TMS9918SubSys subSys = currentProject->getTMS9918subSys();
	return currentProject->gameSystem == TMS9918 && (subSys != MODE_3);
}

void paletteBar::updateColorSelectionTile(unsigned tile, unsigned tab) {
	if (hasAltSelection()) {
		if (currentProject->tileC) {
			uint8_t ent = currentProject->tileC->getExtAttr(tile, slide[tab][1]->value());
			selBox[tab] = ent >> 4;
			selBoxAlt[tab] = ent & 15;

			if (window)
				window->damage(FL_DAMAGE_USER1);
		}
	}
}
