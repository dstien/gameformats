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

// Decompress sub-files in source buffer.
uint stpk_decomp(stpk_Buffer *src, stpk_Buffer *dst, int maxPasses, int verbose)
{
	uchar passes, type;
	uint retval = 1, finalLen, i;

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
		STPK_ERR("Reached EOF while parsing file header\n");
		return 1;
	}

	for (i = 0; i < passes; i++) {
		STPK_NOVERBOSE("Pass %d/%d: ", i + 1, passes);
		STPK_VERBOSE1("\nPass %d/%d\n", i + 1, passes);

		type = src->data[src->offset++];
		stpk_getLength(src, &dst->len);
		STPK_VERBOSE1("  %-10s %d\n", "dstLen", dst->len);

		if ((dst->data = (uchar*)malloc(sizeof(uchar) * dst->len)) == NULL) {
			STPK_ERR("Error allocating memory for destination buffer. (%s)\n", strerror(errno));
			return 1;
		}

		switch (type) {
			case STPK_TYPE_RLE:
				STPK_VERBOSE1("  %-10s Run-length encoding\n", "type");
				retval = stpk_decompRLE(src, dst, verbose);
				break;
			case STPK_TYPE_VLE:
				STPK_VERBOSE1("  %-10s Variable-length encoding\n", "type");
				retval = stpk_decompVLE(src, dst, verbose);
				break;
			default:
				STPK_ERR("Error parsing source file. Expected type 1 (run-length) or 2 (variable-length), got %02X\n", type);
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
uint stpk_decompRLE(stpk_Buffer *src, stpk_Buffer *dst, int verbose)
{
	uint retval = 1, srcLen, i;
	uchar unk, escLen, esc[STPK_RLE_ESCLEN_MAX], escLookup[STPK_RLE_ESCLOOKUP_LEN];

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
		STPK_ERR("escLen & STPK_RLE_ESCLEN_MASK greater than max length %02X, got %02X\n", STPK_RLE_ESCLEN_MAX, escLen & STPK_RLE_ESCLEN_MASK);
		return 1;
	}

	// Read escape codes.
	for (i = 0; i < (escLen & STPK_RLE_ESCLEN_MASK); i++) esc[i] = src->data[src->offset++];
	STPK_VERBOSE_ARR(esc, escLen & STPK_RLE_ESCLEN_MASK, "esc");

	if (src->offset > src->len) {
		STPK_ERR("Reached end of source buffer while parsing run-length header\n");
		return 1;
	}

	// Generate escape code lookup table.
	for (i = 0; i < STPK_RLE_ESCLOOKUP_LEN; i++) escLookup[i] = 0;
	for (i = 0; i < (escLen & STPK_RLE_ESCLEN_MASK); i++) escLookup[esc[i]] = i + 1;
	STPK_VERBOSE_ARR(escLookup, STPK_RLE_ESCLOOKUP_LEN, "escLookup");

	STPK_NOVERBOSE("Run-length ");

	stpk_Buffer tmp, *finalSrc;
	tmp.data = NULL;

	if (!STPK_GET_FLAG(escLen, STPK_RLE_ESCLEN_NOSEQ)) {
		finalSrc = &tmp;

		tmp.len = dst->len;
		tmp.offset = 0;

		if ((tmp.data = (uchar*)malloc(sizeof(uchar) * tmp.len)) == NULL) {
			STPK_ERR("Error allocating memory for temporary RLE buffer. (%s)\n", strerror(errno));
			return 1;
		}

		if ((retval = stpk_rleDecodeSeq(src, &tmp, esc[STPK_RLE_ESCSEQ_POS], verbose))) {
			goto freeTmpBuf;
		}

		tmp.len = tmp.offset;
		tmp.offset = 0;
	}
	else {
		finalSrc = src;
	}

	retval = stpk_rleDecodeOne(finalSrc, dst, escLookup, verbose);

freeTmpBuf:
	free(tmp.data);

	return retval;
}

// Decode sequence runs.
uint stpk_rleDecodeSeq(stpk_Buffer *src, stpk_Buffer *dst, uchar esc, int verbose)
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
					STPK_ERR("Reached end of source buffer before finding sequence end escape code %02X\n", esc);
					return 1;
				}

				dst->data[dst->offset++] = cur;
			}

			rep = src->data[src->offset++] - 1; // Already wrote sequence once.
			STPK_VERBOSE2("%6d %6d %02X  %2.*X\n", src->offset, dst->offset, rep + 1, src->offset - seqOffset - 2, src->data[seqOffset]);

			while (rep--) {
				for (i = 0; i < (src->offset - seqOffset - 2); i++) {
					if (dst->offset >= dst->len) {
						STPK_ERR("Reached end of temporary buffer while writing repeated sequence\n");
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
				STPK_ERR("Reached end of temporary buffer while writing non-RLE byte\n");
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
uint stpk_rleDecodeOne(stpk_Buffer *src, stpk_Buffer *dst, uchar *esc, int verbose)
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
			STPK_ERR("Reached unexpected end of source buffer while decoding single-byte runs\n");
		}

		if (esc[cur] & 0xFF) {
			switch (esc[cur]) {
				case 1:
					rep = src->data[src->offset];
					cur = src->data[src->offset + 1];
					src->offset += 2;
					STPK_VERBOSE2("%6d %6d    %02X  %02X\n", src->offset, dst->offset, rep, cur);

					while (rep--) {
						if (dst->offset >= dst->len) {
							STPK_ERR("Reached end of temporary buffer while writing byte run\n");
							return 1;
						}

						dst->data[dst->offset++] = cur;
					}
					break;

				case 3:
					rep = src->data[src->offset] | src->data[src->offset + 1] << 8;
					cur = src->data[src->offset + 2];
					src->offset += 3;
					STPK_VERBOSE2("%6d %6d  %04X  %02X\n", src->offset, dst->offset, rep, cur);

					while (rep--) {
						if (dst->offset >= dst->len) {
							STPK_ERR("Reached end of temporary buffer while writing byte run\n");
							return 1;
						}

						dst->data[dst->offset++] = cur;
					}
					break;

				default:
					rep = esc[cur] - 1;
					cur = src->data[src->offset++];
					STPK_VERBOSE2("%6d %6d    %02X  %02X\n", src->offset, dst->offset, rep, cur);

					while (rep--) {
						if (dst->offset >= dst->len) {
							STPK_ERR("Reached end of temporary buffer while writing byte run\n");
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

// Decompress variable-length sub-file.
uint stpk_decompVLE(stpk_Buffer *src, stpk_Buffer *dst, int verbose)
{
	uchar widthsLen, alphabet[STPK_VLE_ALPH_LEN], symbols[STPK_VLE_ALPH_LEN], widths[STPK_VLE_ALPH_LEN];
	ushort esc1[STPK_VLE_ESCARR_LEN], esc2[STPK_VLE_ESCARR_LEN];
	uint i, widthsOffset, codesOffset, alphLen;

	widthsLen = src->data[src->offset++];
	widthsOffset = src->offset;

	STPK_VERBOSE1("  %-10s %d (unknown flag = %d)\n\n", "widthsLen", widthsLen & STPK_VLE_WDTLEN_MASK, STPK_GET_FLAG(widthsLen, STPK_VLE_WDTLEN_UNK));

	if (STPK_GET_FLAG(widthsLen, STPK_VLE_WDTLEN_UNK)) {
		STPK_ERR("Invalid source file. Unknown flag set in widthsLen\n");
		return 1;
	}
	else if ((widthsLen & STPK_VLE_WDTLEN_MASK) > STPK_VLE_WDTLEN_MAX) {
		STPK_ERR("widthsLen & STPK_VLE_WDTLEN_MASK greater than %02X, got %02X\n", STPK_VLE_WDTLEN_MAX, widthsLen & STPK_VLE_WDTLEN_MASK);
		return 1;
	}

	alphLen = stpk_vleGenEsc(src, esc1, esc2, widthsLen, verbose);

	if (alphLen > STPK_VLE_ALPH_LEN) {
		STPK_ERR("alphLen greater than %02X, got %02X\n", STPK_VLE_ALPH_LEN, alphLen);
		return 1;
	}

	// Read alphabet.
	for (i = 0; i < alphLen; i++) alphabet[i] = src->data[src->offset++];
	STPK_VERBOSE_ARR(alphabet, alphLen, "alphabet");

	if (src->offset > src->len) {
		STPK_ERR("Reached end of source buffer while parsing variable-length header\n");
		return 1;
	}

	codesOffset = src->offset;
	src->offset = widthsOffset;

	stpk_vleGenLookup(src, widthsLen, alphabet, symbols, widths, verbose);

	src->offset = codesOffset;

	return stpk_vleDecode(src, dst, alphabet, symbols, widths, esc1, esc2, verbose);
}

// Read widths to generate escape table and return length of alphabet.
uint stpk_vleGenEsc(stpk_Buffer *src, ushort *esc1, ushort *esc2, uint widthsLen, int verbose)
{
	uchar tmp;
	uint i, inc = 0, alphLen = 0;

	for (i = 0; i < widthsLen; i++) {
		inc *= 2;
		esc1[i] = alphLen - inc;
		tmp = src->data[src->offset++];

		inc += tmp;
		alphLen += tmp;

		esc2[i] = inc;

		STPK_VERBOSE1("  esc1[%02X] = %04X  esc2[%02X] = %04X\n", i, esc1[i], i, esc2[i]);
	}
	STPK_VERBOSE1("\n");

	return alphLen;
}

// Generate code lookup table for symbols and widths.
void stpk_vleGenLookup(stpk_Buffer *src, uint widthsLen, uchar *alphabet, uchar *symbols, uchar *widths, int verbose)
{
	uint i, j, width = 1, widthDistrLen = (widthsLen >= 8 ? 8 : widthsLen);
	uchar symbsWidth, symbsCount = STPK_VLE_BYTE_MSB, symbsCountLeft;

	// Distribution of symbols and widths.
	for (i = 0, j = 0; width <= widthDistrLen; width++, symbsCount >>= 1) {
		for (symbsWidth = src->data[src->offset++]; symbsWidth > 0; symbsWidth--, j++) {
			for (symbsCountLeft = symbsCount; symbsCountLeft; symbsCountLeft--, i++) {
				symbols[i] = alphabet[j];
				widths[i] = width;
			}
		}
	}
	STPK_VERBOSE_ARR(symbols, i, "symbols");

	// Pad widths.
	for (; i < STPK_VLE_ALPH_LEN; i++) widths[i] = STPK_VLE_ESC_WIDTH;
	STPK_VERBOSE_ARR(widths, i, "widths");
}

// Decode variable-length compression codes.
uint stpk_vleDecode(stpk_Buffer *src, stpk_Buffer *dst, uchar *alphabet, uchar *symbols, uchar *widths, ushort *esc1, ushort *esc2, int verbose)
{
	uchar curWidth = 8, nextWidth = 0, code, ind;
	ushort curWord = 0;
	uint progress = 0, done;

	curWord = src->data[src->offset++] << 8;
	curWord |= src->data[src->offset++];

	STPK_NOVERBOSE("Var-length [");

	STPK_VERBOSE1("Decoding compression codes... \n");
	STPK_VERBOSE2("\nsrcOff dstOff cW nW curWord               cd    Description\n");

	while (dst->offset < dst->len) {
		STPK_VERBOSE2("~~~~~~ ~~~~~~ ~~ ~~ ~~~~~~~~~~~~~~~~~~~~~ ~~    ~~~~~~~~~~~~~~~~~~\n");

		code = (curWord & 0xFF00) >> 8;
		STPK_VERBOSE_VLE("Shifted %d bits", nextWidth);

		nextWidth = widths[code];

		if (nextWidth > 8) {
			if (nextWidth != STPK_VLE_ESC_WIDTH) {
				STPK_ERR("Invalid escape value. nextWidth != %02X, got %02X\n", STPK_VLE_ESC_WIDTH, nextWidth);
				return 1;
			}

			code = (curWord & 0x00FF);
			curWord >>= 8;
			STPK_VERBOSE_VLE("Escaping");

			ind = 7;
			done = 0;

			while (!done) {
				if (!curWidth) {
					code = src->data[src->offset++];
					curWidth = 8;
					STPK_VERBOSE_VLE("Read %02X", src->data[src->offset - 1]);
				}

				curWord = (curWord << 1) + STPK_GET_FLAG(code, STPK_VLE_BYTE_MSB);
				code <<= 1;
				curWidth--;
				ind++;
				STPK_VERBOSE_VLE("ind = %02X", ind);

				if (ind >= STPK_VLE_ESCARR_LEN) {
					STPK_ERR("Escape array index out of bounds (%04X >= %04X)\n", ind, STPK_VLE_ESCARR_LEN);
					return 1;
				}

				if (curWord < esc2[ind]) {
					curWord += esc1[ind];

					if (curWord > 0xFF) {
						STPK_ERR("Alphabet index out of bounds (%04X > %04X)\n", curWord, STPK_VLE_ALPH_LEN);
						return 1;
					}

					dst->data[dst->offset++] = alphabet[curWord];
					STPK_VERBOSE_VLE("Wrote %02X", dst->data[dst->offset - 1]);

					done = 1;
				}
			}

			// Reset and continue.
			curWord = (code << curWidth) | src->data[src->offset++];
			nextWidth = 8 - curWidth;
			curWidth = 8;
			STPK_VERBOSE_VLE("Read %02X, returning", src->data[src->offset - 1]);
		}
		else {
			dst->data[dst->offset++] = symbols[code];
			STPK_VERBOSE_VLE("Wrote %02X", dst->data[dst->offset - 1]);

			if (curWidth < nextWidth) {
				curWord <<= curWidth;
				STPK_VERBOSE_VLE("Shifted %d bits", curWidth);

				nextWidth -= curWidth;
				curWidth = 8;

				curWord |= src->data[src->offset++];
				STPK_VERBOSE_VLE("Read %02X", src->data[src->offset - 1]);
			}
		}

		curWord <<= nextWidth;
		curWidth -= nextWidth;

		if ((src->offset - 1) > src->len && dst->offset < dst->len) {
			STPK_ERR("Reached unexpected end of source buffer while decoding variable-length compression codes\n");
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
		STPK_WARN("Variable-length decoding finished with unprocessed data left in source buffer (%d bytes left)\n", src->len - src->offset);
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
	int i;
	for (i = 0; i <  16; i++) stpk_b16[(16 - 1) - i] = '0' + STPK_GET_FLAG(val, (1 << i));
	stpk_b16[i] = 0;

	return stpk_b16;
}

// Print formatted array. Used in verbose output.
void stpk_printArray(uchar *arr, uint len, char *name)
{
	int i = 0;

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

