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
#pragma once
#include <array>
#include <boost/endian/conversion.hpp>
#include <cstdio>
#include <map>
#include <memory>
#include <set>
#include "project.h"
#include "filemisc.h"
#include "compressionWrapper.h"
typedef std::array<uint8_t, 3> rgbArray_t;
typedef uint16_t paletteRawValue_t;
typedef std::map<paletteRawValue_t, rgbArray_t> rawValPalMap_t;
enum class BgColProcessMode {ALL, MAIN, ALT, ALT_FIRST_ROW, FIXED_SPRITE_ROW, ALL_IGNORE_FIXED};
class palette {
	void reduceRow(rawValPalMap_t& rowMap, unsigned targetRow, unsigned targetColorCount);
	void calculateRowStartEnd(unsigned& start, unsigned& end, BgColProcessMode mode) const;
	void interpolateBackgroundColors(const palette& other, BgColProcessMode src, BgColProcessMode dst);
	void importBackgroundColors(const palette& other);
	void groupRows(const palette& other, const std::unique_ptr<rawValPalMap_t[]>& colorMap, const std::unique_ptr<std::set<rgbArray_t>[]>& uniqueColors, BgColProcessMode src, BgColProcessMode dst);
	void sortAndReduceColors(const palette& other); // This is used for import.
	void setFixedPalette();
	void boundsCheckEntry(unsigned ent) const;
	void checkPaletteComponentIndex(unsigned paletteComponentIndex) const;
	unsigned maxValForPaletteComponent(unsigned paletteComponentIndex) const;
	unsigned paletteComponentCount() const;
	paletteRawValue_t changeValueRaw(unsigned value, unsigned paletteComponentIndex, paletteRawValue_t rawVal) const;
	std::set<paletteRawValue_t> getAllColors();
	std::set<paletteRawValue_t> allColorsCache;
	paletteRawValue_t searchPermuations(const std::set<paletteRawValue_t> & permutations, double& bestd, unsigned r, unsigned g, unsigned b);
public:
	Project*prj;
	bool rgbPalIsManagedByClass = true;
	uint8_t*rgbPal;
	uint8_t*palDat;
	uint8_t*palType;/*!<Sets 3 different types for each palette entry free, locked and reserved*/
	unsigned colorCnt;//Total entries in palette for project
	unsigned colorCntalt;//Alternative palette color count
	unsigned rowCntPal;//Number of palette rows
	unsigned rowCntPalalt;
	unsigned perRow;//Colors per each palette row
	unsigned perRowalt;
	unsigned esize;//Bytes per palette entry
	boost::endian::order paletteDataEndian;
	int fixedSpriteRow; // fixedSpriteRow and haveAlt cannot be enabled at the same time.
	bool haveAlt;//Does the current game system use an alternative sprite palette?
	palette(Project*prj);
	palette(uint8_t*rgbData, unsigned mainColorCount, unsigned spritePaletteCount, unsigned mainPerRowCount, unsigned spritePalettePerRowCount, int fSpriteRow);
	~palette(void);
	palette(const palette&other, Project*prj);
	void setVars(enum gameSystemEnum gameSystem);
	void read(FILE*fp, bool supportsAlt);
	void write(FILE*fp);
	void updateRGBindex(unsigned index);
	void clear(void);
	paletteRawValue_t rgbToValue(unsigned r, unsigned g, unsigned b);
	rgbArray_t valueToRGB(const paletteRawValue_t val) const;
	paletteRawValue_t getEntry(const unsigned ent)const;
	bool isRawValueValid(paletteRawValue_t val)const;
	void setEntry(const paletteRawValue_t rawVal, const unsigned ent);
	void rgbToEntry(unsigned r, unsigned g, unsigned b, unsigned ent);
	uint8_t to_nes_color_rgb(uint8_t red, uint8_t green, uint8_t blue);
	uint8_t to_nes_color(unsigned pal_index);
	uint8_t toNesChan(uint8_t ri, uint8_t gi, uint8_t bi, uint8_t chan);
	uint16_t to_sega_genesis_colorRGB(uint8_t r, uint8_t g, uint8_t b);
	unsigned calMaxPerRow(unsigned row);
	void swapEntry(unsigned one, unsigned two);
	void savePalette(const char*fname, unsigned start, unsigned end, bool skipzero, fileType_t type, int clipboard, const char*label = "palDat");
	void import(const palette& other);
	unsigned getIndexByRow(unsigned row, unsigned offset, bool isAlt) const;
	unsigned getIndexByRow(unsigned row, unsigned offset) const;
	void changeIndexRaw(unsigned value, unsigned entryIndex, unsigned index);
	void loadFromFile(const char * fname, fileType_t forceType, unsigned offset, CompressionType compression); // use fileType_t::tCancel to prompt for the filetype.
	void updateEmphasis(void);
	int setPaletteFromRGB(const uint8_t* colors, const unsigned nColors, const unsigned minEntry, const unsigned maxEntry);
	rgbArray_t rgbToNearestSystemColor(rgbArray_t rgbIn);
	unsigned nearestColIndex(int red, int green, int blue, const uint8_t*pal, unsigned amt, bool checkType = false, unsigned off = 0);
	bool isAltRow(unsigned row) const {
		return row >= rowCntPal;
	}
	unsigned getMaxRows(bool alt) const {
		return (alt && haveAlt) ? rowCntPalalt : rowCntPal;
	}
	unsigned getMaxCols(bool alt) const {
		return (alt && haveAlt) ? colorCntalt : colorCnt;
	}
	unsigned getPerRow(bool alt) const {
		return (alt && haveAlt) ? perRowalt : perRow;
	}
	unsigned totalColors() const {
		return colorCnt + colorCntalt;
	}
	unsigned totalMemoryUsage() const {
		return totalColors() * (4 + esize);
	}
	unsigned totalRows() const {
		return rowCntPal + rowCntPalalt;
	}
	void paletteToRgb(void) {
		for (unsigned i = 0; i < totalColors(); ++i)
			updateRGBindex(i);
	}
};
extern const uint8_t TMS9918Palette[];
