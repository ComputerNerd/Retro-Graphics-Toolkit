/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Nemesis encoder/decoder
 * Copyright (C) Flamewing 2011 <flamewing.sonic@gmail.com>
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

#include <tr1/memory>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <vector>

#include "kens.h"
#include "nemesis.h"
#include "bigendian_io.h"
#include "bitstream.h"
//#include "GetFileSize.h"

#define SUCCESS					0x00
#define ERROR_UNKNOWN				0x01
#define ERROR_SOURCE_FILE_DOES_NOT_EXIST	0x02
#define ERROR_CANT_CREATE_DESTINATION_FILE	0x04

static long Result;
static long end_result;
long NComp(char *SrcFile, char *DstFile)
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
         if(nemesis::encode(src, dst))
         {
                return SUCCESS;
         }else{
                return ERROR_UNKNOWN;
         }
}

long NDecomp(char *SrcFile, char *DstFile, long Pointer)
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
         if(nemesis::decode(src, dst, Pointer, 0))
         {
                return SUCCESS;
         }else{
                return ERROR_UNKNOWN;
         }
}

// This represents a nibble run of up to 7 repetitions of the starting nibble.
class nibble_run
{
private:
	unsigned char nibble;   // Nibble we are interested in.
	unsigned char count;	// How many times the nibble is repeated.
public:
	// Constructors.
	nibble_run() : nibble(0), count(0) {  }
	nibble_run(unsigned char n, unsigned char c) : nibble(n), count(c) {  }
	nibble_run(nibble_run const& other) : nibble(other.nibble), count(other.count) {  }
	nibble_run& operator=(nibble_run const& other)
	{
		if (this == &other)
			return *this;
		nibble = other.nibble;
		count = other.count;
		return *this;
	}
	// Sorting operator.
	bool operator<(nibble_run const& other) const
	{	return (nibble < other.nibble) || (nibble == other.nibble && count < other.count);	}
	// Sorting operator.
	bool operator>(nibble_run const& other) const
	{	return other < *this;	}
	bool operator==(nibble_run const& other) const
	{   return !(*this < other) && !(other < *this);	}
	bool operator!=(nibble_run const& other) const
	{   return !(*this == other);	}
	// Getters/setters for all properties.
	unsigned char get_nibble() const
	{	return nibble;	}
	unsigned char get_count() const
	{	return count;	}
	void set_nibble(unsigned char tf)
	{	nibble = tf;	}
	void set_count(unsigned char tf)
	{	count = tf;		}
};

// Slightly based on code by Mark Nelson for Huffman encoding.
// http://marknelson.us/1996/01/01/priority-queues/
// This represents a node (leaf or branch) in the Huffman encoding tree.
class node : public std::tr1::enable_shared_from_this<node>
{
private:
	std::tr1::shared_ptr<node> child0, child1;
	int weight;
	nibble_run value;
public:
	// Construct a new leaf node for character c.
	node(nibble_run const& val, int wgt = -1)
	: weight(wgt), value (val)
	{      }
	// Construct a new internal node that has children c1 and c2.
	node(std::tr1::shared_ptr<node> c0, std::tr1::shared_ptr<node> c1)
	{
		value = nibble_run();
		weight = c0->weight + c1->weight;
		child0 = c0;
		child1 = c1;
	}
	~node()
	{
		child0.reset();
		child1.reset();
	}
	// Free the memory used by the child nodes.
	void prune()
	{
		child0.reset();
		child1.reset();
	}
	// Comparison operators.
	bool operator<(node const& other) const
	{	return weight < other.weight;	}
	bool operator>(node const& other) const
	{	return weight > other.weight;	}
	// This tells if the node is a leaf or a branch.
	bool is_leaf() const
	{   return child0 == 0 && child1 == 0;  }
	// Getters/setters for all properties.
	std::tr1::shared_ptr<node const> get_child0() const
	{	return child0;	}
	std::tr1::shared_ptr<node const> get_child1() const
	{	return child1;	}
	int get_weight() const
	{	return weight;	}
	nibble_run const& get_value() const
	{	return value;	}
	void set_child0(std::tr1::shared_ptr<node> c0)
	{	child0 = c0;	}
	void set_child1(std::tr1::shared_ptr<node> c1)
	{	child1 = c1;	}
	void set_weight(int w)
	{	weight = w;	}
	void set_value(nibble_run const& v)
	{	value = v;	}
	// This goes through the tree, starting with the current node, generating
	// a map associating a nibble run with its code length.
	void traverse(std::map<nibble_run, size_t>& sizemap) const
	{
		if (is_leaf())
			sizemap[value] += 1;
		else
		{
			if (child0)
				child0->traverse(sizemap);
			if (child1)
				child1->traverse(sizemap);
		}
	}
};

struct Compare_size
{
	bool operator()(std::pair<size_t,std::pair<size_t,nibble_run> > const& lhs,
	                std::pair<size_t,std::pair<size_t,nibble_run> > const& rhs)
	{
		if (lhs.first < rhs.first)
			return true;
		else if (lhs.first > rhs.first)
			return false;
		//rhs.first == lhs.first
		if (lhs.second.first > rhs.second.first)
			return true;
		else if (lhs.second.first < rhs.second.first)
			return false;
		//rhs.second == lhs.second
		nibble_run const& left = lhs.second.second, right = rhs.second.second;
		return (left.get_nibble() < right.get_nibble() ||
		        (left.get_nibble() == right.get_nibble() &&
		         left.get_count() > right.get_count()));
	}
};

struct Compare_node
{
	bool operator()(std::tr1::shared_ptr<node> const& lhs,
	                std::tr1::shared_ptr<node> const& rhs)
	{
		return *lhs > *rhs;
	}
};

void nemesis::decode_header(std::istream& Src, std::ostream& Dst,
                            Codemap& codemap)
{
	// storage for output value to decompression buffer
	size_t out_val = 0;

	// main loop. Header is terminated by the value of 0xFF
	for (size_t in_val = Read1(Src); in_val != 0xFF; in_val = Read1(Src))
	{
		// if most significant bit is set, store the last 4 bits and discard the rest
		if ((in_val & 0x80) != 0)
		{
			out_val = in_val & 0xf;
			in_val = Read1(Src);
		}

		nibble_run run(out_val, ((in_val & 0x70) >> 4) + 1);

		unsigned char code = Read1(Src), len = in_val & 0xf;
		// Read the run's code from stream.
		codemap[std::make_pair(code, len)] = run;
	}
}

void nemesis::decode_internal(std::istream& Src, std::ostream& Dst,
                              Codemap& codemap, size_t rtiles,
                              bool alt_out, int *endptr)
{
	// This buffer is used for alternating mode decoding.
	std::stringstream dst(std::ios::in|std::ios::out|std::ios::binary);

	// Set bit I/O streams.
	ibitstream<unsigned char> bits(Src);
	obitstream<unsigned char> out(dst);
	unsigned char code = bits.get(), len = 1;

	// When to stop decoding: number of tiles * $20 bytes per tile * 8 bits per byte.
	size_t total_bits = rtiles << 8, bits_written = 0;
	while (bits_written < total_bits)
	{
		if (code == 0x3f && len == 6)
		{
			// Bit pattern %111111; inline RLE.
			// First 3 bits are repetition count, followed by the inlined nibble.
			size_t cnt    = bits.read(3) + 1, nibble = bits.read(4);
			bits_written += cnt * 4;

			// Write single nibble if needed.
			if ((cnt & 1) != 0)
				out.write(nibble, 4);

			// Now write pairs of nibbles.
			cnt >>= 1;
			nibble |= (nibble << 4);
			for (size_t i = 0; i < cnt; i++)
				out.write(nibble, 8);

			if (bits_written >= total_bits)
				break;
			
			// Read next bit, replacing previous data.
			code = bits.get();
			len = 1;
		}
		else
		{
			// Find out if the data so far is a nibble code.
			Codemap::const_iterator it = codemap.find(std::make_pair(code, len));
			if (it != codemap.end())
			{
				// If it is, then it is time to output the encoded nibble run.
				nibble_run const& run = it->second;
				size_t nibble = run.get_nibble();
				size_t cnt    = run.get_count();
				bits_written += cnt * 4;

				// Write single nibble if needed.
				if ((cnt & 1) != 0)
					out.write(nibble, 4);

				// Now write pairs of nibbles.
				cnt >>= 1;
				nibble |= (nibble << 4);
				for (size_t i = 0; i < cnt; i++)
					out.write(nibble, 8);

				if (bits_written >= total_bits)
					break;

				// Read next bit, replacing previous data.
				code = bits.get();
				len = 1;
			}
			else
			{
				// Read next bit and append to current data.
				code = (code << 1) | bits.get();
				len++;
			}
		}
	}

	if (endptr)
		*endptr = Src.tellg();

	// Write out any remaining bits, padding with zeroes.
	out.flush();

	if (alt_out)
	{
		// For alternating decoding, we must now incrementally XOR and output
		// the lines.
		dst.seekg(0);
		dst.clear();
		unsigned long in = LittleEndian::Read4(dst);
		LittleEndian::Write4(Dst, in);
		while (dst.tellg() < rtiles << 5)
		{
			in ^= LittleEndian::Read4(dst);
			LittleEndian::Write4(Dst, in);
		}
	}
	else
		Dst.write(dst.str().c_str(), rtiles << 5);
}

bool nemesis::decode(std::istream& Src, std::ostream& Dst, std::streampos Location,
                     int *endptr)
{
	Src.seekg(Location);

	Codemap codemap;
	size_t rtiles = BigEndian::Read2(Src);
	// sets the output mode based on the value of the first bit
	bool alt_out = (rtiles & 0x8000) != 0;
	rtiles &= 0x7fff;

	if (rtiles > 0)
	{
		decode_header(Src, Dst, codemap);
		decode_internal(Src, Dst, codemap, rtiles, alt_out, endptr);
	}
	else if (endptr)
		*endptr = Src.tellg();
	return true;
}

static size_t estimate_file_size
	(
	 std::map<nibble_run, std::pair<size_t, unsigned char> >& tempcodemap,
	 std::map<nibble_run,size_t>& counts
	)
{
	// We now compute the final file size for this code table.
	// 2 bytes at the start of the file, plus 1 byte at the end of the
	// code table.
	size_t tempsize_est = 3 * 8;
	size_t last = 0xff;
	// Start with any nibble runs with their own code.
	for (std::map<nibble_run, std::pair<size_t, unsigned char>
	             >::iterator it = tempcodemap.begin();
		 it != tempcodemap.end(); ++it)
	{
		// Each new nibble needs an extra byte.
		if (last != it->first.get_nibble())
		{
			tempsize_est += 8;
			// Be sure to SET the last nibble to the current nibble... this
			// fixes a bug that caused file sizes to increase in some cases.
			last = it->first.get_nibble();
		}
		// 2 bytes per nibble run in the table.
		tempsize_est += 2 * 8;
		// How many bits this nibble run uses in the file.
		tempsize_est += counts[it->first] * it->second.second;
	}

	// Supplementary code map for the nibble runs that can be broken up into
	// shorter nibble runs with a smaller bit length than inlining.
	std::map<nibble_run, std::pair<size_t, unsigned char> > supcodemap;
	// Now we will compute the size requirements for inline nibble runs.
	for (std::map<nibble_run,size_t>::iterator it = counts.begin();
		 it != counts.end(); ++it)
	{
		// Find out if this nibble run has a code for it.
		std::map<nibble_run, std::pair<size_t, unsigned char >
		        >::iterator it2 = tempcodemap.find(it->first);
		if (it2 == tempcodemap.end())
		{
			// Nibble run does not have its own code. We need to find out if
			// we can break it up into smaller nibble runs with total code
			// size less than 13 bits or if we need to inline it (13 bits).
			if (it->first.get_count() == 0)
				// If this is a nibble run with zero repeats, we can't break
				// it up into smaller runs, so we inline it.
				tempsize_est += (6 + 7) * it->second;
			else if (it->first.get_count() == 1)
			{
				// We stand a chance of breaking the nibble run.
				
				// This case is rather trivial, so we hard-code it.
				// We can break this up only as 2 consecutive runs of a nibble
				// run with count == 0.
				nibble_run trg(it->first.get_nibble(),0);
				it2 = tempcodemap.find(trg);
				if (it2 == tempcodemap.end() || it2->second.second > 6)
				{
					// The smaller nibble run either does not have its own code
					// or it results in a longer bit code when doubled up than
					// would result from inlining the run. In either case, we
					// inline the nibble run.
					tempsize_est += (6 + 7) * it->second;
				}
				else
				{
					// The smaller nibble run has a small enough code that it is
					// more efficient to use it twice than to inline our nibble
					// run. So we do exactly that, by adding a (temporary) entry
					// in the supplementary codemap, which will later be merged
					// into the main codemap.
					size_t code = it2->second.first;
					unsigned char len = it2->second.second;
					code = (code << len) | code;
					len <<= 1;
					tempsize_est += len * it->second;
					supcodemap.insert(std::make_pair(it->first, std::make_pair(code, 0x80|len)));
				}
			}
			else
			{
				// We stand a chance of breaking it the nibble run.
				
				// This is a linear optimization problem subjected to 2
				// constraints. If the number of repeats of the current nibble
				// run is N, then we have N dimensions.
				// Pointer to table of linear coefficients. This table has
				// N columns for each line.
				size_t *linear_coeffs;
				// Here are some hard-coded tables, obtained by brute-force:
				static size_t linear_coeffs2[ 2][2] = {{3,0}, {1,1}};
				static size_t linear_coeffs3[ 4][3] = {{4,0,0}, {2,1,0}, {1,0,1}, {0,2,0}};
				static size_t linear_coeffs4[ 6][4] = {{5,0,0,0}, {3,1,0,0}, {2,0,1,0},
				                                       {1,2,0,0}, {1,0,0,1}, {0,1,1,0}};
				static size_t linear_coeffs5[10][5] = {{6,0,0,0,0}, {4,1,0,0,0}, {3,0,1,0,0}, {2,2,0,0,0}, {2,0,0,1,0},
				                                       {1,1,1,0,0}, {1,0,0,0,1}, {0,3,0,0,0}, {0,1,0,1,0}, {0,0,2,0,0}};
				static size_t linear_coeffs6[14][6] = {{7,0,0,0,0,0}, {5,1,0,0,0,0}, {4,0,1,0,0,0}, {3,2,0,0,0,0}, {3,0,0,1,0,0},
				                                       {2,1,1,0,0,0}, {2,0,0,0,1,0}, {1,3,0,0,0,0}, {1,1,0,1,0,0}, {1,0,2,0,0,0},
				                                       {1,0,0,0,0,1}, {0,2,1,0,0,0}, {0,1,0,0,1,0}, {0,0,1,1,0,0}};
				static size_t linear_coeffs7[21][7] = {{8,0,0,0,0,0,0}, {6,1,0,0,0,0,0}, {5,0,1,0,0,0,0}, {4,2,0,0,0,0,0},
				                                       {4,0,0,1,0,0,0}, {3,1,1,0,0,0,0}, {3,0,0,0,1,0,0}, {2,3,0,0,0,0,0},
				                                       {2,1,0,1,0,0,0}, {2,0,2,0,0,0,0}, {2,0,0,0,0,1,0}, {1,2,1,0,0,0,0},
				                                       {1,1,0,0,1,0,0}, {1,0,1,1,0,0,0}, {1,0,0,0,0,0,1}, {0,4,0,0,0,0,0},
				                                       {0,2,0,1,0,0,0}, {0,1,2,0,0,0,0}, {0,1,0,0,0,1,0}, {0,0,1,0,1,0,0},
				                                       {0,0,0,2,0,0,0}};
				size_t n = it->first.get_count(), rows;
				// Get correct coefficient table:
				switch (n)
				{
					case 2: linear_coeffs = &linear_coeffs2[0][0]; rows =  2; break;
					case 3: linear_coeffs = &linear_coeffs3[0][0]; rows =  4; break;
					case 4: linear_coeffs = &linear_coeffs4[0][0]; rows =  6; break;
					case 5: linear_coeffs = &linear_coeffs5[0][0]; rows = 10; break;
					case 6: linear_coeffs = &linear_coeffs6[0][0]; rows = 14; break;
					case 7: linear_coeffs = &linear_coeffs7[0][0]; rows = 21; break;
				}

				unsigned char nibble = it->first.get_nibble();
				// Vector containing the code length of each nibble run, or 13
				// if the nibble run is not in the codemap.
				std::vector<size_t> runlen;
				// Init vector.
				for (size_t i = 0; i < n; i++)
				{
					// Is this run in the codemap?
					nibble_run trg(nibble,i);
					std::map<nibble_run, std::pair<size_t, unsigned char >
							>::iterator it3 = tempcodemap.find(trg);
					if (it3 == tempcodemap.end())
						// It is not.
						// Put inline length in the vector.
						runlen.push_back(6+7);
					else
						// It is.
						// Put code length in the vector.
						runlen.push_back(it3->second.second);
				}

				// Now go through the linear coefficient table and tally up
				// the total code size, looking for the best case.
				// The best size is initialized to be the inlined case.
				size_t best_size = 6 + 7;
				int best_line = -1;
				size_t base = 0;
				for (size_t i = 0; i < rows; i++, base += n)
				{
					// Tally up the code length for this coefficient line.
					size_t len = 0;
					for (size_t j = 0; j < n; j++)
					{
						size_t c = linear_coeffs[base + j];
						if (!c)
							continue;
						
						len += c * runlen[j];
					}
					// Is the length better than the best yet?
					if (len < best_size)
					{
						// If yes, store it as the best.
						best_size = len;
						best_line = base;
					}
				}
				// Have we found a better code than inlining?
				if (best_line >= 0)
				{
					// We have; use it. To do so, we have to build the code
					// and add it to the supplementary code table.
					size_t code = 0, len = 0;
					for (size_t i = 0; i < n; i++)
					{
						size_t c = linear_coeffs[best_line + i];
						if (!c)
							continue;
						// Is this run in the codemap?
						nibble_run trg(nibble,i);
						std::map<nibble_run, std::pair<size_t, unsigned char >
								>::iterator it3 = tempcodemap.find(trg);
						if (it3 != tempcodemap.end())
						{
							// It is; it MUST be, as the other case is impossible
							// by construction.
							for (int j = 0; j < c; j++)
							{
								len += it3->second.second;
								code <<= it3->second.second;
								code |= it3->second.first;
							}
						}
					}
					if (len != best_size)
					{
						// ERROR! DANGER! THIS IS IMPOSSIBLE!
						// But just in case...
						tempsize_est += (6 + 7) * it->second;
					}
					else
					{
						// By construction, best_size is at most 12.
						unsigned char c = best_size;
						// Add it to supplementary code map.
						supcodemap.insert(std::make_pair(it->first, std::make_pair(code, 0x80|c)));
						tempsize_est += best_size * it->second;
					}
				}
				else
					// No, we will have to inline it.
					tempsize_est += (6 + 7) * it->second;
			}
		}
	}
	tempcodemap.insert(supcodemap.begin(), supcodemap.end());

	// Round up to a full byte.
	if ((tempsize_est & 7) != 0)
		tempsize_est = (tempsize_est & ~7) + 8;

	return tempsize_est;
}

void nemesis::encode_internal(std::istream& Src, std::ostream& Dst, int mode, size_t sz)
{
	// Unpack source so we don't have to deal with nibble IO after.
	std::vector<unsigned char> unpack;
	for (size_t i = 0; i < sz; i++)
	{
		size_t c = Read1(Src);
		unpack.push_back((c & 0xf0) >> 4);
		unpack.push_back((c & 0x0f));
	}
	unpack.push_back(0xff);

	// Build RLE nibble runs, RLE-encoding the nibble runs as we go along.
	// Maximum run length is 8, meaning 7 repetitions.
	std::vector<nibble_run> rleSrc;
	std::map<nibble_run,size_t> counts;
	nibble_run curr(unpack[0], 0);
	for (size_t i = 1; i < unpack.size(); i++)
	{
		nibble_run next(unpack[i], 0);
		if (next.get_nibble() != curr.get_nibble() || curr.get_count() >= 7)
		{
			rleSrc.push_back(curr);
			counts[curr] += 1;
			curr = next;
		}
		else
			curr.set_count(curr.get_count()+1);
	}
	// No longer needed.
	unpack.clear();

	// We will use the Package-merge algorithm to build the optimal length-limited
	// Huffman code for the current file. To do this, we must map the current
	// problem onto the Coin Collector's problem.
	// Build the basic coin collection.
	std::priority_queue<std::tr1::shared_ptr<node>,
		     std::vector<std::tr1::shared_ptr<node> >, Compare_node> qt;
	for (std::map<nibble_run,size_t>::iterator it = counts.begin();
		 it != counts.end(); ++it)
		// No point in including anything with weight less than 2, as they
		// would actually increase compressed file size if used.
		if (it->second > 1)
			qt.push(std::tr1::shared_ptr<node>(new node(it->first, it->second)));

	// The base coin collection for the length-limited Huffman coding has
	// one coin list per character in length of the limmitation. Each coin list
	// has a constant "face value", and each coin in a list has its own
	// "numismatic value". The "face value" is unimportant in the way the code
	// is structured below; the "numismatic value" of each coin is the number
	// of times the underlying nibble run appears in the source file.
	
	// This will hold the Huffman code map.
	std::map<nibble_run, std::pair<size_t, unsigned char> > codemap;
	// Size estimate. This is used to build the optimal compressed file.
	size_t size_est = 0xffffffff;

	// We will solve the Coin Collector's problem several times, each time
	// ignoring more of the least frequent nibble runs. This allows us to find
	// *the* lowest file size.
	while (qt.size() > 1)
	{
		// Make a copy of the basic coin collection.
		std::priority_queue<std::tr1::shared_ptr<node>,
				 std::vector<std::tr1::shared_ptr<node> >, Compare_node> q0(qt);
		// Ignore the lowest weighted item. Will only affect the next iteration
		// of the loop. If it can be proven that there is a single global
		// minimum (and no local minima for file size), then this could be
		// simplified to a binary search.
		qt.pop();

		// We now solve the Coin collector's problem using the Package-merge
		// algorithm. The solution goes here.
		std::vector<std::tr1::shared_ptr<node> > solution;
		// This holds the packages from the last iteration.
		std::priority_queue<std::tr1::shared_ptr<node>,
				 std::vector<std::tr1::shared_ptr<node> >, Compare_node> q(q0);
		int target = (q0.size() - 1) << 8, idx = 0;
		while (target != 0)
		{
			// Gets lowest bit set in its proper place:
			int val = (target & -target), r = 1 << idx;
			// Is the current denomination equal to the least denomination?
			if (r == val)
			{
				// If yes, take the least valuable node and put it into the solution.
				solution.push_back(q.top());
				q.pop();
				target -= r;
			}

			// The coin collection has coins of values 1 to 8; copy from the
			// original in those cases for the next step.
			std::priority_queue<std::tr1::shared_ptr<node>,
				 std::vector<std::tr1::shared_ptr<node> >, Compare_node> q1;
			if (idx < 7)
				q1 = q0;
		
			// Split the current list into pairs and insert the packages into
			// the next list.
			while (q.size() > 1)
			{
				std::tr1::shared_ptr<node> child1 = q.top();
				q.pop();
				std::tr1::shared_ptr<node> child0 = q.top();
				q.pop();
				q1.push(std::tr1::shared_ptr<node>(new node(child0, child1)));
			}
			idx++;
			q = q1;
		}
		
		// The Coin Collector's problem has been solved. Now it is time to
		// map the solution back into the length-limited Huffman coding problem.
		
		// To do that, we iterate through the solution and count how many times
		// each nibble run has been used (remember that the coin collection had
		// multiple coins associated with each nibble run) -- this number is the
		// optimal bit length for the nibble run for the current coin collection.
		std::map<nibble_run, size_t> basesizemap;
		for (std::vector<std::tr1::shared_ptr<node> >::iterator it = solution.begin();
			 it != solution.end(); ++it)
			(*it)->traverse(basesizemap);

		// With the length-limited Huffman coding problem solved, it is now time
		// to build the code table. As input, we have a map associating a nibble
		// run to its optimal encoded bit length. We will build the codes using
		// the canonical Huffman code.
		
		// To do that, we must invert the size map so we can sort it by code size.
		std::multiset<size_t> sizeonlymap;
		// This map contains lots more information, and is used to associate
		// the nibble run with its optimal code. It is sorted by code size,
		// then by frequency of the nibble run, then by the nibble run.
		std::multiset<std::pair<size_t,std::pair<size_t,nibble_run> >,
			            Compare_size> sizemap;
		for (std::map<nibble_run, size_t>::iterator it = basesizemap.begin();
			 it != basesizemap.end(); ++it)
		{
			size_t size = it->second, count = counts[it->first];
			if ((6 + 7) * count < 16 + size * count)
				continue;
			sizeonlymap.insert(size);
			sizemap.insert(std::make_pair(size,
				                     std::make_pair(count, it->first)));
		}

		// We now build the canonical Huffman code table.
		// "base" is the code for the first nibble run with a given bit length.
		// "carry" is how many nibble runs were demoted to a higher bit length
		// at an earlier step.
		// "cnt" is how many nibble runs have a given bit length.
		size_t base = 0, carry = 0, cnt;
		// This vector contains the codes sorted by size.
		std::vector<std::pair<size_t,unsigned char> > codes;
		for (unsigned char i = 1; i <= 8; i++)
		{
			// How many nibble runs have the desired bit length.
			cnt = sizeonlymap.count(i) + carry;
			carry = 0;
			for (size_t j = 0; j < cnt; j++)
			{
				// Sequential binary numbers for codes.
				size_t code = base + j;
				size_t mask = (size_t(1) << i) - 1;
				// We do not want any codes composed solely of 1's or which
				// start with 111111, as that sequence is reserved.
				if ((i <= 6 && code == mask) ||
					(i > 6 && code == (mask & ~((size_t(1) << (i - 6)) - 1))))
				{
					// We must demote this many nibble runs to a longer code.
					carry = cnt - j;
					cnt = j;
					break;
				}
				else
					codes.push_back(std::make_pair(code, i));
			}
			// This is the beginning bit pattern for the next bit length.
			base = (base + cnt) << 1;
		}

		// With the canonical table build, the codemap can finally be built.
		std::map<nibble_run, std::pair<size_t, unsigned char> > tempcodemap;
		size_t pos = 0;
		for (std::multiset<std::pair<size_t,std::pair<size_t,nibble_run> >,
			            Compare_size>::iterator it = sizemap.begin();
			 it != sizemap.end() && pos < codes.size(); ++it, pos++)
			tempcodemap[it->second.second] = codes[pos];

		// We now compute the final file size for this code table.
		// 2 bytes at the start of the file, plus 1 byte at the end of the
		// code table.
		size_t tempsize_est = estimate_file_size(tempcodemap, counts);

		// Is this iteration better than the best?
		if (tempsize_est < size_est)
		{
			// If yes, save the codemap and file size.
			codemap = tempcodemap;
			size_est = tempsize_est;
		}
	}
	// Special case.
	if (qt.size() == 1)
	{
		std::map<nibble_run, std::pair<size_t, unsigned char> > tempcodemap;
		std::tr1::shared_ptr<node> child = qt.top();
		tempcodemap[child->get_value()] = std::pair<size_t, unsigned char>(0, 1);
		size_t tempsize_est = estimate_file_size(tempcodemap, counts);

		// Is this iteration better than the best?
		if (tempsize_est < size_est)
		{
			// If yes, save the codemap and file size.
			codemap = tempcodemap;
			size_est = tempsize_est;
		}
	}
	// This is no longer needed.
	counts.clear();

	// We now have a prefix-free code map associating the RLE-encoded nibble
	// runs with their code. Now we write the file.
	// Write header.
	BigEndian::Write2(Dst, (mode << 15) | (sz >> 5));
	unsigned char lastnibble = 0xff;
	for (std::map<nibble_run, std::pair<size_t, unsigned char> >::iterator it = codemap.begin();
	     it != codemap.end(); ++it)
	{
		nibble_run const& run = it->first;
		size_t code = (it->second).first, len = (it->second).second;
		// len with bit 7 set is a special device for further reducing file size, and
		// should NOT be on the table.
		if ((len & 0x80) != 0)
			continue;
		if (run.get_nibble() != lastnibble)
		{
			// 0x80 marks byte as setting a new nibble.
			Write1(Dst, 0x80 | run.get_nibble());
			lastnibble = run.get_nibble();
		}

		Write1(Dst, (run.get_count() << 4) | (len));
		Write1(Dst, code);
	}
	
	// Mark end of header.
	Write1(Dst, 0xff);

	// Time to write the encoded bitstream.
	obitstream<unsigned char> bits(Dst);

	// The RLE-encoded source makes for a far faster encode as we simply
	// use the nibble runs as an index into the map, meaning a quick binary
	// search gives us the code to use (if in the map) or tells us that we
	// need to use inline RLE.
	for (std::vector<nibble_run>::iterator it = rleSrc.begin();
	     it != rleSrc.end(); ++it)
	{
		nibble_run const& run = *it;
		std::map<nibble_run, std::pair<size_t, unsigned char> >::iterator val =
			codemap.find(run);
		if (val != codemap.end())
		{
			size_t code = (val->second).first;
			unsigned char len = (val->second).second;
			// len with bit 7 set is a device to bypass the code table at the
			// start of the file. We need to clear the bit here before writing
			// the code to the file.
			len &= 0x7f;
			// We can have codes in the 9-12 range due to the break up of large
			// inlined runs into smaller non-inlined runs. Deal with those high
			// bits first, if needed.
			if (len > 8)
			{
				bits.write(static_cast<unsigned char>((code >> 8) & 0xff), len-8);
				len = 8;
			}
			bits.write(static_cast<unsigned char>(code & 0xff), len);
		}
		else
		{
			bits.write(0x3f, 6);
			bits.write(run.get_count(), 3);
			bits.write(run.get_nibble(), 4);
		}
	}
	// Fill remainder of last byte with zeroes and write if needed.
	bits.flush();
}

bool nemesis::encode(std::istream& Src, std::ostream& Dst)
{
	// We will use these as output buffers, as well as an input/output
	// buffers for the padded Nemesis input.
	std::stringstream mode0buf(std::ios::in|std::ios::out|std::ios::binary),
	                  mode1buf(std::ios::in|std::ios::out|std::ios::binary),
	                  src(std::ios::in|std::ios::out|std::ios::binary);

	// Get original source length.
	Src.seekg(0, std::ios::end);
	std::streampos sz = Src.tellg();
	Src.seekg(0);

	// Copy to buffer.
	src << Src.rdbuf();

	// Is the source length a multiple of 32 bits?
	if ((sz & 0x1f) != 0)
	{
		// If not, pad it with zeroes until it is.
		while ((src.tellp() & 0x1f) != 0)
		{
			Write1(src, 0);
			sz += 1;
		}
	}

	// Now we will build the alternating bit stream for mode 1 compression.
	src.clear();
	src.seekg(0);

	std::string sin = src.str();
	for (size_t i = sin.size() - 4; i > 0; i -=4)
	{
		sin[i + 0] ^= sin[i - 4];
		sin[i + 1] ^= sin[i - 3];
		sin[i + 2] ^= sin[i - 2];
		sin[i + 3] ^= sin[i - 1];
	}
	std::stringstream alt(sin, std::ios::in|std::ios::out|std::ios::binary);

	// Reposition input streams to the beginning.
	src.clear();
	alt.clear();
	src.seekg(0);
	alt.seekg(0);

	// Encode in both modes.
	encode_internal(src, mode0buf, 0, sz);
	encode_internal(alt, mode1buf, 1, sz);

	// We will pick the smallest resulting stream as output.
	size_t sz0 = mode0buf.str().size(), sz1 = mode1buf.str().size();
	
	// Reposition output streams to the start.
	mode0buf.seekg(0);
	mode1buf.seekg(0);

	if (sz0 <= sz1)
		Dst << mode0buf.rdbuf();
	else
		Dst << mode1buf.rdbuf();

	// Pad to even size.
	if ((Dst.tellp() & 1) != 0)
		Dst.put(0);
	
	return true;
}
