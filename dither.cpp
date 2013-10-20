#include <inttypes.h>
#include "dither.h"
#include "global.h"
#include "color_convert.h"
#define NONE 0
#define UP 1
#define LEFT 2
#define DOWN 3
#define RIGHT 4
static uint8_t useHiL;//no use for these varibles outside of this file
static uint8_t useMode;
static uint8_t rgbPixelsize;
static bool USEofColGlob;
static bool forcedfun;
static uint8_t theforcedfun;
static uint8_t *img_ptr_dither;
uint8_t nearest_color_chan(uint8_t val,uint8_t chan,uint8_t row){
	//returns closest value
	//palette_muliplier
	uint8_t i;
    int32_t distanceSquared, minDistanceSquared, bestIndex = 0;
    minDistanceSquared = 255*255 + 1;
	uint8_t max_rgb=0;
	switch (useMode){
		case sega_genesis:
			if(USEofColGlob)
				return palTab[nearest_color_index(val)];
			max_rgb=48;//16*3=48
		break;
		case NES:
			if(USEofColGlob){
				img_ptr_dither-=chan;
				uint8_t returnme=toNesChan(*img_ptr_dither,*(img_ptr_dither+1),*(img_ptr_dither+2),chan);
				img_ptr_dither+=chan;
				return returnme;
			}
			max_rgb=12;//4*3=12
		break;
		case 255://alpha
			return (val&128)?255:0;
		break;
	}
	row*=max_rgb;
	for (i=0; i<max_rgb; i+=3){
		int32_t Rdiff = (int) val - (int)currentProject->rgbPal[i+row+chan];
		distanceSquared = Rdiff*Rdiff;
		if (distanceSquared < minDistanceSquared){
			minDistanceSquared = distanceSquared;
			bestIndex = i;
		}
	}
    return currentProject->rgbPal[bestIndex+row+chan];
}
/* variables needed for the Riemersma
 * dither algorithm */ 
static int32_t cur_x=0, cur_y=0;
static int32_t img_width=0, img_height=0; 

static uint8_t rgb_select=0;
    
#define SIZE 16	/* queue size: number of
				 * pixels remembered */ 
#define MAX  16	/* relative weight of
				 * youngest pixel in the
				 * queue, versus the oldest
				 * pixel */
    
static int32_t weights[SIZE];	/* weights for
								 * the errors
								 * of recent
								 * pixels */
    
static void init_weights(int32_t a[],int32_t size,int32_t max){
	double m = exp(log(max)/(size-1));
	double v;
	int32_t i;  
	for (i=0, v=1.0; i<size;++i){
		a[i]=(int)(v+0.5);	/* store rounded
							 * value */ 
		v*=m;				/* next value */
	} /*for */
}
    
static void dither_pixel(uint8_t *pixel) {
static int32_t error[SIZE]; /* queue with error
                         * values of recent
                         * pixels */
	int32_t i,pvalue,err;
    
	for (i=0,err=0L; i<SIZE;++i)
		err+=error[i]*weights[i];
	//pvalue=*pixel + err/MAX;
	if (*pixel+err/MAX > 255)
		pvalue=255;
	else if (*pixel+err/MAX < 0)
		pvalue=0;
	else
		pvalue=*pixel + err/MAX;
  //pvalue = (pvalue>=128) ? 255 : 0;
	if ((game_system == sega_genesis) && (useHiL == 9)){
		bool tempSet=currentProject->tileMapC->get_prio(cur_x/8,cur_y/8)^true;
		set_palette_type(tempSet);
	}
	if(forcedfun)
		pvalue=nearest_color_chan(pvalue,rgb_select,theforcedfun);
	else
		pvalue=nearest_color_chan(pvalue,rgb_select,currentProject->tileMapC->get_palette_map(cur_x/8,cur_y/8));
  /* shift queue */ 
	memmove(error, error+1,(SIZE-1)*sizeof error[0]); 
	error[SIZE-1] = *pixel - pvalue;
	*pixel=(uint8_t)pvalue;
}
    
static void move(int32_t direction){
  /* dither the current pixel */
	if (cur_x >= 0 && cur_x < img_width && cur_y >= 0 && cur_y < img_height)
		dither_pixel(img_ptr_dither);
    
  /* move to the next pixel */
  switch (direction) {
  case LEFT:
    cur_x--;
    img_ptr_dither-=rgbPixelsize;
    break;
  case RIGHT:
    cur_x++;
    img_ptr_dither+=rgbPixelsize;
    break;
  case UP:
    cur_y--;
    img_ptr_dither-=img_width*rgbPixelsize;
    break;
  case DOWN:
    cur_y++;
    img_ptr_dither+=img_width*rgbPixelsize;
    break;
  } /* switch */
}
void hilbert_level(int32_t level,int32_t direction){
  if (level==1) {
    switch (direction) {
    case LEFT:
      move(RIGHT);
      move(DOWN);
      move(LEFT);
      break;
    case RIGHT:
      move(LEFT);
      move(UP);
      move(RIGHT);
      break;
    case UP:
      move(DOWN);
      move(RIGHT);
      move(UP);
      break;
    case DOWN:
      move(UP);
      move(LEFT);
      move(DOWN);
      break;
    } /* switch */
  } else {
    switch (direction) {
    case LEFT:
      hilbert_level(level-1,UP);
      move(RIGHT);
      hilbert_level(level-1,LEFT);
      move(DOWN);
      hilbert_level(level-1,LEFT);
      move(LEFT);
      hilbert_level(level-1,DOWN);
      break;
    case RIGHT:
      hilbert_level(level-1,DOWN);
      move(LEFT);
      hilbert_level(level-1,RIGHT);
      move(UP);
      hilbert_level(level-1,RIGHT);
      move(RIGHT);
      hilbert_level(level-1,UP);
      break;
    case UP:
      hilbert_level(level-1,LEFT);
      move(DOWN);
      hilbert_level(level-1,UP);
      move(RIGHT);
      hilbert_level(level-1,UP);
      move(UP);
      hilbert_level(level-1,RIGHT);
      break;
    case DOWN:
      hilbert_level(level-1,RIGHT);
      move(UP);
      hilbert_level(level-1,DOWN);
      move(LEFT);
      hilbert_level(level-1,DOWN);
      move(DOWN);
      hilbert_level(level-1,LEFT);
      break;
    } /* switch */
  } /* if */
}
static inline int32_t log2int(int32_t value){
	int32_t result=0;
	while (value>1) {
		value >>= 1;
		result++;
	}
	return result;
}
void Riemersma(uint8_t *image, int32_t width,int32_t height,uint8_t rgb_sel){
	int32_t level,size;
	rgb_select=rgb_sel;
	/* determine the required order of the
	 * Hilbert curve */ 
	size=max(width,height);
	level=log2int(size);
	if ((1L << level) < size)
		level++;
	init_weights(weights,SIZE,MAX);
	img_ptr_dither=image;
	img_ptr_dither+=rgb_sel;
	img_width=width;
	img_height=height;
	cur_x=0;
	cur_y=0;
	if (level>0)
		hilbert_level(level,UP);
	move(NONE);
}

#define COMPARE_RGB 1

/* 8x8 threshold map */
static const unsigned char mapY3[8*8] = {
     0,48,12,60, 3,51,15,63,
    32,16,44,28,35,19,47,31,
     8,56, 4,52,11,59, 7,55,
    40,24,36,20,43,27,39,23,
     2,50,14,62, 1,49,13,61,
    34,18,46,30,33,17,45,29,
    10,58, 6,54, 9,57, 5,53,
    42,26,38,22,41,25,37,21 };

static const double Gamma = 2.2; // Gamma correction we use.

double GammaCorrect(double v)   { return pow(v, Gamma);       }
double GammaUncorrect(double v) { return pow(v, 1.0 / Gamma); }

/* CIE C illuminant */
static const double illum[3*3] =
{ 0.488718, 0.176204, 0.000000,
  0.310680, 0.812985, 0.0102048,
  0.200602, 0.0108109, 0.989795 };
struct LabItem // CIE L*a*b* color value with C and h added.
{
    double L,a,b,C,h;

    LabItem() { }
    LabItem(double R,double G,double B) { Set(R,G,B); }
    void Set(double R,double G,double B)
    {
        const double* const i = illum;
        double X = i[0]*R + i[3]*G + i[6]*B, x = X / (i[0] + i[1] + i[2]);
        double Y = i[1]*R + i[4]*G + i[7]*B, y = Y / (i[3] + i[4] + i[5]);
        double Z = i[2]*R + i[5]*G + i[8]*B, z = Z / (i[6] + i[7] + i[8]);
        const double threshold1 = (6*6*6.0)/(29*29*29.0);
        const double threshold2 = (29*29.0)/(6*6*3.0);
        double x1 = (x > threshold1) ? pow(x, 1.0/3.0) : (threshold2*x)+(4/29.0);
        double y1 = (y > threshold1) ? pow(y, 1.0/3.0) : (threshold2*y)+(4/29.0);
        double z1 = (z > threshold1) ? pow(z, 1.0/3.0) : (threshold2*z)+(4/29.0);
        L = (29*4)*y1 - (4*4);
        a = (500*(x1-y1) );
        b = (200*(y1-z1) );
        C = sqrt(a*a + b+b);
        h = atan2(b, a);
    }
    LabItem(unsigned rgb) { Set(rgb); }
    void Set(unsigned rgb)
    {
        Set( (rgb>>16)/255.0, ((rgb>>8)&0xFF)/255.0, (rgb&0xFF)/255.0 );
    }
};

/* From the paper "The CIEDE2000 Color-Difference Formula: Implementation Notes, */
/* Supplementary Test Data, and Mathematical Observations", by */
/* Gaurav Sharma, Wencheng Wu and Edul N. Dalal, */
/* Color Res. Appl., vol. 30, no. 1, pp. 21-30, Feb. 2005. */
/* Return the CIEDE2000 Delta E color difference measure squared, for two Lab values */
static double ColorCompare(const LabItem& lab1, const LabItem& lab2)
{
    #define RAD2DEG(xx) (180.0/M_PI * (xx))
    #define DEG2RAD(xx) (M_PI/180.0 * (xx))
    /* Compute Cromanance and Hue angles */
    double C1,C2, h1,h2;
    {
        double Cab = 0.5 * (lab1.C + lab2.C);
        double Cab7 = pow(Cab,7.0);
        double G = 0.5 * (1.0 - sqrt(Cab7/(Cab7 + 6103515625.0)));
        double a1 = (1.0 + G) * lab1.a;
        double a2 = (1.0 + G) * lab2.a;
        C1 = sqrt(a1 * a1 + lab1.b * lab1.b);
        C2 = sqrt(a2 * a2 + lab2.b * lab2.b);

        if (C1 < 1e-9)
            h1 = 0.0;
        else {
            h1 = RAD2DEG(atan2(lab1.b, a1));
            if (h1 < 0.0)
                h1 += 360.0;
        }

        if (C2 < 1e-9)
            h2 = 0.0;
        else {
            h2 = RAD2DEG(atan2(lab2.b, a2));
            if (h2 < 0.0)
                h2 += 360.0;
        }
    }

    /* Compute delta L, C and H */
    double dL = lab2.L - lab1.L, dC = C2 - C1, dH;
    {
        double dh;
        if (C1 < 1e-9 || C2 < 1e-9) {
            dh = 0.0;
        } else {
            dh = h2 - h1;
            /**/ if (dh > 180.0)  dh -= 360.0;
            else if (dh < -180.0) dh += 360.0;
        }

        dH = 2.0 * sqrt(C1 * C2) * sin(DEG2RAD(0.5 * dh));
    }

    double h;
    double L = 0.5 * (lab1.L  + lab2.L);
    double C = 0.5 * (C1 + C2);
    if (C1 < 1e-9 || C2 < 1e-9) {
        h = h1 + h2;
    } else {
        h = h1 + h2;
        if (fabs(h1 - h2) > 180.0) {
            /**/ if (h < 360.0)  h += 360.0;
            else if (h >= 360.0) h -= 360.0;
        }
        h *= 0.5;
    }
    double T = 1.0
      - 0.17 * cos(DEG2RAD(h - 30.0))
      + 0.24 * cos(DEG2RAD(2.0 * h))
      + 0.32 * cos(DEG2RAD(3.0 * h + 6.0))
      - 0.2 * cos(DEG2RAD(4.0 * h - 63.0));
    double hh = (h - 275.0)/25.0;
    double ddeg = 30.0 * exp(-hh * hh);
    double C7 = pow(C,7.0);
    double RC = 2.0 * sqrt(C7/(C7 + 6103515625.0));
    double L50sq = (L - 50.0) * (L - 50.0);
    double SL = 1.0 + (0.015 * L50sq) / sqrt(20.0 + L50sq);
    double SC = 1.0 + 0.045 * C;
    double SH = 1.0 + 0.015 * C * T;
    double RT = -sin(DEG2RAD(2 * ddeg)) * RC;
    double dLsq = dL/SL, dCsq = dC/SC, dHsq = dH/SH;
    return dLsq*dLsq + dCsq*dCsq + dHsq*dHsq + RT*dCsq*dHsq;
#undef RAD2DEG
#undef DEG2RAD
}

static double ColorCompare(int r1,int g1,int b1, int r2,int g2,int b2)
{
    double luma1 = (r1*299 + g1*587 + b1*114) / (255.0*1000);
    double luma2 = (r2*299 + g2*587 + b2*114) / (255.0*1000);
    double lumadiff = luma1-luma2;
    double diffR = (r1-r2)/255.0, diffG = (g1-g2)/255.0, diffB = (b1-b2)/255.0;
    return (diffR*diffR*0.299 + diffG*diffG*0.587 + diffB*diffB*0.114)*0.75
         + lumadiff*lumadiff;
}


/* Palette */
static const unsigned maxpalettesize = 512;
static unsigned palettesize = 16;

/* Luminance for each palette entry, to be initialized as soon as the program begins */
static unsigned luma[maxpalettesize];
static LabItem  meta[maxpalettesize];
static double   pal_g[maxpalettesize][3]; // Gamma-corrected palette entry
static uint16_t offsetGloablY3=0;
inline bool PaletteCompareLuma(unsigned index1, unsigned index2)
{
    return luma[index1+offsetGloablY3] < luma[index2+offsetGloablY3];
}

typedef std::vector<unsigned> MixingPlan;
MixingPlan DeviseBestMixingPlanY2(uint8_t rIn,uint8_t gIn,uint8_t bIn,uint8_t * pal,uint16_t offset,size_t limit){
    // Input color in RGB
	int input_rgb[3] = {rIn,gIn,bIn};
	pal+=offset*3;
	offsetGloablY3=offset;
    // Input color in CIE L*a*b*
	LabItem input((double)rIn/255.0,(double)gIn/255.0,(double)bIn/255.0);

    // Tally so far (gamma-corrected)
	double so_far[3] = { 0,0,0 };
    
	MixingPlan result;
	while(result.size() < limit){
		unsigned chosen_amount = 1;
		unsigned chosen        = 0;

		const unsigned max_test_count = result.empty() ? 1 : result.size();

		double least_penalty = -1;
		for(unsigned index=0; index<palettesize; ++index){
			//const unsigned color = pal[index];
			double sum[3] = { so_far[0], so_far[1], so_far[2] };
			double add[3] = { pal_g[index+offset][0], pal_g[index+offset][1], pal_g[index+offset][2] };

			for(unsigned p=1; p<=max_test_count; p*=2){
				for(unsigned c=0; c<3; ++c) sum[c] += add[c];
				for(unsigned c=0; c<3; ++c) add[c] += add[c];
                double t = result.size() + p;

                double test[3] = { GammaUncorrect(sum[0]/t),
                                   GammaUncorrect(sum[1]/t),
                                   GammaUncorrect(sum[2]/t) };
                           
#if COMPARE_RGB
                double penalty = ColorCompare(input_rgb[0],input_rgb[1],input_rgb[2],test[0]*255, test[1]*255, test[2]*255);
#else
                LabItem test_lab( test[0], test[1], test[2] );
                double penalty = ColorCompare(test_lab, input);
#endif
                if(penalty < least_penalty || least_penalty < 0){
                    least_penalty = penalty;
                    chosen        = index;
                    chosen_amount = p;
                }
            }
        }

        // Append "chosen_amount" times "chosen" to the color list
        result.resize(result.size() + chosen_amount, chosen);

        for(unsigned c=0; c<3; ++c)
            so_far[c] += pal_g[chosen+offset][c] * chosen_amount;
    }
    // Sort the colors according to luminance
    std::sort(result.begin(), result.end(), PaletteCompareLuma);
    return result;
}
MixingPlan DeviseBestMixingPlanY3(uint8_t rIn,uint8_t gIn,uint8_t bIn,uint8_t * pal,uint16_t offset,size_t limit){
    // Input color in RGB
	int input_rgb[3] = {rIn,gIn,bIn};
	pal+=offset*3;
	offsetGloablY3=offset;
    // Input color in CIE L*a*b*
	LabItem input((double)rIn/255.0,(double)gIn/255.0,(double)bIn/255.0);

	std::map<unsigned, unsigned> Solution;

    // The penalty of our currently "best" solution.
	double current_penalty = -1;

	// First, find the closest color to the input color.
    // It is our seed.
    {
		unsigned chosen = 0;
		for(unsigned index=0; index<palettesize; ++index){
			//const unsigned color = pal[index];
			unsigned r=pal[index*3],g=pal[(index*3)+1],b=pal[(index*3)+2];
	#if COMPARE_RGB
			//unsigned r = color>>16, g = (color>>8)&0xFF, b = color&0xFF;
			double penalty = ColorCompare(input_rgb[0],input_rgb[1],input_rgb[2],r,g,b);
    #else
            LabItem test_lab(color);
            double penalty = ColorCompare(input, test_lab);
    #endif
            if(penalty < current_penalty || current_penalty < 0)
                { current_penalty = penalty; chosen = index; }
        }

        Solution[chosen] = limit;
    }

	double dbllimit = 1.0 / limit;
	while(current_penalty != 0.0){
        // Find out if there is a region in Solution that
        // can be split in two for benefit.
		double   best_penalty      = current_penalty;
		unsigned best_splitfrom    = ~0u;
		unsigned best_split_to[2]  = { 0,0};

		for(std::map<unsigned,unsigned>::iterator i = Solution.begin(); i != Solution.end(); ++i){
            //if(i->second <= 1) continue;
			unsigned split_color = i->first;
			unsigned split_count = i->second;
            // Tally the other colors
			double sum[3] = {0,0,0};
			for(std::map<unsigned,unsigned>::iterator j = Solution.begin(); j != Solution.end(); ++j){
                if(j->first == split_color) continue;
                sum[0] += pal_g[offset+j->first ][0] * j->second * dbllimit;
                sum[1] += pal_g[offset+j->first ][1] * j->second * dbllimit;
                sum[2] += pal_g[offset+j->first ][2] * j->second * dbllimit;
            }
            double portion1 = (split_count / 2            ) * dbllimit;
            double portion2 = (split_count - split_count/2) * dbllimit;
			for(unsigned a=0; a<palettesize; ++a){
				//if(a != split_color && Solution.find(a) != Solution.end()) continue;
				unsigned firstb = 0;
				if(portion1 == portion2) firstb = a+1;
				for(unsigned b=firstb; b<palettesize; ++b){
					if(a == b) continue;
                    //if(b != split_color && Solution.find(b) != Solution.end()) continue;
					int lumadiff = int(luma[offset+a]) - int(luma[offset+b]);
					if(lumadiff < 0) lumadiff = -lumadiff;
                    if(lumadiff > 80000) continue;

                    double test[3] =
                        { GammaUncorrect(sum[0] + pal_g[offset+a][0] * portion1 + pal_g[offset+b][0] * portion2),
                          GammaUncorrect(sum[1] + pal_g[offset+a][1] * portion1 + pal_g[offset+b][1] * portion2),
                          GammaUncorrect(sum[2] + pal_g[offset+a][2] * portion1 + pal_g[offset+b][2] * portion2) };
                    // Figure out if this split is better than what we had
#if COMPARE_RGB
                    double penalty = ColorCompare(input_rgb[0],input_rgb[1],input_rgb[2],test[0]*255, test[1]*255, test[2]*255);
#else
                    LabItem test_lab( test[0], test[1], test[2] );
                    double penalty = ColorCompare(input, test_lab);
#endif
					if(penalty < best_penalty){
                        best_penalty   = penalty;
                        best_splitfrom = split_color;
                        best_split_to[0] = a;
                        best_split_to[1] = b;
                    }
                    if(portion2 == 0) break;
				}
			}
		}
        if(best_penalty == current_penalty) break; // No better solution was found.

        std::map<unsigned,unsigned>::iterator i = Solution.find(best_splitfrom);
        unsigned split_count = i->second, split1 = split_count/2, split2 = split_count-split1;
        Solution.erase(i);
        if(split1 > 0) Solution[best_split_to[0]] += split1;
        if(split2 > 0) Solution[best_split_to[1]] += split2;
        current_penalty = best_penalty;
    }

    // Sequence the solution.
    MixingPlan result;
	for(std::map<unsigned,unsigned>::iterator
		i = Solution.begin(); i != Solution.end(); ++i)
    {
        result.resize(result.size() + i->second, i->first);
    }
    // Sort the colors according to luminance
    std::sort(result.begin(), result.end(), PaletteCompareLuma);
    return result;
}
inline uint8_t addCheck(uint16_t val,uint8_t add){
	val+=add;
	return (val>255)?255:val;
}
void ditherImage(uint8_t * image,uint16_t w,uint16_t h,bool useAlpha,bool colSpace,bool forceRow,uint8_t forcedrow){
	/*!
	this function will take an input with or without alpha and dither it
	Also note that this function now has the option to first dither to color space
	*/
	uint8_t ditherSetting=window->ditherPower->value();
	uint8_t type_temp=palTypeGen;
	uint8_t temp=0;
	uint8_t rgbRowsize;
	uint16_t x,y;
	if (useAlpha){
		rgbPixelsize=4;
		rgbRowsize=32;
	}
	else{
		rgbPixelsize=3;
		rgbRowsize=24;
	}
	uint8_t r_old,g_old,b_old,a_old;
	uint8_t r_new,g_new,b_new,a_new;
	uint8_t pal_row;
	int16_t error_rgb[4];
	switch (ditherAlg){
	case 3://Yliluoma's ordered dithering algorithm 1
	{
		if(colSpace){
			if(!fl_ask("Dither to colorspace? WARNING SLOW!"))
				return;
		}
		uint16_t tempPalSize;
		uint8_t * colPtr;
		if(colSpace){
			uint8_t rl,gl,bl;
			switch(game_system){
				case sega_genesis:
				{
					tempPalSize=512;
					palettesize=512;
					colPtr=(uint8_t *)malloc(512*3);
					for(rl=0;rl<=7;++rl){
						for(gl=0;gl<=7;++gl){
							for(bl=0;bl<=7;++bl){
								*colPtr++=palTab[rl];
								*colPtr++=palTab[gl];
								*colPtr++=palTab[bl];
							}
						}
					}
				}
				colPtr-=512*3;
				break;
				case NES:
					tempPalSize=64;
					palettesize=64;
					colPtr=(uint8_t *)malloc(64*3);
					for(rl=0;rl<64;++rl){
						*colPtr++=nespaltab_r[rl];
						*colPtr++=nespaltab_r[gl];
						*colPtr++=nespaltab_r[bl];
					}
					colPtr-=64*3;
				break;
			}
		}else{
			colPtr=currentProject->rgbPal;
			switch(game_system){
				case sega_genesis:
					tempPalSize=64;
					palettesize=16;
				break;
				case NES:
					tempPalSize=16;
					palettesize=4;
				break;
			}
		}
		for(unsigned c=0; c<tempPalSize; ++c){
			//unsigned r = pal[c]>>16, g = (pal[c]>>8) & 0xFF, b = pal[c] & 0xFF;
			unsigned r=colPtr[c*3],g=colPtr[(c*3)+1],b=colPtr[(c*3)+2];
			//gdImageColorAllocate(im, r,g,b);
			luma[c] = r*299 + g*587 + b*114;
			meta[c].Set((double)r/255.0,(double)g/255.0,(double)b/255.0);
			pal_g[c][0] = GammaCorrect(r/255.0);
			pal_g[c][1] = GammaCorrect(g/255.0);
			pal_g[c][2] = GammaCorrect(b/255.0);
		}
		Fl_Window *win;
		Fl_Progress *progress;
		if(h>8){
			win = new Fl_Window(400,45,"Progress");           // access parent window
			win->begin();                                // add progress bar to it..
			progress = new Fl_Progress(25,7,350,30);
			progress->minimum(0.0);                      // set progress range to be 0.0 ~ 1.0
			progress->maximum((double)h);
			progress->color(0x88888800);               // background color
			progress->selection_color(0x4444ff00);     // progress bar color
			progress->labelcolor(FL_WHITE);            // percent text color
			progress->value(0);
			win->end();                                  // end adding to window
			win->show();
			Fl::check();
		}
		for(y=0;y<h;++y){
			for(x=0;x<w;++x){
				r_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)];
				g_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+1];
				b_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+2];
				if (useAlpha)
					a_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+3];
				if (game_system == sega_genesis && type_temp != 0){
					uint8_t tempSet=(currentProject->tileMapC->get_prio(x/8,y/8)^1)*8;
					set_palette_type(tempSet);//0 normal 8 shadowed 16 highlighted
				}
				unsigned map_value = mapY3[(x & 7) + ((y & 7) << 3)];
				MixingPlan plan;
				if(colSpace){
					plan = DeviseBestMixingPlanY3(r_old,g_old,b_old,colPtr,0, 16);
				}else{
					if(forceRow)
						pal_row=forcedrow;
					else
						pal_row=currentProject->tileMapC->get_palette_map(x/8,y/8);
					plan = DeviseBestMixingPlanY3(r_old,g_old,b_old,currentProject->rgbPal,pal_row*palettesize, 16);
				}
				//unsigned color = gdImageGetTrueColorPixel(srcim, x, y);
				map_value = map_value * plan.size() / 64;
				//gdImageSetPixel(im, x,y, plan[ map_value ]);
				uint8_t tempPalOff=(plan[ map_value ]+(pal_row*palettesize))*3;
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)]=currentProject->rgbPal[tempPalOff];
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+1]=currentProject->rgbPal[tempPalOff+1];
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+2]=currentProject->rgbPal[tempPalOff+2];
				//puts("x");
				if(colSpace&&((x&31)==0)){
					//this is slower
					char txtbuf[128];
					sprintf(txtbuf,"%d/%d,%d/%d",y,h,x,w);
					progress->copy_label(txtbuf);
					progress->value((double)y+((double)x/(double)w));
					Fl::check();
				}
			}
			char txtbuf[128];
			sprintf(txtbuf,"%d/%d",y,h);
			progress->copy_label(txtbuf);
			progress->value(y);
			Fl::check();
		}
		if(h>8){
			win->remove(progress);// remove progress bar from window
			delete(progress);// deallocate it
			delete win;
		}
		if(colSpace)
			free(colPtr);
	}
	break;
	case 2://nearest color
	case 4://vertical dithering
		for (y=0;y<h;++y){
			for (x=0;x<w*rgbPixelsize;x+=rgbPixelsize){
				r_old=image[x+(y*w*rgbPixelsize)];
				g_old=image[x+(y*w*rgbPixelsize)+1];
				b_old=image[x+(y*w*rgbPixelsize)+2];
				if (useAlpha)
					a_old=image[x+(y*w*rgbPixelsize)+3];
				if(!colSpace){
					if(forceRow)
						pal_row=forcedrow;
					else
						pal_row=currentProject->tileMapC->get_palette_map(x/rgbRowsize,y/8);
				}
				//find nearest color
				uint8_t half=18;
				if (game_system == sega_genesis && type_temp != 0){
					uint8_t tempSet=(currentProject->tileMapC->get_prio(x/rgbRowsize,y/8)^1)*8;
					set_palette_type(tempSet);//0 normal 8 shadowed 16 highlighted
					half=(tempSet==0)?18:9;
				}
				if(colSpace){
					switch (game_system){
						case sega_genesis:
							r_new=palTab[nearest_color_index(r_old)];
							g_new=palTab[nearest_color_index(g_old)];
							b_new=palTab[nearest_color_index(b_old)];
						break;
						case NES:
							{uint32_t temprgb=toNesRgb(r_old,g_old,b_old);
							b_new=temprgb&255;
							g_new=(temprgb>>8)&255;
							r_new=(temprgb>>16)&255;}
						break;
					}
				}else{
					if(ditherAlg==4){
						if((x/rgbPixelsize)&1){
							r_old=addCheck(r_old,half);
							g_old=addCheck(g_old,half);
							b_old=addCheck(b_old,half);
							if(useAlpha)
								a_old=addCheck(a_old,half);
						}
					}
					temp=find_near_color_from_row_rgb(pal_row,r_old,g_old,b_old);
					r_new=currentProject->rgbPal[temp];
					g_new=currentProject->rgbPal[temp+1];
					b_new=currentProject->rgbPal[temp+2];
				}
				if (useAlpha)
					a_old=(a_old&128)?255:0;
				image[x+(y*w*rgbPixelsize)]=r_new;
				image[x+(y*w*rgbPixelsize)+1]=g_new;
				image[x+(y*w*rgbPixelsize)+2]=b_new;
				if (useAlpha)
					image[x+(y*w*rgbPixelsize)+3]=a_old;
			}
		}
	break;
	case 1:
		useMode=game_system;
		USEofColGlob=colSpace;
		forcedfun=forceRow;
		theforcedfun=forcedrow;
		Riemersma(image,w,h,0);
		Riemersma(image,w,h,1);
		Riemersma(image,w,h,2);
		if (useAlpha){
			useMode=255;
			Riemersma(image,w,h,3);
		}
	break;
	case 0:
		for (y=0;y<h;y++){
			for (x=0;x<w;x++){
				//we need to get nearest color
				r_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)];
				g_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+1];
				b_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+2];
				if (useAlpha)
					a_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+3];
				if(!colSpace){
					if(forceRow)
						pal_row=forcedrow;
					else
						pal_row=currentProject->tileMapC->get_palette_map(x/8,y/8);
				}
				//find nearest color
				if (game_system == sega_genesis && type_temp != 0){
					uint8_t tempSet=(currentProject->tileMapC->get_prio(x/8,y/8)^1)*8;
					set_palette_type(tempSet);//0 normal 8 shadowed 16 highlighted
				}
				if(colSpace){
					switch (game_system){
						case sega_genesis:
							r_new=palTab[nearest_color_index(r_old)];
							g_new=palTab[nearest_color_index(g_old)];
							b_new=palTab[nearest_color_index(b_old)];
						break;
						case NES:
							{uint32_t temprgb=toNesRgb(r_old,g_old,b_old);
							b_new=temprgb&255;
							g_new=(temprgb>>8)&255;
							r_new=(temprgb>>16)&255;}
						break;
					}
				}else{
					temp=find_near_color_from_row_rgb(pal_row,r_old,g_old,b_old);
					r_new=currentProject->rgbPal[temp];
					g_new=currentProject->rgbPal[temp+1];
					b_new=currentProject->rgbPal[temp+2];
				}
				if (useAlpha){
					a_new=(a_old&128)?255:0;
					//a_new&=128;
					error_rgb[3]=(int16_t)a_old-(int16_t)a_new;
					image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+3]=a_new;
				}
				error_rgb[0]=(int16_t)r_old-(int16_t)r_new;
				error_rgb[1]=(int16_t)g_old-(int16_t)g_new;
				error_rgb[2]=(int16_t)b_old-(int16_t)b_new;
				for (uint8_t channel=0;channel<rgbPixelsize;channel++){
					//add the offset
					if (x+1 < w){
						plus_truncate_uchar(image[((x+1)*rgbPixelsize)+(y*w*rgbPixelsize)+channel],(error_rgb[channel]*7) / ditherSetting);
					}
					if (x-1 > 0 && y+1 < h){
						plus_truncate_uchar(image[((x-1)*rgbPixelsize)+((y+1)*w*rgbPixelsize)+channel],(error_rgb[channel]*3) / ditherSetting);
					}
					if (y+1 < h){
						plus_truncate_uchar(image[(x*rgbPixelsize)+((y+1)*w*rgbPixelsize)+channel],(error_rgb[channel]*5) / ditherSetting);
					}
					if (x+1 < w && y+1 < h){
						plus_truncate_uchar(image[((x+1)*rgbPixelsize)+((y+1)*w*rgbPixelsize)+channel],(error_rgb[channel]) / ditherSetting);
					}
				}
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)]=r_new;
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+1]=g_new;
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+2]=b_new;
			}
		}
	break;
	}
	if (game_system == sega_genesis)
		set_palette_type(type_temp);

}
