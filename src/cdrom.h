#ifndef DK_CDROM_H
#define DK_CDROM_H

#include "bflib_sound.h"

#ifdef __cplusplus
extern "C" {
#endif

void SetRedbookVolume(SoundVolume);
TbBool PlayRedbookTrack(int);
void PauseRedbookTrack(void);
void ResumeRedbookTrack(void);
void StopRedbookTrack(void);

#ifdef __cplusplus
}
#endif
#endif
