/*
 This file is part of Retro Graphics Toolkit

    Retro Graphics Toolkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or any later version.

    Retro Graphics Toolkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#include "global.h"
#include "palette.h"
uint8_t nespaltab_r[64];
uint8_t nespaltab_g[64];
uint8_t nespaltab_b[64];
#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786783165271201909145648
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
uint8_t to_nes_color_rgb(uint8_t red,uint8_t green,uint8_t blue){
	//this function does not set any values to global palette it is done in other functions
	int min_error =(255*255) +(255*255) +(255*255) +1;
	uint8_t bestcolor=0;
	for (uint8_t a=0;a<16;a++){
		for (uint8_t c=0;c<4;c++){
			uint8_t temp=a|(c<<4);
			int rdiff= (int)nespaltab_r[temp] - (int)red;
			int gdiff= (int)nespaltab_g[temp] - (int)green;
			int bdiff= (int)nespaltab_b[temp] - (int)blue;
			int dist = (rdiff*rdiff) + (gdiff*gdiff) + (bdiff*bdiff);
			if (dist <= min_error){
				min_error = dist;
				bestcolor=a|(c<<4);
			}
		}
	}
	return bestcolor;
}
uint8_t to_nes_color(uint8_t pal_index){
	//this function does not set any values to global palette it is done in other functions
	pal_index*=3;
	int min_error =(255*255) +(255*255) +(255*255) +1;
	uint8_t bestcolor=0;
	for (uint8_t a=0;a<16;++a){
		for (uint8_t c=0;c<4;++c){
			uint8_t temp=a|(c<<4);
			int rdiff= (int)nespaltab_r[temp] - (int)currentProject->rgbPal[pal_index];
			int gdiff= (int)nespaltab_g[temp] - (int)currentProject->rgbPal[pal_index+1];
			int bdiff= (int)nespaltab_b[temp] - (int)currentProject->rgbPal[pal_index+2];
			int dist = (rdiff*rdiff) + (gdiff*gdiff) + (bdiff*bdiff);
			if (dist <= min_error){
				min_error = dist;
				bestcolor=a|(c<<4);
			}
		}
	}
	return bestcolor;
}
uint8_t toNesChan(uint8_t ri,uint8_t gi,uint8_t bi,uint8_t chan){
	int min_error =(255*255) +(255*255) +(255*255) +1;
	uint8_t bestcolor=0;
	for (uint8_t a=0;a<16;++a){
		for (uint8_t c=0;c<4;++c){
			uint8_t temp=a|(c<<4);
			int rdiff=(int)nespaltab_r[temp]-(int)ri;
			int gdiff=(int)nespaltab_g[temp]-(int)gi;
			int bdiff=(int)nespaltab_b[temp]-(int)bi;
			int dist = (rdiff*rdiff) + (gdiff*gdiff) + (bdiff*bdiff);
			if (dist <= min_error){
				min_error = dist;
				bestcolor=a|(c<<4);
			}
		}
	}
	uint32_t rgb_out=MakeRGBcolor(bestcolor);
	uint8_t b=rgb_out&255;
	uint8_t g=(rgb_out>>8)&255;
	uint8_t r=(rgb_out>>16)&255;
	switch (chan){
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
uint32_t toNesRgb(uint8_t ri,uint8_t gi,uint8_t bi){
	uint8_t bestcolor=0;
	if(nearestAlg){
		uint32_t min_error=(255*255)+(255*255)+(255*255)+1;
		for (uint8_t a=0;a<16;a++){//hue
			for (uint8_t c=0;c<4;c++){//value
				uint8_t temp=a|(c<<4);
				uint32_t dist = square(nespaltab_r[temp]-ri)+square(nespaltab_g[temp]-gi)+square(nespaltab_b[temp]-bi);
				if (dist < min_error){
					min_error = dist;
					bestcolor=a|(c<<4);
				}
			}
		}
	}else{
		double min_error=9001.0;
		for (uint8_t a=0;a<16;a++){//hue
			for (uint8_t c=0;c<4;c++){//value
				uint8_t temp=a|(c<<4);
				double dist = ciede2000rgb(nespaltab_r[temp],nespaltab_g[temp],nespaltab_b[temp],ri,gi,bi);
				if (dist < min_error){
					min_error = dist;
					bestcolor=a|(c<<4);
				}
			}
		}
	}
	return MakeRGBcolor(bestcolor);
}
uint16_t to_sega_genesis_color(uint16_t pal_index){
	//note this function only set the new rgb colors not the outputed sega genesis palette format
	pal_index*=3;
	uint8_t r,g,b;
	r=nearest_color_index(currentProject->rgbPal[pal_index]);
	g=nearest_color_index(currentProject->rgbPal[pal_index+1]);
	b=nearest_color_index(currentProject->rgbPal[pal_index+2]);
	currentProject->rgbPal[pal_index]=palTab[r];
	currentProject->rgbPal[pal_index+1]=palTab[g];
	currentProject->rgbPal[pal_index+2]=palTab[b];
	//bgr format
	return ((r-palTypeGen)<<1)|((g-palTypeGen)<<5)|((b-palTypeGen)<<9);
}
uint32_t count_colors(uint8_t * image_ptr,uint32_t w,uint32_t h,uint8_t *colors_found,bool useAlpha=false){
	/*!
	Scans for colors in an image stops at over 256 as if there is an excess of 256 colors there is no reason to countinue
	*/
	//memset(colors_found,0,w*h*3);
	uint32_t colors_amount=3;
	colors_found[0]=*image_ptr++;
	colors_found[1]=*image_ptr++;
	colors_found[2]=*image_ptr++;
	if (useAlpha)
		image_ptr++;
	uint8_t start=1;
	uint32_t y;
	for (y=0;y<h;++y){
		for (uint32_t x=start;x<w;++x){
			start=0;
			uint8_t r,g,b;
			r=*image_ptr++;
			g=*image_ptr++;
			b=*image_ptr++;
			if (useAlpha)
				image_ptr++;
			bool new_col=true;
			for (uint32_t c=0;c<colors_amount;c+=3){
				if (r == colors_found[c] && g == colors_found[c+1] && b == colors_found[c+2]){
					new_col=false;
					break;//exit loop
				}
			}
			if (new_col==true){
				colors_found[colors_amount]=r;
				colors_found[colors_amount+1]=g;
				colors_found[colors_amount+2]=b;
				colors_amount+=3;
			}
			if (colors_amount >= 765){
				printf("\nOver 255 colors timing out no need for operation to countinue.\n");
				return colors_amount/3;
			}
		}
			//update progress
			printf("counting colors %% %f Colors Found: %d\r",((float)y/(float)h)*100.0,colors_amount/3);
	}
	printf("\n");
	return colors_amount/3;
}
void updateNesTab(uint8_t emps){
	uint8_t c,v;
	uint32_t rgb_out;
	for(c=0;c<16;++c){
		for(v=0;v<4;++v){
			uint8_t temp=c|(v<<4);
			rgb_out=MakeRGBcolor(temp|emps);
			nespaltab_r[temp]=(rgb_out>>16)&255;//red
			nespaltab_g[temp]=(rgb_out>>8)&255;//green
			nespaltab_b[temp]=rgb_out&255;//blue
		}
	}
}
void update_emphesis(Fl_Widget*,void*){
	uint8_t emps;
	switch (mode_editor){
		case pal_edit:
			emps=palEdit.pal_b->value();
		break;
		case tile_edit:
			emps=tileEdit_pal.pal_b->value();
		break;
		case tile_place:
			emps=tileMap_pal.pal_b->value();
		break;
	}
	/*76543210
	  ||||||||
	  ||||++++- Hue (phase)
	  ||++----- Value (voltage)
	  ++------- Unimplemented, reads back as 0*/
	emps<<=6;
	uint8_t c,v;
	uint32_t rgb_out;
	updateNesTab(emps);
	for (c=0;c<48;c+=3){
		rgb_out=MakeRGBcolor(currentProject->palDat[c/3]|emps);
		currentProject->rgbPal[c]=(rgb_out>>16)&255;//red
		currentProject->rgbPal[c+1]=(rgb_out>>8)&255;//green
		currentProject->rgbPal[c+2]=rgb_out&255;//blue
	}
	window->redraw();
}
