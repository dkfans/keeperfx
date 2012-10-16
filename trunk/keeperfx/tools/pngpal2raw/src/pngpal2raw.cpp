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

int verbose = 0;

#define LogMsg(format,args...) fprintf(stdout,format "\n", ## args)
#define LogErr(format,args...) fprintf(stderr,format "\n", ## args)

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
        fname_pal.clear();
        fname_out.clear();
    }
    std::string fname_inp;
    std::string fname_pal;
    std::string fname_out;
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
    WorkingSet():alg(0),requested_colors(0),requested_col_bits(0){}
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
    int alg;
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
 * @param alg Diffusuion algorithm index.
 * @param w Error delta value.
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
    red = clipIntensity(red + ws.mapErrorR[x+SHIFT][y]);
    green = clipIntensity(green + ws.mapErrorG[x+SHIFT][y]);
    blue = clipIntensity(blue + ws.mapErrorB[x+SHIFT][y]);

    int bestIndex = nearest_palette_color_index(palette,(red)|(green<<8)|(blue<<16)|(alpha<<24));

    propagateError(ws.alg, red - palette[bestIndex].red , ws.mapErrorR, x, y);
    propagateError(ws.alg, green - palette[bestIndex].green , ws.mapErrorG, x, y);
    propagateError(ws.alg, blue - palette[bestIndex].blue , ws.mapErrorB, x, y);

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
    int transLinePad=transLineLen - ((img.width+7)/8);
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

        for(int i=0; i<transLinePad; ++i) *transPtr++ = 0;
    }

    img.col_bits = ws.requestedColorBPP();

    return ERR_OK;
}

//packs a line of width pixels (1 byte per pixel) in row, with 8/nbits pixels packed
//into each byte
//returns the new number of bytes in row
int pack(png_bytep row,int width,int nbits)
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

std::string file_name_strip_path(const char *fname_inp)
{
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
    if (tmp1 == NULL)
      return "";
    return tmp1;
}

int load_command_line_options(ProgramOptions &opts, int argc, char *argv[])
{
    opts.clear();
    while (1)
    {
        static struct option long_options[] = {
            {"verbose", no_argument,       0, 'v'},
            {"output",  required_argument, 0, 'o'},
            {"palette",  required_argument,0, 'p'},
            {NULL,      0,                 0,'\0'}
        };
        /* getopt_long stores the option index here. */
        int c;
        int option_index = 0;
        c = getopt_long(argc, argv, "vo:p:", long_options, &option_index);
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
            opts.fname_out = optarg;
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
    if (optind < argc)
    {
        opts.fname_inp = argv[optind++];
    }
    if ((optind < argc) || (opts.fname_inp.size() < 1))
    {
        LogErr("Incorrectly specified input file name.");
        return false;
    }
    // fill names that were not set by arguments
    if (opts.fname_out.length() < 1)
    {
        opts.fname_out = file_name_change_extension(opts.fname_inp.c_str(),"raw");
    }
    if (opts.fname_pal.length() < 1)
    {
        opts.fname_pal = file_name_change_extension(opts.fname_inp.c_str(),"pal");
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
    printf("    -p<file>,--palette<file> Input PAL file name\n");
    printf("    -o<file>,--output<file>  Output RAW file name\n");
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

short save_raw_file(ImageData& img, const std::string& fname_raw, ProgramOptions& opts)
{
    // Open and write the RAW file
    {
        FILE* rawfile = fopen(fname_raw.c_str(),"wb");
        if (rawfile == NULL) {
            perror(fname_raw.c_str());
            return ERR_CANT_OPEN;
        }
        png_bytep * row_pointers = png_get_rows(img.png_ptr, img.info_ptr);
        for (int y=0; y<img.height; y++)
        {
            png_bytep row = row_pointers[y];
            int newLength = pack(row,img.width,img.colorBPP());
            if (fwrite(row,newLength,1,rawfile)!=1) {perror(fname_raw.c_str()); exit(1);}
            for(int i=0; i<xorMaskLineLen(img)-newLength; ++i) writeByte(rawfile,0);
        }
        fclose(rawfile);

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

    LogMsg("Loading image \"%s\".",opts.fname_inp.c_str());
    ImageData img = ImageData();
    if (load_inp_png_file(img, opts.fname_inp, opts) != ERR_OK) {
        return 2;
    }

    ws.requestedColors(256);
    LogMsg("Loading palette file \"%s\".",opts.fname_pal.c_str());
    if (load_inp_palette_file(ws, opts.fname_pal, opts) != ERR_OK) {
        LogErr("Loading palette failed.");
        return 4;
    }

    LogMsg("Converting colors to indexes...");
    if (convert_rgb_to_indexed(ws, img, ((img.color_type & PNG_COLOR_MASK_ALPHA) != ERR_OK))) {
        LogErr("Converting colors failed.");
        return 6;
    }

    LogMsg("Saving file \"%s\".",opts.fname_out.c_str());
    if (save_raw_file(img, opts.fname_out, opts) != ERR_OK) {
        return 8;
    }

    return 0;
}


