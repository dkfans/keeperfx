/******************************************************************************/
/** @file lbpathutil.h
 * Library for operations on file names and path strings.
 * @par Purpose:
 *     Header file. Defines exported routines from lbpathutil.c
 * @par Comment:
 *   None.
 * @author   Tomasz Lis
 * @date     11 Mar 2005 - 20 Oct 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LBPATHUTIL_H
#define LBPATHUTIL_H

// Routines

char *file_name_change_extension(const char *fname_inp,const char *ext);
char *file_name_strip_to_body(const char *fname_inp);
char *file_name_strip_path(const char *fname_inp);

#endif
