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
#pragma once
extern const uint8_t palTabGameGear[];
extern const uint8_t palTabMasterSystem[];
extern const uint8_t*palTabPtr[];
extern const uint8_t*palTab;
extern const uint8_t palTabGenReal[];
extern const uint8_t palTabGen255div7[];
extern const uint8_t palTabGen36[];
extern const uint8_t palTabGen32[];
extern unsigned palTypeGen;
void sortBy(unsigned type, bool perRow);
void set_palette_type(void);
void set_palette_type_force(unsigned type);
