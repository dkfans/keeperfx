/******************************************************************************/
/** @file dernc.c
 * RNC decompression support.
 * @par Purpose:
 *   Compiled normally, this file is a well-behaved, re-entrant code
 *   module exporting only `rnc_ulen', `rnc_unpack' and `rnc_error'.
 *   Compiled with MAIN_DERNC defined, it's a standalone program which will
 *   decompress argv[1] into argv[2].
 * @par Comment:
 *   in/out buffers should have 8 redundant "safe bytes" at end.
 * @author   Tomasz Lis
 * @author   Jon Skeet
 * @date     14 Oct 1997 - 22 Jul 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifdef MAIN_DERNC
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#endif

#define INTERNAL
#include "lblogging.h"
#include "lbfileio.h"
#include "dernc.h"

#ifdef MAIN_DERNC
int main_unpack (char *pname, char *iname, char *oname);
int copy_file (char *iname, char *oname);

/**
 * Shows usage if building stand-alone DeRNC tool.
 * @param fname Name of the executable file.
 * @return Returns 1 on success.
 */
short show_usage(char *fname)
{
    printf("usage:\n");
    printf("    %s <files>\n", fname);
    printf(" or\n");
    printf("    %s -o <infile> <outfile>\n", fname);
    return 1;
}

/**
 * Main function if building stand-alone DeRNC tool.
 * @param argc Command line arguments count.
 * @param argv Command line arguments vector.
 * @return Returns 0 on success.
 */
int main(int argc, char **argv)
{
    int mode=0;
    int i;

    printf("\nPRO-PACK's alternate RNC files decompressor\n");
    printf("-------------------------------\n");

    if (argc<2)
    {
        show_usage(*argv);
        return 0;
    }
    for (i=1; i < argc; i++)
      if (!strcmp (argv[i], "-o"))
        mode=i;
    if (mode && argc != 4)
    {
        show_usage(*argv);
        return 1;
    }
    switch (mode)
    {
    case 0:
        for (i=1; i < argc; i++)
        {
          printf("Extracting %s...\n",argv[i]);
          if (main_unpack(*argv, argv[i], argv[i]))
            return 1;
        }
        return 0;
    case 1:
        printf("Extracting %s to %s...\n",argv[2],argv[3]);
        return main_unpack(*argv, argv[2], argv[3]);
    case 2:
        printf("Extracting %s to %s...\n",argv[1],argv[3]);
        return main_unpack(*argv, argv[1], argv[3]);
    case 3:
        printf("Extracting %s to %s...\n",argv[1],argv[2]);
        return main_unpack(*argv, argv[1], argv[2]);
    default:
        ERRORF("Internal fault.");
    }
    return 1;
}

/**
 * Decompresses single file if building stand-alone DeRNC tool.
 * @param pname Name of the program executable.
 * @param iname File name of the compressed input file.
 * @param oname File name of the decompressed output.
 * @return Returns 0 on success. On error prints a message
 *     and returns nonzero value.
 */
int main_unpack (char *pname, char *iname, char *oname)
{
    FILE *ifp, *ofp;
    long plen, ulen;
    void *packed, *unpacked;
    char buffer[4];

    ifp = fopen(iname, "rb");
    if (!ifp)
    {
        ERRORF("%s: %s",iname,strerror(errno));
        return 1;
    }
    //Checking if the file is RNC
    fseek (ifp, 0L, SEEK_END);
    plen = ftell (ifp);
    rewind (ifp);
    if (plen < 4) // Can't be an RNC file
    {
        if (strcmp (iname, oname))
            return copy_file (iname, oname);
        return 0;
    }
    fread(buffer, 1, 4, ifp);
    if (strncmp(buffer, "RNC", 3) != 0)
    {
      fclose (ifp);
      if (strcmp(iname, oname) != 0)
        return copy_file(iname, oname);
      return 0;
    }
    rewind(ifp);
    //Reading compressed data, 8 bytes in buffer are for safety
    packed = malloc(plen+8);
    if (packed==NULL)
    {
        fclose(ifp);
        ERRORF("%s: %s",pname,strerror(errno));
        return 1;
    }
    unsigned long rdlen;
    rdlen=fread(packed, 1, plen, ifp);
    fclose(ifp);
    //Getting unpacked file size & allocating space
    ulen = rnc_ulen(packed);
    if (ulen < 0)
    {
        free(packed);
        if (ulen == -1) // File wasn't RNC to start with
          return 0;
        printf("Error: %s\n", rnc_error (ulen));
        return 1;
    }

    //Creating output buffer, 8 bytes are for safety
    unpacked = malloc(ulen+8);
    if (unpacked == NULL)
    {
        free(packed);
        ERRORF("%s: %s",pname,strerror(errno));
        return 1;
    }

    //Do the decompression
    ulen = rnc_unpack (packed, unpacked, RNC_IGNORE_NONE);
    if (ulen < 0)
    {
    printf("%s\n", rnc_error(ulen));
    return 1;
    }

    //Write results to a file
    ofp = fopen(oname, "wb");
    if (!ofp)
    {
        ERRORF("%s: %s",oname,strerror(errno));
        return 1;
    }

    fwrite (unpacked, 1, ulen, ofp);
    fclose (ofp);

    free (unpacked);
    free (packed);

    return 0;
}

/**
 * Copies single file if building stand-alone DeRNC tool.
 * @param iname Source file name.
 * @param oname Destination file name.
 * @return Returns 0 on success. On error prints a message
 *     and returns nonzero value.
 */
int copy_file (char *iname, char *oname)
{
    char *sysbuf;

    sysbuf = malloc(strlen(iname)+strlen(oname)+6);
    if (sysbuf == NULL)
    {
      ERRORF("Out of memory.");
      return 1;
    }
    strcpy(sysbuf, "cp ");
    strcat(sysbuf, iname);
    strcat(sysbuf, " ");
    strcat(sysbuf, oname);
    system(sysbuf);
    free(sysbuf);
    return 0;
}

#endif

/**
 * Holds between 16 and 32 bits from packed buffer.
 * It can be used to read up to 16 bits from input buffer.
 * The buffer pointer in not in the struct, and it is increased
 * to always point at start of the byte which wasn't read yet.
 */
typedef struct {
    unsigned long bitbuf;       // data bits
    int bitcount;               // how many bits does bitbuf hold?
} bit_stream;

#define HUFTABLE_ENTRIES 32

/**
 * Huffman code table, used for decompression.
 */
typedef struct {
    int num;                   // number of nodes in the tree
    struct {
    unsigned long code;
    int codelen;
    int value;
    } table[HUFTABLE_ENTRIES];
} huf_table;

#include "globals.h"

static void read_huftable (huf_table *h, bit_stream *bs,
                   const unsigned char **p, const unsigned char *pend);
static unsigned long huf_read (huf_table *h, bit_stream *bs,
                   const unsigned char **p,const unsigned char *pend);

inline void bitread_init (bit_stream *bs, const unsigned char **p, const unsigned char *pend);
inline void bitread_fix (bit_stream *bs, const unsigned char **p, const unsigned char *pend);
inline unsigned long bit_peek (bit_stream *bs, unsigned long mask);
inline void bit_advance (bit_stream *bs, int n,
                   const unsigned char **p, const unsigned char *pend);
static unsigned long bit_read (bit_stream *bs, unsigned long mask,
                   int n, const unsigned char **p, const unsigned char *pend);

static unsigned long mirror (unsigned long x, int n);

/**
 * Returns an error string corresponding to an error code.
 * @param errcode Error code returned by a function.
 * @return Error message string.
 */
char *rnc_error (long errcode)
{
    static char *const errors[] = {
    "No error",
    "File is not RNC-1 format",
    "Huffman decode error",
    "File size mismatch",
    "CRC error in packed data",
    "CRC error in unpacked data",
    "Compressed file header invalid",
    "Huffman decode leads outside buffers",
    "Unknown error"
    };

    errcode = -errcode;
    if (errcode < 0)
    errcode = 0;
    if (errcode > sizeof(errors)/sizeof(*errors) - 1)
    errcode = sizeof(errors)/sizeof(*errors) - 1;
    return errors[errcode];
}

/**
 * Return the uncompressed length of a packed data block.
 * The packed buffer must be longer than SIZEOF_RNC_HEADER.
 * @param packed Packed buffer.
 * @return Uncompressed length of the packed block,
 *     or a negative error code.
 */
long rnc_ulen(void *packed)
{
    unsigned char *p = packed;
    if (read_int32_be_buf(p) != RNC_SIGNATURE_INT)
    return RNC_FILE_IS_NOT_RNC;
    return read_int32_be_buf(p+4);
}

/**
 * Return the compressed length of a packed data block.
 * The packed buffer must be longer than SIZEOF_RNC_HEADER.
 * @param packed Packed buffer.
 * @return Returns compressed length of the packed block,
 *     or a negative error code.
 */
long rnc_plen(void *packed)
{
    unsigned char *p = packed;
    if (read_int32_be_buf(p) != RNC_SIGNATURE_INT)
    return RNC_FILE_IS_NOT_RNC;
    return read_int32_be_buf(p+8);
}

/**
 * Decompress a packed data block.
 *
 * @param packed Packed source data buffer.
 * @param unpacked Unpacked destination data buffer.
 * @param flags Option flags for the decompressor.
 * @return Returns the unpacked length if successful,
 *    or negative error code if not.
 * If COMPRESSOR is defined, it also returns the leeway number
 * (which gets stored at offset 16 into the compressed-file header)
 * in `*leeway', if `leeway' isn't NULL.
 *
 */
long rnc_unpack (const void *packed, void *unpacked, const unsigned int flags
#ifdef COMPRESSOR
         , long *leeway
#endif
         )
{
    const unsigned char *input = (const unsigned char *)packed;
    unsigned char *output = (unsigned char *)unpacked;
    const unsigned char *inputend;
    unsigned char *outputend;
    bit_stream bs;
    huf_table raw, dist, len;
    unsigned long ch_count;
    unsigned long ret_len, inp_len;
    unsigned out_crc;
#ifdef COMPRESSOR
    long lee = 0;
#endif
    // Reading header
    if (read_int32_be_buf(input) != RNC_SIGNATURE_INT)
        if (!(flags&RNC_IGNORE_HEADER_VAL_ERROR)) return RNC_HEADER_VAL_ERROR;
    ret_len = read_int32_be_buf(input+4);
    inp_len = read_int32_be_buf(input+8);
    if ((ret_len>(RNC_MAX_FILESIZE))||(inp_len>(RNC_MAX_FILESIZE)))
        return RNC_HEADER_VAL_ERROR;
    //Setting variables
    outputend = output + ret_len;
    inputend = input + SIZEOF_RNC_HEADER + inp_len;
    input += SIZEOF_RNC_HEADER;               // skip header

    // Check the packed-data CRC. Also save the unpacked-data CRC
    // for later.
    if (rnc_crc(input, inputend-input) != read_int16_be_buf(input-4))
        if (!(flags&RNC_IGNORE_PACKED_CRC_ERROR)) return RNC_PACKED_CRC_ERROR;
    out_crc = read_int16_be_buf(input-6);

    bitread_init(&bs, &input, inputend);
    bit_advance(&bs, 2, &input, inputend);      // discard first two bits

   // Process chunks.

    while (output < outputend)
    {
      DEBUGF("while (output < outputend) repeats");
#ifdef COMPRESSOR
      long this_lee;
#endif
      if (inputend-input<6)
      {
          DEBUGF("too small inputend-input=%d",inputend-input);
          if (!(flags&RNC_IGNORE_HUF_EXCEEDS_RANGE))
              return RNC_HUF_EXCEEDS_RANGE;
            else
              {output=outputend;ch_count=0;break;}
      }
      read_huftable (&raw,  &bs, &input, inputend);
      read_huftable (&dist, &bs, &input, inputend);
      read_huftable (&len,  &bs, &input, inputend);
      ch_count = bit_read (&bs, 0xFFFF, 16, &input, inputend);

      while (1)
      {
          DEBUGF("repeating while");
          long length, posn;

          length = huf_read(&raw, &bs, &input, inputend);
          DEBUGF("huf_read length=%ld",length);
          if (length < 0)
          {
            if (!(flags&RNC_IGNORE_HUF_DECODE_ERROR))
            {
                return RNC_HUF_DECODE_ERROR;
            } else
            {
                output=outputend;
                ch_count=0;
                break;
            }
          }
          if (length)
          {
            while (length--)
            {
                if ((input>=inputend) || (output>=outputend))
                {
                    DEBUGF("out of buffers, output-outputend=%d input-inputend=%d length=%ld",
                      (void *)output-(void *)outputend,(void *)input-(void *)inputend,length);
                    if (!(flags&RNC_IGNORE_HUF_EXCEEDS_RANGE))
                        return RNC_HUF_EXCEEDS_RANGE;
                    else
                        {output=outputend;ch_count=0;break;}
                }
                *output++ = *input++;
            }
            bitread_fix(&bs, &input, inputend);
          }
          if (--ch_count <= 0)
            break;

          posn = huf_read(&dist, &bs, &input, inputend);
          DEBUGF("huf_read done, posn=%ld",posn);
          if (posn == -1)
          {
            if (!(flags&RNC_IGNORE_HUF_DECODE_ERROR))
                return RNC_HUF_DECODE_ERROR;
            else
                {output=outputend;ch_count=0;break;}
          }
          length = huf_read (&len, &bs, &input, inputend);
          DEBUGF("huf_read done, length=%ld",length);
          if (length < 0)
          {
            if (!(flags&RNC_IGNORE_HUF_DECODE_ERROR))
                return RNC_HUF_DECODE_ERROR;
            else
                {output=outputend;ch_count=0;break;}
          }
          posn += 1;
          length += 2;
          while (length--)
          {
            DEBUGF("while (length--) repeats, length=%ld",length);
            if ((((void *)output-posn)<unpacked)||((output-posn)>outputend)||
                (((void *)output)<unpacked)||((output)>outputend))
            {
                DEBUGF("out of buffers, output-outputend=%d input-inputend=%d length=%ld",
                    (void *)output-(void *)outputend,(void *)input-(void *)inputend,length);
                if (!(flags&RNC_IGNORE_HUF_EXCEEDS_RANGE))
                    return RNC_HUF_EXCEEDS_RANGE;
                else
                    {output=outputend;ch_count=0;break;}
            }
            output[0] = output[-posn];
            output++;
          }
#ifdef COMPRESSOR
          this_lee = (inputend - input) - (outputend - output);
          if (lee < this_lee)
              lee = this_lee;
#endif
      }
    }

    DEBUGF("while (output < outputend) ends");

    if (outputend != output)
    {
      if (!(flags&RNC_IGNORE_FILE_SIZE_MISMATCH))
          return RNC_FILE_SIZE_MISMATCH;
    }

#ifdef COMPRESSOR
    if (leeway)
      *leeway = lee;
#endif

    // Check the unpacked-data CRC.
    if (rnc_crc(outputend-ret_len, ret_len) != out_crc)
    {
      DEBUGF("rnc_crc failed");
      if (!(flags&RNC_IGNORE_UNPACKED_CRC_ERROR))
          return RNC_UNPACKED_CRC_ERROR;
    }
    DEBUGF("rnc_crc ok");
    return ret_len;
}

/**
 * Read a Huffman table out of the bit stream and data stream given.
 */
static void read_huftable (huf_table *h, bit_stream *bs,
                          const unsigned char **p, const unsigned char *pend)
{
    int i, j, k, num;
    int leaflen[32];
    int leafmax;
    unsigned long codeb;           // big-endian form of code

    num = bit_read (bs, 0x1F, 5, p, pend);
    if (!num)
        return;

    leafmax = 1;
    for (i=0; i<num; i++)
    {
        leaflen[i] = bit_read (bs, 0x0F, 4, p, pend);
        if (leafmax < leaflen[i])
            leafmax = leaflen[i];
    }

    codeb = 0L;
    k = 0;
    for (i=1; i<=leafmax; i++)
    {
    for (j=0; j<num; j++)
        if (leaflen[j] == i)
        {
            h->table[k].code = mirror (codeb, i);
            h->table[k].codelen = i;
            h->table[k].value = j;
            codeb++;
            k++;
        }
    codeb <<= 1;
    }

    h->num = k;
}

/**
 * Read a value out of the bit stream using the given Huffman table.
 */
static unsigned long huf_read (huf_table *h, bit_stream *bs,
                   const unsigned char **p,const unsigned char *pend)
{
    int i;
    unsigned long val;

    for (i=0; i<h->num; i++)
    {
        unsigned long mask = (1 << h->table[i].codelen) - 1;
        if (bit_peek(bs, mask) == h->table[i].code)
            break;
    }
    if (i == h->num)
        return -1;
    bit_advance (bs, h->table[i].codelen, p, pend);

    val = h->table[i].value;

    if (val >= 2)
    {
        val = 1 << (val-1);
        val |= bit_read (bs, val-1, h->table[i].value - 1, p, pend);
    }
    return val;
}

/**
 * Initialises a bit stream with the first two bytes of the packed data.
 * Checks pend for proper buffer pointers range. The pend should point
 * to the last readable byte in buffer.
 * If buffer is exceeded, fills output (or part of output) with zeros.
 */
inline void bitread_init (bit_stream *bs, const unsigned char **p, const unsigned char *pend)
{
    if (pend-(*p) >= 1)
    {
        bs->bitbuf = (unsigned long)read_int16_le_buf(*p);
        bs->bitcount = 16;
    } else
    if (pend-(*p) >= 0)
    {
        bs->bitbuf = (unsigned long)read_int8_buf(*p);
        bs->bitcount = 8;
    } else
        bs->bitbuf = 0;
}

/**
 * Fixes up a bit stream after literals have been read out of the
 * data stream.
 * Checks pend for proper buffer pointers range. The pend should point
 * to the last readable byte in buffer.
 */
inline void bitread_fix (bit_stream *bs, const unsigned char **p, const unsigned char *pend)
{
    bs->bitcount -= 16;
    if (bs->bitcount<0) bs->bitcount=0;
    bs->bitbuf &= (1<<bs->bitcount)-1; // remove the top 16 bits
    // replace with what's at *p, or zeroes if nothing more to read
    if (pend-(*p) >= 1)
    {
        bs->bitbuf |= (read_int16_le_buf(*p)<<bs->bitcount);
        bs->bitcount += 16;
    } else
    if (pend-(*p) >= 0)
    {
        bs->bitbuf |= (read_int8_buf(*p)<<bs->bitcount);
        bs->bitcount += 8;
    }
}

/**
 * Returns some bits, masked with given bit mask.
 */
inline unsigned long bit_peek (bit_stream *bs, const unsigned long mask)
{
    return bs->bitbuf & mask;
}

/**
 * Advances the bit stream. Reads n bits from the bit_stream, then makes sure
 * there are still at least 16 bits to read in next operation.
 * The new bits are taken from buffer *p.
 * Checks pend for proper buffer pointers range. The pend should point
 * to the last readable byte in buffer.
 */
inline void bit_advance (bit_stream *bs, int n,
    const unsigned char **p, const unsigned char *pend)
{
    bs->bitbuf >>= n;
    bs->bitcount -= n;
    if (bs->bitcount < 16)
    {
        (*p) += 2;
        if (pend-(*p) >= 1)
        {
            bs->bitbuf |= ((unsigned long)read_int16_le_buf(*p)<<bs->bitcount);
            bs->bitcount += 16;
        } else
        if (pend-(*p) >= 0)
        {
            bs->bitbuf |= ((unsigned long)read_int8_buf(*p)<<bs->bitcount);
            bs->bitcount += 8;
        } else
        if (bs->bitcount < 0)
            bs->bitcount = 0;
    }
}

/*
 * Reads some bits and advances the bit stream.
 * Works like the bit_peek and bit_advance routines combined.
 */
static unsigned long bit_read (bit_stream *bs, unsigned long mask,
                   int n, const unsigned char **p, const unsigned char *pend)
{
    unsigned long result = bit_peek (bs, mask);
    bit_advance (bs, n, p, pend);
    return result;
}

/**
 * Mirror the bottom n bits of x.
 * @param n Amount of bits to mirror.
 * @param x The data to mirror.
 * @return The value of x with proper bits mirrored.
 */
static unsigned long mirror (unsigned long x, int n)
{
    unsigned long top = 1 << (n-1), bottom = 1;
    while (top > bottom)
    {
        unsigned long mask = top | bottom;
        unsigned long masked = x & mask;
        if (masked != 0 && masked != mask)
            x ^= mask;
        top >>= 1;
        bottom <<= 1;
    }
    return x;
}

/**
 * Calculate a CRC, the RNC way.
 * CRC is a 16-bit value that allows to verify data in a buffer.
 * @param data The data buffer.
 * @param len data Length of the data buffer.
 * @return The CRC value for given data.
 */
long rnc_crc(const void *data, unsigned long len)
{
    static unsigned short crctab[256];
    static short crctab_ready=0;
    unsigned short val;
    int i, j;
    const unsigned char *p = data;
    //computing CRC table
    if (!crctab_ready)
    {
      for (i=0; i<256; i++)
      {
        val = i;

        for (j=0; j<8; j++)
        {
          if (val & 1)
               val = (val >> 1) ^ 0xA001;
          else
            val = (val >> 1);
        }
        crctab[i] = val;
      }
      crctab_ready=1;
    }

    // Calculating CRC of the given buffer
    val = 0;
    while (len--)
    {
        val ^= *p++;
        val = (val >> 8) ^ crctab[val & 0xFF];
    }

    return val;
}
