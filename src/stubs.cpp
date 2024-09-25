#include "bflib_sound.h"
#include "bflib_sndlib.h"

extern "C" int init_miles_sound_system() { return 0; }
extern "C" void unload_miles_sound_system() {}
extern "C" void FreeAudio() {}
extern "C" void SetRedbookVolume(SoundVolume) { }
extern "C" void SetSoundMasterVolume(SoundVolume) {}
extern "C" void SetMusicMasterVolume(SoundVolume) {}
extern "C" TbBool GetSoundInstalled() { return false; }
extern "C" void PlayRedbookTrack(int) {}
extern "C" void PauseRedbookTrack() {}
extern "C" void ResumeRedbookTrack() {}
extern "C" void MonitorStreamedSoundTrack() {}
extern "C" void StopRedbookTrack() {}
extern "C" void * GetSoundDriver() { return nullptr; }
extern "C" void StopAllSamples() {}
extern "C" struct SampleInfo * GetFirstSampleInfoStructure() { return nullptr; }
extern "C" TbBool InitAudio(SoundSettings *) { return 0; }
extern "C" void SetupAudioOptionDefaults(SoundSettings *) { }
extern "C" TbBool IsSamplePlaying(SoundMilesID) { return false; }
extern "C" struct SampleInfo * GetLastSampleInfoStructure() { return nullptr; }
extern "C" SoundVolume GetCurrentSoundMasterVolume() { return 0; }
extern "C" void StopSample(SoundEmitterID, SoundSmplTblID) {}
extern "C" void SetSampleVolume(SoundEmitterID, SoundSmplTblID, SoundVolume) {}
extern "C" void SetSamplePan(SoundEmitterID, SoundSmplTblID, SoundPan) {}
extern "C" void SetSamplePitch(SoundEmitterID, SoundSmplTblID, SoundPitch) {}
extern "C" SampleInfo * PlaySampleFromAddress(SoundEmitterID, SoundSmplTblID, SoundVolume, SoundPan, SoundPitch, unsigned char a6, unsigned char a7, void * buf, SoundSFXID) { return nullptr; }

extern "C" short play_smk_(char *fname, int smkflags, int plyflags) { return 0; }
extern "C" short play_smk_direct(char *fname, int smkflags, int plyflags) { return 0; }
extern "C" short play_smk_via_buffer(char *fname, int smkflags, int plyflags) { return 0; }

// Exported functions - FLI related
extern "C" short anim_open(char *fname, int arg1, short arg2, int width, int height, int arg5, unsigned int flags) { return 0; }
extern "C" short anim_stop(void) { return 0; }
extern "C" short anim_record(void) { return 0; }
extern "C" TbBool anim_record_frame(unsigned char *screenbuf, unsigned char *palette) { return 0; }

extern "C" int steam_api_init() { return 0; }
extern "C" void steam_api_shutdown() {}
