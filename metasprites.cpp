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
#include <cstring>

#include "metasprites.h"

const char*defMDesc = "DefaultForAllMetasprites";
metasprites::metasprites(Project*prj) {
	this->prj = prj;
	sps.emplace_back(sprites(prj));
	name.assign(defMDesc);
}
metasprites::metasprites(const metasprites&o, Project*prj) {
	this->prj = prj;
	sps = o.sps;
}
void metasprites::setPrjPtr(Project*prj) {
	this->prj = prj;

	for (unsigned i = 0; i < sps.size(); ++i)
		sps[i].prj = prj;
}
void metasprites::save(FILE*fp) {
	if (strcmp(name.c_str(), defMDesc) != 0)
		fputs(name.c_str(), fp);

	fputc(0, fp);
	uint32_t amt = sps.size();
	fwrite(&amt, sizeof(uint32_t), 1, fp);

	for (uint32_t i = 0; i < amt; ++i)
		sps[i].save(fp);
}
void metasprites::load(FILE*fp, uint32_t version) {
	if (version >= 8) {
		char firstC = fgetc(fp);

		if (firstC) {
			name.clear();

			do {
				name.push_back(firstC);
			} while ((firstC = fgetc(fp)));
		} else
			name.assign(defMDesc);

		uint32_t amt;
		fread(&amt, sizeof(uint32_t), 1, fp);
		sps.clear();
		sps.resize(amt, sprites(prj));

		for (uint32_t i = 0; i < amt; ++i)
			sps[i].load(fp, version);
	} else {
		name.assign(defMDesc);
		sps.clear();
		sps.resize(1, sprites(prj));
		sps[0].load(fp, version);
	}
}
