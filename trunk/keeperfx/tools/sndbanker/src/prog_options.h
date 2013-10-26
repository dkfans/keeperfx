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

class SoundData {

};

#pragma pack(1)

class SampleEntry {
public:
    /** Name of the sound file the sample comes from. */
    char fname[18];
    /** Offset of the sample data in DAT file. */
    size_t data;
    /** Unknown1. */
    long unkn1;
    /** Size of the sample file. */
    size_t length;
    /** Unknown3. */
    short unkn3;
};

#pragma pack()

class SoundFile {
public:
    SoundFile(const std::string &nname):
        fname(nname) { };
    /** Name of the source sound sample file */
    std::string fname;
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
        inp.clear();
        fname_lst.clear();
        fname_out.clear();
    }
    std::vector<SoundFile> inp;
    std::string fname_lst;
    std::string fname_out;
};

