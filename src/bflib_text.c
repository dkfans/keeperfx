/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_text.c
 *     Text routines.
 * @par Purpose:
 *     Text routines.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_text.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#else
#include <iconv.h>
#endif

#include <stdlib.h>
#include <string.h>
#include "bflib_basics.h"
#include "bflib_sprfnt.h"
#include "post_inc.h"

typedef struct DbcMapEntry
{
    unsigned long unicode;
    unsigned short code;
} DbcMapEntry;

/******************************************************************************/

static const char *get_dbc_encoding(void)
{
    switch (dbc_language)
    {
    case DbcId_Japanese:
        return "SHIFT_JIS";
    case DbcId_ChineseInt:
    case DbcId_ChineseTra:
        return "GBK";
    case DbcId_Korean:
        return "EUC-KR";
    default:
        return NULL;
    }
}

size_t convert_utf8_to_codepage_string(const char *src, char *dst, size_t dst_size, const char *encoding)
{
    if ((src == NULL) || (dst == NULL) || (dst_size == 0) || (encoding == NULL))
        return 0;

#if defined(_WIN32) || defined(__CYGWIN__)
    UINT codepage = 0;
    if (strcmp(encoding, "SHIFT_JIS") == 0 || strcmp(encoding, "CP932") == 0)
        codepage = 932;
    else if (strcmp(encoding, "GBK") == 0 || strcmp(encoding, "CP936") == 0)
        codepage = 936;
    else if (strcmp(encoding, "EUC-KR") == 0)
        codepage = 949;
    else
        return 0;

    int wlen = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (wlen <= 0)
        return 0;

    WCHAR *wbuf = (WCHAR *)malloc((size_t)wlen * sizeof(WCHAR));
    if (wbuf == NULL)
        return 0;

    if (MultiByteToWideChar(CP_UTF8, 0, src, -1, wbuf, wlen) == 0)
    {
        free(wbuf);
        return 0;
    }

    int mb_len = WideCharToMultiByte(codepage, 0, wbuf, wlen, dst, (int)dst_size, NULL, NULL);
    free(wbuf);
    if (mb_len <= 0)
        return 0;

    if ((size_t)mb_len >= dst_size)
    {
        dst[dst_size - 1] = '\0';
        return dst_size - 1;
    }
    dst[mb_len] = '\0';
    return (size_t)mb_len;
#else
    iconv_t cd = iconv_open(encoding, "UTF-8");
    if (cd == (iconv_t)(-1))
    {
        if (strcmp(encoding, "SHIFT_JIS") == 0)
            cd = iconv_open("CP932", "UTF-8");
        else if (strcmp(encoding, "GBK") == 0)
            cd = iconv_open("CP936", "UTF-8");
        if (cd == (iconv_t)(-1))
            return 0;
    }

    char *inbuf = (char *)src;
    size_t inbytes = strlen(src);
    char *outbuf = dst;
    size_t outbytes = dst_size;

    size_t res = iconv(cd, &inbuf, &inbytes, &outbuf, &outbytes);
    iconv_close(cd);
    if (res == (size_t)(-1))
    {
        if (dst_size > 0)
            dst[0] = '\0';
        return 0;
    }

    size_t written = dst_size - outbytes;
    if (written == 0)
    {
        if (dst_size > 0)
            dst[0] = '\0';
        return 0;
    }
    if (written >= dst_size)
        written = dst_size - 1;
    dst[written] = '\0';
    return written;
#endif
}

// TODO: remove legacy internal codepage support once all text is UTF-8.

void codepoint_to_utf8(unsigned long codepoint, char out[5])
{
    if (codepoint <= 0x7F)
    {
        out[0] = (char)codepoint;
        out[1] = '\0';
    }
    else if (codepoint <= 0x7FF)
    {
        out[0] = (char)(0xC0 | ((codepoint >> 6) & 0x1F));
        out[1] = (char)(0x80 | (codepoint & 0x3F));
        out[2] = '\0';
    }
    else if (codepoint <= 0xFFFF)
    {
        out[0] = (char)(0xE0 | ((codepoint >> 12) & 0x0F));
        out[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[2] = (char)(0x80 | (codepoint & 0x3F));
        out[3] = '\0';
    }
    else if (codepoint <= 0x10FFFF)
    {
        out[0] = (char)(0xF0 | ((codepoint >> 18) & 0x07));
        out[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        out[3] = (char)(0x80 | (codepoint & 0x3F));
        out[4] = '\0';
    }
    else
    {
        out[0] = '?';
        out[1] = '\0';
    }
}

static unsigned char convert_codepoint_to_internal_byte(unsigned long codepoint)
{
    if (codepoint < 0x80)
        return (unsigned char)codepoint;

    static const struct
    {
        unsigned long unicode;
        unsigned char byte;
    } codepage_map[] = {
        {0x0410, 0x41},
        {0x0411, 0xB1},
        {0x0412, 0x42},
        {0x0413, 0xB2},
        {0x0414, 0xB3},
        {0x0415, 0x45},
        {0x0416, 0xB4},
        {0x0417, 0xC2},
        {0x0418, 0xC3},
        {0x0419, 0xC4},
        {0x041A, 0x4B},
        {0x041B, 0x7F},
        {0x041C, 0x4D},
        {0x041D, 0x48},
        {0x041E, 0x4F},
        {0x041F, 0x92},
        {0x0420, 0x50},
        {0x0421, 0x43},
        {0x0422, 0x54},
        {0x0423, 0xC5},
        {0x0424, 0xC8},
        {0x0425, 0x58},
        {0x0426, 0xC9},
        {0x0427, 0xCA},
        {0x0428, 0xCB},
        {0x0429, 0xCC},
        {0x042A, 0x62},
        {0x042B, 0xE8},
        {0x042C, 0x62},
        {0x042D, 0x9B},
        {0x042E, 0x9C},
        {0x042F, 0x9D},
        {0x0430, 0x61},
        {0x0431, 0xCD},
        {0x0432, 0xCE},
        {0x0433, 0xCF},
        {0x0434, 0xD0},
        {0x0435, 0x65},
        {0x0436, 0xD1},
        {0x0437, 0xD5},
        {0x0438, 0xD9},
        {0x0439, 0xDA},
        {0x043A, 0x9E},
        {0x043B, 0x9F},
        {0x043C, 0xA6},
        {0x043D, 0xA7},
        {0x043E, 0x6F},
        {0x043F, 0xA9},
        {0x0440, 0x70},
        {0x0441, 0x63},
        {0x0442, 0xDB},
        {0x0443, 0x79},
        {0x0444, 0xDC},
        {0x0445, 0x78},
        {0x0446, 0xDD},
        {0x0447, 0xDF},
        {0x0448, 0xE6},
        {0x0449, 0xE7},
        {0x044A, 0xAA},
        {0x044B, 0xAB},
        {0x044C, 0xAC},
        {0x044D, 0xAE},
        {0x044E, 0xAF},
        {0x044F, 0xB0},
        {0x0407, 0xD8},
        {0x0457, 0x8B},
        {0x0456, 0x69},
        {0x0406, 0x49},
        {0x0404, 0x0F},
        {0x0454, 0x10},
        {0x0490, 0x11},
        {0x0491, 0x12},
        {0x2019, 0x27},
        {0x2014, 0x2D},
        {0x0451, 0x89},
        {0x0401, 0xD3},
        {0x00C7, 0x80},
        {0x00FC, 0x81},
        {0x00E9, 0x82},
        {0x00E2, 0x83},
        {0x00E4, 0x84},
        {0x00E0, 0x85},
        {0x00E5, 0x86},
        {0x00E7, 0x87},
        {0x00EA, 0x88},
        {0x00EB, 0x89},
        {0x00E8, 0x8A},
        {0x00EF, 0x8B},
        {0x00EE, 0x8C},
        {0x00EC, 0x8D},
        {0x00C4, 0x8E},
        {0x00C5, 0x8F},
        {0x00C9, 0x90},
        {0x00E6, 0x91},
        {0x00F4, 0x93},
        {0x00F6, 0x94},
        {0x00F2, 0x95},
        {0x00FB, 0x96},
        {0x00F9, 0x97},
        {0x00FF, 0x98},
        {0x00D6, 0x99},
        {0x00DC, 0x9A},
        {0x00E1, 0xA0},
        {0x00ED, 0xA1},
        {0x00F3, 0xA2},
        {0x00FA, 0xA3},
        {0x00F1, 0xA4},
        {0x00D1, 0xA5},
        {0x00BF, 0xA8},
        {0x00A1, 0xAD},
        {0x00C1, 0xB5},
        {0x00C2, 0xB6},
        {0x00C0, 0xB7},
        {0x00E3, 0xC6},
        {0x00C3, 0xC7},
        {0x00CA, 0xD2},
        {0x00CB, 0xD3},
        {0x00C8, 0xD4},
        {0x00CD, 0xD6},
        {0x00CE, 0xD7},
        {0x00CF, 0xD8},
        {0x00CC, 0xDE},
        {0x00D3, 0xE0},
        {0x00DF, 0xE1},
        {0x00D4, 0xE2},
        {0x00D2, 0xE3},
        {0x00F5, 0xE4},
        {0x00D5, 0xE5},
        {0x00DA, 0xE9},
        {0x00DB, 0xEA},
        {0x00D9, 0xEB},
        {0x00FD, 0xEC},
        {0x00DD, 0xED},
        {0x0105, 0xEE},
        {0x0104, 0xEF},
        {0x0107, 0xF0},
        {0x0106, 0xF1},
        {0x0119, 0xF2},
        {0x0118, 0xF3},
        {0x0142, 0xF4},
        {0x0141, 0xF5},
        {0x0144, 0xF6},
        {0x0143, 0xF7},
        {0x015B, 0xF8},
        {0x015A, 0xF9},
        {0x017A, 0xFA},
        {0x0179, 0xFB},
        {0x017C, 0xFC},
        {0x017B, 0xFD},
        {0x011B, 0x65},
        {0x010D, 0x63},
        {0x010F, 0x64},
        {0x0159, 0x72},
        {0x0161, 0x73},
        {0x017E, 0x7A},
        {0x0165, 0x74},
        {0x0148, 0x6E},
        {0x016F, 0x75},
        {0x010C, 0x43},
        {0x010E, 0x44},
        {0x0158, 0x52},
        {0x0160, 0x53},
        {0x017D, 0x5A},
        {0x0164, 0x54},
        {0x0147, 0x4E},
        {0x016E, 0x55},
        {0x011A, 0x45},
    };
    for (size_t i = 0; i < sizeof(codepage_map) / sizeof(codepage_map[0]); ++i)
    {
        if (codepage_map[i].unicode == codepoint)
            return codepage_map[i].byte;
    }

    char utf8_char[5];
    codepoint_to_utf8(codepoint, utf8_char);
    return '?';
}

uint32_t read_utf_8_codepoint(const char *text, size_t *out_seq_len)
{
    if ((text[0] & 0x80) == 0)
    {
        *out_seq_len = 1;
        return text[0];
    }
    else if ((text[0] & 0xE0) == 0xC0 && text[1] != '\0' && (text[1] & 0xC0) == 0x80)
    {
        *out_seq_len = 2;
        return ((text[0] & 0x1F) << 6) | (text[1] & 0x3F);
    }
    else if ((text[0] & 0xF0) == 0xE0 && text[1] != '\0' && text[2] != '\0' && (text[1] & 0xC0) == 0x80 && (text[2] & 0xC0) == 0x80)
    {
        *out_seq_len = 3;
        return ((text[0] & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
    }
    else if ((text[0] & 0xF8) == 0xF0 && text[1] != '\0' && text[2] != '\0' && text[3] != '\0' && (text[1] & 0xC0) == 0x80 && (text[2] & 0xC0) == 0x80 && (text[3] & 0xC0) == 0x80)
    {
        *out_seq_len = 4;
        return ((text[0] & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
    }
    else
    {
        *out_seq_len = 1;
        ERRORLOG("Invalid UTF-8 sequence starting with byte 0x%02X; using '?'", (unsigned char)text[0]);
        return '?';
    }
}

void LbTextConvertUtf8ToInternalCodepage(const char *src, char *dst, size_t dst_size)
{
    size_t out_len = 0;
    const unsigned char *text = (const unsigned char *)src;

    if (dbc_language == 0)
    {
        convert_utf8_to_codepage_string(src, dst, dst_size, get_dbc_encoding());
        return;
    }

    while (*text != '\0' && out_len + 1 < dst_size)
    {
        uint32_t codepoint;
        size_t seq_len;
        codepoint = read_utf_8_codepoint((const char *)text, &seq_len);


        dst[out_len++] = (char)convert_codepoint_to_internal_byte(codepoint);
        text += seq_len;
        
    }
    dst[out_len] = '\0';
}