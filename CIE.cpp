#include "CIE.h"
#include <cmath>
//Conversion code from http://www.getreuer.info/home/colorspace
/*
 * == License (BSD) ==
 * Copyright (c) 2005-2010, Pascal Getreuer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @brief Min of A and B */
#define MIN(A,B)	(((A) <= (B)) ? (A) : (B))

/** @brief Max of A and B */
#define MAX(A,B)	(((A) >= (B)) ? (A) : (B))

/** @brief Min of A, B, and C */
#define MIN3(A,B,C)	(((A) <= (B)) ? MIN(A,C) : MIN(B,C))

/** @brief Max of A, B, and C */
#define MAX3(A,B,C)	(((A) >= (B)) ? MAX(A,C) : MAX(B,C))


/**
 * @brief sRGB gamma correction, transforms R to R'
 * http://en.wikipedia.org/wiki/SRGB
 */
#define GAMMACORRECTION(t)	\
	(((t) <= 0.0031306684425005883) ? \
	(12.92*(t)) : (1.055*std::pow((t), 0.416666666666666667) - 0.055))


/**
 * @brief Inverse sRGB gamma correction, transforms R' to R
 */
#define INVGAMMACORRECTION(t)	\
	(((t) <= 0.0404482362771076) ? \
	((t)/12.92) : std::pow(((t) + 0.055)/1.055, 2.4))
/** @brief XYZ color of the D65 white point */
#define WHITEPOINT_X	0.950456
#define WHITEPOINT_Y	1.0
#define WHITEPOINT_Z	1.088754
#define LABF(t)	\
	((t >= 8.85645167903563082e-3) ? \
	std::pow(t,0.333333333333333) : (841.0/108.0)*(t) + (4.0/29.0))
/**
 * @brief CIE L*a*b* inverse f function
 * http://en.wikipedia.org/wiki/Lab_color_space
 */
#define LABINVF(t)	\
	((t >= 0.206896551724137931) ? \
	((t)*(t)*(t)) : (108.0/841.0)*((t) - (4.0/29.0)))

/**
 * @brief Transform sRGB to CIE XYZ with the D65 white point
 *
 * @param X, Y, Z pointers to hold the result
 * @param R, G, B the input sRGB values
 *
 * Poynton, "Frequently Asked Questions About Color," page 10
 * Wikipedia: http://en.wikipedia.org/wiki/SRGB
 * Wikipedia: http://en.wikipedia.org/wiki/CIE_1931_color_space
 */
static void Rgb2Xyz(double*X, double*Y, double*Z, double R, double G, double B) {
	R = INVGAMMACORRECTION(R);
	G = INVGAMMACORRECTION(G);
	B = INVGAMMACORRECTION(B);
	*X = (double)(0.4123955889674142161 * R + 0.3575834307637148171 * G + 0.1804926473817015735 * B);
	*Y = (double)(0.2125862307855955516 * R + 0.7151703037034108499 * G + 0.07220049864333622685 * B);
	*Z = (double)(0.01929721549174694484 * R + 0.1191838645808485318 * G + 0.9504971251315797660 * B);
}
/**
 * Convert CIE XYZ to CIE L*a*b* (CIELAB) with the D65 white point
 *
 * @param L, a, b pointers to hold the result
 * @param X, Y, Z the input XYZ values
 *
 * Wikipedia: http://en.wikipedia.org/wiki/Lab_color_space
 */
static void Xyz2Lab(double *L, double *a, double *b, double X, double Y, double Z) {
	X /= WHITEPOINT_X;
	Y /= WHITEPOINT_Y;
	Z /= WHITEPOINT_Z;
	X = LABF(X);
	Y = LABF(Y);
	Z = LABF(Z);
	*L = 116.0 * Y - 16.0;
	*a = 500.0 * (X - Y);
	*b = 200.0 * (Y - Z);
}
/**
 * Convert CIE L*a*b* (CIELAB) to CIE XYZ with the D65 white point
 *
 * @param X, Y, Z pointers to hold the result
 * @param L, a, b the input L*a*b* values
 *
 * Wikipedia: http://en.wikipedia.org/wiki/Lab_color_space
 */
static void Lab2Xyz(double *X, double *Y, double *Z, double L, double a, double b) {
	L = (L + 16) / 116;
	a = L + a / 500;
	b = L - b / 200;
	*X = WHITEPOINT_X * LABINVF(a);
	*Y = WHITEPOINT_Y * LABINVF(L);
	*Z = WHITEPOINT_Z * LABINVF(b);
}
/**
 * @brief Transform CIE XYZ to sRGB with the D65 white point
 *
 * @param R, G, B pointers to hold the result
 * @param X, Y, Z the input XYZ values
 *
 * Official sRGB specification (IEC 61966-2-1:1999)
 * Poynton, "Frequently Asked Questions About Color," page 10
 * Wikipedia: http://en.wikipedia.org/wiki/SRGB
 * Wikipedia: http://en.wikipedia.org/wiki/CIE_1931_color_space
 */
static void Xyz2Rgb(double *R, double *G, double *B, double X, double Y, double Z) {
	double R1, B1, G1, Min;


	R1 = (double)( 3.2406 * X - 1.5372 * Y - 0.4986 * Z);
	G1 = (double)(-0.9689 * X + 1.8758 * Y + 0.0415 * Z);
	B1 = (double)( 0.0557 * X - 0.2040 * Y + 1.0570 * Z);

	Min = MIN3(R1, G1, B1);

	/* Force nonnegative values so that gamma correction is well-defined. */
	if (Min < 0)
	{
		R1 -= Min;
		G1 -= Min;
		B1 -= Min;
	}

	/* Transform from RGB to R'G'B' */
	*R = GAMMACORRECTION(R1);
	*G = GAMMACORRECTION(G1);
	*B = GAMMACORRECTION(B1);
}
void Rgb2Lab(double *L, double *a, double *b, double R, double G, double B) {
	double X, Y, Z;
	Rgb2Xyz(&X, &Y, &Z, R, G, B);
	Xyz2Lab(L, a, b, X, Y, Z);
}
void Rgb2Lab255(double *L, double *a, double *b, unsigned r, unsigned g, unsigned bl) {
	double R = double(r) / 255.0, G = double(g) / 255.0, B = double(bl) / 255.0;
	Rgb2Lab(L, a, b, R, G, B);
}
void Lab2Rgb(double *R, double *G, double *B, double L, double a, double b) {
	double X, Y, Z;
	Lab2Xyz(&X, &Y, &Z, L, a, b);
	Xyz2Rgb(R, G, B, X, Y, Z);
}
static uint8_t dToClip(double val) {
	long v = std::lround(val * 255.0);

	if (v < 0)
		v = 0;

	if (v > 255)
		v = 255;

	return v;
}
void Lab2Rgb255(uint8_t*r, uint8_t *g, uint8_t *blue, double L, double a, double b) {
	double R, G, B;
	Lab2Rgb(&R, &G, &B, L, a, b);
	*r = dToClip(R);
	*g = dToClip(G);
	*blue = dToClip(B);
}
/**
 * Convert CIE XYZ to CIE L*C*H* with the D65 white point
 *
 * @param L, C, H pointers to hold the result
 * @param X, Y, Z the input XYZ values
 *
 * CIE L*C*H* is related to CIE L*a*b* by
 *    a* = C* cos(H* pi/180),
 *    b* = C* sin(H* pi/180).
 */
static void Xyz2Lch(double *L, double *C, double *H, double X, double Y, double Z) {
	double a, b;


	Xyz2Lab(L, &a, &b, X, Y, Z);
	*C = std::sqrt(a * a + b * b);
	*H = std::atan2(b, a) * 180.0 / M_PI;

	if (*H < 0.)
		*H += 360.;
}

/**
 * Convert CIE L*C*H* to CIE XYZ with the D65 white point
 *
 * @param X, Y, Z pointers to hold the result
 * @param L, C, H the input L*C*H* values
 */
static void Lch2Xyz(double *X, double *Y, double *Z, double L, double C, double H) {
	double a = C * std::cos(H * (M_PI / 180.0));
	double b = C * std::sin(H * (M_PI / 180.0));


	Lab2Xyz(X, Y, Z, L, a, b);
}

void Rgb2Lch(double *L, double *C, double *H, double R, double G, double B) {
	double X, Y, Z;
	Rgb2Xyz(&X, &Y, &Z, R, G, B);
	Xyz2Lch(L, C, H, X, Y, Z);
}
void Rgb2Lch255(double *L, double *C, double *H, unsigned R, unsigned G, unsigned B) {
	Rgb2Lch(L, C, H, double(R) / 255.0, double(G) / 255.0, double(B) / 255.0);
}
void Lch2Rgb(double *R, double *G, double *B, double L, double C, double H) {
	double X, Y, Z;
	Lch2Xyz(&X, &Y, &Z, L, C, H);
	Xyz2Rgb(R, G, B, X, Y, Z);
}
void Lch2Rgb255(uint8_t*r, uint8_t *g, uint8_t *b, double L, double C, double H) {
	double R, G, B;
	Lch2Rgb(&R, &G, &B, L, C, H);
	*r = dToClip(R);
	*g = dToClip(G);
	*b = dToClip(B);
}
