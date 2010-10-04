/******************************************************************************/
// PE/DLL Rebuilder of Export Section
/******************************************************************************/
/** @file peresec.c
 *     Program code file.
 * @par Purpose:
 *     Contains code to read, modify and write PE format binary files.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2009 - 20 Apr 2010
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
#include <ctype.h>
#include <limits.h>
#include <getopt.h>
#include <time.h>
#include "peresec_version.h"

/** Maximum size of function name from input .MAP file.
 * Used to verify the input file sanity. */
#define MAX_EXPORT_NAMELEN 255
/** Maximal amount of exported functions.
 * Used to allocate static arrays. */
#define MAX_EXPORTS 0x7fff
/** Maximal amount of relocation entries.
 * Used to allocate static arrays. */
#define MAX_RELOCATIONS 0x7ffff
/** Max length of line in any of input files */
#define MAX_LINE_LEN 4096
/** Name of exports section. */
const char export_section_name[]=".edata";
/** Name of relocations section. */
const char relocations_section_name[]=".reloc";
/** Text added at end of export section; just for fun. */
const char export_end_str[] ="Blessed are those who have not seen and yet believe";
const char * relocation_methods[] = {
    "ABSOLUTE",
    "HIGH",
    "LOW",
    "HIGHLOW",
    "HIGHADJ",
    "MIPS_JMPADDR",
    "TYPE6",
    "TYPE7",
    "TYPE8",
    "IA64_IMM64",
    "DIR64",
    "TYPE11",
    "TYPE12",
    "TYPE13",
    "TYPE14",
    "TYPE15",
};

// Sections
#define MAX_SECTIONS_NUM        64
#define MAX_SECTION_NAME_LEN    8

#define EXPORT_DIRECTORY_SIZE   0x0028
#define MZ_SIZEOF_HEADER        0x0040
#define MZ_NEWHEADER_OFS        0x003C
#define PE_SIZEOF_SIGNATURE     4
#define PE_SIZEOF_FILE_HEADER   20
#define PE_NUM_SECTIONS_OFS     2
#define PE_TIMEDATSTAMP_OFS     4
#define PE_CHARACTERISTICS_OFS  18
#define PE_SIZEOF_OPTN_HEADER   96
#define PE_OPTH_IMAGE_BASE           28
#define PE_OPTH_SECTION_ALIGNMENT    32
#define PE_OPTH_FILE_ALIGNMENT       36
#define PE_OPTH_SIZE_OF_IMAGE        56
#define PE_OPTH_SIZE_OF_HEADERS      60
#define PE_OPTH_NUM_RVAS_AND_SIZES   92
#define PE_SIZEOF_DATADIR_ENTRY       8
#define PE_DATA_DIRECTORY_EXPORT      0
#define PE_DATA_DIRECTORY_IMPORT      1
#define PE_DATA_DIRECTORY_RESOURCE    2
#define PE_DATA_DIRECTORY_EXCEPTION   3
#define PE_DATA_DIRECTORY_SECURITY    4
#define PE_DATA_DIRECTORY_BASERELOC   5

#define PE_SIZEOF_SECTHDR_ENTRY 40

#define RELOCATION_BASE_SIZE    8
#define RELOCATION_OFFSET_SIZE  2

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

enum {
    ERR_OK          =  0,
    ERR_CANT_OPEN   = -1, // fopen problem
    ERR_BAD_FILE    = -2, // incorrect file format
    ERR_NO_MEMORY   = -3, // malloc error
    ERR_FILE_READ   = -4, // fget/fread/fseek error
    ERR_FILE_WRITE  = -5, // fput/fwrite error
    ERR_LIMIT_EXCEED= -6, // static limit exceeded
};

enum DefFormat {
    DEF_DEFAULT, //default DLLTOOL compatible .DEF
    DEF_MSLIB, //LIB compatible .DEF (MSVC suite)
};

#ifdef __cplusplus
#pragma pack(1)
#endif

struct section_entry {
    unsigned long vaddr; // virtual address
    unsigned long vsize;
    unsigned long raddr; // RAW address
    unsigned long rsize;
    unsigned char name[MAX_SECTION_NAME_LEN+1];
};

struct export_entry {
       unsigned short seg;
       unsigned long offs;
       unsigned long nmoffs;
       char srcname[MAX_EXPORT_NAMELEN+1];
       char dstname[MAX_EXPORT_NAMELEN+1];
};

struct relocation_entry {
    unsigned short seg;
    unsigned long offs;
    unsigned char method;
};

/** Complete PE Information structure. */
struct PEInfo {
    unsigned long new_header_raddr; // RAW address or PE Header
    unsigned long rvas_and_sizes_raddr; // RAW address or "RVAs and sizes" array
    long rvas_and_sizes_num; // Number of "RVAs and sizes" entries
    unsigned long image_base_vaddr; // default base address, from header
    unsigned long timedatestamp; // timedate stamp of the file
    struct section_entry *sections[MAX_SECTIONS_NUM];
    long sections_num;
    struct export_entry *exports[MAX_EXPORTS];
    long exports_num;
    struct relocation_entry relocations[MAX_RELOCATIONS];
    long relocations_num;
};

#ifdef __cplusplus
#pragma pack()
#endif

int verbose = 0;
int extract = 0;

/** A struct closing non-global command line parameters */
struct ProgramOptions {
    char *fname_inp;
    char *fname_out;
    char *fname_map;
    char *fname_def;
    char *fname_rmap;
    char *module_name;
    const char *funcname_prefix;
    char *def_format;
};

void clear_prog_options(struct ProgramOptions *opts)
{
    opts->fname_inp = NULL;
    opts->fname_out = NULL;
    opts->fname_map = NULL;
    opts->fname_def = NULL;
    opts->fname_rmap = NULL;
    opts->module_name = NULL;
    opts->funcname_prefix = "";
    opts->def_format = NULL;
}

void free_prog_options(struct ProgramOptions *opts)
{
    free(opts->fname_inp);
    free(opts->fname_out);
    free(opts->fname_map);
    free(opts->fname_def);
    free(opts->fname_rmap);
    free(opts->module_name);
    free(opts->def_format);
}

void export_sort(struct export_entry **exp, long exp_size)
{
   int sorted=false;
   long i;
   struct export_entry *pTemp;
   // Sort the strings in ascending order
   while(!sorted)
   {
     sorted = true;
     for (i=0; i < exp_size-1; i++)
       if(strcmp(exp[i]->dstname, exp[i+1]->dstname) > 0)
       {
         sorted = false;     // We were out of order
         // Swap pointers exp[i] and exp[i+1]
         pTemp = exp[i];
         exp[i] = exp[i+1];
         exp[i+1] = pTemp;
       }
   }
}

int find_dupename(struct export_entry *exp[],unsigned int exp_size)
{
   int i;
   for (i = 0 ; i < exp_size-1 ; i++)
       if(stricmp(exp[i]->dstname, exp[i+1]->dstname) == 0)
       {
         return i;
       }
   return -1;
}

int find_dupeoffs(struct export_entry *exp[],unsigned int exp_size)
{
   int i,k;
   for (i = 0 ; i < exp_size ; i++)
     for (k = 0 ; k < exp_size ; k++)
       if ((i!=k) && (exp[i]->seg==exp[k]->seg) && (exp[i]->offs==exp[k]->offs))
       {
         return i;
       }
   return -1;
}

char *get_name_with_prefix(char *dname, const char *sname, const char *prefix)
{
  int name_begin;
  if (strlen(sname)<2)
  {
    strcpy(dname,prefix);
    int dname_begin=strlen(dname);
    strcpy(dname+dname_begin,sname);
    return dname;
  }
  char name0=sname[0];
  char name1=sname[1];
  if ((name1=='@')||(name1=='?'))
  {
    dname[0]=name0;
    dname[1]=name1;
    name_begin=2;
  } else
  if ((name0=='@')||(name0=='?'))
  {
    dname[0]=name0;
    name_begin=1;
  } else
  {
    name_begin=0;
  }
  strcpy(dname+name_begin,prefix);
  int dname_begin=strlen(dname);
  strncpy(dname+dname_begin,sname+name_begin,MAX_EXPORT_NAMELEN-dname_begin-1);
  dname[MAX_EXPORT_NAMELEN]='\0';
  return dname;
}

int recognize_relocation_method(const char *mth_str)
{
    int i;
    for (i=0; i < sizeof(relocation_methods)/sizeof(relocation_methods[0]); i++)
    {
        if (strcasecmp(mth_str,relocation_methods[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

/** Uses given prefix and export->srcname to fill export->dstname with correct string. */
void export_fill_dstnames(struct export_entry **exp, long exp_size, const char *funcname_prefix)
{
   long i;
   // Sort the strings in ascending order
   for (i=0; i < exp_size; i++)
   {
      get_name_with_prefix(exp[i]->dstname,exp[i]->srcname,funcname_prefix);
   }
}

void relocation_sort(struct relocation_entry *rel, long rel_size)
{
   int sorted=false;
   long i;
   struct relocation_entry rel_buf;
   // Sort the strings in ascending order
   while(!sorted)
   {
     sorted = true;
     for (i=0; i < rel_size-1; i++)
       if ((rel[i].seg > rel[i+1].seg) || ((rel[i].seg == rel[i+1].seg) && (rel[i].offs > rel[i+1].offs)))
       {
         sorted = false;     // We were out of order
         // Swap items rel[i] and rel[i+1]
         memcpy(&rel_buf, &rel[i], sizeof(struct relocation_entry));
         memcpy(&rel[i], &rel[i+1], sizeof(struct relocation_entry));
         memcpy(&rel[i+1], &rel_buf, sizeof(struct relocation_entry));
       }
   }
}

/**
 * Writes 2-byte little-endian number into given buffer.
 */
void write_int16_le_buf (unsigned char *buff, unsigned short x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
}

/**
 * Writes 4-byte little-endian number into given buffer.
 */
void write_int32_le_buf (unsigned char *buff, unsigned long x)
{
    buff[0]=(x&255);
    buff[1]=((x>>8)&255);
    buff[2]=((x>>16)&255);
    buff[3]=((x>>24)&255);
}

/**
 * Reads 4-byte little-endian number from given buffer.
 */
inline long read_int32_le_buf (const unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    l += buff[2]<<16;
    l += buff[3]<<24;
    return l;
}

/**
 * Reads 2-byte little-endian number from given buffer.
 */
inline unsigned short read_int16_le_buf (const unsigned char *buff)
{
    long l;
    l = buff[0];
    l += buff[1]<<8;
    return l;
}

/**
 * Returns length of opened file.
 * Value -1 means error.
 */
inline long file_length_opened (FILE *fp)
{
    long length;
    long lastpos;

    if (fp==NULL)
      return -1;
    lastpos = ftell (fp);
    if (fseek(fp, 0, SEEK_END) != 0)
      return -1;
    length = ftell(fp);
    fseek(fp, lastpos, SEEK_SET);
    return length;
}

char *file_name_change_extension(const char *fname_inp,const char *ext)
{
    char *fname;
    char *tmp1,*tmp2;
    if (fname_inp == NULL)
      return NULL;
    fname = malloc(strlen(fname_inp)+strlen(ext)+2);
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
            {"extract", no_argument,       0, 'x'},
            {"output",  required_argument, 0, 'o'},
            {"modname", required_argument, 0, 'n'},
            {"map",     required_argument, 0, 'm'},
            {"def",     required_argument, 0, 'f'},
            {"prefix",  required_argument, 0, 'p'},
            {"rmap",    required_argument, 0, 'r'},
            {"deff", required_argument, 0, 'e' },
            {NULL,      0,                 0,'\0'}
        };
        /* getopt_long stores the option index here. */
        int c;
        int option_index = 0;
        c = getopt_long(argc, argv, "vxo:m:f:n:p:r:e:", long_options, &option_index);
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
        case 'x':
            extract = 1;
            break;
        case 'o':
            opts->fname_out = strdup(optarg);
            break;
        case 'm':
            opts->fname_map = strdup(optarg);
            break;
        case 'f':
            opts->fname_def = strdup(optarg);
            break;
        case 'r':
            opts->fname_rmap = strdup(optarg);
            break;
        case 'n':
            opts->module_name = strdup(optarg);
            break;
        case 'p':
            opts->funcname_prefix = optarg; // Note that it's not duplicated
            break;
        case 'e':
            opts->def_format = strdup(optarg);
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
        opts->fname_out = file_name_change_extension(opts->fname_inp,"dll");
    }
    if (opts->fname_map == NULL)
    {
        opts->fname_map = file_name_change_extension(opts->fname_inp,"map");
    }
    if (opts->fname_def == NULL)
    {
        opts->fname_def = file_name_change_extension(opts->fname_out,"def");
    }
    if (opts->module_name == NULL)
    {
        opts->module_name = file_name_strip_to_body(opts->fname_out);
    }
    return true;
}

short show_head(void)
{
    printf("\nPE/DLL Rebuilder of Export Section (PeRESec) %s\n",VER_STRING);
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
    printf("where <filename> should be the input PE file, and [options] are:\n");
    printf("    -v,--verbose             Verbose console output mode\n");
    printf("    -x,--extract             Extract data from input file\n");
    printf("    -o<file>,--output<file>  Output file name\n");
    printf("    -n<text>,--modname<text> Module name for export table\n");
    printf("    -m<file>,--map<file>     Input .MAP file name\n");
    printf("    -r<file>,--rmap<file>    Input .RMAP file name\n");
    printf("    -f<file>,--def<file>     Output .DEF file name\n");
    printf("    -p<text>,--prefix<text>  Function names prefix\n");
    printf("    -e<format>,--deff<format> .DEF output format.\n");
    return ERR_OK;
}

enum DefFormat get_def_format(const char * str)
{
    if (str == NULL) {
        return DEF_DEFAULT;
    }
    else if (strcmp(str, "mslib") == 0) {
        return DEF_MSLIB;
    }
    else {
        return DEF_DEFAULT;
    }
}

/** Reads any disk file into memory; allocates the buffer first. */
long read_file_to_memory(const char *fname, unsigned char **buf_ptr)
{
    unsigned char *buf;
    long len;
    FILE *fp;
    *buf_ptr = NULL;
    fp = fopen(fname,"rb");
    if (fp == NULL)
    {
        printf("Can't open '%s' file!\n",fname);
        return ERR_CANT_OPEN;
    }
    // Get file size
    len = file_length_opened(fp);
    // Allocate memory
    buf = malloc(len + 1024);
    if (buf == NULL)
    {
        printf("Cannot reserve memory to load '%s'.\n",fname);
        fclose(fp);
        return ERR_NO_MEMORY;
    }
    memset(buf+len,0,1024);
    if (fread(buf, len, 1, fp) != 1)
    {
        printf("Read error in '%s'.\n",fname);
        fclose(fp);
        free(buf);
        return ERR_FILE_READ;
    }
    fclose(fp);
    *buf_ptr = buf;
    return len;
}

/** Writes memory buffer into disk file, then deallocates the buffer. */
short write_file_from_memory(const char *fname, unsigned char **buf_ptr, long len)
{
    unsigned char *buf = *buf_ptr;
    FILE *fp;
    *buf_ptr = NULL;
    fp = fopen(fname,"wb");
    if (fp == NULL)
    {
        printf("Can't open '%s' file!\n",fname);
        if (verbose)
            printf("Check if it's not read-only, and you have rights to write it.\n");
        return ERR_CANT_OPEN;
    }
    if (fwrite(buf, len, 1, fp) != 1)
    {
        printf("Write error in '%s'.\n",fname);
        fclose(fp);
        free(buf);
        return ERR_FILE_WRITE;
    }
    fclose(fp);
    free(buf);
    return ERR_OK;
}

/** Reads .MAP file into exports array; returns num of entries. */
long read_map(const char *fname, struct export_entry **exports)
{
  long idx,lnnum;
  FILE *fhndl;
  char *lnbuf;
  char *ln;
  lnbuf = malloc(MAX_LINE_LEN);
  if (lnbuf == NULL)
  {
      printf("Memory allocation error!\n");
      abort();
      return ERR_NO_MEMORY;
  }
  fhndl = fopen(fname,"rb");
  if (fhndl == NULL)
  {
    printf("Can't open '%s' file!\n",fname);
    if (verbose)
        printf("Check if file name is correct, and you have rights to read it.\n");
    free(lnbuf);
    return ERR_CANT_OPEN;
  }
  idx=0;
  lnnum=0;
  while (!feof(fhndl))
  {
      lnnum++;
      ln = fgets(lnbuf,MAX_LINE_LEN,fhndl);
      if (ln == NULL)
          break;
      // Check if the line is correct
      while (isspace(*ln)) ln++;
      if (*ln == '\0')
          continue;
      if (*ln != '0')
      {
          if (idx > 0)
          {
              printf("Export entry %ld (line %ld) does not start with 4-digit segment number!\n",idx,lnnum);
              if (verbose)
                  printf("Fix the .MAP file with text editor and try again.\n");
              fclose(fhndl);
              free(lnbuf);
              return ERR_BAD_FILE;
          }
          continue;
      }
      exports[idx] = malloc(sizeof(struct export_entry));
      if (exports[idx] == NULL)
      {
          printf("Memory allocation error!\n");
          fclose(fhndl);
          free(lnbuf);
          abort();
          return ERR_NO_MEMORY;
      }
      int nread;
      nread = sscanf(ln," %hx:%lx %255s",&(exports[idx]->seg),&(exports[idx]->offs),exports[idx]->srcname);
      if ((nread<3) || (strlen(exports[idx]->srcname)<1))
      {
          if ((nread<=0) && feof(fhndl))
          {
            free(exports[idx]);
            exports[idx]=NULL;
            break;
          }
          printf("Error reading export entry %ld (line %ld)!\n",idx,lnnum);
          if (verbose)
              printf("Fix the .MAP file and then retry.\n");
          fclose(fhndl);
          free(lnbuf);
          return ERR_BAD_FILE;
      } else
      {
        exports[idx]->dstname[0] = '\0';
        exports[idx]->nmoffs=0;
        idx++;
        if (idx >= MAX_EXPORTS)
        {
            printf("Too many (over %d) exports in .MAP file!\n",MAX_EXPORTS);
            if (verbose)
                printf("Strip the file or increase MAX_EXPORTS to fix this.\n");
            fclose(fhndl);
            free(lnbuf);
            return ERR_LIMIT_EXCEED;
        }
      }
  }
  fclose(fhndl);
  free(lnbuf);
  printf("Got %ld entries from .MAP file.\n",idx);
  return idx;
}

/** Creates .MAP file from given exports array. */
short create_map(const char *fname, const char *fname_dll, struct PEInfo *pe)
{
    long idx;
    FILE *fhndl;
    char *fname_strip;
    time_t ftime;
    char *ftime_str;
    fhndl = fopen(fname,"wb");
    if (fhndl == NULL)
    {
      printf("Can't create file '%s'!\n",fname);
      if (verbose)
          printf("Check if you have rights to write to it.\n");
      return ERR_CANT_OPEN;
    }
    fname_strip = file_name_strip_path(fname_dll);
    fprintf(fhndl,"%s\n",fname_strip);
    free(fname_strip);
    ftime = pe->timedatestamp;
    ftime_str = ctime(&ftime);
    fname_strip = strchr(ftime_str,'\n');
    if (fname_strip != NULL)
        *fname_strip = '\0';
    fprintf(fhndl,"\nTimestamp is %lx (%s)\n",(unsigned long)pe->timedatestamp,ftime_str);
    fprintf(fhndl,"Preferred load address is %08lx\n",(unsigned long)(pe->image_base_vaddr));
    fprintf(fhndl,"\n  %-15s %-29s %s\n\n","Address","Publics by Name","Rva+Base");
    for (idx=0; idx < pe->exports_num; idx++)
    {
        if (pe->exports[idx] == NULL)
          continue;
        const char *name;
        name = pe->exports[idx]->srcname;
        unsigned long rva = pe->exports[idx]->offs+pe->sections[(pe->exports[idx]->seg)%MAX_SECTIONS_NUM]->vaddr;
        fprintf(fhndl,"  %04lX:%08lX   %-29s %08lx\n",(long)pe->exports[idx]->seg,(long)pe->exports[idx]->offs,name,(unsigned long)(pe->image_base_vaddr+rva));
    }
    fclose(fhndl);
    printf("Written %ld names into .MAP file.\n",idx);
    return idx;
}

/** Creates .DEF file from given exports list. */
short create_def(const char *fname, const char *fname_dll, struct PEInfo *pe, enum DefFormat format)
{
  long idx;
  FILE *fhndl;
  char *fname_strip;
  fhndl = fopen(fname,"wb");
  if (fhndl == NULL)
  {
      printf("Can't create file '%s'!\n",fname);
      if (verbose)
          printf("Check if you have rights to write to it.\n");
      return ERR_CANT_OPEN;
  }
  fname_strip = file_name_strip_path(fname_dll);
  fprintf(fhndl,"LIBRARY     %s\n",fname_strip);
  free(fname_strip);
  fprintf(fhndl,"\nEXPORTS\n");
  for (idx=0; idx < pe->exports_num; idx++)
  {
      if (pe->exports[idx] == NULL)
        continue;
      const char *name;
      name = pe->exports[idx]->dstname;
      unsigned long val = pe->exports[idx]->offs+pe->sections[(pe->exports[idx]->seg)%MAX_SECTIONS_NUM]->vaddr;

      if (format == DEF_MSLIB) {
          //handle symbols in .data and .cdata differently - add DATA attribute
          if (pe->exports[idx]->seg == 2 || pe->exports[idx]->seg == 3) {
              fprintf(fhndl,"    %s DATA ; RVA=0x%08lX\n",name,val);
          }
          else {
              fprintf(fhndl,"    %s ; RVA=0x%08lX\n",name,val);
          }
      }
      else {
          //default
          fprintf(fhndl,"    %-36s ; RVA=0x%08lX\n",name,val);
      }
  }
  fclose(fhndl);
  printf("Written %ld names into .DEF file.\n",idx);
  return ERR_OK;
}

/** Reads .RMAP file into relocations array; returns num of entries. */
long read_rmap(const char *fname, struct relocation_entry *relocations)
{
  long idx,lnnum;
  FILE *fhndl;
  char *lnbuf,*rdbuf;
  char *ln;
  long k;
  lnbuf = malloc(MAX_LINE_LEN);
  rdbuf = malloc(MAX_LINE_LEN);
  if ((lnbuf == NULL) || (rdbuf == NULL))
  {
      printf("Memory allocation error!\n");
      abort();
      return ERR_NO_MEMORY;
  }
  fhndl = fopen(fname,"rb");
  if (fhndl == NULL)
  {
    printf("Can't open file '%s'!\n",fname);
    if (verbose)
        printf("Check if file name is correct, and you have rights to read it.\n");
    free(lnbuf);free(rdbuf);
    return ERR_CANT_OPEN;
  }
  idx=0;
  lnnum=0;
  while (!feof(fhndl))
  {
      lnnum++;
      ln = fgets(lnbuf,MAX_LINE_LEN,fhndl);
      if (ln == NULL)
          break;
      // Check if the line is correct
      while (isspace(*ln)) ln++;
      if (*ln == '\0')
          continue;
      if (*ln != '0')
      {
          if (*ln != ';')
          {
              printf("Relocation entry %ld (line %ld) does not start with 4-digit segment number!\n",idx,lnnum);
              if (verbose)
                  printf("Fix the .RMAP file with text editor and try again.\n");
              fclose(fhndl);
              free(lnbuf);free(rdbuf);
              return ERR_BAD_FILE;
          }
          continue;
      }
      int nread;
      memset(rdbuf,'\0',MAX_LINE_LEN);
      nread = sscanf(ln," %hx:%lx %255s",&(relocations[idx].seg),&(relocations[idx].offs),rdbuf);
      k = recognize_relocation_method(rdbuf);
      if ((nread < 3) || (k < 0))
      {
          if ((nread<=0) && feof(fhndl))
          {
            break;
          }
          printf("Error reading relocation entry %ld (line %ld)!\n",idx,lnnum);
          if (verbose)
              printf("Fix the .RMAP file and then retry.\n");
          fclose(fhndl);
          free(lnbuf);free(rdbuf);
          return ERR_BAD_FILE;
      } else
      {
          relocations[idx].method = k;
          idx++;
          if (idx >= MAX_RELOCATIONS)
          {
              printf("Too many (over %d) relocations in .RMAP file!\n",MAX_RELOCATIONS);
              if (verbose)
                  printf("Strip the file or increase MAX_RELOCATIONS to fix this.\n");
              fclose(fhndl);
              free(lnbuf);free(rdbuf);
              return ERR_LIMIT_EXCEED;
          }
      }
  }
  fclose(fhndl);
  free(lnbuf);free(rdbuf);
  printf("Got %ld entries from .RMAP file.\n",idx);
  return idx;
}

/** Creates .RMAP file from given relocations list. */
short create_rmap(const char *fname, const char *fname_dll, struct PEInfo *pe)
{
    long idx;
    FILE *fhndl;
    char *fname_strip;
    fhndl = fopen(fname,"wb");
    if (fhndl == NULL)
    {
        printf("Can't create file '%s'!\n",fname);
        if (verbose)
            printf("Check if you have rights to write to it.\n");
        return ERR_CANT_OPEN;
    }
    fname_strip = file_name_strip_path(fname_dll);
    fprintf(fhndl,"; Relocations list file for %s\n",fname_strip);
    free(fname_strip);
    fprintf(fhndl,"; This file can be loaded by PeRESec in order to\n");
    fprintf(fhndl,"; rebuild relocation table inside target PE file.\n\n");
    for (idx=0; idx < pe->relocations_num; idx++)
    {
        const char *method = relocation_methods[pe->relocations[idx].method & 0x0f];
        unsigned long rva = pe->relocations[idx].offs+pe->sections[(pe->relocations[idx].seg)%MAX_SECTIONS_NUM]->vaddr;
        fprintf(fhndl,"  %04lX:%08lX       %-8s ; RVA=0x%08lX\n",(long)pe->relocations[idx].seg,(long)pe->relocations[idx].offs,method,rva);
    }
    fclose(fhndl);
    printf("Written %ld entries into .RMAP file.\n",idx);
    return ERR_OK;
}

/** Aligns given size to multiplication of "file alignment" value. */
unsigned long align_file_section_size(unsigned char *buf, long filesize, unsigned long isize, struct PEInfo *pe)
{
    unsigned char *data;
    unsigned long optional_header_raw_pos;
    long align;
    unsigned long i;
    // Reading the value to align to
    optional_header_raw_pos = pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_SIZEOF_FILE_HEADER;
    data = buf + optional_header_raw_pos+PE_OPTH_FILE_ALIGNMENT;
    align = read_int32_le_buf(data);
    i = align - (isize%align);
    if ((i > 0) && (i < align))
      return isize+i;
    return isize;
}

/** Aligns given size to multiplication of "section alignment" value. */
unsigned long align_virt_section_size(unsigned char *buf, long filesize, unsigned long isize, struct PEInfo *pe)
{
    unsigned char *data;
    unsigned long optional_header_raw_pos;
    long align;
    unsigned long i;
    // Reading the value to align to
    optional_header_raw_pos = pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_SIZEOF_FILE_HEADER;
    data = buf + optional_header_raw_pos+PE_OPTH_SECTION_ALIGNMENT;
    align = read_int32_le_buf(data);
    i = align - (isize%align);
    if ((i > 0) && (i < align))
      return isize+i;
    return isize;
}

/** Sets specified data directory to the offset of given section. */
short set_pe_data_directory_to_section(unsigned char *buf, long filesize, struct section_entry *sec, long ddir_idx, struct PEInfo *pe)
{
    unsigned char *data;
    // Positions in headers
    unsigned long data_directory_entry_raw_pos;
    if (pe->rvas_and_sizes_num <= ddir_idx)
    {
      printf("The PE file data directory doesn't have entry no %d!\n",(int)ddir_idx);
      if (verbose)
          printf("Reserve more space for the data directory to fix this.\n");
      return ERR_BAD_FILE;
    }
    // Locating data directory entry
    data_directory_entry_raw_pos = pe->rvas_and_sizes_raddr + ddir_idx*PE_SIZEOF_DATADIR_ENTRY;
    data = buf + data_directory_entry_raw_pos;
    if (sec == NULL)
    {
        write_int32_le_buf(data,0); data += 4; // directory entry rva
        write_int32_le_buf(data,0); data += 4; // directory entry size
        if (verbose)
            printf("PE data directory no %d cleared.\n",(int)ddir_idx);
    } else
    {
        write_int32_le_buf(data,sec->vaddr); data += 4; // directory entry rva
        write_int32_le_buf(data,sec->vsize); data += 4; // directory entry size
        if (verbose)
            printf("PE data directory no %d set to RVA=%08lXh.\n",(int)ddir_idx,sec->vaddr);
    }
    return ERR_OK;
}

/** Updates PE file characteristics flags.
 * Requires read_sections_list() to be called before.
 */
short pe_file_characteristics_flag_set(unsigned char *buf, long filesize,
        struct PEInfo *pe, unsigned long flag_up, unsigned long flag_down)
{
    unsigned char *data;
    unsigned long i;
    // Reading num. of sections
    data = buf + pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_CHARACTERISTICS_OFS;
    if (pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_SIZEOF_FILE_HEADER > filesize)
    {
      printf("The .DLL file has no valid 'new header'!\n");
      return ERR_BAD_FILE;
    }
    i = read_int16_le_buf(data);
    i |= flag_up;
    i &= ~flag_down;
    write_int16_le_buf(data,i);
    return ERR_OK;
}

/** Reads list of sections from PE file buffer into PEInfo structure */
short read_sections_list(unsigned char *buf, long filesize, struct PEInfo *pe)
{
    unsigned char *data;
    struct section_entry *sec;
    // Positions in headers
    unsigned long optional_header_raw_pos;
    unsigned long section_headers_raw_pos;
    long idx;
    // Locating PE header
    data = buf + MZ_NEWHEADER_OFS;
    pe->new_header_raddr = read_int32_le_buf(data);
    if ((pe->new_header_raddr < MZ_SIZEOF_HEADER) ||
    (pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_SIZEOF_FILE_HEADER+PE_SIZEOF_OPTN_HEADER > filesize))
    {
      printf("The .DLL file has no valid 'new header'!\n");
      if (verbose)
          printf("File is truncated or header offset is invalid.\n");
      return ERR_BAD_FILE;
    }
    // Reading num. of sections
    data = buf + pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_NUM_SECTIONS_OFS;
    pe->sections_num = read_int16_le_buf(data);
    if (pe->sections_num >= MAX_SECTIONS_NUM)
    {
      printf("The PE file has too many sections!\n");
      if (verbose)
          printf("Increasing MAX_SECTIONS_NUM to above %d will fix this.\n",(int)pe->sections_num);
      return ERR_BAD_FILE;
    }
    // Reading TimeDate stamp
    data = buf + pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_TIMEDATSTAMP_OFS;
    pe->timedatestamp = read_int32_le_buf(data);
    // Reading num. of RVAs and sizes
    optional_header_raw_pos = pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_SIZEOF_FILE_HEADER;
    data = buf + optional_header_raw_pos+PE_OPTH_NUM_RVAS_AND_SIZES;
    pe->rvas_and_sizes_num = read_int32_le_buf(data);
    pe->rvas_and_sizes_raddr = optional_header_raw_pos + PE_SIZEOF_OPTN_HEADER;
    // Reading image base address
    data = buf + optional_header_raw_pos+PE_OPTH_IMAGE_BASE;
    pe->image_base_vaddr = read_int32_le_buf(data);

    // Now we have section headers position
    section_headers_raw_pos = pe->rvas_and_sizes_raddr + pe->rvas_and_sizes_num*PE_SIZEOF_DATADIR_ENTRY;
//    if (verbose)
//      printf("File has %d sections, their headers are at RAW 0x%08lX.\n",pe->sections_num,section_headers_raw_pos);
    // Reading section headers
    for (idx=0; idx <= pe->sections_num; idx++)
    {
        pe->sections[idx] = malloc(sizeof(struct section_entry));
        if (pe->sections[idx] == NULL)
        {
          printf("Memory allocation error!\n");
          abort(); // won't make it to next line
          return ERR_NO_MEMORY;
        }
        sec = pe->sections[idx];
        memset(sec->name,0,MAX_SECTION_NAME_LEN+1);
        sec->vsize=0;
        sec->vaddr=0x0f000000;
        sec->rsize=0;
        sec->raddr=0x0f000000;
    }
    for (idx=0; idx < pe->sections_num; idx++)
    {
        sec = pe->sections[idx+1];
        data = buf + section_headers_raw_pos + idx*PE_SIZEOF_SECTHDR_ENTRY;
        // Read section name
        strncpy(sec->name, data, MAX_SECTION_NAME_LEN);
        sec->name[MAX_SECTION_NAME_LEN]='\0';
        data += MAX_SECTION_NAME_LEN;
        // Read addresses and sizes
        sec->vsize = read_int32_le_buf(data); data += 4;
        sec->vaddr = read_int32_le_buf(data); data += 4;
        sec->rsize = read_int32_le_buf(data); data += 4;
        sec->raddr = read_int32_le_buf(data); data += 4;
        //printf("Section %d: name '%s', data RAW 0x%08lX.\n",idx+1,sec->name,sec->raddr);
    }
    return ERR_OK;
}

short add_pe_section_datablock_to_buf(unsigned char **buf, long *filesize, struct section_entry *new_sec, struct PEInfo *pe)
{
    // Reallocate buf to fit the new section
    long old_size = (*filesize);
    *buf = realloc(*buf, old_size+new_sec->rsize);
    if (*buf == NULL)
    {
        printf("Memory re-allocation error!\n");
        abort(); // won't make it to next line
        return ERR_NO_MEMORY;
    }
    (*filesize) += new_sec->rsize;
    // Copy data at end of EXE to its new end
    if (old_size > new_sec->raddr)
    {
        memmove((*buf)+new_sec->raddr+new_sec->rsize, (*buf)+new_sec->raddr, old_size - new_sec->raddr);
    }
    // Fill the new section with '\0's
    memset((*buf)+new_sec->raddr,'\0',new_sec->rsize);
    if (verbose)
      printf("Body for PE section '%s', created at RAW address 0x%08lX.\n",new_sec->name,new_sec->raddr);
    return ERR_OK;
}

short add_pe_section_header_to_buf(unsigned char **buf, long *filesize, struct section_entry *new_sec, struct PEInfo *pe)
{
    unsigned char *data;
    struct section_entry *sec;
    // Positions in headers
    unsigned long optional_header_raw_pos;
    unsigned long section_headers_raw_pos;
    unsigned long first_section_data_raddr;
    long insert_idx;
    long idx;
    // Get section headers position
    section_headers_raw_pos = pe->rvas_and_sizes_raddr + pe->rvas_and_sizes_num*PE_SIZEOF_DATADIR_ENTRY;
    // Initial value of the first section offset
    first_section_data_raddr = (*filesize);
    insert_idx = 0;
    // Find a place to insert our new header
    for (idx=0; idx < pe->sections_num; idx++)
    {
        sec = pe->sections[idx+1];
        // Check section name
        if (strcmp(sec->name, new_sec->name) == 0)
        {
            printf("The PE file already contains a section named '%s'!\n",sec->name);
            if (verbose)
                printf("It is not possible to add two sections with same name.\n"
                       " Old section must be renamed or removed first.\n");
            return ERR_BAD_FILE;
        }
        // Check and update first section address
        if (first_section_data_raddr > sec->raddr)
            first_section_data_raddr = sec->raddr;
        // Check and update new section position
        if (new_sec->raddr > sec->raddr)
            insert_idx = idx+1;
    }
    // Check if we have place for a new section header
    if (section_headers_raw_pos + (pe->sections_num+1)*PE_SIZEOF_SECTHDR_ENTRY > first_section_data_raddr)
    {
        // TODO PeRESec: We could move all sections to make more space
        printf("There's not enough free space to add new section header!\n");
        if (verbose)
            printf("Adding a new section header would overwrite body of first section.\n"
                   " Add the new section manually, or reserve more space for section headers.\n");
        return ERR_BAD_FILE;
    }
    // Make empty space for our new section
    data = (*buf) + section_headers_raw_pos + insert_idx*PE_SIZEOF_SECTHDR_ENTRY;
    if (insert_idx < pe->sections_num)
    {
        memmove(data+PE_SIZEOF_SECTHDR_ENTRY, data, (pe->sections_num-insert_idx)*PE_SIZEOF_SECTHDR_ENTRY);
    }
    // Clear the new section
    memset(data, '\0', PE_SIZEOF_SECTHDR_ENTRY);
    // Write section name
    strncpy(data, new_sec->name, MAX_SECTION_NAME_LEN);
    data += MAX_SECTION_NAME_LEN;
    // Write addresses and sizes
    write_int32_le_buf(data,new_sec->vsize); data += 4; // virtual size
    write_int32_le_buf(data,new_sec->vaddr); data += 4; // virtual address
    write_int32_le_buf(data,new_sec->rsize); data += 4; // RAW size
    write_int32_le_buf(data,new_sec->raddr); data += 4; // RAW address
    data += 12;
    write_int32_le_buf(data,0x40000040); data += 4; // Characteristics (flags)
    // Update sections count in PE header
    data = (*buf) + pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_NUM_SECTIONS_OFS;
    write_int16_le_buf(data,pe->sections_num+1);
    // Update size of image in PE header
    optional_header_raw_pos = pe->new_header_raddr+PE_SIZEOF_SIGNATURE+PE_SIZEOF_FILE_HEADER;
    data = (*buf) + optional_header_raw_pos+PE_OPTH_SIZE_OF_IMAGE;
    idx = read_int32_le_buf(data);
    write_int32_le_buf(data,idx+new_sec->rsize);
    if (verbose)
      printf("Header for new PE section no %d, '%s', created.\n",(int)insert_idx+1,new_sec->name);
    return ERR_OK;
}

/** Get index of a section in PEInfo structure with given name. */
long get_section_index(const char *sec_name, struct PEInfo *pe)
{
    long idx;
    for (idx=0; idx <= pe->sections_num; idx++)
    {
        if (stricmp(pe->sections[idx]->name,sec_name) == 0)
        { return idx; }
    }
    return -1;
}

/** Get index of a section in PEInfo structure containing given RVA. */
long get_rva_section_index(long rva, struct PEInfo *pe)
{
    long idx;
    for (idx=0; idx <= pe->sections_num; idx++)
    {
        if ((rva >= pe->sections[idx]->vaddr) && (rva < pe->sections[idx]->vaddr+pe->sections[idx]->vsize))
        { return idx; }
    }
    return -1;
}

/** Reads relocations from given section into PEInfo struct. */
short read_relocations_section(unsigned char *buf, long filesize, struct PEInfo *pe)
{
    long idx,pos,endpos;
    unsigned char *data;
    long base_vaddr, base_size, rva, k;
    struct relocation_entry *rel;

    pe->relocations_num = 0;
    // Find the relocations section
    struct section_entry *rel_sec;
    idx = get_section_index(relocations_section_name,pe);
    if (idx < 1)
    {
        printf("Cannot locate entry '%s' in section headers!\n",relocations_section_name);
        if (verbose)
            printf("To fix the problem, create relocations section or rename it.\n");
        return ERR_BAD_FILE;
    }
    rel_sec = pe->sections[idx];
    printf("Relocations section '%s' located at RAW %08lXh.\n",relocations_section_name,(long)rel_sec->raddr);
    if (verbose)
        printf("Section size is %ld bytes (%08lXh).\n",(long)rel_sec->rsize,(long)rel_sec->rsize);
    pos = rel_sec->raddr;
    endpos = pos + rel_sec->rsize;
    while (pos+RELOCATION_BASE_SIZE < endpos)
    {
        data = buf + pos;
        base_vaddr = read_int32_le_buf(data+0);
        base_size = read_int32_le_buf(data+4);
        if (base_size == 0)
            break;
        if (base_size < RELOCATION_BASE_SIZE+RELOCATION_OFFSET_SIZE)
        {
            printf("Invalid relocations base size, %ld!\n",base_size);
            break;
        }
        for (idx=RELOCATION_BASE_SIZE; idx < base_size; idx+=RELOCATION_OFFSET_SIZE)
        {
            if (pe->relocations_num >= MAX_RELOCATIONS)
            {
                printf("Cannot load more than %d relocations!\n",MAX_RELOCATIONS);
                if (verbose)
                    printf("Increase MAX_RELOCATIONS and recompile PeRESec to fix this.\n");
                return ERR_NO_MEMORY;
            }
            rel = &pe->relocations[pe->relocations_num];
            k = read_int16_le_buf(data+idx);
            rel->method = (k >> 12) & 0x0f;
            // Ignore "absolute" relocations - they are only placefillers.
            if (rel->method == 0)
                continue;
            // Find out which section contains the relocation RVA
            rva = base_vaddr + (k & 0x0fff);
            k = get_rva_section_index(rva, pe);
            if (k < 0)
            {
                printf("Cannot find section containing RVA=0x%08lX.\n",rva);
                if (verbose)
                    printf("The address is incorrect and have been skipped.\n");
                continue;
            }
            // Write relocation entry (rel->method was set before)
            rel->seg = k;
            rel->offs = rva - pe->sections[k%MAX_SECTIONS_NUM]->vaddr;
            pe->relocations_num++;
        }
        pos += base_size;
    }
    if (verbose)
        printf("Got %ld relocations from .DLL file.\n",(long)pe->relocations_num);
    return ERR_OK;
}

/** Reads exports from given section into PEInfo struct. */
short read_exports_section(unsigned char *buf, long filesize, struct PEInfo *pe)
{
    long idx;

    pe->exports_num = 0;
    // Find the export section
    struct section_entry *exp_sec;
    idx = get_section_index(export_section_name,pe);
    if (idx < 1)
    {
        printf("Cannot locate entry '%s' in section headers!\n",export_section_name);
        if (verbose)
            printf("To fix the problem, create exports section or rename it.\n");
        return ERR_BAD_FILE;
    }
    exp_sec = pe->sections[idx];
    printf("Exports section '%s' located at RAW %08lXh.\n",export_section_name,(long)exp_sec->raddr);
    if (verbose)
        printf("Section size is %ld bytes (%08lXh).\n",(long)exp_sec->rsize,(long)exp_sec->rsize);

    //TODO PeRESec: Should read exports from .DLL, not from .MAP!
    //pe->exports_num = read_map(opts.fname_map,pe->exports);
    printf("This function is NOT SUPPORTED (yet)!\n");

    if (verbose)
        printf("Got %ld exports from .DLL file.\n",(long)pe->exports_num);
    return ERR_OK;
}

/** Returns expected size of export section in given PE buffer.
 *  This function should exactly correspond to the commands in
 *  update_pe_export_section_content(), and return size of the section
 *  created by that function.
 */
long compute_pe_export_section_size(const char *module_name, struct PEInfo *pe)
{
    long idx,pos;
    int module_name_size = strlen(module_name)+1;
    while ((module_name_size%8)!=0) module_name_size++;
    // Offsets relative to position of export section
    const unsigned long func_namestr_ofs   = EXPORT_DIRECTORY_SIZE + (pe->exports_num)*4 + (pe->exports_num)*2 + (pe->exports_num)*4 + module_name_size;
    // Adding function names
    pos = func_namestr_ofs;
    for (idx=0; idx < pe->exports_num; idx++)
    {
      char *name = pe->exports[idx]->dstname;
      long n = strlen(name)+1;
      pos += n;
    }
    return pos;
}

/** Updates export section in given PE buffer.
 * Uses PEInfo structure to locate and obtain new parameters of the section.
 * Requires section list to be loaded and exports filled in PEInfo before call.
 */
short update_pe_export_section_content(unsigned char *buf, long filesize,
      const char *module_name, struct PEInfo *pe)
{
    long idx,pos;
    unsigned char *data;
    int module_name_size = strlen(module_name)+1;
    while ((module_name_size%8)!=0) module_name_size++;

    // Find the export section
    struct section_entry *exp_sec;
    idx = get_section_index(export_section_name,pe);
    if (idx < 1)
    {
        printf("Cannot locate entry '%s' in section headers!\n",export_section_name);
        if (verbose)
            printf("Create the export section, or rename it to fix the problem.\n");
        return ERR_BAD_FILE;
    }
    exp_sec = pe->sections[idx];
    printf("Export section '%s' located at RAW %08lXh.\n",export_section_name,(long)exp_sec->raddr);
    if (verbose)
        printf("Section size is %ld bytes (%08lXh).\n",(long)exp_sec->rsize,(long)exp_sec->rsize);

    // Offsets relative to position of export section; these are computed using
    // the constants defined above
    const unsigned long adress_table_ofs   = EXPORT_DIRECTORY_SIZE;
    const unsigned long ordinal_table_ofs  = EXPORT_DIRECTORY_SIZE + (pe->exports_num)*4;
    const unsigned long fnnames_table_ofs  = EXPORT_DIRECTORY_SIZE + (pe->exports_num)*4 + (pe->exports_num)*2;
    const unsigned long module_namestr_ofs = EXPORT_DIRECTORY_SIZE + (pe->exports_num)*4 + (pe->exports_num)*2 + (pe->exports_num)*4;
    const unsigned long func_namestr_ofs   = EXPORT_DIRECTORY_SIZE + (pe->exports_num)*4 + (pe->exports_num)*2 + (pe->exports_num)*4 + module_name_size;

    // Computing offsets inside the export section
    unsigned long address_table_raw_pos  = exp_sec->raddr+adress_table_ofs;
    unsigned long ordinal_table_raw_pos  = exp_sec->raddr+ordinal_table_ofs;
    unsigned long fnnames_table_raw_pos  = exp_sec->raddr+fnnames_table_ofs;
    unsigned long module_namestr_raw_pos = exp_sec->raddr+module_namestr_ofs;
    unsigned long func_namestr_raw_pos   = exp_sec->raddr+func_namestr_ofs;
    long          max_namestrs_size      = exp_sec->rsize-func_namestr_ofs;
    long          arr_raw_to_rva         = exp_sec->vaddr-exp_sec->raddr;

    if (max_namestrs_size < MAX_EXPORT_NAMELEN)
    {
        printf("Cannot put %d entries in small export section!\n",MAX_EXPORT_NAMELEN);
        if (verbose)
            printf("Cut the .MAP file or make bigger section to fix this.\n");
        return ERR_BAD_FILE;
    }

    // Writing module name
    data = buf + module_namestr_raw_pos;
    for (idx=0;idx<strlen(module_name);idx++)
        data[idx] = module_name[idx];
    for (;idx<module_name_size;idx++)
        data[idx] = '\0';
    // Writing function names
    pos = func_namestr_raw_pos;
    for (idx=0; idx < pe->exports_num; idx++)
    {
      if (pe->exports[idx]==NULL) break;
      char *name = pe->exports[idx]->dstname;
      pe->exports[idx]->nmoffs=pos+arr_raw_to_rva;
      if (pos+strlen(name) >= func_namestr_raw_pos+max_namestrs_size)
      {
          printf("Function names space exceeded on func. %ld!\n",idx);
          if (verbose)
              printf("Cut the .MAP file or make bigger section to fix this.\n");
          pe->exports[idx]=NULL;
          break;
      }
      data = buf + pos;
      long n = strlen(name)+1;
      memcpy(data,name,n);
      pos += n;
    }
    long end_of_used_space = pos;
    long remain_bts = func_namestr_raw_pos+max_namestrs_size-end_of_used_space-strlen(export_end_str);
    if (remain_bts>=0)
    {
        data = buf + end_of_used_space;
        while (remain_bts>0)
        {
            *data = '\0';
            data++;
            remain_bts--;
        }
        memcpy(data,export_end_str,strlen(export_end_str));
    }
    printf("Created %ld function export names in .DLL.\n",idx);
    // Updating section header
    data = buf + exp_sec->raddr;
    {
        // export flags
        write_int32_le_buf(data, 0); data += 4;
        // export table creation date
        time_t dtime; time(&dtime);
        write_int32_le_buf(data, dtime); data += 4;
        // export table version
        write_int32_le_buf(data, 0); data += 4;
        // module name address
        write_int32_le_buf(data, arr_raw_to_rva+module_namestr_raw_pos); data += 4;
        // ordinal base
        write_int32_le_buf(data, 1); data += 4;
        // number of functions
        write_int32_le_buf(data, idx); data += 4;
        // number of names
        write_int32_le_buf(data, idx); data += 4;
        // address of functions
        write_int32_le_buf(data, arr_raw_to_rva+address_table_raw_pos); data += 4;
        // address of names
        write_int32_le_buf(data, arr_raw_to_rva+fnnames_table_raw_pos); data += 4;
        // address of ordinals
        write_int32_le_buf(data, arr_raw_to_rva+ordinal_table_raw_pos); data += 4;
    }
    if (verbose)
        printf("Export section header updated.\n");
    data = buf + ordinal_table_raw_pos;
    for (idx=0; idx < pe->exports_num; idx++)
    {
      if (pe->exports[idx] == NULL)
      {
        write_int16_le_buf(data, 0);
      } else
      {
        write_int16_le_buf(data, idx);
      }
      data += 2;
    }
    if (verbose)
        printf("Created %ld function export ordinals in .DLL.\n",idx);
    data = buf + address_table_raw_pos;
    for (idx=0; idx < pe->exports_num; idx++)
    {
      if (pe->exports[idx] == NULL)
      {
        write_int32_le_buf(data, 0);
      } else
      {
        unsigned long val = pe->exports[idx]->offs+pe->sections[(pe->exports[idx]->seg)%MAX_SECTIONS_NUM]->vaddr;
        write_int32_le_buf(data, val);
      }
      data += 4;
    }
    if (verbose)
        printf("Created %ld function addresses in .DLL.\n",idx);
    data = buf + fnnames_table_raw_pos;
    for (idx=0; idx < pe->exports_num; idx++)
    {
      if (pe->exports[idx] == NULL)
      {
        write_int32_le_buf(data, 0);
      } else
      {
        unsigned long val=pe->exports[idx]->nmoffs;
        write_int32_le_buf(data, val);
      }
      data += 4;
    }
    if (verbose)
        printf("Placed %ld function name offsets in .DLL.\n",idx);
    long esection_used = end_of_used_space-exp_sec->raddr;
    long esection_free = exp_sec->rsize-esection_used;
    printf("Used %ld bytes in the export section; %ld remain free.\n",esection_used,esection_free);
    return ERR_OK;
}

short create_or_update_export_section(unsigned char **buf, long *filesize,
        const char *module_name, struct PEInfo *pe)
{
    struct section_entry *sec_ptr;
    struct section_entry sectn;
    long idx;
    short ret;
    sectn.rsize = compute_pe_export_section_size(module_name, pe);
    // Find the export section
    idx = get_section_index(export_section_name,pe);
    // If the export section already exists
    if (idx >= 0)
    {
        // Check size of the existing section
        sec_ptr = pe->sections[idx];
        if (sec_ptr->rsize < sectn.rsize)
        {
            printf("Export section '%s' has only %d bytes, but %d bytes are needed.\n",export_section_name,(int)sec_ptr->rsize,(int)sectn.rsize);
            if (verbose)
                printf("Section is too small. Remove some MAP entries,\n"
                      " or delete the section to allow its re-creation.\n");
            return ERR_BAD_FILE;
        }
    }
    // If the export section doesn't exist, create it
    if (idx < 0)
    {
        // Get the last section
        sec_ptr = pe->sections[pe->sections_num];
        // And add the new one after it
        sectn.rsize = align_file_section_size(*buf, *filesize, sectn.rsize, pe);
        sectn.vsize = align_virt_section_size(*buf, *filesize, sectn.rsize, pe);
        sectn.vaddr = align_virt_section_size(*buf, *filesize, sec_ptr->vaddr+sec_ptr->rsize, pe);
        sectn.raddr = align_file_section_size(*buf, *filesize, sec_ptr->raddr+sec_ptr->rsize, pe);
        strcpy(sectn.name,export_section_name);
        ret = add_pe_section_header_to_buf(buf, filesize, &sectn, pe);
        if (ret == ERR_OK)
            ret = read_sections_list(*buf, *filesize, pe);
        if (ret == ERR_OK)
            ret = add_pe_section_datablock_to_buf(buf, filesize, &sectn, pe);
        if (ret == ERR_OK)
            ret = set_pe_data_directory_to_section(*buf, *filesize, &sectn, PE_DATA_DIRECTORY_EXPORT, pe);
        if (ret != ERR_OK)
            return ret;
    }
  //Updating export section in memory image of the file
  ret = update_pe_export_section_content(*buf, *filesize, module_name, pe);
  return ret;
}

long compute_pe_relocation_section_size(struct PEInfo *pe)
{
    long idx;
    long data,lastbasesize;
    long base_vaddr, base_size, rva, k;
    struct relocation_entry *rel;

    data = 0;
    lastbasesize = 0;
    base_size = RELOCATION_BASE_SIZE;
    base_vaddr = -0x0fff;
    for (idx=0; idx < pe->relocations_num; idx++)
    {
        rel = &pe->relocations[idx];
        rva = rel->offs+pe->sections[(rel->seg)%MAX_SECTIONS_NUM]->vaddr;
        if ((rva < base_vaddr) || (rva > base_vaddr+0x0fff))
        {
            // Update size of the previous base
            if (lastbasesize != 0)
            {
                // Check if we have enough space
                // Add dummy entry to make the position multiplication of 4
                if ((base_size & 0x03) == 2)
                {
                    base_size += RELOCATION_OFFSET_SIZE;
                    data += RELOCATION_OFFSET_SIZE;
                }
            }
            // start new base
            k = (rva & 0x0fff);
            base_vaddr = rva - k;
            base_size = RELOCATION_BASE_SIZE;
            lastbasesize = data+4;
            data += RELOCATION_BASE_SIZE;
        }
        base_size += RELOCATION_OFFSET_SIZE;
        data += RELOCATION_OFFSET_SIZE;
    }
    // Finish the last base
    if (lastbasesize != 0)
    {
        // Add dummy entry to make the position multiplication of 4
        if ((base_size & 0x03) == 2)
        {
            base_size += RELOCATION_OFFSET_SIZE;
            data += RELOCATION_OFFSET_SIZE;
        }
    }
    // Add dummy base entry at end
    data += RELOCATION_OFFSET_SIZE;
    return data;
}

/** Updates export section in given PE buffer.
 * Uses PEInfo structure to locate and obtain new parameters of the section.
 * Requires section list to be loaded and exports filled in PEInfo before call.
 */
short update_pe_relocation_section_content(unsigned char *buf, long filesize,
      struct PEInfo *pe)
{
    long idx;
    unsigned char *data,*enddata,*lastbasesize;
    long base_vaddr, base_size, rva, k;
    struct relocation_entry *rel;

    // Find the relocations section
    struct section_entry *rel_sec;
    idx = get_section_index(relocations_section_name,pe);
    if (idx < 1)
    {
        printf("Cannot locate entry '%s' in section headers!\n",relocations_section_name);
        if (verbose)
            printf("To fix the problem, create relocations section or rename it.\n");
        return ERR_BAD_FILE;
    }
    rel_sec = pe->sections[idx];
    printf("Relocations section '%s' located at RAW %08lXh.\n",relocations_section_name,(long)rel_sec->raddr);
    if (verbose)
        printf("Section size is %ld bytes (%08lXh).\n",(long)rel_sec->rsize,(long)rel_sec->rsize);
    data = buf + rel_sec->raddr;
    enddata = data + rel_sec->rsize;
    lastbasesize = NULL;
    base_size = RELOCATION_BASE_SIZE;
    base_vaddr = -0x0fff;
    for (idx=0; idx < pe->relocations_num; idx++)
    {
        rel = &pe->relocations[idx];
        rva = rel->offs+pe->sections[(rel->seg)%MAX_SECTIONS_NUM]->vaddr;
        if ((rva < base_vaddr) || (rva > base_vaddr+0x0fff))
        {
            // Update size of the previous base
            if (lastbasesize != NULL)
            {
                // Check if we have enough space
                if (data+RELOCATION_BASE_SIZE+RELOCATION_OFFSET_SIZE >= enddata)
                {
                    printf("Section is too small to store base of relocation %ld!\n",idx);
                    if (verbose)
                        printf("Resize or remove the relocation section to fix it.\n");
                    return ERR_NO_MEMORY;
                }
                // Add dummy entry to make the position multiplication of 4
                if ((base_size & 0x03) == 2)
                {
                    base_size += RELOCATION_OFFSET_SIZE;
                    write_int16_le_buf(data,0);
                    data += RELOCATION_OFFSET_SIZE;
                }
                // Write the base size
                write_int32_le_buf(lastbasesize,base_size);
            }
            // start new base
            k = (rva & 0x0fff);
            base_vaddr = rva - k;
            base_size = RELOCATION_BASE_SIZE;
            write_int32_le_buf(data+0,base_vaddr);
            write_int32_le_buf(data+4,base_size);
            lastbasesize = data+4;
            data += RELOCATION_BASE_SIZE;
        }
        // add entry to the base
        if (data+RELOCATION_OFFSET_SIZE >= enddata)
        {
            printf("Section is too small to store relocation %ld!\n",idx);
            if (verbose)
                printf("Resize or remove the relocation section to fix it.\n");
            return ERR_NO_MEMORY;
        }
        k = (rva - base_vaddr) | (rel->method << 12);
        base_size += RELOCATION_OFFSET_SIZE;
        write_int16_le_buf(data,k);
        data += RELOCATION_OFFSET_SIZE;
    }
    // Finish the last base
    if (lastbasesize != NULL)
    {
        // Check if we have enough space
        if (data+RELOCATION_BASE_SIZE >= enddata)
        {
            printf("Section is too small to store base of relocation %ld!\n",idx);
            if (verbose)
                printf("Resize or remove the relocation section to fix it.\n");
            return ERR_NO_MEMORY;
        }
        // Add dummy entry to make the position multiplication of 4
        if ((base_size & 0x03) == 2)
        {
            base_size += RELOCATION_OFFSET_SIZE;
            write_int16_le_buf(data,0);
            data += RELOCATION_OFFSET_SIZE;
        }
        // Write the base size
        write_int32_le_buf(lastbasesize,base_size);
    }
    // Add dummy base entry at end
    write_int32_le_buf(data+0,0);
    write_int32_le_buf(data+4,0);
    data += RELOCATION_OFFSET_SIZE;
    printf("Placed %ld relocations in .DLL.\n",idx);
    return ERR_OK;
}

short create_or_update_relocation_section(unsigned char **buf, long *filesize,
        struct PEInfo *pe)
{
    struct section_entry *sec_ptr;
    struct section_entry sectn;
    long idx;
    short ret;
    sectn.rsize = compute_pe_relocation_section_size(pe);
    // Find the relocation section
    idx = get_section_index(relocations_section_name,pe);
    // If the relocation section already exists
    if (idx >= 0)
    {
        // Check size of the existing section
        sec_ptr = pe->sections[idx];
        if (sec_ptr->rsize < sectn.rsize)
        {
            printf("Relocation section '%s' has only %d bytes, but %d bytes are needed.\n",relocations_section_name,(int)sec_ptr->rsize,(int)sectn.rsize);
            if (verbose)
                printf("Section is too small. Remove some .RMAP entries,\n"
                      " or delete the section to allow its re-creation.\n");
            return ERR_BAD_FILE;
        }
    }
    // If the relocation section doesn't exist, create it
    if (idx < 0)
    {
        // Get the last section
        sec_ptr = pe->sections[pe->sections_num];
        // And add the new one after it
        sectn.rsize = align_file_section_size(*buf, *filesize, sectn.rsize, pe);
        sectn.vsize = align_virt_section_size(*buf, *filesize, sectn.rsize, pe);
        sectn.vaddr = align_virt_section_size(*buf, *filesize, sec_ptr->vaddr+sec_ptr->rsize, pe);
        sectn.raddr = align_file_section_size(*buf, *filesize, sec_ptr->raddr+sec_ptr->rsize, pe);
        strcpy(sectn.name,relocations_section_name);
        ret = add_pe_section_header_to_buf(buf, filesize, &sectn, pe);
        if (ret == ERR_OK)
            ret = read_sections_list(*buf, *filesize, pe);
        if (ret == ERR_OK)
            ret = add_pe_section_datablock_to_buf(buf, filesize, &sectn, pe);
        if (ret == ERR_OK)
            ret = set_pe_data_directory_to_section(*buf, *filesize, &sectn, PE_DATA_DIRECTORY_BASERELOC, pe);
        if (ret != ERR_OK)
            return ret;
    }
    //Updating relocation section in memory image of the file
    ret = update_pe_relocation_section_content(*buf, *filesize, pe);
    return ret;
}

int main(int argc, char *argv[])
{
    static struct PEInfo peinfo;
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
    unsigned char *dll_data;
    long dll_size;
    int idx;
    for (idx=0;idx<MAX_EXPORTS;idx++)
        peinfo.exports[idx]=NULL;
    if ((opts.fname_inp == NULL) || (opts.fname_map == NULL) || (opts.fname_def == NULL)
     || (opts.fname_out == NULL) || (opts.module_name == NULL))
    {
        printf("Memory allocation error!\n");
        abort();
    }
  // Reading functions
  peinfo.exports_num = read_map(opts.fname_map,peinfo.exports);
  if (peinfo.exports_num < 0)
  {
      free_prog_options(&opts);
      return 1;
  }
  // Creating destination names from input names
  export_fill_dstnames(peinfo.exports,peinfo.exports_num,opts.funcname_prefix);
  // Sorting functions
  export_sort(peinfo.exports,peinfo.exports_num);
  if (verbose)
      printf("Entries are now sorted in memory.\n");
  // Checking if there are no entries with same names
  int dupidx;
  dupidx = find_dupename(peinfo.exports,peinfo.exports_num);
  if (dupidx >= 0)
  {
      if (verbose)
          printf("Duplicate entry name found!\n");
      printf("Entry \"%s\" duplicates. Aborting.\n",peinfo.exports[dupidx]->dstname);
      if (verbose)
          printf("Remove duplicated entry from .MAP file to fix this.\n");
      free_prog_options(&opts);
      return 7;
  }
  // Checking if there are no entries with same offsets
  dupidx = find_dupeoffs(peinfo.exports,peinfo.exports_num);
  if (dupidx >= 0)
  {
      if (verbose)
          printf("Duplicate entry offset found!\n");
      printf("Offset 0x%08lX duplicates. Aborting.\n",peinfo.exports[dupidx]->offs);
      if (verbose)
          printf("Remove duplicated entry from .MAP file to fix this.\n");
      free_prog_options(&opts);
      return 8;
  }

  //Loading the PE file to be updated
  dll_size = read_file_to_memory(opts.fname_inp,&dll_data);
  if (dll_size < 0)
  {
      free_prog_options(&opts);
      return 1;
  }
  //Reading list of sections from memory image of the file
  if (read_sections_list(dll_data, dll_size, &peinfo) != ERR_OK)
  {
      free_prog_options(&opts);
      free(dll_data);
      return 9;
  }
  // Now, are we here for creating DLL, or extracting from input file?
  if (extract)
  {
      // Writing relocations into .rmap file
      if(opts.fname_rmap != NULL)
      {
          if (read_relocations_section(dll_data, dll_size, &peinfo) != ERR_OK)
          {
              free_prog_options(&opts);
              free(dll_data);
              return 9;
          }
          if (create_rmap(opts.fname_rmap,opts.fname_out,&peinfo) != ERR_OK)
          {
              free_prog_options(&opts);
              free(dll_data);
              return 10;
          }
      }
      // Writing exports into .map file
      if (read_exports_section(dll_data, dll_size, &peinfo) == ERR_OK)
      {
          if (peinfo.exports_num > 0)
          {
              if (create_map(opts.fname_map,opts.fname_out,&peinfo) != ERR_OK)
              {
                  free_prog_options(&opts);
                  free(dll_data);
                  return 10;
              }
          }
      }
      free(dll_data);
  } else
  {
      //Setting "is a DLL" flag in characteristics in memory image of the file
      //We're not un-setting "is executable" flag - DLLs seem to have it always set, too.
      if (pe_file_characteristics_flag_set(dll_data,dll_size,&peinfo, 0x2000, 0x0000) != ERR_OK)
      {
          free_prog_options(&opts);
          free(dll_data);
          return 9;
      }
      //Creating or updating export section in memory image of the file
      if (create_or_update_export_section(&dll_data,&dll_size,opts.module_name,&peinfo) != ERR_OK)
      {
          free_prog_options(&opts);
          free(dll_data);
          return 9;
      }
      //Creating or updating relocations section in memory image of the file
      if(opts.fname_rmap != NULL)
      {
          peinfo.relocations_num = read_rmap(opts.fname_rmap, peinfo.relocations);
          if (peinfo.relocations_num >= 0)
          {
              relocation_sort(peinfo.relocations, peinfo.relocations_num);
              //Creating or updating relocation section in memory image of the file
              if (create_or_update_relocation_section(&dll_data,&dll_size,&peinfo) != ERR_OK)
              {
                  free_prog_options(&opts);
                  free(dll_data);
                  return 9;
              }
          }
      }
      //Saving the resulting file to disk
      if (write_file_from_memory(opts.fname_out,&dll_data,dll_size) != ERR_OK)
      {
          free_prog_options(&opts);
          return 10;
      }
      if (create_def(opts.fname_def,opts.fname_out,&peinfo, get_def_format(opts.def_format)) != ERR_OK)
      {
          free_prog_options(&opts);
          return 10;
      }
  }
  free_prog_options(&opts);
  return 0;
}
