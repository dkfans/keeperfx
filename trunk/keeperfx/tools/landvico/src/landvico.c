/******************************************************************************/
// Land View files converter for KeeperFX
/******************************************************************************/
/** @file landvico.c
 *     Program code file.
 * @par Purpose:
 *     Contains code to read BMP files and convert them to 8bpp RAWs with
 *     special palette for KeeperFX land views.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Jul 2012 - 18 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>
#include "landvico_version.h"


#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

enum {
    ERR_OK          =  0,
    ERR_CANT_OPEN   = -1, // fopen problem
    ERR_BAD_FILE    = -2, // incorrect file format
    ERR_NO_MEMORY   = -3, // malloc error
    ERR_FILE_READ   = -4, // fget/fread/fseek error
    ERR_FILE_WRITE  = -5, // fput/fwrite error
    ERR_LIMIT_EXCEED= -6, // static limit exceeded
};

#ifdef __cplusplus
#pragma pack(1)
#endif


#ifdef __cplusplus
#pragma pack()
#endif

int verbose = 0;

/** A struct closing non-global command line parameters */
struct ProgramOptions {
    char *fname_inp;
    char *fname_out;
};

void clear_prog_options(struct ProgramOptions *opts)
{
    opts->fname_inp = NULL;
    opts->fname_out = NULL;
}

void free_prog_options(struct ProgramOptions *opts)
{
    free(opts->fname_inp);
    free(opts->fname_out);
}

/**
 * Writes 2-byte little-endian number into given buffer.
 */
void write_int16_le_buf (unsigned char *buff, unsigned short x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
}

/**
 * Writes 4-byte little-endian number into given buffer.
 */
void write_int32_le_buf (unsigned char *buff, unsigned long x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
    buff[2]=((x>>16)&255);
    buff[3]=((x>>24)&255);
}

/**
 * Reads 4-byte little-endian number from given buffer.
 */
inline long read_int32_le_buf (const unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    l += buff[2]<<16;
    l += buff[3]<<24;
    return l;
}

/**
 * Reads 2-byte little-endian number from given buffer.
 */
inline unsigned short read_int16_le_buf (const unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    return l;
}

/**
 * Returns length of opened file.
 * Value -1 means error.
 */
inline long file_length_opened (FILE *fp)
{
    long length;
    long lastpos;

    if (fp==NULL)
      return -1;
    lastpos = ftell (fp);
    if (fseek(fp, 0, SEEK_END) != 0)
      return -1;
    length = ftell(fp);
    fseek(fp, lastpos, SEEK_SET);
    return length;
}

char *file_name_change_extension(const char *fname_inp,const char *ext)
{
    char *fname;
    char *tmp1,*tmp2;
    if (fname_inp == NULL)
      return NULL;
    fname = malloc(strlen(fname_inp)+strlen(ext)+2);
    if (fname == NULL)
      return NULL;
    strcpy(fname,fname_inp);
    tmp1 = strrchr(fname, '/');
    tmp2 = strrchr(fname, '\\');
    if ((tmp1 == NULL) || (tmp1 < tmp2))
        tmp1 = tmp2;
    if (tmp1 == NULL)
        tmp1 = fname;
    tmp2 = strrchr(tmp1,'.');
    if ((tmp2 != NULL) && (tmp1+1 < tmp2))
    {
        sprintf(tmp2,".%s",ext);
    } else
    {
        tmp2 = fname + strlen(fname);
        sprintf(tmp2,".%s",ext);
    }
    return fname;
}

char *file_name_strip_to_body(const char *fname_inp)
{
    char *fname;
    const char *tmp1;
    char *tmp2;
    if (fname_inp == NULL)
      return NULL;
    tmp1 = strrchr(fname_inp, '/');
    tmp2 = strrchr(fname_inp, '\\');
    if ((tmp1 == NULL) || (tmp1 < tmp2))
        tmp1 = tmp2;
    if (tmp1 != NULL)
        tmp1++; // skip the '/' or '\\' char
    else
        tmp1 = fname_inp;
    fname = strdup(tmp1);
    if (fname == NULL)
      return NULL;
    tmp2 = strrchr(fname,'.');
    if ((tmp2 != NULL) && (fname+1 < tmp2))
    {
        *tmp2 = '\0';
    }
    return fname;
}

char *file_name_strip_path(const char *fname_inp)
{
    char *fname;
    const char *tmp1;
    char *tmp2;
    if (fname_inp == NULL)
      return NULL;
    tmp1 = strrchr(fname_inp, '/');
    tmp2 = strrchr(fname_inp, '\\');
    if ((tmp1 == NULL) || (tmp1 < tmp2))
        tmp1 = tmp2;
    if (tmp1 != NULL)
        tmp1++; // skip the '/' or '\\' char
    else
        tmp1 = fname_inp;
    fname = strdup(tmp1);
    if (fname == NULL)
      return NULL;
    return fname;
}

int load_command_line_options(struct ProgramOptions *opts, int argc, char *argv[])
{
    clear_prog_options(opts);
    while (1)
    {
        static struct option long_options[] = {
            {"verbose", no_argument,       0, 'v'},
            {"output",  required_argument, 0, 'o'},
            {NULL,      0,                 0,'\0'}
        };
        /* getopt_long stores the option index here. */
        int c;
        int option_index = 0;
        c = getopt_long(argc, argv, "vo:m:", long_options, &option_index);
        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c)
        {
        case 0:
               /* If this option set a flag, do nothing else now. */
               if (long_options[option_index].flag != 0)
                 break;
               printf ("option %s", long_options[option_index].name);
               if (optarg)
                 printf (" with arg %s", optarg);
               printf ("\n");
               break;
        case 'v':
            verbose = 1;
            break;
        case 'o':
            opts->fname_out = strdup(optarg);
            break;
        case '?':
               // unrecognized option
               // getopt_long already printed an error message
        default:
            return false;
        }
    }
    // remaining command line arguments (not options)
    if (optind < argc)
    {
        opts->fname_inp = strdup(argv[optind++]);
    }
    if ((optind < argc) || (opts->fname_inp == NULL))
    {
        printf("Incorrectly specified input file name.\n");
        return false;
    }
    // fill names that were not set by arguments
    if (opts->fname_out == NULL)
    {
        opts->fname_out = file_name_change_extension(opts->fname_inp,"raw");
    }
    return true;
}

short show_head(void)
{
    printf("\nLand View files converter for KeeperFX (LandViCo) %s\n",VER_STRING);
    printf("  Created by Tomasz Lis; %s\n",LEGAL_COPYRIGHT);
    printf("----------------------------------------\n");
    return ERR_OK;
}

/** Displays information about how to use this tool. */
short show_usage(char *fname)
{
    char *xname = file_name_strip_path(fname);
    printf("usage:\n");
    printf("    %s [options] <filename>\n", xname);
    free(xname);
    printf("where <filename> should be the input BMP file, and [options] are:\n");
    printf("    -v,--verbose             Verbose console output mode\n");
    printf("    -o<file>,--output<file>  Output RAW file name\n");
    printf("    -p<file>,--palette<file> Output PAL file name\n");
    return ERR_OK;
}

int main(int argc, char *argv[])
{
    static struct ProgramOptions opts;

    if (!load_command_line_options(&opts, argc, argv))
    {
        show_head();
        show_usage(argv[0]);
        return 11;
    }
    // start the work
    if (verbose)
        show_head();
  free_prog_options(&opts);
  return 0;
}
