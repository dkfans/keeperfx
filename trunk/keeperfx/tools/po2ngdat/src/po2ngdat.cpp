/******************************************************************************/
// PO translation to engine DAT files converter for KeeperFX
/******************************************************************************/
/** @file po2ngdat.cpp
 *     Program code file.
 * @par Purpose:
 *     Contains code to read UTF .PO translation files, convert texts
 *     into engine-readable encoding and write as .DAT files.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis <listom@gmail.com>
 * @date     5 Aug 2012 - 22 Sep 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdbool.h> // <cstdbool> requires c++11
#include <cstring>
#include <getopt.h>

#include "po2ngdat_version.h"

//#include "poparser.h"
#include "catalog.hpp"
#include "kfxenc.hpp"

enum {
    ERR_OK          =  0,
    ERR_CANT_OPEN   = -1, // fopen problem
    ERR_BAD_FILE    = -2, // incorrect file format
    ERR_NO_MEMORY   = -3, // malloc error
    ERR_FILE_READ   = -4, // fget/fread/fseek error
    ERR_FILE_WRITE  = -5, // fput/fwrite error
    ERR_LIMIT_EXCEED= -6, // static limit exceeded
    ERR_BAD_FORMAT  = -7, // input data format is invalid
};

int verbose = 0;

/** A struct closing non-global command line parameters */
struct ProgramOptions {
    char *fname_inp;
    char *fname_out;
    char *fname_enc;
};

void clear_prog_options(struct ProgramOptions *opts)
{
    opts->fname_inp = NULL;
    opts->fname_out = NULL;
    opts->fname_enc = NULL;
}

void free_prog_options(struct ProgramOptions *opts)
{
    free(opts->fname_inp);
    free(opts->fname_out);
    free(opts->fname_enc);
}

char *file_name_change_extension(const char *fname_inp,const char *ext)
{
    char *fname;
    char *tmp1,*tmp2;
    if (fname_inp == NULL)
      return NULL;
    fname = (char *)malloc(strlen(fname_inp)+strlen(ext)+2);
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
    const char *tmp2;
    char *tmp3;
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
    tmp3 = strrchr(fname,'.');
    if ((tmp3 != NULL) && (fname+1 < tmp3))
    {
        *tmp3 = '\0';
    }
    return fname;
}

char *file_name_strip_path(const char *fname_inp)
{
    char *fname;
    const char *tmp1;
    const char *tmp2;
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
            {"encfile", required_argument, 0, 'e'},
            {NULL,      0,                 0,'\0'}
        };
        /* getopt_long stores the option index here. */
        int c;
        int option_index = 0;
        c = getopt_long(argc, argv, "vo:e:", long_options, &option_index);
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
        case 'e':
            opts->fname_enc = strdup(optarg);
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
        opts->fname_out = file_name_change_extension(opts->fname_inp,"dat");
    }
    if (opts->fname_enc == NULL)
    {
        opts->fname_enc = strdup("char_encoding_tbl_eu.txt");
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
    printf("where <filename> should be the input PO file, and [options] are:\n");
    printf("    -v,--verbose             Verbose console output mode\n");
    printf("    -o<file>,--output<file>  Output DAT file name\n");
    printf("    -e<file>,--encfile<file> Encoding table input file name\n");
    return ERR_OK;
}

bool write_dat(const std::string& out_file, const std::vector<std::string> &data)
{
    std::fstream f;

    f.open(out_file.c_str(), std::fstream::out|std::fstream::binary);

    for (std::vector<std::string>::const_iterator it = data.begin(); it != data.end(); ++it)
    {
        f << *it << '\0';
    }
    f << '\0';

    f.close();
    return true;
}

int main(int argc, char* argv[])
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

    // Read PO file
    printf("Loading input file \"%s\"\n", opts.fname_inp);
    class Catalog * catalog = new Catalog();
    catalog->Load(opts.fname_inp, 0);
    std::vector<std::wstring> * translations;
    // Find translation lines
    printf("Retrieve sorted list of translations\n");
    translations = catalog->ToTranslationsVectorByRef(L"guitext",10000);
    delete catalog;
    // Show warnings for missing translations
    size_t i = 0;
    for (std::vector<std::wstring>::iterator it = translations->begin(); it != translations->end(); ++it)
    {
        if (((*it).length() <= 0) && (i != 201))
            printf("Warning: No reference to line %ld\n",(long)i);
        i++;
    }
    // Read character encoding
    printf("Loading encoding array \"%s\"\n", opts.fname_enc);
    UnicodeConvert * kfx_convert = new UnicodeConvert();
    kfx_convert->Load(opts.fname_enc, 0);
    // Convert strings to local encoding
    printf("Encoding translations\n");
    std::vector<std::string> * encoded = new std::vector<std::string>();
    for (std::vector<std::wstring>::iterator it = translations->begin(); it != translations->end(); ++it)
    {
        encoded->push_back(kfx_convert->EncodeU16String(*it));
    }
    delete kfx_convert;
    delete translations;
    printf("Writing output file \"%s\"\n",opts.fname_out);
    write_dat(opts.fname_out, *encoded);
    delete encoded;
    free_prog_options(&opts);
    return 0;
}
