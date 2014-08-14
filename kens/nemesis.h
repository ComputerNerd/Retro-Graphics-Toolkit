/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2013 <flamewing.sonic@gmail.com>
 * Loosely based on code by Roger Sanders (AKA Nemesis) and William Sanders
 * (AKA Milamber)
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NEMESIS_H_
#define _NEMESIS_H_

#include <iosfwd>
#include <map>

struct Code;
class nibble_run;
typedef std::map<Code, nibble_run> CodeNibbleMap;

class nemesis {
private:
	static void decode_header(std::istream &Src, CodeNibbleMap &codemap);
	static void decode_internal(std::istream &Src, std::ostream &Dst,
	                            CodeNibbleMap &codemap, size_t rtiles,
	                            bool alt_out = false, int *endptr = 0);
	template<typename Compare>
	static size_t encode_internal(std::istream &Src, std::ostream &Dst, int mode,
	                              size_t sz, Compare const &comp);
public:
	static bool decode(std::istream &Src, std::ostream &Dst, std::streampos Location = 0,
	                   int *endptr = 0);
	static bool encode(std::istream &Src, std::ostream &Dst);
};

#endif // _NEMESIS_H_
