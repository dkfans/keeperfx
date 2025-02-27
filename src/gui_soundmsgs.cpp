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
#include "config_settings.h"
#include "game_legacy.h"
#include "bflib_sndlib.h"
#include "bflib_planar.h"
#include <deque>
#include <algorithm>
#include <string>
#include <utility>
#include <memory>
#include <map>
#include "post_inc.h"

namespace {

struct Message;

std::deque<std::unique_ptr<Message>> g_message_queue;
std::map<SoundSmplTblID, long> g_recent_samples;
std::map<std::string, long> g_recent_filenames;
std::unique_ptr<Message> g_current_message;

#define MAX_QUEUED_MESSAGES 4

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
		g_recent_samples[sample_id] = game.play_gameturn + duration;
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
		play_streamed_sample(fname.c_str(), settings.mentor_volume);
		g_recent_filenames[fname] = game.play_gameturn + duration;
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
	if (g_message_queue.size() >= MAX_QUEUED_MESSAGES) {
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
		return it->second >= game.play_gameturn;
	}
	return false;
}

bool played_recently(const char * fname)
{
	const auto it = g_recent_filenames.find(fname);
	if (it != g_recent_filenames.end()) {
		return it->second >= game.play_gameturn;
	}
	return false;
}

} // local

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
		case OMsg_RoomNeeded: return output_message(roomst->msg_needed, MESSAGE_DURATION_ROOM_NEED);
		case OMsg_RoomTooSmall: return output_message(roomst->msg_too_small, MESSAGE_DURATION_ROOM_SMALL);
		case OMsg_RoomFull: return output_message(roomst->msg_too_small, MESSAGE_DURATION_WORSHOP_FULL);
		case OMsg_RoomNoRoute: return output_message(roomst->msg_no_route, MESSAGE_DURATION_ROOM_NEED);
	}
	return false;
}
