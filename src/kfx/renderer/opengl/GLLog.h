/******************************************************************************/
/** @file GLLog.h
 *     Log macros for the OpenGL renderer.
 */
/******************************************************************************/
#ifndef KFX_GLLOG_H
#define KFX_GLLOG_H

#include "bflib_basics.h"

/** OpenGL log prefix; Lb*Log is safe from any thread. */
#define GLLOG(format, ...) LbJustLog("[gl] " format "\n", ##__VA_ARGS__)
#define GLWARN(format, ...) LbWarnLog("[gl] " format "\n", ##__VA_ARGS__)
#define GLERR(format, ...) LbErrorLog("[gl] " format "\n", ##__VA_ARGS__)

#endif
