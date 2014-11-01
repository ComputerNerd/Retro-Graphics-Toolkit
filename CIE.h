#pragma once
#include <stdint.h>
void Rgb2Lab(double R,double G,double B,double * L,double * a,double * b);
void Rgb2Lab255(uint8_t ri,uint8_t gi,uint8_t bi,double * L,double * a,double * b);
