/** @file TileSetsLog.h
 *     Log macros for tile set loading. Game thread only. */
#ifndef KFX_TILESETS_TILESETSLOG_H
#define KFX_TILESETS_TILESETSLOG_H

#include "bflib_basics.h"

#define TILESETLOG(format, ...)  LbJustLog("[tilesets] " format "\n", ##__VA_ARGS__)
#define TILESETWARN(format, ...) LbWarnLog("[tilesets] " format "\n", ##__VA_ARGS__)

#endif
