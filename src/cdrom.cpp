#include "pre_inc.h"
#include "bflib_sndlib.h"

// SDL completely removed CD-ROM support, go native

#if defined(__MINGW32__)
// mingw is somewhat broken...
typedef struct LPMSG *MSG;
#endif

// All the MCI stuff is not part of LEAN_AND_MEAN
#include <windows.h>
#include "post_inc.h"

namespace {

MCIDEVICEID g_redbook_device = 0;
SoundVolume g_redbook_volume = 0;

MCIDEVICEID mci_open(const char * drive) {
    MCI_OPEN_PARMS params = {};
    params.lpstrElementName = drive;
    params.lpstrDeviceType = "cdaudio";
    const auto flags = MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE;
    mciSendCommand(0, MCI_OPEN, flags, (DWORD_PTR) &params);
    return params.wDeviceID; // will be zero on error
}

bool mci_close(MCIDEVICEID device_id) {
    MCI_GENERIC_PARMS params = {};
    const auto result = mciSendCommand(device_id, MCI_CLOSE, 0, (DWORD_PTR) &params);
	return result == 0;
}

bool mci_set_time_format(MCIDEVICEID device_id) {
    MCI_SET_PARMS params = {};
    params.dwTimeFormat = MCI_FORMAT_TMSF;
    const auto flags = MCI_SET_TIME_FORMAT;
    const auto result = mciSendCommand(device_id, MCI_SET, flags, (DWORD_PTR) &params);
    return result == 0;
}

bool mci_play(MCIDEVICEID device_id, int track) {
    MCI_PLAY_PARMS params = {};
    params.dwFrom = MCI_MAKE_TMSF(track, 0, 0, 0);
    params.dwTo = MCI_MAKE_TMSF(track + 1, 0, 0, 0);
    const auto flags = MCI_FROM | MCI_TO;
    const auto result = mciSendCommand(device_id, MCI_PLAY, flags, (DWORD_PTR) &params);
    return result == 0;
}

bool mci_pause(MCIDEVICEID device_id) {
	MCI_GENERIC_PARMS params;
	const auto result = mciSendCommand(device_id, MCI_PAUSE, 0, (DWORD_PTR) &params);
	return result == 0;
}

bool mci_resume(MCIDEVICEID device_id) {
	MCI_GENERIC_PARMS params;
	const auto result = mciSendCommand(device_id, MCI_RESUME, 0, (DWORD_PTR) &params);
	return result == 0;
}

int mci_status(MCIDEVICEID device_id, int what) {
    MCI_STATUS_PARMS params = {};
    params.dwItem = what;
    const auto flags = MCI_STATUS_ITEM;
    mciSendCommand(device_id, MCI_STATUS, flags, (DWORD_PTR) &params);
    return params.dwReturn; // returns zero on error
}

bool open_redbook_device() {
	if (g_redbook_device == 0) {
		// find first cdrom device that has a disk in it
		char drive[] = "C:\\";
		for (char letter = 'C'; letter <= 'Z'; ++letter) {
			drive[0] = letter;
			if (GetDriveType(drive) != DRIVE_CDROM) {
				continue;
			}
			if (const auto device_id = mci_open(drive)) {
				const auto num_tracks = mci_status(device_id, MCI_STATUS_NUMBER_OF_TRACKS);
				if (num_tracks > 0) {
					JUSTLOG("Using cdrom drive %s for music", drive);
					g_redbook_device = device_id;
					return true;
				}
				mci_close(device_id);
			}
		}
		JUSTLOG("No cdrom drives found with a disk in it");
		return false;
	}
	return true;
}

} // local

extern "C" void SetRedbookVolume(SoundVolume value) {
	// TODO: Not implemented
	g_redbook_volume = value;
}

extern "C" void PlayRedbookTrack(int track) {
	if (open_redbook_device()) {
		mci_set_time_format(g_redbook_device);
		mci_play(g_redbook_device, track);
	}
}

extern "C" void PauseRedbookTrack() {
	if (open_redbook_device()) {
		mci_pause(g_redbook_device);
	}
}

extern "C" void ResumeRedbookTrack() {
	if (open_redbook_device()) {
		mci_resume(g_redbook_device);
	}
}
