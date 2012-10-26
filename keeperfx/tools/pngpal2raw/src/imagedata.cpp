/******************************************************************************/
// PNG and PAL to RAW/DAT/SPR files converter for KeeperFX
/******************************************************************************/
/** @file pngpal2raw.cpp
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

#include "imagedata.h"
#include "prog_options.h"

#include <png.h>

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

