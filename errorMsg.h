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
    Copyright Sega16 (or whatever you wish to call me (2012-2014)
*/
#pragma once
void malloc_error(int line,const char * file,const char * function,int bytes);
void realloc_error(int line,const char * file,const char * function,int bytes);
void default_trigger(int line,const char * file,const char * function);
#define show_malloc_error(bytes_error) malloc_error(__LINE__,__FILE__,__FUNCTION__,bytes_error);
#define show_realloc_error(bytes_error) realloc_error(__LINE__,__FILE__,__FUNCTION__,bytes_error);
#define show_default_error default_trigger(__LINE__,__FILE__,__FUNCTION__);
