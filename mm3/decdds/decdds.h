/*
 * decdds - Midtown Madness 3 CDDS extractor
 *
 * License: As is
 * Author:  Daniel Stien <daniel@stien.org>
 * URL:     https://github.com/dstien/gameformats
 */

#ifndef DECDDS_H
#define DECDDS_H

#include <stdint.h>

#define DECDDS_BANNER          "decdds - Midtown Madness 3 CDDS extractor (2013-09-17)\n\n"
#define DECDDS_USAGE           "Usage: decdds [-v] [-q] infile.cdds [outfile.dds]\n"

#define DECDDS_MAGIC           0x990F44C8
#define DECDDS_DDS_MAGIC       0x20534444
#define DECDDS_FLG_ALPHA       0x00000001
#define DECDDS_FLG_FOURCC      0x00000004
#define DECDDS_FLG_RGB         0x00000040
#define DECDDS_FMT_DXT1        0x31545844
#define DECDDS_FMT_DXT2        0x32545844
#define DECDDS_FMT_DXT3        0x33545844
#define DECDDS_FMT_DXT4        0x34545844
#define DECDDS_FMT_DXT5        0x35545844
#define DECDDS_CAP_CUBE        0x00000200
#define DECDDS_CAP_CUBE_POSX   0x00000400
#define DECDDS_CAP_CUBE_NEGX   0x00000800
#define DECDDS_CAP_CUBE_POSY   0x00001000
#define DECDDS_CAP_CUBE_NEGY   0x00002000
#define DECDDS_CAP_CUBE_POSZ   0x00004000
#define DECDDS_CAP_CUBE_NEGZ   0x00008000
#define DECDDS_CAP_VOLUME      0x00200000

#define DECDDS_BUFLEN          0x40
#define DECDDS_BUFMASK         (DECDDS_BUFLEN - 1)
#define DECDDS_TAB1_MAX        0x01FF
#define DECDDS_TAB2_BITS(x)    ((x) >> 12)
#define DECDDS_TAB2_CODE(x)    ((x) & 0x0FFF)
#define DECDDS_SEQ_LOOKBACK(x) (((x) - 0x100) >> 3)
#define DECDDS_SEQ_LENGTH(x)   (((x) - 0x100) & 7)
#define DECDDS_MAX(x, y)       ((x) > (y) ? (x) : (y))

#define DECDDS_ERR_USAGE        -1
#define DECDDS_ERR_MEM          -2
#define DECDDS_ERR_OPEN         -3
#define DECDDS_ERR_READ         -4
#define DECDDS_ERR_WRITE        -5
#define DECDDS_ERR_WRONGTYPE    -6
#define DECDDS_ERR_UNSUPPFMT    -7
#define DECDDS_ERR_EOFHDR       -8
#define DECDDS_ERR_EOFIMG       -9
#define DECDDS_ERR_INVALIDHDR  -10
#define DECDDS_ERR_INVALIDIMG  -11

// Context variables for decompression routine.
typedef struct decdds_ctx_t {
    const uint8_t  *srcptr;
    uint32_t        srcdword;
    int32_t         srcbits;

    const uint32_t *tab1;
    const uint16_t *tab2;

    uint32_t        numbits;
    uint32_t        lookupmask;
    uint32_t        idxcode;
    uint32_t        idxbits;

    uint8_t         buf[DECDDS_BUFLEN];
    uint32_t        buflen;
    uint32_t        bufend;
    uint32_t        bufpos;
} decdds_ctx_t;

// DDS file header. Must be parsed in order to determine data size.
typedef struct decdds_ddshdr_t {
    uint32_t magic;
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t plsize;
    uint32_t depth;
    uint32_t mips;
    uint32_t reserved1[11];

    struct {
        uint32_t size;
        uint32_t flags;
        uint32_t fourcc;
        uint32_t rgbbits;
        uint32_t rmask;
        uint32_t gmask;
        uint32_t bmask;
        uint32_t amask;
    } pxfmt;

    struct {
        uint32_t caps1;
        uint32_t caps2;
        uint32_t reserved[2];
    } caps;

    uint32_t reserved2;
} decdds_ddshdr_t;

void decdds_ctx_init(decdds_ctx_t *ctx, uint32_t offset, const uint8_t *srcptr);

void decdds_ctx_reset(decdds_ctx_t *ctx, int32_t numbits, const uint32_t *tab1, const uint16_t *tab2);

int decdds_decode(decdds_ctx_t *ctx, int numbytes, uint8_t *dst);

int decdds_extract(const uint8_t *srcData, uint32_t srcLen, uint8_t **dstData, uint32_t *dstLen, int verbosity);

#endif
