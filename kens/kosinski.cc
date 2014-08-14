/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Copyright (C) Flamewing 2011-2013 <flamewing.sonic@gmail.com>
 * Copyright (C) 2002-2004 The KENS Project Development Team
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

//#define COUNT_FREQUENCIES 1

#include <istream>
#include <ostream>
#include <sstream>

#ifdef COUNT_FREQUENCIES
#include <iostream>
#endif

#include "kosinski.h"
#include "bigendian_io.h"
#include "bitstream.h"
#include "lzss.h"

using namespace std;

// NOTE: This has to be changed for other LZSS-based compression schemes.
struct KosinskiAdaptor {
	typedef unsigned char  stream_t;
	typedef unsigned short descriptor_t;
	typedef littleendian<descriptor_t> descriptor_endian_t;
	enum {
		// Number of bits on descriptor bitfield.
		NumDescBits = sizeof(descriptor_t) * 8,
		// Number of bits used in descriptor bitfield to signal the end-of-file
		// marker sequence.
		NumTermBits = 2,
		// Flag that tells the compressor that new descriptor fields are needed
		// as soon as the last bit in the previous one is used up.
		NeedEarlyDescriptor = 1,
		// Flag that marks the descriptor bits as being in little-endian bit
		// order (that is, lowest bits come out first).
		DescriptorLittleEndianBits = 1
	};
	// Computes the cost of covering all of the "len" vertices starting from
	// "off" vertices ago.
	// A return of "numeric_limits<size_t>::max()" means "infinite",
	// or "no edge".
	static size_t calc_weight(size_t dist, size_t len) {
		// Preconditions:
		// len != 0 && len <= RecLen && dist != 0 && dist <= SlideWin
		if (len == 1)
			// Literal: 1-bit descriptor, 8-bit length.
			return 1 + 8;
		else if (len == 2 && dist > 256)
			// Can't represent this except by inlining both nodes.
			return numeric_limits<size_t>::max();	// "infinite"
		else if (len <= 5 && dist <= 256)
			// Inline RLE: 2-bit descriptor, 2-bit count, 8-bit distance.
			return 2 + 2 + 8;
		else if (len >= 3 && len <= 9)
			// Separate RLE, short form: 2-bit descriptor, 13-bit distance,
			// 3-bit length.
			return 2 + 13 + 3;
		else //if (len >= 3 && len <= 256)
			// Separate RLE, long form: 2-bit descriptor, 13-bit distance,
			// 3-bit marker (zero), 8-bit length.
			return 2 + 13 + 8 + 3;
	}
	// Given an edge, computes how many bits are used in the descriptor field.
	static size_t desc_bits(AdjListNode const &edge) {
		// Since Kosinski non-descriptor data is always 1, 2 or 3 bytes, this is
		// a quick way to compute it.
		return edge.get_weight() & 7;
	}
	// Kosinski finds no additional matches over normal LZSS.
	static void extra_matches(stream_t const *UNUSED(data),
	                          size_t UNUSED(basenode),
	                          size_t UNUSED(ubound), size_t UNUSED(lbound),
	                          LZSSGraph<KosinskiAdaptor>::MatchVector &UNUSED(matches)) {
	}
	// KosinskiM needs to pad each module to a multiple of 16 bytes.
	static size_t get_padding(size_t totallen, size_t padmask) {
		// Add in the size of the end-of-file marker.
		if (!padmask) {
			return 0;
		}
		size_t padding = totallen + 3 * 8;
		return ((padding + padmask) & ~padmask) - totallen;
	}
};

void kosinski::decode_internal(istream &in, iostream &Dst, size_t &DecBytes) {
	typedef LZSSIStream<KosinskiAdaptor> KosIStream;

	KosIStream src(in);

	while (in.good()) {
		if (src.descbit()) {
			Write1(Dst, src.getbyte());
			++DecBytes;
		} else {
			// Count and distance
			size_t Count = 0;
			size_t distance = 0;

			if (src.descbit()) {
				unsigned char Low = src.getbyte(), High = src.getbyte();

				Count = size_t(High & 0x07);

				if (!Count) {
					Count = src.getbyte();
					if (!Count)
						break;
					else if (Count == 1)
						continue;
					Count += 1;
				} else {
					Count += 2;
				}

				distance = (~size_t(0x1FFF)) | (size_t(0xF8 & High) << 5) | size_t(Low);
			} else {
				unsigned char Low  = src.descbit(),
				              High = src.descbit();

				Count = ((size_t(Low) << 1) | size_t(High)) + 2;

				distance = src.getbyte();
				distance |= ~size_t(0xFF);
			}

			for (size_t i = 0; i < Count; i++) {
				size_t Pointer = Dst.tellp();
				Dst.seekg(Pointer + distance);
				unsigned char Byte = Read1(Dst);
				Dst.seekp(Pointer);
				Write1(Dst, Byte);
			}
			DecBytes += Count;
		}
	}
}

bool kosinski::decode(istream &Src, iostream &Dst,
                      streampos Location, bool Moduled,
                      streamsize const ModulePadding) {
	size_t DecBytes = 0;

	Src.seekg(0, ios::end);
	streamsize sz = streamsize(Src.tellg()) - Location;
	Src.seekg(Location);

	stringstream in(ios::in | ios::out | ios::binary);
	in << Src.rdbuf();

	// Pad to even length, for safety.
	if ((sz & 1) != 0)
		in.put(0x00);

	in.seekg(0);

	if (Moduled) {
		streamsize const PadMask = ModulePadding - 1;
		size_t FullSize = BigEndian::Read2(in);
		if (FullSize == 0xA000)
			FullSize = 0x8000;
		
		while (true) {
			decode_internal(in, Dst, DecBytes);
			if (DecBytes >= FullSize)
				break;

			// Skip padding between modules
			size_t paddingEnd = (((size_t(in.tellg()) - 2) + PadMask) & ~PadMask) + 2;
			in.seekg(paddingEnd);
		}
	} else
		decode_internal(in, Dst, DecBytes);

	return true;
}

void kosinski::encode_internal(ostream &Dst, unsigned char const *&Buffer,
                               streamoff SlideWin, streamoff RecLen,
                               streamsize const BSize,
                               streamsize const Padding) {
	typedef LZSSGraph<KosinskiAdaptor> KosGraph;
	typedef LZSSOStream<KosinskiAdaptor> KosOStream;

	// Compute optimal Kosinski parsing of input file.
	KosGraph enc(Buffer, BSize, SlideWin, RecLen, Padding);
	typename KosGraph::AdjList list = enc.find_optimal_parse();
	KosOStream out(Dst);
#ifdef COUNT_FREQUENCIES
	size_t nliteral = 0, ninline = 0, nrle = 0;
#endif

	streamoff pos = 0;
	// Go through each edge in the optimal path.
	for (typename KosGraph::AdjList::const_iterator it = list.begin();
	        it != list.end(); ++it) {
		AdjListNode const &edge = *it;
		size_t len = edge.get_length(), dist = edge.get_distance();
		// The weight of each edge uniquely identifies how it should be written.
		// NOTE: This needs to be changed for other LZSS schemes.
		switch (edge.get_weight()) {
			case 9:
				// Literal.
				out.descbit(1);
				out.putbyte(Buffer[pos]);
#ifdef COUNT_FREQUENCIES
				++nliteral;
#endif
				break;
			case 12:
				// Inline RLE.
				out.descbit(0);
				out.descbit(0);
				len -= 2;
				out.descbit((len >> 1) & 1);
				out.descbit(len & 1);
				out.putbyte(-dist);
#ifdef COUNT_FREQUENCIES
				++ninline;
#endif
				break;
			case 18:
			case 26: {
				// Separate RLE.
				out.descbit(0);
				out.descbit(1);
				dist = (-dist) & 0x1FFF;
				unsigned short high = (dist >> 5) & 0xF8,
				               low  = (dist & 0xFF);
				if (edge.get_weight() == 18) {
					// 2-byte RLE.
					out.putbyte(low);
					out.putbyte(high | (len - 2));
				} else {
					// 3-byte RLE.
					out.putbyte(low);
					out.putbyte(high);
					out.putbyte(len - 1);
				}
#ifdef COUNT_FREQUENCIES
				++nrle;
#endif
				break;
			}
			default:
				// This should be unreachable.
#ifdef COUNT_FREQUENCIES
				cerr << "Divide By Cucumber Error. Please Reinstall Universe And Reboot." << endl;
#endif
				break;
		}
		// Go to next position.
		pos = edge.get_dest();
	}

#ifdef COUNT_FREQUENCIES
	++nrle;
	cout << "[1]: " << nliteral << ";\t[00]: " << ninline << ";\t[01]: " << nrle << endl;
#endif

	// Push descriptor for end-of-file marker.
	out.descbit(0);
	out.descbit(1);

	// Write end-of-file marker. Maybe use 0x00 0xF8 0x00 instead?
	out.putbyte(0x00);
	out.putbyte(0xF0);
	out.putbyte(0x00);
}

bool kosinski::encode(istream &Src, ostream &Dst, streamoff SlideWin,
                      streamoff RecLen, bool Moduled, streamoff ModuleSize,
                      streamsize const ModulePadding) {
	Src.seekg(0, ios::end);
	streamsize BSize = Src.tellg();
	Src.seekg(0);
	char *const Buffer = new char[BSize];
	unsigned char const *ptr = reinterpret_cast<unsigned char *>(Buffer);
	Src.read(Buffer, BSize);

	if (Moduled) {
		if (BSize > 65535)  // Decompressed size would fill RAM or VRAM.
			return false;

		streamoff FullSize = BSize, CompBytes = 0;
		streamsize const PadMask = ModulePadding - 1;

		if (BSize > ModuleSize)
			BSize = ModuleSize;

		BigEndian::Write2(Dst, FullSize);

		while (true) {
			// We want to manage internal padding for all modules but the last.
			CompBytes += BSize;
			streamsize const Padding = (CompBytes < FullSize) ? ModulePadding : 1u;
			encode_internal(Dst, ptr, SlideWin, RecLen, BSize, Padding);

			ptr += BSize;

			if (CompBytes >= FullSize)
				break;

			// Padding between modules
			size_t paddingEnd = (((size_t(Dst.tellp()) - 2) + PadMask) & ~PadMask) + 2;
			size_t pos = size_t(Dst.tellp());
			if (paddingEnd > pos) {
				size_t n = paddingEnd - pos;

				for (size_t ii = 0; ii < n; ii++) {
					Dst.put(0x00);
				}
			}

			BSize = min(ModuleSize, FullSize - CompBytes);
		}
	} else
		encode_internal(Dst, ptr, SlideWin, RecLen, BSize, 1u);

	// Pad to even size.
	if ((Dst.tellp() & 1) != 0)
		Dst.put(0);

	delete [] Buffer;
	return true;
}
