#pragma once

#include <string>
#include <vector>
#include <cstdio>

extern int verbose;

#define LogMsg(format,args...) std::fprintf(stdout,format "\n", ## args)
#define LogDbg(format,args...) if (verbose) std::fprintf(stdout,format "\n", ## args)
#define LogErr(format,args...) std::fprintf(stderr,format "\n", ## args)

enum {
    ERR_OK          =  0,
    ERR_CANT_OPEN   = -1, // fopen problem
    ERR_BAD_FILE    = -2, // incorrect file format
    ERR_NO_MEMORY   = -3, // malloc error
    ERR_FILE_READ   = -4, // fget/fread/fseek error
    ERR_FILE_WRITE  = -5, // fput/fwrite error
    ERR_LIMIT_EXCEED= -6, // static limit exceeded
};

/** A class closing non-global command line parameters */
class ProgramOptions {
public:
    ProgramOptions()
    {
        clear();
    }
    void clear()
    {
        fname_inp.clear();
        fname_out.clear();
    }
    std::string fname_inp;
    std::string fname_out;
};

