/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2016)
*/
#include "includes.h"
#include "image.h"
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Scroll.H>
static Fl_Window * win;
static bool useAlpha;
static bool retOkay;
static void RetCB(Fl_Widget*,void*r){
	retOkay=r?true:false;
	win->hide();
}
class ScrollBox : public Fl_Box{
public:
	Fl_Scroll *scroll;
	Fl_Shared_Image*loaded_image;
	unsigned depth,w,h,*remap;
	uint8_t*palMap;
	uint8_t r,g,b,ent;
	Fl_Color_Chooser*colsel;
	bool grayscale;
	int handle(int e){
		if(e==FL_PUSH){
			int x=Fl::event_x() - scroll->x() + scroll->hscrollbar.value(),y=Fl::event_y() - scroll->y() + scroll->scrollbar.value();
			if((x>=0)&&(x<w)&&(y>=0)&&(y<h)){
				uint8_t*imgptr;
				switch(depth){
					case 1:
						if(grayscale){
							imgptr=(uint8_t*)loaded_image->data()[0];
							imgptr+=(y*w)+x;
							r=*imgptr;
							g=*imgptr;
							b=*imgptr;

						}else{
							imgptr=(uint8_t*)loaded_image->data()[y+2];
							imgptr+=x;
							if(*imgptr!=' '){
								ent=*imgptr;
								r=palMap[remap[ent]+1];
								g=palMap[remap[ent]+2];
								b=palMap[remap[ent]+3];
							}
						}
					break;
					case 3:
						imgptr=(uint8_t*)loaded_image->data()[0];
						imgptr+=((y*w)+x)*3;
						r=*imgptr++;
						g=*imgptr++;
						b=*imgptr;
					break;
					case 4:
						imgptr=(uint8_t*)loaded_image->data()[0];
						imgptr+=((y*w)+x)*4;
						if(imgptr[3]){
							r=*imgptr++;
							g=*imgptr++;
							b=*imgptr;
						}
					break;
				}
				double rc,gc,bc;
				rc=(double)r/255.0;
				gc=(double)g/255.0;
				bc=(double)b/255.0;
				colsel->rgb(rc,gc,bc);
				if(!useAlpha)
					win->redraw();
			}
		}
		return(Fl_Box::handle(e));
	}
	ScrollBox(int x,int y,int w,int h,const char*l=0) : Fl_Box(x,y,w,h,l){
		box(FL_NO_BOX);
	}
};
static ScrollBox*box;
static void setAlphaCB(Fl_Widget*,void*a){
	if(a){
		useAlpha=true;
		box->colsel->hide();
	}else{
		useAlpha=false;
		box->colsel->show();
	}
	win->redraw();
}
bool getMaskColorImg(Fl_Shared_Image*loaded_image,bool grayscale,unsigned*remap,uint8_t*palMap,uint8_t*mask,bool&alphaSel){
	unsigned depth=loaded_image->d();
	uint64_t*histr=0,*histg=0,*histb=0,*hista=0;
	unsigned w=loaded_image->w(),h=loaded_image->h();
	unsigned imgb=w*h;
	uint8_t*imgptr;
	double rc,gc,bc;
	uint8_t r=0,g=0,b=0,ent=0;
	histr=(uint64_t*)calloc(sizeof(uint64_t),256);
	if(((depth==1)&&(!grayscale))||(depth==4))
		hista=(uint64_t*)calloc(sizeof(uint64_t),256);
	if(depth>1){
		histg=(uint64_t*)calloc(sizeof(uint64_t),256);
		histb=(uint64_t*)calloc(sizeof(uint64_t),256);
	}
	useAlpha=false;
	switch(depth){
		case 4:
			imgptr=(uint8_t*)loaded_image->data()[0];
			for(unsigned i=0;i<imgb;++i){
				++histr[*imgptr++];
				++histg[*imgptr++];
				++histb[*imgptr++];
				++hista[*imgptr++];
			}
			{uint64_t maxr=*histr,maxg=*histg,maxb=*histb,maxa=*hista,*ptrr=histr,*ptrg=histg,*ptrb=histb,*ptra=hista;
			unsigned alpha=0;
			for(unsigned i=0;i<256;++i,++ptrr,++ptrg,++ptrb,++ptra){
				if(maxr<*ptrr){
					maxr=*ptrr;
					r=i;
				}
				if(maxg<*ptrg){
					maxg=*ptrg;
					g=i;
				}
				if(maxb<*ptrb){
					maxb=*ptrb;
					b=i;
				}
				if(maxa<*ptra){
					maxa=*ptra;
					alpha=i;
				}
			}
			useAlpha=(alpha)?false:true;
			}
		break;
		case 3:
			imgptr=(uint8_t*)loaded_image->data()[0];
			for(unsigned i=0;i<imgb;++i){
				++histr[*imgptr++];
				++histg[*imgptr++];
				++histb[*imgptr++];
			}
			{uint64_t maxr=*histr,maxg=*histg,maxb=*histb,*ptrr=histr,*ptrg=histg,*ptrb=histb;
			for(unsigned i=0;i<256;++i,++ptrr,++ptrg,++ptrb){
				if(maxr<*ptrr){
					maxr=*ptrr;
					r=i;
				}
				if(maxg<*ptrg){
					maxg=*ptrg;
					g=i;
				}
				if(maxb<*ptrb){
					maxb=*ptrb;
					b=i;
				}
			}
			}
		break;
		case 1:
			if(grayscale){
				imgptr=(uint8_t*)loaded_image->data()[0];
				for(unsigned i=0;i<imgb;++i)
					++histr[*imgptr++];
			}else{
				for(unsigned y=0;y<h;++y){
					imgptr=(uint8_t*)loaded_image->data()[y+2];
					for(unsigned x=0;x<w;++x){
						if(*imgptr==' '){
							++hista[0];
						}else{
							++histr[*imgptr++];
							++hista[255];
						}
					}
				}
			}
			{uint64_t maxv=*histr,*ptrv=histr;
			if(grayscale){
				for(unsigned i=0;i<256;++i,++ptrv){
					if(maxv<*ptrv){
						maxv=*ptrv;
						ent=i;
					}
				}
				r=g=b=ent;
			}else{
				uint64_t maxa=*hista,*ptra=hista;
				unsigned alpha=0;
				for(unsigned i=0;i<256;++i,++ptrv,++ptra){
					if(maxv<*ptrv){
						maxv=*ptrv;
						ent=i;
					}
					if(maxa<*ptra){
						maxa=*ptra;
						alpha=i;
					}
				}
				useAlpha=(alpha)?false:true;
				r=palMap[remap[ent]+1];
				g=palMap[remap[ent]+2];
				b=palMap[remap[ent]+3];
			}
			}
		break;
	}
	if(histr)
		free(histr);
	if(histg)
		free(histg);
	if(histb)
		free(histb);
	if(hista)
		free(hista);
	rc=(double)r/255.0;
	gc=(double)g/255.0;
	bc=(double)b/255.0;
	win=new Fl_Double_Window(640,480,"Background color selection");
	win->begin();
	win->resizable(win);
	Fl_Button * Ok=new Fl_Button(52,448,64,24,"Okay");
	Ok->callback(RetCB,(void*)1);
	Fl_Button * Cancel=new Fl_Button(116,448,64,24,"Cancel");
	Cancel->callback(RetCB,0);
	Fl_Color_Chooser*colsel=new Fl_Color_Chooser(16,32,208,156,"Background color\nif incorrect click on the image");
	colsel->rgb(rc,gc,bc);
	colsel->mode(1);
	Fl_Scroll*scroll=new Fl_Scroll(232,16,408,464);
	box=new ScrollBox(232,16,w,h);
	box->scroll=scroll;
	box->w=w;
	box->h=h;
	box->depth=depth;
	box->remap=remap;
	box->palMap=palMap;
	box->r=r;
	box->g=g;
	box->b=b;
	box->ent=ent;
	box->colsel=colsel;
	box->grayscale=grayscale;
	box->loaded_image=loaded_image;
	box->image(loaded_image);
	scroll->end();
	if(((depth==1)&&(!grayscale)&&(hista[0]))||(depth==4)){
		Fl_Group*g=new Fl_Group(8,240,240,72);
		Fl_Round_Button*ua,*um;
		if(useAlpha){
			ua=new Fl_Round_Button(8,240,128,72,"Use alpha\nchannel\nDetected as such");
			ua->set();
			colsel->hide();
		}else
			ua=new Fl_Round_Button(8,240,128,48,"Use alpha\nchannel");
		ua->callback(setAlphaCB,(void*)1);
		ua->type(FL_RADIO_BUTTON);
		if(useAlpha)
			um=new Fl_Round_Button(144,240,96,48,"Use mask\ncolor");
		else{
			um=new Fl_Round_Button(144,240,96,72,"Use mask\ncolor\nDetected as such");
			um->set();
		}
		um->callback(setAlphaCB,0);
		um->type(FL_RADIO_BUTTON);
		g->end();
	}
	win->end();
	win->set_modal();
	win->show();
	while(win->shown())
		Fl::wait();
	if(depth>1){
		mask[0]=box->r;
		mask[1]=box->g;
		mask[2]=box->b;
	}else
		*mask=box->ent;
	alphaSel=useAlpha;
	delete win;
	return retOkay;
}
bool handle1byteImg(Fl_Shared_Image*loaded_image,unsigned*remap,unsigned*numcol){
	char*timgptr=(char*)loaded_image->data()[0];
	//See if grayscale or colormapped xpm
	if(isdigit(*timgptr)){
		/* Checking to see if the first byte is a digit is not enough.
		 * What if the first pixel just happen to fall in digit range?
		 * Avoid this by verifying width and height*/
		if(strtol(timgptr,&timgptr,10)==loaded_image->w()){
			if(strtol(timgptr,&timgptr,10)==loaded_image->h()){
				unsigned numcolors=abs(strtol(timgptr,&timgptr,10));
				if(numcol)
					*numcol=numcolors;
				uint8_t*palMap=(uint8_t*)loaded_image->data()[1];
				std::fill(remap,remap+256,0);
				for(unsigned xx=0;xx<numcolors*4;xx+=4)
					remap[palMap[xx]]=xx;
				return false;
			}else
				return true;
		}else
			return true;

	}else
		return true;
}
