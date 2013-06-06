/*-------------------------------------------------------------------------------------*\
|											|
|	libkosinski: Compression / Decompression of data in sega formats		|
|	Copyright  2002-2004 The KENS Project Development Team				|
|											|
|	This library is free software; you can redistribute it and/or			|
|	modify it under the terms of the GNU Lesser General Public			|
|	License as published by the Free Software Foundation; either			|
|	version 2.1 of the License, or (at your option) any later version.		|
|											|
|	This library is distributed in the hope that it will be useful,			|
|	but WITHOUT ANY WARRANTY; without even the implied warranty of			|
|	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU		|
|	Lesser General Public License for more details.					|
|											|
|	You should have received a copy of the GNU Lesser General Public		|
|	License along with this library; if not, write to the Free Software		|
|	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA	|
|											|
\*-------------------------------------------------------------------------------------*/

//kens version number
#pragma once
#define KENS_VERSION = 1.5.0

#ifndef __KENS_H_
#define __KENS_H_

#ifdef __cplusplus
extern "C" {
#endif

long KComp(char *SrcFile, char *DstFile, int SlideWin, int RecLen, bool Moduled);
long KDecomp(char *SrcFile, char *DstFile, long Pointer, bool Moduled);
long EComp(char *SrcFile, char *DstFile, bool Padding);
long EDecomp(char *SrcFile, char *DstFile, long Pointer, bool Padding);
long NComp(char *SrcFile, char *DstFile);
long NDecomp(char *SrcFile, char *DstFile, long Pointer);
long SComp(char *SrcFile, char *DstFile, bool WithSize);
long SDecomp(char *SrcFile, char *DstFile, long Pointer, unsigned short Size);

#ifdef __cplusplus
long FreeBuffer(char *&Buffer);
}
#include <map>
class istream;
class ostream;
class nibble_run;

typedef std::map<std::pair<unsigned char,unsigned char>, nibble_run> Codemap;

class kosinski
{
private:
	static void decode_internal(std::istream& in, std::iostream& Dst, size_t &DecBytes);
	static void encode_internal(std::ostream& Dst, unsigned char const *&Buffer, std::streamoff SlideWin, std::streamoff RecLen, std::streamsize const BSize);
public:
	static bool decode(std::istream& Src, std::iostream& Dst, std::streampos Location = 0, bool Moduled = false);
	static bool encode(std::istream& Src, std::ostream& Dst, std::streamoff SlideWin = 8192, std::streamoff RecLen = 256, bool Moduled = false, std::streamoff ModuleSize = 0x1000);
};

class enigma
{
private:
	static void decode_internal(std::istream& Src, std::ostream& Dst, std::streamsize sz);
	static void encode_internal(std::istream& Src, std::ostream& Dst, std::streamsize sz);
public:
	static bool decode(std::istream& Src, std::ostream& Dst, std::streampos Location = 0, bool padding = false);
	static bool encode(std::istream& Src, std::ostream& Dst, bool padding = false);
};

class nemesis
{
private:
	static void decode_header(std::istream& Src, std::ostream& Dst, Codemap& codemap);
	static void decode_internal(std::istream& Src, std::ostream& Dst, Codemap& codemap, size_t rtiles, bool alt_out = false, int *endptr = 0);
	static void encode_internal(std::istream& Src, std::ostream& Dst, int mode, size_t sz);
public:
	static bool decode(std::istream& Src, std::ostream& Dst, std::streampos Location = 0, int *endptr = 0);
	static bool encode(std::istream& Src, std::ostream& Dst);
};

class saxman
{
private:
	static void decode_internal(std::istream& in, std::iostream& Dst, size_t &DecBytes);
	static void encode_internal(std::ostream& data, unsigned char const *&Buffer, std::streamsize const BSize);
public:
	static bool decode(std::istream& Src, std::iostream& Dst, std::streampos Location, unsigned short size);
	static bool encode(std::istream& Src, std::ostream& Dst, bool WithSize);
};
#endif

#endif
