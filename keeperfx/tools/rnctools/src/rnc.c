/******************************************************************************/
/** @file rnc.c
 * ProPak/RNC compression support.
 * @par Purpose:
 *   Compiled normally, this file is a well-behaved, re-entrant code
 *   module exporting only the compression routines.
 *   Compiled with MAIN_RNC defined, it's a standalone program which will
 *   compress argv[1] into argv[2].
 * @par Comment:
 *   Requires dernc.c to compute the leeway header field.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPRESSOR
#define INTERNAL
#include "lblogging.h"
#include "lbfileio.h"
#include "dernc.h"
#include "lbfileio.h"

typedef struct rnc_packdata_s rnc_packdata;
typedef struct huf_table_s huf_table;
typedef struct tuple_s tuple;

static void *rnc_pack (void *data, long datalen, long *packlen, rnc_callback callback);
static void do_block (rnc_packdata *pd);
static void write_block (rnc_packdata *pd);
static void build_huf (huf_table *h, int *freqs);
static void write_huf (rnc_packdata *pd, huf_table *h);
static void write_hval (rnc_packdata *pd, huf_table *h, unsigned long value);
static void emit_raw (rnc_packdata *pd, int n);
static void emit_pair (rnc_packdata *pd, int pos, int len);
static int write_bits (rnc_packdata *pd, unsigned long value, int nbits);
static int write_literal (rnc_packdata *pd, unsigned long value);
inline int check_size (rnc_packdata *pd);
inline int length (unsigned long value);
static unsigned long mirror (unsigned long x, int n);
int main_pack (char *pname, char *iname, char *oname, char verbose);

int show_usage(char *fname)
{
    printf("usage:\n");
    printf("    %s <files>\n", fname);
    printf(" or\n");
    printf("    %s -o <infile> <outfile>\n", fname);
    return 0;
}

#ifdef MAIN_RNC

int main(int argc, char **argv)
{
    int mode;
    int i;

    printf("\nPRO-PACK's alternate RNC files compressor\n");
    printf("-------------------------------\n");

    if (argc<=1)
    {
      show_usage(*argv);
      return 0;
    }
    mode=0;
    for (i=1; i < argc; i++)
    {
      if (!strcmp (argv[1], "-o"))
        mode=i;
    }
    if ( (mode>0) && (argc!=4) )
    {
      show_usage(*argv);
      return 1;
    }
    switch (mode)
    {
    case 0 :
        for (i=1; i < argc; i++)
        {
          printf("Compressing %s...\n",argv[i]);
          if (main_pack(*argv, argv[i], argv[i],1))
            return 1;
        }
        return 0;
    case 1 :
        printf("Compressing %s to %s...\n",argv[2],argv[3]);
        return main_pack(*argv, argv[2], argv[3],1);
    case 2 :
        printf("Compressing %s to %s...\n",argv[1],argv[3]);
        return main_pack(*argv, argv[1], argv[3],1);
    case 3 :
        printf("Compressing %s to %s...\n",argv[1],argv[2]);
        return main_pack(*argv, argv[1], argv[2],1);
    default :
      ERRORF("Internal fault.");
    }
    return 1;
}

#endif

/**
 * Compresses single file if building stand-alone DeRNC tool.
 * @param pname Name of the program executable.
 * @param iname File name of the input file.
 * @param oname File name of the compressed output.
 * @return Returns 0 on success. On error prints a message
 *     (depends on macro definition) and returns nonzero value.
 */
int main_pack (char *pname, char *iname, char *oname, char verbose)
{
    FILE *ifp, *ofp;
    long ulen, ulen2, plen;
    void *packed, *unpacked, *unpacked2;
    long leeway;

    if (verbose)
      printf("  Reading input file \"%s\"\n",iname);
    // Reading unpacked file
    ifp = fopen(iname, "rb");
    if (ifp == NULL)
    {
      ERRORF("%s: %s",iname,strerror(errno));
      return 1;
    }
    fseek (ifp, 0L, SEEK_END);
    ulen = ftell (ifp);
    rewind (ifp);
    unpacked = malloc(ulen);
    if (unpacked == NULL)
    {
      ERRORF("%s: %s",pname,strerror(errno));
      return 1;
    }
    fread (unpacked, 1, ulen, ifp);
    fclose (ifp);

    if (verbose)
      printf("  Compressing...\n");
    // Packing
    packed = rnc_pack(unpacked, ulen, &plen, rnc_printcallback);
      printf("\n");
    if (packed == NULL)
    {
      ERRORF("Error in compression.");
      return 1;
    }

    if (verbose)
      printf("  Testing...\n");
    unpacked2 = malloc(ulen);
    if (unpacked2 == NULL)
    {
      ERRORF("%s: %s",pname,strerror(errno));
      return 1;
    }

    ulen2 = rnc_unpack (packed, unpacked2, 0, &leeway);
    if (ulen2 < 0)
    {
      ERRORF("Test unpack: %s.", rnc_error(ulen2));
      return 1;
    }

    if (ulen2 != ulen)
    {
      ERRORF("Test unpack: lengths do not match.");
      return 1;
    }
    if (memcmp (unpacked, unpacked2, ulen))
    {
      ERRORF("Test unpack: files do not match.");
      return 1;
    }

    if (leeway > 255)
    {
      ERRORF("Unable to handle leeway > 255.");
      return 1;
    }
    ((unsigned char *)packed)[16] = leeway;

    if (verbose)
      printf("  Writing output file \"%s\"\n",oname);
    ofp = fopen(oname, "wb");
    if (ofp == NULL)
    {
      ERRORF("%s: %s",oname,strerror(errno));
      return 1;
    }

    fwrite (packed, 1, plen, ofp);
    fclose (ofp);

    free (unpacked2);
    free (packed);
    free (unpacked);

    return 0;
}

#define BLOCKMAX 8192
#define WINMAX 32767
#define MAXTUPLES 4096
#define HASHMAX 509               // it's prime
// Packed data buffer - minimum size
#define PACKED_MIN_DELTA 16
#define PACKED_DELTA 4096
#define TUPLE_MAX_LENGTH 32
#define HUF_TABLE_LENGTH 32

struct huf_table_s {
    struct {
    unsigned long code;
    int codelen;
    } table[HUF_TABLE_LENGTH];
};

struct tuple_s {
    unsigned long rawlen;
    int pos;
    int len;
};

struct rnc_packdata_s {
    unsigned char blk[WINMAX];
    int linkp[WINMAX];
    int hashp[HASHMAX];
    int blkstart, bpos;
    int blklen;

    tuple tuples[MAXTUPLES];
    int ntuple;

    unsigned char *packed;
    long packedlen;
    long packedpos, bitpos, bitcount, bitbuf;
};

inline int hash (unsigned char *a) {
    return ((a[0] * 7 + a[1]) * 7 + a[2]) % HASHMAX;
}

static void *rnc_pack(void *original, long datalen, long *packlen, rnc_callback callback)
{
    int i;
    unsigned char *data = original;
    long origlen = datalen;
    rnc_packdata *pd;
    pd = malloc(sizeof(rnc_packdata));
    if (pd == NULL)
    {
      ERRORF("compression struct allocation: %s",strerror(errno));
      return NULL;
    }
    memset(pd,0,sizeof(rnc_packdata));
    pd->packed = malloc(PACKED_DELTA);
    if (pd->packed == NULL)
    {
      ERRORF("packed buffer allocation: %s",strerror(errno));
      free(pd);
      return NULL;
    }
    callback(0,origlen);
    pd->packedlen = PACKED_DELTA;
    pd->packedpos = SIZEOF_RNC_HEADER+2;
    write_int32_be_buf(pd->packed+4, datalen);

    pd->bitpos = SIZEOF_RNC_HEADER;
    pd->bitcount = 0;
    pd->bitbuf = 0;
    write_bits(pd, 0, 2);

    while (datalen > 0)
    {
      long currpos=origlen-datalen;
      pd->blklen = (datalen>BLOCKMAX) ? BLOCKMAX : datalen;
      pd->blkstart = WINMAX - BLOCKMAX;
      if (pd->blkstart > currpos)
          pd->blkstart = currpos;
      memcpy (pd->blk, data-pd->blkstart, pd->blkstart+pd->blklen);
      for (i=0; i<HASHMAX; i++)
          pd->hashp[i] = -1;
      pd->ntuple = 0;
      pd->tuples[pd->ntuple].rawlen = 0;
      pd->blklen += pd->blkstart;
      do_block(pd);
      data += pd->bpos - pd->blkstart;
      datalen -= pd->bpos - pd->blkstart;
      write_block(pd);
      callback(currpos,origlen);
    }

    if (pd->bitcount > 0)
    {
      DEBUGF("Flushing extra %ld bits",17-pd->bitcount);
      write_bits(pd, 0, 17-pd->bitcount); // force flush
      pd->packedpos -= 2;            // write_bits will have moved it on
    }

    *packlen = pd->packedpos;

    write_int32_be_buf(pd->packed, RNC_SIGNATURE_INT);
    write_int32_be_buf(pd->packed+12, rnc_crc(pd->packed+SIZEOF_RNC_HEADER, pd->packedpos-SIZEOF_RNC_HEADER));
    write_int32_be_buf(pd->packed+10, rnc_crc(original, origlen));
    write_int32_be_buf(pd->packed+8, pd->packedpos-SIZEOF_RNC_HEADER);
    pd->packed[16] = pd->packed[17] = 0;
    callback(origlen,origlen);
    data = pd->packed;
    free(pd);
    return data;
}

static void do_block (rnc_packdata *pd)
{
    int lazylen = 0, lazypos = 0, lazyraw = 0;
    int hashv, ph, h;

    pd->bpos = 0;
    while (pd->bpos < pd->blkstart)
    {
      hashv = hash(pd->blk+pd->bpos);
      h = pd->hashp[hashv];
      ph = -1;
      while (h != -1)
      {
        ph = h;
        h = pd->linkp[h];
      }
      if (ph != -1)
        pd->linkp[ph] = pd->bpos;
      else
        pd->hashp[hashv] = pd->bpos;
      pd->linkp[pd->bpos] = -1;
      pd->bpos++;
    }

    while ((pd->bpos < pd->blklen) && (pd->ntuple < MAXTUPLES-1))
    {
      if (pd->blklen - pd->bpos < 3)
      {
        emit_raw(pd, pd->blklen - pd->bpos);
      } else
      {
        int len, maxlen, maxlenpos;
        int savebpos;

        hashv = hash(pd->blk+pd->bpos);
        h = pd->hashp[hashv];

        maxlen = 0;
        maxlenpos = ph = -1;

        while (h != -1)
        {
          unsigned char *p = pd->blk+pd->bpos;
          unsigned char *v = pd->blk+h;
          len = 0;
          while (*p == *v && p < pd->blk+pd->blklen)
              p++, v++, len++;
          if (maxlen < len)
          {
              maxlen = len;
              maxlenpos = h;
          }
          ph = h;
          h = pd->linkp[h];
        }

        if (ph != -1)
            pd->linkp[ph] = pd->bpos;
        else
            pd->hashp[hashv] = pd->bpos;

        pd->linkp[pd->bpos] = -1;
        savebpos = pd->bpos;

        pd->bpos -= lazyraw;
        if (lazyraw)
        {
          if (maxlen >= lazylen+2)
          {
            emit_raw(pd, lazyraw);
            lazyraw = 1;
            lazypos = maxlenpos;
            lazylen = maxlen;
          } else
          {
            emit_pair(pd, lazypos, lazylen);
            lazyraw = lazypos = lazylen = 0;
          }
        } else
        if (maxlen >= 3)
        {
          lazyraw = 1;
          lazypos = maxlenpos;
          lazylen = maxlen;
        } else
        {
          emit_raw (pd, 1);
        }
        pd->bpos += lazyraw;

        while (++savebpos < pd->bpos)
        {
          hashv = hash(pd->blk+savebpos);
          h = pd->hashp[hashv];
          ph = -1;
          while (h != -1)
          {
              ph = h;
              h = pd->linkp[h];
          }
          if (ph != -1)
              pd->linkp[ph] = savebpos;
          else
              pd->hashp[hashv] = savebpos;
          pd->linkp[savebpos] = -1;
        }
      }
    }
    if (lazyraw)
    {
      pd->bpos -= lazyraw;
      emit_raw(pd, lazyraw);
    }
}

static void write_block (rnc_packdata *pd)
{
    int lengths[TUPLE_MAX_LENGTH];
    huf_table raw, dist, len;
    int i, j, k;

    for (i=0; i<TUPLE_MAX_LENGTH; i++)
      lengths[i] = 0;
    for (i=0; i<=pd->ntuple; i++)
      lengths[length(pd->tuples[i].rawlen)]++;
    build_huf (&raw, lengths);
    write_huf (pd, &raw);

    for (i=0; i<TUPLE_MAX_LENGTH; i++)
      lengths[i] = 0;
    for (i=0; i<pd->ntuple; i++)
      lengths[length(pd->tuples[i].pos-1)]++;
    build_huf (&dist, lengths);
    write_huf (pd, &dist);

    for (i=0; i<TUPLE_MAX_LENGTH; i++)
      lengths[i] = 0;
    for (i=0; i<pd->ntuple; i++)
      lengths[length(pd->tuples[i].len-2)]++;
    build_huf (&len, lengths);
    write_huf (pd, &len);

    write_bits(pd, pd->ntuple+1, 16);

    k = pd->blkstart;
    for (i=0; i<=pd->ntuple; i++)
    {
      write_hval(pd, &raw, pd->tuples[i].rawlen);
      for (j=0; j<pd->tuples[i].rawlen; j++)
        write_literal(pd, pd->blk[k++]);
      if (i == pd->ntuple)
        break;
      write_hval (pd, &dist, pd->tuples[i].pos-1);
      write_hval (pd, &len, pd->tuples[i].len-2);
      k += pd->tuples[i].len;
    }
}

static void build_huf (huf_table *h, int *freqs)
{
    struct hnode {
    int freq, parent, lchild, rchild;
    } pool[64];
    int i, j, k, m, toobig;
    int maxcodelen;
    unsigned long codeb;

    j = 0;                   // j counts nodes in the pool
    toobig = 1;
    for (i=0; i<32; i++)
    {
      if (freqs[i])
      {
        pool[j].freq = freqs[i];
        pool[j].parent = -1;
        pool[j].lchild = -1;
        pool[j].rchild = i;
        j++;
        toobig += freqs[i];
      }
    }

    k = j;                   // k counts _free_ nodes in the pool
    while (k > 1)
    {
      int min = toobig;
      int nextmin = toobig+1;
      int minpos = 0, nextminpos = 0;
      for (i=0; i<j; i++)           // loop through free nodes
        if (pool[i].parent == -1)
        {
          if (min > pool[i].freq) {
            nextmin = min;
            nextminpos = minpos;
            min = pool[i].freq;
            minpos = i;
          } else
          if (nextmin > pool[i].freq)
          {
            nextmin = pool[i].freq;
            nextminpos = i;
          }
        }
      pool[j].freq = min + nextmin;
      pool[j].parent = -1;
      pool[j].lchild = minpos;
      pool[j].rchild = nextminpos;
      pool[minpos].parent = j;
      pool[nextminpos].parent = j;
      j++;
      k--;
    }

    for (i=0; i<HUF_TABLE_LENGTH; i++)
      h->table[i].codelen = 0;

    maxcodelen = 0;
    for (i=0; i<j; i++)               // loop through original nodes
      if (pool[i].lchild == -1)
      {
        m = 0;
        k = i;
        while (pool[k].parent != -1)
          m++, k = pool[k].parent;
        h->table[pool[i].rchild].codelen = m;
        if (maxcodelen < m)
          maxcodelen = m;
      }

    codeb = 0;
    for (i=1; i<=maxcodelen; i++)
    {
      for (j=0; j<HUF_TABLE_LENGTH; j++)
        if (h->table[j].codelen == i)
        {
          h->table[j].code = mirror(codeb, i);
          codeb++;
        }
      codeb <<= 1;
    }
}

static void write_huf (rnc_packdata *pd, huf_table *h)
{
    int i, j;

    j = 0;
    for (i=0; i<HUF_TABLE_LENGTH; i++)
    {
      if (h->table[i].codelen > 0)
        j = i+1;
    }

    write_bits(pd, j, 5);

    for (i=0; i<j; i++)
      write_bits(pd, h->table[i].codelen, 4);
}

static void write_hval (rnc_packdata *pd, huf_table *h, unsigned long value)
{
    int len = length(value);

    write_bits(pd, h->table[len].code, h->table[len].codelen);
    if (len >= 2)
      write_bits(pd, value, len-1);
}

static void emit_raw (rnc_packdata *pd, int n)
{
    pd->tuples[pd->ntuple].rawlen += n;
    pd->bpos += n;
}

static void emit_pair (rnc_packdata *pd, int pos, int len)
{
    pd->tuples[pd->ntuple].pos = pd->bpos - pos;
    pd->tuples[pd->ntuple].len = len;
    pd->tuples[++pd->ntuple].rawlen = 0;
    pd->bpos += len;
}

static int write_bits (rnc_packdata *pd, unsigned long value, int nbits)
{
    value &= (1 << nbits)-1;
    pd->bitbuf |= (value << pd->bitcount);
    pd->bitcount += nbits;

    if (pd->bitcount > 16)
    {
      pd->packed[pd->bitpos] = pd->bitbuf;
      pd->bitbuf >>= 8;
      pd->packed[pd->bitpos+1] = pd->bitbuf;
      pd->bitbuf >>= 8;
      pd->bitcount -= 16;
      pd->bitpos = pd->packedpos;
      pd->packedpos += 2;
      if (check_size(pd))
        return 1;
    }
    return 0;
}

/**
 * Writes a data value into packed buffer.
 */
static int write_literal (rnc_packdata *pd, unsigned long value)
{
    pd->packed[pd->packedpos++] = value;
    return check_size(pd);
}

/**
 * Checks size of packed buffer, and reallocates memory for it if required.
 */
inline int check_size (rnc_packdata *pd)
{
    if (pd->packedpos+PACKED_MIN_DELTA <= pd->packedlen)
      return 0;
    if (pd->packedpos > pd->packedlen)
      DEBUGF("Buffer size exceeded before packed grow!");

    //long pos = pd->packedlen;
    pd->packedlen += PACKED_DELTA;
    pd->packed = realloc(pd->packed, pd->packedlen);
    if (pd->packed == NULL)
    {
      ERRORF("%s: %s","realloc",strerror(errno));
      return 1;
    }
    /*while(pos < pd->packedlen)
    {
        pd->packed[pos]='\0';
        pos++;
    }*/
    return 0;
}

/**
 * Returns length of given tuple.
 * The length may never exceed sizeof(long)=32.
 */
inline int length (unsigned long value)
{
    int ret = 0;
    while (value != 0)
    {
      value >>= 1;
      ret++;
    }
    return ret;
}

/**
 * Mirror the bottom n bits of x.
 */
static unsigned long mirror (unsigned long x, int n)
{
    unsigned long top = 1 << (n-1), bottom = 1;
    while (top > bottom)
    {
        unsigned long mask = top | bottom;
        unsigned long masked = x & mask;
        if ((masked != 0) && (masked != mask))
            x ^= mask;
        top >>= 1;
        bottom <<= 1;
    }
    return x;
}
