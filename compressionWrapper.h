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
   Copyright Sega16 (or whatever you wish to call me) (2012-2017)
*/
#pragma once
#include <string>
enum class CompressionType {Cancel = -1,
                            Uncompressed = 0,
                            Nemesis, Kosinski,
                            Enigma, Saxman,
                            Comper
                           };
const char*typeToText(CompressionType type);
CompressionType compressionAsk(void);
std::string decodeTypeStr(const char * filename, size_t &filesize, CompressionType type);
void*decodeTypeRam(const uint8_t*dat, size_t inputSize, size_t &filesize, CompressionType type);
void*decodeType(const char * filename, size_t &filesize, CompressionType type);
void*encodeType(const void*in, size_t n, size_t&outSize, CompressionType type);
