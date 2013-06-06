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

#ifndef _KOSINSKI_H_
#define _KOSINSKI_H_

#ifdef __cplusplus
extern "C" {
#endif
    
long Comp(char *SrcFile, char *DstFile, bool Moduled);
long CompEx(char *SrcFile, char *DstFile, long SlideWin, long RecLen, bool Moduled);
long Decomp(char *SrcFile, char *DstFile, long Pointer, bool Moduled);

#ifdef __cplusplus
long CompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize , bool Moduled);
long CompToBufEx(char *SrcFile, char *&DstBuffer, long *BufSize, long SlideWin, long RecLen, bool Moduled);
long DecompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize, long Pointer, bool Moduled);
}
#endif

#endif /* _KOSINSKI_H_ */
