/*
	This file is part of Retro Graphics Toolkit

	Retro Graphics Toolkit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or any later version.

	Retro Graphics Toolkit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Retro Graphics Toolkit. If not, see <http://www.gnu.org/licenses/>.
	Copyright Sega16 (or whatever you wish to call me) (2012-2018)
*/
#include <boost/endian/conversion.hpp>

#include <vector>
#include <string>
#include <stdint.h>
struct filereader {
	size_t amt, lenTotal;
	std::vector<size_t>lens;
	std::vector<void*>dat;
	std::vector<std::string>names;
	filereader(boost::endian::order endian, unsigned bytesPerElement, const char*title = nullptr, bool relptr = false, unsigned offbits = 16, bool be = true, const char * filename = nullptr, fileType_t forceType = fileType_t::tCancel);
	unsigned selDat(void);
	~filereader();
};
