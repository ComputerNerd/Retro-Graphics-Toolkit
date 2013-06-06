/*-----------------------------------------------------------------------------*\
|																				|
|	libkosinski: Compression / Decompression of data in Kosinski format		|
|	Copyright  2002-2004 The KENS Project Development Team						|
|																				|
|	This library is free software; you can redistribute it and/or				|
|	modify it under the terms of the GNU Lesser General Public					|
|	License as published by the Free Software Foundation; either				|
|	version 2.1 of the License, or (at your option) any later version.			|
|																				|
|	This library is distributed in the hope that it will be useful,				|
|	but WITHOUT ANY WARRANTY; without even the implied warranty of				|
|	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU			|
|	Lesser General Public License for more details.								|
|																				|
|	You should have received a copy of the GNU Lesser General Public			|
|	License along with this library; if not, write to the Free Software			|
|	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA	|
|																				|
\*-----------------------------------------------------------------------------*/

#include <stdio.h>

//-----------------------------------------------------------------------------------------------
// Name: GetFileSize(FILE *File)
// Desc: Returns the file size
//-----------------------------------------------------------------------------------------------
unsigned long GetFileSize(FILE *File);
unsigned long GetFileSize(FILE *File)
{
	unsigned long OrgPos, Size;
	OrgPos=ftell(File);
	fseek(File,0,SEEK_END);
	Size=ftell(File);
	fseek(File,OrgPos,SEEK_SET);
	return Size;
}
