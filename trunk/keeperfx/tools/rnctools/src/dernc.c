/******************************************************************************/
/** @file dernc.c
 * RNC decompression support.
 * @par Purpose:
 *   Compiled normally, this file is a well-behaved, re-entrant code
 *   module exporting only `rnc_ulen', `rnc_unpack' and `rnc_error'.
 *   Compiled with MAIN_DERNC defined, it's a standalone program which will
 *   decompress argv[1] into argv[2].
 * @par Comment:
 *   in/out buffers should have 8 redundant "safe bytes" at end.
 * @author   Tomasz Lis
 * @author   Jon Skeet
 * @date     14 Oct 1997 - 22 Jul 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

#define INTERNAL
#include "lblogging.h"
#include "lbrncbase.h"

int main_unpack (char *pname, char *iname, char *oname);
int copy_file (char *iname, char *oname);

/**
 * Shows usage if building stand-alone DeRNC tool.
 * @param fname Name of the executable file.
 * @return Returns 1 on success.
 */
short show_usage(char *fname)
{
    printf("usage:\n");
    printf("    %s <files>\n", fname);
    printf(" or\n");
    printf("    %s -o <infile> <outfile>\n", fname);
    return 1;
}

/**
 * Main function if building stand-alone DeRNC tool.
 * @param argc Command line arguments count.
 * @param argv Command line arguments vector.
 * @return Returns 0 on success.
 */
int main(int argc, char **argv)
{
    int mode=0;
    int i;

    printf("\nPRO-PACK's alternate RNC files decompressor\n");
    printf("-------------------------------\n");

    if (argc<2)
    {
        show_usage(*argv);
        return 0;
    }
    for (i=1; i < argc; i++) {
      if (!strcmp (argv[i], "-o"))
        mode=i;
    }
    if (mode && argc != 4)
    {
        show_usage(*argv);
        return 1;
    }
    switch (mode)
    {
    case 0:
        for (i=1; i < argc; i++)
        {
          printf("Extracting %s...\n",argv[i]);
          if (main_unpack(*argv, argv[i], argv[i]))
            return 1;
        }
        return 0;
    case 1:
        printf("Extracting %s to %s...\n",argv[2],argv[3]);
        return main_unpack(*argv, argv[2], argv[3]);
    case 2:
        printf("Extracting %s to %s...\n",argv[1],argv[3]);
        return main_unpack(*argv, argv[1], argv[3]);
    case 3:
        printf("Extracting %s to %s...\n",argv[1],argv[2]);
        return main_unpack(*argv, argv[1], argv[2]);
    default:
        ERRORF("Internal fault.");
    }
    return 1;
}

/**
 * Decompresses single file if building stand-alone DeRNC tool.
 * @param pname Name of the program executable.
 * @param iname File name of the compressed input file.
 * @param oname File name of the decompressed output.
 * @return Returns 0 on success. On error prints a message
 *     and returns nonzero value.
 */
int main_unpack (char *pname, char *iname, char *oname)
{
    FILE *ifp, *ofp;
    long plen, ulen;
    void *packed, *unpacked;
    char buffer[4];
    long leeway;

    ifp = fopen(iname, "rb");
    if (!ifp)
    {
        ERRORF("%s: %s",iname,strerror(errno));
        return 1;
    }
    //Checking if the file is RNC
    fseek (ifp, 0L, SEEK_END);
    plen = ftell (ifp);
    rewind (ifp);
    if (plen < 4) // Can't be an RNC file
    {
        if (strcmp (iname, oname))
            return copy_file (iname, oname);
        return 0;
    }
    fread(buffer, 1, 4, ifp);
    if (strncmp(buffer, "RNC", 3) != 0)
    {
      fclose (ifp);
      if (strcmp(iname, oname) != 0)
        return copy_file(iname, oname);
      return 0;
    }
    rewind(ifp);
    //Reading compressed data, 8 bytes in buffer are for safety
    packed = malloc(plen+8);
    if (packed==NULL)
    {
        fclose(ifp);
        ERRORF("%s: %s",pname,strerror(errno));
        return 1;
    }
    unsigned long rdlen;
    rdlen=fread(packed, 1, plen, ifp);
    fclose(ifp);
    //Getting unpacked file size & allocating space
    ulen = rnc_ulen(packed);
    if (ulen < 0)
    {
        free(packed);
        if (ulen == -1) // File wasn't RNC to start with
          return 0;
        printf("Error: %s\n", rnc_error (ulen));
        return 1;
    }

    //Creating output buffer, 8 bytes are for safety
    unpacked = malloc(ulen+8);
    if (unpacked == NULL)
    {
        free(packed);
        ERRORF("%s: %s",pname,strerror(errno));
        return 1;
    }

    //Do the decompression
    ulen = rnc_unpack (packed, unpacked, RNC_IGNORE_NONE, &leeway);
    if (ulen < 0)
    {
    printf("%s\n", rnc_error(ulen));
    return 1;
    }

    //Write results to a file
    ofp = fopen(oname, "wb");
    if (!ofp)
    {
        ERRORF("%s: %s",oname,strerror(errno));
        return 1;
    }

    fwrite (unpacked, 1, ulen, ofp);
    fclose (ofp);

    free (unpacked);
    free (packed);

    return 0;
}

/**
 * Copies single file if building stand-alone DeRNC tool.
 * @param iname Source file name.
 * @param oname Destination file name.
 * @return Returns 0 on success. On error prints a message
 *     and returns nonzero value.
 */
int copy_file (char *iname, char *oname)
{
    char *sysbuf;

    sysbuf = malloc(strlen(iname)+strlen(oname)+6);
    if (sysbuf == NULL)
    {
      ERRORF("Out of memory.");
      return 1;
    }
    strcpy(sysbuf, "cp ");
    strcat(sysbuf, iname);
    strcat(sysbuf, " ");
    strcat(sysbuf, oname);
    system(sysbuf);
    free(sysbuf);
    return 0;
}
