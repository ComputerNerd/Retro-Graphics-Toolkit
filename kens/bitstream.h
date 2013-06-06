/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * S2-SSEdit
 * Copyright (C) Flamewing 2011 <flamewing.sonic@gmail.com>
 * 
 * S2-SSEdit is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * S2-SSEdit is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

#include <istream>
#include <ostream>

template <typename T>
class bigendian
{
public:
	size_t read(std::istream& src)
	{	return BigEndian::ReadN<std::istream&, sizeof(T)>(src);	}
	void write(std::ostream& dst, size_t c)
	{	BigEndian::WriteN<std::ostream&, sizeof(T)>(dst, c);	}
};

template <typename T>
class littleendian
{
public:
	size_t read(std::istream& src)
	{	return LittleEndian::ReadN<std::istream&, sizeof(T)>(src);	}
	void write(std::ostream& dst, size_t c)
	{	LittleEndian::WriteN<std::ostream&, sizeof(T)>(dst, c);	}
};

// This class allows reading bits from a stream.
template <typename T, typename Reader=bigendian<T> >
class ibitstream
{
private:
	std::istream& src;
	Reader r;
	int readbits;
	T bytebuffer;
	void check_buffer()
	{
		if (readbits)
			return;

		bytebuffer = r.read(src);
		if (src.good())
			readbits = sizeof(T) * 8;
		else
			readbits = 16;
	}
public:
	ibitstream(std::istream& s)	: src(s), readbits(sizeof(T) * 8)
	{	bytebuffer = r.read(src); 	}
	// Gets a single bit from the stream. Remembers previously read bits,
	// and gets a character from the actual stream once all bits in the current
	// byte have been read.
	T get()
	{
		check_buffer();
		--readbits;
		T bit = (bytebuffer >> readbits) & 1;
		bytebuffer ^= (bit << readbits);
		return bit;
	}
	// Gets a single bit from the stream. Remembers previously read bits,
	// and gets a character from the actual stream once all bits in the current
	// byte have been read.
	// Treats bits as being in the reverse order of the get function.
	T pop()
	{
		--readbits;
		T bit = bytebuffer & 1;
		bytebuffer >>= 1;
		check_buffer();
		return bit;
	}
	// Reads up to sizeof(T) * 8 bits from the stream. Remembers previously read bits,
	// and gets a character from the actual stream once all bits in the current
	// byte have been read.
	T read(unsigned char cnt)
	{
		check_buffer();
		if (readbits < cnt)
		{
			int delta = (cnt - readbits);
			T bits = bytebuffer << delta;
			bytebuffer = r.read(src);
			readbits = (sizeof(T) * 8) - delta;
			T newbits = (bytebuffer >> readbits);
			bytebuffer ^= (newbits << readbits);
			return bits | newbits;
		}
		else
		{
			readbits -= cnt;
			T bits = bytebuffer >> readbits;
			bytebuffer ^= (bits << readbits);
			return bits;
		}
	}
};

// This class allows outputting bits into a stream.
template <typename T, typename Writer=bigendian<T> >
class obitstream
{
private:
	std::ostream& dst;
	Writer w;
	int waitingbits;
	T bytebuffer;
public:
	obitstream(std::ostream& d)	: dst(d), waitingbits(0), bytebuffer(0)
	{	}
	// Puts a single bit into the stream. Remembers previously written bits,
	// and outputs a character to the actual stream once there are at least
	// sizeof(T) * 8 bits stored in the buffer.
	bool put(T data)
	{
		bytebuffer = (bytebuffer << 1) | (data & 1);
		if (++waitingbits >= sizeof(T) * 8)
		{
			w.write(dst, bytebuffer);
			waitingbits = 0;
			return true;
		}
		return false;
	}
	// Puts a single bit into the stream. Remembers previously written bits,
	// and outputs a character to the actual stream once there are at least
	// sizeof(T) * 8 bits stored in the buffer.
	// Treats bits as being in the reverse order of the put function.
	bool push(T data)
	{
		bytebuffer |= ((data & 1) << waitingbits);
		if (++waitingbits >= sizeof(T) * 8)
		{
			w.write(dst, bytebuffer);
			waitingbits = 0;
			bytebuffer = 0;
			return true;
		}
		return false;
	}
	// Writes up to sizeof(T) * 8 bits to the stream. Remembers previously written bits,
	// and outputs a character to the actual stream once there are at least
	// sizeof(T) * 8 bits stored in the buffer.
	bool write(T data, unsigned char size)
	{
		if (waitingbits + size >= sizeof(T) * 8)
		{
			int delta = (sizeof(T) * 8 - waitingbits);
			waitingbits = (waitingbits + size) % (sizeof(T) * 8);
			w.write(dst, (bytebuffer << delta) | (data >> waitingbits));
			bytebuffer = data;
			return true;
		}
		else
		{
			bytebuffer = (bytebuffer << size) | data;
			waitingbits += size;
			return false;
		}
	}
	// Flushes remaining bits (if any) to the buffer, completing the byte by
	// padding with zeroes.
	bool flush(bool unchanged = false)
	{
		if (waitingbits)
		{
			if (!unchanged)
				bytebuffer <<= ((sizeof(T) * 8) - waitingbits);
			w.write(dst, bytebuffer);
			waitingbits = 0;
			return true;
		}
		return false;
	}
	int have_waiting_bits() const
	{	return waitingbits;	}
};

#endif // _BITSTREAM_H_
