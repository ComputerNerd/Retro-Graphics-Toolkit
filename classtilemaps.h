/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
*/
#pragma once
#include "includes.h"
#include "classtilemap.h"
class tilemaps{
public:
	std::vector<class tileMap> maps;
	std::vector<std::string> planeName;
	tilemaps();
	tilemaps(const tilemaps&other);
	void setPlaneCnt(unsigned cnt);
	void assignNum(unsigned i){
		char tmp[16];
		snprintf(tmp,16,"%u",i);
		planeName[i].assign(tmp);
	}
};
