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
#include "includes.h"
/*Some users may not be programmers and will not run a debugger these error messages should make it easier for them to convey a glitch to me
All they have to do is copy and paste the error message*/
void malloc_error(int line,const char * file,const char * function,int bytes){
	fl_alert("malloc error in file %s function %s line %d\nNumber of bytes attempted %d",file,function,line,bytes);
}
void realloc_error(int line,const char * file,const char * function,int bytes){
	fl_alert("realloc error in file %s function %s line %d\nNumber of bytes attempted %d",file,function,line,bytes);
}
void default_trigger(int line,const char * file,const char * function){
	/*!
	In a switch statment sometimes there should be no defualt action so this function gets called when the varible does not compare with any cases
	*/
	fl_alert("Default triggered in file %s function %s line %d",file,function,line);
}
