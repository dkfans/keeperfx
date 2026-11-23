/******************************************************************************/
/** @file AudioLog.h
 *     Log macros for the audio code.
 */
/******************************************************************************/
#ifndef KFX_AUDIOLOG_H
#define KFX_AUDIOLOG_H

#include "bflib_basics.h"

/** Audio log prefix; Lb*Log is safe from the audio thread. */
#define AUDIOLOG(format, ...) LbJustLog("[audio] " format "\n", ##__VA_ARGS__)
#define AUDIOWARN(format, ...) LbWarnLog("[audio] " format "\n", ##__VA_ARGS__)
#define AUDIOERR(format, ...) LbErrorLog("[audio] " format "\n", ##__VA_ARGS__)

#endif
