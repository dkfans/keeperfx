#include "bflib_sound.h"

extern "C" int init_miles_sound_system() { return 0; }
extern "C" void unload_miles_sound_system() {}
extern "C" int FreeAudio() { return 0; }
extern "C" int SetRedbookVolume(int) { return 0; }
extern "C" int SetSoundMasterVolume(int) { return 0; }
extern "C" int SetMusicMasterVolume(int) { return 0; }
extern "C" int GetSoundInstalled(void) { return 0; }
extern "C" int PlayRedbookTrack(int) { return 0; }
extern "C" int PauseRedbookTrack(void) { return 0; }
extern "C" int ResumeRedbookTrack(void) { return 0; }
extern "C" int MonitorStreamedSoundTrack(void) { return 0; }
extern "C" int StopRedbookTrack(void) { return 0; }
extern "C" void * GetSoundDriver(void) { return nullptr; }
extern "C" int StopAllSamples(void) { return 0; }
extern "C" struct SampleInfo * GetFirstSampleInfoStructure(void) { return nullptr; }
extern "C" int InitAudio(void *) { return 0; }
extern "C" int SetupAudioOptionDefaults(void *) { return 0; }
extern "C" int IsSamplePlaying(int a1, int a2, int a3) { return 0; }
extern "C" struct SampleInfo * GetLastSampleInfoStructure(void) { return nullptr; }
extern "C" int GetCurrentSoundMasterVolume(void) { return 0; }
extern "C" int StopSample(SoundEmitterID emit_id, long smptbl_id) { return 0; }
extern "C" int SetSampleVolume(SoundEmitterID emit_id, long smptbl_id,long volume,long d) { return 0; }
extern "C" int SetSamplePan(SoundEmitterID emit_id, long smptbl_id,long pan,int d) { return 0; }
extern "C" int SetSamplePitch(SoundEmitterID emit_id, long smptbl_id,long pitch,int d) { return 0; }
extern "C" struct SampleInfo * PlaySampleFromAddress(SoundEmitterID emit_id, int smpl_idx, int a3, int a4, int a5, unsigned char a6, unsigned char a7, void * buf, int sfxid) { return 0; }

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
