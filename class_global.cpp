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
#include <stdexcept>

#include "class_global.h"
#include "callback_chunk.h"
#include "callbacksprites.h"
#include "classSprite.h"
#include "undo.h"
#include "classpalettebar.h"
#include "gui.h"
#include "runlua.h"
#include "luaconfig.h"
static void rect_alpha_grid(uint8_t rgba[4], unsigned x, unsigned y) {
	uint8_t grid[32 * 32 * 3];
	//first generate grid
	uint8_t * ptr_grid = grid;
	int e = 16, c;

	while (e--) {
		c = 48;

		while (c--)
			*ptr_grid++ = 255;

		c = 48;

		while (c--)
			*ptr_grid++ = 160;
	}

	e = 16;

	while (e--) {
		c = 48;

		while (c--)
			*ptr_grid++ = 160;

		c = 48;

		while (c--)
			*ptr_grid++ = 255;
	}

	if (rgba[3]) {
		ptr_grid = grid;
		double percent = rgba[3] / 255.0;

		for (unsigned c = 0; c < 32 * 32; ++c) {
			for (unsigned e = 0; e < 3; ++e) {
				double gridNerd = *ptr_grid;
				*ptr_grid++ = ((double)rgba[e] * percent) + (gridNerd * (1.0 - percent));
			}
		}
	}

	fl_draw_image(grid, x, y, 32, 32, 3);
}
static void uintstr(unsigned x, char*tmp) {
	snprintf(tmp, 16, "%u", x);
}
void editor::updateChunkSel(void) {
	chunk_select->maximum(currentProject->Chunk->amt - 1);

	if (chunk_select->value() > currentProject->Chunk->amt - 1) {
		chunk_select->value(currentProject->Chunk->amt - 1);
		currentChunk = currentProject->Chunk->amt - 1;
	}
}
void editor::updateMapWH(uint32_t w, uint32_t h) {
	char tmp[16];
	uintstr(w, tmp);
	map_w->value(tmp);
	uintstr(h, tmp);
	map_h->value(tmp);
}
void editor::updateMapWH(void) {
	updateMapWH(currentProject->tms->maps[currentProject->curPlane].mapSizeW, currentProject->tms->maps[currentProject->curPlane].mapSizeH);
}
void editor::updateBlockTilesChunk(uint32_t prj) {
	if (projects->at(prj).Chunk->useBlocks) {
		tile_select_3->label("Block select");
		useBlocksChunkCBtn->value(1);
	} else {
		tile_select_3->label("Tile select");
		useBlocksChunkCBtn->value(0);
	}
}
void editor::updateBlockTilesChunk(void) {
	updateBlockTilesChunk(curProjectID);
}
static void intstr(int x, char*tmp) {
	snprintf(tmp, 16, "%d", x);
}
void editor::updateSpriteSliders(uint32_t prjIDX) {
	Project&prj = projects->at(prjIDX);

	bool haveSprite = (prj.ms->sps[curSpritemeta].groups[curSpritegroup].list.size()) ? true : false;
	metaspritesel->maximum(prj.ms->sps.size() - 1);

	if (metaspritesel->value() >= metaspritesel->maximum()) {
		metaspritesel->value(prj.ms->sps.size() - 1);
		curSpritemeta = prj.ms->sps.size() - 1;
	}

	spriteselgroup->maximum(prj.ms->sps[curSpritemeta].amt - 1);

	if (spriteselgroup->value() > prj.ms->sps[curSpritemeta].amt - 1) {
		spriteselgroup->value(prj.ms->sps[curSpritemeta].amt - 1);
		curSpritegroup = prj.ms->sps[curSpritemeta].amt - 1;
	}

	if (haveSprite) {
		int fixedRow = currentProject->fixedSpirtePalRow();

		if (!spritesel->visible()) {
			spritesel->show();
			spritest->show();
			spriteslat->show();
			spritesize[0]->show();
			spritesize[1]->show();

			if (fixedRow < 0)
				spritepalrow->show();

			spritesoff[0]->show();
			spritesoff[1]->show();
			spritehflip->show();
			spritevflip->show();
			spriteprio->show();

			for (unsigned i = 0; i < 4; ++i)
				spritealign[i]->show();
		}

		spritesel->maximum(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list.size() - 1);

		if (spritesel->value() > prj.ms->sps[curSpritemeta].groups[curSpritegroup].list.size() - 1) {
			spritesel->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list.size() - 1);
			curSprite = prj.ms->sps[curSpritemeta].groups[curSpritegroup].list.size() - 1;
		}

		spritest->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].starttile);
		spriteslat->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].loadat);
		spritesize[0]->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].w);
		spritesize[1]->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].h);

		if (fixedRow < 0) {
			spritepalrow->show();
			spritepalrow->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].palrow);
		} else
			spritepalrow->hide();

		char tmp[16];
		intstr(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].offx, tmp);
		spritesoff[0]->value(tmp);
		intstr(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].offy, tmp);
		spritesoff[1]->value(tmp);
		spritehflip->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].hflip);
		spritevflip->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].vflip);
		spriteprio->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].prio);
	} else {
		curSprite = 0;

		if (spritesel->visible()) {
			spritesel->hide();
			spritest->hide();
			spriteslat->hide();
			spritesize[0]->hide();
			spritesize[1]->hide();
			spritepalrow->hide();
			spritesoff[0]->hide();
			spritesoff[1]->hide();
			spritehflip->hide();
			spritevflip->hide();
			spriteprio->hide();

			for (unsigned i = 0; i < 4; ++i)
				spritealign[i]->hide();
		}
	}

	spritegrouptxt->value(prj.ms->sps[curSpritemeta].groups[curSpritegroup].name.c_str());
}
void editor::updateSpriteSliders(void) {
	updateSpriteSliders(curProjectID);
}
void editor::updateChunkSize(uint32_t wi, uint32_t hi) {
	char tmp[16];
	uintstr(wi, tmp);
	chunksize[0]->value(tmp);
	uintstr(hi, tmp);
	chunksize[1]->value(tmp);
}
void editor::updateChunkSize(void) {
	updateChunkSize(currentProject->Chunk->wi, currentProject->Chunk->hi);
}
void editor::draw_non_gui(void) {
	//When resizing the window things move around so we need to compensate for that
	int x, y; //we will need to reuse these later
	unsigned tiles_size = tile_size->value();
	unsigned placer_tile_size = place_tile_size->value();

	switch (mode_editor) {
		case pal_edit:
			palBar.updateSize(0);
			palBar.drawBoxes(0);
			break;

		case tile_edit:
			palBar.updateSize(1);
			//draw truecolor preview box
			true_color_box_y = (double)((double)h() / 600.0) * (double)default_true_color_box_y;
			true_color_box_x = (double)((double)w() / 800.0) * (double)default_true_color_box_x;
			rect_alpha_grid(truecolor_temp, true_color_box_x, true_color_box_y);
			tile_edit_truecolor_off_x = (double)((double)w() / 800.0) * (double)default_tile_edit_truecolor_off_x;
			tile_edit_truecolor_off_y = (double)((double)h() / 600.0) * (double)default_tile_edit_truecolor_off_y;
			tile_edit_offset_y = (double)((double)h() / 600.0) * (double)default_tile_edit_offset_y;
			tile_edit_offset_x = (tiles_size * (currentProject->tileC->width() + 1)) + tile_edit_truecolor_off_x; //I multiplied it by +1 to allow for some spacing between the tiles
			//draw palette selection box
			palBar.drawBoxes(1);

			if (currentProject->tileC->tDat.size()) {
				currentProject->tileC->draw_truecolor(getCurrentTileCurrentTab(), tile_edit_truecolor_off_x, tile_edit_truecolor_off_y, false, false, tiles_size);
				currentProject->tileC->draw_tile(tile_edit_offset_x, tile_edit_offset_y, getCurrentTileCurrentTab(), tiles_size, palBar.selRow[1], false, false, false, false);

				if (show_grid) {
					if (tiles_size > 4) {
						for (y = 0; y < currentProject->tileC->height(); ++y) {
							for (x = 0; x < currentProject->tileC->width(); ++x)
								fl_draw_box(FL_EMBOSSED_FRAME, (x * tiles_size) + tile_edit_offset_x, (y * tiles_size) + tile_edit_offset_y, tiles_size, tiles_size, FL_FOREGROUND_COLOR);
						}

						for (y = 0; y < currentProject->tileC->height(); y++) {
							for (x = 0; x < currentProject->tileC->width(); ++x)
								fl_draw_box(FL_EMBOSSED_FRAME, (x * tiles_size) + tile_edit_truecolor_off_x, (y * tiles_size) + tile_edit_truecolor_off_y, tiles_size, tiles_size, FL_FOREGROUND_COLOR);
						}
					}
				}
			}

			break;

		case tile_place:
			palBar.updateSize(2);
			tile_placer_tile_offset_y = (double)((double)h() / 600.0) * (double)default_tile_placer_tile_offset_y;
			palBar.drawBoxes(2);
			//now draw the tile
			currentProject->tileC->draw_tile(tile_placer_tile_offset_x, tile_placer_tile_offset_y, getCurrentTileCurrentTab(), placer_tile_size, palBar.selRow[2], G_hflip[0], G_vflip[0]);
			//convert position
			map_off_y = (float)((float)h() / 600.f) * (float)default_map_off_y;
			map_off_x = (float)((float)w() / 800.f) * (float)default_map_off_x;
			//draw tile map
			uint32_t max_map_w, max_map_h; //used to calculate the displayable tiles
			max_map_w = ((placer_tile_size * currentProject->tileC->width()) + w() - map_off_x) / (placer_tile_size * currentProject->tileC->width()); //this will allow one tile to go partly off screen
			max_map_h = ((placer_tile_size * currentProject->tileC->height()) + h() - map_off_y) / (placer_tile_size * currentProject->tileC->height());
			currentProject->tms->maps[currentProject->curPlane].drawPart(map_off_x, map_off_y, map_scroll_pos_x, map_scroll_pos_y, std::min(currentProject->tms->maps[currentProject->curPlane].mapSizeW - map_scroll_pos_x, max_map_w), std::min((currentProject->tms->maps[currentProject->curPlane].mapSizeHA) - map_scroll_pos_y, max_map_h), rowSolo ? palBar.selRow[2] : -1, placer_tile_size, showTrueColor);

			if (show_grid_placer) {
				//draw box over tiles
				for (y = 0; y < std::min((currentProject->tms->maps[currentProject->curPlane].mapSizeHA) - map_scroll_pos_y, max_map_h); ++y) {
					for (x = 0; x < std::min(currentProject->tms->maps[currentProject->curPlane].mapSizeW - map_scroll_pos_x, max_map_w); ++x)
						fl_draw_box(FL_EMBOSSED_FRAME, map_off_x + ((x * currentProject->tileC->width())*placer_tile_size), map_off_y + ((y * currentProject->tileC->height())*placer_tile_size), placer_tile_size * currentProject->tileC->width(), placer_tile_size * currentProject->tileC->height(), FL_FOREGROUND_COLOR);
				}
			}

			if (tileEditModePlace_G) {
				int32_t xo, yo;
				xo = ((selTileE_G[0] - map_scroll_pos_x) * currentProject->tileC->width() * placer_tile_size) + map_off_x;
				yo = ((selTileE_G[1] - map_scroll_pos_y) * currentProject->tileC->height() * placer_tile_size) + map_off_y;

				if ((xo >= map_off_x) && (yo >= map_off_y))
					fl_draw_box(FL_EMBOSSED_FRAME, xo, yo, placer_tile_size * currentProject->tileC->width() + 1, placer_tile_size * currentProject->tileC->height() + 1, FL_FOREGROUND_COLOR);
			}

			break;

		case chunkEditor:
			tiles_size = chunk_tile_size->value();
			ChunkOff[0] = (double)((double)w() / 800.0) * (double)DefaultChunkX;
			ChunkOff[1] = (double)((double)h() / 600.0) * (double)DefaultChunkY;
			currentProject->Chunk->drawChunk(currentChunk, ChunkOff[0], ChunkOff[1], tiles_size, scrollChunks_G[0], scrollChunks_G[1]);

			if (tileEditModeChunk_G) {
				int32_t xo, yo;
				unsigned tsx, tsy;
				tsx = currentProject->tileC->width() * tiles_size;
				tsy = currentProject->tileC->height() * tiles_size;

				if (currentProject->Chunk->useBlocks) {
					tsx *= currentProject->tms->maps[currentProject->curPlane].mapSizeW;
					tsy *= currentProject->tms->maps[currentProject->curPlane].mapSizeH;
				}

				xo = ((editChunk_G[0] - scrollChunks_G[0]) * tsx);
				yo = ((editChunk_G[1] - scrollChunks_G[1]) * tsy);
				xo += ChunkOff[0];
				yo += ChunkOff[1];

				if ((xo >= ChunkOff[0]) && (yo >= ChunkOff[1]))
					fl_draw_box(FL_EMBOSSED_FRAME, xo, yo, tsx + 1, tsy + 1, FL_FOREGROUND_COLOR);
			}

			break;

		case spriteEditor:
			palBar.updateSize(3);
			palBar.drawBoxes(3);
			SpriteOff[0] = (double)((double)w() / 800.0) * (double)defaultspritex;
			SpriteOff[1] = (double)((double)w() / 600.0) * (double)defaultspritey;

			if (currentProject->ms->sps[curSpritemeta].groups[curSpritegroup].list.size()) {
				if (centerSpriteDraw_G)
					currentProject->ms->sps[curSpritemeta].draw(curSpritegroup, w() / 2, h() / 2, spritezoom->value(), centerSpriteDraw_G, &spriteEndDraw[0], &spriteEndDraw[1]);
				else
					currentProject->ms->sps[curSpritemeta].draw(curSpritegroup, SpriteOff[0], SpriteOff[1], spritezoom->value(), centerSpriteDraw_G, &spriteEndDraw[0], &spriteEndDraw[1]);

				//Now draw the tile selection
				if (spriteEndDraw[1] < (h() - 48)) {
					tilesSpriteOff[0] = unsigned(double(192.0 * (double)w() / 800.0));
					tilesSpriteOff[1] = spriteEndDraw[1] + unsigned(double(32.0 * (double)h() / 600.0));
					unsigned perw = (w() - tilesSpriteOff[0]) / (currentProject->tileC->width() * 2);
					unsigned perh = (h() - (tilesSpriteOff[1])) / (currentProject->tileC->height() * 2);
					unsigned starttile = currentProject->ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].starttile;

					if (starttile > ((perw * perh) / 2))
						starttile -= (perw * perh) / 2;
					else
						starttile = 0;

					unsigned looptile = starttile;
					unsigned tileatx = 0, tileaty = 0;

					for (unsigned y = tilesSpriteOff[1]; y < h(); y += currentProject->tileC->height() * 2) {
						for (unsigned x = tilesSpriteOff[0]; x < w(); x += currentProject->tileC->width() * 2, ++looptile) {
							if (looptile >= currentProject->tileC->amount())
								break;

							unsigned palrow = currentProject->ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].palrow;

							if (currentProject->pal->haveAlt)
								palrow += currentProject->pal->rowCntPal;

							currentProject->tileC->draw_tile(x, y, looptile, 2, palrow, false, false);

							if (looptile == currentProject->ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].starttile) {
								tileatx = x;
								tileaty = y;
							}
						}

						if (looptile >= currentProject->tileC->amount())
							break;
					}

					fl_draw_box(FL_EMBOSSED_FRAME, tileatx, tileaty, (currentProject->ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].w * currentProject->ms->sps[curSpritemeta].groups[curSpritegroup].list[curSprite].h * currentProject->tileC->height() * 2) + 1, 17, FL_FOREGROUND_COLOR);
				}
			}

			break;
	}//end of switch statement
}

void editor::draw() {
	switch (mode_editor) {
		case tile_edit:
			palBar.updateColorSelectionTile(getCurrentTileCurrentTab(), tile_edit);

		case tile_place:
			if (tileEditModePlace_G)
				tile_select_2->value(getCurrentTileCurrentTab());

			palBar.updateColorSelectionTile(getCurrentTileCurrentTab(), tile_place);
			break;
	}

	if (damage() != FL_DAMAGE_USER1)
		Fl_Double_Window::draw();

	lua_getglobal(Lconf, "drawCBfirst");
	lua_pushinteger(Lconf, mode_editor);
	runLuaFunc(Lconf, 1, 0);
	draw_non_gui();
	lua_getglobal(Lconf, "drawCBlast");
	lua_pushinteger(Lconf, mode_editor);
	runLuaFunc(Lconf, 1, 0);
}

// Create a window at the specified position
editor::editor(int X, int Y, int W, int H, const char *L)
	: Fl_Double_Window(X, Y, W, H, L) {
	_editor();
}


// Create a block window
editor::editor(int W, int H, const char *L)
	: Fl_Double_Window(W, H, L) {
	_editor();
}
static void setXYdisp(int x, int y, unsigned n) {
	char tmp[64];
	snprintf(tmp, 64, "X: %d, Y: %d", x, y);
	window->cordDisp[n]->copy_label(tmp);
}
static void setXYdispBlock(int x, int y) {
	if (currentProject->tms->maps[currentProject->curPlane].isBlock) {
		char tmp[128];
		snprintf(tmp, 128, "Block: %d X: %d, Y: %d", y / currentProject->tms->maps[currentProject->curPlane].mapSizeH, x % currentProject->tms->maps[currentProject->curPlane].mapSizeW, y % currentProject->tms->maps[currentProject->curPlane].mapSizeH);
		window->cordDisp[0]->copy_label(tmp);
	} else
		setXYdisp(x, y, 0);
}
int pushed_g;
void editor::updateTileMapGUI(uint32_t x, uint32_t y) {
	if (x >= currentProject->tms->maps[currentProject->curPlane].mapSizeW)
		x = currentProject->tms->maps[currentProject->curPlane].mapSizeW - 1;

	if (y >= currentProject->tms->maps[currentProject->curPlane].mapSizeHA)
		y = currentProject->tms->maps[currentProject->curPlane].mapSizeHA - 1;

	selTileE_G[0] = x;
	selTileE_G[1] = y;
	G_highlow_p[0] = currentProject->tms->maps[currentProject->curPlane].get_prio(x, y);
	G_hflip[0] = currentProject->tms->maps[currentProject->curPlane].get_hflip(x, y);
	G_vflip[0] = currentProject->tms->maps[currentProject->curPlane].get_vflip(x, y);
	hflipCB[0]->value(G_hflip[0]);
	vflipCB[0]->value(G_vflip[0]);
	prioCB[0]->value(G_highlow_p[0]);

	uint32_t cT = currentProject->tms->maps[currentProject->curPlane].get_tile(x, y);
	tile_select_2->value(cT);
	palBar.updateColorSelectionTile(cT, 2);
	uint8_t Rm = currentProject->tms->maps[currentProject->curPlane].getPalRow(x, y);
	palBar.changeRow(Rm, 2);
	unsigned focus = 0;

	for (int as = 0; as < 4; ++as)
		focus |= Fl::focus() == palRTE[as + 4];

	for (int as = 0; as < 4; ++as) {
		palRTE[as + 4]->value(as == Rm);

		if (focus && (as == Rm))
			Fl::focus(palRTE[as + 4]);
	}

	setXYdispBlock(x, y);
}
void editor::updateChunkGUI(uint32_t tx, uint32_t ty) {
	editChunk_G[0] = tx;
	editChunk_G[1] = ty;
	G_hflip[1] = currentProject->Chunk->getHflip(currentChunk, tx, ty);
	G_vflip[1] = currentProject->Chunk->getVflip(currentChunk, tx, ty);
	G_highlow_p[1] = currentProject->Chunk->getPrio(currentChunk, tx, ty);
	hflipCB[1]->value(G_hflip[1]);
	vflipCB[1]->value(G_vflip[1]);
	prioCB[1]->value(G_highlow_p[1]);

	tile_select_3->value(currentProject->Chunk->getBlock(currentChunk, tx, ty));
	solidBits_G = currentProject->Chunk->getSolid(currentChunk, tx, ty);
	solidChunkMenu->value(solidBits_G);
	setXYdisp(tx, ty, 1);
}
static inline unsigned cal_offset_truecolor(unsigned x, unsigned y, unsigned rgb, uint32_t tile) {
	/*!<
	cal_offset_truecolor is made to help when accessing a true color tile array
	an example of it would be
	red_temp=truecolor_data[cal_offset_truecolor(0,0,0,0)]//get the red pixel at pixel 0,0 at tile 0
	green_temp=truecolor_data[cal_offset_truecolor(0,0,1,0)]//get the green pixel at pixel 0,0 at tile 0
	blue_temp=truecolor_data[cal_offset_truecolor(0,0,2,0)]//get the blue pixel at pixel 0,0 at tile 0
	*/
	return (x * 4) + (y * currentProject->tileC->width() * 4) + rgb + (tile * currentProject->tileC->tcSize);
}
int editor::handle(int event) {
	if (event == FL_PUSH)
		pushed_g = 1; //The slider callback will need to clear this

	lua_getglobal(Lconf, "eventCBfirst");
	lua_pushinteger(Lconf, event);
	lua_pushinteger(Lconf, mode_editor);
	runLuaFunc(Lconf, 2, 1);
	int ret = luaL_checkinteger(Lconf, -1);
	lua_pop(Lconf, 1);

	if (ret)
		return 1;

	if (Fl_Double_Window::handle(event)) return (1);

	unsigned tiles_size;

	switch (event) {
		case FL_PUSH:
			switch (mode_editor) {
				case pal_edit:
					palBar.checkBox(Fl::event_x(), Fl::event_y(), 0);
					break;

				case tile_edit:
					palBar.checkBox(Fl::event_x(), Fl::event_y(), 1);
					//first see if we are in a "valid" range
					tiles_size = tile_size->value();

					//start by handling true color
					if ((Fl::event_x() > tile_edit_truecolor_off_x) && (Fl::event_y() > tile_edit_truecolor_off_y) && (Fl::event_x() < tile_edit_truecolor_off_x + (tiles_size * currentProject->tileC->width()))  && (Fl::event_y() < tile_edit_truecolor_off_y + (tiles_size * currentProject->tileC->height()))) {
						//if all conditions have been met that means we are able to edit the truecolor tile
						unsigned temp_two, temp_one;
						temp_one = (Fl::event_x() - tile_edit_truecolor_off_x) / tiles_size;
						temp_two = (Fl::event_y() - tile_edit_truecolor_off_y) / tiles_size;
						pushTilePixel(getCurrentTileCurrentTab(), temp_one, temp_two, tTypeTruecolor);
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 0, getCurrentTileCurrentTab())] = truecolor_temp[0]; //red
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 1, getCurrentTileCurrentTab())] = truecolor_temp[1]; //green
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 2, getCurrentTileCurrentTab())] = truecolor_temp[2]; //blue
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 3, getCurrentTileCurrentTab())] = truecolor_temp[3]; //alpha
						pushTile(getCurrentTileCurrentTab(), tTypeTile);
						currentProject->tileC->truecolor_to_tile(palBar.selRow[1], getCurrentTileCurrentTab(), false);
						damage(FL_DAMAGE_USER1);
					}

					if (Fl::event_x() > tile_edit_offset_x && Fl::event_y() > tile_edit_offset_y && Fl::event_x() < tile_edit_offset_x + (tiles_size * currentProject->tileC->width()) && Fl::event_y() < tile_edit_offset_y + (tiles_size * currentProject->tileC->height())) {
						unsigned temp_two, temp_one;
						temp_one = (Fl::event_x() - tile_edit_offset_x) / tiles_size;
						temp_two = (Fl::event_y() - tile_edit_offset_y) / tiles_size;
						unsigned get_pal = palBar.getEntry(1) * 3;
						pushTilePixel(getCurrentTileCurrentTab(), temp_one, temp_two, tTypeTruecolor);
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 0, getCurrentTileCurrentTab())] = currentProject->pal->rgbPal[get_pal]; //red
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 1, getCurrentTileCurrentTab())] = currentProject->pal->rgbPal[get_pal + 1]; //green
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 2, getCurrentTileCurrentTab())] = currentProject->pal->rgbPal[get_pal + 2]; //blue
						currentProject->tileC->truetDat[cal_offset_truecolor(temp_one, temp_two, 3, getCurrentTileCurrentTab())] = 255;
						pushTile(getCurrentTileCurrentTab(), tTypeTile);
						currentProject->tileC->truecolor_to_tile(palBar.selRow[1], getCurrentTileCurrentTab(), false);
						damage(FL_DAMAGE_USER1);
					}

					break;

				case tile_place:
					palBar.checkBox(Fl::event_x(), Fl::event_y(), 2);
					tiles_size = place_tile_size->value();

					//see if the user placed a tile on the map
					if ((Fl::event_x() > map_off_x) && (Fl::event_y() > map_off_y) && (Fl::event_x() < map_off_x + ((tiles_size * currentProject->tileC->width()) * (currentProject->tms->maps[currentProject->curPlane].mapSizeW - map_scroll_pos_x))) && (Fl::event_y() < map_off_y + ((tiles_size * currentProject->tileC->height()) * ((currentProject->tms->maps[currentProject->curPlane].mapSizeHA) - map_scroll_pos_y)))) {
						uint32_t temp_two, temp_one;
						temp_one = ((Fl::event_x() - map_off_x) / tiles_size) / currentProject->tileC->width();
						temp_two = ((Fl::event_y() - map_off_y) / tiles_size) / currentProject->tileC->height();
						temp_one += +map_scroll_pos_x;
						temp_two += +map_scroll_pos_y;

						if (Fl::event_button() == FL_LEFT_MOUSE) {
							if (!((selTileE_G[0] == temp_one) && (selTileE_G[1] == temp_two) && tileEditModePlace_G)) {
								pushTilemapEdit(temp_one, temp_two);
								currentProject->tms->maps[currentProject->curPlane].set_tile_full(temp_one, temp_two, getCurrentTileCurrentTab(), palBar.selRow[2], G_hflip[0], G_vflip[0], G_highlow_p[0]);
								setXYdispBlock(temp_one, temp_two);
							}

							tileEditModePlace_G = false;
							damage(FL_DAMAGE_USER1);
						} else {
							if (((tileEditModePlace_G) && (selTileE_G[0] == temp_one) && (selTileE_G[1] == temp_two))) {
								tileEditModePlace_G = false;
								damage(FL_DAMAGE_USER1);
							} else {
								tileEditModePlace_G = true;
								updateTileMapGUI(temp_one, temp_two);
								redraw();
							}
						}
					}

					if (Fl::event_x() > tile_placer_tile_offset_x && Fl::event_y() > tile_placer_tile_offset_y && Fl::event_x() < tile_placer_tile_offset_x + (tiles_size * currentProject->tileC->width()) && Fl::event_y() < tile_placer_tile_offset_y + (tiles_size * currentProject->tileC->height())) {
						unsigned temp_two, temp_one;
						temp_one = (Fl::event_x() - tile_placer_tile_offset_x) / tiles_size;
						temp_two = (Fl::event_y() - tile_placer_tile_offset_y) / tiles_size;

						if (G_hflip[0])
							temp_one = 7 - temp_one;

						if (G_vflip[0])
							temp_two = 7 - temp_two;

						pushTilePixel(getCurrentTileCurrentTab(), temp_one, temp_two, tTypeTile);
						currentProject->tileC->setPixel(getCurrentTileCurrentTab(), temp_one, temp_two, palBar.selBox[2]);
						damage(FL_DAMAGE_USER1);//no need to redraw the gui
					}

					break;

				case chunkEditor:
					if ((Fl::event_x() >= ChunkOff[0]) && (Fl::event_y() >= ChunkOff[1])) {
						uint_fast32_t maxx, maxy;
						tiles_size = chunk_tile_size->value();
						maxx = currentProject->Chunk->wi * currentProject->tileC->width() * tiles_size;
						maxy = currentProject->Chunk->hi * currentProject->tileC->height() * tiles_size;

						if (currentProject->Chunk->useBlocks) {
							maxx *= currentProject->tms->maps[currentProject->curPlane].mapSizeW;
							maxy *= currentProject->tms->maps[currentProject->curPlane].mapSizeH;
						}

						maxx += ChunkOff[0];
						maxy += ChunkOff[1];

						if ((Fl::event_x() <= maxx) && (Fl::event_y() <= maxy)) {
							unsigned tx, ty;
							tx = (Fl::event_x() - ChunkOff[0]) / (currentProject->tileC->width() * tiles_size);
							ty = (Fl::event_y() - ChunkOff[1]) / (currentProject->tileC->height() * tiles_size);

							if (currentProject->Chunk->useBlocks) {
								tx /= currentProject->tms->maps[currentProject->curPlane].mapSizeW;
								ty /= currentProject->tms->maps[currentProject->curPlane].mapSizeH;
							}

							tx += scrollChunks_G[0];
							ty += scrollChunks_G[1];

							if (Fl::event_button() == FL_LEFT_MOUSE) {
								if (!((tileEditModeChunk_G) && (tx == editChunk_G[0]) && (ty == editChunk_G[1]))) {
									pushChunkEdit(currentChunk, tx, ty);
									currentProject->Chunk->setSolid(currentChunk, tx, ty, solidBits_G);
									currentProject->Chunk->setHflip(currentChunk, tx, ty, G_hflip[1]);
									currentProject->Chunk->setVflip(currentChunk, tx, ty, G_vflip[1]);
									currentProject->Chunk->setBlock(currentChunk, tx, ty, tile_select_3->value());
									setXYdisp(tx, ty, 1);
								}

								tileEditModeChunk_G = false;
								damage(FL_DAMAGE_USER1);
							} else {
								if ((tileEditModeChunk_G) && (tx == editChunk_G[0]) && (ty == editChunk_G[1])) {
									tileEditModeChunk_G = false;
									damage(FL_DAMAGE_USER1);
								} else {
									tileEditModeChunk_G = true;
									updateChunkGUI(tx, ty);
									redraw();
								}
							}
						}
					}

					break;

				case spriteEditor:
					palBar.checkBox(Fl::event_x(), Fl::event_y(), 3);
					break;
			}

			break;

		case FL_SHORTCUT:
			switch (mode_editor) {
				case tile_place:
					if (tileEditModePlace_G) {
						const char*t = Fl::event_text();
						bool nr = false;

						switch (tolower(t[0])) {
							case 'h':
								if (selTileE_G[0])
									--selTileE_G[0];

								nr = true;
								break;

							case 'j':
								if (selTileE_G[1] < (currentProject->tms->maps[currentProject->curPlane].mapSizeHA - 1))
									++selTileE_G[1];

								nr = true;
								break;

							case 'k':
								if (selTileE_G[1])
									--selTileE_G[1];

								nr = true;
								break;

							case 'l':
								if (selTileE_G[0] < (currentProject->tms->maps[currentProject->curPlane].mapSizeW - 1))
									++selTileE_G[0];

								nr = true;
								break;
						}

						if (nr) {
							updateTileMapGUI(selTileE_G[0], selTileE_G[1]);
							redraw();
						}
					}

					break;
			}

			break;
	}

	lua_getglobal(Lconf, "eventCBlast");
	lua_pushinteger(Lconf, event);
	lua_pushinteger(Lconf, mode_editor);
	runLuaFunc(Lconf, 2, 1);
	ret = luaL_checkinteger(Lconf, -1);
	lua_pop(Lconf, 1);

	if (ret)
		return 1;

	return 0;
}
unsigned editor::getCurrentTileCurrentTab()const {
	switch (mode_editor) {
		case tile_edit:
			return tile_select->value();

		case tile_place:
			if (tileEditModePlace_G)
				return currentProject->tms->maps[currentProject->curPlane].get_tile(selTileE_G[0], selTileE_G[1]);

			return tile_select_2->value();

		case spriteEditor:
			return spritest->value();

		default:
			throw new std::logic_error("Unexpected tab for getCurrentTileCurrentTab." + std::to_string(mode_editor));
	}
}
