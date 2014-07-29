/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Enigma encoder/decoder
 * Copyright (C) Flamewing 2011 <flamewing.sonic@gmail.com>
 * Copyright (C) 2002-2004 The KENS Project Development Team
 * Copyright (C) 2002-2003 Roger Sanders (AKA Nemesis)
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

#include <istream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

#include "kens.h"
#include "enigma.h"
#include "bigendian_io.h"
#include "bitstream.h"
//#include "GetFileSize.h"

#define SUCCESS					0x00
#define ERROR_UNKNOWN				0x01
#define ERROR_SOURCE_FILE_DOES_NOT_EXIST	0x02
#define ERROR_CANT_CREATE_DESTINATION_FILE	0x04
//-----------------------------------------------------------------------------------------------
// Name: LoadBuffer(char *&Buffer)
// Desc: Loads the content of the destination file into a buffer and returns its size
//-----------------------------------------------------------------------------------------------
/*long LoadBuffer(char *&Buffer)
{
	unsigned long Size, Read;
	FILE *hFile = fopen("/tmp/eni-buf.tmp", "rb");
	Size = GetFileSize(hFile);
	if (Buffer==NULL) Buffer = new char[Size];
	fread(Buffer, (size_t)Size, 1, hFile);
	fclose(hFile);
	return Size;
}

//-- Compression --------------------------------------------------------------------------------
long Comp(char *SrcFile, char *DstFile, bool Padding)
{
	return EComp(SrcFile, DstFile, Padding);
}

//-- Compression To Buffer ----------------------------------------------------------------------
long CompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize, bool Padding)
{
	Result = EComp(SrcFile, (char *)"/tmp/eni-buf.tmp", Padding);
	*BufSize = LoadBuffer(DstBuffer);
	return Result;
}

//-- Decompression ------------------------------------------------------------------------------
long Decomp(char *SrcFile, char *DstFile, long Pointer, bool Padding)
{
	return EDecomp(SrcFile, DstFile, Pointer, Padding);
}

//-- Decompression To Buffer --------------------------------------------------------------------
long DecompToBuf(char *SrcFile, char *&DstBuffer, long *BufSize, long Pointer, bool Padding)
{
	end_result = EDecomp(SrcFile, (char *)"/tmp/~eni-buf.tmp", Pointer, Padding);
	*BufSize = LoadBuffer(DstBuffer);
	return end_result;
}

long FreeBuffer(char *&Buffer)
{
	delete[] Buffer;
	Buffer = NULL;
	return 0;
}*/

long EComp(char *SrcFile, char *DstFile, bool Padding)
{
         std::ifstream src(SrcFile, std::ios::in|std::ios::binary);
         if(!src.good())
         {
                return ERROR_SOURCE_FILE_DOES_NOT_EXIST;
         }
         std::fstream dst(DstFile, std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);
         if(!dst.good())
         {
                return ERROR_CANT_CREATE_DESTINATION_FILE;
         }
         if(enigma::encode(src, dst, Padding))
         {
                return SUCCESS;
         }else{
                return ERROR_UNKNOWN;
         }
}

long EDecomp(char *SrcFile, char *DstFile, long Pointer, bool Padding)
{
         std::ifstream src(SrcFile, std::ios::in|std::ios::binary);
         if(!src.good())
         {
                return ERROR_SOURCE_FILE_DOES_NOT_EXIST;
         }
         std::fstream dst(DstFile, std::ios::in|std::ios::out|std::ios::binary|std::ios::trunc);
         if(!dst.good())
         {
                return ERROR_CANT_CREATE_DESTINATION_FILE;
         }
         if(enigma::decode(src, dst, Pointer, Padding))
         {
                return SUCCESS;
         }else{
                return ERROR_UNKNOWN;
         }
}

// Pure virtual base class.
class base_flag_io
{
public:
	static base_flag_io *create(size_t n);
	virtual unsigned short read_bitfield(ibitstream<unsigned short>& bits) const = 0;
	virtual void write_bitfield(obitstream<unsigned short>& bits, unsigned short flags) const = 0;
};

// Templated for blazing speed.
template<int const N>
class flag_io : public base_flag_io
{
public:
	virtual unsigned short read_bitfield(ibitstream<unsigned short>& bits) const
	{
		unsigned short flags = 0;
		if ((N & (1 << 4)) != 0)
			flags |= bits.get() << 15;
		if ((N & (1 << 3)) != 0)
			flags |= bits.get() << 14;
		if ((N & (1 << 2)) != 0)
			flags |= bits.get() << 13;
		if ((N & (1 << 1)) != 0)
			flags |= bits.get() << 12;
		if ((N & (1 << 0)) != 0)
			flags |= bits.get() << 11;
		return flags;
	}
	virtual void write_bitfield(obitstream<unsigned short>& bits, unsigned short flags) const
	{
		if ((N & (1 << 4)) != 0)
			bits.put((flags >> 15) & 1);
		if ((N & (1 << 3)) != 0)
			bits.put((flags >> 14) & 1);
		if ((N & (1 << 2)) != 0)
			bits.put((flags >> 13) & 1);
		if ((N & (1 << 1)) != 0)
			bits.put((flags >> 12) & 1);
		if ((N & (1 << 0)) != 0)
			bits.put((flags >> 11) & 1);
	}
};

// Selects the appropriate template instance to be used.
base_flag_io *base_flag_io::create(size_t n)
{
	switch (n)
	{
		case 0x00:	return new flag_io<0x00>;		case 0x01:	return new flag_io<0x01>;
		case 0x02:	return new flag_io<0x02>;		case 0x03:	return new flag_io<0x03>;
		case 0x04:	return new flag_io<0x04>;		case 0x05:	return new flag_io<0x05>;
		case 0x06:	return new flag_io<0x06>;		case 0x07:	return new flag_io<0x07>;
		case 0x08:	return new flag_io<0x08>;		case 0x09:	return new flag_io<0x09>;
		case 0x0a:	return new flag_io<0x0a>;		case 0x0b:	return new flag_io<0x0b>;
		case 0x0c:	return new flag_io<0x0c>;		case 0x0d:	return new flag_io<0x0d>;
		case 0x0e:	return new flag_io<0x0e>;		case 0x0f:	return new flag_io<0x0f>;
		case 0x10:	return new flag_io<0x10>;		case 0x11:	return new flag_io<0x11>;
		case 0x12:	return new flag_io<0x12>;		case 0x13:	return new flag_io<0x13>;
		case 0x14:	return new flag_io<0x14>;		case 0x15:	return new flag_io<0x15>;
		case 0x16:	return new flag_io<0x16>;		case 0x17:	return new flag_io<0x17>;
		case 0x18:	return new flag_io<0x18>;		case 0x19:	return new flag_io<0x19>;
		case 0x1a:	return new flag_io<0x1a>;		case 0x1b:	return new flag_io<0x1b>;
		case 0x1c:	return new flag_io<0x1c>;		case 0x1d:	return new flag_io<0x1d>;
		case 0x1e:	return new flag_io<0x1e>;		case 0x1f:	return new flag_io<0x1f>;
	}
}

void enigma::decode_internal(std::istream& Src, std::ostream& Dst, std::streamsize sz)
{
	std::stringstream in(std::ios::in|std::ios::out|std::ios::binary);
	in << Src.rdbuf();
	in.seekg(0);

	// Read header.
	size_t const packet_length = Read1(in);
	base_flag_io *mask = base_flag_io::create(Read1(in));
	size_t incrementing_value = BigEndian::Read2(in);
	size_t const common_value = BigEndian::Read2(in);

	ibitstream<unsigned short> bits(in);

	// Lets put in a safe termination condition here.
	while (in.good())
	{
		if (bits.get())
		{
			int mode = bits.read(2);
			switch (mode)
			{
				case 2:
					mode = -1;
				case 1:
				case 0:
				{
					size_t cnt = bits.read(4) + 1;
					unsigned short flags = mask->read_bitfield(bits),
					               outv  = bits.read(packet_length);
					outv |= flags;

					for (size_t i = 0; i < cnt; i++)
					{
						BigEndian::Write2(Dst, outv);
						outv += mode;
					}
					break;
				}
				case 3:
				{
					size_t cnt = bits.read(4);
					// This marks decompression as being done.
					if (cnt == 0x0F)
						return;

					cnt++;
					for (size_t i = 0; i < cnt; i++)
					{
						unsigned short flags = mask->read_bitfield(bits),
								       outv  = bits.read(packet_length);
						BigEndian::Write2(Dst, outv | flags);
					}
					break;
				}
			}
		}
		else
		{
			if (!bits.get())
			{
				size_t cnt = bits.read(4) + 1;
				for (size_t i = 0; i < cnt; i++)
					BigEndian::Write2(Dst, incrementing_value++);
			}
			else
			{
				size_t cnt = bits.read(4) + 1;
				for (size_t i = 0; i < cnt; i++)
					BigEndian::Write2(Dst, common_value);
			}
		}
	}
}

bool enigma::decode(std::istream& Src, std::ostream& Dst, std::streampos Location,
                    bool padding)
{
	Src.seekg(0, std::ios::end);
	std::streamsize sz = Src.tellg() - Location;
	Src.seekg(Location);
	if (padding)
	{
		// This is a plane map into a full pattern name table.
		std::stringstream out(std::ios::in|std::ios::out|std::ios::binary);
		decode_internal(Src, out, sz);
		out.clear();
		out.seekg(0);

		std::string pad(0x80 * 0x20, (char)0);
		char buf[0x40];
		// Add a lot of padding (plane map).
		Dst.write(pad.c_str(), 0x80 * 0x20);
		for (size_t i = 0; i < 0x20; i++)
		{
			Dst.write(pad.c_str(), 0x20);
			out.read(buf, sizeof(buf));
			Dst.write(buf, sizeof(buf));
			Dst.write(pad.c_str(), 0x20);
		}
		Dst.write(pad.c_str(), 0x80 * 0x20);
	}
	else
		decode_internal(Src, Dst, sz);
	return true;
}

// Blazing fast function that gives the index of the MSB.
static inline unsigned char log2i(unsigned short v)//Changed name to avoid c++11 conflict
{
	register unsigned char r; // result of log2(v) will go here
	register unsigned char shift;

	r =     (v > 0xFF) << 3; v >>= r;
	shift = (v > 0xF ) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3 ) << 1; v >>= shift; r |= shift;
	                                      r |= (v >> 1);
	return r;
}

// Comparison functor, see below.
struct Compare_count
{
	bool operator()(std::pair<unsigned short const, size_t>& it1,
	                std::pair<unsigned short const, size_t>& it2)
	{
		return (it1.second < it2.second);
	}
};

// This flushes (if needed) the contents of the inlined data buffer.
static inline void flush_buffer(std::vector<unsigned short>& buf,
                  obitstream<unsigned short>& bits,
                  base_flag_io *&mask,
                  unsigned short const packet_length)
{
	if (!buf.size())
		return;

	bits.write(0x70 | (buf.size() - 1) & 0xf, 7);
	for (std::vector<unsigned short>::iterator it = buf.begin();
	     it != buf.end(); ++it)
	{
		unsigned short v = *it;
		mask->write_bitfield(bits, v);
		bits.write(v & 0x7ff, packet_length);
	}
	buf.clear();
}

void enigma::encode_internal(std::istream& Src, std::ostream& Dst, std::streamsize sz)
{
	// To unpack source into 2-byte words.
	std::vector<unsigned short> unpack;
	// Frequency map.
	std::map<unsigned short, size_t> counts;
	// Presence map.
	std::set<unsigned short> elems;

	// Unpack source into array. Along the way, build frequency and presence maps.
	unsigned short maskval = 0;
	Src.clear();
	Src.seekg(0);
	while (true)
	{
		unsigned short v = BigEndian::Read2(Src);
		if (!Src.good())
			break;
		maskval |= v;
		counts[v] += 1;
		elems.insert(v);
		unpack.push_back(v);
	}

	base_flag_io *mask = base_flag_io::create(maskval >> 11);
	unsigned short const packet_length = log2i(maskval & 0x7ff) + 1;

	// Find the most common 2-byte value.
	Compare_count cmp;
	std::map<unsigned short, size_t>::iterator high = 
			max_element(counts.begin(), counts.end(), cmp);
	unsigned short const common_value = high->first;
	// No longer needed.
	counts.clear();

	// Find incrementing (not neccessarily contiguous) runs.
	// The original algorithm does this for all 65536 2-byte words, while
	// this version only checks the 2-byte words actually in the file.
	std::map<unsigned short, size_t> runs;
	for (std::set<unsigned short>::iterator it = elems.begin();
	     it != elems.end(); ++it)
	{
		unsigned short next = *it;
		std::map<unsigned short, size_t>::iterator val =
			runs.insert(std::pair<unsigned short, size_t>(next, 0)).first;
		for (std::vector<unsigned short>::iterator it2 = unpack.begin();
		     it2 != unpack.end(); ++it2)
		{
			if (*it2 == next)
			{
				next++;
				val->second += 1;
			}
		}
	}
	// No longer needed.
	elems.clear();

	// Find the starting 2-byte value with the longest incrementing run.
	std::map<unsigned short, size_t>::iterator incr = 
			max_element(runs.begin(), runs.end(), cmp);
	unsigned short incrementing_value = incr->first;
	// No longer needed.
	runs.clear();

	// Output header.
	Write1(Dst, packet_length);
	Write1(Dst, maskval >> 11);
	BigEndian::Write2(Dst, incrementing_value);
	BigEndian::Write2(Dst, common_value);
	
	// Time now to compress the file.
	obitstream<unsigned short> bits(Dst);
	std::vector<unsigned short> buf;
	size_t pos = 0;
	while (pos < unpack.size())
	{
		unsigned short v = unpack[pos];
		if (v == incrementing_value)
		{
			flush_buffer(buf, bits, mask, packet_length);
			unsigned short next = v + 1;
			size_t cnt = 0;
			for (size_t i = pos + 1; i < unpack.size() && cnt < 0xf; i++)
			{
				if (next != unpack[i])
					break;
				next++;
				cnt++;
			}
			bits.write(0x00 | cnt, 6);
			incrementing_value = next;
			pos += cnt;
		}
		else if (v == common_value)
		{
			flush_buffer(buf, bits, mask, packet_length);
			unsigned short next = v;
			size_t cnt = 0;
			for (size_t i = pos + 1; i < unpack.size() && cnt < 0xf; i++)
			{
				if (next != unpack[i])
					break;
				cnt++;
			}
			bits.write(0x10 | cnt, 6);
			pos += cnt;
		}
		else
		{
			unsigned short next = unpack[pos + 1];
			int delta = int(next) - int(v);
			if (pos + 1 < unpack.size() && next != incrementing_value &&
			    (delta == -1 || delta == 0 || delta == 1))
			{
				flush_buffer(buf, bits, mask, packet_length);
				size_t cnt = 1;
				next += delta;
				for (size_t i = pos + 2; i < unpack.size() && cnt < 0xf; i++)
				{
					if (next != unpack[i] || next == incrementing_value)
						break;
					next += delta;
					cnt++;
				}

				if (delta == -1)
					delta = 2;

				delta = ((delta | 4) << 4);
				bits.write(delta | cnt, 7);
				mask->write_bitfield(bits, v);
				bits.write(v & 0x7ff, packet_length);
				pos += cnt;
			}
			else
			{
				if (buf.size() >= 0xf)
					flush_buffer(buf, bits, mask, packet_length);

				buf.push_back(v);
			}
		}
		pos++;
	}

	flush_buffer(buf, bits, mask, packet_length);

	// Terminator.
	bits.write(0x7f, 7);
	bits.flush();
}

bool enigma::encode(std::istream& Src, std::ostream& Dst, bool padding)
{
	Src.seekg(0, std::ios::end);
	std::streamsize sz = Src.tellg();
	Src.seekg(0);

	// Remove padding associated with S1 special stages in 80x80 block version.
	if (padding && sz >= 0x3000)
	{
		std::stringstream src(std::ios::in|std::ios::out|std::ios::binary);
		Src.ignore(0x80 * 0x20);
		char buf[0x40];
		for (size_t i = 0; i < 0x20; i++)
		{
			Src.ignore(0x20);
			Src.read(buf, sizeof(buf));
			src.write(buf, sizeof(buf));
			Src.ignore(0x20);
		}
		encode_internal(src, Dst, 0x1000);
	}
	else
	{
		encode_internal(Src, Dst, sz);
		// Pad to even size.
		if ((Dst.tellp() & 1) != 0)
			Dst.put(0);
	}

	return true;
}


