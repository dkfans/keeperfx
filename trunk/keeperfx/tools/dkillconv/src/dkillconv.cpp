/******************************************************************************/
// .dkill conv - Dungeon Keeper Independent Level Layout  converter for KeeperFX
/******************************************************************************/
/** @file dkillconv.cpp
 *     Program code file.
 * @par Purpose:
 *     Contains code to read WAV files and merge them into one DAT file.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis <listom@gmail.com>
 * @date     15 Jan 2015 - 28 Jan 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <cstdio>
#include <cstddef>
#include <vector>
#include <climits>
#include <getopt.h>

#include <cstring>
#include <cmath>
#include <fstream>
#include <sstream>

#include "prog_options.h"
#include "dkillconv_version.h"

using namespace std;

int verbose = 0;

class WorkingSet
{
public:
    WorkingSet() {}
private:
};

std::string file_name_get_path(const std::string &fname_inp)
{
    size_t tmp1,tmp2;
    tmp1 = fname_inp.find_last_of('/');
    tmp2 = fname_inp.find_last_of('\\');
    if ((tmp1 == std::string::npos) || ((tmp2 != std::string::npos) && (tmp1 < tmp2)))
        tmp1 = tmp2;
    if (tmp1 != std::string::npos)
        return fname_inp.substr(0,tmp1);
    return "";
}

std::string file_name_strip_path(const std::string &fname_inp)
{
    size_t tmp1,tmp2;
    tmp1 = fname_inp.find_last_of('/');
    tmp2 = fname_inp.find_last_of('\\');
    if ((tmp1 == std::string::npos) || ((tmp2 != std::string::npos) && (tmp1 < tmp2)))
        tmp1 = tmp2;
    if (tmp1 != std::string::npos)
        return fname_inp.substr(tmp1+1);
    return fname_inp;
}

std::string file_name_change_extension(const std::string &fname_inp, const std::string &ext)
{
    std::string fname = fname_inp;
    size_t tmp2;
    tmp2 = fname.find_last_of('.');
    if (tmp2 != std::string::npos)
    {
        fname.replace(tmp2+1,fname.length()-tmp2,ext);
    } else
    {
        fname += "." + ext;
    }
    return fname;
}

/**
 * Returns length of opened file.
 * Value -1 means error.
 */
inline long file_length_opened(FILE *fp)
{
    long length;
    long lastpos;

    if (fp == NULL)
      return -1;
    lastpos = ftell(fp);
    if (fseek(fp, 0, SEEK_END) != 0)
      return -1;
    length = ftell(fp);
    fseek(fp, lastpos, SEEK_SET);
    return length;
}

/**
 * Reads 4-byte little-endian number from given buffer.
 */
inline long read_int32_le_buf(const unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    l += buff[2]<<16;
    l += buff[3]<<24;
    return l;
}

int load_command_line_options(ProgramOptions &opts, int argc, char *argv[])
{
    opts.clear();
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
               if (optarg) {
                   LogDbg("option %s with arg %s", long_options[option_index].name, optarg);
               } else {
                   LogDbg("option %s with no arg", long_options[option_index].name);
               }
               break;
        case 'v':
            verbose++;
            break;
        case 'o':
            opts.fname_out = optarg;
            break;
        case '?':
               // unrecognized option
               // getopt_long already printed an error message
        default:
            return false;
        }
    }
    // remaining command line arguments (not options)
    while (optind < argc)
    {
        opts.fname_inp = argv[optind++];
        break;
    }
    if ((optind < argc) || (opts.fname_inp.empty()))
    {
        LogErr("Incorrectly specified input file name.");
        return false;
    }
    // fill names that were not set by arguments
    if (opts.fname_out.length() < 1)
    {
        opts.fname_out = file_name_change_extension(file_name_strip_path(opts.fname_inp),"dat");
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
short show_usage(const std::string &fname)
{
    std::string xname = file_name_strip_path(fname.c_str());
    printf("usage:\n");
    printf("    %s [options] <filename>\n", xname.c_str());
    printf("where <filename> should be the input level file, and [options] are:\n");
    printf("    -v,--verbose             Verbose console output mode\n");
    printf("    -o<file>,--output<file>  Output sound bank file name\n");
    return ERR_OK;
}


int main(int argc, char* argv[])
{
    static ProgramOptions opts;
    if (!load_command_line_options(opts, argc, argv))
    {
        show_head();
        show_usage(argv[0]);
        return 11;
    }
    // start the work
    if (verbose)
        show_head();

    static WorkingSet ws;


    return 0;
}


