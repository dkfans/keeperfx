/******************************************************************************/
/** @file rnc.c
 * ProPak/RNC compression support.
 * @par Purpose:
 *   Compiled normally, this file is a well-behaved, re-entrant code
 *   module exporting only the compression routines.
 *   Compiled with MAIN_RNC defined, it's a standalone program which will
 *   compress argv[1] into argv[2].
 * @par Comment:
 *   Requires dernc.c to compute the leeway header field.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#ifndef COMPRESSOR
#define COMPRESSOR
#endif
#define INTERNAL
#include "lblogging.h"
#include "lbfileio.h"
#include "lbpathutil.h"
#include "lbrncpack.h"
#include "globals.h"
#include "rnc_version.h"

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
        c = getopt_long(argc, argv, "vo:", long_options, &option_index);
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
            verbose++;
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
        opts->fname_out = strdup(opts->fname_inp);
    }
    return true;
}

short show_head(void)
{
    printf("\n%s (%s) %s\n",PROGRAM_FULL_NAME,PROGRAM_NAME,VER_STRING);
    printf("  Created by %s; %s\n",PROGRAM_AUTHORS,LEGAL_COPYRIGHT);
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
    printf("where <filename> should be the input file, and [options] are:\n");
    printf("    -v,--verbose             Verbose console output mode\n");
    printf("    -o<file>,--output<file>  Output file name\n");
    return ERR_OK;
}

int main(int argc, char **argv)
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

    int ret;
    printf("Compressing %s to %s...\n",opts.fname_inp, opts.fname_out);
    ret = main_pack(argv[0], opts.fname_inp, opts.fname_out, verbose);
    free_prog_options(&opts);
    return ret;
}
