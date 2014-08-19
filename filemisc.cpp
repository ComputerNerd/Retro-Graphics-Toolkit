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
#include "includes.h"
int clipboardAsk(void){
	return fl_choice("File or clipboard?","File","Clipboard","Cancel");
}
fileType_t askSaveType(void){
	return (fileType_t)MenuPopup("How would you like the file saved?","Set if the file is saved as binary C header bex or asm",4,"Binary","C header","Asm","BEX");
}
bool saveBinAsText(void * ptr,size_t sizeBin,FILE * fp,fileType_t type,const char*comment,const char*label,int bits){
	/*!
	This function saves binary data as plain text useful for c headers each byte is seperated by a comma
	To use the clipboard specify file as NULL
	Returns True on sucess false on error
	Type can be:
	1 - c header
	2 - asm
	3 - bex
	*/
	bits=8;
	uint8_t * dat8=(uint8_t *)ptr;
	uint16_t * dat16=(uint16_t *)ptr;
	uint32_t * dat32=(uint32_t *)ptr;
	char endc=',';
	std::string temp;
	unsigned mask=(bits/8)-1;
	char tmp[16];
	if(mask){
		if(sizeBin&mask){
			fl_alert("Error filetype unaligned to %d bits",bits);
			return false;
		}
		sizeBin/=mask+1;
	}
	switch(bits){
		case 8:
			mask=31;
		break;
		case 16:
			mask=15;
		break;
		case 32:
			mask=7;
		break;
	}
	if(comment){
		switch(type){
			case 1:
				temp.assign("// ");
			break;
			case 2:
				temp.assign("; ");
			break;
			case 3:
				temp.assign("' ");
			break;
		}
		temp.append(comment);
		temp.push_back('\n');
	}
	switch(type){
		case tCheader:
			temp.append("const uint");
			snprintf(tmp,16,"%d",bits);
			temp.append(tmp);
			temp.append("_t ");
			temp.append(label);
			temp.append("[]={");
		break;
		case tAsm:
		case tBex:
			temp.append(label);
			temp.push_back(':');
		break;
	}
	for (size_t x=0;x<sizeBin;++x){
		if ((x&mask)==0){
			temp.push_back('\n');
			switch(type){
				case 2:
					switch(bits){
						case 8:
							temp.append("\tdc.b ");
						break;
						case 16:
							temp.append("\tdc.w ");
						break;
						case 32:
							temp.append("\tdc.l ");
						break;
					}
				break;
				case 3:
					switch(bits){
						case 8:
							temp.append("\tdata ");
						break;
						case 16:
							temp.append("\tdataint ");
						break;
						case 32:
							temp.append("\tdatalong ");
						break;
					}
				break;
			}
		}
		if(((x&mask)==mask)&&(type!=1))
			endc=0;
		else
			endc=',';
		if(x==(sizeBin-1))
			endc='\n';
		switch(bits){
			case 8:
				snprintf(tmp,16,"%u",*dat8++);
			break;
			case 16:
				snprintf(tmp,16,"%u",*dat16++);
			break;
			case 32:
				snprintf(tmp,16,"%u",*dat32++);
			break;
		}
		temp.append(tmp);
		if(endc)
			temp.push_back(endc);
	}
	if(type==1)
		temp.append("};\n");
	if(fp)
		fputs(temp.c_str(),fp);
	else
		Fl::copy(temp.c_str(),temp.length(),1);
	return true;
}
