/******************************************************************************/
/** @file lblogging.h
 * Library for text status information output.
 * @par Purpose:
 *     Header file. Defines exported routines from lblogging.c
 * @par Comment:
 *   Those are simple, low-level functions, so all are defined as inline.
 * @author   Tomasz Lis
 * @date     11 Mar 2005 - 22 Jul 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef LBLOGGING_H
#define LBLOGGING_H

# include <stdio.h>

// Routines

#define DEBUGF(fmt,args...)
// printf("\n%s: " fmt "\n",__func__,##args)
#define ERRORF(fmt,args...) { printf("%s: " fmt "\n",__func__,##args); }

long rnc_nocallback(long done,long total);
long rnc_printcallback(long done,long total);

#endif
