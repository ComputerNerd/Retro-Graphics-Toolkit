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
#include <cstring>

#include "project.h"
#include "classpalette.h"
#include "CIE.h"
#include "nearestColor.h"
unsigned find_near_color_from_row_rgb(unsigned row, int r, int g, int b, bool alt) {
	unsigned pr = currentProject->pal->getPerRow(alt);

	if (currentProject->pal->haveAlt && alt)
		row = currentProject->pal->colorCnt + (row * pr);
	else
		row *= pr;

	const uint8_t*rgbPtr = currentProject->pal->rgbPal + (row * 3);

	unsigned nearestIndex = currentProject->pal->nearestColIndex(r, g, b, rgbPtr, pr, true, row);

	return (nearestIndex + row) * 3; //Yes this function does return three times the value. TODO refactor
}
unsigned find_near_color_from_row(unsigned row, int r, int g, int b, bool alt) {
	return (find_near_color_from_row_rgb(row, r, g, b, alt) / 3) - (row * currentProject->pal->getPerRow(alt));
}
unsigned chooseTwoColor(unsigned index0, unsigned index1, int rgoal, int ggoal, int bgoal) {
	uint8_t tmp[6];
	memcpy(tmp, currentProject->pal->rgbPal + (index0 * 3), 3);
	memcpy(tmp + 3, currentProject->pal->rgbPal + (index1 * 3), 3);
	bool second = currentProject->pal->nearestColIndex(rgoal, ggoal, bgoal, tmp, 2, false, 0);

	if (second && (currentProject->pal->palType[index1] != 2))
		return index1;
	else if (currentProject->pal->palType[index0] != 2)
		return index0;
	else
		return 0;
}
unsigned nearestOneChannel(int val, const uint8_t*pal, unsigned amt) {
	int_fast32_t distanceSquared, minDistanceSquared;
	unsigned bestIndex = 0;
	minDistanceSquared = 255 * 255 + 1;

	for (unsigned i = 0; i < amt; ++i) {
		int32_t Rdiff = val - (int)pal[i];
		distanceSquared = Rdiff * Rdiff;

		if (distanceSquared < minDistanceSquared) {
			minDistanceSquared = distanceSquared;
			bestIndex = i;
		}
	}

	return bestIndex;
}
static inline int sq(int x) {
	return x * x;
}
static inline double sqd(double x) {
	return x * x;
}
unsigned palette::nearestColIndex(int red, int green, int blue, const uint8_t*pal, unsigned amt, bool checkType, unsigned off) {
	unsigned bestcolor = 0;

	switch (((prj->settings) >> nearestColorShift)&nearestColorSettingsMask) {
		case aCiede2000:
		{
			double minerrord = 1e99;

			for (int i = (amt - 1) * 3; i >= 0; i -= 3) {
				double distance = ciede2000rgb(red, green, blue, pal[i], pal[i + 1], pal[i + 2]);

				if (!checkType || (palType[i / 3 + off] != 2)) {
					if (distance < minerrord) {
						minerrord = distance;
						bestcolor = i;
					}
				}
			}
		}
		break;

		case aWeighted:
		{
			uint32_t minerrori = 0xFFFFFFFF;

			for (int i = (amt - 1) * 3; i >= 0; i -= 3) {
				uint32_t distance = ColourDistance(red, green, blue, pal[i], pal[i + 1], pal[i + 2]);

				if (!checkType || (palType[i / 3 + off] != 2)) {
					if (distance < minerrori) {
						minerrori = distance;
						bestcolor = i;
					}
				}
			}
		}
		break;

		case aCIE76:
		{
			double minerrord = 1e99;

			for (int i = (amt - 1) * 3; i >= 0; i -= 3) {
				double L1, L2, a1, a2, b1, b2;
				Rgb2Lab255(&L1, &a1, &b1, red, green, blue);
				Rgb2Lab255(&L2, &a2, &b2, pal[i], pal[i + 1], pal[i + 2]);
				double distance = sqd(L1 - L2) + sqd(a1 - a2) + sqd(b1 - b2);

				if (!checkType || (palType[i / 3 + off] != 2)) {
					if (distance < minerrord) {
						minerrord = distance;
						bestcolor = i;
					}
				}
			}
		}
		break;

		default:
		{
			int minerrori = (255 * 255) + (255 * 255) + (255 * 255) + 1;

			for (int i = (amt - 1) * 3; i >= 0; i -= 3) {
				int distance = sq((int)pal[i] - red) + sq((int)pal[i + 1] - green) + sq((int)pal[i + 2] - blue);

				if (!checkType || (palType[i / 3 + off] != 2)) {
					if (distance < minerrori) {
						minerrori = distance;
						bestcolor = i;
					}
				}
			}

		}

	}

	return bestcolor / 3;
}
