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
#include <stdio.h>
#include <stdint.h>
#include "gui.h"
#include "filemisc.h"
int askSaveType(void){
	return MenuPopup("How would you like the file saved?","Set if the file is saved as binary C header bex or asm",4,"Binary","C header","Asm","BEX");
}
bool saveBinAsText(void * ptr,size_t sizeBin,FILE * fp,int type,const char*comment,const char*label){
	/*!
	This function saves binary data as plain text useful for c headers each byte is seperated by a comma
	Returns True on sucess false on error
	Type can be:
	1 - c header
	2 - asm
	3 - bex
	*/
	uint8_t * dat=(uint8_t *)ptr;
	char endc=',';
	if(comment){
		switch(type){
			case 1:
				fputs("// ",fp);
			break;
			case 2:
				fputs("; ",fp);
			break;
			case 3:
				fputs("' ",fp);
			break;
		}
		fputs(comment,fp);
		fputc('\n',fp);
	}
	switch(type){
		case 1:
			fprintf(fp,"const uint8_t %s[]={\n",label);
		break;
		case 2:
		case 3:
			fputs(label,fp);
			fputs(":\n",fp);
		break;
	}
	for (size_t x=0;x<sizeBin;++x){
		if ((x&31)==0){
			if (fputc('\n',fp)==0)
				return false;
			switch(type){
				case 2:
					fputs("\tdc.b ",fp);
				break;
				case 3:
					fputs("\tdata ",fp);
				break;
			}
		}
		if(((x&31)==31)&&(type!=1))
			endc=0;
		else
			endc=',';
		if(x==(sizeBin-1))
			endc='\n';
		if(endc)
			fprintf(fp,"%d%c",*dat,endc);
		else
			fprintf(fp,"%d",*dat);
		++dat;
	}
	if(type==1)
		fputs("};\n",fp);
	return true;
}
