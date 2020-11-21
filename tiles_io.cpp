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
	Copyright Sega16 (or whatever you wish to call me) (2012-2018)
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
void load_tiles(Fl_Widget*, void*o) {
	//if o=0 load if o=1 append if o=2 load at
	if (!currentProject->containsData(pjHaveTiles)) {
		currentProject->haveMessage(pjHaveTiles);
		return;
	}

	size_t file_size;
	unsigned mode = (uintptr_t)o;
	char * returned = (char*)fl_input("What row should these tiles use?\nEnter 0 to 3 to selected a row or -1 to -4 to auto determine based on tilemap\nWhen specifing a negative number to figure out what the default will be use this formula abs(row)-1", "-1");

	if (unlikely(!returned))
		return;

	if (unlikely(!verify_str_number_only(returned)))
		return;

	int row = atoi(returned);

	if (unlikely((row > 3) || (row < -4))) {
		fl_alert("You entered %d which is out of range it must be in range of -4 to 3", row);
		return;
	}

	unsigned defaultRow = row >= 0 ? row : std::abs(row) - 1;
	bool alphaZero = fl_ask("Set color #0 to alpha 0 instead of 255") ? true : false;
	filereader f = filereader(boost::endian::order::native, 1, "Load tiles");

	if (f.amt == 0)
		return;

	unsigned idx = f.selDat();
	file_size = f.lens[idx];
	unsigned truecolor_multiplier;
	truecolor_multiplier = 256 / currentProject->tileC->tileSize;

	if (file_size % currentProject->tileC->tileSize) {
		fl_alert("Error: This is not a valid tile file each tile is %d bytes and this file is not a multiple of %d so it is not a valid tile file.", currentProject->tileC->tileSize, currentProject->tileC->tileSize);
		return;
	}

	uint32_t offset_tiles;
	uint32_t offset_tiles_bytes;

	if (mode == 2) {
		const char * str = fl_input("Counting from zero which tile should this start at?");

		if (!str)
			return;

		if (!verify_str_number_only((char*)str))
			return;

		int off = atoi(str);

		if (off >= 0) {
			offset_tiles = off;
			offset_tiles_bytes = offset_tiles * currentProject->tileC->tileSize;
		} else {
			fl_alert("You must enter a number greater than or equal to zero.");
			return;
		}
	} else if (mode == 1) {
		offset_tiles = currentProject->tileC->amount();
		offset_tiles_bytes = offset_tiles * currentProject->tileC->tileSize;
	} else {
		offset_tiles = 0;
		offset_tiles_bytes = 0;
	}

	if (mode == 2) {
		if (offset_tiles + (file_size / currentProject->tileC->tileSize) >= currentProject->tileC->amount())
			currentProject->tileC->tDat.resize(offset_tiles_bytes + file_size);
	} else
		currentProject->tileC->tDat.resize(offset_tiles_bytes + file_size);

	memcpy(currentProject->tileC->tDat.data() + offset_tiles_bytes, f.dat[idx], file_size);

	if (currentProject->getTileType() != PLANAR_TILE)
		currentProject->tileC->toPlanar(currentProject->getTileType(), offset_tiles, offset_tiles + (file_size / currentProject->tileC->tileSize));

	if (mode == 2) {
		if (offset_tiles + (file_size / currentProject->tileC->tileSize) >= currentProject->tileC->amount())
			currentProject->tileC->truetDat.resize((file_size * truecolor_multiplier) + (offset_tiles_bytes * truecolor_multiplier));
	} else
		currentProject->tileC->truetDat.resize((file_size * truecolor_multiplier) + (offset_tiles_bytes * truecolor_multiplier));

	for (uint32_t c = offset_tiles; c < (file_size / currentProject->tileC->tileSize) + offset_tiles; c++) {
		if (row < 0) {
			uint32_t x, y;
			uint8_t foundRow = defaultRow;

			for (y = 0; y < currentProject->tms->maps[currentProject->curPlane].mapSizeHA; ++y) {
				for (x = 0; x < currentProject->tms->maps[currentProject->curPlane].mapSizeW; ++x) {
					if (currentProject->tms->maps[currentProject->curPlane].get_tile(x, y) == c) {
						foundRow = currentProject->tms->maps[currentProject->curPlane].getPalRow(x, y);
						goto doTile;
					}
				}
			}

doTile:
			currentProject->tileC->tileToTrueCol(&currentProject->tileC->tDat[(c * currentProject->tileC->tileSize)], &currentProject->tileC->truetDat[(c * 256)], foundRow, true, alphaZero);
		} else
			currentProject->tileC->tileToTrueCol(&currentProject->tileC->tDat[(c * currentProject->tileC->tileSize)], &currentProject->tileC->truetDat[(c * 256)], defaultRow, true, alphaZero);
	}

	updateTileSelectAmt();
	window->tile_select->value(0);
	window->tile_select_2->value(0);
	window->redraw();
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
