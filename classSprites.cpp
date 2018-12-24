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
#include <FL/Fl_Scroll.H>
#include <FL/fl_ask.H>
#include <FL/filename.H>

#include <cstdlib>
#include <cstring>
#include <ctime>

#include "macros.h"
#include "classSprite.h"
#include "classSprites.h"
#include "callback_tiles.h"
#include "image.h"
#include "class_global.h"
#include "gui.h"
#include "errorMsg.h"
const char*spriteDefName = "DefaultGroupLabel";
const char*spritesName = "AllGroupsLabel";
sprites::sprites(Project*prj) {
	this->prj = prj;
	amt = 1;
	groups.push_back(spriteGroup());
	groups[0].list.push_back(sprite());
	groups[0].name.assign(spriteDefName);
	name.assign(spritesName);
	extraOptDPLC = false;
}
sprites::sprites(const sprites&other, Project*prj) {
	this->prj = prj;
	groups.reserve(other.groups.size());
	name = other.name;

	for (uint32_t i = 0; i < other.groups.size(); ++i)
		groups.push_back(spriteGroup());

	for (uint32_t j = 0; j < other.groups.size(); ++j) {
		unsigned sz = other.groups[j].list.size();
		groups[j].list.reserve(sz);
		groups[j].name = other.groups[j].name;

		for (uint32_t i = 0; i < sz; ++i)
			groups[j].list.push_back(sprite(other.groups[j].list[i].w, other.groups[j].list[i].h, other.groups[j].list[i].palrow, other.groups[j].list[i].starttile, other.groups[j].list[i].hflip, other.groups[j].list[i].vflip, other.groups[j].list[i].prio, other.groups[j].list[i].loadat, other.groups[j].list[i].offx, other.groups[j].list[i].offy));
	}

	amt = other.amt;
}
sprites::~sprites() {
	for (uint32_t j = 0; j < amt; ++j) {
		groups[j].list.clear();
		groups[j].name.clear();
	}

	name.clear();
	groups.clear();
}
static bool chkNotZero(uint8_t*dat, unsigned n) {
	while (n--) {
		if (*dat++)
			return true;
	}

	return false;
}
void sprites::fixDel(unsigned at, unsigned tamt) {
	for (unsigned i = 0; i < amt; ++i) {
		for (unsigned j = 0; j < groups[i].list.size(); ++j) {
			if (groups[i].list[j].starttile >= at) {
				groups[i].list[j].starttile -= tamt;
				groups[i].list[j].loadat -= tamt;
			}
		}
	}
}
uint32_t sprites::getTileOnSprite(unsigned x, unsigned y, unsigned which, unsigned i)const {
	return groups[which].list[i].starttile + (x * groups[which].list[i].h) + y;
}
void sprites::freeOptmizations(unsigned which) {
	for (int i = groups[which].list.size() - 1; i >= 0; --i) {
		//First check if the sprite is completely blank
		bool notBlank = false;

		for (unsigned h = 0, ctile = groups[which].list[i].starttile; h < groups[which].list[i].h; ++h) {
			for (unsigned w = 0; w < groups[which].list[i].w; ++w, ++ctile)
				notBlank |= chkNotZero(prj->tileC->truetDat.data() + (ctile * 256), 256);
		}

		if (!notBlank) {
			//Completely remove the sprite
			int tiledel = groups[which].list[i].starttile, tiledelamt = groups[which].list[i].h * groups[which].list[i].w;

			if (groups[which].list.size() <= 1)
				del(which);
			else
				delingroup(which, i);

			for (int td = tiledel + tiledelamt - 1; td >= tiledel; --td)
				prj->tileC->remove_tile_at(td);

			fixDel(tiledel, tiledelamt);

			if (groups[which].list.size() < 1)
				return;
		}
	}

	for (int i = groups[which].list.size() - 1; i >= 0; --i) {
		//Check for blank columns at the end
		for (unsigned w = groups[which].list[i].w, ctile = groups[which].list[i].starttile + (groups[which].list[i].h * groups[which].list[i].w) - 1; w--;) {
			bool notBlank = false;

			for (unsigned h = groups[which].list[i].h; h--; --ctile) {
				if (ctile >= prj->tileC->amt) {
					printf("Tile %u exceeded %u\n", ctile, prj->tileC->amt - 1);
					continue;
				} else
					notBlank |= chkNotZero(prj->tileC->truetDat.data() + (ctile * 256), 256);
			}

			if (notBlank)
				break;

			else {
				--groups[which].list[i].w;
				int tiledel = ctile, tiledelamt = groups[which].list[i].h;

				for (int td = tiledel + tiledelamt; td > tiledel; --td)
					prj->tileC->remove_tile_at(td);

				fixDel(tiledel, tiledelamt);
			}
		}
	}

	//More blank column removal this time when a blank column is detected the sprite's offset will be adjusted
	for (int i = groups[which].list.size() - 1; i >= 0; --i) {
		for (unsigned w = 0, ctile = groups[which].list[i].starttile; w < groups[which].list[i].w; ++w) {
			bool notBlank = false;

			for (unsigned h = 0; h < groups[which].list[i].h; ++h, ++ctile)
				notBlank |= chkNotZero(prj->tileC->truetDat.data() + (ctile * 256), 256);

			if (notBlank)
				break;

			else {
				--groups[which].list[i].w;
				groups[which].list[i].offx += 8;

				for (int td = ctile - 1; td >= ctile - groups[which].list[i].h; --td)
					prj->tileC->remove_tile_at(td);

				fixDel(ctile - groups[which].list[i].h, groups[which].list[i].h);
				groups[which].list[i].starttile += groups[which].list[i].h;
				ctile -= groups[which].list[i].h;
				w = 0;
			}
		}
	}

	//Remove blank rows starting at the bottom
	for (int i = groups[which].list.size() - 1; i >= 0; --i) {
		for (unsigned h = groups[which].list[i].h; h--;) {
			bool notBlank = false;

			for (unsigned w = 0; w < groups[which].list[i].w; ++w)
				notBlank |= chkNotZero(prj->tileC->truetDat.data() + (getTileOnSprite(w, h, which, i)) * 256, 256);

			if (notBlank)
				break;

			else {
				for (unsigned w = groups[which].list[i].w; w--;) {
					unsigned delWhich = getTileOnSprite(w, h, which, i);
					prj->tileC->remove_tile_at(delWhich);
					fixDel(delWhich, 1);
				}

				--groups[which].list[i].h;
			}
		}
	}

	//Remove blank rows start at top and adjust offset
	for (int i = groups[which].list.size() - 1; i >= 0; --i) {
		for (unsigned h = 0; h < groups[which].list[i].h; ++h) {
			bool notBlank = false;

			for (unsigned w = 0; w < groups[which].list[i].w; ++w)
				notBlank |= chkNotZero(prj->tileC->truetDat.data() + (getTileOnSprite(w, h, which, i)) * 256, 256);

			if (notBlank)
				break;

			else {
				for (unsigned w = groups[which].list[i].w; w--;) {
					unsigned delWhich = getTileOnSprite(w, h, which, i);
					prj->tileC->remove_tile_at(delWhich);
					fixDel(delWhich + 1, 1);
				}

				groups[which].list[i].offy += 8;
				--groups[which].list[i].h;
				h = 0;
			}
		}
	}
}
static bool isMask(int x, int y, Fl_Shared_Image*loaded_image, bool grayscale, bool useAlpha, uint8_t*mask) {
	uint8_t*imgptr;
	int w = loaded_image->w(), h = loaded_image->h();

	if (x >= w)
		return true;

	if (x < 0)
		return true;

	if (y >= h)
		return true;

	if (y < 0)
		return true;

	unsigned depth = loaded_image->d();

	switch (depth) {
		case 1:
			if (grayscale) {
				imgptr = (uint8_t*)loaded_image->data()[0];
				imgptr += (y * w) + x;
				return ((*imgptr) == (*mask));
			} else {
				imgptr = (uint8_t*)loaded_image->data()[y + 2];
				imgptr += x;

				if (useAlpha)
					return ((*imgptr) == ' ');
				else
					return ((*imgptr) == (*mask));
			}

			break;

		case 3:
			imgptr = (uint8_t*)loaded_image->data()[0];
			imgptr += ((y * w) + x) * 3;

			if (imgptr[0] == mask[0]) {
				if (imgptr[1] == mask[1]) {
					if (imgptr[2] == mask[2])
						return true;
				}
			}

			return false;
			break;

		case 4:
			imgptr = (uint8_t*)loaded_image->data()[0];
			imgptr += ((y * w) + x) * 4;

			if (useAlpha)
				return (imgptr[3]) ? false : true;
			else {
				if (imgptr[3]) {
					if (imgptr[0] == mask[0]) {
						if (imgptr[1] == mask[1]) {
							if (imgptr[2] == mask[2])
								return true;
						}
					}
				}

				return false;
			}

			break;
	}
}
static bool inRange(int num, int x, int y) {
	return (num >= std::min(x, y)) && (num <= std::max(x, y));
}
class RectBox : public Fl_Box {
public:
	std::vector<int>*rects;
	std::vector<bool>*deleted;
	int sel;
	Fl_Scroll *scroll;
	int handle(int e) {
		return (Fl_Box::handle(e));
	}
	void draw(void) {
		int minx = scroll->x() - x(), miny = scroll->y() - y();
		int maxx = minx + scroll->w(), maxy = miny + scroll->h();
		Fl_Box::draw();

		//Now draw the rectangles
		for (unsigned i = 0; i < rects->size(); i += 4) {
			if (((*rects)[i] >= minx) || ((*rects)[i] <= maxx)) {
				if (((*rects)[i + 2] >= miny) || ((*rects)[i + 2] <= maxy)) {
					if (!(*deleted)[i / 4])
						fl_draw_box(FL_EMBOSSED_FRAME, (*rects)[i] + x(), (*rects)[i + 2] + y(), (*rects)[i + 1] - (*rects)[i] + 1, (*rects)[i + 3] - (*rects)[i + 2] + 1, 0);
				}
			}
		}
	}
	RectBox(int x, int y, int w, int h, const char*l = 0) : Fl_Box(x, y, w, h, l) {
		box(FL_NO_BOX);
	}
};
static RectBox*box;
static bool retOkay;
static Fl_Window * win;
static void RetCB(Fl_Widget*, void*r) {
	retOkay = r ? true : false;
	win->hide();
}
bool sprites::recttoSprite(int x0, int x1, int y0, int y1, int where, Fl_Shared_Image*loaded_image, bool grayscale, unsigned*remap, uint8_t*palMap, uint8_t*mask, bool useMask, bool useAlpha) {
	if (where < 0) {
		where = amt;
		setAmt(amt + 1);
	}

	unsigned depth = loaded_image->d();
	unsigned wmax, hmax;

	switch (prj->gameSystem) {
		case segaGenesis:
			wmax = hmax = 32;
			break;

		case NES:
		case masterSystem:
		case gameGear:
			wmax = 8;
			hmax = 16;
			break;

		default:
			show_default_error
	}

	unsigned wf, hf, w, h, wt, ht;
	wf = loaded_image->w();
	hf = loaded_image->h();
	w = x1 - x0 + 1;
	h = y1 - y0 + 1;
	wt = (w + 7) & (~7);
	ht = (h + 7) & (~7);
	//Determine how many sprites will be created
	unsigned spritesnew = ((wt + wmax - 8) / wmax) * ((ht + hmax - 8) / hmax);

	if (where >= amt)
		setAmt(where + 1);

	unsigned startTile = prj->tileC->amt;
	uint8_t*out = prj->tileC->truetDat.data() + (startTile * 256);
	unsigned newTiles = (wt / 8) * (ht / 8);
	prj->tileC->amt += newTiles;
	//set new amount
	prj->tileC->resizeAmt();
	out = prj->tileC->truetDat.data() + (startTile * prj->tileC->tcSize);
	setAmtingroup(where, spritesnew);
	unsigned center[3];
	center[0] = (wt - w) / 2;
	center[1] = (ht - h) / 2;
	center[2] = w + center[0];
	uint8_t * imgptr = (uint8_t *)loaded_image->data()[0];
	printf("Center %d %d %d\n", center[0], center[1], center[2]);
	printf("w: %d h: %d wt: %d ht: %d new tiles: %d\n", w, h, wt, ht, newTiles);

	for (unsigned y = 0, cnt = 0, tilecnt = startTile; y < ht; y += hmax) {
		for (unsigned x = 0; x < wt; x += wmax, ++cnt) {
			unsigned dimx, dimy;
			dimx = ((wt - x) >= wmax) ? wmax : (wt - x) % wmax;
			dimy = ((ht - y) >= hmax) ? hmax : (ht - y) % hmax;
			groups[where].list[cnt].w = dimx / 8;
			groups[where].list[cnt].h = dimy / 8;
			groups[where].list[cnt].starttile = tilecnt;
			groups[where].list[cnt].offx = x;
			groups[where].list[cnt].offy = y;
			groups[where].list[cnt].loadat = tilecnt;
			tilecnt += (dimx / 8) * (dimy / 8);

			for (unsigned i = 0; i < dimx; i += 8) {
				for (unsigned j = 0; j < dimy; j += 8) {
					for (unsigned b = 0; b < 8; ++b) {
						for (unsigned a = 0; a < 8; ++a) {
							unsigned xx = x + i + a;
							unsigned yy = y + j + b;

							if ((!((yy < center[1]) || (yy >= (h + center[1])))) && (depth == 1) && (!grayscale))
								imgptr = (uint8_t*)loaded_image->data()[yy + y0 + 2 - center[1]];
							else if (!((yy < center[1]) || (yy >= (h + center[1])))) {
								imgptr = (uint8_t *)loaded_image->data()[0];
								imgptr += ((yy + y0) - center[1]) * wf * depth;
							}

							if ((xx > center[0]) && (xx <= center[2]))
								imgptr += ((xx + x0) - center[0]) * depth;

							if ((yy < center[1]) || (yy >= (h + center[1])))
								memset(out, 0, 4);

							else if (xx < center[0])
								memset(out, 0, 4);

							else if (xx >= center[2])
								memset(out, 0, 4);

							else {
								//Can actually convert pixel to tile
								if (useMask && (!useAlpha)) {
									switch (depth) {
										case 1:
											if (grayscale) {
												if ((*imgptr) != mask[0]) {
													memset(out, *imgptr, 3);
													out[3] = 255;
												} else
													memset(out, 0, 4);
											} else {
												if ((*imgptr) == ' ')
													memset(out, 0, 4);

												else {
													if ((*imgptr) != mask[0]) {
														unsigned p = (*imgptr++);
														out[0] = palMap[remap[p] + 1];
														out[1] = palMap[remap[p] + 2];
														out[2] = palMap[remap[p] + 3];
														out[3] = 255;
													} else
														memset(out, 0, 4);
												}
											}

											break;

										case 3:
											if ((imgptr[0] == mask[0]) && (imgptr[1] == mask[1]) && (imgptr[2] == mask[2]))
												memset(out, 0, 4);

											else {
												memcpy(out, imgptr, 3);
												out[3] = 255;
											}

											break;

										case 4:
											if ((imgptr[0] == mask[0]) && (imgptr[1] == mask[1]) && (imgptr[2] == mask[2]) && (imgptr[3]))
												memset(out, 0, 4);

											else
												memcpy(out, imgptr, 4);

											break;
									}
								} else {
									switch (depth) {
										case 1:
											if (grayscale) {
												memset(out, *imgptr, 3);
												out[3] = 255;
											} else {
												if ((*imgptr) == ' ')
													memset(out, 0, 4);

												else {
													unsigned p = (*imgptr++);
													out[0] = palMap[remap[p] + 1];
													out[1] = palMap[remap[p] + 2];
													out[2] = palMap[remap[p] + 3];
													out[3] = 255;
												}
											}

											break;

										case 3:
											memcpy(out, imgptr, 3);
											out[3] = 255;
											break;

										case 4:
											memcpy(out, imgptr, 4);
											break;
									}
								}
							}

							out += 4;
						}
					}
				}
			}
		}
	}
}
void sprites::importSpriteSheet(const char*fname) {
	if (!fname) {
		if (!load_file_generic("Load image"))
			return;

		fname = the_file.c_str();
	}

	if (fname) {
		Fl_Shared_Image * loaded_image = Fl_Shared_Image::get(fname);

		if (!loaded_image) {
			fl_alert("Error loading image");
			return;
		}

		unsigned depth = loaded_image->d();

		if (unlikely(depth != 3 && depth != 4 && depth != 1)) {
			fl_alert("Please use color depth of 1,3 or 4\nYou Used %d", depth);
			loaded_image->release();
			return;
		} else
			printf("Image depth %d\n", depth);

		uint32_t w, h;
		w = loaded_image->w();
		h = loaded_image->h();
		bool grayscale;
		uint8_t*palMap;
		uint8_t*imgptr;
		unsigned remap[256];

		if (depth == 1) {
			grayscale = handle1byteImg(loaded_image, remap);

			if (!grayscale) {
				palMap = (uint8_t*)loaded_image->data()[1];
				imgptr = (uint8_t*)loaded_image->data()[2];
			}
		}

		uint8_t mask[3];
		bool useAlpha;

		if (getMaskColorImg(loaded_image, grayscale, remap, palMap, mask, useAlpha)) {
			std::vector<int> rects;//x0,x1,y0,y1
			Fl_Window *winP;
			Fl_Progress *progress;
			mkProgress(&winP, &progress);
			time_t lasttime = time(NULL);
			progress->maximum(h);
			Fl::check();

			for (int y = 0; y < h; ++y) {
				for (int x = 0; x < w; ++x) {
					if (!isMask(x, y, loaded_image, grayscale, useAlpha, mask)) {
						rects.push_back(x);

						while (!isMask(x + 1, y, loaded_image, grayscale, useAlpha, mask))
							++x;

						rects.push_back(x);
						rects.push_back(y);
						rects.push_back(y);
					}
				}

				time_t currentTime = std::time(nullptr);

				if ((currentTime - lasttime) >= 1) {
					lasttime = currentTime;
					progress->value(h);
					Fl::check();
				}
			}

			progress->maximum(rects.size());
			progress->value(0);
			//Now combine the rectangles
			//Start by combining rectangles by that touch with y values
			bool canEnd;
			int pass = 0;
			char txtbufstage[1024];
			char txtbuf[1024];

			do {
				canEnd = true;
				snprintf(txtbufstage, 1024, "Stage 1 pass %d", pass++);
				winP->label(txtbufstage);
				Fl::check();

				for (int i = 0; i < rects.size(); i += 4) {
					for (int j = 0; j < rects.size(); j += 4) {
						if (i == j)
							continue;

						//See if rectangles are touching or overlap
						//if((inRange(rects[j+2],rects[i+2]-1,rects[i+3]+1)||inRange(rects[i+2],rects[j+2]-1,rects[j+3]+1))&&(!((rects[i+2]==rects[j+2])||(rects[i+3]==rects[j+3])))){//Is rectange j directly above or below i
						if ((rects[j + 3] - rects[i + 2]) == 1) {
							if ((inRange(rects[j], rects[i] - 1, rects[i + 1] + 1) || inRange(rects[i], rects[j] - 1, rects[j + 1] + 1))) {
								canEnd = false;
								//Merge the two squares obtaining maximum size
								//Now try and find the combination that results in the largest rectangle
								rects[i] = std::min(rects[i], rects[j]);
								rects[i + 1] = std::max(rects[i + 1], rects[j + 1]);
								rects[i + 2] = std::min(rects[i + 2], rects[j + 2]);
								rects[i + 3] = std::max(rects[i + 3], rects[j + 3]);
								rects.erase(rects.begin() + j, rects.begin() + j + 4);
								//Now try to find next in sequence
								bool foundit;

								do {
									foundit = false;

									for (int a = 0; a < rects.size(); a += 4) {
										int look = rects[i + 3] + 1;

										if (rects[a + 2] == look) {
											if ((inRange(rects[a], rects[i] - 1, rects[i + 1] + 1) || inRange(rects[i], rects[a] - 1, rects[a + 1] + 1))) {
												foundit = true;
												rects[i] = std::min(rects[i], rects[a]);
												rects[i + 1] = std::max(rects[i + 1], rects[a + 1]);
												rects[i + 2] = std::min(rects[i + 2], rects[a + 2]);
												rects[i + 3] = std::max(rects[i + 3], rects[a + 3]);
												rects.erase(rects.begin() + a, rects.begin() + a + 4);
											}
										}
									}
								} while (foundit);
							}
						}
					}

					time_t currentTime = std::time(nullptr);

					if ((currentTime - lasttime) >= 1) {
						lasttime = currentTime;
						progress->maximum(rects.size());
						progress->value(i);
						snprintf(txtbuf, 1024, "Rectangles: %d", rects.size());
						progress->label(txtbuf);
						Fl::check();
					}
				}
			} while (!canEnd);

			pass = 0;

			do {
				canEnd = true;
				snprintf(txtbufstage, 1024, "Stage 2 pass %d", pass++);
				winP->label(txtbufstage);
				progress->maximum(rects.size());
				progress->value(0);
				Fl::check();

				for (int i = 0; i < rects.size(); i += 4) {
					for (int j = 0; j < rects.size(); j += 4) {
						if (i == j)
							continue;

						//Merge overlapping rectangles
						if ((rects[i] <= rects[j + 1]) && (rects[i + 1] >= rects[j]) && (rects[i + 2] <= rects[j + 3]) && (rects[i + 3] >= rects[j + 2])) {
							canEnd = false;
							rects[i] = std::min(rects[i], rects[j]);
							rects[i + 1] = std::max(rects[i + 1], rects[j + 1]);
							rects[i + 2] = std::min(rects[i + 2], rects[j + 2]);
							rects[i + 3] = std::max(rects[i + 3], rects[j + 3]);
							rects.erase(rects.begin() + j, rects.begin() + j + 4);
						}

						//Merge touching rectangles
						if (abs(rects[i + 1] - rects[j]) == 1) {
							if ((inRange(rects[j + 2], rects[i + 2] - 1, rects[i + 3] + 1) || inRange(rects[i + 2], rects[j + 2] - 1, rects[j + 3] + 1))) {
								canEnd = false;
								rects[i] = std::min(rects[i], rects[j]);
								rects[i + 1] = std::max(rects[i + 1], rects[j + 1]);
								rects[i + 2] = std::min(rects[i + 2], rects[j + 2]);
								rects[i + 3] = std::max(rects[i + 3], rects[j + 3]);
								rects.erase(rects.begin() + j, rects.begin() + j + 4);
							}
						}
					}

					time_t currentTime = std::time(nullptr);

					if ((currentTime - lasttime) >= 1) {
						lasttime = currentTime;
						progress->maximum(rects.size());
						progress->value(i);
						snprintf(txtbuf, 1024, "Rectangles: %d", rects.size());
						progress->label(txtbuf);
						Fl::check();
					}
				}
			} while (!canEnd);

			winP->remove(progress);// remove progress bar from window
			delete (progress); // deallocate it
			delete winP;
			std::vector<bool> deleted;
			deleted.resize(rects.size() / 4);

			//Now show the window allowing user to adjust sprite settings
			if (window) {
				win = new Fl_Double_Window(640, 480, "Sprite selection");
				win->begin();
				win->resizable(win);
				Fl_Button * Ok = new Fl_Button(256, 448, 64, 24, "Okay");
				Ok->callback(RetCB, (void*)1);
				Fl_Button * Cancel = new Fl_Button(320, 448, 64, 24, "Cancel");
				Cancel->callback(RetCB, 0);
				Fl_Scroll*scroll = new Fl_Scroll(8, 8, 624, 440);
				box = new RectBox(8, 8, w, h);
				box->scroll = scroll;
				box->rects = &rects;
				box->deleted = &deleted;
				box->image(loaded_image);
				scroll->end();
				win->end();
				win->set_modal();
				win->show();
				Fl::check();

				while (win->shown())
					Fl::wait();

				delete win;
			} else
				retOkay = true;

			if (retOkay) {
				for (unsigned i = 0; i < rects.size(); i += 4)
					recttoSprite(rects[i], rects[i + 1], rects[i + 2], rects[i + 3], -1, loaded_image, grayscale, remap, palMap, mask, true, useAlpha);

				updateTileSelectAmt();
			}

			deleted.clear();
			rects.clear();
		}

		loaded_image->release();
	}
}
extern const char*rtVersionStr;
void sprites::exportMapping(gameType_t game)const {
	if (load_file_generic("Save mapping to:", true)) {
		FILE*fp;

		if (game == tSonic1) {
			fp = fopen(the_file.c_str(), "w");
			fprintf(fp, ";Sprite mapping generated by %s\n%s:\n", rtVersionStr, name.c_str());
			uint32_t dup;

			for (uint32_t i = 0; i < amt; ++i) {
				if (groups[i].list.size()) {
					if (checkDupmapping(i, dup))
						fprintf(fp, "\tdc.w %s-%s\n", groups[dup].name.c_str(), name.c_str());
					else
						fprintf(fp, "\tdc.w %s-%s\n", groups[i].name.c_str(), name.c_str());
				} else
					fputs("\tdc.w 0\n", fp);
			}

			for (uint32_t j = 0; j < amt; ++j) {
				if ((groups[j].list.size()) && (!checkDupmapping(j, dup))) {
					fprintf(fp, "%s:\n\tdc.b %d\n", groups[j].name.c_str(), (int)groups[j].list.size());

					for (uint32_t i = 0; i < groups[j].list.size(); ++i)
						fprintf(fp, "\tdc.b %d,%u,%u,%u,%d\n", groups[j].list[i].offy, ((groups[j].list[i].w - 1) << 2) | (groups[j].list[i].h - 1), (groups[j].list[i].prio << 7) | (groups[j].list[i].palrow << 5) | (groups[j].list[i].vflip << 4) | (groups[j].list[i].hflip << 3) | ((groups[j].list[i].loadat >> 8) & 7), groups[j].list[i].loadat & 255, groups[j].list[i].offx);
				}
			}

			fputs("\teven\n", fp);
		} else {
			fp = fopen(the_file.c_str(), "wb");
			std::vector<uint8_t> tmpbuf;
			unsigned acum = amt * 2;
			tmpbuf.resize(acum);

			for (unsigned i = 0; i < amt; ++i) {
				uint32_t dup;

				if (!groups[i].list.size())
					tmpbuf[i * 2] = tmpbuf[i * 2 + 1] = 0;
				else if (checkDupmapping(i, dup)) {
					printf("Duplicate mapping: %d %d\n", i, dup);
					tmpbuf[i * 2] = tmpbuf[dup * 2];
					tmpbuf[i * 2 + 1] = tmpbuf[dup * 2 + 1];
				} else {
					tmpbuf[i * 2] = acum >> 8;
					tmpbuf[i * 2 + 1] = acum & 255;
					unsigned amtg = groups[i].list.size();
					tmpbuf.push_back(amtg >> 8);
					tmpbuf.push_back(amtg & 255);
					acum += 2;

					for (unsigned j = 0; j < groups[i].list.size(); ++j) {
						int8_t off = groups[i].list[j].offy;
						tmpbuf.push_back(off);
						uint8_t tmp = ((groups[i].list[j].w - 1) << 2) | (groups[i].list[j].h - 1);
						tmpbuf.push_back(tmp);
						unsigned tile = groups[i].list[j].loadat;

						if (tile > 2047) {
							printf("Tile overflow was %d\n", tile);
							tile = 2047;
						}

						tmp = (groups[i].list[j].prio << 7) | ((groups[i].list[j].palrow & 3) << 5) | (groups[i].list[j].vflip << 4) | (groups[i].list[j].hflip << 3) | (tile >> 8);
						tmpbuf.push_back(tmp);
						tmp = tile & 255;
						tmpbuf.push_back(tmp);

						if (game == tSonic2) {
							tile /= 2;
							tmp = (groups[i].list[j].prio << 7) | ((groups[i].list[j].palrow & 3) << 5) | (groups[i].list[j].vflip << 4) | (groups[i].list[j].hflip << 3) | (tile >> 8);
							tmpbuf.push_back(tmp);
							tmp = tile & 255;
							tmpbuf.push_back(tmp);
						}

						int8_t tmpi = groups[i].list[j].offx >> 8;
						tmpbuf.push_back(tmpi);
						tmpi = groups[i].list[j].offx & 255;
						tmpbuf.push_back(tmpi);

						if (game == tSonic2)
							acum += 8;
						else
							acum += 6;
					}
				}
			}

			fwrite(tmpbuf.data(), 1, tmpbuf.size(), fp);
			tmpbuf.clear();
		}

		fclose(fp);
	}
}
static char*skipComment(char*txt) {
	while ((*txt != '\n') && (*txt != 0))
		++txt;

	return txt;
}
static char*readNbytesAsm(uint8_t*buf, char*txt, unsigned n) {
	for (unsigned j = 0; j < n; ++j) {
anotherTry:

		while ((!isdigit(*txt)) && ((*txt) != '$') && ((*txt) != '-') && ((*txt) != ';'))
			++txt;

		long tmp;

		if (*txt == ';') {
			txt = skipComment(txt); //Some sprite mapping assembly data may contain comments with numbers skip them
			goto anotherTry;
		}

		if (*txt == '$')
			tmp = strtol(txt + 1, &txt, 16);
		else
			tmp = strtol(txt, &txt, 0);

		buf[j] = tmp & 255;
	}

	return txt;
}
void sprites::mappingItem(void*in, uint32_t id, gameType_t game) {
	if (!in)
		return;

	if (game == tSonic1) {
		char*txt = (char*)in;

		if (txt = strstr(txt, "dc.b")) {
			uint32_t amtgroup;
			txt += strlen("dc.b");

			while (isspace(*txt++));

			--txt;

			if (*txt == '$')
				amtgroup = strtol(txt + 1, &txt, 16);
			else
				amtgroup = strtol(txt, &txt, 0);

			setAmtingroup(id, amtgroup);

			for (uint32_t i = 0; i < amtgroup; ++i) {
				uint8_t buf[5];
				txt = readNbytesAsm(buf, txt, 5);
				//Now convert sprite format
				/*From sonic retro wiki
				 * Each mapping is 5 bytes long, taking the form TTTT TTTT 0000 WWHH PCCY XAAA AAAA AAAA LLLL LLLL.
				 *
				 * LLLL LLLL is the left co-ordinate of where the mapping appears.
				 * TTTT TTTT is the top co-ordinate of where the mapping appears.
				 * WW is the width of the mapping, in tiles minus one. So 0 means 8 pixels wide, 1 means 16 pixels wide, 2 means 24 pixels wide and 3 means 32 pixels wide.
				 * HH is the height of the mapping, in the same format as the width.
				 * P is the priority-flag. If P is set, the mapping will appear above everything else.
				 * CC is the palette line.
				 * X is the x-flip-flag. If X is set, the mapping will be flipped horizontally.
				 * Y is the y-flip-flag. If Y is set, the mapping will be flipped vertically.
				 * AAA AAAA AAAA is the tile index. */
				int8_t*bufi = (int8_t*)buf;
				groups[id].list[i].offy = bufi[0];
				groups[id].list[i].w = ((buf[1] >> 2) & 3) + 1;
				groups[id].list[i].h = (buf[1] & 3) + 1;
				groups[id].list[i].prio = (buf[2] & (1 << 7)) >> 7;
				groups[id].list[i].palrow = (buf[2] & (3 << 5)) >> 5;
				groups[id].list[i].vflip = (buf[2] & (1 << 4)) >> 4;
				groups[id].list[i].hflip = (buf[2] & (1 << 3)) >> 3;
				uint16_t tile = (buf[2] & 7) << 8;
				tile |= buf[3];
				groups[id].list[i].starttile = tile;
				groups[id].list[i].loadat = tile;
				groups[id].list[i].offx = bufi[4];
			}
		} else
			return;
	} else {
		/* From the Sonic Retro Wiki
		 * First word:
		 * High byte is the relative signed top edge position of the sprite from the center of the object.
		 * Low byte is the size of the sprite, in tiles minus one.
		 * The upper four bits are ignored, the next two bits control the width and the lowest two bits control the height.
		 * Thus sprites can be of any size from 1x1 tile to 4x4 tiles. For example, $01 is a 1x2 sprite, $02 is a 1x3 sprite, $04 is a 2x1 sprite, and so on.
		 * Second and third words:
		 * The second word applies to one-player mode; the third applies to two-player mode.
		 * The relevant word will be added to the object's VRAM offset and then used as the pattern index for that sprite.
		 * Like all SEGA Genesis VDP pattern indices, it is a bitmask of the form PCCY XAAA AAAA AAAA.
		 * P is the priority flag,
		 * CC is the palette line to use,
		 * X and Y indicate that the sprite should be flipped horizontally and vertically respectively and
		 * AAA AAAA AAAA is the actual tile index, i.e. the VRAM offset of the pattern divided by $20 (or bit-shifted right by 5).
		 * Fourth word: This is the relative signed left edge position of the sprite from the center of the object. */
		uint16_t*buf = (uint16_t*)in;
		uint16_t tmp = *buf++;
		boost::endian::conditional_reverse_inplace<boost::endian::order::big, boost::endian::order::native>(tmp);
		unsigned amtgroup = tmp;
		setAmtingroup(id, amtgroup);

		for (unsigned i = 0; i < amtgroup; ++i) {
			int8_t*bufi = (int8_t*)buf;
			groups[id].list[i].offy = *bufi++;
			uint8_t*bufu = (uint8_t*)bufi;
			groups[id].list[i].w = ((*bufu >> 2) & 3) + 1;
			groups[id].list[i].h = ((*bufu++) & 3) + 1;
			buf = (uint16_t*)bufu;
			tmp = *buf++;
			boost::endian::conditional_reverse_inplace<boost::endian::order::big, boost::endian::order::native>(tmp);

			if (game == tSonic2)
				++buf;//Skip two player data

			groups[id].list[i].starttile = tmp & 2047;
			groups[id].list[i].loadat = tmp & 2047;
			groups[id].list[i].prio = (tmp >> 15) & 1;
			groups[id].list[i].palrow = (tmp >> 13) & 3;
			groups[id].list[i].vflip = (tmp >> 12) & 1;
			groups[id].list[i].hflip = (tmp >> 11) & 1;
			bufi = (int8_t*)buf;
			groups[id].list[i].offx = (bufi[0] << 8) | bufi[1];
			++buf;
		}
	}
}
void sprites::handleDPLC(unsigned which, void*buf, unsigned n) {
	unsigned range = 0;
	uint8_t*b8 = (uint8_t*)buf;

	for (unsigned i = n; i--;) {
		range += ((*b8) >> 4) + 1;
		b8 += 2;
	}

	uint32_t*vram = (uint32_t*)malloc(range * sizeof(uint32_t));
	uint32_t*v = vram;
	b8 = (uint8_t*)buf;

	for (unsigned i = n; i--;) {
		for (unsigned i = ((*b8) >> 4) + 1, t = ((*b8 & 15) << 8) | (b8[1]); i--; *v++ = t++);

		b8 += 2;
	}

	for (unsigned i = 0; i < groups[which].list.size(); ++i) {
		if (groups[which].list[i].loadat < range)
			groups[which].list[i].starttile = vram[groups[which].list[i].loadat];
		else
			printf("Out of bounds %u %u\n", which, i);
	}

	free(vram);
}
void sprites::DplcItem(void*in, uint32_t which, gameType_t game) {
	/*Sonic 1 format:
	 * uint8_t amount
	 * for each of amount
	 * uint8_t how many and offset high byte
	 * The high nibbles signifies how many tiles to load - 1
	 * uint8_t offset low byte
	 * */
	//Just ignore the high nibble on the high byte to get start tile
	if (game == tSonic1) {
		char*txt = (char*)in;
		unsigned amtd;

		if (txt = strstr(txt, "dc.b")) {
			txt += sizeof("dc.b") - 1;

			while (isspace(*txt++));

			--txt;

			if (*txt == '$')
				amtd = strtol(txt + 1, &txt, 16);
			else
				amtd = strtol(txt, &txt, 0);

			uint8_t*buf = (uint8_t*)malloc(amtd * 2);

			for (unsigned i = 0; i < amtd * 2; i += 2)
				txt = readNbytesAsm(buf + i, txt, 2);

			handleDPLC(which, buf, amtd);
			free(buf);
		}
	} else {
		//The format is pretty much the same as sonic 1 except amount is now a word instead of a byte
		uint16_t*buf = (uint16_t*)in;
		uint16_t tmp = *buf++;
		boost::endian::conditional_reverse_inplace<boost::endian::order::big, boost::endian::order::native>(tmp);
		unsigned amtd = tmp;
		handleDPLC(which, buf, amtd);
	}
}
bool sprites::alreadyLoaded(uint32_t id, uint32_t subid)const {
	if (!subid)
		return false;

	for (uint32_t i = 0; i < subid; ++i) {
		if (i == subid)
			continue;

		if (groups[id].list[subid].w == groups[id].list[i].w) {
			if (groups[id].list[subid].h == groups[id].list[i].h) {
				if (groups[id].list[subid].starttile == groups[id].list[i].starttile)
					return true;
			}
		}
	}

	return false;
}
bool sprites::checkDupdplc(uint32_t id, uint32_t&which)const {
	if (!id)
		return false;

	for (uint32_t i = 0; i < id; ++i) { //We only search before id in question so later dplc entries will refer to dplc entries before
		if (groups[i].list.size() == groups[id].list.size()) {
			bool match = true;

			for (uint32_t j = 0; j < groups[i].list.size(); ++j) {
				if (groups[i].list[j].starttile != groups[id].list[j].starttile) {
					match = false;
					break;
				}

				if (groups[i].list[j].h != groups[id].list[j].h) {
					match = false;
					break;
				}

				if (groups[i].list[j].w != groups[id].list[j].w) {
					match = false;
					break;
				}
			}

			if (match) {
				which = i;
				return true;
			}
		}
	}

	return false;
}
bool sprites::checkDupmapping(uint32_t id, uint32_t&which)const {
	if (!id)
		return false;

	for (uint32_t i = 0; i < id; ++i) { //Now check what has not already been checked
		if (groups[i].list.size() == groups[id].list.size()) {
			bool match = true;

			for (uint32_t j = 0; j < groups[i].list.size(); ++j) {
				if (groups[i].list[j].palrow != groups[id].list[j].palrow) {
					match = false;
					break;
				}

				if (groups[i].list[j].hflip != groups[id].list[j].hflip) {
					match = false;
					break;
				}

				if (groups[i].list[j].vflip != groups[id].list[j].vflip) {
					match = false;
					break;
				}

				if (groups[i].list[j].prio != groups[id].list[j].prio) {
					match = false;
					break;
				}

				if (groups[i].list[j].h != groups[id].list[j].h) {
					match = false;
					break;
				}

				if (groups[i].list[j].w != groups[id].list[j].w) {
					match = false;
					break;
				}

				if (groups[i].list[j].loadat != groups[id].list[j].loadat) {
					match = false;
					break;
				}

				if (groups[i].list[j].offx != groups[id].list[j].offx) {
					match = false;
					break;
				}

				if (groups[i].list[j].offy != groups[id].list[j].offy) {
					match = false;
					break;
				}
			}

			if (match) {
				which = i;
				return true;
			}
		}
	}

	return false;
}

std::vector<uint8_t> sprites::optDPLC(unsigned which, gameType_t game)const {
	printf("%u\n", which);
	std::vector<unsigned> tmp;//amount,offset
	tmp.reserve(groups[which].list.size());

	for (unsigned i = 0; i < groups[which].list.size(); ++i) {
		tmp.push_back(groups[which].list[i].w * groups[which].list[i].h);
		tmp.push_back(groups[which].list[i].starttile);
	}

	//Remove duplicates
dupIt:

	for (unsigned i = 0; i < tmp.size(); i += 2) {
		for (unsigned j = tmp.size(); j -= 2;) {
			if (i == j)
				continue;

			if ((tmp[i] == tmp[j]) && (tmp[i + 1] == tmp[j + 1])) {
				printf("Duplicate %d %d\n", i / 2, j / 2);
				tmp.erase(tmp.begin() + j, tmp.begin() + j + 2);
				goto dupIt;
			}
		}
	}

	//Merge if possible
	for (int i = 0; i < tmp.size(); i += 2) {
mergeIt:

		for (unsigned j = tmp.size(); j -= 2;) {
			if (i == j)
				continue;

			if (inRange(tmp[j + 1], tmp[i + 1], tmp[i + 1] + tmp[i])) {
				unsigned amtnew = std::max(tmp[i] + tmp[i + 1], tmp[j] + tmp[j + 1]) - std::min(tmp[i + 1], tmp[j + 1]);
				printf("Found merge candidate: %d %d %d %d amount: %u\n", tmp[i], tmp[i + 1], tmp[j], tmp[j + 1], amtnew);
				unsigned st = std::min(tmp[i + 1], tmp[j + 1]);
				bool swapped;

				if (i > j) {
					swapped = true;
					unsigned sw = i;
					i = j;
					j = sw;
				} else
					swapped = false;

				while (amtnew > 16) {
					tmp[i] = 16;
					tmp[i + 1] = st;
					i += 2;
					j += 2;
					st += 16;
					tmp.insert(tmp.begin() + i + 2, 2, 0);
					amtnew -= 16;
				}

				if (amtnew) {
					tmp[i] = amtnew;
					tmp[i + 1] = st;
				}

				tmp.erase(tmp.begin() + j, tmp.begin() + j + 2);

				if (swapped)
					i = i > j ? j : i;

				goto mergeIt;
			}
		}
	}

	std::vector<uint8_t> out;
	out.reserve(tmp.size());

	for (unsigned i = 0; i < tmp.size(); i += 2) {
		unsigned tile = tmp[i + 1];

		if (tile > 4095) {
			printf("Tile overflow in sprite group %d tile value was %d\n", which, tile);
			tile = 4095;
		}

		if (game == tSonic3) {
			out.push_back((tile >> 4));
			out.push_back(((tile & 15) << 4) | ((tmp[i] - 1)));
		} else {
			out.push_back(((tmp[i] - 1) << 4) | (tile >> 8));
			out.push_back(tile & 255);
		}
	}

	tmp.clear();
	return out;
}
void sprites::exportDPLC(gameType_t game)const {
	if (load_file_generic("Save DPLC", true)) {
		FILE*fp;

		if (game == tSonic1) {
			fp = fopen(the_file.c_str(), "w");
			fprintf(fp, ";DPLC generated by %s\n%s_DPLC:\n", rtVersionStr, name.c_str());
			uint32_t dup;

			for (unsigned i = 0; i < amt; ++i) {
				if (!groups[i].list.size())
					fputs("\tdc.w 0\n", fp);
				else if (checkDupdplc(i, dup)) {
					fprintf(fp, "\tdc.w %s-%s\n", groups[dup].name.c_str(), name.c_str());
					printf("Table entry can be optimized: %d %d\n", i, dup);
				} else
					fprintf(fp, "\tdc.w %s-%s\n", groups[i].name.c_str(), name.c_str());
			}

			for (unsigned i = 0; i < amt; ++i) {
				if (groups[i].list.size()) {
					if (!checkDupdplc(i, dup)) {
						fprintf(fp, "%s:\n\tdc.b %d\n", groups[i].name.c_str(), groups[i].list.size());

						for (unsigned j = 0; j < groups[i].list.size(); ++j) {
							if (!alreadyLoaded(i, j))
								fprintf(fp, "\tdc.b %d,%d\n", (((groups[i].list[j].w * groups[i].list[j].h) - 1) << 4) | ((groups[i].list[j].starttile & 4095) >> 8), groups[i].list[j].starttile & 255);
							else
								printf("Already loaded group: %d sprite: %d\n", i, j);
						}
					}
				}
			}
		} else {
			fp = fopen(the_file.c_str(), "wb");
			std::vector<uint8_t> tmpbuf;
			unsigned acum = amt * 2;
			tmpbuf.resize(acum);

			for (unsigned i = 0; i < amt; ++i) {
				uint32_t dup;

				if (!groups[i].list.size()) {
					if (game == tSonic3)
						printf("Null frame sonic 3 warning group %u\n", i);

					tmpbuf[i * 2] = tmpbuf[i * 2 + 1] = 0;
				} else if (checkDupdplc(i, dup)) {
					printf("Table entry can be optimized: %d %d\n", i, dup);
					tmpbuf[i * 2] = tmpbuf[dup * 2];
					tmpbuf[i * 2 + 1] = tmpbuf[dup * 2 + 1];
				} else {
					tmpbuf[i * 2] = acum >> 8;
					tmpbuf[i * 2 + 1] = acum & 255;
					std::vector<uint8_t> out = optDPLC(i, game);
					unsigned amtg = out.size() / 2;

					if (game == tSonic3) {
						if (amtg)
							--amtg;

						else
							printf("Null frame sonic 3 warning group %u\n", i);
					}

					tmpbuf.push_back(amtg >> 8);
					tmpbuf.push_back(amtg & 255);
					tmpbuf.insert(tmpbuf.end(), out.begin(), out.end());
					acum += out.size() + 2;
					out.clear();
				}
			}

			fwrite(tmpbuf.data(), 1, tmpbuf.size(), fp);
			tmpbuf.clear();
		}

		fclose(fp);
	}
}
static char*findLabel(char*txt, const char*label) {
searchagain:

	if (txt = strstr(txt, label)) {
		txt += strlen(label);

		if ((*txt++) != ':')
			goto searchagain;
	}

	return txt;
}
void sprites::importDPLC(gameType_t game) {
	if (load_file_generic("Load DPLC")) {
		FILE*fp;

		if (game == tSonic1)
			fp = fopen(the_file.c_str(), "r");
		else
			fp = fopen(the_file.c_str(), "rb");

		fseek(fp, 0, SEEK_END);
		size_t sz = ftell(fp);
		rewind(fp);
		char*buf = (char*)malloc(sz + 1);
		fread(buf, 1, sz, fp);

		if (game == tSonic1)
			buf[sz] = 0; //Ensure that the C-string is null terminated

		fclose(fp);
		uint32_t sp = 0;

		if (game == tSonic1) {
			char*bufp = buf;
			char*bufend = buf + sz - 1;

			while (bufp < bufend) {
				if (bufp = strstr(bufp, "dc.w")) {
					bufp += strlen("dc.w");

					while (isspace(*bufp++));

					--bufp;
					char*minus = strstr(bufp, "-");

					if (!minus)
						break;

					*minus = 0;

					DplcItem(findLabel(minus + 1, bufp), sp++, game);
					bufp = minus + 1;
				} else
					break;
			}
		} else {
			uint16_t*ptr = (uint16_t*)buf;
			unsigned amtd;

			for (uint16_t*pt = ptr; !(amtd = boost::endian::big_to_native(*pt++) / 2););

			for (unsigned i = 0; i < amtd; ++i) {
				unsigned off = boost::endian::big_to_native(ptr[i]) / 2;
				DplcItem(ptr + off, i, game);
			}
		}

		free(buf);
	}
}
void sprites::importMapping(gameType_t game) {
	bool append = fl_choice("Append new sprites or overwrite", "Overwrite", "Append", 0);

	if (load_file_generic("Load sprite mapping")) {
		unsigned amtnew;

		if (append)
			amtnew = amt;
		else
			amtnew = 0;

		FILE*fp;

		if (game == tSonic1)
			fp = fopen(the_file.c_str(), "r");
		else
			fp = fopen(the_file.c_str(), "rb");

		fseek(fp, 0, SEEK_END);
		size_t sz = ftell(fp);
		rewind(fp);
		char*buf = (char*)malloc(sz + 1);
		fread(buf, 1, sz, fp);

		if (game == tSonic1)
			buf[sz] = 0; //Ensure that the C-string is null terminated

		fclose(fp);
		char*bufp = buf;
		char*bufend = buf + sz - 1;

		if (game == tSonic1) {
			while (bufp < bufend) {
				if (bufp = strstr(bufp, "dc.w")) {
					bufp += strlen("dc.w");

					while (isspace(*bufp++));

					--bufp;
					char*minus = strstr(bufp, "-");

					if (!minus)
						break;

					*minus = 0;
					++amtnew;
					setAmt(amtnew);
					mappingItem(findLabel(minus + 1, bufp), amt - 1, game);
					groups[amtnew - 1].name.assign(bufp);
					bufp = minus + 1;
					//The dc.w pseudo-op can contain multiple words
					char*end = bufp;

					while ((isalnum(*end)) || (*end == '_'))
						++end;

					char endold = *end;
					*end = 0;
					name.assign(bufp);
					*end = endold;
					bufp = end;

					while (1) {
						char*comma = strstr(bufp, ",");

						if (!comma)
							break;

						char*nl = strstr(bufp, "\n"); //New Line

						if (!nl)
							break;

						if (nl && (comma < nl)) {
							//Label on same line found
							bufp = comma + 1;

							while (isspace(*bufp++));

							--bufp;
							minus = strstr(bufp, "-");
							*minus = 0;
							++amtnew;
							setAmt(amtnew);
							mappingItem(strstr(minus + 1, bufp) + strlen(bufp), amt - 1, game);
							groups[amtnew - 1].name.assign(bufp);
							bufp = minus + 1;
							comma = strstr(bufp, ",");

							if ((!comma) || (comma > nl)) //Is the next comma on a newline?
								break;
						} else
							break;
					}
				} else
					break;
			}
		} else {
			//Sonic 2 and Sonic 3's mapping is usually stored in binary
			name.assign(fl_filename_name(the_file.c_str()));
			uint16_t*ptr = (uint16_t*)buf;
			unsigned off;

			for (uint16_t*pt = ptr; !(off = boost::endian::big_to_native(*pt++) / 2););

			setAmt(off);//First nonzero pointer can be used to find amount just divide by two

			for (unsigned i = 0; i < amt; ++i) {
				off = boost::endian::big_to_native(ptr[i]) / 2;
				mappingItem(ptr + off, i, game);
				groups[i].name.assign(fl_filename_name(the_file.c_str()));
				char tmp[16];
				snprintf(tmp, 16, "_%u", i);
				groups[i].name.append(tmp);
			}

		}

		free(buf);
	}
}
static uint8_t*rect2rect(uint8_t*in, uint8_t*out, unsigned xin, unsigned yin, unsigned win, unsigned wout, unsigned hout, unsigned depth, bool reverse = false) {
	in += (yin * win * depth) + (xin * depth);

	while (hout--) {
		if (depth == 4) {
			if (reverse)
				memcpy(in, out, wout * 4);
			else
				memcpy(out, in, wout * 4);

			in += win * 4;
			out += wout * 4;
		} else if (depth == 3) {
			if (reverse) {
				for (unsigned i = 0; i < wout; ++i) {
					*in++ = *out++;
					*in++ = *out++;
					*in++ = *out++;
					++out;
				}
			} else {
				for (unsigned i = 0; i < wout; ++i) {
					*out++ = *in++;
					*out++ = *in++;
					*out++ = *in++;
					*out++ = 255;
				}
			}

			in += (win - wout) * 3;
		} else if (depth == 1) {
			if (reverse)
				memcpy(in, out, wout);
			else
				memcpy(out, in, wout);

			in += win;
			out += wout;
		}
	}

	return out;
}
void sprites::spriteGroupToImage(uint8_t*img, uint32_t id, int row, bool alpha) {
	int32_t miny, maxy, minx, maxx;
	minmaxoffy(id, miny, maxy);
	minmaxoffx(id, minx, maxx);
	uint32_t w = abs(maxx - minx);
	uint32_t h = abs(maxy - miny);
	unsigned bpp;//Bytes per pixel

	if (alpha)
		bpp = 4;
	else
		bpp = 3;

	memset(img, 0, w * h * bpp);

	for (uint32_t i = 0; i < groups[id].list.size(); ++i) {
		int32_t xoff = groups[id].list[i].offx;
		int32_t yoff = groups[id].list[i].offy;
		xoff -= minx;
		yoff -= miny;
		uint32_t ttile = groups[id].list[i].starttile;

		if ((row != groups[id].list[i].palrow) && (row >= 0))
			continue;//Skip if we only want a specific row

		for (uint32_t x = 0; x < groups[id].list[i].w * prj->tileC->width(); x += prj->tileC->width()) {
			for (uint32_t y = 0; y < groups[id].list[i].h * prj->tileC->height(); y += prj->tileC->height(), ++ttile) {
				uint8_t*outptr = prj->tileC->truetDat.data() + (ttile * prj->tileC->tcSize);
				rect2rect(img, outptr, xoff + x, yoff + y, w, prj->tileC->width(), prj->tileC->height(), alpha ? 4 : 3, true);
			}
		}
	}
}
void sprites::spriteImageToTiles(uint8_t*img, uint32_t id, int rowUsage, bool alpha, bool isIndexArray) {
	int32_t miny, maxy, minx, maxx;
	minmaxoffy(id, miny, maxy);
	minmaxoffx(id, minx, maxx);
	uint8_t tcTemp[256];
	uint32_t w = abs(maxx - minx);
	uint32_t h = abs(maxy - miny);

	for (uint32_t i = 0; i < groups[id].list.size(); ++i) {
		int32_t xoff = groups[id].list[i].offx;
		int32_t yoff = groups[id].list[i].offy;
		xoff -= minx;
		yoff -= miny;
		uint32_t ttile = groups[id].list[i].starttile;

		if ((rowUsage != groups[id].list[i].palrow) && (rowUsage >= 0))
			continue;//Skip if we only want a specific row

		for (uint32_t x = 0; x < groups[id].list[i].w * prj->tileC->width(); x += prj->tileC->width()) {
			for (uint32_t y = 0; y < groups[id].list[i].h * prj->tileC->height(); y += prj->tileC->height(), ++ttile) {
				rect2rect(img, tcTemp, xoff + x, yoff + y, w, prj->tileC->width(), prj->tileC->height(), isIndexArray ? 1 : (alpha ? 4 : 3), false);
				prj->tileC->truecolor_to_tile_ptr(groups[id].list[i].palrow, ttile, tcTemp, false, true, isIndexArray);
			}
		}
	}
}
void sprites::minmaxoffy(uint32_t id, int32_t&miny, int32_t&maxy)const {
	miny = maxy = groups[id].list[0].offy;

	for (uint32_t i = 0; i < groups[id].list.size(); ++i) {
		if (groups[id].list[i].offy < miny)
			miny = groups[id].list[i].offy;

		int32_t tmpy = groups[id].list[i].offy + (groups[id].list[i].h * prj->tileC->height());

		if (tmpy > maxy)
			maxy = tmpy;
	}
}
void sprites::minmaxoffx(uint32_t id, int32_t&minx, int32_t&maxx)const {
	minx = maxx = groups[id].list[0].offx;

	for (uint32_t i = 0; i < groups[id].list.size(); ++i) {
		if (groups[id].list[i].offx < minx)
			minx = groups[id].list[i].offx;

		int32_t tmpx = groups[id].list[i].offx + (groups[id].list[i].w * prj->tileC->width());

		if (tmpx > maxx)
			maxx = tmpx;
	}
}
uint32_t sprites::width(uint32_t id)const {
	int32_t minx, maxx;
	minmaxoffx(id, minx, maxx);
	return abs(maxx - minx);
}
uint32_t sprites::height(uint32_t id)const {
	int32_t miny, maxy;
	minmaxoffy(id, miny, maxy);
	return abs(maxy - miny);
}
void sprites::draw(uint32_t id, uint32_t x, uint32_t y, int32_t zoom, bool mode, int32_t*outx, int32_t*outy) {
	if (groups[id].list.size()) {
		int32_t minx, maxx;
		int32_t miny, maxy;

		if (!mode) {
			minmaxoffx(id, minx, maxx);
			minmaxoffy(id, miny, maxy);
		}

		maxx = maxy = 0;

		for (uint32_t i = 0; i < groups[id].list.size(); ++i) {
			int xoff = x + (groups[id].list[i].offx * zoom);

			if (!mode)
				xoff -= minx * zoom;

			int yoff = y + (groups[id].list[i].offy * zoom);

			if (!mode)
				yoff -= miny * zoom;

			if (maxx < (xoff + (groups[id].list[i].w * zoom * 8)))
				maxx = xoff + (groups[id].list[i].w * zoom * 8);

			if (maxy < (yoff + (groups[id].list[i].h * zoom * 8)))
				maxy = yoff + (groups[id].list[i].h * zoom * 8);

			if (xoff >= window->w())
				continue;

			if (yoff >= window->h())
				continue;

			groups[id].list[i].draw(xoff, yoff, zoom);
		}

		if (outx)
			*outx = maxx;

		if (outy)
			*outy = maxy;
	}
}
void sprites::setAmt(uint32_t amtnew) {
	if (amtnew > amt) {
		//Create more sprites with default parameter
		groups.resize(amtnew);

		for (unsigned n = amt; n < amtnew; ++n) {
			groups[n].list.push_back(sprite());
			groups[n].name.assign(spriteDefName);
		}
	} else if (amtnew < amt) {
		for (unsigned n = amtnew; n < amt; ++n) {
			groups[n].name.clear();
			groups[n].list.clear();
		}

		groups.resize(amtnew);
	}

	amt = amtnew;
}
void sprites::setAmtingroup(uint32_t id, uint32_t amtnew) {
	uint32_t amtold = groups[id].list.size();

	if (amtold == amtnew)
		return;

	groups[id].list.resize(amtnew);
}
bool sprites::save(FILE*fp)const {
	/* Format:
	 * uint32_t group amount
	 * uint8_t extra DPLC optimization
	 * Null terminated string containing name (all sprites)
	 * for each group
	 * uint32_t sprite amount
	 * Null terminated sprite group name or just 0 if default name
	 * for each sprite in group
	 * int32_t offset x
	 * int32_t offset y
	 * uint32_t loadat
	 * uint32_t w
	 * uint32_t h
	 * uint32_t starttile
	 * uint32_t pal row
	 * uint8_t hvflip flags bit 0 hflip bit 1 vflip bit 2 priority
	 */
	fwrite(&amt, sizeof(uint32_t), 1, fp);
	uint8_t tmpOpt = extraOptDPLC;
	fwrite(&tmpOpt, sizeof(uint8_t), 1, fp);

	if (strcmp(name.c_str(), spritesName))
		fputs(name.c_str(), fp);

	fputc(0, fp);

	for (unsigned n = 0; n < amt; ++n) {
		uint32_t amtgroup = groups[n].list.size();
		fwrite(&amtgroup, sizeof(uint32_t), 1, fp);

		if (strcmp(groups[n].name.c_str(), spriteDefName))
			fputs(groups[n].name.c_str(), fp);

		fputc(0, fp);

		for (uint32_t i = 0; i < amtgroup; ++i) {
			fwrite(&groups[n].list[i].offx, sizeof(int32_t), 1, fp);
			fwrite(&groups[n].list[i].offy, sizeof(int32_t), 1, fp);
			fwrite(&groups[n].list[i].loadat, sizeof(uint32_t), 1, fp);
			fwrite(&groups[n].list[i].w, sizeof(uint32_t), 1, fp);
			fwrite(&groups[n].list[i].h, sizeof(uint32_t), 1, fp);
			fwrite(&groups[n].list[i].starttile, sizeof(uint32_t), 1, fp);
			fwrite(&groups[n].list[i].palrow, sizeof(uint32_t), 1, fp);
			uint8_t hvflip = groups[n].list[i].hflip | (groups[n].list[i].vflip << 1) | (groups[n].list[i].prio << 2);
			fwrite(&hvflip, sizeof(uint8_t), 1, fp);
		}
	}

	return true;
}
bool sprites::load(FILE*fp, uint32_t version) {
	if (version >= 7) {
		uint32_t amtnew;
		fread(&amtnew, sizeof(uint32_t), 1, fp);

		if (version >= 8) {
			uint8_t tmpOpt;
			fread(&tmpOpt, sizeof(uint8_t), 1, fp);
			extraOptDPLC = tmpOpt;
		} else
			extraOptDPLC = false;

		setAmt(amtnew);
		char a = fgetc(fp);

		if (a) {
			name.clear();

			do {
				name.push_back(a);
			} while (a = fgetc(fp));
		}

		if (window)
			window->spriteglobaltxt->value(name.c_str());

		for (unsigned n = 0; n < amt; ++n) {
			uint32_t amtgroup;
			fread(&amtgroup, sizeof(int32_t), 1, fp);
			setAmtingroup(n, amtgroup);
			char a = fgetc(fp);

			if (a) {
				groups[n].name.clear();

				do {
					groups[n].name.push_back(a);
				} while (a = fgetc(fp));
			}

			for (uint32_t i = 0; i < amtgroup; ++i) {
				fread(&groups[n].list[i].offx, sizeof(int32_t), 1, fp);
				fread(&groups[n].list[i].offy, sizeof(int32_t), 1, fp);
				fread(&groups[n].list[i].loadat, sizeof(uint32_t), 1, fp);
				fread(&groups[n].list[i].w, sizeof(uint32_t), 1, fp);
				fread(&groups[n].list[i].h, sizeof(uint32_t), 1, fp);
				fread(&groups[n].list[i].starttile, sizeof(uint32_t), 1, fp);
				fread(&groups[n].list[i].palrow, sizeof(uint32_t), 1, fp);
				uint8_t hvflip;
				fread(&hvflip, sizeof(uint8_t), 1, fp);
				groups[n].list[i].hflip = hvflip & 1;
				groups[n].list[i].vflip = (hvflip & 2) >> 1;
				groups[n].list[i].prio = (hvflip & 4) >> 2;
			}
		}
	} else {
		/* Old format
		 * uint32_t amount
		 * And for each sprite:
		 * uint32_t width
		 * uint32_t height
		 * uint32_t start tile
		 * uint32_t pal row*/
		uint32_t amtnew;
		fread(&amtnew, sizeof(uint32_t), 1, fp);
		setAmt(amtnew);

		for (unsigned n = 0; n < amt; ++n) {
			fread(&groups[n].list[0].w, sizeof(uint32_t), 1, fp);
			fread(&groups[n].list[0].h, sizeof(uint32_t), 1, fp);
			fread(&groups[n].list[0].starttile, sizeof(uint32_t), 1, fp);
			fread(&groups[n].list[0].palrow, sizeof(uint32_t), 1, fp);
		}
	}

	return true;
}
void sprites::importImg(uint32_t to) {
	if (load_file_generic("Load image")) {
		Fl_Shared_Image * loaded_image = Fl_Shared_Image::get(the_file.c_str());

		if (!loaded_image) {
			fl_alert("Error loading image");
			return;
		}

		unsigned depth = loaded_image->d();

		if (unlikely(depth != 3 && depth != 4 && depth != 1)) {
			fl_alert("Please use color depth of 1,3 or 4\nYou Used %d", loaded_image->d());
			loaded_image->release();
			return;
		} else
			printf("Image depth %d\n", loaded_image->d());

		uint8_t * img_ptr = (uint8_t *)loaded_image->data()[0];
		groups[to].name.assign(fl_filename_name(the_file.c_str()));

		if (depth == 1)
			img_ptr = (uint8_t*)loaded_image->data()[2];

		bool grayscale;
		uint8_t*palMap;
		unsigned remap[256];

		if (depth == 1) {
			grayscale = handle1byteImg(loaded_image, remap);

			if (!grayscale) {
				palMap = (uint8_t*)loaded_image->data()[1];
				img_ptr = (uint8_t*)loaded_image->data()[2];
			}
		}

		bool useMask = fl_ask("Use mask color?");
		uint8_t mask[3];
		bool useAlpha = false;

		if (useMask) {
			if (!getMaskColorImg(loaded_image, grayscale, remap, palMap, mask, useAlpha)) {
				loaded_image->release();
				return;
			}
		}

		recttoSprite(0, loaded_image->w() - 1, 0, loaded_image->h() - 1, to, loaded_image, grayscale, remap, palMap, mask, useMask, useAlpha);
		loaded_image->release();
		window->updateSpriteSliders();
		updateTileSelectAmt();
		window->redraw();
	}
}
void sprites::del(uint32_t id) {
	if (amt <= 1) {
		fl_alert("If you want no sprites uncheck have sprites instead.");
		return;
	}

	if (id < amt) {
		groups[id].name.clear();
		groups[id].list.clear();
		groups.erase(groups.begin() + id);
		--amt;
	} else
		fl_alert("You cannot delete what does not exist");
}
void sprites::delingroup(uint32_t id, uint32_t subid) {
	uint32_t amtold = groups[id].list.size();

	if (amtold <= 0) {
		fl_alert("Delete the entire group instead if that is what you want");
		return;
	}

	if (subid < amtold)
		groups[id].list.erase(groups[id].list.begin() + subid);

	else
		fl_alert("You cannot delete what does not exist");
}
void sprites::enforceMax(unsigned wmax, unsigned hmax) {
	for (unsigned j = 0; j < amt; ++j) {
		unsigned told = groups[j].list.size();
		struct spriteGroup& group = groups[j];

		for (unsigned i = 0; i < told; ++i) {
			struct sprite& gli = group.list[i]; // gli = Group List[I].

			if ((gli.w > wmax) || (gli.h > hmax)) {
				//Divide it up into more sprites
				unsigned w = gli.w;
				unsigned h = gli.h;
				unsigned nSpritesW = ((w + (wmax - 1)) / wmax);
				unsigned nSpritesH = ((h + (hmax - 1)) / hmax);
				unsigned snew = nSpritesW * nSpritesH - 1;
				unsigned start = group.list.size();
				unsigned st = gli.starttile;
				unsigned la = gli.loadat;

				if (gli.w > wmax)
					gli.w = wmax;

				if (gli.h > hmax)
					gli.h = hmax;

				st += gli.h * gli.w;
				la += gli.h * gli.w;
				setAmtingroup(j, start + snew);

				for (unsigned x = 0, a = start; x < nSpritesW; ++x) {
					struct sprite& gla = group.list[a];

					for (unsigned y = 0; y < nSpritesH; ++y) {
						if ((!x) && (!y)) // The original sprite is already corrected.
							continue;

						if (x == (w / wmax))
							gla.w = w % wmax;
						else
							gla.w = wmax;

						if (y == (h / hmax))
							gla.h = h % hmax;
						else
							gla.h = hmax;

						gla.offx = gli.offx + (x * wmax * 8);
						gla.offy = gli.offy + (y * hmax * 8);
						gla.starttile = st;
						gla.loadat = la;
						st += gla.w * gla.h;
						la += gla.w * gla.h;
						++a;
					}
				}
			}
		}
	}
}
void sprites::allToPalRow(unsigned palRow) {
	for (unsigned j = 0; j < amt; ++j) {
		for (unsigned i = 0; i < groups[j].list.size(); ++i)
			groups[j].list[i].palrow = palRow;
	}
}
