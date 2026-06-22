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
#include "config_keeperfx.h"
#include "post_inc.h"


// Reversed codepage map: indexed by byte value (0-255), contains unicode codepoint
static const uint32_t internal_codepage_map[256] = {
         0, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,  // 0x00-0x07
    0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x0404,  // 0x08-0x0F
    0x0454, 0x0490, 0x0491, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,  // 0x10-0x17
    0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,  // 0x18-0x1F
       ' ',    '!',    '"',    '#',    '$',    '%',    '&',   '\'',  // 0x20-0x27
       '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',  // 0x28-0x2F
       '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',  // 0x30-0x37
       '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',  // 0x38-0x3F
       '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G',  // 0x40-0x47
       'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',  // 0x48-0x4F
       'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',  // 0x50-0x57
       'X',    'Y',    'Z',    '[',   '\\',    ']',    '^',    '_',  // 0x58-0x5F
       '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',  // 0x60-0x67
       'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',  // 0x68-0x6F
       'p',    'q',    'r',    's',    't',    'u',    'v',    'w',  // 0x70-0x77
       'x',    'y',    'z',    '{',    '|',    '}',    '~', 0x041B,  // 0x78-0x7F
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,  // 0x80-0x87
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,  // 0x88-0x8F
    0x00C9, 0x00E6, 0x041F, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,  // 0x90-0x97
    0x00FF, 0x00D6, 0x00DC, 0x042D, 0x042E, 0x042F, 0x043A, 0x043B,  // 0x98-0x9F
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x043C, 0x043D,  // 0xA0-0xA7
    0x00BF, 0x043F, 0x044A, 0x044B, 0x044C, 0x00A1, 0x044D, 0x044E,  // 0xA8-0xAF
    0x044F, 0x0411, 0x0413, 0x0414, 0x0416, 0x00C1, 0x00C2, 0x00C0,  // 0xB0-0xB7
        0,      0,      0,       0,      0,      0,      0,      0,  // 0xB8-0xBF
        0,      0,  0x0417, 0x0418, 0x0419, 0x0423, 0x00E3, 0x00C3,  // 0xC0-0xC7
    0x0424, 0x0426, 0x0427, 0x0428, 0x0429, 0x0431, 0x0432, 0x0433,  // 0xC8-0xCF
    0x0434, 0x0436, 0x00CA, 0x00CB, 0x00C8, 0x0437, 0x00CD, 0x00CE,  // 0xD0-0xD7
    0x00CF, 0x0438, 0x0439, 0x0442, 0x0444, 0x0446, 0x00CC, 0x0447,  // 0xD8-0xDF
    0x00D3, 0x00DF, 0x00D4, 0x00D2, 0x00F5, 0x00D5, 0x0448, 0x0449,  // 0xE0-0xE7
    0x042B, 0x00DA, 0x00DB, 0x00D9, 0x00FD, 0x00DD, 0x0105, 0x0104,  // 0xE8-0xEF
    0x0107, 0x0106, 0x0119, 0x0118, 0x0142, 0x0141, 0x0144, 0x0143,  // 0xF0-0xF7
    0x015B, 0x015A, 0x017A, 0x0179, 0x017C, 0x017B,      0,      0,  // 0xF8-0xFF
};


/******************************************************************************/


static uint32_t internal_byte_to_unicode(unsigned char byte)
{
    if (byte == 0)
        return 0;
    uint32_t unicode = internal_codepage_map[byte];
    return (unicode != 0) ? unicode : (uint32_t)'?';
}

size_t encode_utf8_codepoint(uint32_t codepoint, char *dst, size_t dst_size)
{
    if (dst_size == 0)
        return 0;
    if (codepoint < 0x80)
    {
        dst[0] = (char)codepoint;
        return 1;
    }
    else if (codepoint < 0x800)
    {
        if (dst_size < 2) return 0;
        dst[0] = (char)(0xC0 | (codepoint >> 6));
        dst[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    }
    else if (codepoint < 0x10000)
    {
        if (dst_size < 3) return 0;
        dst[0] = (char)(0xE0 | (codepoint >> 12));
        dst[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        dst[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        if (dst_size < 4) return 0;
        dst[0] = (char)(0xF0 | (codepoint >> 18));
        dst[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        dst[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        dst[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }
    return 0;
}

size_t convert_codepage_to_utf8_buffer(const char *src, size_t src_size, char *dst, size_t dst_size, uint8_t lang_id)
{
    if ((src == NULL) || (dst == NULL) || (src_size == 0) || (dst_size == 0))
        return 0;

    if (lang_id != Lang_Japanese && lang_id != Lang_ChineseInt && lang_id != Lang_ChineseTra && lang_id != Lang_Korean)
    {
        size_t out_pos = 0;
        for (size_t i = 0; i < src_size; ++i)
        {
            uint32_t codepoint = internal_byte_to_unicode((unsigned char)src[i]);
            size_t written = encode_utf8_codepoint(codepoint, dst + out_pos, dst_size - out_pos);
            if (written == 0)
            {
                if (out_pos < dst_size)
                    dst[out_pos] = '\0';
                return out_pos;
            }
            out_pos += written;
        }
        if (out_pos < dst_size)
            dst[out_pos] = '\0';
        else if (dst_size > 0)
            dst[dst_size - 1] = '\0';
        return out_pos;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
    UINT codepage = 0;
    if (lang_id == Lang_Japanese)
        codepage = 932; // SHIFT_JIS
    else if (lang_id == Lang_ChineseInt || lang_id == Lang_ChineseTra)
        codepage = 936; // GBK
    else if (lang_id == Lang_Korean)
        codepage = 949; // EUC-KR
    else
        return 0;

    int wlen = MultiByteToWideChar(codepage, 0, src, (int)src_size, NULL, 0);
    if (wlen <= 0)
        return 0;

    WCHAR *wbuf = (WCHAR *)malloc((size_t)wlen * sizeof(WCHAR));
    if (wbuf == NULL)
        return 0;

    if (MultiByteToWideChar(codepage, 0, src, (int)src_size, wbuf, wlen) == 0)
    {
        free(wbuf);
        return 0;
    }

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wbuf, wlen, dst, (int)dst_size, NULL, NULL);
    free(wbuf);
    if (utf8_len <= 0)
    {
        if (dst_size > 0)
            dst[0] = '\0';
        return 0;
    }

    if ((size_t)utf8_len >= dst_size)
    {
        dst[dst_size - 1] = '\0';
        return dst_size - 1;
    }
    dst[utf8_len] = '\0';
    return (size_t)utf8_len;
#else

    iconv_t cd;
    if (lang_id == Lang_Japanese)
        cd = iconv_open("UTF-8", "CP932");
    else if (lang_id == Lang_ChineseInt || lang_id == Lang_ChineseTra)
        cd = iconv_open("UTF-8", "CP936");
    else if (lang_id == Lang_Korean)
        cd = iconv_open("UTF-8", "EUC-KR");
    else
        return 0;

    char *inbuf = (char *)src;
    size_t inbytes = src_size;
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

uint32_t read_utf_8_codepoint_f(const char *text, size_t *out_seq_len, const char *func_name)
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
        ERRORLOG("%s: Invalid UTF-8 sequence starting with byte 0x%02X; using '?'", func_name, (unsigned char)text[0]);
        return '?';
    }
}

