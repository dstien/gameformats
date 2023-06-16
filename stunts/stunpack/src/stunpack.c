/*
 * stunpack - Stunts/4D [Sports] Driving game resource unpacker
 * Copyright (C) 2008 Daniel Stien <daniel@stien.org>
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stunpack.h"

inline void stpk_getLength(stpk_Buffer *buf, uint *len);

// Decompress sub-files in source buffer.
uint stpk_decomp(stpk_Buffer *src, stpk_Buffer *dst, int maxPasses, int verbose, char *err)
{
	uchar passes, type, i;
	uint retval = 1, finalLen;

	passes = src->data[src->offset];
	if (STPK_GET_FLAG(passes, STPK_PASSES_RECUR)) {
		src->offset++;

		passes &= STPK_PASSES_MASK;
		STPK_VERBOSE1("  %-10s %d\n", "passes", passes);

		stpk_getLength(src, &finalLen);
		STPK_VERBOSE1("  %-10s %d\n", "finalLen", finalLen);
		STPK_VERBOSE1("    %-8s %d\n", "srcLen", src->len);
		STPK_VERBOSE1("    %-8s %.2f\n", "ratio", (float)finalLen / src->len);
	}
	else {
		passes = 1;
	}

	if (src->offset > src->len) {
		STPK_ERR2("Reached EOF while parsing file header\n");
		return 1;
	}

	for (i = 0; i < passes; i++) {
		STPK_NOVERBOSE("Pass %d/%d: ", i + 1, passes);
		STPK_VERBOSE1("\nPass %d/%d\n", i + 1, passes);

		type = src->data[src->offset++];
		stpk_getLength(src, &dst->len);
		STPK_VERBOSE1("  %-10s %d\n", "dstLen", dst->len);

		if ((dst->data = (uchar*)malloc(sizeof(uchar) * dst->len)) == NULL) {
			STPK_ERR2("Error allocating memory for destination buffer. (%s)\n", strerror(errno));
			return 1;
		}

		switch (type) {
			case STPK_TYPE_RLE:
				STPK_VERBOSE1("  %-10s Run-length encoding\n", "type");
				retval = stpk_decompRLE(src, dst, verbose, err);
				break;
			case STPK_TYPE_HUFF:
				STPK_VERBOSE1("  %-10s Huffman coding\n", "type");
				retval = stpk_decompHuff(src, dst, verbose, err);
				break;
			default:
				STPK_ERR2("Error parsing source file. Expected type 1 (run-length) or 2 (Huffman), got %02X\n", type);
				return 1;
		}

		if (retval) {
			return retval;
		}

		if (i + 1 == maxPasses && passes != maxPasses) {
			STPK_MSG("Parsing limited to %d decompression pass(es), aborting.\n", maxPasses);
			return 0;
		}

		// Destination buffer is source for next pass.
		if (i < (passes - 1)) {
			free(src->data);
			src->data = dst->data;
			src->len = dst->len;
			dst->data = NULL;
			src->offset = dst->offset = 0;
		}
	}

	return 0;
}

// Decompress run-length encoded sub-file.
uint stpk_decompRLE(stpk_Buffer *src, stpk_Buffer *dst, int verbose, char *err)
{
	uint retval = 1, srcLen, i;
	uchar unk, escLen, esc[STPK_RLE_ESCLEN_MAX], escLookup[STPK_RLE_ESCLOOKUP_LEN];
	stpk_Buffer tmp, *finalSrc;

	stpk_getLength(src, &srcLen);
	STPK_VERBOSE1("  %-10s %d\n", "srcLen", srcLen);

	unk = src->data[src->offset++];
	STPK_VERBOSE1("  %-10s %02X\n", "unk", unk);

	if (unk) {
		STPK_WARN("Unknown RLE header field (unk) is %02X, expected 0\n", unk);
	}

	escLen = src->data[src->offset++];
	STPK_VERBOSE1("  %-10s %d (no sequences = %d)\n\n", "escLen", escLen & STPK_RLE_ESCLEN_MASK, STPK_GET_FLAG(escLen, STPK_RLE_ESCLEN_NOSEQ));

	if ((escLen & STPK_RLE_ESCLEN_MASK) > STPK_RLE_ESCLEN_MAX) {
		STPK_ERR2("escLen & STPK_RLE_ESCLEN_MASK greater than max length %02X, got %02X\n", STPK_RLE_ESCLEN_MAX, escLen & STPK_RLE_ESCLEN_MASK);
		return 1;
	}

	// Read escape codes.
	for (i = 0; i < (escLen & STPK_RLE_ESCLEN_MASK); i++) esc[i] = src->data[src->offset++];
	STPK_VERBOSE_ARR(esc, escLen & STPK_RLE_ESCLEN_MASK, "esc");

	if (src->offset > src->len) {
		STPK_ERR2("Reached end of source buffer while parsing run-length header\n");
		return 1;
	}

	// Generate escape code lookup table where the index is the escape code
	// and the value is the escape code's positional property.
	for (i = 0; i < STPK_RLE_ESCLOOKUP_LEN; i++) escLookup[i] = 0;
	for (i = 0; i < (escLen & STPK_RLE_ESCLEN_MASK); i++) escLookup[esc[i]] = i + 1;
	STPK_VERBOSE_ARR(escLookup, STPK_RLE_ESCLOOKUP_LEN, "escLookup");

	STPK_NOVERBOSE("Run-length ");

	tmp.data = NULL;

	if (!STPK_GET_FLAG(escLen, STPK_RLE_ESCLEN_NOSEQ)) {
		finalSrc = &tmp;

		tmp.len = dst->len;
		tmp.offset = 0;

		if ((tmp.data = (uchar*)malloc(sizeof(uchar) * tmp.len)) == NULL) {
			STPK_ERR2("Error allocating memory for temporary RLE buffer. (%s)\n", strerror(errno));
			return 1;
		}

		if ((retval = stpk_rleDecodeSeq(src, &tmp, esc[STPK_RLE_ESCSEQ_POS], verbose, err))) {
			goto freeTmpBuf;
		}

		tmp.len = tmp.offset;
		tmp.offset = 0;
	}
	else {
		finalSrc = src;
	}

	retval = stpk_rleDecodeOne(finalSrc, dst, escLookup, verbose, err);

freeTmpBuf:
	free(tmp.data);

	return retval;
}

// Decode sequence runs.
uint stpk_rleDecodeSeq(stpk_Buffer *src, stpk_Buffer *dst, uchar esc, int verbose, char *err)
{
	uchar cur;
	uint progress = 0, seqOffset, rep, i;

	STPK_NOVERBOSE("[");

	STPK_VERBOSE1("Decoding sequence runs...    ");
	STPK_VERBOSE2("\n\nsrcOff dstOff rep seq\n");
	STPK_VERBOSE2("~~~~~~ ~~~~~~ ~~~ ~~~~~~~~\n");

	// We do not know the destination length for this pass, dst->len covers both RLE passes.
	while (src->offset < src->len) {
		cur = src->data[src->offset++];

		if (cur == esc) {
			seqOffset = src->offset;

			while ((cur = src->data[src->offset++]) != esc) {
				if (src->offset >= src->len) {
					STPK_ERR2("Reached end of source buffer before finding sequence end escape code %02X\n", esc);
					return 1;
				}

				dst->data[dst->offset++] = cur;
			}

			rep = src->data[src->offset++] - 1; // Already wrote sequence once.
			STPK_VERBOSE2("%6d %6d %02X  %2.*X\n", src->offset, dst->offset, rep + 1, src->offset - seqOffset - 2, src->data[seqOffset]);

			while (rep--) {
				for (i = 0; i < (src->offset - seqOffset - 2); i++) {
					if (dst->offset >= dst->len) {
						STPK_ERR2("Reached end of temporary buffer while writing repeated sequence\n");
						return 1;
					}

					dst->data[dst->offset++] = src->data[seqOffset + i];
				}
			}

		}
		else {
			dst->data[dst->offset++] = cur;
			STPK_VERBOSE2("%6d %6d     %02X\n", src->offset, dst->offset, cur);

			if (dst->offset > dst->len) {
				STPK_ERR2("Reached end of temporary buffer while writing non-RLE byte\n");
				return 1;
			}
		}

		// Progress bar.
		if (verbose && (verbose < 3) && ((src->offset * 100) / src->len) >= (progress * 25)) {
			printf("%4d%%", progress++ * 25);
		}
	}

	STPK_VERBOSE1("\n");
	STPK_NOVERBOSE("]   ");

	return 0;
}

// Decode single-byte runs.
uint stpk_rleDecodeOne(stpk_Buffer *src, stpk_Buffer *dst, uchar *esc, int verbose, char *err)
{
	uchar cur;
	uint progress = 0, rep;

	STPK_NOVERBOSE("[");

	STPK_VERBOSE1("Decoding single-byte runs... ");

	STPK_VERBOSE2("\n\nsrcOff dstOff   rep cur\n");
	STPK_VERBOSE2("~~~~~~ ~~~~~~ ~~~~~ ~~~\n");

	while (dst->offset < dst->len) {
		cur = src->data[src->offset++];

		if (src->offset > src->len) {
			STPK_ERR2("Reached unexpected end of source buffer while decoding single-byte runs\n");
			return 1;
		}

		if (esc[cur] & 0xFF) {
			switch (esc[cur]) {
				// Type 1: One-byte counter for repetitions
				case 1:
					rep = src->data[src->offset];
					cur = src->data[src->offset + 1];
					src->offset += 2;
					STPK_VERBOSE2("%6d %6d    %02X  %02X\n", src->offset, dst->offset, rep, cur);

					while (rep--) {
						if (dst->offset >= dst->len) {
							STPK_ERR2("Reached end of temporary buffer while writing byte run\n");
							return 1;
						}

						dst->data[dst->offset++] = cur;
					}
					break;

				// Type 2: Used for sequences. Serves no purpose here, but
				// would be handled by the default case if it were to occur.

				// Type 3: Two-byte counter for repetitions
				case 3:
					rep = src->data[src->offset] | src->data[src->offset + 1] << 8;
					cur = src->data[src->offset + 2];
					src->offset += 3;
					STPK_VERBOSE2("%6d %6d  %04X  %02X\n", src->offset, dst->offset, rep, cur);

					while (rep--) {
						if (dst->offset >= dst->len) {
							STPK_ERR2("Reached end of temporary buffer while writing byte run\n");
							return 1;
						}

						dst->data[dst->offset++] = cur;
					}
					break;

				// Type n: n repetitions
				default:
					rep = esc[cur] - 1;
					cur = src->data[src->offset++];
					STPK_VERBOSE2("%6d %6d    %02X  %02X\n", src->offset, dst->offset, rep, cur);

					while (rep--) {
						if (dst->offset >= dst->len) {
							STPK_ERR2("Reached end of temporary buffer while writing byte run\n");
							return 1;
						}

						dst->data[dst->offset++] = cur;
					}
			}
		}
		else {
			dst->data[dst->offset++] = cur;
			STPK_VERBOSE2("%6d %6d        %02X\n", src->offset, dst->offset, cur);
		}

		// Progress bar.
		if (verbose && (verbose < 3) && ((src->offset * 100) / src->len) >= (progress * 25)) {
			printf("%4d%%", progress++ * 25);
		}
	}

	STPK_VERBOSE1("\n");
	STPK_NOVERBOSE("]\n");

	if (src->offset < src->len) {
		STPK_WARN("RLE decoding finished with unprocessed data left in source buffer (%d bytes left)\n", src->len - src->offset);
	}

	return 0;
}

// Decompress Huffman coded sub-file.
uint stpk_decompHuff(stpk_Buffer *src, stpk_Buffer *dst, int verbose, char *err)
{
	uchar levels, delta, leafNodesPerLevel[STPK_HUFF_LEVELS_MAX], alphabet[STPK_HUFF_ALPH_LEN], symbols[STPK_HUFF_PREFIX_LEN], widths[STPK_HUFF_PREFIX_LEN];
	short codeOffsets[STPK_HUFF_LEVELS_MAX];
	ushort totalCodes[STPK_HUFF_LEVELS_MAX];
	uint i, alphLen;

	levels = src->data[src->offset++];
	delta = STPK_GET_FLAG(levels, STPK_HUFF_LEVELS_DELTA);
	levels &= STPK_HUFF_LEVELS_MASK;

	STPK_VERBOSE1("  %-10s %d\n", "levels", levels);
	STPK_VERBOSE1("  %-10s %d\n\n", "delta", delta);

	if ((levels & STPK_HUFF_LEVELS_MASK) > STPK_HUFF_LEVELS_MAX) {
		STPK_ERR2("Huffman tree levels greater than %d, got %d\n", STPK_HUFF_LEVELS_MAX, levels & STPK_HUFF_LEVELS_MASK);
		return 1;
	}

	for (i = 0; i < levels; i++) {
		leafNodesPerLevel[i] = src->data[src->offset++];
	}

	alphLen = stpk_huffGenOffsets(levels, leafNodesPerLevel, codeOffsets, totalCodes, verbose);

	if (alphLen > STPK_HUFF_ALPH_LEN) {
		STPK_ERR2("Alphabet longer than than %d, got %d\n", STPK_HUFF_ALPH_LEN, alphLen);
		return 1;
	}

	// Read alphabet.
	for (i = 0; i < alphLen; i++) alphabet[i] = src->data[src->offset++];
	STPK_VERBOSE_ARR(alphabet, alphLen, "alphabet");

	if (src->offset > src->len) {
		STPK_ERR2("Reached end of source buffer while parsing Huffman header\n");
		return 1;
	}

	stpk_huffGenPrefix(levels, leafNodesPerLevel, alphabet, symbols, widths, verbose);

	return stpk_huffDecode(src, dst, alphabet, symbols, widths, codeOffsets, totalCodes, verbose, err);
}

// Generate offset table for translating Huffman codes wider than 8 bits to alphabet indices.
uint stpk_huffGenOffsets(uint levels, uchar *leafNodesPerLevel, short *codeOffsets, ushort *totalCodes, int verbose)
{
	uint level, codes = 0, alphLen = 0;

	for (level = 0; level < levels; level++) {
		codes *= 2;
		codeOffsets[level] = alphLen - codes;

		codes += leafNodesPerLevel[level];
		alphLen += leafNodesPerLevel[level];

		totalCodes[level] = codes;

		STPK_VERBOSE1("  codeOffsets[%2d] = %6d  totalCodes[%2d] = %6d\n", level, codeOffsets[level], level, totalCodes[level]);
	}
	STPK_VERBOSE1("\n");

	return alphLen;
}

// Generate prefix table for direct lookup of Huffman codes up to 8 bits wide.
void stpk_huffGenPrefix(uint levels, uchar *leafNodesPerLevel, uchar *alphabet, uchar *symbols, uchar *widths, int verbose)
{
	uint prefix, alphabetIndex, width = 1, maxWidth = STPK_MIN(levels, STPK_HUFF_PREFIX_WIDTH);
	uchar leafNodes, totalNodes = STPK_HUFF_PREFIX_MSB, remainingNodes;

	// Fill all prefixes with data from last leaf node.
	for (prefix = 0, alphabetIndex = 0; width <= maxWidth; width++, totalNodes >>= 1) {
		for (leafNodes = leafNodesPerLevel[width - 1]; leafNodes > 0; leafNodes--, alphabetIndex++) {
			for (remainingNodes = totalNodes; remainingNodes; remainingNodes--, prefix++) {
				symbols[prefix] = alphabet[alphabetIndex];
				widths[prefix] = width;
			}
		}
	}
	STPK_VERBOSE_ARR(symbols, prefix, "symbols");

	// Pad with escape value for codes wider than 8 bits.
	for (; prefix < STPK_HUFF_ALPH_LEN; prefix++) widths[prefix] = STPK_HUFF_WIDTH_ESC;
	STPK_VERBOSE_ARR(widths, prefix, "widths");
}

// Decode Huffman codes.
uint stpk_huffDecode(stpk_Buffer *src, stpk_Buffer *dst, uchar *alphabet, uchar *symbols, uchar *widths, short *codeOffsets, ushort *totalCodes, int verbose, char *err)
{
	uchar readWidth = 8, curWidth = 0, code, level;
	ushort curWord = 0;
	uint progress = 0, done;

	curWord = src->data[src->offset++] << 8;
	curWord |= src->data[src->offset++];

	STPK_NOVERBOSE("Huffman    [");

	STPK_VERBOSE1("Decoding Huffman codes... \n");
	STPK_VERBOSE2("\nsrcOff dstOff rW cW curWord               cd    Description\n");

	while (dst->offset < dst->len) {
		STPK_VERBOSE2("~~~~~~ ~~~~~~ ~~ ~~ ~~~~~~~~~~~~~~~~~~~~~ ~~    ~~~~~~~~~~~~~~~~~~\n");

		code = (curWord & 0xFF00) >> 8;
		STPK_VERBOSE_HUFF("Shifted %d bits", curWidth);

		curWidth = widths[code];

		// If code is wider than 8 bits, read more bits and decode with offset table.
		if (curWidth > STPK_HUFF_PREFIX_WIDTH) {
			if (curWidth != STPK_HUFF_WIDTH_ESC) {
				STPK_ERR2("Invalid escape value. curWidth != %02X, got %02X\n", STPK_HUFF_WIDTH_ESC, curWidth);
				return 1;
			}

			code = (curWord & 0x00FF);
			curWord >>= STPK_HUFF_PREFIX_WIDTH;
			STPK_VERBOSE_HUFF("Escaping to offset table");

			for (level = STPK_HUFF_PREFIX_WIDTH, done = 0; !done; level++) {
				if (!readWidth) {
					code = src->data[src->offset++];
					readWidth = STPK_HUFF_PREFIX_WIDTH;
					STPK_VERBOSE_HUFF("Read %02X", src->data[src->offset - 1]);
				}

				curWord = (curWord << 1) + STPK_GET_FLAG(code, STPK_HUFF_PREFIX_MSB);
				code <<= 1;
				readWidth--;
				STPK_VERBOSE_HUFF("level = %d", level);

				if (level >= STPK_HUFF_LEVELS_MAX) {
					STPK_ERR2("Offset table out of bounds (%d >= %d)\n", level, STPK_HUFF_LEVELS_MAX);
					return 1;
				}

				if (curWord < totalCodes[level]) {
					curWord += codeOffsets[level];

					if (curWord > 0xFF) {
						STPK_ERR2("Alphabet index out of bounds (%04X > %04X)\n", curWord, STPK_HUFF_ALPH_LEN);
						return 1;
					}

					dst->data[dst->offset++] = alphabet[curWord];
					STPK_VERBOSE_HUFF("Wrote %02X using offset table", dst->data[dst->offset - 1]);

					done = 1;
				}
			}

			// Read another byte since the processed code was wider than a byte.
			curWord = (code << readWidth) | src->data[src->offset++];
			curWidth = 8 - readWidth;
			readWidth = 8;
			STPK_VERBOSE_HUFF("Read %02X", src->data[src->offset - 1]);
		}
		// Code is 8 bits wide or less, do direct prefix lookup.
		else {
			dst->data[dst->offset++] = symbols[code];
			STPK_VERBOSE_HUFF("Wrote %02X from prefix table", dst->data[dst->offset - 1]);

			if (readWidth < curWidth) {
				curWord <<= readWidth;
				STPK_VERBOSE_HUFF("Shifted %d bits", readWidth);

				curWidth -= readWidth;
				readWidth = 8;

				curWord |= src->data[src->offset++];
				STPK_VERBOSE_HUFF("Read %02X", src->data[src->offset - 1]);
			}
		}

		curWord <<= curWidth;
		readWidth -= curWidth;

		if ((src->offset - 1) > src->len && dst->offset < dst->len) {
			STPK_ERR2("Reached unexpected end of source buffer while decoding Huffman codes\n");
			return 1;
		}

		// Progress bar.
		if (verbose && (verbose < 3) && ((dst->offset * 100) / dst->len) >= (progress * 10)) {
			printf("%4d%%", progress++ * 10);
		}
	}

	STPK_NOVERBOSE("]\n");
	STPK_VERBOSE1("\n");

	if (src->offset < src->len) {
		STPK_WARN("Huffman decoding finished with unprocessed data left in source buffer (%d bytes left)\n", src->len - src->offset);
	}

	return 0;
}

// Read file length: WORD remainder + BYTE multiplier * 0x10000.
inline void stpk_getLength(stpk_Buffer *buf, uint *len)
{
	*len =  buf->data[buf->offset] | buf->data[buf->offset + 1] << 8; // Read remainder.
	*len += 0x10000 * buf->data[buf->offset + 2]; // Add multiplier.
	buf->offset += 3;
}

// Write bit values as string to stpk_b16. Used in verbose output.
char *stpk_stringBits16(ushort val)
{
	static char stpk_b16[16 + 1];
	int i;
	for (i = 0; i < 16; i++) stpk_b16[(16 - 1) - i] = '0' + STPK_GET_FLAG(val, (1 << i));
	stpk_b16[i] = 0;

	return stpk_b16;
}

// Print formatted array. Used in verbose output.
void stpk_printArray(uchar *arr, uint len, char *name)
{
	uint i = 0;

	printf("  %s[%02X]\n", name, len);
	printf("    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

	while (i < len) {
		if ((i % 0x10) == 0) printf(" %2X", i / 0x10);
		printf(" %02X", arr[i++]);
		if ((i % 0x10) == 0) printf("\n");
	}

	if ((i % 0x10) != 0) printf("\n");
	printf("\n");
}

