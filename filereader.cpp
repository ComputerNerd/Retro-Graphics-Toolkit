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
   Copyright Sega16 (or whatever you wish to call me) (2012-2015)
*/
#include <stdio.h>
#include <sys/stat.h>
#include <FL/fl_ask.H>
#include "filereader.h"
#include "filemisc.h"
#include "gui.h"
static const char*skipWS(const char*ptr){
	while(isspace(*ptr++));
	return ptr-1;
}
static const char*nextLine(const char*ptr){
	while(*ptr!='\n'&&*ptr!='\r')
		++ptr;
	return ptr;
}
filereader::filereader(const char*title){
	char*fname;
	if(title)
		fname=loadsavefile(title);
	else
		fname=loadsavefile();
	if(fname){
		fileType_t tp=askSaveType();
		if(tp==tCancle){
			amt=0;
			free(fname);
			return;
		}
		struct stat st;
		FILE*fp=fopen(fname,tp==tBinary?"rb":"r");
		free(fname);
		if(!fp){
			fl_alert("An error has occurred: %s",strerror(errno));
			amt=0;
			return;
		}
		if(fstat(fileno(fp),&st)!=0){
			fl_alert("An error has occurred: %s",strerror(errno));
			amt=0;
			fclose(fp);
			return;
		}
		if(tp==tBinary){
			lens.push_back(st.st_size);
			lenTotal=st.st_size;
			dat.push_back(std::vector<uint8_t>(lenTotal));
			fread(dat[0].data(),1,lenTotal,fp);
		}else{
			char*tmp=(char*)malloc(st.st_size);
			fread(tmp,1,st.st_size,fp);
			for(;;){
				//Find label
				unsigned cbits=0;
				const char*ptr;
				for(ptr=tmp;ptr<(tmp+st.st_size);){
					if(isspace(*ptr))
						++ptr;
					else{
						if(isalpha(*ptr)){
							if(tp==tCheader){
								static const char*keywords[]={"static","const","unsigned"};
								static const unsigned lens[]={sizeof("static")-1,sizeof("const")-1,sizeof("unsigned")-1};
								for(unsigned i=0;i<sizeof(keywords)/sizeof(keywords[0]);++i){
									if(!strncmp(ptr,keywords[i],lens[i])){
										ptr+=lens[i];
										continue;
									}
								}
								if(cbits){
									//Label found
									break;
								}
							}
						}
					}
				}
			}
			free(tmp);
		}
		fclose(fp);
	}else
		amt=0;
}
