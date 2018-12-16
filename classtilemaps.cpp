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
#include "classtilemaps.h"
tilemaps::tilemaps(Project*prj) {
	this->prj = prj;
	maps.emplace_back(prj);
	maps[0].planeName.assign("0");
}
tilemaps::tilemaps(const tilemaps&other, Project*prj) {
	this->prj = prj;
	unsigned cnt = other.maps.size();
	maps.reserve(cnt);

	for (unsigned i = 0; i < cnt; ++i)
		maps.emplace_back(other.maps[i], prj);
}
void tilemaps::setPlaneCnt(unsigned cnt) {
	unsigned oldCnt = maps.size();

	if (cnt > oldCnt) {
		maps.reserve(cnt);

		for (unsigned i = oldCnt; i < cnt; ++i) {
			char tmp[16];
			snprintf(tmp, 16, "%u", i);
			maps.emplace_back(tileMap(prj));
			maps[i].planeName.assign(tmp);
		}
	} else if (cnt < oldCnt) {
		for (int i = oldCnt; i > (int)cnt; --i)
			maps.pop_back();
	}
}
void tilemaps::changePrjPtr(Project*prj) {
	this->prj = prj;

	for (unsigned i = 0; i < maps.size(); ++i)
		maps[i].prj = prj;
}
