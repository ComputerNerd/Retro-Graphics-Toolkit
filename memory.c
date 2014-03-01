/*	memory.c
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
#include <stdarg.h>
#include <stddef.h>
#include "memory.h"

/* This allocates several memory chunks in one block - making it one single
 * point of allocation failure, and needing just a single free() later on.
 * On Windows, allocations aren't guaranteed to be double-aligned, so
 * MA_ALIGN_DOUBLE flag is necessary there unless no chunks contain doubles. */
void *multialloc(int flags, void *ptr, int size, ...)
{
	va_list args;
	void *res;
	char *tmp;
	size_t tsz, sz = size, align = 0;


	if ((flags & MA_ALIGN_MASK) == MA_ALIGN_DOUBLE)
		align = sizeof(double) - 1;

	va_start(args, size);
	while (va_arg(args, void *))
	{
		sz = (sz + align) & ~align;
		sz += va_arg(args, int);
	}
	va_end(args);
	if (align) sz += align + 1;
	tmp = res = calloc(1, sz);
	if (res)
	{
		tmp = ALIGNED(tmp, align + 1);
		sz = 0; tsz = size;
		va_start(args, size);
		while (1)
		{
			if (tsz || !(flags & MA_SKIP_ZEROSIZE))
				*(void **)ptr = (void *)(tmp + sz);

			if (!(ptr = va_arg(args, void *))) break;

			sz = (sz + tsz + align) & ~align;
			tsz = va_arg(args, int);
		}
		va_end(args);
	}
	return (res);
}
