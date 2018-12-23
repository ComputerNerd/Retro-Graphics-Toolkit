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
#include <FL/fl_ask.H>
#include <FL/filename.H>

#include <stdio.h>
#include <sys/stat.h>

#include "compressionWrapper.h"
#include "errorMsg.h"
#include "filemisc.h"
#include "filereader.h"
#include "gui.h"
#include "luaconfig.h"
#include "runlua.h"
filereader::filereader(boost::endian::order endian, unsigned bytesPerElement, const char*title, bool relptr, unsigned offbits, bool be) {
	char*fname;

	if (title)
		fname = loadsavefile(title);
	else
		fname = loadsavefile();

	if (fname) {
		int compression = compressionAsk();

		if (compression < 0) {
			amt = 0;
			return;
		}

		char*ext = (char*)fl_filename_ext(fname);
		ext = strdup(ext);

		for (char*p = ext; *p; ++p)
			*p = tolower(*p);

		fileType_t def = fileType_t::tBinary;

		if ((!strcmp(ext, ".asm")) || (!strcmp(ext, ".s")))
			def = fileType_t::tASM;
		else if (!strcmp(ext, ".bex"))
			def = fileType_t::tBEX;
		else if ((!strcmp(ext, ".h")) || (!strcmp(ext, ".hh")) || (!strcmp(ext, ".hpp"))
		         || (!strcmp(ext, ".c")) || (!strcmp(ext, ".cpp")) || (!strcmp(ext, ".cxx")) || (!strcmp(ext, ".cc")))
			def = fileType_t::tCheader;

		free(ext);
		fileType_t tp = askSaveType(false, def);

		if (tp == fileType_t::tCancel) {
			amt = 0;
			free(fname);
			return;
		}

		struct stat st;

		FILE*fp = fopen(fname, tp == fileType_t::tBinary ? "rb" : "r");

		if (!fp) {
			fl_alert("An error has occurred: %s", strerror(errno));
			amt = 0;
			return;
		}

		if (fstat(fileno(fp), &st) != 0) {
			fl_alert("An error has occurred: %s", strerror(errno));
			amt = 0;
			fclose(fp);
			return;
		}

		char*tmp = (char*)malloc(st.st_size);
		fread(tmp, 1, st.st_size, fp);
		fclose(fp);
		//This is handled by Lua code so the user can modify the behavior of this function with ease
		lua_getglobal(Lconf, "filereaderProcessText");
		lua_pushinteger(Lconf, (int)tp);
		lua_pushboolean(Lconf, relptr);
		lua_pushinteger(Lconf, offbits);
		lua_pushboolean(Lconf, be);
		lua_pushlstring(Lconf, tmp, st.st_size); //Lua makes a copy of the string
		lua_pushstring(Lconf, fname);
		free(fname);
		free(tmp);
		runLuaFunc(Lconf, 6, 1);
		size_t len = lua_rawlen(Lconf, -1);

		if (len) {
			lenTotal = 0;
			amt = len;

			for (size_t i = 1; i <= len; ++i) {
				lua_rawgeti(Lconf, -1, i);
				//Get its name
				lua_rawgeti(Lconf, -1, 1);
				names.emplace_back(lua_tostring(Lconf, -1));
				lua_pop(Lconf, 1);
				//Get its data
				lua_rawgeti(Lconf, -1, 2);
				size_t ln;
				const char*src = lua_tolstring(Lconf, -1, &ln);

				if (ln % bytesPerElement) {
					fl_alert("This is not a multiple of %d bytes", bytesPerElement);
					continue;
				}

				void* dst;

				if (compression) {
					dst = decodeTypeRam((const uint8_t*)src, ln, ln, compression);

					if (ln == 0) {
						free(dst);
						continue;
					}
				} else {
					dst = malloc(ln);
					memcpy(dst, src, ln);
				}

				if (boost::endian::order::native != endian) {
					switch (bytesPerElement) {
						case 1:
							// No action required.
							break;

						case 2:
						{
							uint16_t*ptr = (uint16_t*)dst;

							for (size_t i = 0; i < ln / 2; ++i) {
								uint16_t tmp = *ptr;

								if (endian == boost::endian::order::big)
									boost::endian::big_to_native_inplace(tmp);
								else if (endian == boost::endian::order::little)
									boost::endian::little_to_native_inplace(tmp);

								*ptr++ = tmp;

							}
						}
						break;

						case 4:
						{
							uint32_t*ptr = (uint32_t*)dst;

							for (size_t i = 0; i < ln / 4; ++i) {
								uint32_t tmp = *ptr;

								if (endian == boost::endian::order::big)
									boost::endian::big_to_native_inplace(tmp);
								else if (endian == boost::endian::order::little)
									boost::endian::little_to_native_inplace(tmp);

								*ptr++ = tmp;

							}
						}
						break;

						default:
							show_default_error
					}
				}

				lens.push_back(ln);
				dat.push_back(dst);
				lenTotal += ln;
				lua_pop(Lconf, 2);
			}
		} else
			amt = 0;

		lua_pop(Lconf, 1);
	} else
		amt = 0;
}

filereader::~filereader() {
	for (void*elm : dat)
		free(elm);
}

unsigned filereader::selDat(void) {
	if (amt > 1)
		return menuPopupVector("Select an array", "There are multiple arrays. Which one should be loaded?", names);

	else
		return 0;
}
