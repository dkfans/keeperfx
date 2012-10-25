/******************************************************************************/
// Best 8bpp palette selector for a series of PNG files
/******************************************************************************/
/** @file png2bestpal.cpp
 *     Program code file.
 * @par Purpose:
 *     Contains code to read PNG files and select the best special 8bpp
 *     palette for KeeperFX which would allow to display all the files.
 *     Based on png2ico project.
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

#include "png2bestpal_version.h"

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

#define LogMsg(format,args...) fprintf(stdout,format "\n", ## args)
#define LogDbg(format,args...) if (verbose) fprintf(stdout,format "\n", ## args)
#define LogErr(format,args...) fprintf(stderr,format "\n", ## args)

int verbose = 0;

/** A class closing non-global command line parameters */
class ProgramOptions {
public:
    ProgramOptions()
    {
        clear();
    }
    void clear()
    {
        fnames_inp.clear();
        fname_pal.clear();
        fname_map.clear();
    }
    std::vector<std::string> fnames_inp;
    std::string fname_pal;
    std::string fname_map;
};

class ImageData
{
public:
    ImageData():png_ptr(NULL),info_ptr(NULL),end_info(NULL),width(0),height(0),
        transMap(NULL),col_bits(0),transparency_threshold(196){}
    int colorBPP(void)
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

typedef unsigned long RGBAQuad;
typedef png_color RGBColor;
typedef hash_map<RGBAQuad,signed int> MapQuadToPal;
typedef std::vector<RGBColor> ColorPalette;

inline RGBAQuad mkquad(int r,int g,int b,int a)
{ return (r)|(g<<8)|(b<<16)|(a<<24); }
inline int quad_r(RGBAQuad q)
{ return (q)&255; }
inline int quad_g(RGBAQuad q)
{ return (q>>8)&255; }
inline int quad_b(RGBAQuad q)
{ return (q>>16)&255; }
inline int quad_a(RGBAQuad q)
{ return (q>>24)&255; }

class WorkingSet
{
public:
    WorkingSet():requested_colors(0),requested_col_bits(0){}
    void requestedColors(int reqColors)
    {
        requested_colors = reqColors;
        for (requested_col_bits=1; (1<<requested_col_bits) < requested_colors; requested_col_bits++);
        paletteRemap.resize(requested_colors);
        for (int i=0; i < requested_colors; i++)
            paletteRemap[i] = i;
        palette.reserve(reqColors);
    }
    int requestedColors(void)
    { return requested_colors; }
    int requestedColorBPP(void)
    { return requested_col_bits; }
    void addPaletteBlindEntry(int r, int g, int b)
    {
        RGBColor ncol;
        ncol.red = r;
        ncol.green = g;
        ncol.blue = b;
        palette.push_back(ncol);
    }
    void addPaletteQuad(RGBAQuad quad)
    {
        int palentry = palette.size();
        RGBColor ncol;
        ncol.red = quad_r(quad);
        ncol.green = quad_g(quad);
        ncol.blue = quad_b(quad);
        palette.push_back(ncol);
        mapQuadToPalEntry[quad] = palentry;
    }
    //std::vector<ImageData> images;
    ColorPalette palette;
    std::vector<int> paletteRemap;
    MapQuadToPal mapQuadToPalEntry;
private:
    int requested_colors;
    int requested_col_bits;
};

typedef bool (*checkTransparent_t)(png_bytep, const ImageData&);

bool checkTransparent1(png_bytep data, const ImageData& img)
{
  return (data[3]<img.transparency_threshold);
}

bool checkTransparent3(png_bytep, const ImageData&)
{
  return false;
}

/**
 * Gather all colors, evaluate values with alpha channel.
 * Makes sure alpha channel (if present) contains only 0 and 255 if
 * an alpha channel is present, set all transparent pixels to RGBA (0,0,0,0)
 * transparent pixels will already be mapped to palette entry 0,
 * non-transparent pixels will not get a mapping yet (-1).
 * @param ws
 * @param img
 * @param hasAlpha
 * @return
 */
short gather_list_of_colors_in_image(WorkingSet& ws, ImageData& img, bool hasAlpha)
{
    checkTransparent_t checkTrans=checkTransparent1;
    int bytesPerPixel = (img.colorBPP()+7) >> 3;
    if (!hasAlpha)
    {
        checkTrans=checkTransparent3;
    }
    png_bytep* row_pointers=png_get_rows(img.png_ptr, img.info_ptr);

    for (int y=img.height-1; y>=0; --y)
    {
        png_bytep pixel=row_pointers[y];
        for (unsigned i=0; i<img.width; ++i)
        {
            unsigned char r,g,b,a;
            RGBAQuad quad;
            bool trans=(*checkTrans)(pixel,img);

            r = pixel[0];
            g = pixel[1];
            b = pixel[2];
            if (trans) {
                a = 0;
            } else {
                a=255;
            }
            quad=mkquad(r,g,b,a);

            if (trans)
                ws.mapQuadToPalEntry[quad] = 0;
            else
                ws.mapQuadToPalEntry[quad] = -1;

            pixel+=bytesPerPixel;
        }
    }
    return ERR_OK;
}


/**
 * Fill up the palette with colors from the image.
 * This is done by repeatedly picking the color most different from the
 * previously picked colors and adding this to the palette.
 * This is done to make sure that in case there are more image colors than
 * palette entries, palette entries are not wasted on similar colors.
 *
 * @param ws
 * @return
 */
short pick_palette_of_most_different_colors(WorkingSet& ws)
{
    int maxColors = ws.requestedColors();
    while (ws.palette.size() < maxColors)
    {
        RGBAQuad mostDifferentQuad=0;
        long mdqMinDist = -1; //smallest distance to an entry in the palette for mostDifferentQuad
        long long mdqDistSum = -1; //sum over all distances to palette entries for mostDifferentQuad
        MapQuadToPal::iterator stop = ws.mapQuadToPalEntry.end();
        MapQuadToPal::iterator iter = ws.mapQuadToPalEntry.begin();
        // For all unique colors from the complete list
        while(iter!=stop)
        {
            MapQuadToPal::value_type& mapping=*iter++;
            // If the color is unmapped yet
            if (mapping.second < 0)
            {
                RGBAQuad quad = mapping.first;
                int red = quad_r(quad);  //must be signed
                int green = quad_g(quad);
                int blue = quad_b(quad);
                /* Compute sum of all distances to this color
                 * and minimal distance (to the closest color) for this entry.
                 */
                long long distSum=0;
                long minDist=LONG_MAX;
                ColorPalette::iterator paliter;
                for (paliter = ws.palette.begin(); paliter != ws.palette.end(); paliter++)
                {
                    int temp;
                    long dist;
                    temp = (red - (int)paliter->red);
                    dist = temp*temp;
                    temp = (green - (int)paliter->green);
                    dist += temp*temp;
                    temp = (blue - (int)paliter->blue);
                    dist += temp*temp;
                    if (dist < minDist) minDist = dist;
                    distSum += dist;
                }

                if ( (minDist > mdqMinDist) || (minDist==mdqMinDist && distSum>mdqDistSum))
                {
                    mostDifferentQuad = quad;
                    mdqMinDist = minDist;
                    mdqDistSum = distSum;
                }
            }
        }

        if (mdqMinDist>0) {
            /* If we have found a most different quad, add it to the palette,
             * and map it to the new palette entry.
             */
            ws.addPaletteQuad(mostDifferentQuad);
        } else {
            /* otherwise (i.e. all quads are mapped) the palette is finished
             */
            break;
        }
    }
    // If there are not enough colors, fill the empty entries with more colors from the palette
    while (ws.palette.size() < maxColors)
    {
        ws.addPaletteBlindEntry(0,0,0);
    }
    return ERR_OK;
}

/**
 * Maps all yet unmapped colors to the most appropriate palette entry.
 * @param ws
 * @return
 */
short map_palette_colors_to_most_apropriate(WorkingSet& ws)
{
    MapQuadToPal::iterator stop = ws.mapQuadToPalEntry.end();
    MapQuadToPal::iterator iter = ws.mapQuadToPalEntry.begin();
    while(iter!=stop)
    {
        MapQuadToPal::value_type& mapping=*iter++;
        if (mapping.second<0)
        {
            RGBAQuad quad=mapping.first;
            int red=quad_r(quad);  //must be signed
            int green=quad_g(quad);
            int blue=quad_b(quad);
            int minDist=INT_MAX;
            int bestIndex=0;
            ColorPalette::iterator paliter;
            for (paliter = ws.palette.begin(); paliter != ws.palette.end(); paliter++)
            {
                int dist=(red - paliter->red);
                dist*=dist;
                int temp=(green - paliter->green);
                dist+=temp*temp;
                temp=(blue - paliter->blue);
                dist+=temp*temp;
                if (dist<minDist) {
                    minDist=dist;
                    bestIndex = (paliter - ws.palette.begin());
                }
            }

            mapping.second=bestIndex;
        }
    }
    return ERR_OK;
}

/**
 * Adjusts all palette entries to be the mean of all colors mapped to it.
 *
 * @param ws
 * @return
 */
short update_palette_colors_to_average_of_uses(WorkingSet& ws)
{
    ColorPalette::iterator paliter;
    for (paliter = ws.palette.begin(); paliter != ws.palette.end(); paliter++)
    {
        long long red=0;
        long long green=0;
        long long blue=0;
        long long numMappings=0;
        MapQuadToPal::iterator stop = ws.mapQuadToPalEntry.end();
        MapQuadToPal::iterator iter = ws.mapQuadToPalEntry.begin();
        while(iter!=stop)
        {
            MapQuadToPal::value_type& mapping=*iter++;
            if (mapping.second == (paliter - ws.palette.begin()))
            {
                RGBAQuad quad=mapping.first;
                if (quad_a(quad) <= 192)
                    continue;
                red+=quad_r(quad);
                green+=quad_g(quad);
                blue+=quad_b(quad);
                numMappings++;
            }
        }

        if (numMappings>0)
        {
            int c;
            c = (red+red+numMappings)/(numMappings);
            paliter->red = ((c+(c&1))>>1);
            c = (green+green+numMappings)/(numMappings);
            paliter->green = ((c+(c&1))>>1);
            c = (blue+blue+numMappings)/(numMappings);
            paliter->blue = ((c+(c&1))>>1);
        }
    }

    return ERR_OK;
}

short remap_palette_colors_order(WorkingSet& ws)
{
    ColorPalette orig;
    orig.insert(orig.begin(),ws.palette.begin(),ws.palette.end());
    for (int i = 0; i < ws.paletteRemap.size(); i++) {
        ws.palette[i] = orig[ws.paletteRemap[i]];
        LogDbg("color map %d -> %d",i,ws.paletteRemap[i]);
    }
    return ERR_OK;
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
            {"palmap",  required_argument, 0, 'm'},
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
            verbose++;
            break;
        case 'o':
            opts.fname_pal = optarg;
            break;
        case 'm':
            opts.fname_map = optarg;
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
        opts.fnames_inp.push_back(argv[optind++]);
    }
    if ((optind < argc) || (opts.fnames_inp.size() < 1))
    {
        LogErr("Incorrectly specified input file name.");
        return false;
    }
    // fill names that were not set by arguments
    if (opts.fname_pal.length() < 1)
    {
        opts.fname_pal = file_name_change_extension(opts.fnames_inp[0].c_str(),"pal");
    }
    return true;
}

short show_head(void)
{
    LogMsg("\n%s (%s) %s",PROGRAM_FULL_NAME,PROGRAM_NAME,VER_STRING);
    LogMsg("  Created by %s; %s",PROGRAM_AUTHORS,LEGAL_COPYRIGHT);
    LogMsg("----------------------------------------");
    return ERR_OK;
}

/** Displays information about how to use this tool. */
short show_usage(char *fname)
{
    std::string xname = file_name_strip_path(fname);
    LogMsg("usage:\n");
    LogMsg("    %s [options] <filename>", xname.c_str());
    LogMsg("where <filename> should be the input BMP file, and [options] are:");
    LogMsg("    -v,--verbose             Verbose console output mode");
    LogMsg("    -m<file>,--palmap<file>  Static palette entries mapping file name");
    LogMsg("    -o<file>,--output<file>  Output PAL file name");
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

short load_pal_mapping_file(WorkingSet& ws, const std::string& fname_map, ProgramOptions& opts)
{
    std::fstream f;
    size_t currentLine;
    size_t errors;

    /* Load the .txt file: */
    f.open(fname_map.c_str(), std::fstream::in);

    currentLine = 0;
    errors = 0;
    while (!f.eof())
    {
        // read next line and strip insignificant whitespace from it:
        std::string line;
        std::getline(f, line);
        currentLine++;

        size_t pos;
        pos = line.find_first_not_of(" \n\r\t");
        // If empty line or comment
        if ((pos == std::string::npos) || (line[pos] == '#'))
            continue;

        int code = strtol(line.substr(pos).c_str(),NULL,16);
        if (code < 0)
        {
            errors++;
            LogErr("Invalid entry id in PAL mapping entry at line %ld.", (long)currentLine);
            continue;
        }
        pos = line.find("\t",pos);
        // If invalid line
        if (pos == std::string::npos)
        {
            errors++;
            LogErr("PAL mapping entry at line %ld has no tab separated quad.", (long)currentLine);
            continue;
        }
        int quad = strtol(line.substr(pos+1).c_str(),NULL,16);
        if (quad == 0)
        {
            errors++;
            LogErr("Invalid quad value in PAL mapping entry at line %ld.", (long)currentLine);
            continue;
        }
        quad = mkquad((quad>>16)&255,(quad>>8)&255,quad&255,255);
        {
            int i = ws.palette.size();
            ws.addPaletteQuad(quad);
            ws.paletteRemap[code] = i;
        }
    }
    /* Make sure no palette entries are lost.
     * The code below adds co-mappings to the mappings added above.
     * Note that it assumes the entries are added to palette exactly as above.
     */
    for (int i = 0; i < ws.paletteRemap.size(); i++)
    {
        vector<int>::iterator iter;
        iter = std::find(ws.paletteRemap.begin()+i,ws.paletteRemap.end(),i);
        if (iter != ws.paletteRemap.end())
            continue;
        int k = i;
        while (ws.paletteRemap[k] != k)
            k = ws.paletteRemap[k];
        ws.paletteRemap[k] = i;
    }

    if (errors > 0)
    {
        LogErr("Couldn't load file %s, it is probably corrupted.", fname_map.c_str());
        return ERR_FILE_READ;
    }

    f.close();

    return ERR_OK;
}

short save_pal_file(WorkingSet& ws, const std::string& fname_pal, ProgramOptions& opts)
{
    // Open and write the PAL file
    {
        FILE* palfile = fopen(fname_pal.c_str(),"wb");
        if (palfile == NULL) {
            perror(fname_pal.c_str());
            return ERR_CANT_OPEN;
        }
        ColorPalette::iterator iter;
        for (iter = ws.palette.begin(); iter != ws.palette.end(); iter++)
        {
            unsigned char col[3];
            col[0] = ((iter->red + (iter->red&2)) >> 2);
            col[1] = ((iter->green + (iter->green&2)) >> 2);
            col[2] = ((iter->blue + (iter->blue&2)) >> 2);
            if (fwrite(col,3,1,palfile) != 1) {
                perror(fname_pal.c_str());
                return ERR_FILE_WRITE;
            }
        }
        fclose(palfile);
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

    ws.requestedColors(256);
    ws.mapQuadToPalEntry.resize(1048576);
    if (opts.fname_map.length() > 0)
    {
        // load unmoveable palette entries from mapping file
        if (load_pal_mapping_file(ws, opts.fname_map, opts) != ERR_OK) {
            LogErr("Mappings load failed.");
            return 7;
        }
    } else
    {
        //map (non-transparent) black to entry 0
        ws.addPaletteQuad(mkquad(0,0,0,255));
        //map (non-transparent) white to entry 1
        ws.addPaletteQuad(mkquad(255,255,255,255));
    }
    {
        std::vector<std::string>::iterator iter;
        for (iter = opts.fnames_inp.begin(); iter != opts.fnames_inp.end(); iter++)
        {
            LogMsg("Loading image \"%s\".",iter->c_str());
            ImageData img = ImageData();
            if (load_inp_png_file(img, *iter, opts) != ERR_OK) {
                return 2;
            }
            LogMsg("Gathering colors of image...");
            if (gather_list_of_colors_in_image(ws, img, (img.color_type & PNG_COLOR_MASK_ALPHA)) != ERR_OK) {
                LogErr("Gathering failed.");
                return 3;
            }
        }
    }
    {
        LogMsg("Picking most diverse palette...");
        if (pick_palette_of_most_different_colors(ws) != ERR_OK) {
            LogErr("Picking palette failed.");
            return 4;
        }
        LogMsg("Mapping colors to palette...");
        if (map_palette_colors_to_most_apropriate(ws) != ERR_OK) {
            LogErr("Mapping colors failed.");
            return 5;
        }
        LogMsg("Updating palette to average...");
        if (update_palette_colors_to_average_of_uses(ws) != ERR_OK) {
            LogErr("Updating palette to average failed.");
            return 6;
        }
        LogMsg("Reordering palette entries...");
        if (remap_palette_colors_order(ws) != ERR_OK) {
            LogErr("Reordering failed.");
            return 6;
        }
    }
    LogMsg("Saving...");
    if (save_pal_file(ws, opts.fname_pal, opts) != ERR_OK) {
        return 8;
    }
    LogMsg("Done.");
    return 0;
}

