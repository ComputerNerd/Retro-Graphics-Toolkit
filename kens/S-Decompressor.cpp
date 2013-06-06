/*-----------------------------------------------------------------------------*\
|																				|
|	libsaxman: Compression / Decompression of data in Saxman format			|
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

#include <iostream>
#include <malloc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kens.h"
#include "saxman.h"

extern unsigned long GetFileSize(FILE *File);
long end_result;
extern long LoadBuffer(char *&Buffer);

#define SUCCESS										0x00
#define ERROR_UNKNOWN								0x01
#define ERROR_SOURCE_FILE_DOES_NOT_EXIST			0x02

//-- Decompression with size optionally included in input or set by the user --------------------
long Decomp2(char *SrcFile, char *DstFile, long Pointer, unsigned short Size)
{
	return SDecomp(SrcFile, DstFile, Pointer, Size);
}

//-- Decompression To Buffer --------------------------------------------------------------------
long DecompToBuf2(char *SrcFile, char *&DstBuffer, long *BufSize, long Pointer, unsigned short Size)
{
	end_result = SDecomp(SrcFile, (char *)"/tmp/~sax-buf.tmp", Pointer, Size);
	*BufSize = LoadBuffer(DstBuffer);
	return end_result;
}

//-- Decompression with size included in input --------------------------------------------------
long Decomp(char *SrcFile, char *DstFile, long Pointer)
{
	return SDecomp(SrcFile, DstFile, Pointer, 0);
}

//-- Decompression To Buffer --------------------------------------------------------------------
long DecompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize, long Pointer)
{
	end_result = SDecomp(SrcFile, (char *)"/tmp/~sax-buf.tmp", Pointer, 0);
	*BufSize = LoadBuffer(DstBuffer);
	return end_result;
}

//-----------------------------------------------------------------------------------------------
// Name: SDecomp(char *SrcFile, char *DstFile, long Location, unsigned short Size)
// Desc: Decompresses the data using the Saxman compression format
//-----------------------------------------------------------------------------------------------
long SDecomp(char *SrcFile, char *DstFile, long Location, unsigned short Size)
{
// Files
	FILE *Src;
	FILE *Dst;

// Info Byte, Flag, Count and Offset (with initial values)
	unsigned char InfoByte = 0;
	unsigned char IBP = 8;
	unsigned char Flag = 0;
	unsigned char Count = 0;
	unsigned short Offset = 0;
//	unsigned short Size = 0;		// Size of the compressed data

// Other info
	unsigned char Byte;				// Used to store a Byte temporarly
	int Pointer;					// Used to store a Pointer temporarly
	int i;							// Counter

//------------------------------------------------------------------------------------------------

	Src=fopen(SrcFile,"rb");
	if (Src==NULL) return ERROR_SOURCE_FILE_DOES_NOT_EXIST;
	Dst=fopen(DstFile,"w+b");

	fseek(Src, Location, SEEK_SET);

	if (!Size) fread(&Size, 2, 1, Src);

	while(1)
	{
		if (IBP==8) { IBP=0; if (fread(&InfoByte, 1, 1, Src)==0) break; if (ftell(Src) >= Location + Size) break; }
		Flag = ( InfoByte >> (IBP++) ) & 1;
		switch(Flag)
		{
			case 0:
				Offset=0; // See 3 lines below
				if (fread(&Offset, 1, 1, Src)==0) break; if (ftell(Src) >= Location + Size) break;
				if (fread(&Count, 1, 1, Src)==0) break; if (ftell(Src) >= Location + Size) break;
				Offset = ( Offset | ((Count & 0xF0) << 4) ) + 0x12; // Can be improved
				Offset |= (ftell(Dst) & 0xF000);
				Count&=0x0F;
				Count+=3;
				if(Offset>=ftell(Dst))
				{
					Offset -= 0x1000;
				}
				if (Offset<ftell(Dst))
				{
					for (i=0; i<Count; ++i)
					{
						Pointer=ftell(Dst);
						fseek(Dst, Offset + i, SEEK_SET);
						if (fread(&Byte, 1, 1, Dst)==0) break;
						fseek(Dst, Pointer, SEEK_SET);
						fwrite(&Byte, 1, 1, Dst);
					}
				}
				else
				{
					Byte=0;
					for (i=0; i<Count; ++i)
					{
						fwrite(&Byte, 1, 1, Dst);
					}
				}
				break;

			case 1:
				if (fread(&Byte, 1, 1, Src)==0) break; if (ftell(Src) >= Location + Size) break;
				fwrite(&Byte, 1, 1, Dst);
				break;
		}
	}

//------------------------------------------------------------------------------------------------

	fclose(Dst);
	fclose(Src);
	return SUCCESS;
}
