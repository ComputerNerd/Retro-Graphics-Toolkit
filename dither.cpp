#include <algorithm>
#include <cmath>
#include <ctime>
#include <map>
#include <utility>

#include "dither.h"
#include "color_convert.h"
#include "nearestColor.h"
#include "gui.h"
#include "palette.h"
#include "errorMsg.h"
#define NONE 0
#define UP 1
#define LEFT 2
#define DOWN 3
#define RIGHT 4
static unsigned useHiL;//no use for these variables outside of this file
static unsigned useMode;
static unsigned rgbPixelsize;
static bool USEofColGlob;
static bool isSpriteG;
static bool shouldForceRow;
static unsigned selectedForceRow;
static uint8_t*img_ptr_dither;
static bool isChunkD_G;
static uint32_t idChunk_G;
static uint8_t nearest_color_chanColSpace(uint8_t val, uint8_t chan) {
	switch (useMode) {
		case segaGenesis:
			return palTab[nearest_color_index(val)];
			break;

		case NES:
		{	img_ptr_dither -= chan;
			uint8_t returnme = currentProject->pal->toNesChan(*img_ptr_dither, img_ptr_dither[1], img_ptr_dither[2], chan);
			img_ptr_dither += chan;
			return returnme;
		}
		break;

		case masterSystem:
			return palTabMasterSystem[nearestOneChannel(val, palTabMasterSystem, 4)];
			break;

		case gameGear:
			return palTabGameGear[nearestOneChannel(val, palTabGameGear, 16)];
			break;

		case TMS9918:
			//return nearestColIndex(r_old,g_old,b_old,currentProject->pal->rgbPal,currentProject->pal->colorCnt);
			return 0;//TODO
			break;

		case 255://alpha
			return (val & 128) ? 255 : 0;
			break;

		default:
			show_default_error
			return 0;
	}
}
static uint8_t nearest_color_chan(int_fast32_t val, uint8_t chan, uint8_t row) {
	//returns closest value
	//palette_multiplier
	int_fast32_t bestIndex = 0;
	int_fast32_t minDistanceSquared = 255 * 255 + 1;
	unsigned max_rgb = (currentProject->pal->haveAlt ? currentProject->pal->perRowalt : currentProject->pal->perRow) * 3;

	if (useMode == 255)
		return (val & 128) ? 255 : 0;

	if (currentProject->pal->haveAlt && isSpriteG)
		row += currentProject->pal->colorCnt * 3;

	row *= currentProject->pal->perRow * 3;

	max_rgb += row;

	for (unsigned i = row; i < max_rgb; i += 3) {
		int_fast32_t Rdiff = val - (int_fast32_t)currentProject->pal->rgbPal[i + chan];
		int_fast32_t distanceSquared = Rdiff * Rdiff;

		if (distanceSquared < minDistanceSquared) {
			minDistanceSquared = distanceSquared;
			bestIndex = i;
		}
	}

	return bestIndex + chan;
}
/* variables needed for the Riemersma
 * dither algorithm */
static int32_t cur_x = 0, cur_y = 0;
static int32_t img_width = 0, img_height = 0;

static unsigned rgb_select = 0;

#define SIZE 16	/* queue size: number of
		 * pixels remembered */
#define MAX  16	/* relative weight of
		 * youngest pixel in the
		 * queue, versus the oldest
		 * pixel */

static int32_t weights[SIZE];	/* weights for
				 * the errors
				 * of recent
				 * pixels */

static void init_weights(int32_t a[], int32_t size, int32_t max) {
	double m = exp(log(max) / (size - 1));
	double v;
	int_fast32_t i;

	for (i = 0, v = 1.0; i < size; ++i) {
		a[i] = (int)(v + 0.5);	/* store rounded
					 * value */
		v *= m;			/* next value */
	}
}

static void dither_pixel(uint8_t *pixel) {
	static int32_t error[SIZE]; /* queue with error
				     * values of recent
				     * pixels */
	int32_t i, pvalue, err;

	for (i = 0, err = 0L; i < SIZE; ++i)
		err += error[i] * weights[i];

	if (*pixel + err / MAX > 255)
		pvalue = 255;
	else if (*pixel + err / MAX < 0)
		pvalue = 0;
	else
		pvalue = *pixel + err / MAX;

	if ((currentProject->gameSystem == segaGenesis) && (useHiL == 9)) {
		unsigned tempSet;

		if (isChunkD_G)
			tempSet = (currentProject->Chunk->getPrio_t(idChunk_G, cur_x / 8, cur_y / 8) ^ 1) * 8;
		else
			tempSet = (currentProject->tms->maps[currentProject->curPlane].get_prio(cur_x / 8, cur_y / 8) ^ 1) * 8;

		set_palette_type_force(tempSet);
	}

	if (USEofColGlob)
		pvalue = nearest_color_chanColSpace(pvalue, rgb_select);
	else {
		if (shouldForceRow)
			pvalue = nearest_color_chan(pvalue, rgb_select, selectedForceRow);
		else {
			if (isChunkD_G)
				pvalue = nearest_color_chan(pvalue, rgb_select, currentProject->Chunk->getTileRow_t(idChunk_G, cur_x / 8, cur_y / 8));
			else
				pvalue = nearest_color_chan(pvalue, rgb_select, currentProject->tms->maps[currentProject->curPlane].getPalRow(cur_x / 8, cur_y / 8));
		}


		if (useMode != 255)
			pvalue = currentProject->pal->rgbPal[pvalue];
	}

	// shift queue
	memmove(error, error + 1, (SIZE - 1)*sizeof error[0]);
	error[SIZE - 1] = *pixel - pvalue;
	*pixel = (uint8_t)pvalue;
}
static void move(int32_t direction) {
	/* dither the current pixel */
	if (cur_x >= 0 && cur_x < img_width && cur_y >= 0 && cur_y < img_height)
		dither_pixel(img_ptr_dither);

	/* move to the next pixel */
	switch (direction) {
		case LEFT:
			cur_x--;
			img_ptr_dither -= rgbPixelsize;
			break;

		case RIGHT:
			cur_x++;
			img_ptr_dither += rgbPixelsize;
			break;

		case UP:
			cur_y--;
			img_ptr_dither -= img_width * rgbPixelsize;
			break;

		case DOWN:
			cur_y++;
			img_ptr_dither += img_width * rgbPixelsize;
			break;
	} /* switch */
}
static void hilbert_level(int32_t level, int32_t direction) {
	if (level == 1) {
		switch (direction) {
			case LEFT:
				move(RIGHT);
				move(DOWN);
				move(LEFT);
				break;

			case RIGHT:
				move(LEFT);
				move(UP);
				move(RIGHT);
				break;

			case UP:
				move(DOWN);
				move(RIGHT);
				move(UP);
				break;

			case DOWN:
				move(UP);
				move(LEFT);
				move(DOWN);
				break;
		} /* switch */
	} else {
		switch (direction) {
			case LEFT:
				hilbert_level(level - 1, UP);
				move(RIGHT);
				hilbert_level(level - 1, LEFT);
				move(DOWN);
				hilbert_level(level - 1, LEFT);
				move(LEFT);
				hilbert_level(level - 1, DOWN);
				break;

			case RIGHT:
				hilbert_level(level - 1, DOWN);
				move(LEFT);
				hilbert_level(level - 1, RIGHT);
				move(UP);
				hilbert_level(level - 1, RIGHT);
				move(RIGHT);
				hilbert_level(level - 1, UP);
				break;

			case UP:
				hilbert_level(level - 1, LEFT);
				move(DOWN);
				hilbert_level(level - 1, UP);
				move(RIGHT);
				hilbert_level(level - 1, UP);
				move(UP);
				hilbert_level(level - 1, RIGHT);
				break;

			case DOWN:
				hilbert_level(level - 1, RIGHT);
				move(UP);
				hilbert_level(level - 1, DOWN);
				move(LEFT);
				hilbert_level(level - 1, DOWN);
				move(DOWN);
				hilbert_level(level - 1, LEFT);
				break;
		} /* switch */
	} /* if */
}
static inline int32_t log2int(int32_t value) {
	int32_t result = 0;

	while (value > 1) {
		value >>= 1;
		result++;
	}

	return result;
}
static void Riemersma(uint8_t *image, int32_t width, int32_t height, unsigned rgb_sel) {
	int32_t level, size;
	rgb_select = rgb_sel;
	/* determine the required order of the
	 * Hilbert curve */
	size = std::max(width, height);
	level = log2int(size);

	if ((1L << level) < size)
		level++;

	init_weights(weights, SIZE, MAX);
	img_ptr_dither = image;
	img_ptr_dither += rgb_sel;
	img_width = width;
	img_height = height;
	cur_x = 0;
	cur_y = 0;

	if (level > 0)
		hilbert_level(level, UP);

	move(NONE);
}

//#define COMPARE_RGB 1

/* 8x8 threshold map */
static const uint8_t mapY3[8 * 8] = {
	0, 48, 12, 60, 3, 51, 15, 63,
	32, 16, 44, 28, 35, 19, 47, 31,
	8, 56, 4, 52, 11, 59, 7, 55,
	40, 24, 36, 20, 43, 27, 39, 23,
	2, 50, 14, 62, 1, 49, 13, 61,
	34, 18, 46, 30, 33, 17, 45, 29,
	10, 58, 6, 54, 9, 57, 5, 53,
	42, 26, 38, 22, 41, 25, 37, 21
};

#define GammaAmt 2.2f // Gamma correction we use.

static float GammaCorrect(float v)   {
	return powf(v, GammaAmt);
}
static float GammaUncorrect(float v) {
	return powf(v, 1.0f / GammaAmt);
}

/* CIE C illuminate */
static const double illum[3 * 3] = {0.488718, 0.176204, 0.000000,
                                    0.310680, 0.812985, 0.0102048,
                                    0.200602, 0.0108109, 0.989795
                                   };
struct LabItem // CIE L*a*b* color value with C and h added.
{
	double L, a, b, C, h;

	LabItem() { }
	LabItem(double R, double G, double B) {
		Set(R, G, B);
	}
	void Set(double R, double G, double B) {
		const double* const i = illum;
		double X = i[0] * R + i[3] * G + i[6] * B, x = X / (i[0] + i[1] + i[2]);
		double Y = i[1] * R + i[4] * G + i[7] * B, y = Y / (i[3] + i[4] + i[5]);
		double Z = i[2] * R + i[5] * G + i[8] * B, z = Z / (i[6] + i[7] + i[8]);
		const double threshold1 = (6 * 6 * 6.0) / (29 * 29 * 29.0);
		const double threshold2 = (29 * 29.0) / (6 * 6 * 3.0);
		double x1 = (x > threshold1) ? pow(x, 1.0 / 3.0) : (threshold2 * x) + (4 / 29.0);
		double y1 = (y > threshold1) ? pow(y, 1.0 / 3.0) : (threshold2 * y) + (4 / 29.0);
		double z1 = (z > threshold1) ? pow(z, 1.0 / 3.0) : (threshold2 * z) + (4 / 29.0);
		L = (29 * 4) * y1 - (4 * 4);
		a = (500 * (x1 - y1) );
		b = (200 * (y1 - z1) );
		C = sqrt(a * a + b + b);
		h = atan2(b, a);
	}
	LabItem(unsigned rgb) {
		Set(rgb);
	}
	void Set(unsigned rgb) {
		Set( (rgb >> 16) / 255.0, ((rgb >> 8) & 0xFF) / 255.0, (rgb & 0xFF) / 255.0 );
	}
};

/* From the paper "The CIEDE2000 Color-Difference Formula: Implementation Notes, */
/* Supplementary Test Data, and Mathematical Observations", by */
/* Gaurav Sharma, Wencheng Wu and Edul N. Dalal, */
/* Color Res. Appl., vol. 30, no. 1, pp. 21-30, Feb. 2005. */
/* Return the CIEDE2000 Delta E color difference measure squared, for two Lab values */
#ifndef COMPARE_RGB
static double ColorCompare(const LabItem& lab1, const LabItem& lab2) {
	//return ciede2000(lab1.L,lab1.a,lab1.b,lab2.L,lab2.a,lab2.b,1.0,1.0,1.0);
#define RAD2DEG(xx) (180.0/M_PI * (xx))
#define DEG2RAD(xx) (M_PI/180.0 * (xx))
	/* Compute Chrominance and Hue angles */
	double C1, C2, h1, h2;
	{
		double Cab = 0.5 * (lab1.C + lab2.C);
		double Cab7 = pow(Cab, 7.0);
		double G = 0.5 * (1.0 - sqrt(Cab7 / (Cab7 + 6103515625.0)));
		double a1 = (1.0 + G) * lab1.a;
		double a2 = (1.0 + G) * lab2.a;
		C1 = sqrt(a1 * a1 + lab1.b * lab1.b);
		C2 = sqrt(a2 * a2 + lab2.b * lab2.b);

		if (C1 < 1e-9)
			h1 = 0.0;
		else {
			h1 = RAD2DEG(atan2(lab1.b, a1));

			if (h1 < 0.0)
				h1 += 360.0;
		}

		if (C2 < 1e-9)
			h2 = 0.0;
		else {
			h2 = RAD2DEG(atan2(lab2.b, a2));

			if (h2 < 0.0)
				h2 += 360.0;
		}
	}

	// Compute delta L, C and H
	double dL = lab2.L - lab1.L, dC = C2 - C1, dH;
	{
		double dh;

		if (C1 < 1e-9 || C2 < 1e-9)
			dh = 0.0;

		else {
			dh = h2 - h1;

			if (dh > 180.0)  dh -= 360.0;
			else if (dh < -180.0) dh += 360.0;
		}

		dH = 2.0 * sqrt(C1 * C2) * sin(DEG2RAD(0.5 * dh));
	}

	double h;
	double L = 0.5 * (lab1.L  + lab2.L);
	double C = 0.5 * (C1 + C2);

	if (C1 < 1e-9 || C2 < 1e-9)
		h = h1 + h2;

	else {
		h = h1 + h2;

		if (fabs(h1 - h2) > 180.0) {
			if (h < 360.0)  h += 360.0;
			else if (h >= 360.0) h -= 360.0;
		}

		h *= 0.5;
	}

	double T = 1.0
	           - 0.17 * cos(DEG2RAD(h - 30.0))
	           + 0.24 * cos(DEG2RAD(2.0 * h))
	           + 0.32 * cos(DEG2RAD(3.0 * h + 6.0))
	           - 0.2 * cos(DEG2RAD(4.0 * h - 63.0));
	double hh = (h - 275.0) / 25.0;
	double ddeg = 30.0 * exp(-hh * hh);
	double C7 = pow(C, 7.0);
	double RC = 2.0 * sqrt(C7 / (C7 + 6103515625.0));
	double L50sq = (L - 50.0) * (L - 50.0);
	double SL = 1.0 + (0.015 * L50sq) / sqrt(20.0 + L50sq);
	double SC = 1.0 + 0.045 * C;
	double SH = 1.0 + 0.015 * C * T;
	double RT = -sin(DEG2RAD(2 * ddeg)) * RC;
	double dLsq = dL / SL, dCsq = dC / SC, dHsq = dH / SH;
	return dLsq * dLsq + dCsq * dCsq + dHsq * dHsq + RT * dCsq * dHsq;
#undef RAD2DEG
#undef DEG2RAD
}
#endif
static inline double ColorCompare(double r1, double g1, double b1, double r2, double g2, double b2) {
	LabItem c1(r1, g1, b1);
	LabItem c2(r2, g2, b2);
	return ColorCompare(c1, c2);
}

/* Palette */
#define maxpalettesize 512
static unsigned palettesize = 16;

/* Luminance for each palette entry, to be initialized as soon as the program begins */
static unsigned luma[maxpalettesize];
//static LabItem  meta[maxpalettesize];
static float   pal_g[maxpalettesize][3]; // Gamma-corrected palette entry
static unsigned offsetGloablY3 = 0;
static inline bool PaletteCompareLuma(unsigned index1, unsigned index2) {
	return luma[index1 + offsetGloablY3] < luma[index2 + offsetGloablY3];
}

typedef std::vector<unsigned> MixingPlan;



/* 8x8 threshold map */
#define d(x) (float)x/64.0f
static const float mapY1[8 * 8] = {
	d( 0), d(48), d(12), d(60), d( 3), d(51), d(15), d(63),
	d(32), d(16), d(44), d(28), d(35), d(19), d(47), d(31),
	d( 8), d(56), d( 4), d(52), d(11), d(59), d( 7), d(55),
	d(40), d(24), d(36), d(20), d(43), d(27), d(39), d(23),
	d( 2), d(50), d(14), d(62), d( 1), d(49), d(13), d(61),
	d(34), d(18), d(46), d(30), d(33), d(17), d(45), d(29),
	d(10), d(58), d( 6), d(54), d( 9), d(57), d( 5), d(53),
	d(42), d(26), d(38), d(22), d(41), d(25), d(37), d(21)
};
#undef d



// Compare the difference of two RGB values
static inline float EvaluateMixingError(int r, int g, int b,
                                        int r0, int g0, int b0,
                                        int r1, int g1, int b1,
                                        int r2, int g2, int b2,
                                        float ratio) {
	return ColorCompare(r, g, b, r0, g0, b0)
	       + (ColorCompare(r1, g1, b1, r2, g2, b2) * 0.1f * (fabsf(ratio - 0.5f) + 0.5f));
}

struct MixingPlanY1 {
	unsigned colors[2];
	float ratio; /* 0 = always index1, 1 = always index2, 0.5 = 50% of both */
};
static MixingPlanY1 DeviseBestMixingPlanY1(const unsigned r, const unsigned g, const unsigned b, uint8_t * pal, uint16_t offset) {
	//const unsigned r = color>>16, g = (color>>8)&0xFF, b = color&0xFF;
	pal += offset * 3;
	offsetGloablY3 = offset;
	MixingPlanY1 result = { {0, 0}, 0.5f };
	float least_penalty = 1e99;

	for (unsigned index1 = 0; index1 < palettesize; ++index1)
		for (unsigned index2 = index1; index2 < palettesize; ++index2)
		{
			// Determine the two component colors
			//unsigned color1 = pal[index1], color2 = pal[index2];
			//unsigned r1 = color1>>16, g1 = (color1>>8)&0xFF, b1 = color1&0xFF;
			//unsigned r2 = color2>>16, g2 = (color2>>8)&0xFF, b2 = color2&0xFF;
			unsigned r1 = pal[index1 * 3], g1 = pal[index1 * 3 + 1], b1 = pal[index1 * 3 + 2];
			unsigned r2 = pal[index2 * 3], g2 = pal[index2 * 3 + 1], b2 = pal[index2 * 3 + 2];
			int ratio = 32;

			if ((r1 != r2) || (g1 != g2) || (b1 != b2)) {
				// Determine the ratio of mixing for each channel.
				//   solve(r1 + ratio*(r2-r1)/64 = r, ratio)
				// Take a weighed average of these three ratios according to the
				// perceived luminosity of each channel (according to CCIR 601).
				ratio = ((r2 != r1 ? 299 * 64 * int(r - r1) / int(r2 - r1) : 0)
				         +  (g2 != g1 ? 587 * 64 * int(g - g1) / int(g2 - g1) : 0)
				         +  (b1 != b2 ? 114 * 64 * int(b - b1) / int(b2 - b1) : 0))
				        / ((r2 != r1 ? 299 : 0)
				           + (g2 != g1 ? 587 : 0)
				           + (b2 != b1 ? 114 : 0));

				if (ratio < 0) ratio = 0;
				else if (ratio > 63) ratio = 63;
			}

			// Determine what mixing them in this proportion will produce
			unsigned r0 = r1 + ratio * int(r2 - r1) / 64;
			unsigned g0 = g1 + ratio * int(g2 - g1) / 64;
			unsigned b0 = b1 + ratio * int(b2 - b1) / 64;
			float penalty = EvaluateMixingError(
			                    r, g, b, r0, g0, b0, r1, g1, b1, r2, g2, b2,
			                    ratio / float(64.0f));

			if (penalty < least_penalty) {
				least_penalty = penalty;
				result.colors[0] = index1;
				result.colors[1] = index2;
				result.ratio = ratio / float(64.0);

				if (penalty == 0.0f)
					return result;
			}
		}

	return result;
}

static MixingPlan DeviseBestMixingPlanY2(uint8_t rIn, uint8_t gIn, uint8_t bIn, uint8_t * pal, uint16_t offset, size_t limit) {
	// Input color in RGB
	int input_rgb[3] = {rIn, gIn, bIn};
	pal += offset * 3;
	offsetGloablY3 = offset;
	// Input color in CIE L*a*b*
#ifndef COMPARE_RGB
	LabItem input((float)rIn / 255.0f, (float)gIn / 255.0f, (float)bIn / 255.0f);
#endif
	// Tally so far (gamma-corrected)
	float so_far[3] = { 0.0f, 0.0f, 0.0f };

	MixingPlan result;

	while (result.size() < limit) {
		unsigned chosen_amount = 1;
		unsigned chosen		= 0;

		const unsigned max_test_count = result.empty() ? 1 : result.size();

		float least_penalty = -1.0f;

		for (unsigned index = 0; index < palettesize; ++index) {
			//const unsigned color = pal[index];
			float sum[3] = { so_far[0], so_far[1], so_far[2] };
			float add[3] = { pal_g[index + offset][0], pal_g[index + offset][1], pal_g[index + offset][2] };

			for (unsigned p = 1; p <= max_test_count; p *= 2) {
				for (unsigned c = 0; c < 3; ++c) sum[c] += add[c];

				for (unsigned c = 0; c < 3; ++c) add[c] += add[c];

				float t = result.size() + p;

				float test[3] = { GammaUncorrect(sum[0] / t),
				                  GammaUncorrect(sum[1] / t),
				                  GammaUncorrect(sum[2] / t)
				                };

#if COMPARE_RGB
				float penalty = ColorCompare(input_rgb[0], input_rgb[1], input_rgb[2], test[0] * 255.0f, test[1] * 255.0f, test[2] * 255.0f);
#else
				LabItem test_lab(test[0], test[1], test[2]);
				float penalty = ColorCompare(test_lab, input);
#endif

				if (penalty < least_penalty || least_penalty < 0) {
					least_penalty = penalty;
					chosen		= index;
					chosen_amount = p;
				}
			}
		}

		// Append "chosen_amount" times "chosen" to the color list
		result.resize(result.size() + chosen_amount, chosen);

		for (unsigned c = 0; c < 3; ++c)
			so_far[c] += pal_g[chosen + offset][c] * chosen_amount;
	}

	// Sort the colors according to luminance
	std::sort(result.begin(), result.end(), PaletteCompareLuma);
	return result;
}
static MixingPlan DeviseBestMixingPlanY3(uint8_t rIn, uint8_t gIn, uint8_t bIn, uint8_t * pal, uint16_t offset, size_t limit) {
	// Input color in RGB
	float input_rgb[3] = {(float)rIn, (float)gIn, (float)bIn};
	pal += offset * 3;
	offsetGloablY3 = offset;
	// Input color in CIE L*a*b*
#ifndef COMPARE_RGB
	LabItem input((float)rIn / 255.0f, (float)gIn / 255.0f, (float)bIn / 255.0f);
#endif
	std::map<unsigned, unsigned> Solution;

	// The penalty of our currently "best" solution.
	float current_penalty = -1.0f;

	// First, find the closest color to the input color.
	// It is our seed.
	{
		unsigned chosen = 0;

		for (unsigned index = 0; index < palettesize; ++index) {
			//const unsigned color = pal[index];
			unsigned r = pal[index * 3], g = pal[(index * 3) + 1], b = pal[(index * 3) + 2];
#if COMPARE_RGB
			//unsigned r = color>>16, g = (color>>8)&0xFF, b = color&0xFF;
			float penalty = ColorCompare(input_rgb[0], input_rgb[1], input_rgb[2], r, g, b);
#else
			LabItem test_lab(r, g, b);
			float penalty = ColorCompare(input, test_lab);
#endif

			if (penalty < current_penalty || current_penalty < 0)
			{
				current_penalty = penalty;
				chosen = index;
			}
		}

		Solution[chosen] = limit;
	}

	float dbllimit = 1.0f / limit;

	while (current_penalty != 0.0f) {
		// Find out if there is a region in Solution that
		// can be split in two for benefit.
		float   best_penalty = current_penalty;
		unsigned best_splitfrom	= ~0u;
		unsigned best_split_to[2]  = { 0, 0};

		for (std::map<unsigned, unsigned>::iterator i = Solution.begin(); i != Solution.end(); ++i) {
			//if(i->second <= 1) continue;
			unsigned split_color = i->first;
			unsigned split_count = i->second;
			// Tally the other colors
			float sum[3] = {0.0f, 0.0f, 0.0f};

			for (std::map<unsigned, unsigned>::iterator j = Solution.begin(); j != Solution.end(); ++j) {
				if (j->first == split_color) continue;

				sum[0] += pal_g[offset + j->first ][0] * j->second * dbllimit;
				sum[1] += pal_g[offset + j->first ][1] * j->second * dbllimit;
				sum[2] += pal_g[offset + j->first ][2] * j->second * dbllimit;
			}

			float portion1 = (split_count / 2.0f) * dbllimit;
			float portion2 = (split_count - split_count / 2.0f) * dbllimit;

			for (unsigned a = 0; a < palettesize; ++a) {
				//if(a != split_color && Solution.find(a) != Solution.end()) continue;
				unsigned firstb = 0;

				if (portion1 == portion2) firstb = a + 1;

				for (unsigned b = firstb; b < palettesize; ++b) {
					if (a == b) continue;

					//if(b != split_color && Solution.find(b) != Solution.end()) continue;
					int lumadiff = int(luma[offset + a]) - int(luma[offset + b]);

					if (lumadiff < 0) lumadiff = -lumadiff;

					if (lumadiff > 80000) continue;

					float test[3] =
					{	GammaUncorrect(sum[0] + pal_g[offset + a][0] * portion1 + pal_g[offset + b][0] * portion2),
						GammaUncorrect(sum[1] + pal_g[offset + a][1] * portion1 + pal_g[offset + b][1] * portion2),
						GammaUncorrect(sum[2] + pal_g[offset + a][2] * portion1 + pal_g[offset + b][2] * portion2)
					};
					// Figure out if this split is better than what we had
#if COMPARE_RGB
					float penalty = ColorCompare(input_rgb[0], input_rgb[1], input_rgb[2], test[0] * 255, test[1] * 255, test[2] * 255);
#else
					LabItem test_lab( test[0], test[1], test[2] );
					float penalty = ColorCompare(input, test_lab);
#endif

					if (penalty < best_penalty) {
						best_penalty   = penalty;
						best_splitfrom = split_color;
						best_split_to[0] = a;
						best_split_to[1] = b;
					}

					if (portion2 == 0) break;
				}
			}
		}

		if (best_penalty == current_penalty) break; // No better solution was found.

		std::map<unsigned, unsigned>::iterator i = Solution.find(best_splitfrom);
		unsigned split_count = i->second, split1 = split_count / 2, split2 = split_count - split1;
		Solution.erase(i);

		if (split1 > 0) Solution[best_split_to[0]] += split1;

		if (split2 > 0) Solution[best_split_to[1]] += split2;

		current_penalty = best_penalty;
	}

	// Sequence the solution.
	MixingPlan result;

	for (std::map<unsigned, unsigned>::iterator
	        i = Solution.begin(); i != Solution.end(); ++i)
		result.resize(result.size() + i->second, i->first);

	// Sort the colors according to luminance
	std::sort(result.begin(), result.end(), PaletteCompareLuma);
	return result;
}
static inline uint8_t addCheck(uint16_t val, uint8_t add) {
	val += add;
	return (val > 255) ? 255 : val;
}
#define plus_truncate_uchar(a, b) \
	if (((int)(a)) + (b) < 0) \
		(a) = 0; \
	else if (((int)(a)) + (b) > 255) \
		(a) = 255; \
	else \
		(a) += (b);
/* 8x8 threshold map (note: the patented pattern dithering algorithm uses 4x4) */
static const unsigned char mapTK[8 * 8] = {
	0, 48, 12, 60, 3, 51, 15, 63,
	32, 16, 44, 28, 35, 19, 47, 31,
	8, 56, 4, 52, 11, 59, 7, 55,
	40, 24, 36, 20, 43, 27, 39, 23,
	2, 50, 14, 62, 1, 49, 13, 61,
	34, 18, 46, 30, 33, 17, 45, 29,
	10, 58, 6, 54, 9, 57, 5, 53,
	42, 26, 38, 22, 41, 25, 37, 21
};
/* Luminance for each palette entry, to be initialized as soon as the program begins */
struct MixingPlanTK {
	unsigned colors[64];
};
static MixingPlanTK DeviseBestMixingPlanTK(uint8_t rIn, uint8_t gIn, uint8_t bIn, uint8_t * pal, uint16_t offset) {
	// Input color in RGB
	pal += offset * 3;
	offsetGloablY3 = offset;
	MixingPlanTK result = { {0} };
	const int src[3] = {rIn, gIn, bIn};

	int e[3] = { 0, 0, 0 }; // Error accumulator

	for (unsigned c = 0; c < 64; ++c) {
		// Current temporary value
		int t[3] = { src[0] + e[0] * 9 / 100, src[1] + e[1] * 9 / 100, src[2] + e[2] * 9 / 100 };

		// Clamp it in the allowed RGB range
		if (t[0] < 0) t[0] = 0;
		else if (t[0] > 255) t[0] = 255;

		if (t[1] < 0) t[1] = 0;
		else if (t[1] > 255) t[1] = 255;

		if (t[2] < 0) t[2] = 0;
		else if (t[2] > 255) t[2] = 255;

		// Find the closest color from the palette
		double least_penalty = 1e99;
		unsigned chosen = c % palettesize;

		for (unsigned index = 0; index < palettesize; ++index) {
			const int pc[3] = {pal[index * 3], pal[index * 3 + 1], pal[index * 3 + 2]};
			double penalty = ColorCompare(pc[0], pc[1], pc[2], t[0], t[1], t[2]);

			if (penalty < least_penalty) {
				least_penalty = penalty;
				chosen = index;
			}
		}

		// Add it to candidates and update the error
		result.colors[c] = chosen;
		const int pc[3] = {pal[chosen * 3], pal[chosen * 3 + 1], pal[chosen * 3 + 2]};
		e[0] += src[0] - pc[0];
		e[1] += src[1] - pc[1];
		e[2] += src[2] - pc[2];
	}

	// Sort the colors according to luminance
	std::sort(result.colors, result.colors + 64, PaletteCompareLuma);
	return result;
}
static void progressUpdate(Fl_Window**win, Fl_Progress**progress, time_t&lasttime, bool&progressHave, unsigned cur, unsigned total) {
	if ((time(NULL) - lasttime) >= 1) {
		lasttime = time(NULL);

		if (!progressHave) {
			progressHave = true;
			mkProgress(win, progress);
			(*progress)->maximum(total);
			Fl::check();
		}

		char txtbuf[128];
		snprintf(txtbuf, sizeof(txtbuf), "%d/%d", cur, total);
		txtbuf[sizeof(txtbuf) - 1] = 0;
		(*progress)->copy_label(txtbuf);
		(*progress)->value(cur);
		Fl::check();
	}
}
void*ditherImage(uint8_t * image, uint32_t w, uint32_t h, bool useAlpha, bool colSpace, bool forceRow, unsigned forcedrow, bool isChunk, uint32_t idChunk, bool isSprite, bool toIndex, int forceAlg, int forceTileIdx) {
	void*retPtr = 0;

	unsigned ditherAlg = forceAlg >= 0 ? forceAlg : currentProject->settings & settingsDitherMask;

	if (ditherAlg != 1) { // Riemersma does not support toIndex.
		if (colSpace && toIndex)
			retPtr = malloc(w * h * ((currentProject->pal->esize == 0) ? 1 : currentProject->pal->esize));
		else if (toIndex)
			retPtr = malloc(w * h);
	}

	if ((!isSprite) && (currentProject->gameSystem == TMS9918) && (currentProject->getTMS9918subSys() == MODE_3))
		colSpace = true; // The entire palette is available for all pixels.

	uint8_t*indexPtr = (uint8_t*)retPtr;
	int ditherSetting = ((currentProject->settings >> subsettingsDitherShift)&subsettingsDitherMask) + 1;
	unsigned type_temp = palTypeGen;
	unsigned temp = 0;
	rgbPixelsize = useAlpha ? 4 : 3;
	unsigned tileWidth = currentProject->tileC->width();
	unsigned tileHeight = currentProject->tileC->height();
	unsigned rgbRowsize = tileWidth * rgbPixelsize;
	unsigned x, y;
	uint_fast8_t r_old, g_old, b_old, a_old;
	uint_fast8_t r_new, g_new, b_new, a_new;
	unsigned pal_row;
	int_fast16_t error_rgb[4];
	Fl_Window *win;
	Fl_Progress *progress;
	bool progressHave = false;
	time_t lasttime = time(NULL);
	bool haveExt = (!isSprite) && currentProject->hasExtAttrs();

	switch (ditherAlg) {
		case 7:
		case 6:
		case 5://Yliluoma's ordered dithering algorithms
		case 4:
		{
			if (colSpace && currentProject->gameSystem != TMS9918)
				return 0;

			unsigned tempPalSize;
			uint8_t * colPtr = currentProject->pal->rgbPal;

			if (currentProject->pal->haveAlt && isSprite) {
				tempPalSize = currentProject->pal->colorCntalt;
				palettesize = currentProject->pal->perRowalt;
				colPtr += palettesize * 3;
			} else {
				tempPalSize = currentProject->pal->colorCnt;
				palettesize = currentProject->pal->perRow;
			}

			for (unsigned c = 0; c < tempPalSize; ++c) {
				unsigned r = colPtr[c * 3], g = colPtr[(c * 3) + 1], b = colPtr[(c * 3) + 2];
				luma[c] = r * 299 + g * 587 + b * 114;
				pal_g[c][0] = GammaCorrect(r / 255.0);
				pal_g[c][1] = GammaCorrect(g / 255.0);
				pal_g[c][2] = GammaCorrect(b / 255.0);
			}

			for (y = 0; y < h; ++y) {
				for (x = 0; x < w; ++x) {
					r_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize)];
					g_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 1];
					b_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 2];

					if (useAlpha)
						a_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 3];

					if (currentProject->gameSystem == segaGenesis && type_temp != 0) {
						unsigned tempSet;

						if (isChunk)
							tempSet = (currentProject->Chunk->getPrio_t(idChunk, x / tileWidth, y / 8) ^ 1) * 8;
						else
							tempSet = (currentProject->tms->maps[currentProject->curPlane].get_prio(x / tileWidth, y / 8) ^ 1) * 8;

						set_palette_type_force(tempSet);//0 normal 8 shadowed 16 highlighted
					}

					unsigned tempPalOff;

					if (forceRow)
						pal_row = forcedrow;
					else {
						if (isChunk)
							pal_row = currentProject->Chunk->getTileRow_t(idChunk, x / tileWidth, y / 8);
						else
							pal_row = currentProject->tms->maps[currentProject->curPlane].getPalRow(x / tileWidth, y / 8);
					}

					if (ditherAlg == 7) {
						unsigned map_value = mapTK[(x & 7) + ((y & 7) << 3)];
						MixingPlanTK plan;
						plan = DeviseBestMixingPlanTK(r_old, g_old, b_old, currentProject->pal->rgbPal, pal_row * palettesize);
						tempPalOff = (plan.colors[map_value] + (pal_row * palettesize));
					} else if (ditherAlg == 4) {
						float map_value = mapY1[(x & 7) + ((y & 7) << 3)];
						MixingPlanY1 plan;
						plan = DeviseBestMixingPlanY1(r_old, g_old, b_old, currentProject->pal->rgbPal, pal_row * palettesize);
						tempPalOff = (plan.colors[map_value < plan.ratio ? 1 : 0] + (pal_row * palettesize));
					} else {
						unsigned map_value = mapY3[(x & 7) + ((y & 7) << 3)];
						MixingPlan plan;

						if (ditherAlg == 5)
							plan = DeviseBestMixingPlanY2(r_old, g_old, b_old, currentProject->pal->rgbPal, pal_row * palettesize, 16);
						else
							plan = DeviseBestMixingPlanY3(r_old, g_old, b_old, currentProject->pal->rgbPal, pal_row * palettesize, 16);

						map_value = map_value * plan.size() / 64;
						tempPalOff = (plan[ map_value ] + (pal_row * palettesize));
					}

					if (toIndex) {
						if (useAlpha)
							indexPtr[x + (y * w)] = a_old >= 128 ? tempPalOff : 0; //TODO dither alpha with these algorithms
						else
							indexPtr[x + (y * w)] = tempPalOff;
					}

					tempPalOff *= 3;
					image[(x * rgbPixelsize) + (y * w * rgbPixelsize)] = currentProject->pal->rgbPal[tempPalOff];
					image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 1] = currentProject->pal->rgbPal[tempPalOff + 1];
					image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 2] = currentProject->pal->rgbPal[tempPalOff + 2];
				}

				progressUpdate(&win, &progress, lasttime, progressHave, y, h);
			}
		}
		break;

		case 2://nearest color
		case 3://vertical dithering
			for (y = 0; y < h; ++y) {
				for (x = 0; x < w * rgbPixelsize; x += rgbPixelsize) {
					r_old = image[x + (y * w * rgbPixelsize)];
					g_old = image[x + (y * w * rgbPixelsize) + 1];
					b_old = image[x + (y * w * rgbPixelsize) + 2];

					if (useAlpha)
						a_old = image[x + (y * w * rgbPixelsize) + 3];

					if (!colSpace && !haveExt) {
						if (forceRow)
							pal_row = forcedrow;
						else {
							if (isChunk)
								pal_row = currentProject->Chunk->getTileRow_t(idChunk, x / rgbRowsize, y / 8);
							else
								pal_row = currentProject->tms->maps[currentProject->curPlane].getPalRow(x / rgbRowsize, y / 8);
						}
					}

					//find nearest color
					uint8_t half = 18;

					if (currentProject->gameSystem == segaGenesis && type_temp != 0) {
						unsigned tempSet;

						if (isChunk)
							tempSet = (currentProject->Chunk->getPrio_t(idChunk, x / rgbRowsize, y / 8) ^ 1) * 8;
						else
							tempSet = (currentProject->tms->maps[currentProject->curPlane].get_prio(x / rgbRowsize, y / 8) ^ 1) * 8;

						set_palette_type_force(tempSet);//0 normal 8 shadowed 16 highlighted
						half = (tempSet == 0) ? 18 : 9;
					}

					if (colSpace) {
						switch (currentProject->gameSystem) {
							case segaGenesis:
								r_new = palTab[nearest_color_index(r_old)];
								g_new = palTab[nearest_color_index(g_old)];
								b_new = palTab[nearest_color_index(b_old)];
								break;

							case NES:
							{	uint32_t temprgb = toNesRgb(r_old, g_old, b_old);
								b_new = temprgb & 255;
								g_new = (temprgb >> 8) & 255;
								r_new = (temprgb >> 16) & 255;
							}
							break;

							case masterSystem:
								r_new = palTabMasterSystem[nearestOneChannel(r_old, palTabMasterSystem, 4)];
								g_new = palTabMasterSystem[nearestOneChannel(g_old, palTabMasterSystem, 4)];
								b_new = palTabMasterSystem[nearestOneChannel(b_old, palTabMasterSystem, 4)];
								break;

							case gameGear:
								r_new = palTabGameGear[nearestOneChannel(r_old, palTabGameGear, 16)];
								g_new = palTabGameGear[nearestOneChannel(g_old, palTabGameGear, 16)];
								b_new = palTabGameGear[nearestOneChannel(b_old, palTabGameGear, 16)];
								break;

							case TMS9918:
								temp = currentProject->pal->nearestColIndex(r_old, g_old, b_old, currentProject->pal->rgbPal, currentProject->pal->colorCnt) * 3;

								if (toIndex)
									indexPtr[(x / rgbPixelsize) + (y * w)] = temp / 3;

								r_new = currentProject->pal->rgbPal[temp];
								g_new = currentProject->pal->rgbPal[temp + 1];
								b_new = currentProject->pal->rgbPal[temp + 2];
								break;

							default:
								show_default_error
						}
					} else {
						if (ditherAlg == 3) {
							if ((x / rgbPixelsize) & 1) {
								r_old = addCheck(r_old, half);
								g_old = addCheck(g_old, half);
								b_old = addCheck(b_old, half);

								if (useAlpha)
									a_old = addCheck(a_old, half);
							}
						}

						if (haveExt) {
							unsigned curTile;

							if (forceTileIdx < 0) {
								if (isChunk)
									curTile = currentProject->Chunk->getTile_t(idChunk, x / rgbRowsize, y / 8);
								else
									curTile = currentProject->tms->maps[currentProject->curPlane].get_tile(x / rgbRowsize, y / 8);
							} else
								curTile = forceTileIdx;

							unsigned extAttr = currentProject->tileC->getExtAttr(curTile, y % 8);
							temp = chooseTwoColor(extAttr & 15, extAttr >> 4, r_old, g_old, b_old) * 3;
							r_new = currentProject->pal->rgbPal[temp];
							g_new = currentProject->pal->rgbPal[temp + 1];
							b_new = currentProject->pal->rgbPal[temp + 2];
							temp = 3 * ((temp / 3) == (extAttr >> 4));
						} else if ((currentProject->pal->haveAlt) && isSprite) {
							temp = find_near_color_from_row_rgb(pal_row, r_old, g_old, b_old, true);
							r_new = currentProject->pal->rgbPal[temp + (currentProject->pal->colorCnt * 3)];
							g_new = currentProject->pal->rgbPal[temp + 1 + (currentProject->pal->colorCnt * 3)];
							b_new = currentProject->pal->rgbPal[temp + 2 + (currentProject->pal->colorCnt * 3)];
						} else {
							temp = find_near_color_from_row_rgb(pal_row, r_old, g_old, b_old, false);
							r_new = currentProject->pal->rgbPal[temp];
							g_new = currentProject->pal->rgbPal[temp + 1];
							b_new = currentProject->pal->rgbPal[temp + 2];
						}

						if (toIndex) {
							if (useAlpha)
								indexPtr[(x / rgbPixelsize) + (y * w)] = a_old >= 128 ? temp / 3 : 0;
							else
								indexPtr[(x / rgbPixelsize) + (y * w)] = temp / 3;
						}
					}

					if (useAlpha)
						a_old = (a_old & 128) ? 255 : 0;

					image[x + (y * w * rgbPixelsize)] = r_new;
					image[x + (y * w * rgbPixelsize) + 1] = g_new;
					image[x + (y * w * rgbPixelsize) + 2] = b_new;

					if (useAlpha)
						image[x + (y * w * rgbPixelsize) + 3] = a_old;
				}

				progressUpdate(&win, &progress, lasttime, progressHave, y, h);
			}

			break;

		case 1:
			useMode = currentProject->gameSystem;
			USEofColGlob = colSpace;
			isSpriteG = isSprite;
			shouldForceRow = forceRow;
			selectedForceRow = forcedrow;
			isChunkD_G = isChunk;
			idChunk_G = idChunk;
			Riemersma(image, w, h, 0);
			progressUpdate(&win, &progress, lasttime, progressHave, 1, useAlpha ? 4 : 3);
			Riemersma(image, w, h, 1);
			progressUpdate(&win, &progress, lasttime, progressHave, 2, useAlpha ? 4 : 3);
			Riemersma(image, w, h, 2);
			progressUpdate(&win, &progress, lasttime, progressHave, 3, useAlpha ? 4 : 3);

			if (useAlpha) {
				useMode = 255;
				Riemersma(image, w, h, 3);
			}


			break;

		case 0:
			for (y = 0; y < h; y++) {
				for (x = 0; x < w; x++) {
					//we need to get nearest color
					r_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize)];
					g_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 1];
					b_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 2];

					if (useAlpha)
						a_old = image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 3];

					if (!colSpace && !haveExt) {
						if (forceRow)
							pal_row = forcedrow;
						else {
							if (isChunk)
								pal_row = currentProject->Chunk->getTileRow_t(idChunk, x / tileWidth, y / 8);
							else
								pal_row = currentProject->tms->maps[currentProject->curPlane].getPalRow(x / tileWidth, y / 8);
						}
					}

					//find nearest color
					if (currentProject->gameSystem == segaGenesis && type_temp != 0) {
						unsigned tempSet;

						if (isChunk)
							tempSet = (currentProject->Chunk->getPrio_t(idChunk, x / tileWidth, y / 8) ^ 1) * 8;
						else
							tempSet = (currentProject->tms->maps[currentProject->curPlane].get_prio(x / tileWidth, y / 8) ^ 1) * 8;

						set_palette_type_force(tempSet);//0 normal 8 shadowed 16 highlighted
					}

					if (colSpace) {
						switch (currentProject->gameSystem) {
							case segaGenesis:
								r_new = palTab[nearest_color_index(r_old)];
								g_new = palTab[nearest_color_index(g_old)];
								b_new = palTab[nearest_color_index(b_old)];
								break;

							case NES:
							{	uint32_t temprgb = toNesRgb(r_old, g_old, b_old);
								b_new = temprgb & 255;
								g_new = (temprgb >> 8) & 255;
								r_new = (temprgb >> 16) & 255;
							}
							break;

							case masterSystem:
								r_new = palTabMasterSystem[nearestOneChannel(r_old, palTabMasterSystem, 4)];
								g_new = palTabMasterSystem[nearestOneChannel(g_old, palTabMasterSystem, 4)];
								b_new = palTabMasterSystem[nearestOneChannel(b_old, palTabMasterSystem, 4)];
								break;

							case gameGear:
								r_new = palTabGameGear[nearestOneChannel(r_old, palTabGameGear, 16)];
								g_new = palTabGameGear[nearestOneChannel(g_old, palTabGameGear, 16)];
								b_new = palTabGameGear[nearestOneChannel(b_old, palTabGameGear, 16)];
								break;

							case TMS9918:
								temp = currentProject->pal->nearestColIndex(r_old, g_old, b_old, currentProject->pal->rgbPal, currentProject->pal->colorCnt) * 3;

								if (toIndex)
									indexPtr[x + (y * w)] = temp / 3;

								r_new = currentProject->pal->rgbPal[temp];
								g_new = currentProject->pal->rgbPal[temp + 1];
								b_new = currentProject->pal->rgbPal[temp + 2];
								break;

							default:
								show_default_error
						}
					} else {
						if (haveExt) {
							unsigned curTile;

							if (forceTileIdx < 0) {
								if (isChunk)
									curTile = currentProject->Chunk->getTile_t(idChunk, x / tileWidth, y / 8);
								else
									curTile = currentProject->tms->maps[currentProject->curPlane].get_tile(x / tileWidth, y / 8);
							} else
								curTile = forceTileIdx;

							unsigned extAttr = currentProject->tileC->getExtAttr(curTile, y % 8);
							temp = chooseTwoColor(extAttr & 15, extAttr >> 4, r_old, g_old, b_old) * 3;
							r_new = currentProject->pal->rgbPal[temp];
							g_new = currentProject->pal->rgbPal[temp + 1];
							b_new = currentProject->pal->rgbPal[temp + 2];
							temp = 3 * ((temp / 3) == (extAttr >> 4));
						} else if (isSprite && (currentProject->gameSystem == TMS9918)) {
							temp = chooseTwoColor(0, pal_row, r_old, g_old, b_old) * 3;
							r_new = currentProject->pal->rgbPal[temp];
							g_new = currentProject->pal->rgbPal[temp + 1];
							b_new = currentProject->pal->rgbPal[temp + 2];
							temp = 3 * ((temp / 3) == pal_row);
						} else {
							temp = find_near_color_from_row_rgb(pal_row, r_old, g_old, b_old, (currentProject->pal->haveAlt) && isSprite);
							r_new = currentProject->pal->rgbPal[temp];
							g_new = currentProject->pal->rgbPal[temp + 1];
							b_new = currentProject->pal->rgbPal[temp + 2];
						}
					}

					if (useAlpha) {
						a_new = (a_old & 128) ? 255 : 0;
						error_rgb[3] = (int_fast16_t)a_old - (int_fast16_t)a_new;
						image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 3] = a_new;
					}

					if (toIndex) {
						if (useAlpha)
							indexPtr[x + (y * w)] = a_new >= 128 ? temp / 3 : 0;
						else
							indexPtr[x + (y * w)] = temp / 3;
					}

					error_rgb[0] = (int_fast16_t)r_old - (int_fast16_t)r_new;
					error_rgb[1] = (int_fast16_t)g_old - (int_fast16_t)g_new;
					error_rgb[2] = (int_fast16_t)b_old - (int_fast16_t)b_new;

					for (unsigned channel = 0; channel < rgbPixelsize; ++channel) {
						//add the offset
						if (x + 1 < w)
							plus_truncate_uchar(image[((x + 1)*rgbPixelsize) + (y * w * rgbPixelsize) + channel], (error_rgb[channel] * 7) / ditherSetting);

						if (x - 1 > 0 && y + 1 < h)
							plus_truncate_uchar(image[((x - 1)*rgbPixelsize) + ((y + 1)*w * rgbPixelsize) + channel], (error_rgb[channel] * 3) / ditherSetting);

						if (y + 1 < h)
							plus_truncate_uchar(image[(x * rgbPixelsize) + ((y + 1)*w * rgbPixelsize) + channel], (error_rgb[channel] * 5) / ditherSetting);

						if (x + 1 < w && y + 1 < h)
							plus_truncate_uchar(image[((x + 1)*rgbPixelsize) + ((y + 1)*w * rgbPixelsize) + channel], (error_rgb[channel]) / ditherSetting);
					}

					image[(x * rgbPixelsize) + (y * w * rgbPixelsize)] = r_new;
					image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 1] = g_new;
					image[(x * rgbPixelsize) + (y * w * rgbPixelsize) + 2] = b_new;
				}

				progressUpdate(&win, &progress, lasttime, progressHave, y, h);
			}

			break;
	}

	if (currentProject->gameSystem == segaGenesis)
		set_palette_type();

	if (progressHave) {
		win->remove(progress);// remove progress bar from window
		delete (progress); // deallocate it
		delete win;
	}

	if (ditherAlg == 1 && toIndex)
		return ditherImage(image, w, h, useAlpha, colSpace, forceRow, forcedrow, isChunk, idChunk, isSprite, toIndex, 2);
	else
		return retPtr;
}
