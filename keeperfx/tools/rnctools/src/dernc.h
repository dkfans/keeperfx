/******************************************************************************/
/** @file dernc.h
 * RNC decompression support.
 * @par Purpose:
 *     Header file. Defines exported routines from dernc.c.
 * @par Comment:
 *     None.
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

#ifndef RNC_DERNC_H
#define RNC_DERNC_H

typedef long (*rnc_callback)(long done,long total);

/*
 * Routines
 */
long rnc_plen (void *packed);
long rnc_ulen (void *packed);
#ifndef COMPRESSOR
long rnc_unpack (const void *packed, void *unpacked, const unsigned int flags);
#else
long rnc_unpack (const void *packed, void *unpacked, const unsigned int flags, long *leeway);
#endif

char *rnc_error (long errcode);
long rnc_crc (const void *data, unsigned long len);

/*
 * Error returns.
 * RNC support reserves error codes -1 to -16.
 */
#define RNC_FILE_IS_NOT_RNC    -1
#define RNC_HUF_DECODE_ERROR   -2
#define RNC_FILE_SIZE_MISMATCH -3
#define RNC_PACKED_CRC_ERROR   -4
#define RNC_UNPACKED_CRC_ERROR -5
#define RNC_HEADER_VAL_ERROR   -6
#define RNC_HUF_EXCEEDS_RANGE  -7

/**
 * Flags to ignore errors.
 */
enum EXTRA_OBJ_LOAD {
    RNC_IGNORE_NONE               = 0x0000,
    RNC_IGNORE_FILE_IS_NOT_RNC    = 0x0001,
    RNC_IGNORE_HUF_DECODE_ERROR   = 0x0002,
    RNC_IGNORE_FILE_SIZE_MISMATCH = 0x0004,
    RNC_IGNORE_PACKED_CRC_ERROR   = 0x0008,
    RNC_IGNORE_UNPACKED_CRC_ERROR = 0x0010,
    RNC_IGNORE_HEADER_VAL_ERROR   = 0x0020,
    RNC_IGNORE_HUF_EXCEEDS_RANGE  = 0x0040,
    };

#ifdef INTERNAL
/**
 * "RNC\001" as int. The compressor needs this define.
 */
#define RNC_SIGNATURE_INT 0x524E4301
#endif

/**
 * Limit of the compressed & decompressed file sizes.
 */
#define RNC_MAX_FILESIZE 1<<30

/**
 * Length of the RNC file header, in bytes.
 */
#define SIZEOF_RNC_HEADER  18

#endif
