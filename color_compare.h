#pragma once
#include <stdint.h>
enum nearestAlgs_t{aCiede2000,aWeighted,aEuclid,aCIE76};
uint32_t ColourDistance(int r1,int g1,int b1, int r2,int g2,int b2);
double ciede2000(double L1,double C1,double h1,double L2,double C2,double h2,double Kl,double Kc,double Kh);
double ciede2000rgb(uint8_t r1,uint8_t g1,uint8_t b1,uint8_t r2,uint8_t g2,uint8_t b2);
