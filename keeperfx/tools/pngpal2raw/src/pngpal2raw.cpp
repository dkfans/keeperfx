/******************************************************************************/
// Land View files converter for KeeperFX
/******************************************************************************/
/** @file pngpal2raw.c
 *     Program code file.
 * @par Purpose:
 *     Contains code to read PNG files and convert them to 8bpp RAWs with
 *     special palette generated with Png2bestPal. Based on png2ico project.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis <listom@gmail.com>
 * @author   Matthias S. Benkmann <matthias@winterdrache.de>
 * @date     25 Jul 2012 - 18 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <cstdio>
#include <vector>
#include <climits>
#include <getopt.h>

#include <cmath>
#include <fstream>
#include <sstream>

#if __GNUC__ > 2
#include <ext/hash_map>
#else
#include <hash_map>
#endif

#include <png.h>

#include "pngpal2raw_version.h"

using namespace std;
namespace __gnu_cxx{};
using namespace __gnu_cxx;

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
int verbose = 0;

#define LogMsg(format,args...) fprintf(stdout,format "\n", ## args)
#define LogDbg(format,args...) if (verbose) fprintf(stdout,format "\n", ## args)
#define LogErr(format,args...) fprintf(stderr,format "\n", ## args)

class ImageArea {
public:
    ImageArea(const std::string &nname, int nx=-1, int ny=-1, int nw=-1, int nh=-1):
        fname(nname),x(nx),y(ny),w(nw),h(nh) {};
    std::string fname;
    int x,y;
    int w,h;
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
        batch = false;
    }
    std::vector<ImageArea> inp;
    std::string fname_lst;
    std::string fname_pal;
    std::string fname_out;
    std::string fname_tab;
    int fmt;
    int alg;
    int lvl;
    bool batch;
};

class RGBAccum {
public:
    RGBAccum(): r(0),g(0),b(0) {};
    long r;
    long g;
    long b;
};


typedef unsigned long RGBAQuad;
typedef png_color RGBColor;
typedef hash_map<RGBAQuad,signed int> MapQuadToPal;
typedef std::vector<RGBColor> ColorPalette;
typedef std::vector<std::vector<float> > DitherError;
class ImageData
{
public:
    ImageData():png_ptr(NULL),info_ptr(NULL),end_info(NULL),width(0),height(0),
           transMap(NULL),col_bits(0),transparency_threshold(196){}
    int colorBPP(void) const
    { return col_bits; }
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;
    png_uint_32 width, height;
    png_bytepp transMap;
    int color_type;
    int col_bits;
    int transparency_threshold;
};

class WorkingSet
{
public:
    WorkingSet():alg(DfsAlg_FldStnbrg),lvl(0),requested_colors(0),requested_col_bits(0){}
    void requestedColors(int reqColors)
    {
        requested_colors = reqColors;
        for (requested_col_bits=1; (1<<requested_col_bits) < requested_colors; requested_col_bits++);
        paletteRemap.resize(requested_colors);
        for (int i=0; i < requested_colors; i++)
            paletteRemap[i] = i;
    }
    int requestedColors(void) const
    { return requested_colors; }
    void ditherLevel(int level)
    {
        lvl = level;
        lvlCurve.resize(512);
        lvlCurve[256+0] = 0;
        if (level > 0) {
            for (int i=1; i < 256; i++) {
                lvlCurve[256+i] = pow(i,lvl/100.);
                lvlCurve[256-i] = -lvlCurve[256+i];
            }
        } else {
            for (int i=1; i < 256; i++) {
                lvlCurve[256+i] = i;
                lvlCurve[256-i] = -i;
            }
        }
    }
    int requestedColorBPP(void) const
    { return requested_col_bits; }
    void addPaletteQuad(RGBAQuad quad)
    {
        int palentry = palette.size();
        RGBColor ncol;
        ncol.red = quad&255;
        ncol.green = (quad>>8)&255;
        ncol.blue = (quad>>16)&255;
        palette.push_back(ncol);
        mapQuadToPalEntry[quad] = palentry;
    }
    //std::vector<ImageData> images;
    ColorPalette palette;
    std::vector<int> paletteRemap;
    DitherError mapErrorR;
    DitherError mapErrorG;
    DitherError mapErrorB;
    MapQuadToPal mapQuadToPalEntry;
    std::vector<float> lvlCurve;
    int alg;
    int lvl;
private:
    int requested_colors;
    int requested_col_bits;
};

/* to avoid indices below 0 in dithering error array */
#define SHIFT 3

float dif[11][3][6] =
{
   {
      {0,        0,        0,        0,        7.0/16.0, 0},
      {0,        0,        3.0/16.0, 5.0/16.0, 1.0/16.0, 0},
      {0,        0       , 0,        0,        0,        0}
   },
   {
      {0,        0,        0,        0,        7.0/48.0, 5.0/48.0},
      {0,        3.0/48.0, 5.0/48.0, 7.0/48.0, 5.0/48.0, 3.0/48.0},
      {0,        1.0/48.0, 3.0/48.0, 5.0/48.0, 3.0/48.0, 1.0/48.0}
   },
   {
      {0,        0,        0,        0,        8.0/42.0, 4.0/42.0},
      {0,        2.0/42.0, 4.0/42.0, 8.0/42.0, 4.0/42.0, 2.0/42.0},
      {0,        1.0/42.0, 2.0/42.0, 4.0/42.0, 2.0/42.0, 1.0/42.0}
   },
   {
      {0,        0,        0,        0,        8.0/32.0, 4.0/32.0},
      {0,        2.0/32.0, 4.0/32.0, 8.0/32.0, 4.0/32.0, 2.0/32.0},
      {0,        0       , 0,        0,        0,        0}
   },
   {
      {0,        0,        0,        0,        7.0/16.0, 0},
      {0,        1.0/16.0, 3.0/16.0, 5.0/16.0, 0,        0},
      {0,        0       , 0,        0,        0,        0}
   },
   {
      {0,        0,        0,        0,        5.0/32.0, 3.0/32.0},
      {0,        2.0/32.0, 4.0/32.0, 5.0/32.0, 4.0/32.0, 2.0/32.0},
      {0,        0       , 2.0/32.0, 3.0/32.0, 2.0/32.0, 0}
   },
   {
      {0,        0,        0,        0,        4.0/16.0, 3.0/16.0},
      {0,        1.0/16.0, 2.0/16.0, 3.0/16.0, 2.0/16.0, 1.0/16.0},
      {0,        0       , 0,        0,        0,        0}
   },
   {
      {0,        0,        0,        0,        2.0/4.0,  0},
      {0,        0,        1.0/4.0,  1.0/4.0,  0,        0},
      {0,        0       , 0,        0,        0,        0}
   },
   {
      {0,        0,        0,        0,        1.0/8.0,  1.0/8.0},
      {0,        0,        1.0/8.0,  1.0/8.0,  1.0/8.0,  0},
      {0,        0,        0,        1.0/8.0,  0,        0}
   },
   {
      {0,        0,        0,        0,        4.0/8.0,  0},
      {0,        1.0/8.0,  1.0/8.0,  2.0/8.0,  0,        0},
      {0,        0,        0,        0,        0,        0}
   },
   {
      {0,        0,        0,        0,        8.0/16.0, 0},
      {1.0/16.0, 1.0/16.0, 2.0/16.0, 4.0/16.0, 0,        0},
      {0,        0,        0,        0,        0,        0}
   }
};

/**
 * Cuts given value to color range (0..255).
 * @param x
 * @return
 */
int clipIntensity(long x)
{
   if (x > 255) return 255;
   if (x < 0) return 0;
   return x;
}


void writeByte(FILE* f, int byte)
{
  char data[1];
  data[0]=(byte&255);
  if (fwrite(data,1,1,f)!=1) {perror("Write error"); exit(1);}
}

int andMaskLineLen(const ImageData& img)
{
  int len=(img.width+7)>>3;
  return (len+3)&~3;
}

int xorMaskLineLen(const ImageData& img)
{
  int pixelsPerByte = (8 / img.colorBPP());
  return ((img.width+pixelsPerByte-1)/pixelsPerByte+3)&~3;
}

typedef bool (*checkTransparent_t)(png_bytep, ImageData&);

bool checkTransparent1(png_bytep data, ImageData& img)
{
  return (data[3] < img.transparency_threshold);
}

bool checkTransparent3(png_bytep, ImageData&)
{
  return false;
}

inline bool isTransparentX(png_bytep data, int x)
{
  return (data[x>>3] & (1<<(7-(x&7))));
}

int nearest_palette_color_index(const ColorPalette& palette, const RGBAQuad quad)
{
    int red=quad&255;  //must be signed
    int green=(quad>>8)&255;
    int blue=(quad>>16)&255;
    int minDist=INT_MAX;
    int bestIndex=0;
    ColorPalette::const_iterator paliter;
    for (paliter = palette.begin(); paliter != palette.end(); paliter++)
    {
        int dist=(red - paliter->red);
        dist*=dist;
        int temp=(green - paliter->green);
        dist+=temp*temp;
        temp=(blue - paliter->blue);
        dist+=temp*temp;
        if (dist<minDist) {
            minDist=dist;
            bestIndex = (paliter - palette.begin());
        }
    }
    return bestIndex;
}

/**
 * Propagates an error into adjacent cells.
 * @param alg Diffusion algorithm index.
 * @param lvl Diffusion level.
 * @param wd Error delta value.
 * @param e The error array.
 * @param i Error central coordinate.
 * @param j Error central coordinate.
 */
void propagateError(int alg, float w, DitherError &e, int i, int j)
{
    e[i+1+SHIFT][j  ] = e[i+1+SHIFT][j  ] + (w*dif[alg][0][4]);
    e[i+2+SHIFT][j  ] = e[i+2+SHIFT][j  ] + (w*dif[alg][0][5]);

    e[i-3+SHIFT][j+1] = e[i-3+SHIFT][j+1] + (w*dif[alg][1][0]);
    e[i-2+SHIFT][j+1] = e[i-2+SHIFT][j+1] + (w*dif[alg][1][1]);
    e[i-1+SHIFT][j+1] = e[i-1+SHIFT][j+1] + (w*dif[alg][1][2]);
    e[i  +SHIFT][j+1] = e[i  +SHIFT][j+1] + (w*dif[alg][1][3]);
    e[i+1+SHIFT][j+1] = e[i+1+SHIFT][j+1] + (w*dif[alg][1][4]);
    e[i+2+SHIFT][j+1] = e[i+2+SHIFT][j+1] + (w*dif[alg][1][5]);

    e[i-3+SHIFT][j+2] = e[i-3+SHIFT][j+2] + (w*dif[alg][2][0]);
    e[i-2+SHIFT][j+2] = e[i-2+SHIFT][j+2] + (w*dif[alg][2][1]);
    e[i-1+SHIFT][j+2] = e[i-1+SHIFT][j+2] + (w*dif[alg][2][2]);
    e[i  +SHIFT][j+2] = e[i  +SHIFT][j+2] + (w*dif[alg][2][3]);
    e[i+1+SHIFT][j+2] = e[i+1+SHIFT][j+2] + (w*dif[alg][2][4]);
    e[i+2+SHIFT][j+2] = e[i+2+SHIFT][j+2] + (w*dif[alg][2][5]);
}

int dithered_palette_color_index(WorkingSet& ws, const ColorPalette& palette, unsigned int x, unsigned int y, RGBAQuad quad)
{
    int red=(quad&255);  //must be signed
    int green=(quad>>8)&255;
    int blue=(quad>>16)&255;
    int alpha=(quad>>24)&255;
    red = clipIntensity(red + (ws.mapErrorR[x+SHIFT][y]+0.5));
    green = clipIntensity(green + (ws.mapErrorG[x+SHIFT][y]+0.5));
    blue = clipIntensity(blue + (ws.mapErrorB[x+SHIFT][y]+0.5));

    int bestIndex = nearest_palette_color_index(palette,(red)|(green<<8)|(blue<<16)|(alpha<<24));

    propagateError(ws.alg, ws.lvlCurve[256 + red - palette[bestIndex].red] , ws.mapErrorR, x, y);
    propagateError(ws.alg, ws.lvlCurve[256 + green - palette[bestIndex].green] , ws.mapErrorG, x, y);
    propagateError(ws.alg, ws.lvlCurve[256 + blue - palette[bestIndex].blue] , ws.mapErrorB, x, y);

    return bestIndex;
}

short convert_rgb_to_indexed(WorkingSet& ws, ImageData& img, bool hasAlpha)
{
    checkTransparent_t checkTrans=checkTransparent1;
    int bytesPerPixel = (img.colorBPP()+7) >> 3;
    if (!hasAlpha)
    {
        checkTrans=checkTransparent3;
    }

    png_bytep* row_pointers=png_get_rows(img.png_ptr, img.info_ptr);

    int transLineLen=andMaskLineLen(img);
    img.transMap=(png_bytepp)malloc(img.height*sizeof(png_bytep));


    ws.mapErrorR.resize(img.width+2*SHIFT);
    ws.mapErrorG.resize(img.width+2*SHIFT);
    ws.mapErrorB.resize(img.width+2*SHIFT);
    for (int i=0; i < img.width+2*SHIFT; i++)
    {
        ws.mapErrorR[i].resize(img.height+2*SHIFT);
        ws.mapErrorG[i].resize(img.height+2*SHIFT);
        ws.mapErrorB[i].resize(img.height+2*SHIFT);
    }

    //second pass: convert RGB to palette entries
    //for (int y=img.height-1; y>=0; --y)
    for (int y=0; y<img.height; y++)
    {
        png_bytep row=row_pointers[y];
        png_bytep pixel=row;
        int count8=0;
        int transbyte=0;
        png_bytep transPtr=img.transMap[y]=(png_bytep)malloc(transLineLen);

        for (unsigned x=0; x<img.width; ++x)
        {
            bool trans=((*checkTrans)(pixel,img));
            unsigned int quad=pixel[0]+(pixel[1]<<8)+(pixel[2]<<16);
            if (!trans) quad+=(255<<24); //NOTE: alpha channel has already been set to 255 for non-transparent pixels, so this is correct even for images with alpha channel

            if (trans) ++transbyte;
            if (++count8==8)
            {
                *transPtr++ = transbyte;
                count8=0;
                transbyte=0;
            }
            transbyte+=transbyte; //shift left 1

            int palentry = dithered_palette_color_index(ws, ws.palette, x, y, quad);
            row[x]=palentry;
            pixel+=bytesPerPixel;
        }
        // Finish the 8-bit bound if it's needed
        while (count8 != 0)
        {
            ++transbyte;
            if (++count8==8)
            {
                *transPtr++ = transbyte;
                count8=0;
                transbyte=0;
            }
            transbyte+=transbyte; //shift left 1
        }
    }

    img.col_bits = ws.requestedColorBPP();

    return ERR_OK;
}

/**
 * Packs a line of width pixels (1 byte per pixel) in row, with 8/nbits pixels packed into each byte.
 * @return the new number of bytes in row
 */
int raw_pack(png_bytep row,int width,int nbits)
{
  int pixelsPerByte=8/nbits;
  if (pixelsPerByte<=1) return width;
  int ander=(1<<nbits)-1;
  int outByte=0;
  int count=0;
  int outIndex=0;
  for (int i=0; i<width; ++i)
  {
    outByte+=(row[i]&ander);
    if (++count==pixelsPerByte)
    {
      row[outIndex]=outByte;
      count=0;
      ++outIndex;
      outByte=0;
    }
    outByte<<=nbits;
  }

  if (count>0)
  {
    outByte<<=nbits*(pixelsPerByte-count);
    row[outIndex]=outByte;
    ++outIndex;
  }

  return outIndex;
}

/**
 * Packs a line of pixels (1 byte per pixel) so that transparent bytes are RLE-encoded into HugeSprite.
 * @return the new number of bytes in row
 */
int hspr_pack(png_bytep out_row, const png_bytep inp_row, const png_bytep inp_trans, int width, const ColorPalette& palette)
{
    int area;
    int outIndex=0;
    int i=0;
    while (i < width)
    {
        // Filled
        area = 0;
        while ( (i+area < width) && !isTransparentX(inp_trans, i+area) )
            area++;
        *(long *)(out_row+outIndex) = area;
        outIndex += sizeof(long);
        memcpy(out_row+outIndex, inp_row+i, area);
        outIndex += area;
        i += area;
        // Transparent
        area = 0;
        while ( (i+area < width) && isTransparentX(inp_trans, i+area) )
            area++;
        *(long *)(out_row+outIndex) = area;
        outIndex += sizeof(long);
        i += area;
    }
    return outIndex;
}

#pragma pack(1)

struct SmallSprite {
        size_t Data;
        unsigned char SWidth;
        unsigned char SHeight;
};

#pragma pack()

/**
 * Packs a line of pixels (1 byte per pixel) so that transparent bytes are RLE-encoded into SmallSprite.
 * @return the new number of bytes in row.
 */
int sspr_pack(png_bytep out_row, const png_bytep inp_row, const png_bytep inp_trans, int width, const ColorPalette& palette)
{
    int area;
    int outIndex=0;
    int i=0;
    while (i < width)
    {
        // Filled
        area = 0;
        while ( (i+area < width) && !isTransparentX(inp_trans, i+area) ) {
            area++;
        }
        LogDbg("fill area %d",area);
        while (area > 0) {
            int part_area;
            if (area > 127) {
                part_area = 127;
                area -= 127;
            } else {
                part_area = area;
                area = 0;
            }
            *(char *)(out_row+outIndex) = (char)(part_area);
            outIndex += sizeof(char);
            memcpy(out_row+outIndex, inp_row+i, part_area);
            outIndex += part_area;
            i += part_area;
        }
        // Transparent
        area = 0;
        while ( (i+area < width) && isTransparentX(inp_trans, i+area) ) {
            area++;
        }
        LogDbg("trans area %d",area);
        if (i+area >= width) {
            i += area;
            area = 0;
        }
        while (area > 0) {
            int part_area;
            if (area > 127) {
                part_area = 127;
                area -= 127;
            } else {
                part_area = area;
                area = 0;
            }
            *(char *)(out_row+outIndex) = (char)(-part_area);
            outIndex += sizeof(char);
            i += part_area;
        }
    }
    { // End a line with 0
        *(char *)(out_row+outIndex) = 0;
        outIndex += sizeof(char);
    }
    return outIndex;
}

std::string file_name_strip_path(const std::string &fname_inp)
{
    size_t tmp1,tmp2;
    tmp1 = fname_inp.find_last_of('/');
    tmp2 = fname_inp.find_last_of('\\');
    if ((tmp1 == std::string::npos) || (tmp1 < tmp2))
        tmp1 = tmp2;
    if (tmp1 != std::string::npos)
        return fname_inp.substr(tmp1+1);
    return fname_inp;
}

std::string file_name_change_extension(const std::string &fname_inp, const std::string &ext)
{
    std::string fname = file_name_strip_path(fname_inp);
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

int load_command_line_options(ProgramOptions &opts, int argc, char *argv[])
{
    opts.clear();
    while (1)
    {
        static struct option long_options[] = {
            {"verbose", no_argument,       0, 'v'},
            {"batchlist",no_argument,      0, 'b'},
            {"format",  required_argument, 0, 'f'},
            {"diffuse", required_argument, 0, 'd'},
            {"dflevel", required_argument, 0, 'l'},
            {"output",  required_argument, 0, 'o'},
            {"outtab",  required_argument, 0, 't'},
            {"palette", required_argument, 0, 'p'},
            {NULL,      0,                 0,'\0'}
        };
        /* getopt_long stores the option index here. */
        int c;
        int option_index = 0;
        c = getopt_long(argc, argv, "vbf:d:l:o:p:", long_options, &option_index);
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
        case 'b':
            opts.batch = true;
            break;
        case 'f':
            if (strcasecmp(optarg,"HSPR") == 0)
                opts.fmt = OutFmt_HSPR;
            else if (strcasecmp(optarg,"SSPR") == 0)
                opts.fmt = OutFmt_SSPR;
            else if (strcasecmp(optarg,"RAW") == 0)
                opts.fmt = OutFmt_RAW;
            else
                return false;
            break;
        case 'd':
            if (strcasecmp(optarg,"FldStnbrg") == 0)
                opts.alg = DfsAlg_FldStnbrg;
            else if (strcasecmp(optarg,"JrvJdcNnk") == 0)
                opts.alg = DfsAlg_JrvJdcNnk;
            else if (strcasecmp(optarg,"Stucki") == 0)
                opts.alg = DfsAlg_Stucki;
            else if (strcasecmp(optarg,"Burkes") == 0)
                opts.alg = DfsAlg_Burkes;
            else if (strcasecmp(optarg,"Fan") == 0)
                opts.alg = DfsAlg_Fan;
            else if (strcasecmp(optarg,"Sierra3") == 0)
                opts.alg = DfsAlg_Sierra3;
            else if (strcasecmp(optarg,"Sierra2") == 0)
                opts.alg = DfsAlg_Sierra2;
            else if (strcasecmp(optarg,"Sierra24A") == 0)
                opts.alg = DfsAlg_Sierra24A;
            else if (strcasecmp(optarg,"Atkinson") == 0)
                opts.alg = DfsAlg_Atkinson;
            else if (strcasecmp(optarg,"ShiauFan4") == 0)
                opts.alg = DfsAlg_ShiauFan4;
            else if (strcasecmp(optarg,"ShiauFan5") == 0)
                opts.alg = DfsAlg_ShiauFan5;
            else
                return false;
            break;
        case 'l':
            opts.lvl = atol(optarg);
            break;
        case 'o':
            opts.fname_out = optarg;
            break;
        case 't':
            opts.fname_tab = optarg;
            break;
        case 'p':
            opts.fname_pal = optarg;
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
        if (opts.batch) {
            // In batch mode, file name is not an image but text file with list
            opts.fname_lst = argv[optind++];
            break;
        }
        opts.inp.push_back(ImageArea(argv[optind++]));
    }
    // Load the files list, if it's provided
    if (!opts.fname_lst.empty())
    {
        ifstream infile;
        infile.open(opts.fname_lst.c_str(), ifstream::in);
         while (infile.good()) {
             std::string str;
             std::getline(infile, str, '\n');
             str.erase(str.find_last_not_of(" \n\r\t")+1);
             if (!str.empty()) {
                 opts.inp.push_back(ImageArea(str));
             }
         }
    }
    if ((optind < argc) || (opts.inp.empty() && opts.fname_lst.empty()))
    {
        LogErr("Incorrectly specified input file name.");
        return false;
    }
    if ((opts.fmt != OutFmt_SSPR) && (opts.inp.size() != 1))
    {
        LogErr("This format supports only one input file name.");
        return false;
    }
    // fill names that were not set by arguments
    if (opts.fname_out.length() < 1)
    {
        switch (opts.fmt)
        {
        case OutFmt_HSPR:
        case OutFmt_SSPR:
            opts.fname_out = file_name_change_extension(opts.inp[0].fname,"dat");
            break;
        case OutFmt_RAW:
        default:
            opts.fname_out = file_name_change_extension(opts.inp[0].fname,"raw");
            break;
        }
    }
    if (opts.fname_tab.length() < 1)
    {
        opts.fname_tab = file_name_change_extension(opts.fname_out,"tab");
    }
    if (opts.fname_pal.length() < 1)
    {
        opts.fname_pal = file_name_change_extension(opts.inp[0].fname,"pal");
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
    printf("where <filename> should be the input PNG file, and [options] are:\n");
    printf("    -v,--verbose             Verbose console output mode\n");
    printf("    -d<alg>,--diffuse<alg>   Diffusion algorithm used for bpp convertion\n");
    printf("    -l<num>,--dflevel<num>   Diffusion level, 1..100\n");
    printf("    -f<fmt>,--format<fmt>    Output file format, RAW or HSPR\n");
    printf("    -p<file>,--palette<file> Input PAL file name\n");
    printf("    -o<file>,--output<file>  Output image file name\n");
    printf("    -t<file>,--outtab<file>  Output tabulation file name\n");
    printf("    -b,--batchlist           Batch, input file is not an image but contains a list of PNGs\n");
    return ERR_OK;
}

short load_inp_png_file(ImageData& img, const std::string& fname_inp, ProgramOptions& opts)
{
    FILE* pngfile = fopen(fname_inp.c_str(),"rb");
    if (pngfile == NULL) {
        perror(fname_inp.c_str());
        return ERR_CANT_OPEN;
    }
    png_byte header[8];
    if (fread(header,8,1,pngfile) != 1) {
        perror(fname_inp.c_str());
        fclose(pngfile);
        return ERR_FILE_READ;
    }
    if (png_sig_cmp(header,0,8)) {
        LogErr("%s: Not a PNG file",fname_inp.c_str());
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    img.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!img.png_ptr)
    {
        LogErr("%s: png_create_read_struct error",fname_inp.c_str());
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    img.info_ptr=png_create_info_struct(img.png_ptr);
    if (!img.info_ptr)
    {
        png_destroy_read_struct(&img.png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        LogErr("%s: png_create_info_struct error",fname_inp.c_str());
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    img.end_info=png_create_info_struct(img.png_ptr);
    if (!img.end_info)
    {
        png_destroy_read_struct(&img.png_ptr, &img.info_ptr, (png_infopp)NULL);
        LogErr("%s: png_create_info_struct error",fname_inp.c_str());
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    if (setjmp(png_jmpbuf(img.png_ptr)))
    {
        png_destroy_read_struct(&img.png_ptr, &img.info_ptr, &img.end_info);
        LogErr("%s: PNG error",fname_inp.c_str());
        fclose(pngfile);
        exit(1);
    }

    png_init_io(img.png_ptr, pngfile);
    png_set_sig_bytes(img.png_ptr,8);
    int trafo=PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_EXPAND;
    png_read_png(img.png_ptr, img.info_ptr, trafo , NULL);

    int bit_depth, interlace_type, compression_type, filter_method;
    png_get_IHDR(img.png_ptr, img.info_ptr, &img.width, &img.height, &bit_depth, &img.color_type,
        &interlace_type, &compression_type, &filter_method);

    if ((img.color_type & PNG_COLOR_MASK_COLOR)==0)
    {
        LogErr("%s: Grayscale image not supported",fname_inp.c_str());
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    fclose(pngfile);

    if (img.color_type==PNG_COLOR_TYPE_PALETTE)
    {
        LogErr("Invalid format. This shouldn't happen. PNG_TRANSFORM_EXPAND transforms image to RGB.");
        return ERR_BAD_FILE;
    }

    if (img.color_type & PNG_COLOR_MASK_ALPHA) {
        img.col_bits = 32;
    } else {
        img.col_bits = 24;
    }

    return ERR_OK;
}

short load_inp_palette_file(WorkingSet& ws, const std::string& fname_pal, ProgramOptions& opts)
{
    std::fstream f;

    /* Load the .pal file: */
    f.open(fname_pal.c_str(), std::ios::in | std::ios::binary);

    while (!f.eof())
    {
        unsigned char col[3];
        unsigned char r,g,b;
        // read next color
        f.read((char *)col, 3);

        if (!f.good())
        {
            break;
        }
        r = (col[0] << 2) + 1;
        g = (col[1] << 2) + 1;
        b = (col[2] << 2) + 1;
        ws.addPaletteQuad((r)|(g<<8)|(b<<16)|(255<<24));
    }

    if (ws.palette.size() != ws.requestedColors())
    {
        LogErr("Incomplete file %s, got %d colors from it while expected %d.", fname_pal.c_str(),(int)ws.palette.size(),ws.requestedColors());
        return ERR_FILE_READ;
    }

    f.close();

    return ERR_OK;
}

short save_raw_file(WorkingSet& ws, ImageData& img, const std::string& fname_out, ProgramOptions& opts)
{
    // Open and write the RAW file
    {
        FILE* rawfile = fopen(fname_out.c_str(),"wb");
        if (rawfile == NULL) {
            perror(fname_out.c_str());
            return ERR_CANT_OPEN;
        }
        png_bytep * row_pointers = png_get_rows(img.png_ptr, img.info_ptr);
        for (int y=0; y<img.height; y++)
        {
            png_bytep row = row_pointers[y];
            int newLength = raw_pack(row,img.width,img.colorBPP());
            if (fwrite(row,newLength,1,rawfile)!=1) {perror(fname_out.c_str()); return ERR_FILE_WRITE; }
            for(int i=0; i<xorMaskLineLen(img)-newLength; ++i) writeByte(rawfile,0);
        }
        fclose(rawfile);

    }
    return ERR_OK;
}

short save_hugspr_file(WorkingSet& ws, ImageData& img, const std::string& fname_out, ProgramOptions& opts)
{
    // Open and write the HugeSprite file
    {
        FILE* rawfile = fopen(fname_out.c_str(),"wb");
        if (rawfile == NULL) {
            perror(fname_out.c_str());
            return ERR_CANT_OPEN;
        }
        long * row_shifts = new long[img.height];
        if (fwrite(row_shifts,img.height*sizeof(long),1,rawfile)!=1) {perror(fname_out.c_str()); return ERR_FILE_WRITE; }
        long base_pos = ftell(rawfile);
        png_bytep out_row = new png_byte[img.width*3];
        png_bytep * row_pointers = png_get_rows(img.png_ptr, img.info_ptr);
        for (int y=0; y<img.height; y++)
        {
            row_shifts[y] = ftell(rawfile) - base_pos;
            png_bytep inp_row = row_pointers[y];
            png_bytep inp_trans = img.transMap[y];
            int newLength = hspr_pack(out_row,inp_row,inp_trans,img.width,ws.palette);
            if (fwrite(out_row,newLength,1,rawfile)!=1) {perror(fname_out.c_str()); return ERR_FILE_WRITE; }
            //writeByte(rawfile,0);
        }
        delete[] out_row;
        fseek(rawfile,0,SEEK_SET);
        if (fwrite(row_shifts,img.height*sizeof(long),1,rawfile)!=1) {perror(fname_out.c_str()); return ERR_FILE_WRITE; }
        delete[] row_shifts;
        fclose(rawfile);
    }
    return ERR_OK;
}

short save_smallspr_file(WorkingSet& ws, std::vector<ImageData>& imgs, const std::string& fname_out, const std::string& fname_tab, ProgramOptions& opts)
{
    std::vector<SmallSprite> spr_shifts;
    // Open and write the SmallSprite file
    {
        FILE* rawfile = fopen(fname_out.c_str(),"wb");
        if (rawfile == NULL) {
            perror(fname_out.c_str());
            return ERR_CANT_OPEN;
        }
        spr_shifts.resize(imgs.size()+1);
        long base_pos = ftell(rawfile);
        {
            unsigned short spr_count;
            spr_count = imgs.size()+1;
            if (fwrite(&spr_count,sizeof(spr_count),1,rawfile) != 1)
            { perror(fname_out.c_str()); return ERR_FILE_WRITE; }
        }
        for (int i = 0; i < imgs.size(); i++)
        {
            ImageData &img = imgs[i];
            spr_shifts[i].Data = ftell(rawfile) - base_pos;
            spr_shifts[i].SWidth = img.width;
            spr_shifts[i].SHeight = img.height;
            std::vector<png_byte> out_row;
            out_row.resize(img.width*3);
            png_bytep * row_pointers = png_get_rows(img.png_ptr, img.info_ptr);
            for (int y=0; y<img.height; y++)
            {
                png_bytep inp_row = row_pointers[y];
                png_bytep inp_trans = img.transMap[y];
                int newLength = sspr_pack(&out_row.front(),inp_row,inp_trans,img.width,ws.palette);
                if (fwrite(&out_row.front(),newLength,1,rawfile) != 1)
                { perror(fname_out.c_str()); return ERR_FILE_WRITE; }
            }
        }
        fclose(rawfile);
    }
    {
        FILE* tabfile = fopen(fname_tab.c_str(),"wb");
        if (tabfile == NULL) {
            perror(fname_tab.c_str());
            return ERR_CANT_OPEN;
        }
        if (fwrite(&spr_shifts.front(),sizeof(SmallSprite),spr_shifts.size(),tabfile) != spr_shifts.size())
        { perror(fname_tab.c_str()); return ERR_FILE_WRITE; }
        fclose(tabfile);
    }
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

    std::vector<ImageData> imgs;
    imgs.resize(opts.inp.size());
    {
        for (int i = 0; i < opts.inp.size(); i++)
        {
            LogMsg("Loading image \"%s\".",opts.inp[i].fname.c_str());
            ImageData& img = imgs[i];
            if (load_inp_png_file(img, opts.inp[i].fname, opts) != ERR_OK) {
                return 2;
            }
        }
    }

    ws.alg = opts.alg;
    ws.ditherLevel(opts.lvl);
    ws.requestedColors(256);
    LogMsg("Loading palette file \"%s\".",opts.fname_pal.c_str());
    if (load_inp_palette_file(ws, opts.fname_pal, opts) != ERR_OK) {
        LogErr("Loading palette failed.");
        return 4;
    }

    {
        std::vector<ImageData>::iterator iter;
        for (iter = imgs.begin(); iter != imgs.end(); iter++)
        {
            LogMsg("Converting image %d colors to indexes...",(int)(iter-imgs.begin()));
            ImageData& img = *iter;
            if (convert_rgb_to_indexed(ws, img, ((img.color_type & PNG_COLOR_MASK_ALPHA) != ERR_OK))) {
                LogErr("Converting colors failed.");
                return 6;
            }
        }
    }

    switch (opts.fmt)
    {
    case OutFmt_RAW:
        LogMsg("Saving RAW file \"%s\".",opts.fname_out.c_str());
        if (save_raw_file(ws, imgs[0], opts.fname_out, opts) != ERR_OK) {
            return 8;
        }
        break;
    case OutFmt_HSPR:
        LogMsg("Saving HSPR file \"%s\".",opts.fname_out.c_str());
        if (save_hugspr_file(ws, imgs[0], opts.fname_out, opts) != ERR_OK) {
            return 8;
        }
        break;
    case OutFmt_SSPR:
        LogMsg("Saving SSPR file \"%s\".",opts.fname_out.c_str());
        if (save_smallspr_file(ws, imgs, opts.fname_out, opts.fname_tab, opts) != ERR_OK) {
            return 8;
        }
        break;
    }

    return 0;
}


