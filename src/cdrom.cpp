#include "pre_inc.h"
#include "bflib_sndlib.h"

// SDL completely removed CD-ROM support, go native

#if defined(__MINGW32__)
// mingw is somewhat broken...
typedef struct LPMSG *MSG;
#endif

// Must include without LEAN_AND_MEAN
#include <windows.h>
#include "post_inc.h"

namespace {

MCIDEVICEID g_redbook_device = 0;
SoundVolume g_redbook_volume = 0;

void stop_cdrom() {
    if (g_redbook_device > 0) {
        MCI_GENERIC_PARMS params;
        mciSendCommand(g_redbook_device, MCI_STOP, 0, (DWORD_PTR) &params);
    }
}

} // local

extern "C" void open_cdrom() {
	if (g_redbook_device == 0) {
		MCI_OPEN_PARMS params = {};
		params.lpstrDeviceType = "cdaudio";
		if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD_PTR) &params) != 0) {
			return;
		}
		g_redbook_device = params.wDeviceID;
	}
}

extern "C" void close_cdrom() {
    stop_cdrom();
    if (g_redbook_device > 0) {
		mciSendCommand(g_redbook_device, MCI_CLOSE, 0, (DWORD_PTR) NULL);
        g_redbook_device = 0;
    }
}

extern "C" void SetRedbookVolume(SoundVolume value) {
	g_redbook_volume = value;
}

extern "C" void PlayRedbookTrack(int track) {
    if (g_redbook_device > 0) {
        MCI_SET_PARMS set_params = {};
        set_params.dwTimeFormat = MCI_FORMAT_TMSF;
        if (mciSendCommand(g_redbook_device, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR) &set_params) != 0) {
            MCI_PLAY_PARMS play_params = {};
            play_params.dwFrom = MCI_MAKE_TMSF(track, 0, 0, 0);
            play_params.dwTo = MCI_MAKE_TMSF(track + 1, 0, 0, 0);
            mciSendCommand(g_redbook_device, MCI_PLAY, MCI_FROM | MCI_TO, (DWORD_PTR) &play_params);
        }
    }
}

extern "C" void PauseRedbookTrack() {
    if (g_redbook_device > 0) {
        MCI_GENERIC_PARMS params;
        mciSendCommand(g_redbook_device, MCI_PAUSE, 0, (DWORD_PTR) &params);
    }
}

extern "C" void ResumeRedbookTrack() {
    if (g_redbook_device > 0) {
        MCI_GENERIC_PARMS params;
        mciSendCommand(g_redbook_device, MCI_RESUME, 0, (DWORD_PTR) &params);
    }
}
