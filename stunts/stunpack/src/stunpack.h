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

typedef enum { STPK_VER_AUTO, STPK_VER_STUNTS10, STPK_VER_STUNTS11 } stpk_Version;
typedef enum { STPK_LOG_INFO, STPK_LOG_WARN, STPK_LOG_ERR } stpk_LogType;
typedef void (*stpk_LogCallback)(stpk_LogType type, const char *msg, ...);
typedef void* (*stpk_AllocCallback)(size_t size);
typedef void (*stpk_DeallocCallback)(void *ptr);

typedef struct {
	uchar *data;
	uint  offset;
	uint  len;
} stpk_Buffer;

typedef struct {
	stpk_Buffer          src;
	stpk_Buffer          dst;
	stpk_Version         version;
	int                  maxPasses;
	int                  verbosity;
	stpk_LogCallback     logCallback;
	stpk_AllocCallback   allocCallback;
	stpk_DeallocCallback deallocCallback;
} stpk_Context;

stpk_Context stpk_init(stpk_Version version, int maxPasses, int verbosity, stpk_LogCallback logCallback, stpk_AllocCallback allocCallback, stpk_DeallocCallback deallocCallback);
void stpk_deinit(stpk_Context *ctx);
uint stpk_decomp(stpk_Context *ctx);

uint stpk_decompRLE(stpk_Context *ctx);
uint stpk_rleDecodeSeq(stpk_Context *ctx, uchar esc);
uint stpk_rleDecodeOne(stpk_Context *ctx, const uchar *escLookup);

uint stpk_decompHuff(stpk_Context *ctx);
uint stpk_huffGenOffsets(stpk_Context *ctx, uint levels, const uchar *leafNodesPerLevel, short *codeOffsets, ushort *totalCodes);
void stpk_huffGenPrefix(stpk_Context *ctx, uint levels, const uchar *leafNodesPerLevel, const uchar *alphabet, uchar *symbols, uchar *widths);
uint stpk_huffDecode(stpk_Context *ctx, const uchar *alphabet, const uchar *symbols, const uchar *widths, const short *codeOffsets, const ushort *totalCodes, int delta);

#endif
