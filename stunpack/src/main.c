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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stunpack.h"

char *banner = STPK_NAME" "STPK_VERSION" - Stunts/4D [Sports] Driving game resource unpacker\n\n";
char *usage  = "Usage: %s [OPTIONS]... SOURCE-FILE [DESTINATION-FILE]\n";

void printHelp(char *progName);
int decompress(char *srcFileName, char *dstFileName, int passes, int verbose);

int main(int argc, char **argv)
{
	char *srcFileName = NULL, *dstFileName = NULL;
	int retval = 0, opt, passes = 0, verbose = 1, srcFileNameLen = 0;

	// Parse options.
	while ((opt = getopt(argc, argv, "p:hqv")) != -1) {
		switch (opt) {
			case 'p':
				passes = atoi(optarg);
				break;
			case 'h':
				printHelp(argv[0]);
				return 0;
			case 'q':
				verbose = 0;
				break;
			case 'v':
				verbose++;
				break;
			case '?':
				retval = 1;
		}
	}

	if ((argc == optind) | (argc - optind > 2) | retval) {
		fprintf(stderr, usage, argv[0]);
		fprintf(stderr, "Try \"%s -h\" for help.\n", argv[0]);
		return 1;
	}

	STPK_MSG(banner);

	// Max two addition params (file names).
	for (; optind < argc; optind++) {
		if (srcFileName == NULL) {
			srcFileName = argv[optind];
		}
		else if (dstFileName == NULL) {
			dstFileName = argv[optind];
		}
	}

	// Generate destination file name if omitted.
	if (dstFileName == NULL) {
		srcFileNameLen = strlen(srcFileName);
		if ((dstFileName = (char*)malloc(sizeof(char) * (srcFileNameLen + 5))) == NULL) {
			STPK_ERR("Error allocating memory for generated destination file name. (%s)\n", strerror(errno));
			return 1;
		}
		strcpy(dstFileName, srcFileName);
		strcat(dstFileName, ".out");
	}

	retval = decompress(srcFileName, dstFileName, passes, verbose);

	// Clean up.
	if (dstFileName != NULL && srcFileNameLen) {
		free(dstFileName);
	}

	return retval;
}

void printHelp(char *progName)
{
	printf(banner);

	printf(usage, progName);
	printf("  -p NUM   limit to NUM decompression passes\n");
	printf("  -v       verbose output\n");
	printf("  -vv      very verbose output\n");
	printf("  -q       no output\n");
	printf("  -h       print this text and exit\n\n");

	printf("Report bugs to <"STPK_BUGS">.\n");
}

int decompress(char *srcFileName, char *dstFileName, int passes, int verbose)
{
	uint retval = 1;
	FILE *srcFile, *dstFile;
	stpk_Buffer src, dst;

	src.data = dst.data = NULL;
	src.offset = dst.offset = 0;

	if ((srcFile = fopen(srcFileName, "rb")) == NULL) {
		STPK_ERR("Error opening source file \"%s\" for reading. (%s)\n", srcFileName, strerror(errno));
		return 1;
	}

	STPK_MSG("Reading file \"%s\"...\n", srcFileName);

	if (fseek(srcFile, 0, SEEK_END) != 0) {
		STPK_ERR("Error seeking for EOF in source file \"%s\". (%s)\n", srcFileName, strerror(errno));
		goto closeSrcFile;
	}

	if ((src.len = ftell(srcFile)) == -1) {
		STPK_ERR("Error getting EOF position in source file \"%s\". (%s)\n", srcFileName, strerror(errno));
		goto closeSrcFile;
	}

	if (src.len > STPK_MAX_SIZE) {
		STPK_ERR("Source file \"%s\" size (%d) exceeds max size (%d).\n", srcFileName, src.len, STPK_MAX_SIZE);
		goto closeSrcFile;
	}

	if (fseek(srcFile, 0, SEEK_SET) != 0) {
		STPK_ERR("Error seeking for start position in source file \"%s\". (%s)\n", srcFileName, strerror(errno));
		goto closeSrcFile;
	}

	if ((src.data = (uchar*)malloc(sizeof(uchar) * src.len)) == NULL) {
		STPK_ERR("Error allocating memory for source file \"%s\" content. (%s)\n", srcFileName, strerror(errno));
		goto closeSrcFile;
	}

	if (fread(src.data, sizeof(uchar), src.len, srcFile) != src.len) {
		STPK_ERR("Error reading source file \"%s\" content. (%s)\n", srcFileName, strerror(errno));
		goto freeBuffers;
	}

	retval = stpk_decomp(&src, &dst, passes, verbose);

	// Flush unpacked data to file.
	if (!retval) {
		STPK_MSG("Writing file \"%s\"... ", dstFileName);

		if ((dstFile = fopen(dstFileName, "wb")) == NULL) {
			STPK_ERR("Error opening destination file \"%s\" for writing. (%s)\n", dstFileName, strerror(errno));
			retval = 1;
			goto freeBuffers;
		}

		if (fwrite(dst.data, 1, dst.len, dstFile) != dst.len) {
			STPK_ERR("Error writing destination file \"%s\" content. (%s)\n", dstFileName, strerror(errno));
			retval = 1;
			goto closeDstFile;
		}

		STPK_MSG("Done!\n");

closeDstFile:
		if (fclose(dstFile) != 0) {
			STPK_ERR("Error closing destination file \"%s\". (%s)\n", dstFileName, strerror(errno));
			retval = 1;
		}
	}

freeBuffers:
	free(src.data);
	free(dst.data);

closeSrcFile:
	if (fclose(srcFile) != 0) {
		STPK_ERR("Error closing source file \"%s\". (%s)\n", srcFileName, strerror(errno));
		retval = 1;
	}

	return retval;
}

