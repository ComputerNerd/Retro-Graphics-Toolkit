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

#include "macros.h"
#include "savepng.h"
#include "dither.h"
#include "callback_chunk.h"
#include "callbacksprites.h"
#include "undo.h"
#include "image.h"
#include "classpalettebar.h"
#include "class_global.h"
#include "gui.h"
#include "palette.h"
#include "errorMsg.h"
void pickExtAttrsCB(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveMap)) {
		currentProject->haveMessage(pjHaveMap);
		return;
	}

	currentProject->tms->maps[currentProject->curPlane].pickExtAttrs();
	window->damage(FL_DAMAGE_USER1);
}
void setTmapOffsetCB(Fl_Widget*o, void*) {
	Fl_Int_Input*i = (Fl_Int_Input*)o;
	currentProject->tms->maps[currentProject->curPlane].offset = atoi(i->value());
}
void tileDPicker(Fl_Widget*, void*) {
	Fl_Window *win;
	Fl_Progress *progress;
	mkProgress(&win, &progress);
	currentProject->tms->maps[currentProject->curPlane].pickRowDelta(true, progress);
	win->remove(progress);// remove progress bar from window
	delete (progress); // deallocate it
	delete win;
	window->damage(FL_DAMAGE_USER1);
}
void resizeBlocksCB(Fl_Widget*o, void*) {
	int32_t wtmp, htmp;
	wtmp = SafeTxtInput(window->map_w);
	htmp = SafeTxtInput(window->map_h);
	currentProject->tms->maps[currentProject->curPlane].resizeBlocks(wtmp, htmp);
	window->redraw();
}
void blocksAmtCB(Fl_Widget*o, void*) {
	Fl_Int_Input*i = (Fl_Int_Input*)o;
	int amtTmp = SafeTxtInput(i);
	pushTilemapBlocksAmt(amtTmp);
	currentProject->tms->maps[currentProject->curPlane].blockAmt(amtTmp);
	window->redraw();
}
void toggleBlocksCB(Fl_Widget*o, void*) {
	Fl_Check_Button* b = (Fl_Check_Button*)o;
	bool Toggle = b->value() ? true : false;
	currentProject->tms->maps[currentProject->curPlane].toggleBlocks(Toggle);

	if (!Toggle) {
		currentProject->Chunk->useBlocks = false;
		window->useBlocksChunkCBtn->value(0);
	}

	window->redraw();
}
void FixOutOfRangeCB(Fl_Widget*, void*) {
	//use current attributes
	pushTilemapAll(false);

	for (int y = 0; y < currentProject->tms->maps[currentProject->curPlane].mapSizeHA; ++y) {
		for (int x = 0; x < currentProject->tms->maps[currentProject->curPlane].mapSizeW; ++x) {
			if (currentProject->tms->maps[currentProject->curPlane].get_tile(x, y) >= currentProject->tileC->amount())
				currentProject->tms->maps[currentProject->curPlane].set_tile_full(x, y, window->getCurrentTileCurrentTab(), palBar.selRow[2], G_hflip[0], G_vflip[0], G_highlow_p[0]);
		}
	}

	window->damage(FL_DAMAGE_USER1);
}
void callback_resize_map(Fl_Widget* o, void*) {
	int32_t w, h;
	w = SafeTxtInput(window->map_w);
	h = SafeTxtInput(window->map_h);
	pushTilemapResize(w, h);
	currentProject->tms->maps[currentProject->curPlane].resize_tile_map(w, h);
	window->redraw();
}
void set_grid(Fl_Widget*, void*) {
	/*this function will only be trigger when the check button is pressed
	so we just need to invert the bool using xor to avoid if statements*/
	show_grid ^= true;
	window->redraw();//redraw to reflect the updated statues of the grid
}
void set_grid_placer(Fl_Widget*, void*) {
	show_grid_placer ^= true;
	window->redraw();//redraw to reflect the updated statues of the grid
}

void save_tilemap_as_image(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveMap | pjHaveTiles)) {
		currentProject->haveMessage(pjHaveMap | pjHaveTiles);
		return;
	}

	std::string the_file;

	if (currentProject->containsData(pjHaveTiles | pjHaveMap)) {
		if (loadOrSaveFile(the_file, "Save PNG as", true)) {
			uint32_t w = currentProject->tms->maps[currentProject->curPlane].mapSizeW * currentProject->tileC->width();
			uint32_t h = currentProject->tms->maps[currentProject->curPlane].mapSizeHA * currentProject->tileC->height();
			uint8_t * image = (uint8_t*)malloc(w * h);
			uint8_t * imageold = image;

			if (!image)
				show_malloc_error(w * h)
				for (unsigned y = 0; y < h; y += currentProject->tileC->height()) {
					for (unsigned x = 0; x < w; x += currentProject->tileC->width()) {
						unsigned tCur = currentProject->tms->maps[currentProject->curPlane].get_tile(x / currentProject->tileC->width(), y / currentProject->tileC->height());
						unsigned off = currentProject->tms->maps[currentProject->curPlane].getPalRow(x / currentProject->tileC->width(), y / currentProject->tileC->height()) * currentProject->pal->perRow;

						for (unsigned yy = 0; yy < currentProject->tileC->height(); ++yy) {
							for (unsigned xx = 0; xx < currentProject->tileC->width(); ++xx)
								image[x + xx + ((y + yy)*w)] = currentProject->tileC->getPixel(tCur, xx, yy) + off;
						}
					}
				}

			savePNG(the_file.c_str(), w, h, (void*)imageold, currentProject->pal->rgbPal, currentProject->pal->colorCnt);
			free(imageold);
		}
	}
}
void save_tilemap_as_colspace(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveMap | pjHaveTiles)) {
		currentProject->haveMessage(pjHaveMap | pjHaveTiles);
		return;
	}

	std::string the_file;

	if (loadOrSaveFile(the_file, "Save png as", true) == true) {
		uint32_t w = currentProject->tms->maps[currentProject->curPlane].mapSizeW * 8;
		uint32_t h = currentProject->tms->maps[currentProject->curPlane].mapSizeHA * 8;
		uint8_t * image = (uint8_t*)malloc(w * h * 3);
		currentProject->tms->maps[currentProject->curPlane].truecolor_to_image(image, -1, false);
		ditherImage(image, w, h, false, true);
		savePNG(the_file.c_str(), w, h, (void*)image);
		free(image);
	}
}
void load_tile_map(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveMap)) {
		currentProject->haveMessage(pjHaveMap);
		return;
	}

	pushTilemapAll(false);

	if (unlikely(!currentProject->tms->maps[currentProject->curPlane].loadFromFile()))
		alertWrap("Error: Cannot load file");
}
void save_map(Fl_Widget*, void*) {
	if (!currentProject->containsData(pjHaveMap)) {
		currentProject->haveMessage(pjHaveMap);
		return;
	}

	if (unlikely(!currentProject->tms->maps[currentProject->curPlane].saveToFile()))
		alertWrap("Error: can not save file\nTry making sure that you have permission to save the file here");
}
void fill_tile_map_with_tile(Fl_Widget*, void*) {
	pushTilemapAll(false);

	if (mode_editor != tile_place) {
		alertWrap("To prevent accidental modification to the tile map be in plane editing mode");
		return;
	}

	if (fl_ask("This will erase the entire tilemap and fill it with the currently selected tile\nAre you sure you want to do this?")) {
		for (uint32_t y = 0; y < currentProject->tms->maps[currentProject->curPlane].mapSizeHA; ++y) {
			for (uint32_t x = 0; x < currentProject->tms->maps[currentProject->curPlane].mapSizeW; ++x)
				currentProject->tms->maps[currentProject->curPlane].set_tile_full(x, y, window->getCurrentTileCurrentTab(), palBar.selRow[2], G_hflip[0], G_vflip[0], G_highlow_p[0]);
		}

		window->damage(FL_DAMAGE_USER1);
	}
}
void dither_tilemap_as_imageCB(Fl_Widget*, void*) {
	//normally this program dithers all tiles individually this is not always desirable
	//to fix this I created this function It converts the tilemap to image and dithers all tiles
	//so first create ram for image
	unsigned method;
	method = fl_choice("How would you like this tilemap dithered?", "Dither each palette row separately", "Dither entire image at once", "Cancel");

	if (method == 2)
		return;

	pushTilesAll(tTypeTile);
	currentProject->tms->maps[currentProject->curPlane].ditherAsImage(method);
	Fl::check();
	window->redraw();
}
void load_image_to_tilemap_project_ptr(struct Project* cProject, const char*fname, bool over, bool tilesonly, bool append, unsigned curPlane, bool undo) {
	// TODO: undo will only work correctly when cProject == currentProject
	Fl_Shared_Image*loaded_image = Fl_Shared_Image::get(fname);

	if (!loaded_image) {
		fl_alert("Error loading image");
		return;
	}

	unsigned tilebitw, tilebith;
	tilebitw = cProject->tileC->width();
	tilebith = cProject->tileC->height();

	if ((cProject->subSystem & NES2x2) && (cProject->gameSystem == NES)) {
		tilebitw *= 2;
		tilebith *= 2;
	}

	uint32_t w, h;
	w = loaded_image->w();
	h = loaded_image->h();
	uint32_t w8, h8;
	uint32_t wt, ht;
	int wr, hr;
	wr = w % tilebitw;
	hr = h % tilebith;
	w8 = w / cProject->tileC->width();
	h8 = h / cProject->tileC->height();

	if (wr)
		++w8;

	if (hr)
		++h8;

	if ((cProject->gameSystem == NES) && (cProject->subSystem & NES2x2)) {
		if (w8 & 1)
			++w8;

		if (h8 & 1)
			++h8;
	}

	if (over) {
		if ((w8 != cProject->tms->maps[curPlane].mapSizeW) || (h8 != cProject->tms->maps[curPlane].mapSizeHA)) {
			alertWrap("When importing over tilemap width and height must be the same");
			loaded_image->release();
			return;
		}
	}

	wt = w8 * cProject->tileC->width();
	ht = h8 * cProject->tileC->height();

	if (wr || hr)
		messageWrap("When width and/or height is not a multiple of %d,%d the image will be centered.\nThe width of this image is %d and the height is %d", tilebitw, tilebith, w, h);

	//start by copying the data
	uint8_t * imgptr = (uint8_t *)loaded_image->data()[0];
	//now we can convert to tiles
	unsigned depth = loaded_image->d();
	printf("image width: %u image height: %u depth: %u\n", w, h, depth);

	if (unlikely(depth != 3 && depth != 4 && depth != 1)) {
		alertWrap("Please use color depth of 1,3 or 4\nYou Used %d", loaded_image->d());
		loaded_image->release();
		return;
	} else
		printf("Image depth %d\n", loaded_image->d());

	if (undo)
		pushTilesAll(tTypeBoth);

	unsigned appendoff;

	if (!over) {
		if (append)
			appendoff = cProject->tileC->amount();
		else
			appendoff = 0;

		cProject->tileC->resizeAmt(w8 * h8 + appendoff);
		updateTileSelectAmt();
	} else
		appendoff = 0;

	unsigned center[3];
	center[0] = (wt - w) / 2;
	center[1] = (ht - h) / 2;
	center[2] = wt - w - center[0];
	uint8_t*palMap;
	bool grayscale;
	unsigned remap[256];

	if (depth == 1) {
		unsigned numcol;
		grayscale = handle1byteImg(loaded_image, remap, &numcol);

		if (!grayscale) {
			palMap = (uint8_t*)loaded_image->data()[1];
			imgptr = (uint8_t*)loaded_image->data()[2];

			if (fl_ask("Overwrite palette? This can be undone if you change your mind.")) {
				if (undo)
					pushPaletteAll();

				for (unsigned i = 0; i < std::min(cProject->pal->colorCnt + cProject->pal->colorCntalt, numcol); ++i)
					cProject->pal->rgbToEntry(palMap[i * 4 + 1], palMap[i * 4 + 2], palMap[i * 4 + 3], i);
			}
		}
	}

	for (uint32_t y = 0, tcnt = 0; y < ht; ++y) {
		if (y % cProject->tileC->height())
			tcnt -= wt / cProject->tileC->width();

		if ((!((y < center[1]) || (y >= (h + center[1])))) && (depth == 1) && (!grayscale))
			imgptr = (uint8_t*)loaded_image->data()[y + 2 - center[1]];

		for (uint32_t x = 0; x < wt; x += cProject->tileC->width(), ++tcnt) {
			uint32_t ctile;

			if (over) {
				ctile = cProject->tms->maps[curPlane].get_tile(x / cProject->tileC->width(), y / cProject->tileC->height());

				//See if ctile is allocated
				if (ctile >= cProject->tileC->amount()) {
					//tile on map but not a tile associated with it
					imgptr += cProject->tileC->width() * depth;
					continue;
				}
			} else
				ctile = tcnt;

			ctile += appendoff;
			uint8_t*ttile = cProject->tileC->truetDat.data() + ((ctile * cProject->tileC->tcSize) + ((y % cProject->tileC->height()) * cProject->tileC->width() * 4));
			//First take care of border
			unsigned line = cProject->tileC->width();

			if ((y < center[1]) || (y >= (h + center[1])))
				memset(ttile, 0, line * 4);
			else {
				if (x < center[0]) {
					memset(ttile, 0, center[0] * 4);
					line -= center[0];
					ttile += center[0] * 4;
				} else if (x >= (wt - cProject->tileC->width()))
					line -= center[2];

				switch (depth) {
					case 1:
						for (unsigned xx = 0; xx < line; ++xx) {
							if (grayscale) {
								*ttile++ = *imgptr;
								*ttile++ = *imgptr;
								*ttile++ = *imgptr++;
								*ttile++ = 255;
							} else {
								if (*imgptr == ' ') {
									memset(ttile, 0, 4);
									ttile += 4;
									++imgptr;
								} else {
									unsigned p = (*imgptr++);
									*ttile++ = palMap[remap[p] + 1];
									*ttile++ = palMap[remap[p] + 2];
									*ttile++ = palMap[remap[p] + 3];
									*ttile++ = 255;
								}
							}
						}

						break;

					case 3:
						for (unsigned xx = 0; xx < line; ++xx) {
							*ttile++ = *imgptr++;
							*ttile++ = *imgptr++;
							*ttile++ = *imgptr++;
							*ttile++ = 255;
						}

						break;

					case 4:
						memcpy(ttile, imgptr, line * 4);
						imgptr += line * 4;
						ttile += line * 4;
						break;
				}

				if (x >= (wt - cProject->tileC->width()))
					memset(ttile, 0, center[2] * 4);
			}
		}
	}

	loaded_image->release();

	if ((!over) && (!tilesonly)) {
		if (undo)
			pushTilemapAll(false);

		cProject->tms->maps[curPlane].resize_tile_map(w8, h8);
		uint32_t tilecounter = appendoff;

		for (uint32_t y = 0; y < h8; ++y) {
			for (uint32_t x = 0; x < w8; ++x) {
				cProject->tms->maps[curPlane].set_tile_full(x, y, tilecounter, 0, false, false, false);
				++tilecounter;
			}
		}
	}

	if (window)
		window->redraw();
}
void load_image_to_tilemap(const char*fname, bool over, bool tilesonly, bool append) {
	load_image_to_tilemap_project_ptr(currentProject, fname, over, tilesonly, append, currentProject->curPlane, true);
}
void load_image_to_tilemapCB(Fl_Widget*, void*o) {
	if (!currentProject->containsData(pjHaveTiles | pjHaveMap)) {
		currentProject->haveMessage(pjHaveTiles | pjHaveMap);
		return;
	}

	bool over = (uintptr_t)o & 1;
	bool tilesonly = (uintptr_t)o >> 1;
	bool append;

	if (over)
		append = false;
	else
		append = fl_choice("Append tiles or overwrite starting at 0?", "Overwrite", "Append", 0);

	std::string fname;

	if (loadOrSaveFile(fname, "Select an image"))
		load_image_to_tilemap(fname.c_str(), over, tilesonly, append);
}
void set_prioCB(Fl_Widget*, void*o) {
	unsigned off = (uintptr_t)o;
	G_highlow_p[off] ^= true;

	if ((tileEditModeChunk_G) && (off == 1)) {
		pushChunkEdit(currentChunk, editChunk_G[0], editChunk_G[1]);
		currentProject->Chunk->setPrio(currentChunk, editChunk_G[0], editChunk_G[1], G_highlow_p[off]);
	} else if (tileEditModePlace_G && (off == 0)) {
		pushTilemapEdit(selTileE_G[0], selTileE_G[1]);
		currentProject->tms->maps[currentProject->curPlane].set_prio(selTileE_G[0], selTileE_G[1], G_highlow_p[off]);
	}

	window->redraw();
}
void set_hflipCB(Fl_Widget*, void*o) {
	unsigned off = (uintptr_t)o;
	G_hflip[off] ^= true;

	if ((tileEditModeChunk_G) && (off == 1)) {
		pushChunkEdit(currentChunk, editChunk_G[0], editChunk_G[1]);
		currentProject->Chunk->setHflip(currentChunk, editChunk_G[0], editChunk_G[1], G_hflip[off]);
	} else if (tileEditModePlace_G && (off == 0)) {
		pushTilemapEdit(selTileE_G[0], selTileE_G[1]);
		currentProject->tms->maps[currentProject->curPlane].set_hflip(selTileE_G[0], selTileE_G[1], G_hflip[off]);
	}

	window->redraw();
}
void set_vflipCB(Fl_Widget*, void*o) {
	unsigned off = (uintptr_t)o;
	G_vflip[off] ^= true;

	if ((tileEditModeChunk_G) && (off == 1)) {
		pushChunkEdit(currentChunk, editChunk_G[0], editChunk_G[1]);
		currentProject->Chunk->setVflip(currentChunk, editChunk_G[0], editChunk_G[1], G_vflip[off]);
	} else if (tileEditModePlace_G && (off == 0)) {
		pushTilemapEdit(selTileE_G[0], selTileE_G[1]);
		currentProject->tms->maps[currentProject->curPlane].set_vflip(selTileE_G[0], selTileE_G[1], G_vflip[off]);
	}

	window->redraw();
}
void update_map_scroll_x(Fl_Widget*, void*) {
	map_scroll_pos_x = window->map_x_scroll->value();
	window->redraw();
}
void update_map_scroll_y(Fl_Widget*, void*) {
	map_scroll_pos_y = window->map_y_scroll->value();
	window->redraw();
}
void update_map_size(Fl_Widget*, void*) {
	currentProject->tms->maps[currentProject->curPlane].ScrollUpdate();
	window->redraw();
}
void tilemap_remove_callback(Fl_Widget*, void*) {
	char * str_ptr;
	str_ptr = (char *)fl_input("Enter Tile");

	if (!str_ptr)
		return;

	if (!verify_str_number_only(str_ptr))
		return;

	int32_t tile = atoi(str_ptr);

	if (unlikely(tile < 0)) {
		fl_alert("You must enter a number greater than or equal to 0 however you entered %d\n", tile);
		return;
	}

	if (tile)
		currentProject->tms->maps[currentProject->curPlane].sub_tile_map(tile, tile - 1, false, false);
	else
		currentProject->tms->maps[currentProject->curPlane].sub_tile_map(0, 0, false, false);

	window->damage(FL_DAMAGE_USER1);
}
void shadow_highligh_findout(Fl_Widget*, void*) {
	if (unlikely(currentProject->gameSystem != segaGenesis)) {
		fl_alert("Only the Sega Genesis/Mega Drive supports shadow highlight mode\n");
		return;
	}

	unsigned type = fl_choice("How will it be determined if the tile is shadowed or not?", "Tile brightness", "Delta", 0);
	//this function will see if 3 or less pixels are above 125 and if so set priority to low or set priority to high if bright tile
	unsigned x, y;
	uint32_t xx;

	if (type == 0) {
		for (y = 0; y < currentProject->tms->maps[currentProject->curPlane].mapSizeHA; ++y) {
			for (x = 0; x < currentProject->tms->maps[currentProject->curPlane].mapSizeW; ++x) {
				uint32_t cur_tile = currentProject->tms->maps[currentProject->curPlane].get_tile(x, y);
				uint8_t over = 0;

				for (xx = cur_tile * 256; xx < cur_tile * 256 + 256; xx += 4) {
					if ((currentProject->tileC->truetDat[xx] > 130) || (currentProject->tileC->truetDat[xx + 1] > 130) || (currentProject->tileC->truetDat[xx + 2] > 130))
						over++;
				}

				if (over > 4)
					currentProject->tms->maps[currentProject->curPlane].set_prio(x, y, true); //normal
				else
					currentProject->tms->maps[currentProject->curPlane].set_prio(x, y, false); //shadowed
			}
		}
	} else {
		uint8_t temp[256];

		for (y = 0; y < currentProject->tms->maps[currentProject->curPlane].mapSizeHA; ++y) {
			for (x = 0; x < currentProject->tms->maps[currentProject->curPlane].mapSizeW; ++x) {
				uint32_t cur_tile = currentProject->tms->maps[currentProject->curPlane].get_tile(x, y);
				uint32_t errorSh = 0, errorNorm = 0;
				uint8_t * ptrorgin = &currentProject->tileC->truetDat[(cur_tile * currentProject->tileC->tcSize)];
				set_palette_type_force(0);//normal
				currentProject->tileC->truecolor_to_tile(currentProject->tms->maps[currentProject->curPlane].getPalRow(x, y), cur_tile, false);
				currentProject->tileC->tileToTrueCol(&currentProject->tileC->tDat[(cur_tile * currentProject->tileC->tileSize)], temp, currentProject->tms->maps[currentProject->curPlane].getPalRow(x, y));

				for (xx = 0; xx < 256; xx += 4) {
					errorNorm += abs(temp[xx] - ptrorgin[xx]);
					errorNorm += abs(temp[xx + 1] - ptrorgin[xx + 1]);
					errorNorm += abs(temp[xx + 2] - ptrorgin[xx + 2]);
				}

				set_palette_type_force(8);//shadow
				currentProject->tileC->truecolor_to_tile(currentProject->tms->maps[currentProject->curPlane].getPalRow(x, y), cur_tile, false);
				currentProject->tileC->tileToTrueCol(&currentProject->tileC->tDat[(cur_tile * currentProject->tileC->tileSize)], temp, currentProject->tms->maps[currentProject->curPlane].getPalRow(x, y));

				for (xx = 0; xx < 256; xx += 4) {
					errorSh += abs(temp[xx] - ptrorgin[xx]);
					errorSh += abs(temp[xx + 1] - ptrorgin[xx + 1]);
					errorSh += abs(temp[xx + 2] - ptrorgin[xx + 2]);
				}

				if (errorSh < errorNorm)
					currentProject->tms->maps[currentProject->curPlane].set_prio(x, y, false); //shadowed
				else
					currentProject->tms->maps[currentProject->curPlane].set_prio(x, y, true); //normal
			}
		}

		set_palette_type();//0 normal 8 shadow 16 highlight
	}

	window->redraw();
}
