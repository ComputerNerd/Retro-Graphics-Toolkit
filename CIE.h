#pragma once
#include <stdint.h>
void Rgb2Lab(double *L, double *a, double *b, double R, double G, double B);
void Rgb2Lab255(double *L, double *a, double *b, unsigned r, unsigned g, unsigned bl);
void Lab2Rgb(double *R, double *G, double *B, double L, double a, double b);
void Lab2Rgb255(uint8_t*r, uint8_t *g, uint8_t *blue, double L, double a, double b);
void Rgb2Lch(double *L, double *C, double *H, double R, double G, double B);
void Rgb2Lch255(double *L, double *C, double *H, unsigned R, unsigned G, unsigned B);
void Lch2Rgb(double *R, double *G, double *B, double L, double C, double H);
void Lch2Rgb255(uint8_t*r, uint8_t *g, uint8_t *b, double L, double C, double H);
