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

enum {
    OutFmt_RAW = 0,
    OutFmt_HSPR,
    OutFmt_SSPR,
    OutFmt_JSPR,
};

enum {
    Batch_NONE = 0,
    Batch_FILELIST,
    Batch_ANIMLIST,
};

enum {
    DfsAlg_FldStnbrg = 0,
    DfsAlg_JrvJdcNnk,
    DfsAlg_Stucki,
    DfsAlg_Burkes,
    DfsAlg_Fan,
    DfsAlg_Sierra3,
    DfsAlg_Sierra2,
    DfsAlg_Sierra24A,
    DfsAlg_Atkinson,
    DfsAlg_ShiauFan4,
    DfsAlg_ShiauFan5,
};
/*
"Floyd-Steinberg"
"Jarvis, Judice, Ninke"
"Stucki"
"Burkes"
"Fan"
"Sierra 3"
"Sierra 2"
"Sierra 2-4A (Filter Lite)"
"Atkinson"
"Shiau-Fan (4-cell)"
"Shiau-Fan (5-cell)"
*/

class ImageArea {
public:
    ImageArea(const std::string &nname, int nanum=-1, int nx=-1, int ny=-1, int nw=-1, int nh=-1, int nfd0=0, int nfd1=0, int nfd2=0, int nfd3=0):
        fname(nname),anum(nanum),x(nx),y(ny),w(nw),h(nh) {fd[0]=nfd0;fd[1]=nfd1;fd[2]=nfd2;fd[3]=nfd3;};
    /** Name of the source image file */
    std::string fname;
    /** Animation index, if this image is a part of animated sequence */
    int anum;
    /** X and Y coords of the start of sprite within given image */
    int x,y;
    /** Width and height the part of the image to be used */
    int w,h;
    /** Destination format specific data */
    int fd[4];
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
        fname_pal.clear();
        fname_out.clear();
        fname_tab.clear();
        alg = DfsAlg_FldStnbrg;
        fmt = OutFmt_RAW;
        lvl = 100;
        batch = Batch_NONE;
    }
    std::vector<ImageArea> inp;
    std::string fname_lst;
    std::string fname_pal;
    std::string fname_out;
    std::string fname_tab;
    int fmt;
    int alg;
    int lvl;
    int batch;
};

