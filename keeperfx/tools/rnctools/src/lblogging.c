/******************************************************************************/
/** @file lblogging.c
 * Library for text status information output.
 * @author   Tomasz Lis
 * @author   Jon Skeet
 * @date     14 Oct 1997 - 15 Jul 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "lblogging.h"

/**
 * Empty callback - for use if no callback is needed.
 */
long rnc_nocallback(long done,long total)
{
  return 0;
}

/**
 * Console print callback - for use in console tools.
 */
long rnc_printcallback(long done,long total)
{
  printf("\rProcessing, %2.2f percent done",100.0*done/total);
  return 0;
}

