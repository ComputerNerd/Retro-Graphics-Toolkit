#include "global.h"
#include <cmath>
#include "color_compare.h"
#include "CIE.h"
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
double ciede2000rgb(uint8_t R1,uint8_t G1,uint8_t B1,uint8_t R2,uint8_t G2,uint8_t B2){
	double L1,a1,b1,L2,a2,b2;
	Rgb2Lab255(R1,G1,B1,&L1,&a1,&b1);
	Rgb2Lab255(R2,G2,B2,&L2,&a2,&b2);
	return ciede2000(L1,a1,b1,L2,a2,b2,1.0,1.0,1.0);

}
