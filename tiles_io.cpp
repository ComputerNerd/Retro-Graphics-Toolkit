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
#include <cmath>

#include <FL/fl_ask.H>

#include "project.h"
#include "macros.h"
#include "filemisc.h"
#include "compressionWrapper.h"
#include "gui.h"
#include "class_global.h"
#include "filereader.h"
void save_tiles(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveTiles)) {
		currentProject->haveMessage(pjHaveTiles);
		return;
	}

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
		pickedFile = load_file_generic("Pick a location to save tiles", true);

	if (pickedFile) {
		CompressionType compression = compressionAsk();

		if (compression == CompressionType::Cancel)
			return;

		currentProject->tileC->save(the_file.c_str(), type, clipboard, compression);
	}
}
void load_truecolor_tiles(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveTiles)) {
		currentProject->haveMessage(pjHaveTiles);
		return;
	}

	//start by loading the file
	filereader f = filereader(boost::endian::order::native, 4, "Load truecolor tiles");

	if (!f.amt)
		return;

	unsigned i = f.selDat();
	size_t file_size = f.lens[i];
	unsigned tcTileSize = currentProject->tileC->tcSize;

	if (file_size % tcTileSize) {
		fl_alert("Error: this file is not a multiple of %d so it is not a valid truecolor tiles. The file size is: %d", tcTileSize, file_size);
		return;
	}

	size_t tileAmt = file_size / tcTileSize;

	currentProject->tileC->resizeAmt(tileAmt);

	memcpy(currentProject->tileC->truetDat.data(), f.dat[i], file_size);
	updateTileSelectAmt();

	if (window)
		window->redraw();
}
void save_tiles_truecolor(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveTiles)) {
		currentProject->haveMessage(pjHaveTiles);
		return;
	}

	if (load_file_generic("Save truecolor tiles", true)) {
		FILE * myfile;
		myfile = fopen(the_file.c_str(), "wb");

		if (myfile) {
			fwrite(currentProject->tileC->truetDat.data(), 1, (currentProject->tileC->amount())*currentProject->tileC->tcSize, myfile);
			fclose(myfile);
		} else
			fl_alert("Error: can not save file %s", the_file.c_str());
	}
}
