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
bool getMaskColorImg(Fl_Shared_Image*loaded_image, bool grayscale, unsigned*remap, uint8_t*palMap, uint8_t*mask, bool&alphaSel); //mask must pointer to an array or byte with atleast 1 byte for 1 bpp images or 3 bytes for 3 bpp and 4 bpp images returns true if ok false if cancel
bool handle1byteImg(Fl_Shared_Image*loaded_image, unsigned*remap, unsigned*numcol = nullptr); //Returns true if grayscale false if gif
