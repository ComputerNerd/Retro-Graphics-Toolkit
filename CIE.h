#pragma once
#include <stdint.h>
void Rgb2Lch(double *L, double *C, double *H, uint8_t RI, uint8_t GI, uint8_t BI);
void Rgb2Xyz(double *X, double *Y, double *Z, uint8_t RI, uint8_t GI, uint8_t BI);
void Xyz2Lab(double *L, double *a, double *b, double X, double Y, double Z);
void Lab2Lch(double *C, double *H,double a, double b);
void Rgb2Lab(uint8_t ri,uint8_t gi,uint8_t bi,double * L,double * a,double * b);
