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

#ifndef _SAXMAN_H_
#define _SAXMAN_H_

#ifdef __cplusplus
extern "C" {
#endif

long Comp(char *SrcFile, char *DstFile);
long Comp2(char *SrcFile, char *DstFile, bool WithSize);
long Decomp(char *SrcFile, char *DstFile, long Pointer);
long Decomp2(char *SrcFile, char *DstFile, long Pointer, unsigned short Size);

#ifdef __cplusplus
long CompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize);
long CompToBuf2(char *SrcFile, char *&DstBuffer, long *BufSize, bool WithSize);
long DecompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize, long Pointer);
long DecompToBuf2(char *SrcFile, char *&DstBuffer, long *BufSize, long Pointer, unsigned short Size);
}
#endif

#endif /* _SAXMAN_H_ */
