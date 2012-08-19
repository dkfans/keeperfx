/******************************************************************************/
// Land View files converter for KeeperFX
/******************************************************************************/
/** @file landvico.c
 *     Program code file.
 * @par Purpose:
 *     Contains code to read PNG files and convert them to 8bpp RAWs with
 *     special palette for KeeperFX land views. Based on png2ico project.
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

#if __GNUC__ > 2
#include <ext/hash_map>
#else
#include <hash_map>
#endif

#include <png.h>

#include "landvico_version.h"

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

/** A struct closing non-global command line parameters */
struct ProgramOptions {
    char *fname_inp;
    char *fname_out;
    char *fname_pal;
};

void clear_prog_options(struct ProgramOptions *opts)
{
    opts->fname_inp = NULL;
    opts->fname_out = NULL;
    opts->fname_pal = NULL;
}

void free_prog_options(struct ProgramOptions *opts)
{
    free(opts->fname_inp);
    free(opts->fname_out);
    free(opts->fname_pal);
}

const int word_max=65535;
const int transparency_threshold=196;
const int color_reduce_warning_threshold=512; //maximum quadratic euclidean distance in RGB color space that a palette color may have to a source color assigned to it before a warning is issued
const unsigned int slow_reduction_warn_threshold=262144; //number of colors in source image times number of colors in target image that triggers the warning that the reduction may take a while

void writeByte(FILE* f, int byte)
{
  char data[1];
  data[0]=(byte&255);
  if (fwrite(data,1,1,f)!=1) {perror("Write error"); exit(1);}
}

struct png_data
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;
  png_uint_32 width, height;
  png_colorp palette;
  png_bytepp transMap;
  int color_type;
  int num_palette;
  int requested_colors;
  int col_bits;
  png_data():png_ptr(NULL),info_ptr(NULL),end_info(NULL),width(0),height(0),
             palette(NULL),transMap(NULL),num_palette(0),requested_colors(0),col_bits(0){};
};

int andMaskLineLen(const png_data& img)
{
  int len=(img.width+7)>>3;
  return (len+3)&~3;
}

int xorMaskLineLen(const png_data& img)
{
  int pixelsPerByte=(8/img.col_bits);
  return ((img.width+pixelsPerByte-1)/pixelsPerByte+3)&~3;
}

typedef bool (*checkTransparent_t)(png_bytep, png_data&);

bool checkTransparent1(png_bytep data, png_data&)
{
  return (data[3]<transparency_threshold);
}

bool checkTransparent3(png_bytep, png_data&)
{
  return false;
}

//returns true if color reduction resulted in at least one of the image's colors
//being mapped to a palette color with a quadratic distance of more than
//color_reduce_warning_threshold
bool convert_rgb_to_indexed(png_data& img, bool hasAlpha)
{
    int maxColors=img.requested_colors;

    size_t palSize = sizeof(png_color)*256; //must reserve space for 256 entries here because write loop expects it
    img.palette = (png_colorp)malloc(palSize);
    memset(img.palette,0,palSize); //must initialize whole palette
    img.num_palette = 0;

    checkTransparent_t checkTrans=checkTransparent1;
    int bytesPerPixel=4;
    if (!hasAlpha)
    {
        bytesPerPixel=3;
        checkTrans=checkTransparent3;
    }

    //first pass: gather all colors, make sure
    //alpha channel (if present) contains only 0 and 255
    //if an alpha channel is present, set all transparent pixels to RGBA (0,0,0,0)
    //transparent pixels will already be mapped to palette entry 0, non-transparent
    //pixels will not get a mapping yet (-1)
    hash_map<unsigned int,signed int> mapQuadToPalEntry;
    png_bytep* row_pointers=png_get_rows(img.png_ptr, img.info_ptr);

    for (int y=img.height-1; y>=0; --y)
    {
        png_bytep pixel=row_pointers[y];
        for (unsigned i=0; i<img.width; ++i)
        {
            unsigned int quad=pixel[0]+(pixel[1]<<8)+(pixel[2]<<16);
            bool trans=(*checkTrans)(pixel,img);

            if (hasAlpha)
            {
                if (trans)
                {
                    pixel[0]=0;
                    pixel[1]=0;
                    pixel[2]=0;
                    pixel[3]=0;
                    quad=0;
                }
                else pixel[3]=255;

                quad+=(pixel[3]<<24);
            }
            else if (!trans) quad+=(255<<24);

            if (trans)
                mapQuadToPalEntry[quad]=0;
            else
                mapQuadToPalEntry[quad]=-1;

            pixel+=bytesPerPixel;
        }
    }

    //always allocate entry 0 to black and entry 1 to white because
    //sometimes AND mask is interpreted as color index
    img.num_palette=2;
    img.palette[0].red=0;
    img.palette[0].green=0;
    img.palette[0].blue=0;
    img.palette[1].red=255;
    img.palette[1].green=255;
    img.palette[1].blue=255;

    mapQuadToPalEntry[255<<24]=0; //map (non-transparent) black to entry 0
    mapQuadToPalEntry[255+(255<<8)+(255<<16)+(255<<24)]=1; //map (non-transparent) white to entry 1

    if (mapQuadToPalEntry.size()*img.requested_colors>slow_reduction_warn_threshold)
    {
        fprintf(stdout,"Please be patient. My color reduction algorithm is really slow.\n");
    }

    //Now fill up the palette with colors from the image by repeatedly picking the
    //color most different from the previously picked colors and adding this to the
    //palette. This is done to make sure that in case there are more image colors than
    //palette entries, palette entries are not wasted on similar colors.
    while(img.num_palette < maxColors)
    {
        unsigned int mostDifferentQuad=0;
        int mdqMinDist=-1; //smallest distance to an entry in the palette for mostDifferentQuad
        int mdqDistSum=-1; //sum over all distances to palette entries for mostDifferentQuad
        hash_map<unsigned int,signed int>::iterator stop=mapQuadToPalEntry.end();
        hash_map<unsigned int,signed int>::iterator iter=mapQuadToPalEntry.begin();
        while(iter!=stop)
        {
            hash_map<unsigned int,signed int>::value_type& mapping=*iter++;
            if (mapping.second<0)
            {
                unsigned int quad=mapping.first;
                int red=quad&255;  //must be signed
                int green=(quad>>8)&255;
                int blue=(quad>>16)&255;
                int distSum=0;
                int minDist=INT_MAX;
                for (int i=0; i<img.num_palette; ++i)
                {
                    int dist=(red-img.palette[i].red);
                    dist*=dist;
                    int temp=(green-img.palette[i].green);
                    dist+=temp*temp;
                    temp=(blue-img.palette[i].blue);
                    dist+=temp*temp;
                    if (dist<minDist) minDist=dist;
                    distSum+=dist;
                }

                if (minDist>mdqMinDist || (minDist==mdqMinDist && distSum>mdqDistSum))
                {
                    mostDifferentQuad=quad;
                    mdqMinDist=minDist;
                    mdqDistSum=distSum;
                }
            }
        }

        if (mdqMinDist>0) //if we have found a most different quad, add it to the palette
        {                  //and map it to the new palette entry
            int palentry=img.num_palette;
            img.palette[palentry].red=mostDifferentQuad&255;
            img.palette[palentry].green=(mostDifferentQuad>>8)&255;
            img.palette[palentry].blue=(mostDifferentQuad>>16)&255;
            mapQuadToPalEntry[mostDifferentQuad]=palentry;
            ++img.num_palette;
        }
        else break; //otherwise (i.e. all quads are mapped) the palette is finished
    }

    //Now map all yet unmapped colors to the most appropriate palette entry
    hash_map<unsigned int,signed int>::iterator stop=mapQuadToPalEntry.end();
    hash_map<unsigned int,signed int>::iterator iter=mapQuadToPalEntry.begin();
    while(iter!=stop)
    {
        hash_map<unsigned int,signed int>::value_type& mapping=*iter++;
        if (mapping.second<0)
        {
            unsigned int quad=mapping.first;
            int red=quad&255;  //must be signed
            int green=(quad>>8)&255;
            int blue=(quad>>16)&255;
            int minDist=INT_MAX;
            int bestIndex=0;
            for (int i=0; i<img.num_palette; ++i)
            {
                int dist=(red-img.palette[i].red);
                dist*=dist;
                int temp=(green-img.palette[i].green);
                dist+=temp*temp;
                temp=(blue-img.palette[i].blue);
                dist+=temp*temp;
                if (dist<minDist) { minDist=dist; bestIndex=i; }
            }

            mapping.second=bestIndex;
        }
    }

    //Adjust all palette entries (except for 0 and 1) to be the mean of all
    //colors mapped to it
    for (int i=2; i<img.num_palette; ++i)
    {
        int red=0;
        int green=0;
        int blue=0;
        int numMappings=0;
        hash_map<unsigned int,signed int>::iterator stop=mapQuadToPalEntry.end();
        hash_map<unsigned int,signed int>::iterator iter=mapQuadToPalEntry.begin();
        while(iter!=stop)
        {
            hash_map<unsigned int,signed int>::value_type& mapping=*iter++;
            if (mapping.second==i)
            {
                unsigned int quad=mapping.first;
                red+=quad&255;
                green+=(quad>>8)&255;
                blue+=(quad>>16)&255;
                ++numMappings;
            }
        }

        if (numMappings>0)
        {
            img.palette[i].red=(red+red+numMappings)/(numMappings+numMappings);
            img.palette[i].green=(green+green+numMappings)/(numMappings+numMappings);
            img.palette[i].blue=(blue+blue+numMappings)/(numMappings+numMappings);
        }
    }

    //Now determine if a non-transparent source color got mapped to a target color that
    //has a distance that exceeds the threshold
    bool tooManyColors=false;
    stop=mapQuadToPalEntry.end();
    iter=mapQuadToPalEntry.begin();
    while(iter!=stop)
    {
        hash_map<unsigned int,signed int>::value_type& mapping=*iter++;
        unsigned int quad=mapping.first;
        if ((quad>>24)!=0) //if color is not transparent
        {
            int red=quad&255;
            int green=(quad>>8)&255;
            int blue=(quad>>16)&255;
            int i=mapping.second;
            int dist=(red-img.palette[i].red);
            dist*=dist;
            int temp=(green-img.palette[i].green);
            dist+=temp*temp;
            temp=(blue-img.palette[i].blue);
            dist+=temp*temp;
            if (dist>color_reduce_warning_threshold) tooManyColors=true;
        }
    }


    int transLineLen=andMaskLineLen(img);
    int transLinePad=transLineLen - ((img.width+7)/8);
    img.transMap=(png_bytepp)malloc(img.height*sizeof(png_bytep));

    //second pass: convert RGB to palette entries
    for (int y=img.height-1; y>=0; --y)
    {
        png_bytep row=row_pointers[y];
        png_bytep pixel=row;
        int count8=0;
        int transbyte=0;
        png_bytep transPtr=img.transMap[y]=(png_bytep)malloc(transLineLen);

        for (unsigned i=0; i<img.width; ++i)
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

            int palentry=mapQuadToPalEntry[quad];
            row[i]=palentry;
            pixel+=bytesPerPixel;
        }

        for(int i=0; i<transLinePad; ++i) *transPtr++ = 0;
    }

    return tooManyColors;
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
            verbose = 1;
            break;
        case 'o':
            opts->fname_out = strdup(optarg);
            break;
        case 'p':
            opts->fname_pal = strdup(optarg);
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
    if (opts->fname_pal == NULL)
    {
        opts->fname_pal = file_name_change_extension(opts->fname_out,"pal");
    }
    return true;
}

short show_head(void)
{
    printf("\n%s (%s) %s\n",PROGRAM_FULL_NAME,PROGRAM_NAME,VER_STRING);
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

short load_inp_png_file(png_data &data, const char *fname_inp, struct ProgramOptions * opts)
{
    FILE* pngfile = fopen(fname_inp,"rb");
    if (pngfile == NULL) {
        perror(fname_inp);
        return ERR_CANT_OPEN;
    }
    png_byte header[8];
    if (fread(header,8,1,pngfile) != 1) {
        perror(fname_inp);
        fclose(pngfile);
        return ERR_FILE_READ;
    }
    if (png_sig_cmp(header,0,8)) {
        fprintf(stderr,"%s: Not a PNG file\n",fname_inp);
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    data.requested_colors = 256;
    for (data.col_bits=1; (1<<data.col_bits) < data.requested_colors; data.col_bits++);

    data.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!data.png_ptr)
    {
        fprintf(stderr,"%s: png_create_read_struct error\n",fname_inp);
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    data.info_ptr=png_create_info_struct(data.png_ptr);
    if (!data.info_ptr)
    {
        png_destroy_read_struct(&data.png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fprintf(stderr,"%s: png_create_info_struct error\n",fname_inp);
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    data.end_info=png_create_info_struct(data.png_ptr);
    if (!data.end_info)
    {
        png_destroy_read_struct(&data.png_ptr, &data.info_ptr, (png_infopp)NULL);
        fprintf(stderr,"%s: png_create_info_struct error\n",fname_inp);
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    if (setjmp(png_jmpbuf(data.png_ptr)))
    {
        png_destroy_read_struct(&data.png_ptr, &data.info_ptr, &data.end_info);
        fprintf(stderr,"%s: PNG error\n",fname_inp);
        fclose(pngfile);
        exit(1);
    }

    png_init_io(data.png_ptr, pngfile);
    png_set_sig_bytes(data.png_ptr,8);
    int trafo=PNG_TRANSFORM_PACKING|PNG_TRANSFORM_STRIP_16|PNG_TRANSFORM_EXPAND;
    png_read_png(data.png_ptr, data.info_ptr, trafo , NULL);

    int bit_depth, interlace_type, compression_type, filter_method;
    png_get_IHDR(data.png_ptr, data.info_ptr, &data.width, &data.height, &bit_depth, &data.color_type,
        &interlace_type, &compression_type, &filter_method);

    if ((data.color_type & PNG_COLOR_MASK_COLOR)==0)
    {
        fprintf(stderr,"%s: Grayscale image not supported\n",fname_inp);
        fclose(pngfile);
        return ERR_BAD_FILE;
    }

    fclose(pngfile);

    if (data.color_type==PNG_COLOR_TYPE_PALETTE)
    {
        fprintf(stderr,"Invalid format. This shouldn't happen. PNG_TRANSFORM_EXPAND transforms image to RGB.\n");
        return ERR_BAD_FILE;
    }

    return ERR_OK;
}

short save_raw_pal_files(png_data * img, const char *fname_raw, const char *fname_pal, struct ProgramOptions * opts)
{
    // Open and write the PAL file
    {
        FILE* palfile = fopen(fname_pal,"wb");
        if (palfile == NULL) {
            perror(fname_pal);
            return ERR_CANT_OPEN;
        }
        int i;
        for (i = 0; i < img->requested_colors; i++)
        {
          char col[3];
          col[2] = (img->palette[i].red >> 2);
          col[1] = (img->palette[i].green >> 2);
          col[0] = (img->palette[i].blue >> 2);
          if (fwrite(col,3,1,palfile) != 1) {
              perror("Write error");
              return ERR_FILE_WRITE;
          }
        }
        fclose(palfile);
    }
    // Open and write the RAW file
    {
        FILE* rawfile = fopen(fname_raw,"wb");
        if (rawfile == NULL) {
            perror(fname_raw);
            return ERR_CANT_OPEN;
        }
        png_bytep * row_pointers = png_get_rows(img->png_ptr, img->info_ptr);
        int y;
        for (y = img->height-1; y >= 0; y--)
        {
            png_bytep row=row_pointers[y];
            int newLength = pack(row,img->width,img->col_bits);
            if (fwrite(row,newLength,1,rawfile)!=1) {perror("Write error"); exit(1);}
            for(int i=0; i<xorMaskLineLen(*img)-newLength; ++i) writeByte(rawfile,0);
        }
        for (y = img->height-1; y >= 0; y--)
        {
          png_bytep transPtr=img->transMap[y];
          if (fwrite(transPtr,andMaskLineLen(*img),1,rawfile) != 1) {
              perror("Write error");
              return ERR_FILE_WRITE;
          }
        }
        fclose(rawfile);

    }
    return ERR_OK;
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

    static png_data pngdata;

    if (load_inp_png_file(pngdata,opts.fname_inp,&opts) != ERR_OK) {
        free_prog_options(&opts);
        return 2;
    }

    if (convert_rgb_to_indexed(pngdata, ((pngdata.color_type & PNG_COLOR_MASK_ALPHA)!=0))) {
        fprintf(stderr,"%s: Warning! Color reduction may not be optimal!\nIf the result is not satisfactory, reduce the number of colors\nbefore using this tool.\n",opts.fname_inp);
    }

    if (save_raw_pal_files(&pngdata,opts.fname_out,opts.fname_pal,&opts) != ERR_OK) {
        free_prog_options(&opts);
        return 8;
    }

    free_prog_options(&opts);
    return 0;
}


