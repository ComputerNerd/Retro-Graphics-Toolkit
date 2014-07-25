#pragma once
#include <stdint.h>
uint32_t ColourDistance(int r1,int g1,int b1, int r2,int g2,int b2);
void Rgb2Lch(double *L, double *C, double *H, uint8_t RI, uint8_t GI, uint8_t BI);
void Rgb2Xyz(double *X, double *Y, double *Z, uint8_t RI, uint8_t GI, uint8_t BI);
void Xyz2Lab(double *L, double *a, double *b, double X, double Y, double Z);
void Lab2Lch(double *C, double *H,double a, double b);
void Rgb2Lab(uint8_t ri,uint8_t gi,uint8_t bi,double * L,double * a,double * b);
double ciede2000(double L1,double C1,double h1,double L2,double C2,double h2,double Kl,double Kc,double Kh);
double ciede2000rgb(uint8_t r1,uint8_t g1,uint8_t b1,uint8_t r2,uint8_t g2,uint8_t b2);
