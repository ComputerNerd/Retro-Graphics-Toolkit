#include "global.h"
#include <cmath>
#include "color_compare.h"
//From http://www.compuphase.com/cmetric.htm
uint32_t ColourDistance(int r1,int g1,int b1, int r2,int g2,int b2){
	int_fast32_t rmean = ( (int_fast32_t)r1 + (int_fast32_t)r2 ) / 2;
	int_fast32_t r = (int_fast32_t)r1 - (int32_t)r2;
	int_fast32_t g = (int_fast32_t)g1 - (int_fast32_t)g2;
	int_fast32_t b = (int_fast32_t)b1 - (int_fast32_t)b2;
	return ((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
}
static inline double square(double x){
	return x*x;
}
//macros from http://www.getreuer.info/home/colorspace
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
void Rgb2Xyz(double *X, double *Y, double *Z, uint8_t RI, uint8_t GI, uint8_t BI){
	double R=double(RI)/255.0,G=double(GI)/255.0,B=double(BI)/255.0;
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
void Xyz2Lab(double *L, double *a, double *b, double X, double Y, double Z){
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
void Lab2Lch(double *C, double *H,double a, double b){
	*C = sqrt(a*a + b*b);
	*H = atan2(b, a)*180.0/M_PI;
	
	if(*H < 0)
		*H += 360;
}
double ciede2000(double L1,double a1,double b1,double L2,double a2,double b2,double Kl,double Kc,double Kh){
	double C1,C2;
	C1=sqrt(square(a1)+square(b1));
	C2=sqrt(square(a2)+square(b2));
	double Cab=(C1+C2)/2.0;
	double temp=pow(Cab,7.0);
	double G=0.5*(1.0-(sqrt(temp/(temp+6103515625.0))));
	a1=(1.0+G)*a1;
	a2=(1.0+G)*a2;
	C1=sqrt(square(a1)+square(b1));
	C2=sqrt(square(a2)+square(b2));
	double h1,h2;
	if((a1!=0.0)||(b1!=0.0)){
		h1=atan2(b1,a1);
		if(h1<0.0)
			h1+=2.0*PI;
	}else
		h1=0.0;
	if((a2!=0.0)||(b2!=0.0)){
		h2=atan2(b2,a2);
		if(h2<0.0)
			h2+=2.0*PI;
	}else
		h2=0.0;
	double dL,dC;
	dL=L2-L1;
	dC=C2-C1;
	double dh;
	dh=h2-h1;
	if(C1==0.0&&C2==0.0)
		dh=0.0;
	else if(dh>PI)
		dh-=2.0*PI;
	else if(dh<(-PI))
		dh+=2.0*PI;
	double dH=2.0*sqrt(C1*C2)*sin(dh/2.0);
	double LL,CC;
	LL=(L1+L2)/2.0;
	CC=(C1+C2)/2.0;
	double hh,sillyfun,silly;
	sillyfun=h2-h1;
	silly=h1+h2;
	hh=h1+h2;
	if(C1==0.0&&C2==0.0)
		hh=h1+h2;
	else if(sillyfun<=PI)
		hh=(h1+h2)/2.0;
	else if(sillyfun>PI&&silly<(2.0*PI))
		hh=(h1+h2+(2.0*PI))/2.0;
	else if(sillyfun>PI&&silly>=(2.0*PI))
		hh=(h1+h2-(2.0*PI))/2.0;
	double T=1.0-(0.17*cos(hh-(30.0/180.0*PI)))+(0.24*cos(2.0*hh))+(0.32*cos((3.0*hh)+(6.0/180.0*PI)))-(0.20*cos((4.0*hh)-(63.0/180.0*PI)));
	double dtheta=(30.0/180.0*PI)*exp(-1.0*square((180.0/PI*hh-275.0)/25.0));
	temp=pow(CC,7.0);
	double Rc=2.0*sqrt(temp/(temp+6103515625.0));
	temp=square(LL-50.0);
	double Sl=1.0+((0.015*temp)/sqrt(20.0+temp));
	double Sc=1.0+(0.045*CC);
	double Sh=1.0+(0.015*CC*T);
	double Rt=-1.0*sin(2.0*dtheta)*Rc;
	return sqrt(square(dL/(Kl*Sl))+square(dC/(Kc*Sc))+square(dH/(Kh*Sh))+(Rt*(dC/(Kc*Sc))*(dH/(Kh*Sh))));
}
/*void Rgb2Lch(double *L, double *C, double *H, uint8_t RI, uint8_t GI, uint8_t BI){
	double X,Y,Z;
	Rgb2Xyz(&X, &Y, &Z, RI, GI, BI);
	Xyz2Lch(L, C, H, X, Y, Z);
}*/
void Rgb2Lab(uint8_t ri,uint8_t gi,uint8_t bi,double * L,double * a,double * b){
	//conversion code from http://www.getreuer.info/home/colorspace
	double X, Y, Z;
	Rgb2Xyz(&X,&Y,&Z,ri,gi,bi);
	Xyz2Lab(L, a, b, X, Y, Z);
}
double ciede2000rgb(uint8_t R1,uint8_t G1,uint8_t B1,uint8_t R2,uint8_t G2,uint8_t B2){
	double L1,a1,b1,L2,a2,b2;
	Rgb2Lab(R1,G1,B1,&L1,&a1,&b1);
	Rgb2Lab(R2,G2,B2,&L2,&a2,&b2);
	return ciede2000(L1,a1,b1,L2,a2,b2,1.0,1.0,1.0);

}
