/*	quantizer.h

	See quantizer.c for more information
*/
#pragma once


int dl1quant(unsigned char *inbuf, int width, int height,
			int quant_to, unsigned char userpal[3][256]);

int dl3quant(unsigned char *inbuf, int width, int height,
			int quant_to, unsigned char userpal[3][256]);

int dl3floste(unsigned char *inbuf, unsigned char *outbuf, int width, int height,
			int quant_to, int dither, unsigned char userpal[3][256]);
