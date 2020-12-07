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
	Copyright Sega16 (or whatever you wish to call me) (2012-2020)
*/
#include <FL/fl_ask.H>

#include "macros.h"
#include "class_global.h"
#include "color_convert.h"
#include "filemisc.h"
#include "undo.h"
#include "errorMsg.h"
#include "classpalettebar.h"
#include "classpalette.h"
#include "gui.h"
#include "palette.h"
#include "filereader.h"
void sortRowbyCB(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHavePal)) {
		currentProject->haveMessage(pjHavePal);
		return;
	}

	unsigned type = fl_choice("Sort each row by", "Hue", "Saturation", "Lightness");
	pushPaletteAll();
	sortBy(type, true);
	palBar.updateSlider(palBar.toTab(mode_editor));
	window->redraw();
}
void save_palette(Fl_Widget*, void*) {
	std::string the_file;

	if (!currentProject->containsData(pjHavePal)) {
		currentProject->haveMessage(pjHavePal);
		return;
	}

	char temp[4];
	snprintf(temp, 4, "%u", currentProject->pal->colorCnt - 1);
	char * returned = (char *)fl_input("Counting from zero enter the first entry that you want saved\nFor NES to save the sprite palette the first entry is 16", "0");

	if (!returned)
		return;

	if (!verify_str_number_only(returned))
		return;

	unsigned start = atoi(returned);
	returned = (char *)fl_input("Counting from zero enter the last entry that you want saved", temp);

	if (!returned)
		return;

	if (!verify_str_number_only(returned))
		return;

	unsigned end = atoi(returned) + 1;
	bool skipzero;

	if (currentProject->gameSystem == NES)
		skipzero = fl_ask("Would you like to skip saving color 0 for all rows except zero?");

	else
		skipzero = false;

	fileType_t type = askSaveType();
	int clipboard;

	if (type != fileType_t::tBinary) {
		clipboard = clipboardAsk();

		if (clipboard == 2)
			return;
	} else
		clipboard = 0;

	bool pickedFile;

	if (clipboard)
		pickedFile = true;
	else
		pickedFile = loadOrSaveFile(the_file, "Save palette", true);

	if (pickedFile)
		currentProject->pal->savePalette(the_file.c_str(), start, end, skipzero, type, clipboard);
}
void update_palette(Fl_Widget* o, void* v) {
	//first get the color and draw the box
	Fl_Slider* s = (Fl_Slider*)o;
	//now we need to update the entry we are editing
	unsigned selectedEntry = palBar.getEntry(palBar.toTab(mode_editor));

	if (pushed_g || (Fl::event() == FL_KEYDOWN)) {
		pushed_g = 0;
		pushPaletteEntry(selectedEntry);
	}

	currentProject->pal->changeIndexRaw((unsigned)s->value(), (unsigned)(uintptr_t)v, selectedEntry);

	if (mode_editor == tile_edit)
		currentProject->tileC->truecolor_to_tile(palBar.selRow[1], window->tile_select->value(), false); //update tile

	window->redraw();//update the palette
}
void loadPalette(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHavePal)) {
		currentProject->haveMessage(pjHavePal);
		return;
	}

	uint32_t file_size;
	int offset;
	char * inputTemp = (char *)fl_input("Counting from zero enter the first entry that you want the palette to start at.\nFor NES to load a sprite palette enter 16 or greater.", "0");

	if (!inputTemp)
		return;

	if (!verify_str_number_only(inputTemp))
		return;

	offset = atoi(inputTemp);

	if (offset < 0) {
		fl_alert("Offset must be greater than or equal to zero.");
		return;
	}

	currentProject->pal->loadFromFile(nullptr, fileType_t::tCancel, offset, CompressionType::Cancel);

}
void set_ditherAlg(Fl_Widget*, void* typeset) {
	if ((uintptr_t)typeset == 0)
		window->ditherPower->show();
	else
		window->ditherPower->hide();//imagine the user trying to change the power and nothing happening not fun at all

	currentProject->settings &= ~settingsDitherMask;
	currentProject->settings |= (uintptr_t)typeset & settingsDitherMask;
}
void set_tile_row(Fl_Widget*, void* row) {
	unsigned selrow = (uintptr_t)row;

	switch (mode_editor) {
		case tile_edit:
			palBar.changeRow(selrow, 1);
			currentProject->tileC->truecolor_to_tile(selrow, window->tile_select->value(), false);
			break;

		case tile_place:
			palBar.changeRow(selrow, 2);

			if (tileEditModePlace_G) {
				pushTilemapEdit(selTileE_G[0], selTileE_G[1]);
				currentProject->tms->maps[currentProject->curPlane].set_pal_row(selTileE_G[0], selTileE_G[1], selrow);
			}

			break;
	}

	window->redraw();//trigger a redraw so that the new row is displayed
}
void setPalType(Fl_Widget*, void*type) {
	unsigned palTab = palBar.toTab(mode_editor);
	currentProject->pal->palType[palBar.getEntry(palTab)] = (uintptr_t)type;
	palBar.updateSlider(palTab);
	window->redraw();
}
void pickNearAlg(Fl_Widget*, void*) {
	unsigned old = (currentProject->settings >> nearestColorShift)&nearestColorSettingsMask;
	currentProject->settings &= ~(nearestColorSettingsMask << nearestColorShift);
	currentProject->settings |= MenuPopup("Nearest color algorithm selection", "Select an algorithm", 4, old, "ciede2000", "Weighted", "Euclidean distance", "CIE76") << nearestColorShift;
}
static bool checkTileEditMode(void) {
	if (mode_editor != tile_edit) {
		fl_alert("Be in Tile editor to use this.");
		return false;
	} else
		return true;
}
void rgb_pal_to_entry(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHavePal)) {
		currentProject->haveMessage(pjHavePal);
		return;
	}

	//this function will convert a rgb value to the nearest palette entry
	if (currentProject->isFixedPalette())
		return;

	if (!checkTileEditMode())
		return;

	unsigned ent = palBar.getEntry(1);
	pushPaletteEntry(ent);
	currentProject->pal->rgbToEntry(window->rgb_red->value(), window->rgb_green->value(), window->rgb_blue->value(), ent);
	palBar.updateSlider(1);
	currentProject->tileC->truecolor_to_tile(palBar.selRow[1], window->tile_select->value(), false);
	window->redraw();
}
void entryToRgb(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHavePal)) {
		currentProject->haveMessage(pjHavePal);
		return;
	}

	if (!checkTileEditMode())
		return;

	unsigned en = palBar.getEntry(1) * 3;
	truecolor_temp[0] = currentProject->pal->rgbPal[en];
	truecolor_temp[1] = currentProject->pal->rgbPal[en + 1];
	truecolor_temp[2] = currentProject->pal->rgbPal[en + 2];
	window->rgb_red->value(truecolor_temp[0]);
	window->rgb_green->value(truecolor_temp[1]);
	window->rgb_blue->value(truecolor_temp[2]);
	window->redraw();
}

void clearPalette(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHavePal)) {
		currentProject->haveMessage(pjHavePal);
		return;
	}

	if (fl_ask("This will set all colors to 0 are you sure you want to do this?\nYou can undo this by pressing pressing CTRL+Z")) {
		pushPaletteAll();
		currentProject->pal->clear();
		window->damage(FL_DAMAGE_USER1);
		palBar.updateSliders();
	}
}

void updateYselection(Fl_Widget*, void* tab) {
	palBar.updateColorSelectionTile(window->getCurrentTileCurrentTab(), (uintptr_t)tab);
}

void setBGcolorTMS9918(Fl_Widget*sliderWidget, void*) {
	Fl_Slider * slider = (Fl_Slider*) sliderWidget;
	currentProject->setBGcolorTMS9918(slider->value());
}
