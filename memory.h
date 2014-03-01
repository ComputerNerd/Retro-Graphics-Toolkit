/*	memory.h
	Copyright (C) 2004-2013 Mark Tyler and Dmitry Groshev

	This file is part of mtPaint.

	mtPaint is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	mtPaint is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with mtPaint in the file COPYING.
*/
#define ALIGNED(P, N) ((void *)((char *)(P) + \
	((~(unsigned)((char *)(P) - (char *)0) + 1) & ((N) - 1))))
#ifndef NULL
#define NULL 0
#endif

#define MA_ALIGN_MASK    0x03 /* For alignment constraints */
#define MA_ALIGN_DEFAULT 0x00
#define MA_ALIGN_DOUBLE  0x01
#define MA_SKIP_ZEROSIZE 0x04 /* Leave pointers to zero-size areas unchanged */
