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

#ifndef STPK_STUNPACK_H
#define STPK_STUNPACK_H

#define STPK_VERSION "0.1.0"
#define STPK_NAME    "stunpack"
#define STPK_BUGS    "daniel@stien.org"

#define STPK_MSG(msg, ...) if (verbose) printf(msg, ## __VA_ARGS__)
#define STPK_ERR1(msg, ...) if (verbose) fprintf(stderr, "\n" STPK_NAME ": " msg, ## __VA_ARGS__)
#define STPK_ERR2(msg, ...) STPK_ERR1(msg, ## __VA_ARGS__); \
					if (err != NULL) snprintf(err, 255, msg, ## __VA_ARGS__)
#define STPK_WARN(msg, ...) STPK_ERR1("(Warning) " msg, ## __VA_ARGS__);

#define STPK_NOVERBOSE(msg, ...) if (verbose == 1) printf(msg, ## __VA_ARGS__)
#define STPK_VERBOSE1(msg, ...)  if (verbose > 1)  printf(msg, ## __VA_ARGS__)
#define STPK_VERBOSE2(msg, ...)  if (verbose > 2)  printf(msg, ## __VA_ARGS__)
#define STPK_VERBOSE_ARR(arr, len, name) if (verbose > 1) stpk_printArray(arr, len, name)
#define STPK_VERBOSE_HUFF(msg, ...) STPK_VERBOSE2("%6d %6d %2d %2d %04X %s %02X -> " msg "\n", \
					src->offset, dst->offset, readWidth, curWidth, curWord, \
					stpk_stringBits16(curWord), code, ## __VA_ARGS__)

#define STPK_GET_FLAG(data, mask) ((data & mask) == mask)
#define STPK_MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define STPK_MAX_SIZE          0xFFFFFF
#define STPK_PASSES_MASK       0x7F
#define STPK_PASSES_RECUR      0x80

#define STPK_TYPE_RLE          0x01
#define STPK_TYPE_HUFF         0x02

#define STPK_RLE_ESCLEN_MASK   0x7F
#define STPK_RLE_ESCLEN_MAX    0x0A
#define STPK_RLE_ESCLEN_NOSEQ  0x80
#define STPK_RLE_ESCLOOKUP_LEN 0x100
#define STPK_RLE_ESCSEQ_POS    0x01

#define STPK_HUFF_LEVELS_MASK  0x7F
#define STPK_HUFF_LEVELS_MAX   0x10
#define STPK_HUFF_LEVELS_DELTA 0x80

#define STPK_HUFF_ALPH_LEN     0x100
#define STPK_HUFF_PREFIX_WIDTH 0x08
#define STPK_HUFF_PREFIX_LEN   (1 << STPK_HUFF_PREFIX_WIDTH)
#define STPK_HUFF_PREFIX_MSB   (1 << (STPK_HUFF_PREFIX_WIDTH - 1))
#define STPK_HUFF_WIDTH_ESC    0x40

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;

typedef struct {
	uchar *data;
	uint  offset;
	uint  len;
} stpk_Buffer;

uint stpk_decomp(stpk_Buffer *src, stpk_Buffer *dst, int maxPasses, int verbose, char *err);

uint stpk_decompRLE(stpk_Buffer *src, stpk_Buffer *dst, int verbose, char *err);
uint stpk_rleDecodeSeq(stpk_Buffer *src, stpk_Buffer *dst, uchar esc, int verbose, char *err);
uint stpk_rleDecodeOne(stpk_Buffer *src, stpk_Buffer *dst, uchar *esc, int verbose, char *err);

uint stpk_decompHuff(stpk_Buffer *src, stpk_Buffer *dst, int verbose, char *err);
uint stpk_huffGenOffsets(uint levels, uchar *leafNodesPerLevel, short *codeOffsets, ushort *totalCodes, int verbose);
void stpk_huffGenPrefix(uint levels, uchar *leafNodesPerLevel, uchar *alphabet, uchar *symbols, uchar *widths, int verbose);
uint stpk_huffDecode(stpk_Buffer *src, stpk_Buffer *dst, uchar *alphabet, uchar *symbols, uchar *widths, short *codeOffsets, ushort *totalCodes, int verbose, char *err);

char *stpk_stringBits16(ushort val);
void stpk_printArray(uchar *arr, uint len, char *name);

#endif
