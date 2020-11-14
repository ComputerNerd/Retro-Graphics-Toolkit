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
	Copyright Sega16 (or whatever you wish to call me) (2012-2019)
*/
#include <FL/fl_ask.H>
#include <string.h>
#include "classlevel.h"
#include "filemisc.h"
level::level(Project*prj) {
	this->prj = prj;
	layeramt = 1;
	lvlI.resize(1, {1, 1, 1, 1, 0, 0, CHUNKS});
	dat.resize(1);
	odat.resize(1);
	dat[0] = new std::vector<struct levDat>;
	dat[0]->resize(1);
	odat[0] = new std::vector<struct levobjDat>;
	layernames.emplace_back("0");
}
level::level(const level&o, Project*prj) {
	this->prj = prj;
	layeramt = o.layeramt;
	lvlI = o.lvlI;
	layernames = o.layernames;
	dat.resize(o.dat.size());
	odat.resize(o.odat.size());

	for (unsigned i = 0; i < o.dat.size(); ++i)
		dat[i] = new std::vector<struct levDat>(*o.dat[i]);

	for (unsigned i = 0; i < o.odat.size(); ++i)
		odat[i] = new std::vector<struct levobjDat>(*o.odat[i]);
}
void level::addLayer(unsigned at, bool after) {
	unsigned base = at;

	if (after)
		++at;

	lvlI.insert(lvlI.begin() + at, lvlI[base]);
	std::vector<struct levDat>*vtmp = new std::vector<struct levDat>;
	dat.insert(dat.begin() + at, vtmp);
	dat[at]->resize(lvlI[at].w * lvlI[at].h);
}
void level::removeLayer(unsigned which) {
	lvlI.erase(lvlI.begin() + which);
	delete dat[which];
	dat.erase(dat.begin() + which);
	delete odat[which];
	odat.erase(odat.begin() + which);
	layernames.erase(layernames.begin() + which);
}
bool level::inRangeLayer(unsigned tst) {
	if (tst >= layeramt) {
		fl_alert("Level layer out of range error");
		return false;
	} else
		return true;
}
struct levelInfo*level::getInfo(unsigned layer) {
	return& lvlI.at(layer);
}
void level::setInfo(unsigned layer, struct levelInfo i) {
	lvlI[layer] = i;
}
struct levDat*level::getlevDat(unsigned layer, unsigned x, unsigned y) {
	return&(dat.at(layer)->at(y * lvlI[layer].w + x));
}
void level::setlevDat(unsigned layer, unsigned x, unsigned y, struct levDat d) {
	(*(dat[layer]))[y * lvlI[layer].w + x] = d;
}
struct levobjDat*level::getObjDat(unsigned layer, unsigned idx) {
	return&(odat.at(layer)->at(idx));
}
void level::setlevObjDat(unsigned layer, unsigned idx, struct levobjDat d) {
	(*(odat[layer]))[idx] = d;
}
void level::setlayeramt(unsigned amt, bool lastLayerDim) {
	if (amt <= 0) {
		fprintf(stderr, "level:setlayeramt called with invalid amount (%u). Forcing amount to one.\n", amt);
		amt = 1;
	}

	if (amt > layeramt) {
		if (lastLayerDim)
			lvlI.reserve(amt);
		else {
			lvlI.resize(amt, {1, 1, 1, 1, 0, 0, CHUNKS});
		}

		dat.reserve(amt);
		odat.reserve(amt);
		layernames.reserve(amt);

		for (unsigned i = layeramt; i < amt; ++i) {
			if (lastLayerDim)
				lvlI.push_back(lvlI[layeramt - 1]);

			std::vector<struct levDat>*vtmp = new std::vector<struct levDat>;
			vtmp->resize(lvlI[i].w * lvlI[i].h);
			dat.push_back(vtmp);
			std::vector<struct levobjDat>*votmp = new std::vector<struct levobjDat>;
			odat.push_back(votmp);
			layernames.emplace_back(std::to_string(i));
		}
	} else if (amt < layeramt) {
		for (unsigned i = amt; i < layeramt; ++i) {
			delete dat[i];
			delete odat[i];
		}

		lvlI.resize(amt);
		dat.resize(amt);
		odat.resize(amt);
		layernames.resize(amt);
	}

	layeramt = amt;
}
void level::resizeLayer(unsigned idx, unsigned nw, unsigned nh) {
	std::vector<struct levDat>tmp = std::vector<struct levDat>(*(dat[idx]));
	unsigned ow = lvlI[idx].w, oh = lvlI[idx].h;
	dat[idx]->resize(nw * nh);

	for (unsigned y = 0; y < std::min(oh, nh); ++y)
		std::copy(tmp.begin() + y * ow, tmp.begin() + y * ow + std::min(ow, nw), dat[idx]->begin() + y * nw);

	lvlI[idx].w = nw;
	lvlI[idx].h = nh;
}
void level::save(FILE*fp) const {
	/*Format
	 * uint32_t layers amount
	 * For each layer info struct
	 * Data see struct levDat
	 * uint32_t objects amount
	 * Data see struct levobjDat*/
	fwrite(&layeramt, sizeof(uint32_t), 1, fp);

	for (unsigned i = 0; i < layeramt; ++i) {
		saveStrifNot(fp, layernames[i].c_str(), std::to_string(i).c_str());
		fwrite(&lvlI[i], sizeof(struct levelInfo), 1, fp);
		fwrite(dat[i]->data(), dat[i]->size(), sizeof(struct levDat), fp);
		uint32_t objamt = odat[i]->size();
		fwrite(&objamt, 1, sizeof(uint32_t), fp);

		if (objamt)
			fwrite(odat[i]->data(), odat[i]->size(), sizeof(struct levobjDat), fp);
	}
}
void level::load(FILE*fp, uint32_t version) {
	uint32_t amtnew;
	fread(&amtnew, 1, sizeof(uint32_t), fp);

	if (amtnew <= 0) {
		fprintf(stderr, "Invalid number of level layers detected (%u). Skipping loading levels.\n", amtnew);
		return;
	}

	setlayeramt(amtnew, false);

	for (unsigned i = 0; i < layeramt; ++i) {
		fileToStr(fp, layernames[i], std::to_string(i).c_str());
		uint32_t objamt;
		fread(&lvlI[i], sizeof(struct levelInfo), 1, fp);
		dat[i]->resize(lvlI[i].w * lvlI[i].h);
		fread(dat[i]->data(), sizeof(struct levDat), lvlI[i].w * lvlI[i].h, fp);
		fread(&objamt, sizeof(uint32_t), 1, fp);

		if (objamt) {
			odat[i]->resize(objamt);
			fread(odat[i]->data(), sizeof(struct levobjDat), odat[i]->size(), fp);
		}
	}
}
void level::subType(unsigned oid, unsigned nid, enum source s, int plane) {
	uint_fast32_t x, y, i;
	int_fast32_t temp;

	for (i = 0; i < layeramt; ++i) {
		struct levelInfo*in = getInfo(i);

		if ((in->src & 3) == s) {
			if (s == BLOCKS) {
				if (plane >= 0) {
					if (plane == in->src >> 2)
						continue;
				}
			}

			for (y = 0; y < in->h; ++y) {
				for (x = 0; x < in->w; ++x) {
					struct levDat*l = getlevDat(i, x, y);

					if (l->id == oid)
						l->id = nid;
					else if (l->id > oid) {
						if (l->id > 0)
							l->id--;
					}
				}
			}
		}
	}
}
