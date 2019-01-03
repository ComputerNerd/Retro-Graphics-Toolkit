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
#include <boost/algorithm/clamp.hpp> // C++14 does not provide clamp, this will be available in C++17.
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <stdint.h>

#include "CIE.h"
#include "macros.h"
#include "palette.h"
#include "project.h"
#include "classpalette.h"
#include "system.h"
#include "color_convert.h"
#include "nearestColor.h"
#include "gui.h"
#include "errorMsg.h"
#include "filereader.h"
const uint8_t TMS9918Palette[] = {
	0,   0,   0,
	0,   0,   0,
	33, 200,  66,
	94, 220, 120,
	84,  85, 237,
	125, 118, 252,
	212,  82,  77,
	66, 235, 245,
	252,  85,  84,
	255, 121, 120,
	212, 193,  84,
	230, 206, 128,
	33, 176,  59,
	201,  91, 186,
	204, 204, 204,
	255, 255, 255
};
palette::palette(Project*prj) {
	this->prj = prj;
	rgbPal = nullptr;
	setVars(prj->gameSystem);
	memset(rgbPal, 0, totalMemoryUsage());
}
palette::~palette(void) {
	free(rgbPal);
}
palette::palette(const palette&other, Project*prj) {
	this->prj = prj;
	rgbPal = nullptr;
	setVars(prj->gameSystem);
	memcpy(rgbPal, other.rgbPal, std::min(totalMemoryUsage(), other.totalMemoryUsage()));
}
void palette::setVars(enum gameSystemEnum gameSystem) {
	fixedSpriteRow = prj->fixedSpirtePalRow();

	switch (gameSystem) {
		case segaGenesis:
			rowCntPal = 4;
			colorCnt = 64;
			colorCntalt = 0;
			rowCntPalalt = 0;
			perRow = 16;
			perRowalt = 0;
			haveAlt = false;
			esize = 2;
			break;

		case NES:
			colorCnt = colorCntalt = 16;
			rowCntPal = rowCntPalalt = 4;
			perRowalt = perRow = 4;
			haveAlt = true;
			esize = 1;
			break;

		case masterSystem:
		case gameGear:
			rowCntPal = 2;
			colorCnt = 32;
			colorCntalt = 0;
			rowCntPalalt = 0;
			perRow = 16;
			perRowalt = 0;
			haveAlt = false;
			esize = gameSystem == gameGear ? 2 : 1;
			break;

		case TMS9918:
			rowCntPal = 1;
			colorCnt = 16;
			colorCntalt = 0;
			rowCntPalalt = 0;
			perRow = 16;
			perRowalt = 0;
			haveAlt = false;
			esize = 0;
			break;

		default:
			showGameSysError(gameSystem)
	}

	if (esize <= 1)
		paletteDataEndian = boost::endian::order::native;
	else {
		switch (gameSystem) {
			case segaGenesis:
				paletteDataEndian = boost::endian::order::big;
				break;

			case gameGear:
				paletteDataEndian = boost::endian::order::little;
				break;

			default:
				paletteDataEndian = boost::endian::order::native;
				show_default_error
		}
	}

	rgbPal = (uint8_t*)realloc(rgbPal, totalMemoryUsage()); // Yes this is correct when rgbPal is NULL realloc will behave the same as malloc.

	if (esize)
		palDat = rgbPal + (totalColors() * 3);
	else
		palDat = nullptr;

	palType = rgbPal + (totalColors() * (3 + esize));

	if (prj->isFixedPalette()) {
		setFixedPalette();
		allColorsCache.clear();
	} else
		allColorsCache = getAllColors();

	if (haveAlt && fixedSpriteRow >= 0)
		throw std::logic_error("haveAlt and fixedSpriteRow cannot both be enabled at the same time.");

	if (rowCntPal <= 1 && fixedSpriteRow >= 0)
		throw std::logic_error("A fixed sprite row may only be enabled when there are multiple rows.");

}
void palette::read(FILE*fp, bool supportsAlt) {
	unsigned readDatCnt;

	if (supportsAlt) {
		readDatCnt = totalColors();
		fread(palDat, readDatCnt, esize, fp);
		fread(palType, readDatCnt, 1, fp);
	} else {
		readDatCnt = colorCnt;
		fread(palDat, colorCnt, esize, fp);
		fread(palType, colorCnt, 1, fp);

		if (haveAlt) {
			if (palDat)
				memset(palDat + (colorCnt * esize), 0, colorCntalt * esize);

			memset(palType + colorCnt, 0, colorCntalt);
		}
	}

	if (esize > 1 && paletteDataEndian != boost::endian::order::native) {
		// Fix it.
		if (esize != 2)
			throw std::runtime_error("esize and endian combination not supported");

		uint16_t*dat = (uint16_t*)palDat;

		for (unsigned i = 0; i < readDatCnt; ++i) {
			uint16_t tmp = *dat;

			if (paletteDataEndian == boost::endian::order::big)
				boost::endian::conditional_reverse_inplace<boost::endian::order::big, boost::endian::order::native>(tmp);
			else if (paletteDataEndian == boost::endian::order::little)
				boost::endian::conditional_reverse_inplace<boost::endian::order::little, boost::endian::order::native>(tmp);

			*dat++ = tmp;
		}
	}
}
void palette::write(FILE*fp) {
	saveBinAsText(palDat, totalColors() * esize, fp, fileType_t::tBinary, nullptr, nullptr, esize * 8, paletteDataEndian);
	fwrite(palType, totalColors(), 1, fp);
}

rgbArray_t palette::valueToRGB(const paletteRawValue_t val) const {
	rgbArray_t rgb;

	switch (prj->gameSystem) {
		case segaGenesis:
		{
			rgb[2] = palTab[((val >> 9) & 7) + palTypeGen]; //Blue
			rgb[1] = palTab[((val >> 5) & 7) + palTypeGen]; //Green
			rgb[0] = palTab[((val >> 1) & 7) + palTypeGen]; //Red
		}
		break;

		case NES:
		{
			uint32_t rgb_out = nesPalToRgb(val);
			rgb[2] = rgb_out & 255; //blue
			rgb[1] = (rgb_out >> 8) & 255; //green
			rgb[0] = (rgb_out >> 16) & 255; //red
		}
		break;

		case masterSystem:
			rgb[0] = palTabMasterSystem[val & 3];
			rgb[1] = palTabMasterSystem[(val >> 2) & 3];
			rgb[2] = palTabMasterSystem[(val >> 4) & 3];
			break;

		case gameGear:
		{
			rgb[0] = palTabGameGear[val & 15];
			rgb[1] = palTabGameGear[(val >> 4) & 15];
			rgb[2] = palTabGameGear[(val >> 8) & 15];
		}
		break;

		default:
			show_default_error
	}

	return rgb;
}

void palette::updateRGBindex(unsigned index) {
	boundsCheckEntry(index);

	uint8_t*rgb = rgbPal + (index * 3);

	paletteRawValue_t val = getEntry(index);
	rgbArray_t rgbTmp = valueToRGB(val);
	memcpy((void*)rgb, (const void*)&rgbTmp[0], 3);
}

void palette::clear(void) {
	memset(rgbPal, 0, totalColors() * (3 + esize));
}

paletteRawValue_t palette::rgbToValue(unsigned r, unsigned g, unsigned b) {
	bool needsInit = true;
	double bestd;
	return searchPermuations(allColorsCache, bestd, r, g, b);
}

void palette::boundsCheckEntry(unsigned ent) const {
	const unsigned maxent = totalColors();

	if (ent >= maxent)
		throw std::out_of_range("ent >= maxent");

	if (prj->isFixedPalette())
		throw std::logic_error("Fixed palettes cannot be modified.");
}

paletteRawValue_t palette::getEntry(const unsigned ent)const {
	switch (esize) {
		case 1:
			return palDat[ent];

		case 2:
		{
			const uint16_t*palPtr = (uint16_t*)palDat;
			return palPtr[ent];
		}
	}
}

void palette::setEntry(const paletteRawValue_t rawVal, const unsigned ent) {
	boundsCheckEntry(ent);

	switch (esize) {
		case 1:
			palDat[ent] = rawVal;
			break;

		case 2:
		{
			uint16_t*palPtr = (uint16_t*)palDat;
			palPtr[ent] = rawVal;
		}
		break;

		default:
			show_default_error
	}

	updateRGBindex(ent);
}

void palette::rgbToEntry(unsigned r, unsigned g, unsigned b, unsigned ent) {
	const paletteRawValue_t rawVal = rgbToValue(r, g, b);
	setEntry(rawVal, ent);
}

uint8_t palette::to_nes_color_rgb(uint8_t red, uint8_t green, uint8_t blue) {
	//this function does not set any values to global palette it is done in other functions
	return nearestColIndex(red, green, blue, nespaltab, 64);
}
uint8_t palette::to_nes_color(unsigned pal_index) {
	//this function does not set any values to global palette it is done in other functions
	pal_index *= 3;
	return to_nes_color_rgb(rgbPal[pal_index], rgbPal[pal_index + 1], rgbPal[pal_index + 2]);
}
uint8_t palette::toNesChan(uint8_t ri, uint8_t gi, uint8_t bi, uint8_t chan) {
	uint32_t rgb_out = toNesRgb(ri, gi, bi);
	uint8_t b = rgb_out & 255;
	uint8_t g = (rgb_out >> 8) & 255;
	uint8_t r = (rgb_out >> 16) & 255;

	switch (chan) {
		case 0:
			return r;
			break;

		case 1:
			return g;
			break;

		case 2:
			return b;
			break;
	}

	return 0;
}
uint16_t palette::to_sega_genesis_colorRGB(uint8_t r, uint8_t g, uint8_t b) {
	r = nearest_color_index(r, 0);
	g = nearest_color_index(g, 0);
	b = nearest_color_index(b, 0);
	//bgr format
	return (r << 1) | (g << 5) | (b << 9);
}
unsigned palette::calMaxPerRow(unsigned row) {
	row *= perRow;
	unsigned max = perRow;

	for (unsigned i = row; i < perRow + row; ++i) {
		if (palType[i] && max) //Locked or reserved colors cannot be changed
			--max;
	}

	return max;
}
void palette::swapEntry(unsigned one, unsigned two) {
	if (unlikely(one == two))
		return;

	paletteRawValue_t tmp = getEntry(two);
	setEntry(two, getEntry(one));
	setEntry(one, tmp);

	uint8_t rgb[3];
	one *= 3;
	two *= 3;
	memcpy(rgb, rgbPal + two, 3);
	memcpy(rgbPal + two, rgbPal + one, 3);
	memcpy(rgbPal + one, rgb, 3);
}
void palette::savePalette(const char*fname, unsigned start, unsigned end, bool skipzero, fileType_t type, int clipboard, const char*label) {
	std::array<uint8_t, 32> bufskip;
	unsigned szskip = 0;

	if (skipzero) {
		for (unsigned i = start; i < end; ++i) {
			if ((i & 3) || (i == 0)) {
				bufskip.at(szskip) = palDat[i];
				++szskip;
			}
		}
	}

	FILE * myfile;

	if (clipboard)
		myfile = nullptr; //When file is null for the function saveBinAsText clipboard will be used
	else
		myfile = fopen(fname, type == fileType_t::tBinary ? "wb" : "w");

	if (likely(myfile || clipboard)) {
		//save the palette
		char comment[512];
		snprintf(comment, sizeof(comment), "Colors %d-%d", start, end - 1);
		int bits = esize * 8;
		start *= esize;
		end *= esize;

		if (skipzero) {
			if (!saveBinAsText(bufskip.data(), szskip, myfile, type, comment, label, bits, paletteDataEndian)) {
				fl_alert("Error: can not save file %s", fname);
				return;
			}
		} else {
			if (!saveBinAsText(palDat + start, end - start, myfile, type, comment, label, bits, paletteDataEndian)) {
				fl_alert("Error: can not save file %s", fname);
				return;
			}
		}

		if (myfile)
			fclose(myfile);
	} else
		alertWrap("Cannot open file %s", fname);
}

void palette::calculateRowStartEnd(unsigned& start, unsigned& end, BgColProcessMode mode) const {
	switch (mode) {
		case BgColProcessMode::ALL:
		case BgColProcessMode::ALL_IGNORE_FIXED:
			start = 0;
			end = totalRows() - 1;
			break;

		case BgColProcessMode::MAIN:
			start = 0;
			end = rowCntPal - 1;
			break;

		case BgColProcessMode::ALT:
			start = rowCntPal;
			end = totalRows() - 1;
			break;

		case BgColProcessMode::ALT_FIRST_ROW:
			start = end = rowCntPal;
			break;

		case BgColProcessMode::FIXED_SPRITE_ROW:
			start = end = fixedSpriteRow;
			break;
	}

	if (mode == BgColProcessMode::ALL_IGNORE_FIXED) {
		while (start == fixedSpriteRow)
			++start;

		while (end == fixedSpriteRow)
			--end;
	}
}

void palette::interpolateBackgroundColors(const palette& other, BgColProcessMode src, BgColProcessMode dst) {
	// Preserve background colors.

	// Cases:
	// Multiple source rows one dst row: Average all the colors.
	// One source row multiple dst rows: Copy the same value for all rows.
	// Multiple sources and destinations: Use linear interpolation to fill the destinations rows.

	unsigned rSrcStart, rSrcEnd;
	other.calculateRowStartEnd(rSrcStart, rSrcEnd, src);

	unsigned rDstStart, rDstEnd;
	calculateRowStartEnd(rDstStart, rDstEnd, dst);

	if (rSrcStart == rSrcEnd) { // One source row
		// Copy the background color to all rows.
		unsigned srcIdx = other.getIndexByRow(rSrcStart, 0) * 3;
		const uint8_t* sourceColor = other.rgbPal + srcIdx;

		for (int r = rDstStart; r <= rDstEnd; ++r) {
			if (dst == BgColProcessMode::ALL_IGNORE_FIXED && r == fixedSpriteRow)
				continue;

			unsigned dstIdx = getIndexByRow(r, 0) * 3;
			uint8_t* dstColor = rgbPal + dstIdx;
			std::memcpy(dstColor, sourceColor, 3);
		}
	} else {
		if (rDstStart == rDstEnd) { // One destination row.
			unsigned ra = 0; // red accumulate.
			unsigned ga = 0;
			unsigned ba = 0;
			unsigned nRows = 0;

			for (int r = rSrcStart; r <= rSrcEnd; ++r) {
				if (src == BgColProcessMode::ALL_IGNORE_FIXED && r == other.fixedSpriteRow)
					continue;

				unsigned bgcolIdx = other.getIndexByRow(r, 0) * 3;
				const uint8_t*oldColor = other.rgbPal + bgcolIdx;
				ra += oldColor[0];
				ga += oldColor[1];
				ba += oldColor[2];
				++nRows;
			}

			ra /= nRows;
			ga /= nRows;
			ba /= nRows;
			unsigned newColIdx = getIndexByRow(rDstStart, 0, false);
			rgbToEntry(ra, ga, ba, newColIdx);
		} else {
			std::vector<rgbArray_t> sourceColors;

			for (int r = rSrcStart; r <= rSrcEnd; ++r) {
				if (src == BgColProcessMode::ALL_IGNORE_FIXED && r == other.fixedSpriteRow)
					continue;

				unsigned bgcolIdx = other.getIndexByRow(r, 0) * 3;
				const uint8_t*oldColor = other.rgbPal + bgcolIdx;
				sourceColors.emplace_back(rgbArray_t {oldColor[0], oldColor[1], oldColor[2] });
			}

			double maxRowOld = double(sourceColors.size() - 1);

			unsigned dstRowCount = rDstEnd - rDstStart + 1;

			if (dst == BgColProcessMode::ALL_IGNORE_FIXED) {
				if (rDstStart >= fixedSpriteRow && rDstStart <= fixedSpriteRow)
					--dstRowCount; // We are not including this row.
			}

			double maxRowCurrent = double(dstRowCount - 1);

			unsigned nRow = 0;

			for (int r = rDstStart; r <= rDstEnd; ++r) {
				if (dst == BgColProcessMode::ALL_IGNORE_FIXED && r == fixedSpriteRow)
					continue;

				double weightAndColIdx = double(nRow) * maxRowOld / maxRowCurrent;
				double colIdxDouble;
				double weight = std::modf(weightAndColIdx, &colIdxDouble); // Percentage of how much the second color should be used.
				unsigned sourceColIdx = unsigned(colIdxDouble);

				unsigned newIndex = getIndexByRow(r, 0);

				if (sourceColIdx < (sourceColors.size() - 1)) { // Verify if there is another color available after sourceColIdx.
					double weightInv = 1.0 - weight;
					rgbArray_t rgb;
					const rgbArray_t& oldColor0 = sourceColors[sourceColIdx];
					const rgbArray_t& oldColor1 = sourceColors[sourceColIdx + 1];

					for (unsigned i = 0; i < 3; ++i) {
						double c0 = double(oldColor0[i]);
						double c1 = double(oldColor1[i]);
						double tmp = (c0 * weightInv) + (c1 * weight);
						tmp = boost::algorithm::clamp(tmp, 0.0, 255.0);
						rgb[i] = uint8_t(tmp);
					}

					rgbToEntry(rgb[0], rgb[1], rgb[2], newIndex);
				} else {
					const rgbArray_t& oldColor = sourceColors[sourceColIdx];
					rgbToEntry(oldColor[0], oldColor[1], oldColor[2], newIndex);
				}

				++nRow;
			}
		}
	}
}

void palette::importBackgroundColors(const palette& other) {
	/* Cases:
	 * other.haveAlt -> !haveAlt: Process totalRows() when fixedSpriteRow < 0 otherwise process only the main palette and don't modify the fixed palette row.
	 * other.haveAlt -> haveAlt: Run this algorithm twice. Process the main palette first then the alternative palette.
	 * !other.haveAlt -> haveAlt: if other.fixedSpriteRow < 0: Run this algorithm twice using the same main palette and put the results in both palettes.
	 * 	if other.fixedSpriteRow >= 0: Build the main palette background colors with all rows except the fixed sprite row then set all alt rows to the background color of the sprite row.
	 * !other.haveAlt -> !haveAlt: Process totalRows() If both palettes contain a forced sprite row and that row is different we will need to specially handle this.
	 */

	// A precondition of this code is that when fixedSpriteRow >= 0 we will have multiple rows. There is no reason to enable fixedSpriteRow when there is only one row in the main palette. Having this constraint prevents possible bugs with this code.

	if (other.haveAlt) {
		if (haveAlt) {
			interpolateBackgroundColors(other, BgColProcessMode::MAIN, BgColProcessMode::MAIN);
			interpolateBackgroundColors(other, BgColProcessMode::ALT, BgColProcessMode::ALT);
		} else if (fixedSpriteRow >= 0) { // !haveAlt
			interpolateBackgroundColors(other, BgColProcessMode::MAIN, BgColProcessMode::MAIN);
			interpolateBackgroundColors(other, BgColProcessMode::ALT, BgColProcessMode::FIXED_SPRITE_ROW);
		} else // No alternative palette and sprites are not forced to use a certain row.
			interpolateBackgroundColors(other, BgColProcessMode::ALL, BgColProcessMode::ALL);
	} else {
		if (haveAlt) {
			interpolateBackgroundColors(other, BgColProcessMode::ALL, BgColProcessMode::MAIN);
			interpolateBackgroundColors(other, other.fixedSpriteRow >= 0 ? BgColProcessMode::FIXED_SPRITE_ROW : BgColProcessMode::ALL, BgColProcessMode::ALT);
		} else {
			if (fixedSpriteRow >= 0 && other.fixedSpriteRow >= 0 && fixedSpriteRow != other.fixedSpriteRow) {
				interpolateBackgroundColors(other, BgColProcessMode::FIXED_SPRITE_ROW, BgColProcessMode::FIXED_SPRITE_ROW);
				interpolateBackgroundColors(other, BgColProcessMode::ALL_IGNORE_FIXED, BgColProcessMode::ALL_IGNORE_FIXED);
			} else
				interpolateBackgroundColors(other, BgColProcessMode::ALL, BgColProcessMode::ALL);
		}
	}
}

void palette::reduceRow(rawValPalMap_t& rowMap, unsigned targetRow, unsigned targetColorCount) {
	while (rowMap.size() > targetColorCount) {
		// Find the two closest colors and eliminate the darker color.
		bool needsInit = true;
		paletteRawValue_t k0, k1;
		double bestd;

		for (auto it0 = rowMap.cbegin(); it0 != rowMap.cend(); ++it0) {
			for (auto it1 = rowMap.cbegin(); it1 != rowMap.cend(); ++it1) {
				if (it0 != it1) { // Don't compare colors against themselves.
					const rgbArray_t& c0 = it0->second;
					const rgbArray_t& c1 = it1->second;
					double d = ciede2000rgb(c0[0], c0[1], c0[2], c1[0], c1[1], c1[2]);

					if (needsInit || d < bestd) {
						bestd = d;
						k0 = it0->first;
						k1 = it1->first;
						needsInit = false;
					}
				}

			}
		}

		{
			double L0, L1, a, b; // a and b will not be used. The reason for these variables is because Rgb2Lab255 sets them.
			const rgbArray_t& c0 = rowMap[k0];
			const rgbArray_t& c1 = rowMap[k1];
			Rgb2Lab255(&L0, &a, &b, c0[0], c0[1], c0[2]);
			Rgb2Lab255(&L1, &a, &b, c1[0], c1[1], c1[2]);

			// Normally the darker color is deleted however if L0 == L1 we will fall back on deleting the second color.
			// If this becomes an issue we can add another metric. For example use the Lch color space and delete the one with the smaller c value.
			if (L0 < L1)
				rowMap.erase(k0);
			else
				rowMap.erase(k1);
		}
	}

	unsigned startIdx = getIndexByRow(targetRow, 1); // One instead of zero to avoid changing the background color.

	for (auto it = rowMap.cbegin(); it != rowMap.cend(); ++it) {
		setEntry(it->first, startIdx);
		++startIdx;
	}

	unsigned lastColorIdx = getIndexByRow(targetRow, getPerRow(isAltRow(targetRow)) - 1);

	for (unsigned i = startIdx; i <= lastColorIdx; ++i) {
		// Pad the rest of the palette with black.
		rgbToEntry(0, 0, 0, i);
	}
}

std::set<paletteRawValue_t> palette::getAllColors() {
	std::set<paletteRawValue_t> permutations;
	// Create the closest unique color possible.
	const unsigned palComponentCount = paletteComponentCount();

	if (palComponentCount != 2 && palComponentCount != 3)
		throw std::logic_error("Permutations are not implemented for this number of permutations."); // TODO: is there a more generic way to do this?

	if (palComponentCount == 2) {
		for (unsigned a = 0; a <= maxValForPaletteComponent(0); ++a) {
			for (unsigned b = 0; b <= maxValForPaletteComponent(1); ++b) {
				paletteRawValue_t tmp = changeValueRaw(a, 0, 0);
				tmp = changeValueRaw(b, 1, tmp);
				permutations.insert(tmp);
			}
		}
	} else {
		for (unsigned a = 0; a <= maxValForPaletteComponent(0); ++a) {
			for (unsigned b = 0; b <= maxValForPaletteComponent(1); ++b) {
				for (unsigned c = 0; c <= maxValForPaletteComponent(2); ++c) {
					paletteRawValue_t tmp = changeValueRaw(a, 0, 0);
					tmp = changeValueRaw(b, 1, tmp);
					tmp = changeValueRaw(c, 2, tmp);
					permutations.insert(tmp);
				}
			}
		}
	}

	return permutations;
}

paletteRawValue_t palette::searchPermuations(const std::set<paletteRawValue_t> & permutations, double& bestd, unsigned r, unsigned g, unsigned b) {
	paletteRawValue_t bestColor;
	bool needsInit = true;

	for (auto itP = permutations.cbegin(); itP != permutations.cend(); ++itP) {
		rgbArray_t testColor = valueToRGB(*itP);
		double d = ciede2000rgb(r, g, b, testColor[0], testColor[1], testColor[2]);

		if (needsInit || d < bestd) {
			bestd = d;
			bestColor = *itP;
			needsInit = false;
		}
	}

	return bestColor;
}

void palette::groupRows(const palette& other, const std::unique_ptr<rawValPalMap_t[]>& colorMap, const std::unique_ptr<std::set<rgbArray_t>[]>& uniqueColors, BgColProcessMode src, BgColProcessMode dst) {
	unsigned rSrcStart, rSrcEnd;
	other.calculateRowStartEnd(rSrcStart, rSrcEnd, src);
	unsigned useableSourceRows = rSrcEnd - rSrcStart + 1;

	if (src == BgColProcessMode::ALL_IGNORE_FIXED && other.fixedSpriteRow >= 0) {
		if (rSrcStart >= other.fixedSpriteRow && rSrcStart <= other.fixedSpriteRow)
			--useableSourceRows;
	}

	unsigned rDstStart, rDstEnd;
	calculateRowStartEnd(rDstStart, rDstEnd, dst);
	unsigned useableDstRows = rDstEnd - rDstStart + 1;

	if (dst == BgColProcessMode::ALL_IGNORE_FIXED && fixedSpriteRow >= 0) {
		if (rDstStart >= fixedSpriteRow && rDstStart <= fixedSpriteRow)
			--useableDstRows;
	}

	unsigned rowsInNewRow;

	if (useableSourceRows > getMaxRows(src == BgColProcessMode::ALT))
		rowsInNewRow = (useableSourceRows + useableDstRows - 1) / useableDstRows; // How many rows from the source will be put in one new row.
	else
		rowsInNewRow = 1;

	for (int r = rSrcStart; r <= rSrcEnd; r += rowsInNewRow) {

		int rDst = r - rSrcStart;
		rDst /= rowsInNewRow;
		rDst += rDstStart;

		if (dst == BgColProcessMode::ALL_IGNORE_FIXED && r == fixedSpriteRow)
			continue;

		for (int rr = 0; rr < rowsInNewRow; ++rr) {
			int rSrc = r + rr;

			if (src == BgColProcessMode::ALL_IGNORE_FIXED && rSrc == other.fixedSpriteRow)
				continue;

			std::set<rgbArray_t> uniqueColorsNew;

			for (unsigned col = 1; col < other.getPerRow(isAltRow(rSrc)); ++col) { // Start with one instead of zero to skip the background color.
				unsigned oldColIdx = other.getIndexByRow(rSrc, col) * 3;
				const uint8_t* oldColor = other.rgbPal + oldColIdx;
				rgbArray_t oldC {oldColor[0], oldColor[1], oldColor[2] };

				if (uniqueColors[rDst].count(oldC) < 1) {
					uniqueColorsNew.emplace(oldC);
					uniqueColors[rDst].emplace(oldC);
				}
			}

			for (auto it = uniqueColorsNew.cbegin(); it != uniqueColorsNew.cend(); ++it) {
				const rgbArray_t& color = *it;
				paletteRawValue_t closestColor = rgbToValue(color[0], color[1], color[2]);

				if (colorMap[rDst].count(closestColor) > 0) {

					auto permutations = getAllColors();

					// Remove colors that we already have.
					permutations.erase(closestColor);

					for (auto itCM = colorMap[rDst].cbegin(); itCM != colorMap[rDst].cend(); ++itCM)
						permutations.erase(itCM->first);

					rgbArray_t closestColorRGB = valueToRGB(closestColor);

					double bestd;
					auto bestColor = searchPermuations(permutations, bestd, closestColorRGB[0], closestColorRGB[1], closestColorRGB[2]);

					if (bestd < 20.0)
						colorMap[rDst][bestColor] = valueToRGB(bestColor);

					// Now find the second closest color that does not already exist in
				} else
					colorMap[rDst][closestColor] = valueToRGB(closestColor);
			}

		}
	}
}

void palette::sortAndReduceColors(const palette& other) {
	// Colors must be eliminated.
	// In general if we must remove colors from a row we will prefer eliminating darker colors because dithering handles missing dark colors very well.
	// other.haveAlt -> !haveAlt: Include the alternative colors in the main palette. If the system uses a fixed row for the sprite palette ensure that these alternative colors go in there.
	// other.haveAlt -> haveAlt: Process both palettes separately.
	// !other.haveAlt -> !haveAlt: No special handling needed.
	// !other.haveAlt -> haveAlt: If fixed row use that for the alternative palette otherwise use other.rgbPal for both palettes.

	/* Put the old colors into groups.
	 * We are allowed to merge existing rows however existing rows will not be split up. */
	std::unique_ptr<rawValPalMap_t[]> colorMap(new rawValPalMap_t[totalRows()]);
	std::unique_ptr<std::set<rgbArray_t>[]> uniqueColors(new std::set<rgbArray_t>[totalRows()]);
	// Populate the set. This provides an easy way to eliminate duplicate colors.

	// First handle fixed row sprite palettes.
	bool fixedRowSet = false;

	if (fixedSpriteRow >= 0) {

		if (other.fixedSpriteRow >= 0) {
			// Copy the fixed rows.
			fixedRowSet = true;
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::FIXED_SPRITE_ROW, BgColProcessMode::FIXED_SPRITE_ROW);
		} else if (other.haveAlt) {
			// Copy all alternative palette colors into the one row.
			fixedRowSet = true;
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::ALT, BgColProcessMode::FIXED_SPRITE_ROW);
		} // Otherwise no special actions are taken for the fixed sprite palette.
	}

	if (haveAlt) {
		if (other.fixedSpriteRow >= 0) {
			// Copy all colors from the fixed sprite row into the first row of the alternative palette.
			// When haveAlt is enabled we are guaranteed to have at-least one alternative palette row.
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::FIXED_SPRITE_ROW, BgColProcessMode::ALT_FIRST_ROW);

			groupRows(other, colorMap, uniqueColors, BgColProcessMode::ALL_IGNORE_FIXED, BgColProcessMode::MAIN);
		} else if (other.haveAlt) {
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::MAIN, BgColProcessMode::MAIN);
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::ALT, BgColProcessMode::ALT);
		} else { // Put the old main palette in the alternative palette group. We can't copy the results from the main palette because perRow may not equal perRowalt.
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::MAIN, BgColProcessMode::MAIN);
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::MAIN, BgColProcessMode::ALT);
		}
	} else {
		// First do the main palette.
		groupRows(other, colorMap, uniqueColors, BgColProcessMode::MAIN, fixedRowSet ? BgColProcessMode::ALL_IGNORE_FIXED : BgColProcessMode::MAIN);

		if (other.haveAlt) // Put the old alternative palette in the same group as the main palette.
			groupRows(other, colorMap, uniqueColors, BgColProcessMode::ALT, BgColProcessMode::ALL_IGNORE_FIXED);
	}

	for (unsigned i = 0; i < totalRows(); ++i)
		reduceRow(colorMap[i], i, getPerRow(i >= rowCntPal) - 1); // - 1 because the background color is not included.
}

void palette::import(const palette& other) {
	if (prj->isFixedPalette())
		setFixedPalette();
	else {
		importBackgroundColors(other);
		sortAndReduceColors(other);
	}

	if (prj->tms && prj->containsData(pjHaveMap) && rowCntPal > other.rowCntPal)
		prj->tms->fixPaletteRows(rowCntPal, other.rowCntPal);
}

void palette::setFixedPalette() {
	switch (prj->gameSystem) {
		case TMS9918:
			memcpy(rgbPal, TMS9918Palette, sizeof(TMS9918Palette));
			break;

		default:
			show_default_error
	}
}
unsigned palette::getIndexByRow(unsigned row, unsigned offset, bool isAlt) const {
	if (isAlt) {
		if (haveAlt) {
			if (row >= rowCntPalalt)
				throw std::out_of_range("row >= rowCntPalalt");

			if (offset >= perRowalt)
				throw std::out_of_range("offset >= perRowalt");
			else
				return colorCnt + (row * perRowalt) + offset;
		} else
			throw std::out_of_range("Row out of range.");
	} else {
		if (row >= rowCntPal)
			throw std::out_of_range("row >= rowCntPal");

		if (offset >= perRow)
			throw std::out_of_range("offset >= perRow");
		else
			return (row * perRow) + offset;
	}

	return 0;
}

unsigned palette::getIndexByRow(unsigned row, unsigned offset) const {
	bool isAlt = haveAlt && isAltRow(row);

	if (isAlt)
		row -= rowCntPal;

	return getIndexByRow(row, offset, isAlt);
}

unsigned palette::paletteComponentCount() const {
	switch (prj->gameSystem) { // ensure paletteComponentIndex is in an allowed range.
		case segaGenesis:
		case masterSystem:
		case gameGear:
			return 3;

		case NES:
			return 2;

		default:
			show_default_error
	}

	return 0;
}

void palette::checkPaletteComponentIndex(unsigned palComponentIdx) const {
	unsigned entryIndexThreshold = paletteComponentCount();

	if (palComponentIdx >= entryIndexThreshold)
		throw std::out_of_range("entryIndex >= entryIndexThreshold");
}

unsigned palette::maxValForPaletteComponent(unsigned paletteComponentIndex) const {

	switch (prj->gameSystem) {
		case segaGenesis:
			return 7;

		case NES:
			if (paletteComponentIndex == 0)
				return 15;
			else
				return 3;

		case masterSystem:
			return 3;

		case gameGear:
			return 15;

		default:
			show_default_error
	}

	return 0;
}

paletteRawValue_t palette::changeValueRaw(unsigned value, unsigned paletteComponentIndex, paletteRawValue_t rawVal) const {

	checkPaletteComponentIndex(paletteComponentIndex);

	unsigned maxVal = maxValForPaletteComponent(paletteComponentIndex);

	if (value > maxVal)
		throw std::logic_error("value > maxVal");

	switch (prj->gameSystem) {
		case segaGenesis:
		{
			unsigned shift = paletteComponentIndex * 4 + 1;
			rawVal &= ~(7 << shift);
			rawVal |= (value & 7) << shift;
		}
		break;

		case masterSystem:
		{
			unsigned shift = paletteComponentIndex * 2;
			rawVal &= ~(3 << shift);
			rawVal |= (value & 3) << shift;
		}
		break;

		case gameGear:
		{
			unsigned shift = paletteComponentIndex * 4;
			rawVal &= ~(15 << shift);
			rawVal |= (value & 15) << shift;
		}
		break;

		case NES:
			switch (paletteComponentIndex) {
				/*
				76543210
				||||||||
				||||++++- Hue (phase)
				||++----- Value (voltage)
				++------- Unimplemented, reads back as 0
				*/
				case 0://Hue
					//first read out value
					rawVal &= 3 << 4;
					rawVal |= value;
					break;

				case 1://Value
					rawVal &= 15;
					rawVal |= value << 4;
					break;
			}

			break;

		default:
			show_default_error
	}

	return rawVal;
}

void palette::changeIndexRaw(unsigned value, unsigned paletteComponentIndex, unsigned index) {
	paletteRawValue_t val = changeValueRaw(value, paletteComponentIndex, getEntry(index));
	setEntry(val, index);
	updateRGBindex(index);
}

void palette::loadFromFile(const char * fname, fileType_t forceType, unsigned offset, CompressionType compression) {
	size_t palSize = totalColors();
	palSize -= offset;
	palSize *= esize;
	offset *= esize;
	filereader f = filereader(paletteDataEndian, esize, "Load palette", false, 16, true, fname, forceType, compression);

	if (f.amt < 1)
		return;

	unsigned i = f.selDat();
	memcpy(palDat + offset, f.dat[i], std::min(f.lens[i], palSize));

	//now convert each value to RGB.
	paletteToRgb();

	if (window)
		window->redraw();
}

void palette::updateEmphasis(void) {
	/*76543210
	  ||||||||
	  ||||++++- Hue (phase)
	  ||++----- Value (voltage)
	  ++------- Unimplemented, reads back as 0*/
	unsigned emps = (prj->subSystem >> NESempShift)&NESempMask;
	unsigned empsSprite = (prj->subSystem >> NESempShiftAlt)&NESempMask;
	emps <<= 6;
	empsSprite <<= 6;
	uint32_t rgb_out;
	updateNesTab(emps, false);
	updateNesTab(empsSprite, true);

	for (unsigned c = 0; c < 48; c += 3) {
		rgb_out = nesPalToRgb(palDat[c / 3] | emps);
		rgbPal[c] = (rgb_out >> 16) & 255; //red
		rgbPal[c + 1] = (rgb_out >> 8) & 255; //green
		rgbPal[c + 2] = rgb_out & 255; //blue
	}

	for (unsigned c = 48; c < 96; c += 3) {
		rgb_out = nesPalToRgb(palDat[c / 3] | empsSprite);
		rgbPal[c] = (rgb_out >> 16) & 255; //red
		rgbPal[c + 1] = (rgb_out >> 8) & 255; //green
		rgbPal[c + 2] = rgb_out & 255; //blue
	}
}

int palette::setPaletteFromRGB(const uint8_t* colors, const unsigned nColors, const unsigned minEntry, const unsigned maxEntry) {
	std::set<uint32_t> uniqueColors;
	std::set<paletteRawValue_t> convertedColors;

	for (unsigned i = 0; i < nColors; ++i) {
		uint32_t c = colors[0] | (colors[1] << 8) | (colors[2] << 16);
		uniqueColors.emplace(c);
		colors += 3;
	}

	for (auto it = uniqueColors.cbegin(); it != uniqueColors.cend(); ++it) {
		const uint32_t c = *it;
		convertedColors.emplace(rgbToValue(c & 255, (c >> 8) & 255, c >> 16));
	}

	unsigned maxAllowedColors = maxEntry - minEntry + 1;

	// Find and remove locked colors from the list of converted colors.

	for (unsigned en = minEntry; en <= maxEntry; ++en) {
		if (palType[en] == 1) // Is this a locked color?
			convertedColors.erase(getEntry(en)); // Erase the key if it exists. Otherwise nothing happens.
	}

	if (convertedColors.size() > maxAllowedColors)
		return -1; // Too many colors.

	unsigned e = minEntry;

	// First do a dry run to make sure that it is possible to fill the palette without exceeding maxEntry.
	for (auto it = convertedColors.cbegin(); it != convertedColors.cend(); ++e) {
		if (e > maxEntry)
			return -1; // Could not set all colors.

		if (palType[e])
			continue; // Try again with the next entry.

		++it;
	}


	e = minEntry;

	// Now set the palette.
	for (auto it = convertedColors.cbegin(); it != convertedColors.cend(); ++e) {
		if (palType[e])
			continue; // Try again with the next entry.

		setEntry(*it, e);
		++it;
	}

	return convertedColors.size();
}
