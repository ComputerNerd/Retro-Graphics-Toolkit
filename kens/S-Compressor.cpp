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
long Result;

#define SUCCESS										0x00
#define ERROR_UNKNOWN								0x01
#define ERROR_SOURCE_FILE_DOES_NOT_EXIST			0x02

long LoadBuffer(char *&Buffer)
{
	unsigned long Size, Read;
	FILE *hFile = fopen("/tmp/sax-buf.tmp", "rb");
	Size = GetFileSize(hFile);
	if (Buffer==NULL) Buffer = new char[Size];
	fread(Buffer, (size_t)Size, 1, hFile);
	fclose(hFile);
	return Size;
}

//-- Compression with size optionally written to output or discarded ----------------------------
long Comp2(char *SrcFile, char *DstFile, bool WithSize)
{
	return SComp(SrcFile, DstFile, WithSize);
}

//-- Compression To Buffer ----------------------------------------------------------------------
long CompToBuf2(char *SrcFile, char *&DstBuffer, long *BufSize, bool WithSize)
{
	Result = SComp(SrcFile, (char *)"/tmp/sax-buf.tmp", WithSize);
	*BufSize = LoadBuffer(DstBuffer);
	return Result;
}

//-- Compression with size written to output ----------------------------------------------------
long Comp(char *SrcFile, char *DstFile)
{
	return SComp(SrcFile, DstFile, true);
}

//-- Compression To Buffer ----------------------------------------------------------------------
long CompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize)
{
	Result = SComp(SrcFile, (char *)"/tmp/sax-buf.tmp", true);
	*BufSize = LoadBuffer(DstBuffer);
	return Result;
}

long FreeBuffer(char *&Buffer)
{
	delete[] Buffer;
	Buffer = NULL;
	return 0;
}

//-----------------------------------------------------------------------------------------------
// Name: SComp(char *SrcFile, char *DstFile, bool WithSize)
// Desc: Compresses the data using the Saxman compression format
//-----------------------------------------------------------------------------------------------
long SComp(char *SrcFile, char *DstFile, bool WithSize)
{
// Files
	FILE *Src;
	FILE *Dst;

// Info Byte, IBP and Size
	unsigned char InfoByte;
	unsigned char IBP;
	unsigned short Size;

// Buffer Infos
	unsigned char *Buffer;
	int BSize;
	int BPointer;

// Data info (temp)
	unsigned char Data[64];
	unsigned char DS;
	
// Count and Offest & Info
	int Count = 0;
	int Offset = 0;
	int IOffset = 0;
	unsigned short Info;

// Counters
	int i, j=0, k;

//----------------------------------------------------------------------------------------------------------------

	Src=fopen(SrcFile,"rb");
	if (Src==NULL) return ERROR_SOURCE_FILE_DOES_NOT_EXIST;
	BSize=GetFileSize(Src)+18;
	Buffer = new unsigned char[BSize];
	if (Buffer==NULL) { fclose(Src); return ERROR_UNKNOWN; }
	memset(Buffer, 0, 18);
	fread(Buffer+18,BSize-18,1,Src);
	fclose(Src);
	Dst=fopen(DstFile,"wb");
	if (WithSize) fseek(Dst, 2, SEEK_CUR);
/*
	InfoByte=1;
	IBP=1;
	Data[0]=Buffer[18];
	BPointer=19;
	DS=1;
*/
	InfoByte=0;
	IBP=0;
	BPointer=18;
	DS=0;

//----------------------------------------------------------------------------------------------------------------

start:
	Count=18; if (BSize-BPointer<18) { Count=BSize; Count-=BPointer; }
	k=1; // Minimal recurrence length, will contain the total recurrence length
	//i=0;
	i=BPointer-0x1000; if (i<0) i=0;
	do {
		j=0; // Will contain the total recurrence length for one loop, then will be set to 0
		while ( Buffer[i+j] == Buffer[BPointer+j] ) { if(++j>=Count) break; }
		if (j>k) { k=j; Offset=i; }
		if (i==0) i=17;
	} while (++i<BPointer);
	Count=k;

//----------------------------------------------------------------------------------------------------------------

	if (Count==1)
	{
		InfoByte|=1<<IBP; // 2^IBP
		
		Data[DS]=Buffer[BPointer];
		DS+=1;

		if (++IBP==8) { fwrite(&InfoByte,1,1,Dst); fwrite(&Data,DS,1,Dst); InfoByte=IBP=DS=0; }	
	}

	else if (Count==2)
	{
		InfoByte|=1<<IBP; // 2^IBP

		Data[DS]=Buffer[BPointer];
		DS+=1;

		if (++IBP==8) { fwrite(&InfoByte,1,1,Dst); fwrite(&Data,DS,1,Dst); InfoByte=IBP=DS=0; }	

		--Count;
	}

	else
	{
		//IOffset = Offset - 0x24;
		IOffset = ((Offset - 0x12) & 0x0FFF) - 0x12;

		Info = ( (IOffset & 0xFF) << 8 ) | ( (IOffset & 0xF00) >> 4 ) | ( (Count - 3) & 0x0F );

		Data[DS]=static_cast<char>(Info >> 8);
		Data[DS+1]=static_cast<char>(Info & 0xFF);
		DS+=2;

		if (++IBP==8) { fwrite(&InfoByte,1,1,Dst); fwrite(&Data,DS,1,Dst); InfoByte=IBP=DS=0; }	
	}

//----------------------------------------------------------------------------------------------------------------

	BPointer+=Count;
	if (BPointer<BSize) goto start;

//----------------------------------------------------------------------------------------------------------------

	fwrite(&InfoByte,1,1,Dst); fwrite(&Data,DS,1,Dst); InfoByte=IBP=DS=0;
	if (WithSize)
	{
		Size=ftell(Dst)-2;
		fseek(Dst, 0, SEEK_SET);
		fwrite(&Size, 2, 1, Dst);
	}
	fclose(Dst);
	delete[] Buffer;
	return SUCCESS;
}
