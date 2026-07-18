/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_soundmsgs.c
 *     Allows to play sound messages during the game.
 * @par Purpose:
 *     Functions to manage queue of speeches to be played.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Jun 2010 - 11 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "gui_soundmsgs.h"
#include "config_sounds.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "game_legacy.h"
#include "sound_manager.h"
#include "bflib_sndlib.h"
#include "bflib_fileio.h"
#include "bflib_planar.h"
#include "custom_zip.h"
#include <deque>
#include <algorithm>
#include <string>
#include <utility>
#include <memory>
#include <filesystem>
#include <fstream>
#include <map>
#include "config.h"
#include "post_inc.h"

// Maximum number of messages that can be queued at once.
int g_speech_queue_limit = 4;

namespace {

struct Message;

std::deque<std::unique_ptr<Message>> g_message_queue;
std::map<SoundSmplTblID, long> g_recent_samples;
std::map<std::string, long> g_recent_filenames;
std::unique_ptr<Message> g_current_message;

enum MessageType {
	MT_Default = 1,
	MT_Custom  = 2,
};

struct Message {

	MessageType type;
	long duration;

	Message(MessageType _type, long _duration)
	: type(_type), duration(_duration)
	{}

	virtual bool is(const Message &) const noexcept = 0;
	virtual void play() const noexcept = 0;
};

struct DefaultMessage : Message {

	SoundSmplTblID sample_id;

	DefaultMessage(long _sample_id, long _duration)
	: Message(MT_Default, _duration), sample_id(_sample_id)
	{}

	bool is(const Message & msg) const noexcept override
	{
		return (msg.type == MT_Default) &&
			(static_cast<const DefaultMessage *>(&msg)->sample_id == sample_id);
	}

	void play() const noexcept override
	{
		play_speech_sample(sample_id);
		g_recent_samples[sample_id] = get_gameturn() + duration;
	}
};

struct CustomMessage : Message {

	std::string fname;

	CustomMessage(const char * _fname, long _duration)
	: Message(MT_Custom, _duration), fname(_fname)
	{}

	bool is(const Message & msg) const noexcept override
	{
		return (msg.type == MT_Custom) &&
			(static_cast<const CustomMessage *>(&msg)->fname == fname);
	}

	void play() const noexcept override
	{
		if (play_streamed_sample(fname.c_str(), settings.mentor_volume)) {
			g_recent_filenames[fname] = get_gameturn() + duration;
		}
	}
};

bool already_in_queue(const Message & msg)
{
	return std::any_of(g_message_queue.begin(), g_message_queue.end(), [&] (const auto & elem) {
		return elem->is(msg);
	});
}

bool push_queue(std::unique_ptr<Message> msg)
{
	if (g_message_queue.size() >= (size_t)g_speech_queue_limit) {
		SYNCDBG(8, "message queue full");
		return false;
	} else if (g_current_message && g_current_message->is(*msg)) {
		SYNCDBG(8, "message currently playing");
		return true;
	} else if (already_in_queue(*msg)) {
		SYNCDBG(8, "message already in queue");
		return false;
	}
	g_message_queue.emplace_back(std::move(msg));
	SYNCDBG(8,"Playing queued");
	return true;
}

std::unique_ptr<Message> pop_queue()
{
	auto msg = std::move(g_message_queue.front());
	g_message_queue.pop_front();
	return msg;
}

bool played_recently(SoundSmplTblID sample_id)
{
	const auto it = g_recent_samples.find(sample_id);
	if (it != g_recent_samples.end()) {
		return it->second >= get_gameturn();
	}
	return false;
}

bool played_recently(const char * fname)
{
	const auto it = g_recent_filenames.find(fname);
	if (it != g_recent_filenames.end()) {
		return it->second >= get_gameturn();
	}
	return false;
}

} // local

// Helper: resolve a speech file path using a 5-step fallback strategy:
//   1. Language-specific variant (e.g. speech/dut/file.ogg)
//   2. Base fallback path        (e.g. speech/file.ogg)
//   3. English variant           (e.g. speech/eng/file.ogg)
//   4. First available language subdirectory
//   5. Empty string (not found)
enum class SpeechResolveResult { LanguageMatch, FallbackBase, FallbackEng, FallbackAnyLang, ZipBundled, NotFound };

// Extract a speech entry named in [speech] from the current level's mapNNNNN.zip bundle
// into a cached temp file, and return that file's path.
static std::string extract_zip_speech_to_temp(LevelNumber lvnum, const char* entry_path)
{
	unsigned char* data = nullptr;
	size_t size = 0;
	if (!read_map_zip_entry(lvnum, entry_path, &data, &size)) {
		return "";
	}
	std::string safe_name = entry_path;
	for (auto& c : safe_name) {
		if (c == '/' || c == '\\' || c == ':') c = '_';
	}
	std::error_code ec;
	std::filesystem::path temp_dir = std::filesystem::temp_directory_path(ec);
	if (ec) {
		free(data);
		return "";
	}
	char subdir[64];
	snprintf(subdir, sizeof(subdir), "keeperfx_map%05d_speech", lvnum);
	temp_dir /= subdir;
	std::filesystem::create_directories(temp_dir, ec);
	std::filesystem::path temp_file = temp_dir / safe_name;

	bool already_extracted = false;
	if (std::filesystem::exists(temp_file, ec)) {
		const auto existing_size = std::filesystem::file_size(temp_file, ec);
		already_extracted = !ec && (existing_size == size);
	}
	if (!already_extracted) {
		std::ofstream out(temp_file, std::ios::binary | std::ios::trunc);
		if (!out || !out.write(reinterpret_cast<const char*>(data), (std::streamsize)size)) {
			free(data);
			return "";
		}
	}
	free(data);
	return temp_file.string();
}

static std::string resolve_speech_path(const char* path, const char* lang,
                                       SpeechResolveResult& result,
                                       std::string& found_lang)
{
	static const TbFileGroups search_groups[] = {
		FGrp_CmpgConfig, FGrp_CmpgLvls, FGrp_CmpgMedia, FGrp_Main
	};
	const unsigned int num_groups = sizeof(search_groups) / sizeof(search_groups[0]);
	const char* slash = strrchr(path, '/');
	result = SpeechResolveResult::NotFound;

	// Step 1: language-specific variant
	if (lang != nullptr && lang[0] != '\0') {
		char lang_path[512];
		if (slash != nullptr) {
			snprintf(lang_path, sizeof(lang_path), "%.*s/%s/%s",
				(int)(slash - path), path, lang, slash + 1);
		} else {
			snprintf(lang_path, sizeof(lang_path), "%s/%s", lang, path);
		}
		for (unsigned int g = 0; g < num_groups; g++) {
			const char* candidate = prepare_file_fmtpath(search_groups[g], "%s", lang_path);
			if (candidate != nullptr && LbFileExists(candidate)) {
				result = SpeechResolveResult::LanguageMatch;
				return std::string(candidate);
			}
		}
	}

	// Step 2: English variant (if not already English)
	if (lang == nullptr || strcmp(lang, "eng") != 0) {
		char eng_path[512];
		if (slash != nullptr) {
			snprintf(eng_path, sizeof(eng_path), "%.*s/eng/%s",
				(int)(slash - path), path, slash + 1);
		} else {
			snprintf(eng_path, sizeof(eng_path), "eng/%s", path);
		}
		for (unsigned int g = 0; g < num_groups; g++) {
			const char* candidate = prepare_file_fmtpath(search_groups[g], "%s", eng_path);
			if (candidate != nullptr && LbFileExists(candidate)) {
				result = SpeechResolveResult::FallbackEng;
				return std::string(candidate);
			}
		}
	}

	// Step 3: base fallback path
	for (unsigned int g = 0; g < num_groups; g++) {
		const char* candidate = prepare_file_fmtpath(search_groups[g], "%s", path);
		if (candidate != nullptr && LbFileExists(candidate)) {
			result = SpeechResolveResult::FallbackBase;
			return std::string(candidate);
		}
	}

	// Step 4: first available language subdirectory
	const char* filename = (slash != nullptr) ? (slash + 1) : path;
	for (unsigned int g = 0; g < num_groups; g++) {
		char base_dir[2048];
		if (slash != nullptr) {
			char dir_portion[512];
			snprintf(dir_portion, sizeof(dir_portion), "%.*s", (int)(slash - path), path);
			prepare_file_path_buf(base_dir, sizeof(base_dir), search_groups[g], dir_portion);
		} else {
			prepare_file_path_buf(base_dir, sizeof(base_dir), search_groups[g], "");
		}
		if (base_dir[0] == '\0') continue;
		try {
			for (auto& entry : std::filesystem::directory_iterator(base_dir)) {
				if (!entry.is_directory()) continue;
				const std::string lang_name = entry.path().filename().string();
				const std::string candidate = entry.path().string() + "/" + filename;
				if (LbFileExists(candidate.c_str())) {
					result = SpeechResolveResult::FallbackAnyLang;
					found_lang = lang_name;
					return candidate;
				}
			}
		} catch (...) {}
	}

	// Step 5: current level's map zip bundle.
    // speech/<lang>/file.ogg
    // speech/eng/file.ogg etc. (same fallback order as above)
	{
		const char* zip_filename = (slash != nullptr) ? (slash + 1) : path;

		if (lang != nullptr && lang[0] != '\0') {
			char lang_path[512];
			snprintf(lang_path, sizeof(lang_path), "speech/%s/%s", lang, zip_filename);
			std::string extracted = extract_zip_speech_to_temp(game.last_level, lang_path);
			if (!extracted.empty()) {
				result = SpeechResolveResult::ZipBundled;
				return extracted;
			}
		}
		if (lang == nullptr || strcmp(lang, "eng") != 0) {
			char eng_path[512];
			snprintf(eng_path, sizeof(eng_path), "speech/eng/%s", zip_filename);
			std::string extracted = extract_zip_speech_to_temp(game.last_level, eng_path);
			if (!extracted.empty()) {
				result = SpeechResolveResult::ZipBundled;
				return extracted;
			}
		}
		{
			char base_path[512];
			snprintf(base_path, sizeof(base_path), "speech/%s", zip_filename);
			std::string extracted = extract_zip_speech_to_temp(game.last_level, base_path);
			if (!extracted.empty()) {
				result = SpeechResolveResult::ZipBundled;
				return extracted;
			}
		}
	}

	return "";
}

/**
 * Plays an in-game voice message.
 *
 * @param sample_id Sample ID to play.
 * @param duration Number of ticks to supress future, identical messages.
 * @return True if the message was either played or queued. Otherwise, false.
 */
extern "C" TbBool output_message(SoundSmplTblID sample_id, long duration)
{
	try {
		SYNCDBG(8, "Sample ID %d, duration %ld", sample_id, duration);
		if ((sample_id < 0) || (sample_id >= SMsg_MAX)) {
			SYNCDBG(8, "Sample ID (%d) invalid, skipping", sample_id);
			return false;
		} else if (played_recently(sample_id)) {
			SYNCDBG(8, "Sample ID (%d) played recently, skipping", sample_id);
			return false;
		}
		if (g_speech_overrides[sample_id][0] != '\0') {
			const char* spath = g_speech_overrides[sample_id];
			if (strcasecmp(spath, "none") == 0 || strcasecmp(spath, "null") == 0 || strcmp(spath, "0") == 0) {
				SYNCDBG(8, "Sample ID (%d) silenced by override '%s', skipping", sample_id, spath);
				return false;
			}
			const char* lang = get_language_lwrstr(install_info.lang_id);
			SpeechResolveResult resolve_result;
			std::string found_lang;
			std::string resolved = resolve_speech_path(spath, lang, resolve_result, found_lang);
			if (resolve_result == SpeechResolveResult::FallbackEng) {
                SYNCDBG(8, "No speech for language '%s', using English fallback: %s", lang, spath);
			} else if (resolve_result == SpeechResolveResult::FallbackBase) {
                SYNCDBG(8, "No speech for language '%s' or English, using base fallback: %s", lang, spath);
			} else if (resolve_result == SpeechResolveResult::FallbackAnyLang) {
                SYNCDBG(8, "No speech for language '%s', English, or base fallback, using first available language '%s': %s",
					lang, found_lang.c_str(), spath);
			} else if (resolve_result == SpeechResolveResult::ZipBundled) {
                SYNCDBG(8, "Speech '%s' not found on disk, using map zip bundle", spath);
			} else if (resolve_result == SpeechResolveResult::NotFound) {
				WARNLOG("Speech '%s' not found for language '%s' and no fallback or alternative language exists",
					spath, lang);
				return false;
			}
			return output_custom_message(resolved.c_str(), duration);
		}
		auto msg = std::make_unique<DefaultMessage>(sample_id, duration);
		if (speech_sample_playing()) {
			return push_queue(std::move(msg));
		}
		g_current_message = std::move(msg);
		g_current_message->play();
		return true;
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	return false;
}

/**
 * Plays a speech file by searching campaign/level/media/root directories.
 * Applies 5-step fallback: language variant -> eng -> base path -> any language -> nothing.
 */
extern "C" TbBool output_message_from_path(const char* path, long duration)
{
	const char* lang = get_language_lwrstr(install_info.lang_id);
	SpeechResolveResult resolve_result;
	std::string found_lang;
	std::string resolved = resolve_speech_path(path, lang, resolve_result, found_lang);
	if (resolve_result == SpeechResolveResult::FallbackEng) {
        SYNCDBG(8, "No speech for language '%s', using English fallback: %s", lang, path);
	} else if (resolve_result == SpeechResolveResult::FallbackBase) {
        SYNCDBG(8, "No speech for language '%s' or English, using base fallback: %s", lang, path);
	} else if (resolve_result == SpeechResolveResult::FallbackAnyLang) {
        SYNCDBG(8, "No speech for language '%s', English, or base fallback, using first available language '%s': %s",
			lang, found_lang.c_str(), path);
	} else if (resolve_result == SpeechResolveResult::ZipBundled) {
        SYNCDBG(8, "Speech '%s' not found on disk, using map zip bundle", path);
	} else if (resolve_result == SpeechResolveResult::NotFound) {
		WARNLOG("Speech '%s' not found for language '%s' and no fallback or alternative language exists",
			path, lang);
		return false;
	}
	return output_custom_message(resolved.c_str(), duration);
}

/**
 * Plays a speech message from a SpeechRef, using path-based playback if a path is set.
 */
extern "C" TbBool play_speech_ref(const SpeechRef* ref, long duration)
{
	if (ref->path[0] != '\0')
		return output_message_from_path(ref->path, duration);
	if (ref->id > 0)
		return output_message(ref->id, duration);
	return false;
}

/**
 * Plays a custom in-game voice message.
 *
 * @param fname Name of file to play.
 * @param duration Number of ticks to supress future, identical messages.
 * @return True if the message was either played or queued. Otherwise, false.
 */
extern "C" TbBool output_custom_message(const char * fname, long duration)
{
	try {
		SYNCDBG(8, "Filename %s, duration %ld", fname, duration);
		if (strlen(fname) == 0) {
			SYNCDBG(8, "Filename invalid, skipping");
			return false;
		} else if (played_recently(fname)) {
			SYNCDBG(8, "Filename (%s) played recently, skipping", fname);
			return false;
		}
		auto msg = std::make_unique<CustomMessage>(fname, duration);
		if (speech_sample_playing()) {
			return push_queue(std::move(msg));
		}
		g_current_message = std::move(msg);
		g_current_message->play();
		return true;
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	return false;
}

/**
 * Plays an in-game voice message for a thing out of viewing range.
 *
 * @param thing Thing to play message for.
 * @param sample_id Sample ID to play.
 * @param duration Number of ticks to supress future, identical messages.
 * @return True if the message was either played or queued. Otherwise, false.
 */
extern "C" TbBool output_message_far_from_thing(
	const Thing * thing,
	SoundSmplTblID sample_id,
	long duration
) {
	try {
		SYNCDBG(8, "Sample ID %d, duration %ld", sample_id, duration);
		if ((sample_id < 0) || (sample_id >= SMsg_MAX)) {
			SYNCDBG(8, "Sample ID (%d) invalid, skipping", sample_id);
			return false;
		} else if (played_recently(sample_id)) {
			SYNCDBG(8, "Sample ID (%d) played recently, skipping", sample_id);
			return false;
		} else if (thing_is_invalid(thing)) {
			return false;
		}
		Coord3d rcpos;
		rcpos.x.val = Receiver.pos.val_x;
		rcpos.y.val = Receiver.pos.val_y;
		rcpos.z.val = Receiver.pos.val_z;
		if (get_chessboard_3d_distance(&rcpos, &thing->mappos) <= MaxSoundDistance)
		{
			SYNCDBG(8, "Thing nearby, skipping");
			return false;
		}
		auto msg = std::make_unique<DefaultMessage>(sample_id, duration);
		if (speech_sample_playing()) {
			return push_queue(std::move(msg));
		}
		g_current_message = std::move(msg);
		g_current_message->play();
		return true;
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
	return false;
}

extern "C" void process_messages()
{
	try {
		SYNCDBG(17, "Starting");
		if (speech_sample_playing()) {
			// If already playing, just wait for next time
		} else if (g_message_queue.empty()) {
			// If no messages are in queue, don't play anything
			g_current_message = nullptr;
		} else {
			// Otherwise remove next message from queue and play it
			g_current_message = pop_queue();
			g_current_message->play();
		}
		SYNCDBG(17, "Finished");
	} catch (const std::exception & e) {
		ERRORLOG("%s", e.what());
	}
}

extern "C" void clear_messages()
{
	g_message_queue.clear();
	g_current_message = nullptr;
	g_recent_samples.clear();
	g_recent_filenames.clear();
}

extern "C" TbBool output_room_message(
	PlayerNumber plyr_idx,
	RoomKind rkind,
	OutputMessageKind msg_kind
) {
	if (!is_my_player_number(plyr_idx)) {
		return false;
	}
	const auto roomst = get_room_kind_stats(rkind);
	switch (msg_kind)
	{
		case OMsg_RoomNeeded: return play_speech_ref(&roomst->msg_needed, MESSAGE_DURATION_ROOM_NEED);
		case OMsg_RoomTooSmall: return play_speech_ref(&roomst->msg_too_small, MESSAGE_DURATION_ROOM_SMALL);
		case OMsg_RoomFull: return play_speech_ref(&roomst->msg_too_small, MESSAGE_DURATION_WORSHOP_FULL);
		case OMsg_RoomNoRoute: return play_speech_ref(&roomst->msg_no_route, MESSAGE_DURATION_ROOM_NEED);
	}
	return false;
}

void script_play_message(TbBool param_is_string, const char msgtype_id, const short msg_id, const char *filename)
{

    if (!param_is_string)
    {
        switch (msgtype_id)
        {
            case 1: // speech message
            {
                output_message(msg_id, 0);
                break;
            }
            case 2: // sound effect
            {
                play_non_3d_sample(msg_id);
                break;
            }
        }
    }
    else
    {
        switch (msgtype_id)
        {
            case 1: // speech message
            {
                output_message_from_path(filename, settings.mentor_volume);
                break;
            }
            case 2: // sound effect
            {
                const SoundSmplTblID sound_id = sound_manager_load_named_sound(filename, filename, 1);
                if (sound_id > 0) {
                    play_non_3d_sample(sound_id);
                } else {
                    WARNLOG("Unable to resolve sound effect '%s' for PLAY_MESSAGE", filename);
                }
                break;
            }
        }
    }
}