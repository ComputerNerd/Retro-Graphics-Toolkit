#include <stdint.h>
#include <cmath>
#include <algorithm>
#include <FL/fl_ask.H>
#include "nespal.h"
static float convCoords[8];
static float convMatrix[9];
struct rgbo{
	float r,g,b;
	bool ovr;
};
static struct rgbo absColorimetric(float r,float g,float b){
	float rgb[3] = {r, g, b};
	float xyz[3];
	std::fill(xyz,xyz+3,0);
	const float xyzmat[] = {.4124f,.3576f,.1805f,.2126f,.7152f,.0722f,.0193f,.1192f,.9505f};
	float WY = 1.f;
	float WX = (WY * 0.3127f)/0.3290f;
	float WZ = (WY * (1.f - 0.3127f - 0.3290f))/0.3290f;
	// Grabbed from the Wikipedia page on sRGB and xyY -> XYZ
	const float xyzprims[] = {(.2126*.64)/.33, .2126, (.2126*(1-.64-.33))/.33, (.7153*.3)/.6, .7153, (.7153*(1-.3-.6))/.6, (.0721*.15)/.06, .0721, (.0721*(1-.15-.06))/.06};
	// First, convert RGB to CIEXYZ.
	for (unsigned i = 0; i < 3; i++) {
		if (rgb[i] <= 0.04045f) {
			rgb[i] /= 12.92f;
		} else {
			rgb[i] = powf((rgb[i] + 0.055f)/(1.f+0.055f),2.4f);
		}
	}
	for (unsigned i=0,j=0;i<3;i++){
		xyz[i] = xyzmat[j++] * rgb[0];
		xyz[i] += xyzmat[j++] * rgb[1];
		xyz[i] += xyzmat[j++] * rgb[2];
	}
	// Convert XYZ to LUV.
	float luv[3];
	std::fill(luv,luv+3,0.f);
	float luvprims[9];
	std::fill(luvprims,luv+9,0.f);
	if ((xyz[1] / WY) <= powf(6.f/29.f, 3.f))
		luv[0] = powf(29.f/3.f,3.f) * (xyz[1] / WY);
	else
		luv[0] = (116.f * powf(xyz[1] / WY,1.f/3.f))-16.f;
	for (unsigned i = 0; i < 3; i++) {
		if ((xyzprims[(i*3)+1] / WY) <= powf(6.f/29.f, 3.f))
			luvprims[i*3] = powf(29.f/3.f,3.f) * (xyzprims[(i*3)+1] / WY);
		else
			luvprims[i*3] = (116.f * powf(xyzprims[(i*3)+1] / WY,1.f/3.f))-16.f;
	}
	float upn = (4.f*WX)/(WX + (15.f*WY) + (3.f*WZ));
	float vpn = (9.f*WY)/(WX + (15.f*WY) + (3.f*WZ));
	float up = (4.f*xyz[0])/(xyz[0] + (15*xyz[1]) + (3*xyz[2]));
	float vp = (9.f*xyz[1])/(xyz[0] + (15*xyz[1]) + (3*xyz[2]));
	luv[1] = (13.f*luv[0]) * (up - upn);
	luv[2] = (13.f*luv[0]) * (vp - vpn);
	for (unsigned i = 0; i < 3; i++) {
		up = (4*xyzprims[(i*3)])/(xyzprims[(i*3)] + (15*xyzprims[(i*3)+1]) + (3*xyzprims[(i*3)+2]));
		vp = (9*xyzprims[(i*3)+1])/(xyzprims[(i*3)] + (15*xyzprims[(i*3)+1]) + (3*xyzprims[(i*3)+2]));
		luvprims[(i*3)+1] = (13*luvprims[(i*3)]) * (up - upn);
		luvprims[(i*3)+2] = (13*luvprims[(i*3)]) * (vp - vpn);
	}
	// We've now gone from sRGB -> XYZ -> LUV.
	if (((r >= 0.f) && (r <= 1.f)) &&
		((g >= 0.f) && (g <= 1.f)) &&
		((b >= 0.f) && (b <= 1.f))) {
		return {r,g,b,false};
	}
	return {r,g,b,true};
}
static struct rgbo simpleClip(float r,float g,float b) {
	bool ovr = false;
	if (r > 1.f) {
		r = 1.f;
		ovr = true;
	} else if (r < 0.f) {
		r = 0.f;
	}
	if (g > 1.f) {
		g = 1.f;
		ovr = true;
	} else if (g < 0.f) {
		g = 0.f;
	}
	if (b > 1.f) {
		b = 1.f;
		ovr = true;
	} else if (b < 0.f) {
		b = 0.f;
	}
	return {r,g,b,ovr};
}
static struct rgbo huePreserveClip(float r,float g,float b) {
	bool ovr = false;
	float ratio = 1.f;
	if ((r > 1.f) || (g > 1.f) || (b > 1.f)) {
		ovr = true;
		float max = r;
		if (g > max)
			max = g;
		if (b > max)
			max = b;
		ratio = 1.f / max;
	}
	r *= ratio;
	g *= ratio;
	b *= ratio;
	if (r > 1.f) r = 1.f;
	else if (r < 0.f) r = 0.f;
	if (g > 1.f) g = 1.f;
	else if (g < 0.f) g = 0.f;
	if (b > 1.f) b = 1.f;
	else if (b < 0.f) b = 0.f;
	return {r: r, g: g, b: b, ovr: ovr};
}
static struct rgbo huePreserveClip2(float r,float g,float b) {
	bool ovr = false;
	float ratio = 1.f;
	float min,max;
	if ((r > 1.f) || (g > 1.f) || (b > 1.f)) {
		ovr = true;
		max = r;
		if (g > max) max = g;
		if (b > max) max = b;
		min = r;
		if (g < min) min = g;
		if (b < min) min = b;
		ratio = 1.f / max;
	}
	if (ovr) {
		r -= min;
		g -= min;
		b -= min;
		r *= ratio;
		g *= ratio;
		b *= ratio;
		r += min;
		g += min;
		b += min;
	}
	if (r > 1.f) r = 1.f;
	else if (r < 0.f) r = 0.f;
	if (g > 1.f) g = 1.f;
	else if (g < 0.f) g = 0.f;
	if (b > 1.f) b = 1.f;
	else if (b < 0.f) b = 0.f;
	return {r,g,b,ovr};
}
static struct rgbo huePreserveClip3(float r,float g,float b) {
	float l = (.299f* r) + (0.587f * g) + (0.114f* b);
	bool ovr = false;
	float ratio = 1.f;
	if ((r > 1.f) || (g > 1.f) || (b > 1.f)) {
		ovr = true;
		float max = r;
		if (g > max) max = g;
		if (b > max) max = b;
		ratio = 1.f / max;
	}
	if (ovr) {
		r -= l;
		g -= l;
		b -= l;
		r *= ratio;
		g *= ratio;
		b *= ratio;
		r += l;
		g += l;
		b += l;
	}
	if (r > 1.f) r = 1.f;
	else if (r < 0.f) r = 0.f;
	if (g > 1.f) g = 1.f;
	else if (g < 0.f) g = 0.f;
	if (b > 1.f) b = 1.f;
	else if (b < 0.f) b = 0.f;
	return {r,g,b,ovr};
}
static struct rgbo hueLumPreserveClip(float r,float g,float b,float l){
	bool ovr = false;
	float ratio = 1;
	if ((r > 1.f) || (g > 1.f) || (b > 1.f)) {
		ovr = true;
		float max = r;
		if (g > max) max = g;
		if (b > max) max = b;
		ratio = 1 / max;
	}
	if (ovr) {
		r -= l;
		g -= l;
		b -= l;
		r *= ratio;
		g *= ratio;
		b *= ratio;
		r += l;
		g += l;
		b += l;
	}
	if (r > 1.f) r = 1.f;
	else if (r < 0.f) r = 0.f;
	if (g > 1.f) g = 1.f;
	else if (g < 0.f) g = 0.f;
	if (b > 1.f) b = 1.f;
	else if (b < 0.f) b = 0.f;
	return {r,g,b,ovr};
}
enum clipYIQtoRGB{NONE_Y,CLAMP_Y,DARKEN_Y,DESATURATE_Y};
enum clipStyle{CLAMP_S,DARKEN_S,DESATURATE_S};
static struct rgbo yiqToRgb(float Y,float I,float Q){
	float gamma = 1.f;//TODO gui
	enum clipYIQtoRGB clipMethod=CLAMP_Y;//TODO gui
	enum clipStyle clipMethodB=CLAMP_S;//TODO gui
	
	// This is the YIQ -> RGB formula as defined by the FCC. The calculations for this are in the comments
	// at the end of this script, just take the result matrix and invert it and you should get this.
	float R = Y + (0.9469f*I) + (0.6236f*Q);
	float G = Y - (0.2748f*I) - (0.6357f*Q);
	float B = Y - (1.1085f*I) + (1.709f*Q);

	struct rgbo corrected;
	// Apply desired clipping method to out-of-gamut colors.
	switch (clipMethodB) {
		case DARKEN_S:
			corrected = huePreserveClip(R, G, B);
		break;
		case DESATURATE_S:
			corrected = huePreserveClip3(R, G, B);
		break;
		case CLAMP_S:
		default:
			corrected = simpleClip(R, G, B);
		break;
	}
	if (clipMethodB != DESATURATE_S) {
		R = corrected.r;
		G = corrected.g;
		B = corrected.b;
	}
	
	// This is the conversion matrix for CIEXYZ -> sRGB. I nicked this from:
	// http://www.brucelindbloom.com/Eqn_RGB_XYZ_Matrix.html
	// and I know it's right because when you use the sRGB colorimetry, this matrix produces identical results to
	// just using the raw R, G, and B above.
	const float xyztorgb[] = {3.2404, -1.5371, -0.4985, -0.9693, 1.876, 0.0416, 0.0556, -0.204, 1.0572};

	// Remove the disabled channels.
	// If channels are negative, clamp them to 0. I'm pretty sure this is what TVs do.
	if (R < 0.f) R = 0.f;
	if (G < 0.f) G = 0.f;
	if (B < 0.f) B = 0.f;
		
	// Gamma correction.
	R = powf(R, gamma);
	G = powf(G, gamma);
	B = powf(B, gamma);
	
	// Convert RGB to XYZ using the matrix generated with the specified RGB and W points.	
	float X = (convMatrix[0] * R) + (convMatrix[1] * G) + (convMatrix[2] * B);
	Y = (convMatrix[3] * R) + (convMatrix[4] * G) + (convMatrix[5] * B);
	float Z = (convMatrix[6] * R) + (convMatrix[7] * G) + (convMatrix[8] * B);

	// Convert back to RGB using the XYZ->sRGB matrix.
	R = (xyztorgb[0]*X) + (xyztorgb[1]*Y) + (xyztorgb[2]*Z);
	G = (xyztorgb[3]*X) + (xyztorgb[4]*Y) + (xyztorgb[5]*Z);
	B = (xyztorgb[6]*X) + (xyztorgb[7]*Y) + (xyztorgb[8]*Z);
	
	// Any negative channels are clamped to 0 again.
	if (R < 0) R = 0;
	if (G < 0) G = 0;
	if (B < 0) B = 0;

	
	// Apply desired clipping method to out-of-gamut colors.
	switch (clipMethod) {
		case DARKEN_Y:
			corrected = huePreserveClip(R, G, B);
		break;
		case DESATURATE_Y:
			corrected = huePreserveClip2(R, G, B);
		break;
		case NONE_Y:
			corrected = huePreserveClip3(R, G, B);
		break;
		/*case "4":
			corrected = hueLumPreserveClip(R, G, B, Y);
		break;
		case "5":
			corrected = absColorimetric(R, G, B);
		break;*/
		case CLAMP_Y:
		default:
			corrected = simpleClip(R, G, B);
	}
	// Convert normalized value to the two-character hexadecimal representation.
	return {R,G,B,corrected.ovr};
}
static void updateMatrix(float Rpx,float Rpy,float Gpx,float Gpy,float Bpx,float Bpy,float Wpx,float Wpy){
	convCoords[0]=Rpx;
	convCoords[1]=Rpy;
	convCoords[2]=Gpx;
	convCoords[3]=Gpy;
	convCoords[4]=Bpx;
	convCoords[5]=Bpy;
	convCoords[6]=Wpx;
	convCoords[7]=Wpy;
	//http://www.brucelindbloom.com/Eqn_RGB_XYZ_Matrix.html
	//http://www.dr-lex.be/random/matrix_inv.html
	// Convert the (x,y) values to X Y Z.
	float Xr = Rpx / Rpy;
	float Xg = Gpx / Gpy;
	float Xb = Bpx / Bpy;
	float Xw = Wpx / Wpy;
	float Yr = 1.f;
	float Yg = 1.f;
	float Yb = 1.f;
	float Yw = 1.f;
	float Zr = (1.f - Rpx - Rpy) / Rpy;
	float Zg = (1.f - Gpx - Gpy) / Gpy;
	float Zb = (1.f - Bpx - Bpy) / Bpy;
	float Zw = (1.f - Wpx - Wpy) / Wpy;

	// Get ready for a bunch of painful math. I need to invert a matrix, then multiply it by a vector.
	// Determinant for inverse matrix
	float sDet = (Xr*((Zb*Yg)-(Zg*Yb)))-(Yr*((Zb*Xg)-(Zg*Xb)))+(Zr*((Yb*Xg)-(Yg*Xb)));
	
	float Sr = ((((Zb*Yg)-(Zg*Yb))/sDet)*Xw) + ((-((Zb*Xg)-(Zg*Xb))/sDet)*Yw) + ((((Yb*Xg)-(Yg*Xb))/sDet)*Zw);
	float Sg = ((-((Zb*Yr)-(Zr*Yb))/sDet)*Xw) + ((((Zb*Xr)-(Zr*Xb))/sDet)*Yw) + ((-((Yb*Xr)-(Yr*Xb))/sDet)*Zw);
	float Sb = ((((Zg*Yr)-(Zr*Yg))/sDet)*Xw) + ((-((Zg*Xr)-(Zr*Xg))/sDet)*Yw) + ((((Yg*Xr)-(Yr*Xg))/sDet)*Zw);
	
	// This should be the completed RGB -> XYZ matrix.
	// Multiply each of the first three members by R, then add them together to get X
	convMatrix[0] = Sr*Xr;
	convMatrix[1] = Sg*Xg;
	convMatrix[2] = Sb*Xb;
	convMatrix[3] = Sr*Yr;
	convMatrix[4] = Sg*Yg;
	convMatrix[5] = Sb*Yb;
	convMatrix[6] = Sr*Zr;
	convMatrix[7] = Sg*Zg;
	convMatrix[8] = Sb*Zb;
}
static float clamp255f(float x){
	return std::min(255.f,std::max(x,0.f));;
}
enum colorimetryTypes{FCC_1953,FCC_D65,SMPTE_C_1987,SRGB_PC_Monitors,CUSTOM_COLORIMETRY};
static inline int wave(int p,int color){
	return (color+p+8)%12 < 6;
}
uint32_t nesPalToRgb(unsigned inputPal) {
	enum colorimetryTypes colorimetry=FCC_D65;//TODO gui to set this
	switch (colorimetry) {
		case FCC_1953:
		default:
			updateMatrix(.67, .33, .21, .71, .14, .08, .31, .316);
		break;
		case FCC_D65:
			updateMatrix(.67, .33, .21, .71, .14, .08, .3127, .329);
		break;
		case SMPTE_C_1987:
			updateMatrix(.63, .34, .31, .595, .155, .07, .3127, .329);
		break;
		case SRGB_PC_Monitors:
			updateMatrix(.64, .33, .3, .6, .15, .06, .3127, .329);
		break;
		case CUSTOM_COLORIMETRY:
			/*updateMatrix(
				document.paletteTweaks.custRx.value,
				document.paletteTweaks.custRy.value,
				document.paletteTweaks.custGx.value,
				document.paletteTweaks.custGy.value,
				document.paletteTweaks.custBx.value,
				document.paletteTweaks.custBy.value,
				document.paletteTweaks.custWx.value,
				document.paletteTweaks.custWy.value);*/
			fl_alert("TODO");
		break;
	}

	float hueAdj = -0.25f;//TODO gui
	float hue_tweak=hueAdj/M_PI*12.f/360.f;
	float satAdj = 1.2f;//TODO gui
	float bri = 1.f;//TODO gui
	float con = 1.f;//TODO gui
	float irange = 0.599f;//TODO gui
	float qrange = 0.525f;//TODO gui
	/*76543210
	  ||||||||
	  ||||++++- Hue (phase)
	  ||++----- Value (voltage)
	  ++------- Unimplemented, reads back as 0*/
	unsigned emp=(inputPal>>6)&3;
	unsigned lum=(inputPal>>4)&3;
	unsigned hue=inputPal&15;
	unsigned basecol=inputPal&63;
	int level = hue<0xE ? (inputPal>>4) & 3 : 1;
	static const float levels[8] = {.350f, .518f, .962f,1.550f,  // Signal low
		1.094f,1.506f,1.962f,1.962f}; // Signal high
	float lo_and_hi[2] = { levels[level + 4 * (hue == 0x0)],
		levels[level + 4 * (hue <  0xD)] };
	struct rgbo color;
	/*if(hue>=14){
		hue=14;
		lum=1;
	}
	if(hue&&hue<13){
		--hue;
		float Y = (low + high) / 2;
		float sat = luminances[lum] - luminances[lum + 4];
		sat *= satAdj * con;
		//Colorburst amplitude = -0.208 ~ 0.286 = 0.494
		//Colorburst bias = 0.039
		// Color 8 is used as colorburst. Colorburst is 2.5656 radians.
		float I = sinf((((hue - 7.f) / 12.f) * 6.2832f)+2.5656f+hueAdj) * irange;
		float Q = cosf((((hue - 7.f) / 12.f) * 6.2832f)+2.5656f+hueAdj) * qrange;
		I *= sat;
		Q *= sat;
		color = yiqToRgb(Y,I,Q);
	}else if(hue)
		color = yiqToRgb(low, 0, 0);
	else
		color = yiqToRgb(high, 0, 0);*/
	
	static const float black=.518f, white=1.962f, attenuation=.746f;
	float y=0.f, i=0.f, q=0.f;
	//auto wave = [](int p, int color) { return (color+p+8)%12 < 6; };
	for(int p=0; p<12; ++p) // 12 clock cycles per pixel.
	{
		// NES NTSC modulator (square wave between two voltage levels):
		float spot = lo_and_hi[wave(p,hue)];

		// De-emphasis bits attenuate a part of the signal:
		if(((inputPal & 0x40) && wave(p,12))
				|| ((inputPal & 0x80) && wave(p, 4))
				|| ((inputPal &0x100) && wave(p, 8))) spot *= attenuation;

		// Normalize:
		float v = (spot - black) / (white-black);
		//float v = (spot - black) / (white-black) / 12.f;
		// Ideal TV NTSC demodulator:
		// Apply contrast/brightness
		v = (v - .5f) * con + .5f;
		v *= bri / 12.f;

		y += v;
		i += v * std::cos( (M_PI/6.f) * (p+hue_tweak) );
		q += v * std::sin( (M_PI/6.f) * (p+hue_tweak) );
	}
	i *= satAdj;
	q *= satAdj;
	color = yiqToRgb(y, i, q);
	return lroundf(clamp255f(color.b*256.f))|(lroundf(clamp255f(color.g*256.f))<<8)|(lroundf(clamp255f(color.r*256.f))<<16);
}
