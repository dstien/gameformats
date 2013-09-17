/*
 * decdds - Midtown Madness 3 CDDS extractor
 *
 * License: As is
 * Author:  Daniel Stien <daniel@stien.org>
 * URL:     https://github.com/dstien/gameformats
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "decdds.h"

void panic(const char *fmt)
{
    fprintf(stderr, "%s", fmt);
    abort();
}

void usage()
{
    fprintf(stderr, DECDDS_USAGE);
    exit(DECDDS_ERR_USAGE);
}

int main(int argc, const char* argv[])
{
    char *srcFileName = NULL, *dstFileName = NULL;
    FILE *srcFile, *dstFile;
    uint8_t *srcData, *dstData = NULL;
    int32_t srcLen;
    uint32_t dstLen;
    int retval;
    int verbosity = 1;

    // Parse options.
    for (int i = 1; i < argc; ++i) {
        // Switches
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != 0; ++j) {
                switch (argv[i][j]) {
                    case 'h':
                    case '?':
                        printf(DECDDS_BANNER);
                        printf(DECDDS_USAGE"\n");
                        printf("Options:\n");
                        printf("  -v    increase verbosity\n");
                        printf("  -q    quiet\n");
                        printf("  -?    this helpful output\n\n");
                        return 0;

                    case 'v':
                        ++verbosity;
                        break;

                    case 'q':
                        verbosity = 0;
                        break;

                    default:
                        usage();
                }
            }
        }
        else if (srcFileName == NULL) {
            srcFileName = (char*)argv[i];
        }
        else if (dstFileName == NULL) {
            dstFileName = (char*)argv[i];
        }
        else {
            usage();
        }
    }

    if (verbosity) {
        printf(DECDDS_BANNER);
    }

    // No input file name given.
    if (srcFileName == NULL) {
        usage();
    }

    // Generate output file name if not specified.
    if (dstFileName == NULL) {
        char *srcExt = ".cdds";
        char *dstExt = ".dds";
        size_t lenSrcName = strlen(srcFileName);
        size_t lenSrcExt = strlen(srcExt);
        size_t lenDstExt = strlen(dstExt);

        dstFileName = (char*)malloc(lenSrcName + lenDstExt + 1);
        strncpy(dstFileName, srcFileName, lenSrcName + 1);

        // Replace ".cdds" extension with ".dds".
        if ((lenSrcName > lenSrcExt) && !strncmp(srcFileName + lenSrcName - lenSrcExt, srcExt, lenSrcExt)) {
            strncpy(dstFileName + lenSrcName - lenSrcExt, dstExt, lenDstExt + 1);
        }
        // Append ".dds" extension if input file don't ends with ".cdds".
        else {
            strncpy(dstFileName + lenSrcName, dstExt, lenDstExt + 1);
        }
    }

    if ((srcFile = fopen(srcFileName, "rb")) == NULL) {
        fprintf(stderr, "Error: Can't open input file \"%s\".\n", srcFileName);
        return DECDDS_ERR_OPEN;
    }

    if (fseek(srcFile, 0, SEEK_END) != 0) {
        panic("fseek() failed.\n");
    }

    if ((srcLen = ftell(srcFile)) == -1) {
        panic("ftell() failed.\n");
    }

    if (fseek(srcFile, 0, SEEK_SET) != 0) {
        panic("fseek() failed.\n");
    }

    if ((srcData = (uint8_t*)malloc(srcLen)) == NULL) {
        panic("malloc() failed.\n");
    }

    if (verbosity) {
        printf("Reading \"%s\" (%d bytes)\n", srcFileName, srcLen);
    }

    if (fread(srcData, 1, (uint32_t)srcLen, srcFile) != (uint32_t)srcLen) {
        fprintf(stderr, "Error: Can't read input file \"%s\".\n", srcFileName);
        return DECDDS_ERR_READ;
    }

    if (fclose(srcFile) != 0) {
        panic("fclose() failed.\n");
    }

    if ((retval = decdds_extract(srcData, (uint32_t)srcLen, &dstData, &dstLen, verbosity)) == 0) {
        if (verbosity) {
            printf("Writing \"%s\" (%d bytes)\n", dstFileName, dstLen);
        }

        if ((dstFile = fopen(dstFileName, "wb")) == NULL) {
            fprintf(stderr, "Error: Can't create output file \"%s\".\n", dstFileName);
            return DECDDS_ERR_OPEN;
        }

        if (fwrite(dstData, 1, dstLen, dstFile) != dstLen) {
            fprintf(stderr, "Error: Can't write output file \"%s\".\n", dstFileName);
            return DECDDS_ERR_WRITE;
        }

        if (fclose(dstFile) != 0) {
            panic("fclose failed.\n");
        }
    }
    else {
        switch (retval) {
            case DECDDS_ERR_WRONGTYPE:
                fprintf(stderr, "Error: Not a valid CDDS file\n");
                break;

            case DECDDS_ERR_MEM:
                fprintf(stderr, "Error: Couldn't allocate memory.\n");
                break;

            default:
                fprintf(stderr, "Error: Decoding failed.\n");
                break;
        }
    }

    free(srcData);
    free(dstData);

    return retval;
}
