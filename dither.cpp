#include <inttypes.h>
#include "dither.h"
#include "global.h"

#define NONE 0
#define UP 1
#define LEFT 2
#define DOWN 3
#define RIGHT 4
uint8_t useHiL;
uint8_t useMode;
uint8_t rgbPixelsize;
uint8_t nearest_color_chan(uint8_t val,uint8_t chan,uint8_t row)
{
	//returns closest value
	//palette_muliplier
	unsigned char i;
    int distanceSquared, minDistanceSquared, bestIndex = 0;
    minDistanceSquared = 255*255 + 1;
	unsigned char max_rgb;
	switch (useMode)
	{
		case sega_genesis:
			max_rgb=48;//16*3=48
		break;
		case NES:
			max_rgb=12;//4*3=12
		break;
		case 255://alpha
			//chan/=128;
			//chan*=255;
			chan&=128;
			return chan;
		break;
	}
	row*=max_rgb;
    for (i=0; i<max_rgb; i+=3)
	{
        int Rdiff = (int) val - (int)rgb_pal[i+row+chan];
        distanceSquared = Rdiff*Rdiff;
        if (distanceSquared < minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            bestIndex = i;
        }
    }
    return rgb_pal[bestIndex+row+chan];
}

/* variables needed for the Riemersma
 * dither algorithm */ 
static int cur_x=0, cur_y=0;
static int img_width=0, img_height=0; 
static unsigned char *img_ptr_dither;
static unsigned char rgb_select=0;
    
#define SIZE 16 /* queue size: number of
                 * pixels remembered */ 
#define MAX  16 /* relative weight of
                 * youngest pixel in the
                 * queue, versus the oldest
                 * pixel */
    
static int weights[SIZE]; /* weights for
                           * the errors
                           * of recent
                           * pixels */
    
static void init_weights(int a[],int size,int max) 
{
  double m = exp(log(max)/(size-1));
  double v;
  int i;
    
  for (i=0, v=1.0; i<size; i++) {
    a[i]=(int)(v+0.5); /* store rounded
                        * value */ 
    v*=m;              /* next value */
  } /*for */
}
    
static void dither_pixel(unsigned char *pixel) 
{
static int error[SIZE]; /* queue with error
                         * values of recent
                         * pixels */
  int i,pvalue,err;
    
  for (i=0,err=0L; i<SIZE; i++)
    err+=error[i]*weights[i];
  //pvalue=*pixel + err/MAX;
 if (*pixel+err/MAX > 255)
  {
	  pvalue=255;
  }
  else if (*pixel+err/MAX < 0)
  {
	  pvalue=0;
  }
  else
  {
	  pvalue=*pixel + err/MAX;
  }
  //pvalue = (pvalue>=128) ? 255 : 0;
	if ((game_system == sega_genesis) && (useHiL == 9))
	{
		bool tempSet=get_prio(cur_x/8,cur_y/8)^true;
		set_palette_type(tempSet);
	}
	pvalue=nearest_color_chan(pvalue,rgb_select,get_palette_map(cur_x/8,cur_y/8));
  /* shift queue */ 
  memmove(error, error+1,
    (SIZE-1)*sizeof error[0]); 
  error[SIZE-1] = *pixel - pvalue;
  *pixel=(unsigned char)pvalue;
}
    
static void move(int direction)
{
  /* dither the current pixel */
  if (cur_x >= 0 && cur_x < img_width &&
      cur_y >= 0 && cur_y < img_height)
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
    
void hilbert_level(int level,int direction)
{
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
    
int log2(int value)
{
  int result=0;
  while (value>1) {
    value >>= 1;
    result++;
  } /*while */
  return result;
}
    
void Riemersma(uint8_t *image, int width,int height,uint8_t rgb_sel)
{
  int level,size;
	rgb_select=rgb_sel;
  /* determine the required order of the
   * Hilbert curve */ 
  size=max(width,height);
  level=log2(size);
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

void ditherImage(uint8_t * image,uint16_t w,uint16_t h,bool useAlpha)
{
	/*!
	this function will take an input with or without alpha and dither it
	*/
	uint8_t ditherSetting=window->ditherPower->value();
	uint8_t type_temp,temp;
	uint8_t rgbRowsize;
	uint16_t x,y;
	if (useAlpha)
	{
		rgbPixelsize=4;
		rgbRowsize=32;
	}
	else
	{
		rgbPixelsize=3;
		rgbRowsize=24;
	}
	bool tempSet;
	uint8_t r_old,g_old,b_old,a_old;
	uint8_t r_new,g_new,b_new,a_new;
	uint8_t pal_row;
	int16_t error_rgb[4];
	useHiL=palette_muliplier;
	if (palette_adder==0)
	{
		type_temp=1;
	}
	else
	{
		type_temp=2;
	}
	switch (ditherAlg)
	{
	case 2://nearest color
		for (y=0;y<h;y++)
		{
			for (x=0;x<w*rgbPixelsize;x+=rgbPixelsize)
			{
				//we need to get nearest color
				r_old=image[x+(y*w*rgbPixelsize)];
				g_old=image[x+(y*w*rgbPixelsize)+1];
				b_old=image[x+(y*w*rgbPixelsize)+2];
				if (useAlpha)
					a_old=image[x+(y*w*rgbPixelsize)+3];
				pal_row=get_palette_map(x/rgbRowsize,y/8);
				//find nearest color
				if ((game_system == sega_genesis) && (useHiL == 9))
				{
					tempSet=get_prio(x/rgbRowsize,y/8)^true;
					set_palette_type(tempSet);
				}
				temp=find_near_color_from_row_rgb(pal_row,r_old,g_old,b_old);
				r_new=rgb_pal[temp];
				g_new=rgb_pal[temp+1];
				b_new=rgb_pal[temp+2];
				if (useAlpha)
				{
					a_old/=128;
					a_old*=255;
				}
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
		Riemersma(image,w,h,0);
		Riemersma(image,w,h,1);
		Riemersma(image,w,h,2);
		if (useAlpha)
		{
			useMode=255;
			Riemersma(image,w,h,3);
		}
	break;
	case 0:
		for (y=0;y<h;y++)
		{
			for (x=0;x<w;x++)
			{
				//we need to get nearest color
				r_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)];
				g_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+1];
				b_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+2];
				if (useAlpha)
					a_old=image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+3];
				pal_row=get_palette_map(x/8,y/8);
				//find nearest color
				if ((game_system == sega_genesis) && (useHiL == 9))
				{
					tempSet=get_prio(x/8,y/8)^true;
					set_palette_type(tempSet);
				}
				temp=find_near_color_from_row_rgb(pal_row,r_old,g_old,b_old);
				r_new=rgb_pal[temp];
				g_new=rgb_pal[temp+1];
				b_new=rgb_pal[temp+2];
				if (useAlpha)
				{
					a_new=a_old;
					//a_new/=128;
					//a_new*=255;
					a_new&=128;
					error_rgb[3]=(short)a_old-(short)a_new;
					image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+3]=a_new;
				}
				error_rgb[0]=(short)r_old-(short)r_new;
				error_rgb[1]=(short)g_old-(short)g_new;
				error_rgb[2]=(short)b_old-(short)b_new;
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)]=r_new;
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+1]=g_new;
				image[(x*rgbPixelsize)+(y*w*rgbPixelsize)+2]=b_new;
				for (unsigned char channel=0;channel<rgbPixelsize;channel++)
				{
					//add the offset
					if (x+1 < w)
					{
						plus_truncate_uchar(image[((x+1)*rgbPixelsize)+(y*w*rgbPixelsize)+channel],(error_rgb[channel]*7) / ditherSetting);
					}
					if (x-1 > 0 && y+1 < h)
					{
						plus_truncate_uchar(image[((x-1)*rgbPixelsize)+((y+1)*w*rgbPixelsize)+channel],(error_rgb[channel]*3) / ditherSetting);
					}
					if (y+1 < h)
					{
						plus_truncate_uchar(image[(x*rgbPixelsize)+((y+1)*w*rgbPixelsize)+channel],(error_rgb[channel]*5) / ditherSetting);
					}
					if (x+1 < w && y+1 < h)
					{
						plus_truncate_uchar(image[((x+1)*rgbPixelsize)+((y+1)*w*rgbPixelsize)+channel],(error_rgb[channel]) / ditherSetting);
					}

				}
			}
		}
	break;
	}
	if ((game_system == sega_genesis) && (useHiL == 9))
		set_palette_type(type_temp);

}
