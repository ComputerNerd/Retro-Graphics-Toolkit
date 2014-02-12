#include "global.h"
//From http://www.compuphase.com/cmetric.htm
double ColourDistance(int r1,int g1,int b1, int r2,int g2,int b2){
	long rmean = ( (long)r1 + (long)r2 ) / 2;
	long r = (long)r1 - (long)r2;
	long g = (long)g1 - (long)g2;
	long b = (long)b1 - (long)b2;
	return sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
}
static inline double square(double x){
	return x*x;
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
void rgbtociflab(uint8_t ri,uint8_t gi,uint8_t bi,double * L,double * a,double * b){
/*		//conversion code from http://www.easyrgb.com/index.php?X=MATH
	double var_R = R/255.0;        //R from 0 to 255
	double var_G = G/255.0;        //G from 0 to 255
	double var_B = B/255.0;        //B from 0 to 255
	if(var_R>0.04045)
		var_R = pow(((var_R+0.055)/1.055),2.4);
	else
		var_R/=12.92;
	if(var_G>0.04045)
		var_G = pow(((var_G+0.055)/1.055),2.4);
	else
		var_G/=12.92;
	if(var_B>0.04045)
		var_B = pow(((var_B+0.055)/1.055),2.4);
	else
		var_B/=12.92;
	var_R *=100.0;
	var_G *=100.0;
	var_B *=100.0;
//Observer. = 2°, Illuminant = D65
	double X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
	double Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
	double Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;
	double var_X=X/95.047;          //ref_X =  95.047   Observer= 2°, Illuminant= D65
	double var_Y=Y/100.0;          //ref_Y = 100.000
	double var_Z=Z/108.883;          //ref_Z = 108.883
	if(var_X >0.008856)
		var_X = pow(var_X,(1.0/3.0));
	else
		var_X = (7.787*var_X)+(16.0/116.0);
	if (var_Y>0.008856)
		var_Y = pow(var_Y,(1.0/3.0));
	else
		var_Y = (7.787*var_Y)+(16.0/116.0);
	if (var_Z>0.008856)
		var_Z=pow(var_Z,(1.0/3.0));
	else
		var_Z=(7.787*var_Z)+(16.0/116.0);
	*L=(116.0* var_Y)-16.0;
	*a=500.0*(var_X-var_Y);
	*b=200.0*(var_Y-var_Z);*/
	//conversion code from http://www.getreuer.info/home/colorspace
	double R=(double)ri/255.0;
	double G=(double)gi/255.0;
	double B=(double)bi/255.0;
	R = INVGAMMACORRECTION(R);
	G = INVGAMMACORRECTION(G);
	B = INVGAMMACORRECTION(B);
	double X = (double)(0.4123955889674142161*R + 0.3575834307637148171*G + 0.1804926473817015735*B);
	double Y = (double)(0.2125862307855955516*R + 0.7151703037034108499*G + 0.07220049864333622685*B);
	double Z = (double)(0.01929721549174694484*R + 0.1191838645808485318*G + 0.9504971251315797660*B);
	X /= WHITEPOINT_X;
	Y /= WHITEPOINT_Y;
	Z /= WHITEPOINT_Z;
	X = LABF(X);
	Y = LABF(Y);
	Z = LABF(Z);
	*L = 116*Y - 16;
	*a = 500*(X - Y);
	*b = 200*(Y - Z);
}
double ciede2000rgb(uint8_t R1,uint8_t G1,uint8_t B1,uint8_t R2,uint8_t G2,uint8_t B2){
	double L1,a1,b1,L2,a2,b2;
	rgbtociflab(R1,G1,B1,&L1,&a1,&b1);
	rgbtociflab(R2,G2,B2,&L2,&a2,&b2);
	return ciede2000(L1,a1,b1,L2,a2,b2,1.0,1.0,1.0);

}
