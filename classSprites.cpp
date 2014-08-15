/*
   This file is part of Retro Graphics Toolkit

   Retro Graphics Toolkit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any later version.

   Retro Graphics Toolkit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Retro Graphics Toolkit.  If not, see <http://www.gnu.org/licenses/>.
   Copyright Sega16 (or whatever you wish to call me) (2012-2014)
*/
#include <stdlib.h>
#include <string.h>
#include <FL/Fl_Scroll.H>
#include "classSprite.h"
#include "classSprites.h"
#include "includes.h"
#include "callback_tiles.h"
#include "global.h"
#include "image.h"
const char*spriteDefName="DefaultGroupLabel";
const char*spritesName="AllGroupsLabel";
#if _WIN32
static inline uint16_t swap_word(uint16_t w){
	uint8_t a,b;
	a=w&255;
	b=w>>8;
	return (a<<8)|b;
}
#define be16toh swap_word
#define htobe16 swap_word
#endif
sprites::sprites(){
	amt=1;
	groups.push_back(spriteGroup());
	groups[0].list.push_back(sprite());
	groups[0].name.assign(spriteDefName);
	groups[0].offx.push_back(0);
	groups[0].offy.push_back(0);
	groups[0].loadat.push_back(0);
	name.assign(spritesName);
}
sprites::sprites(const sprites& other){
	groups.reserve(other.groups.size());
	name=other.name;
	for(uint32_t i=0;i<other.groups.size();++i)
		groups.push_back(spriteGroup());
	for(uint32_t j=0;j<other.groups.size();++j){
		unsigned sz=other.groups[j].list.size();
		groups[j].list.reserve(sz);
		groups[j].offx=other.groups[j].offx;
		groups[j].offy=other.groups[j].offy;
		groups[j].loadat=other.groups[j].loadat;
		groups[j].name=other.groups[j].name;
		for(uint32_t i=0;i<sz;++i)
			groups[j].list.push_back(sprite(other.groups[j].list[i].w,other.groups[j].list[i].h,other.groups[j].list[i].palrow,other.groups[j].list[i].starttile,other.groups[j].list[i].hflip,other.groups[j].list[i].vflip));
	}
	amt=other.amt;
}
sprites::~sprites(){
	for(uint32_t j=0;j<amt;++j){
		groups[j].list.clear();
		groups[j].offx.clear();
		groups[j].offy.clear();
		groups[j].list.clear();
		groups[j].name.clear();
	}
	name.clear();
	groups.clear();
}
static bool chkNotZero(uint8_t*dat,unsigned n){
	while(n--){
		if(*dat++)
			return true;
	}
	return false;
}
void sprites::fixDel(unsigned at,unsigned tamt){
	for(unsigned i=0;i<amt;++i){
		for(unsigned j=0;j<groups[i].list.size();++j){
			if(groups[i].list[j].starttile>=at){
				groups[i].list[j].starttile-=tamt;
				groups[i].loadat[j]-=tamt;
			}
		}
	}
}
void sprites::optimizeBlank(unsigned which){
	//Check for blank collums
	for(int i=groups[which].list.size()-1;i>=0;--i){
		//First check if the sprite is completly blank
		bool notBlank=false;
		for(unsigned h=0,ctile=groups[which].list[i].starttile;h<groups[which].list[i].h;++h){
			for(unsigned w=0;w<groups[which].list[i].w;++w,++ctile)
				notBlank|=chkNotZero(currentProject->tileC->truetDat.data()+(ctile*256),256);
		}
		if(!notBlank){
			//Completly remove the sprite
			int tiledel=groups[which].list[i].starttile,tiledelamt=groups[which].list[i].h*groups[which].list[i].w;
				if(groups[which].list.size()<=1)
					del(which);
				else
					delingroup(which,i);
			for(int td=tiledel+tiledelamt-1;td>=tiledel;--td)
				currentProject->tileC->remove_tile_at(td);
			fixDel(tiledel,tiledelamt);
			if(groups[which].list.size()<1)
				return;
		}
	}
}
static bool isMask(int x,int y,Fl_Shared_Image*loaded_image,bool grayscale,bool useAlpha,uint8_t*mask){
	uint8_t*imgptr;
	int w=loaded_image->w(),h=loaded_image->h();
	if(x>=w)
		return true;
	if(x<0)
		return true;
	if(y>=h)
		return true;
	if(y<0)
		return true;
	unsigned depth=loaded_image->d();
	switch(depth){
		case 1:
			if(grayscale){
				imgptr=(uint8_t*)loaded_image->data()[0];
				imgptr+=(y*w)+x;
				return ((*imgptr)==(*mask));
			}else{
				imgptr=(uint8_t*)loaded_image->data()[y+2];
				imgptr+=x;
				if(useAlpha)
					return ((*imgptr)==' ');
				else
					return ((*imgptr)==(*mask));
			}
		break;
		case 3:
			imgptr=(uint8_t*)loaded_image->data()[0];
			imgptr+=((y*w)+x)*3;
			if(imgptr[0]==mask[0]){
				if(imgptr[1]==mask[1]){
					if(imgptr[2]==mask[2])
						return true;
				}
			}
			return false;
		break;
		case 4:
			imgptr=(uint8_t*)loaded_image->data()[0];
			imgptr+=((y*w)+x)*4;
			if(useAlpha)
				return (imgptr[3])?false:true;
			else{
				if(imgptr[3]){
					if(imgptr[0]==mask[0]){
						if(imgptr[1]==mask[1]){
							if(imgptr[2]==mask[2])
								return true;
						}
					}
				}
				return false;
			}
		break;
	}
}
static bool inRange(int num,int min,int max){
	return (num>=min)&&(num<=max);
}
class RectBox : public Fl_Box{
public:
	std::vector<int>*rects;
	std::vector<bool>*deleted;
	int sel;
	Fl_Scroll *scroll;
	int handle(int e){
		return(Fl_Box::handle(e));
	}
	void draw(void){
		int minx=scroll->x()-x(),miny=scroll->y()-y();
		int maxx=minx+scroll->w(),maxy=miny+scroll->h();
		Fl_Box::draw();
		//Now draw the rectanges
		for(unsigned i=0;i<rects->size();i+=4){
			if(((*rects)[i]>=minx)||((*rects)[i]<=maxx)){
				if(((*rects)[i+2]>=miny)||((*rects)[i+2]<=maxy)){
					if(!(*deleted)[i/4])
						fl_draw_box(FL_EMBOSSED_FRAME,(*rects)[i]+x(),(*rects)[i+2]+y(),(*rects)[i+1]-(*rects)[i]+1,(*rects)[i+3]-(*rects)[i+2]+1,0);
				}
			}
		}
	}
	RectBox(int x,int y,int w,int h,const char*l=0) : Fl_Box(x,y,w,h,l){
		box(FL_NO_BOX);
	}
};
static RectBox*box;
static bool retOkay;
static Fl_Window * win;
static void RetCB(Fl_Widget*,void*r){
	retOkay=r?true:false;
	win->hide();
}
bool sprites::recttoSprite(int x0,int x1,int y0,int y1,int where,Fl_Shared_Image*loaded_image,bool grayscale,unsigned*remap,uint8_t*palMap,uint8_t*mask,bool useMask,bool useAlpha){
	if(where<0){
		where=amt;
		setAmt(amt+1);
	}
	unsigned depth=loaded_image->d();
	unsigned wmax,hmax;
	switch(currentProject->gameSystem){
		case sega_genesis:
			wmax=hmax=32;
		break;
		case NES:
			wmax=8;
			hmax=16;
		break;
	}
	unsigned wf,hf,w,h,wt,ht;
	wf=loaded_image->w();
	hf=loaded_image->h();
	w=x1-x0+1;
	h=y1-y0+1;
	wt=(w+7)&(~7);
	ht=(h+7)&(~7);
	//Determin how many sprites will be created
	unsigned spritesnew=((wt+wmax-8)/wmax)*((ht+hmax-8)/hmax);
	if(where>=amt)
		setAmt(where+1);
	unsigned startTile=currentProject->tileC->amt;
	uint8_t*out=currentProject->tileC->truetDat.data()+(startTile*256);
	unsigned newTiles=(wt/8)*(ht/8);
	currentProject->tileC->amt+=newTiles;
	//set new amount
	currentProject->tileC->resizeAmt();
	out=currentProject->tileC->truetDat.data()+(startTile*currentProject->tileC->tcSize);
	setAmtingroup(where,spritesnew);
	unsigned center[3];
	center[0]=(wt-w)/2;
	center[1]=(ht-h)/2;
	center[2]=w+center[0];
	uint8_t * imgptr=(uint8_t *)loaded_image->data()[0];
	printf("Center %d %d %d\n",center[0],center[1],center[2]);
	printf("w: %d h: %d wt: %d ht: %d newtiles: %d\n",w,h,wt,ht,newTiles);
	for(unsigned y=0,cnt=0,tilecnt=startTile;y<ht;y+=hmax){
		for(unsigned x=0;x<wt;x+=wmax,++cnt){
			unsigned dimx,dimy;
			dimx=((wt-x)>=wmax)?wmax:(wt-x)%wmax;
			dimy=((ht-y)>=hmax)?hmax:(ht-y)%hmax;
			groups[where].list[cnt].w=dimx/8;
			groups[where].list[cnt].h=dimy/8;
			groups[where].list[cnt].starttile=tilecnt;
			groups[where].offx[cnt]=x;
			groups[where].offy[cnt]=y;
			groups[where].loadat[cnt]=tilecnt;
			tilecnt+=(dimx/8)*(dimy/8);
			for(unsigned i=0;i<dimx;i+=8){
				for(unsigned j=0;j<dimy;j+=8){
					for(unsigned b=0;b<8;++b){
						for(unsigned a=0;a<8;++a){
							unsigned xx=x+i+a;
							unsigned yy=y+j+b;
							//printf("%d %d\n",xx,yy);
							if((!((yy<center[1])||(yy>=(h+center[1]))))&&(depth==1)&&(!grayscale))
								imgptr=(uint8_t*)loaded_image->data()[yy+y0+2-center[1]];
							else if(!((yy<center[1])||(yy>=(h+center[1])))){
								imgptr=(uint8_t *)loaded_image->data()[0];
								imgptr+=((yy+y0)-center[1])*wf*depth;
							}
							if((xx>center[0])&&(xx<=center[2]))
								imgptr+=((xx+x0)-center[0])*depth;
							if((yy<center[1])||(yy>=(h+center[1]))){
								memset(out,0,4);
							}else if(xx<center[0]){
								memset(out,0,4);
							}else if(xx>=center[2]){
								memset(out,0,4);
							}else{
								//Can actully convert pixel to tile
								if(useMask&&(!useAlpha)){
									switch(depth){
										case 1:
											if(grayscale){
												if((*imgptr)!=mask[0]){
													memset(out,*imgptr,3);
													out[3]=255;
												}else{
													memset(out,0,4);
												}
											}else{
												if((*imgptr)==' '){
													memset(out,0,4);
												}else{
													if((*imgptr)!=mask[0]){
														unsigned p=(*imgptr++);
														out[0]=palMap[remap[p]+1];
														out[1]=palMap[remap[p]+2];
														out[2]=palMap[remap[p]+3];
														out[3]=255;
													}else{
														memset(out,0,4);
													}
												}
											}
										break;
										case 3:
											if((imgptr[0]==mask[0])&&(imgptr[1]==mask[1])&&(imgptr[2]==mask[2])){
												memset(out,0,4);
											}else{
												memcpy(out,imgptr,3);
												out[3]=255;
											}
										break;
										case 4:
											if((imgptr[0]==mask[0])&&(imgptr[1]==mask[1])&&(imgptr[2]==mask[2])&&(imgptr[3])){
												memset(out,0,4);
											}else{
												memcpy(out,imgptr,4);
											}
										break;
									}
								}else{
									switch(depth){
										case 1:
											if(grayscale){
												memset(out,*imgptr,3);
												out[3]=255;
											}else{
												if((*imgptr)==' '){
													memset(out,0,4);
												}else{
													unsigned p=(*imgptr++);
													out[0]=palMap[remap[p]+1];
													out[1]=palMap[remap[p]+2];
													out[2]=palMap[remap[p]+3];
													out[3]=255;
												}
											}
										break;
										case 3:
											memcpy(out,imgptr,3);
											out[3]=255;
										break;
										case 4:
											memcpy(out,imgptr,4);
										break;
									}
								}
							}
							out+=4;
						}
					}
				}
			}
		}
	}
}
void sprites::importSpriteSheet(void){
	if(load_file_generic("Load image")){
		Fl_Shared_Image * loaded_image=Fl_Shared_Image::get(the_file.c_str());
		if(!loaded_image){
			fl_alert("Error loading image");
			return;
		}
		unsigned depth=loaded_image->d();
		if (unlikely(depth != 3 && depth != 4 && depth!=1)){
			fl_alert("Please use color depth of 1,3 or 4\nYou Used %d",depth);
			loaded_image->release();
			return;
		}else
			printf("Image depth %d\n",depth);
		uint32_t w,h;
		w=loaded_image->w();
		h=loaded_image->h();
		bool grayscale;
		uint8_t*palMap;
		uint8_t*imgptr;
		unsigned remap[256];
		if(depth==1){
			grayscale=handle1byteImg(loaded_image,remap);
			if(!grayscale){
				palMap=(uint8_t*)loaded_image->data()[1];
				imgptr=(uint8_t*)loaded_image->data()[2];
			}
		}
		uint8_t mask[3];
		bool useAlpha;
		if(getMaskColorImg(loaded_image,grayscale,remap,palMap,mask,useAlpha)){
			std::vector<int> rects;//x0,x1,y0,y1
			Fl_Window *winP;
			Fl_Progress *progress;
			mkProgress(&winP,&progress);
			time_t lasttime=time(NULL);
			progress->maximum(h);
			Fl::check();
			for(int y=0;y<h;++y){
				for(int x=0;x<w;++x){
					if(!isMask(x,y,loaded_image,grayscale,useAlpha,mask)){
						rects.push_back(x);
						while(!isMask(x+1,y,loaded_image,grayscale,useAlpha,mask))
							++x;
						rects.push_back(x);
						rects.push_back(y);
						rects.push_back(y);
					}
				}
				if((time(NULL)-lasttime)>=1){
					lasttime=time(NULL);
					progress->value(h);
					Fl::check();
				}
			}
			progress->maximum(rects.size());
			progress->value(0);
			//Now combine the rectanges
			//Start by combining rectanges by that touch with y values
			bool canEnd;
			int pass=0;
			char txtbufstage[1024];
			char txtbuf[1024];
			do{
			canEnd=true;
			snprintf(txtbufstage,1024,"Stage 1 pass %d",pass++);
			winP->label(txtbufstage);
			Fl::check();
			for(int i=0;i<rects.size();i+=4){
				for(int j=0;j<rects.size();j+=4){
					//printf("%d %d\n",i,j);
					if(i==j)
						continue;
					//See if rectanges are touching or overlap
					//if((inRange(rects[j+2],rects[i+2]-1,rects[i+3]+1)||inRange(rects[i+2],rects[j+2]-1,rects[j+3]+1))&&(!((rects[i+2]==rects[j+2])||(rects[i+3]==rects[j+3])))){//Is rectange j directly above or below i
					if((rects[j+3]-rects[i+2])==1){
						if((inRange(rects[j],rects[i]-1,rects[i+1]+1)||inRange(rects[i],rects[j]-1,rects[j+1]+1))){
							canEnd=false;
							//Merge the two squares obtainting maximum size
							//Now try and find the combination that results in the largest rectange
							rects[i]=std::min(rects[i],rects[j]);
							rects[i+1]=std::max(rects[i+1],rects[j+1]);
							rects[i+2]=std::min(rects[i+2],rects[j+2]);
							rects[i+3]=std::max(rects[i+3],rects[j+3]);
							rects.erase(rects.begin()+j,rects.begin()+j+4);
							//Now try to find next in sequence
							bool foundit;
							do{
								foundit=false;
								for(int a=0;a<rects.size();a+=4){
									int look=rects[i+3]+1;
									if(rects[a+2]==look){
										if((inRange(rects[a],rects[i]-1,rects[i+1]+1)||inRange(rects[i],rects[a]-1,rects[a+1]+1))){
											foundit=true;
											rects[i]=std::min(rects[i],rects[a]);
											rects[i+1]=std::max(rects[i+1],rects[a+1]);
											rects[i+2]=std::min(rects[i+2],rects[a+2]);
											rects[i+3]=std::max(rects[i+3],rects[a+3]);
											rects.erase(rects.begin()+a,rects.begin()+a+4);
										}
									}
								}
							}while(foundit);
						}
					}
				}
				if((time(NULL)-lasttime)>=1){
					lasttime=time(NULL);
					progress->maximum(rects.size());
					progress->value(i);
					snprintf(txtbuf,1024,"Rectanges: %d",rects.size());
					progress->label(txtbuf);
					Fl::check();
				}
			}
			}while(!canEnd);
			pass=0;
			do{
				canEnd=true;
				snprintf(txtbufstage,1024,"Stage 2 pass %d",pass++);
				winP->label(txtbufstage);
				progress->maximum(rects.size());
				progress->value(0);
				Fl::check();
				for(int i=0;i<rects.size();i+=4){
					for(int j=0;j<rects.size();j+=4){
						//printf("%d %d\n",i,j);
						if(i==j)
							continue;
						//Merdge overlapping rectangles
						if((rects[i]<=rects[j+1])&&(rects[i+1]>=rects[j])&&(rects[i+2]<=rects[j+3])&&(rects[i+3]>=rects[j+2])){
							canEnd=false;
							rects[i]=std::min(rects[i],rects[j]);
							rects[i+1]=std::max(rects[i+1],rects[j+1]);
							rects[i+2]=std::min(rects[i+2],rects[j+2]);
							rects[i+3]=std::max(rects[i+3],rects[j+3]);
							rects.erase(rects.begin()+j,rects.begin()+j+4);
						}
						//Merdge touching rectanges
						if(abs(rects[i+1]-rects[j])==1){
							if((inRange(rects[j+2],rects[i+2]-1,rects[i+3]+1)||inRange(rects[i+2],rects[j+2]-1,rects[j+3]+1))){
								canEnd=false;
								rects[i]=std::min(rects[i],rects[j]);
								rects[i+1]=std::max(rects[i+1],rects[j+1]);
								rects[i+2]=std::min(rects[i+2],rects[j+2]);
								rects[i+3]=std::max(rects[i+3],rects[j+3]);
								rects.erase(rects.begin()+j,rects.begin()+j+4);
							}
						}
					}
					if((time(NULL)-lasttime)>=1){
						lasttime=time(NULL);
						progress->maximum(rects.size());
						progress->value(i);
						snprintf(txtbuf,1024,"Rectanges: %d",rects.size());
						progress->label(txtbuf);
						Fl::check();
					}
				}
			}while(!canEnd);
			winP->remove(progress);// remove progress bar from window
			delete(progress);// deallocate it
			delete winP;
			std::vector<bool> deleted;
			deleted.resize(rects.size()/4);
			//Now show the window allowing user to adjust sprite settings
			win=new Fl_Double_Window(640,480,"Sprite selection");
			win->begin();
			win->resizable(win);
			Fl_Button * Ok=new Fl_Button(256,448,64,24,"Okay");
			Ok->callback(RetCB,(void*)1);
			Fl_Button * Cancel=new Fl_Button(320,448,64,24,"Cancel");
			Cancel->callback(RetCB,0);
			Fl_Scroll*scroll=new Fl_Scroll(8,8,624,440);
			box=new RectBox(8,8,w,h);
			box->scroll=scroll;
			box->rects=&rects;
			box->deleted=&deleted;
			box->image(loaded_image);
			scroll->end();
			win->end();
			win->set_modal();
			win->show();
			Fl::check();

			while(win->shown())
				Fl::wait();
			delete win;
			if(retOkay){
				for(unsigned i=0;i<rects.size();i+=4){
					recttoSprite(rects[i],rects[i+1],rects[i+2],rects[i+3],-1,loaded_image,grayscale,remap,palMap,mask,true,useAlpha);
				}
				updateTileSelectAmt();
			}
			deleted.clear();
			rects.clear();
		}
		loaded_image->release();
	}
}
extern const char*rtVersionStr;
void sprites::exportMapping(gameType_t game){
	if(load_file_generic("Save mapping to:",true)){
		FILE*fp;
		if(game==tSonic1){
			fp=fopen(the_file.c_str(),"w");
			fprintf(fp,";Sprite mapping generated by %s\n%s:\n",rtVersionStr,name.c_str());
			for(uint32_t i=0;i<amt;++i)
				fprintf(fp,"\tdc.w %s-%s\n",groups[i].name.c_str(),name.c_str());
			for(uint32_t j=0;j<amt;++j){
				if(groups[j].list.size()){
					fprintf(fp,"%s:\n\tdc.b %d\n",groups[j].name.c_str(),(int)groups[j].list.size());
					for(uint32_t i=0;i<groups[j].list.size();++i)
						fprintf(fp,"\tdc.b %d,%u,%u,%u,%d\n",groups[j].offy[i],((groups[j].list[i].w-1)<<2)|(groups[j].list[i].h-1),(groups[j].list[i].prio<<7)|(groups[j].list[i].palrow<<5)|(groups[j].list[i].vflip<<4)|(groups[j].list[i].hflip<<3)|((groups[j].loadat[i]>>8)&7),groups[j].loadat[i]&255,groups[j].offx[i]);
				}else
					fprintf(fp,"%s:\n\tdc.b 0\n",groups[j].name.c_str());
			}
			fputs("\teven\n",fp);
		}else{
			fp=fopen(the_file.c_str(),"wb");
			unsigned offtmp=amt*2;
			for(unsigned i=0;i<amt;++i){
				uint16_t offb16=htobe16(offtmp);
				fwrite(&offb16,1,2,fp);
				offtmp+=groups[i].list.size()*8;//Each entry takes four bytes;
				offtmp+=2;//How many takes two bytes
			}
			for(unsigned i=0;i<amt;++i){
				uint16_t amtb=htobe16(groups[i].list.size());
				fwrite(&amtb,1,2,fp);
				for(unsigned j=0;j<groups[i].list.size();++j){
					int8_t off=groups[i].offy[j];
					fwrite(&off,1,1,fp);
					uint8_t tmp=((groups[i].list[j].w-1)<<2)|(groups[i].list[j].h-1);
					fwrite(&tmp,1,1,fp);
					unsigned tile=groups[i].loadat[j];
					if(tile>2047){
						printf("Tile overflow was %d\n",tile);
						tile=2047;
					}
					tmp=(groups[i].list[j].prio<<7)|((groups[i].list[j].palrow&3)<<5)|(groups[i].list[j].vflip<<4)|(groups[i].list[j].hflip<<3)|(tile>>8);
					fwrite(&tmp,1,1,fp);
					tmp=tile&255;
					fwrite(&tmp,1,1,fp);
					tile/=2;
					tmp=(groups[i].list[j].prio<<7)|((groups[i].list[j].palrow&3)<<5)|(groups[i].list[j].vflip<<4)|(groups[i].list[j].hflip<<3)|(tile>>8);
					fwrite(&tmp,1,1,fp);
					tmp=tile&255;
					fwrite(&tmp,1,1,fp);
					int8_t tmpi=groups[i].offx[j]>>8;
					fwrite(&tmpi,1,1,fp);
					tmpi=groups[i].offx[j]&255;
					fwrite(&tmpi,1,1,fp);
				}
			}
		}
		fclose(fp);
	}
}
static char*skipComment(char*txt){
	while((*txt!='\n')&&(*txt!=0))
		++txt;
	return txt;
}
static char*readNbytesAsm(uint8_t*buf,char*txt,unsigned n){
	for(unsigned j=0;j<n;++j){
anotherTry:
		while((!isdigit(*txt))&&((*txt)!='$')&&((*txt)!='-')&&((*txt)!=';'))
			++txt;
		long tmp;
		if(*txt==';'){
			txt=skipComment(txt);//Some sprite mapping assembly data may contain comments with numbers skip them
			goto anotherTry;
		}
		if(*txt=='$')
			tmp=strtol(txt+1,&txt,16);
		else
			tmp=strtol(txt,&txt,0);
		buf[j]=tmp&255;
	}
	return txt;
}
void sprites::mappingItem(void*in,uint32_t id,gameType_t game){
	if(!in)
		return;
	if(game==tSonic1){
		char*txt=(char*)in;
		if(txt=strstr(txt,"dc.b")){
			uint32_t amtgroup;
			txt+=strlen("dc.b");
			while(isspace(*txt++));
			--txt;
			if(*txt=='$')
				amtgroup=strtol(txt+1,&txt,16);
			else
				amtgroup=strtol(txt,&txt,0);
			setAmtingroup(id,amtgroup);
			for(uint32_t i=0;i<amtgroup;++i){
				uint8_t buf[5];
				txt=readNbytesAsm(buf,txt,5);
				//Now convert sprite format
				/*From sonic retro wiki
				 * Each mapping is 5 bytes long, taking the form TTTT TTTT 0000 WWHH PCCY XAAA AAAA AAAA LLLL LLLL.
				 *
				 * LLLL LLLL is the left co-ordinate of where the mapping appears.
				 * TTTT TTTT is the top co-ordinate of where the mapping appears.
				 * WW is the width of the mapping, in tiles minus one. So 0 means 8 pixels wide, 1 means 16 pixels wide, 2 means 24 pixels wide and 3 means 32 pixels wide.
				 * HH is the height of the mapping, in the same format as the width.
				 * P is the priority-flag. If P is set, the mapping will appear above everything else.
				 * CC is the palette line.
				 * X is the x-flip-flag. If X is set, the mapping will be flipped horizontally.
				 * Y is the y-flip-flag. If Y is set, the mapping will be flipped vertically.
				 * AAA AAAA AAAA is the tile index. */
				int8_t*bufi=(int8_t*)buf;
				groups[id].offy[i]=bufi[0];
				groups[id].list[i].w=((buf[1]>>2)&3)+1;
				groups[id].list[i].h=(buf[1]&3)+1;
				groups[id].list[i].prio=(buf[2]&(1<<7))>>7;
				groups[id].list[i].palrow=(buf[2]&(3<<5))>>5;
				groups[id].list[i].vflip=(buf[2]&(1<<4))>>4;
				groups[id].list[i].hflip=(buf[2]&(1<<3))>>3;
				uint16_t tile=(buf[2]&7)<<8;
				tile|=buf[3];
				groups[id].list[i].starttile=tile;
				groups[id].loadat[i]=tile;
				groups[id].offx[i]=bufi[4];
			}
		}else
			return;
	}else{
		/* From the sonic Retro Wiki
		 * First word:
		 * High byte is the relative signed top edge position of the sprite from the center of the object.
		 * Low byte is the size of the sprite, in tiles minus one.
		 * The upper four bits are ignored, the next two bits control the width and the lowest two bits control the height.
		 * Thus sprites can be of any size from 1x1 tile to 4x4 tiles. For example, $01 is a 1x2 sprite, $02 is a 1x3 sprite, $04 is a 2x1 sprite, and so on. 
		 * Second and third words:
		 * The second word applies to one-player mode; the third applies to two-player mode.
		 * The relevant word will be added to the object's VRAM offset and then used as the pattern index for that sprite.
		 * Like all SEGA Genesis VDP pattern indices, it is a bitmask of the form PCCY XAAA AAAA AAAA.
		 * P is the priority flag,
		 * CC is the palette line to use,
		 * X and Y indicate that the sprite should be flipped horizontally and vertically respectively and
		 * AAA AAAA AAAA is the actual tile index, i.e. the VRAM offset of the pattern divided by $20 (or bit-shifted right by 5). 
		 * Fourth word: This is the relative signed left edge position of the sprite from the center of the object. */
		uint16_t*buf=(uint16_t*)in;
		unsigned amtgroup=be16toh(*buf++);
		setAmtingroup(id,amtgroup);
		for(unsigned i=0;i<amtgroup;++i){
			int8_t*bufi=(int8_t*)buf;
			groups[id].offy[i]=*bufi++;
			uint8_t*bufu=(uint8_t*)bufi;
			groups[id].list[i].w=((*bufu>>2)&3)+1;
			groups[id].list[i].h=((*bufu++)&3)+1;
			buf=(uint16_t*)bufu;
			uint16_t tmp=be16toh(*buf++);
			++buf;//Skip two player data
			groups[id].list[i].starttile=tmp&2047;
			groups[id].loadat[i]=tmp&2047;
			groups[id].list[i].prio=(tmp>>15)&1;
			groups[id].list[i].palrow=(tmp>>13)&3;
			groups[id].list[i].vflip=(tmp>>12)&1;
			groups[id].list[i].hflip=(tmp>>11)&1;
			bufi=(int8_t*)buf;
			groups[id].offx[i]=(bufi[0]<<8)|bufi[1];
			++buf;
		}
	}
}
void sprites::DplcItem(void*in,uint32_t which,gameType_t game){
	/*Sonic 1 format:
	 * uint8_t amount
	 * for each of amount
	 * uint8_t how many and offset high byte
	 * The high nybles sigifies how many tiles to load - 1
	 * uint8_t offset low byte
	 * */
	//Just ignore the the high nyble on the high byte to get start tile
	if(game==tSonic1){
		char*txt=(char*)in;
		unsigned amtd;
		if(txt=strstr(txt,"dc.b")){
			txt+=strlen("dc.b");
			while(isspace(*txt++));
			--txt;
			if(*txt=='$')
				amtd=strtol(txt+1,&txt,16);
			else
				amtd=strtol(txt,&txt,0);
			for(uint32_t i=0;i<amtd;++i){
				if(i>=groups[which].list.size())//Avoid writting to nonexistent sprites
					break;
				//Read the two bytes
				uint8_t buf[2];
				txt=readNbytesAsm(buf,txt,2);
				uint16_t tile=((buf[0]&15)<<8)|buf[1];
				groups[which].list[i].starttile=tile;
			}
		}
	}else{
		//The format is pretty much the same as sonic 1 except amount is now a word instead of a byte
		uint16_t*buf=(uint16_t*)in;
		unsigned amtd=be16toh(*buf++);
		for(unsigned i=0;i<std::max(amtd,(unsigned)groups[which].list.size());++i){
			if(i>=groups[which].list.size())//Avoid writting to nonexistent sprites
				break;
			if(i>=amtd){
				//Resort to guessing	
				//Try finding one with same width and height
				unsigned j;
				for(j=0;j<groups[which].list.size();++j){
					if(i==j)
						continue;
					if(groups[which].list[i].w==groups[which].list[j].w){
						if(groups[which].list[i].h==groups[which].list[j].h)
							break;
					}
				}
				if(j>=groups[which].list.size())
					j=groups[which].list.size()-1;
				groups[which].list[i].starttile=groups[which].list[j].starttile;
			}else{
				unsigned tile=be16toh(*buf++)&4095;
				groups[which].list[i].starttile=tile;
			}
		}
	}
}
bool sprites::alreadyLoaded(uint32_t id,uint32_t subid){
	if(!subid)
		return false;
	for(uint32_t i=0;i<subid;++i){
		if(i==subid)
			continue;
		if(groups[id].list[subid].w==groups[id].list[i].w){
			if(groups[id].list[subid].h==groups[id].list[i].h){
				if(groups[id].list[subid].starttile==groups[id].list[i].starttile)
					return true;
			}
		}
	}
	return false;
}
bool sprites::checkDupdplc(uint32_t id,uint32_t&which){
	if(!id)
		return false;
	for(uint32_t i=0;i<id;++i){//We only search before id in question so later dplc enteries will refer to dplc enteries before
		if(groups[i].list.size()==groups[id].list.size()){
			bool match=true;
			for(uint32_t j=0;j<groups[i].list.size();++j){
				if(groups[i].list[j].starttile!=groups[id].list[j].starttile){
					match=false;
					break;
				}
				if(groups[i].list[j].h!=groups[id].list[j].h){
					match=false;
					break;
				}
				if(groups[i].list[j].w!=groups[id].list[j].w){
					match=false;
					break;
				}
			}
			if(match){
				which=i;
				return true;
			}
		}
	}
	return false;
}
void sprites::exportDPLC(gameType_t game){
	if(load_file_generic("Save DPLC",true)){
		FILE*fp;
		if(game==tSonic1){
			fp=fopen(the_file.c_str(),"w");
			fprintf(fp,";DPLC generated by %s\n%s_DPLC:",rtVersionStr,name.c_str());
			uint32_t dup;
			for(unsigned i=0;i<amt;++i){
				if(checkDupdplc(i,dup)){
					fprintf(fp,"\tdc.w %s-%s\n",groups[dup].name.c_str(),name.c_str());
					printf("Table entry can be optimzed: %d %d\n",i,dup);
				}else
					fprintf(fp,"\tdc.w %s-%s\n",groups[i].name.c_str(),name.c_str());
			}
			for(unsigned i=0;i<amt;++i){
				if(!checkDupdplc(i,dup)){
					fprintf(fp,"%s:\n\tdc.b %d\n",groups[i].name.c_str(),groups[i].list.size());
					for(unsigned j=0;j<groups[i].list.size();++j){
						if(!alreadyLoaded(i,j))
							fprintf(fp,"\tdc.b %d,%d\n",(((groups[i].list[j].w*groups[i].list[j].h)-1)<<4)|((groups[i].list[j].starttile&2047)>>8),groups[i].list[j].starttile&255);
						else
							printf("Already loaded group: %d sprite: %d\n",i,j);
					}
				}
			}
		}else{
			fp=fopen(the_file.c_str(),"wb");
			std::vector<uint8_t> tmpbuf;
			unsigned acum=amt*2;
			tmpbuf.resize(amt*2);
			for(unsigned i=0;i<amt;++i){
				uint32_t dup;
				if(checkDupdplc(i,dup)){
					printf("Table entry can be optimzed: %d %d\n",i,dup);
					tmpbuf[i*2]=tmpbuf[dup*2];
					tmpbuf[i*2+1]=tmpbuf[dup*2+1];
				}else{
					tmpbuf[i*2]=acum>>8;
					tmpbuf[i*2+1]=acum&255;
					unsigned amtg=0;
					for(unsigned j=0;j<groups[i].list.size();++j){
						if(!alreadyLoaded(i,j))
							++amtg;
					}
					tmpbuf.push_back(amtg>>8);
					tmpbuf.push_back(amtg&255);
					acum+=2;
					for(unsigned j=0;j<groups[i].list.size();++j){
						if(!alreadyLoaded(i,j)){
							tmpbuf.push_back((((groups[i].list[j].w*groups[i].list[j].h)-1)<<4)|((groups[i].list[j].starttile&4095)>>8));
							tmpbuf.push_back(groups[i].list[j].starttile&255);
							acum+=2;
						}else
							printf("Already loaded group: %d sprite: %d\n",i,j);
					}
				}
			}
			fwrite(tmpbuf.data(),1,tmpbuf.size(),fp);
			tmpbuf.clear();
		}
		fclose(fp);
	}
}
void sprites::importDPLC(gameType_t game){
	if(load_file_generic("Load DPLC")){
		FILE*fp;
		if(game==tSonic1)
			fp=fopen(the_file.c_str(),"r");
		else
			fp=fopen(the_file.c_str(),"rb");
		fseek(fp,0,SEEK_END);
		size_t sz=ftell(fp);
		rewind(fp);
		char*buf=(char*)malloc(sz+1);
		fread(buf,1,sz,fp);
		if(game==tSonic1)
			buf[sz]=0;//Ensure that the C-string is null terminated
		fclose(fp);
		uint32_t sp=0;
		if(game==tSonic1){
			char*bufp=buf;
			char*bufend=buf+sz-1;
			while(bufp<bufend){
				if(bufp=strstr(bufp,"dc.w")){
					bufp+=strlen("dc.w");
					while(isspace(*bufp++));
					--bufp;
					char*minus=strstr(bufp,"-");
					if(!minus)
						break;
					*minus=0;
					DplcItem(strstr(minus+1,bufp)+strlen(bufp),sp++,game);
					bufp=minus+1;
				}else
					break;
			}
		}else{
			uint16_t*ptr=(uint16_t*)buf;
			unsigned amtd=be16toh(*ptr)/2;
			for(uint32_t i=0;i<amtd;++i){
				unsigned off=be16toh(ptr[i])/2;
				DplcItem(ptr+off,i,game);
			}
		}
		free(buf);
	}
}
void sprites::importMapping(gameType_t game){
	bool append=fl_choice("Append new sprites or overwrite","Overwrite","Append",0);
	if(load_file_generic("Load sprite mapping")){
		unsigned amtnew;
		if(append)
			amtnew=amt;
		else
			amtnew=0;
		FILE*fp;
		if(game==tSonic1)
			fp=fopen(the_file.c_str(),"r");
		else
			fp=fopen(the_file.c_str(),"rb");
		fseek(fp,0,SEEK_END);
		size_t sz=ftell(fp);
		rewind(fp);
		char*buf=(char*)malloc(sz+1);
		fread(buf,1,sz,fp);
		if(game==tSonic1)
			buf[sz]=0;//Ensure that the C-string is null terminated
		fclose(fp);
		char*bufp=buf;
		char*bufend=buf+sz-1;
		if(game==tSonic1){
			while(bufp<bufend){
				if(bufp=strstr(bufp,"dc.w")){
					bufp+=strlen("dc.w");
					while(isspace(*bufp++));
					--bufp;
					char*minus=strstr(bufp,"-");
					if(!minus)
						break;
					*minus=0;
					++amtnew;
					setAmt(amtnew);
					mappingItem(strstr(minus+1,bufp)+strlen(bufp),amt-1,game);
					groups[amtnew-1].name.assign(bufp);
					bufp=minus+1;
					//The dc.w psuedo-op can contain multiple words
					char*end=bufp;
					while((isalnum(*end))||(*end=='_'))
						++end;
					char endold=*end;
					*end=0;
					name.assign(bufp);
					*end=endold;
					bufp=end;
					while(1){
						char*comma=strstr(bufp,",");
						if(!comma)
							break;
						char*nl=strstr(bufp,"\n");//New Line
						if(!nl)
							break;
						if(nl&&(comma<nl)){
							//Label on same line found
							bufp=comma+1;
							while(isspace(*bufp++));
							--bufp;
							minus=strstr(bufp,"-");
							*minus=0;
							++amtnew;
							setAmt(amtnew);
							mappingItem(strstr(minus+1,bufp)+strlen(bufp),amt-1,game);
							groups[amtnew-1].name.assign(bufp);
							bufp=minus+1;
							comma=strstr(bufp,",");
							if((!comma)||(comma>nl))//Is the next comma on a newline?
								break;
						}else
							break;
					}
				}else
					break;
			}
		}else{
			//Sonic 2's mapping is usally stored in binary
			name.assign(fl_filename_name(the_file.c_str()));
			uint16_t*ptr=(uint16_t*)buf;
			unsigned off=be16toh(*ptr)/2;
			setAmt(off);//First pointer can be used to find amount just divide by two
			for(uint32_t i=0;i<amt;++i){
				unsigned off=be16toh(ptr[i])/2;
				mappingItem(ptr+off,i,game);
				groups[i].name.assign(fl_filename_name(the_file.c_str()));
				char tmp[16];
				snprintf(tmp,16,"_%u",i);
				groups[i].name.append(tmp);
			}

		}
		free(buf);
	}
}
static uint8_t*rect2rect(uint8_t*in,uint8_t*out,unsigned xin,unsigned yin,unsigned win,unsigned wout,unsigned hout,unsigned depth,bool reverse=false){
	in+=(yin*win*depth)+(xin*depth);
	while(hout--){
		if(depth==4){
			if(reverse)
				memcpy(in,out,wout*4);
			else
				memcpy(out,in,wout*4);
			in+=win*4;
			out+=wout*4;
		}else if(depth==3){
			if(reverse){
				for(unsigned i=0;i<wout;++i){
					*in++=*out++;
					*in++=*out++;
					*in++=*out++;
					++out;
				}
			}else{
				for(unsigned i=0;i<wout;++i){
					*out++=*in++;
					*out++=*in++;
					*out++=*in++;
					*out++=255;
				}
			}
			in+=(win-wout)*3;
		}
	}
	return out;
}
void sprites::spriteGroupToImage(uint8_t*img,uint32_t id,int row,bool alpha){
	int32_t miny,maxy,minx,maxx;
	minmaxoffy(id,miny,maxy);
	minmaxoffx(id,minx,maxx);
	uint32_t w=abs(maxx-minx);
	uint32_t h=abs(maxy-miny);
	unsigned bpp;//Bytes per pixel
	if(alpha)
		bpp=4;
	else
		bpp=3;
	memset(img,0,w*h*bpp);
	for(uint32_t i=0;i<groups[id].offx.size();++i){
		int32_t xoff=groups[id].offx[i];
		int32_t yoff=groups[id].offy[i];
		xoff-=minx;
		yoff-=miny;
		uint32_t ttile=groups[id].list[i].starttile;
		if((row!=groups[id].list[i].palrow)&&(row>=0))
			continue;//Skip if we only want a specific row
		for(uint32_t x=0;x<groups[id].list[i].w*currentProject->tileC->sizew;x+=currentProject->tileC->sizew){
			for(uint32_t y=0;y<groups[id].list[i].h*currentProject->tileC->sizeh;y+=currentProject->tileC->sizeh,++ttile){
				uint8_t*outptr=currentProject->tileC->truetDat.data()+(ttile*currentProject->tileC->tcSize);
				rect2rect(img,outptr,xoff+x,yoff+y,w,currentProject->tileC->sizew,currentProject->tileC->sizeh,alpha?4:3,true);
			}
		}
	}
}
void sprites::spriteImageToTiles(uint8_t*img,uint32_t id,int rowUsage,bool alpha){
	int32_t miny,maxy,minx,maxx;
	minmaxoffy(id,miny,maxy);
	minmaxoffx(id,minx,maxx);
	uint8_t tcTemp[256];
	uint32_t w=abs(maxx-minx);
	uint32_t h=abs(maxy-miny);
	for(uint32_t i=0;i<groups[id].offx.size();++i){
		int32_t xoff=groups[id].offx[i];
		int32_t yoff=groups[id].offy[i];
		xoff-=minx;
		yoff-=miny;
		uint32_t ttile=groups[id].list[i].starttile;
		if((rowUsage!=groups[id].list[i].palrow)&&(rowUsage>=0))
			continue;//Skip if we only want a specific row
		for(uint32_t x=0;x<groups[id].list[i].w*currentProject->tileC->sizew;x+=currentProject->tileC->sizew){
			for(uint32_t y=0;y<groups[id].list[i].h*currentProject->tileC->sizeh;y+=currentProject->tileC->sizeh,++ttile){
				rect2rect(img,tcTemp,xoff+x,yoff+y,w,currentProject->tileC->sizew,currentProject->tileC->sizeh,alpha?4:3,false);
				currentProject->tileC->truecolor_to_tile_ptr(groups[id].list[i].palrow,ttile,tcTemp,false,true);
			}
		}
	}
}
void sprites::minmaxoffy(uint32_t id,int32_t&miny,int32_t&maxy){
	miny=maxy=groups[id].offy[0];
	for(uint32_t i=0;i<groups[id].offy.size();++i){
		if(groups[id].offy[i]<miny)
			miny=groups[id].offy[i];
		int32_t tmpy=groups[id].offy[i]+(groups[id].list[i].h*currentProject->tileC->sizeh);
		if(tmpy>maxy)
			maxy=tmpy;
	}
}
void sprites::minmaxoffx(uint32_t id,int32_t&minx,int32_t&maxx){
	minx=maxx=groups[id].offx[0];
	for(uint32_t i=0;i<groups[id].offx.size();++i){
		if(groups[id].offx[i]<minx)
			minx=groups[id].offx[i];
		int32_t tmpx=groups[id].offx[i]+(groups[id].list[i].w*currentProject->tileC->sizew);
		if(tmpx>maxx)
			maxx=tmpx;
	}
}
uint32_t sprites::width(uint32_t id){
	int32_t minx,maxx;
	minmaxoffx(id,minx,maxx);
	return abs(maxx-minx);
}
uint32_t sprites::height(uint32_t id){
	int32_t miny,maxy;
	minmaxoffy(id,miny,maxy);
	return abs(maxy-miny);
}
void sprites::draw(uint32_t id,uint32_t x,uint32_t y,int32_t zoom,bool mode,int32_t*outx,int32_t*outy){
	if(groups[id].list.size()){
		int32_t minx,maxx;
		int32_t miny,maxy;
		if(!mode){
			minmaxoffx(id,minx,maxx);
			minmaxoffy(id,miny,maxy);
		}
		maxx=maxy=0;
		for(uint32_t i=0;i<groups[id].list.size();++i){
			int xoff=x+(groups[id].offx[i]*zoom);
			if(!mode)
				xoff-=minx*zoom;
			int yoff=y+(groups[id].offy[i]*zoom);
			if(!mode)
				yoff-=miny*zoom;
			if(maxx<(xoff+(groups[id].list[i].w*zoom*8)))
				maxx=xoff+(groups[id].list[i].w*zoom*8);
			if(maxy<(yoff+(groups[id].list[i].h*zoom*8)))
				maxy=yoff+(groups[id].list[i].h*zoom*8);
			if(xoff>=window->w())
				continue;
			if(yoff>=window->h())
				continue;
			groups[id].list[i].draw(xoff,yoff,zoom);
		}
		if(outx)
			*outx=maxx;
		if(outy)
			*outy=maxy;
	}
}
void sprites::setAmt(uint32_t amtnew){
	if(amtnew>amt){
		//Create more sprites with default paramater
		groups.resize(amtnew);
		for(unsigned n=amt;n<amtnew;++n){
			groups[n].list.push_back(sprite());
			groups[n].name.assign(spriteDefName);
			groups[n].offx.push_back(0);
			groups[n].offy.push_back(0);
			groups[n].loadat.push_back(0);
		}
	}else if(amtnew<amt){
		for(unsigned n=amtnew;n<amt;++n){
			groups[n].offx.clear();
			groups[n].offy.clear();
			groups[n].loadat.clear();
			groups[n].name.clear();
			groups[n].list.clear();
		}
		groups.resize(amtnew);
	}
	amt=amtnew;
}
void sprites::setAmtingroup(uint32_t id,uint32_t amtnew){
	uint32_t amtold=groups[id].list.size();
	if(amtold==amtnew)
		return;
	groups[id].list.resize(amtnew);
	groups[id].offx.resize(amtnew);
	groups[id].offy.resize(amtnew);
	groups[id].loadat.resize(amtnew);
	if(amtnew>amtold){
		for(unsigned i=amtold;i<amtnew;++i)
			groups[id].name.assign(spriteDefName);
	}
}
bool sprites::save(FILE*fp){
	/* Format:
	 * uint32_t group amount
	 * Null terminated string containing name (all sprites)
	 * for each group
	 * uint32_t sprite amount
	 * Null terminated sprite group name or just 0 if default name
	 * for each sprite in group
	 * int32_t offset x
	 * int32_t offset y
	 * uint32_t loadat
	 * uint32_t w
	 * uint32_t h
	 * uint32_t starttile
	 * uint32_t pal row
	 * uint8_t hvflip flags bit 0 hflip bit 1 vflip bit 2 priority
	 */
	fwrite(&amt,sizeof(uint32_t),1,fp);
	if(strcmp(name.c_str(),spritesName))
		fputs(name.c_str(),fp);
	fputc(0,fp);
	for(unsigned n=0;n<amt;++n){
		uint32_t amtgroup=groups[n].list.size();
		fwrite(&amtgroup,sizeof(uint32_t),1,fp);
		if(strcmp(groups[n].name.c_str(),spriteDefName))
			fputs(groups[n].name.c_str(),fp);
		fputc(0,fp);
		for(uint32_t i=0;i<amtgroup;++i){
			fwrite(&groups[n].offx[i],sizeof(int32_t),1,fp);
			fwrite(&groups[n].offy[i],sizeof(int32_t),1,fp);
			fwrite(&groups[n].loadat[i],sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].w,sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].h,sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].starttile,sizeof(uint32_t),1,fp);
			fwrite(&groups[n].list[i].palrow,sizeof(uint32_t),1,fp);
			uint8_t hvflip=groups[n].list[i].hflip|(groups[n].list[i].vflip<<1)|(groups[n].list[i].prio<<2);
			fwrite(&hvflip,sizeof(uint8_t),1,fp);
		}
	}
	return true;
}
bool sprites::load(FILE*fp,uint32_t version){
	if(version>=7){
		uint32_t amtnew;
		fread(&amtnew,sizeof(uint32_t),1,fp);
		setAmt(amtnew);
		char a=fgetc(fp);
		if(a){
			name.clear();
			do{
				name.push_back(a);
			}while(a=fgetc(fp));
		}
		window->spriteglobaltxt->value(name.c_str());
		for(unsigned n=0;n<amt;++n){
			uint32_t amtgroup;
			fread(&amtgroup,sizeof(int32_t),1,fp);
			setAmtingroup(n,amtgroup);
			char a=fgetc(fp);
			if(a){
				groups[n].name.clear();
				do{
					groups[n].name.push_back(a);
				}while(a=fgetc(fp));
			}
			for(uint32_t i=0;i<amtgroup;++i){
				fread(&groups[n].offx[i],sizeof(int32_t),1,fp);
				fread(&groups[n].offy[i],sizeof(int32_t),1,fp);
				fread(&groups[n].loadat[i],sizeof(uint32_t),1,fp);
				fread(&groups[n].list[i].w,sizeof(uint32_t),1,fp);
				fread(&groups[n].list[i].h,sizeof(uint32_t),1,fp);
				fread(&groups[n].list[i].starttile,sizeof(uint32_t),1,fp);
				fread(&groups[n].list[i].palrow,sizeof(uint32_t),1,fp);
				uint8_t hvflip;
				fread(&hvflip,sizeof(uint8_t),1,fp);
				groups[n].list[i].hflip=hvflip&1;
				groups[n].list[i].vflip=(hvflip&2)>>1;
				groups[n].list[i].prio=(hvflip&4)>>2;
			}
		}
	}else{
		/* Old format
		 * uint32_t amount
		 * And for each sprite:
		 * uint32_t width
		 * uint32_t height
		 * uint32_t start tile
		 * uint32_t pal row*/
		uint32_t amtnew;
		fread(&amtnew,sizeof(uint32_t),1,fp);
		setAmt(amtnew);
		for(unsigned n=0;n<amt;++n){
			fread(&groups[n].list[0].w,sizeof(uint32_t),1,fp);
			fread(&groups[n].list[0].h,sizeof(uint32_t),1,fp);
			fread(&groups[n].list[0].starttile,sizeof(uint32_t),1,fp);
			fread(&groups[n].list[0].palrow,sizeof(uint32_t),1,fp);
		}
	}
	return true;
}
void sprites::importImg(uint32_t to){
	if(load_file_generic("Load image")){
		Fl_Shared_Image * loaded_image=Fl_Shared_Image::get(the_file.c_str());
		if(!loaded_image){
			fl_alert("Error loading image");
			return;
		}
		unsigned depth=loaded_image->d();
		if (unlikely(depth != 3 && depth != 4 && depth!=1)){
			fl_alert("Please use color depth of 1,3 or 4\nYou Used %d",loaded_image->d());
			loaded_image->release();
			return;
		}else
			printf("Image depth %d\n",loaded_image->d());
		uint8_t * img_ptr=(uint8_t *)loaded_image->data()[0];
		groups[to].name.assign(fl_filename_name(the_file.c_str()));
		if(depth==1)
			img_ptr=(uint8_t*)loaded_image->data()[2];
		bool grayscale;
		uint8_t*palMap;
		unsigned remap[256];
		if(depth==1){
			grayscale=handle1byteImg(loaded_image,remap);
			if(!grayscale){
				palMap=(uint8_t*)loaded_image->data()[1];
				img_ptr=(uint8_t*)loaded_image->data()[2];
			}
		}
		bool useMask=fl_ask("Use mask color?");
		uint8_t mask[3];
		bool useAlpha;
		if(useMask){
			if(!getMaskColorImg(loaded_image,grayscale,remap,palMap,mask,useAlpha)){
				loaded_image->release();
				return;
			}
		}
		recttoSprite(0,loaded_image->w()-1,0,loaded_image->h()-1,to,loaded_image,grayscale,remap,palMap,mask,useMask,useAlpha);
		loaded_image->release();
		window->updateSpriteSliders();
		updateTileSelectAmt();
		window->redraw();
	}
}
void sprites::del(uint32_t id){
	if(amt<=1){
		fl_alert("If you want no sprites uncheck have sprites instead.");
		return;
	}
	if(id<amt){
		groups[id].offx.clear();
		groups[id].offy.clear();
		groups[id].loadat.clear();
		groups[id].name.clear();
		groups[id].list.clear();
		groups.erase(groups.begin()+id);
		--amt;
	}else
		fl_alert("You cannot delete what does not exist");
}
void sprites::delingroup(uint32_t id,uint32_t subid){
	uint32_t amtold=groups[id].list.size();
	if(amtold<=0){
		fl_alert("Delete the entire group instead if that is what you want");
		return;
	}
	if(subid<amtold){
		groups[id].offx.erase(groups[id].offx.begin()+subid);
		groups[id].offy.erase(groups[id].offy.begin()+subid);
		groups[id].loadat.erase(groups[id].loadat.begin()+subid);
		groups[id].list.erase(groups[id].list.begin()+subid);
	}else
		fl_alert("You cannot delete what does not exist");
}
void sprites::enforceMax(unsigned wmax,unsigned hmax){
	for(unsigned j=0;j<amt;++j){
		for(unsigned i=0;i<groups[j].list.size();++i){
		if(groups[j].list[i].w>wmax)
			groups[j].list[i].w=wmax;
		if(groups[j].list[i].h>hmax)
			groups[j].list[i].h=hmax;
		}
	}
}
