#include "pre_inc.h"
// Sample playback and per-voice control.
#include "bflib_sndlib.h"
#include "bflib_sound.h"
#include "kfx/audio/AudioLog.h"
#include "kfx/audio/SoundInternal.h"
#include <AL/al.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "keeperfx.hpp"
#include "post_inc.h"

extern "C" TbBool IsSamplePlaying(SoundMilesID mss_id) {
	return g_voice_mirror.is_playing(mss_id);
}

extern "C" void SetSampleVolume(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundVolume volume) {
	g_voice_mirror.set_volume(emit_id, smptbl_id, volume, g_command_ring);
}

extern "C" void SetSamplePan(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPan pan) {
	g_voice_mirror.set_pan(emit_id, smptbl_id, pan, g_command_ring);
}

extern "C" void SetSamplePitch(SoundEmitterID emit_id, SoundSmplTblID smptbl_id, SoundPitch pitch) {
	g_voice_mirror.set_pitch(emit_id, smptbl_id, pitch, g_command_ring);
}

// No-ops: updates are queued for the audio thread.
extern "C" void begin_sample_updates() {
}

extern "C" void end_sample_updates() {
}

extern "C" SoundMilesID play_sample(
	SoundEmitterID emit_id,
	SoundSmplTblID smptbl_id,
	SoundVolume volume,
	SoundPan pan,
	SoundPitch pitch,
	char repeats, // possible values: -1, 0
	unsigned char ctype, // possible values: 2, 3
	long priority // higher = more important; matches bflib_sound.c's find_slot() convention
) {
	if (emit_id <= 0) {
		AUDIOERR("Can't play sample %d, invalid emitter ID", smptbl_id);
		return 0;
	}
	// Only positional (S3D) emitters are spatialised, not ambient loops or speech.
	const bool is_positional = (emit_id != Non3DEmitter) && (emit_id != SpeechEmitter);
	// Apply raw-ID redirect before bank dispatch (only for effect-bank IDs)
	if (smptbl_id > 0 && smptbl_id < g_speech_offset) {
		auto redir = g_id_redirects.find(smptbl_id);
		if (redir != g_id_redirects.end()) {
			smptbl_id = redir->second;
		}
	}
	// Resolve sample data from unified ID space
	const openal_buffer * buf = nullptr;
	if (smptbl_id >= g_custom_offset) {
		const SoundSmplTblID idx = smptbl_id - g_custom_offset;
		if (idx < 0 || idx >= (SoundSmplTblID)g_custom_bank.size()) {
			AUDIOERR("Can't play custom sample %d, out of range", smptbl_id);
			return 0;
		}
		buf = &g_custom_bank[idx].buffer;
	} else if (smptbl_id >= g_speech_offset) {
		const SoundSmplTblID idx = smptbl_id - g_speech_offset;
		if (idx <= 0 || idx >= (SoundSmplTblID)g_banks[1].size()) {
			AUDIOERR("Can't play speech sample %d, out of range", smptbl_id);
			return 0;
		}
		buf = &g_banks[1][idx].buffer;
	} else {
		if (smptbl_id <= 0 || smptbl_id >= (SoundSmplTblID)g_banks[0].size()) {
			if (smptbl_id != 0) {
				AUDIOERR("Can't play effect sample %d, out of range", smptbl_id);
			}
			return 0;
		}
		buf = &g_banks[0][smptbl_id].buffer;
	}
	try {
		return g_voice_mirror.allocate(emit_id, buf->id, smptbl_id, volume, pan, pitch, repeats, ctype,
		                               priority, is_positional, g_command_ring);
	} catch (const std::exception & e) {
		AUDIOERR("%s", e.what());
	}
	return 0;
}

extern "C" void stop_sample(SoundEmitterID emit_id, SoundSmplTblID smptbl_id) {
	g_voice_mirror.stop(emit_id, smptbl_id, g_command_ring);
}

extern "C" void toggle_bbking_mode() {
	g_bb_king_mode = !g_bb_king_mode;
	g_voice_mirror.set_bb_king_mode(g_bb_king_mode);
}

