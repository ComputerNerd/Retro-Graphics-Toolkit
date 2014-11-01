#include "CIE.h"
#include <cmath>
//Conversion code from http://www.getreuer.info/home/colorspace
/** 
 * @brief Inverse sRGB gamma correction, transforms R' to R 
 */
#define INVGAMMACORRECTION(t)	\
	(((t) <= 0.0404482362771076) ? \
	((t)/12.92) : pow(((t) + 0.055)/1.055, 2.4))
/** @brief XYZ color of the D65 white point */
#define WHITEPOINT_X	0.950456
#define WHITEPOINT_Y	1.0
#define WHITEPOINT_Z	1.088754
#define LABF(t)	\
	((t >= 8.85645167903563082e-3) ? \
	pow(t,0.333333333333333) : (841.0/108.0)*(t) + (4.0/29.0))
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
static void Rgb2Xyz(double*X,double*Y,double*Z,double R,double G,double B){
	R = INVGAMMACORRECTION(R);
	G = INVGAMMACORRECTION(G);
	B = INVGAMMACORRECTION(B);
	*X = (double)(0.4123955889674142161*R + 0.3575834307637148171*G + 0.1804926473817015735*B);
	*Y = (double)(0.2125862307855955516*R + 0.7151703037034108499*G + 0.07220049864333622685*B);
	*Z = (double)(0.01929721549174694484*R + 0.1191838645808485318*G + 0.9504971251315797660*B);
}
/**
 * Convert CIE XYZ to CIE L*a*b* (CIELAB) with the D65 white point
 *
 * @param L, a, b pointers to hold the result
 * @param X, Y, Z the input XYZ values
 *
 * Wikipedia: http://en.wikipedia.org/wiki/Lab_color_space
 */
static void Xyz2Lab(double *L, double *a, double *b, double X, double Y, double Z){
	X /= WHITEPOINT_X;
	Y /= WHITEPOINT_Y;
	Z /= WHITEPOINT_Z;
	X = LABF(X);
	Y = LABF(Y);
	Z = LABF(Z);
	*L = 116.0*Y - 16.0;
	*a = 500.0*(X - Y);
	*b = 200.0*(Y - Z);
}
void Rgb2Lab(double R,double G,double B,double * L,double * a,double * b){
	double X, Y, Z;
	Rgb2Xyz(&X,&Y,&Z,R,G,B);
	Xyz2Lab(L, a, b, X, Y, Z);
}
void Rgb2Lab255(uint8_t ri,uint8_t gi,uint8_t bi,double * L,double * a,double * b){
	double R=double(ri)/255.0,G=double(gi)/255.0,B=double(bi)/255.0;
	Rgb2Lab(R,G,B,L,a,b);
}
