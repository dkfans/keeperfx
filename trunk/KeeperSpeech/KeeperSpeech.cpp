// KeeperSpeech.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KeeperCommands.h"
#include "KeeperSpeech.h"
#include "resource.h"

#define MAX_EVENTS	4 //probably could be 1 and it wouldn't matter much... since DK can only issue 1 packet/(player*frame)

extern HMODULE hModule;

struct State
{
	CComPtr<ISpRecognizer> engine;
	CComPtr<ISpRecoContext> recog;
	CComPtr<ISpRecoGrammar> grammar;

	int next_event;
	KEEPERSPEECH_EVENT events[MAX_EVENTS];
} static state;

static KEEPERSPEECH_EVENT * pushEvent(KEEPERSPEECH_EVENT_TYPE type)
{
	KEEPERSPEECH_EVENT * ev = &state.events[state.next_event];
	state.next_event = (state.next_event + 1) % MAX_EVENTS;

	memset(ev, 0, sizeof(*ev));
	ev->type = type;

	return ev;
}

static void toKeeperString(LPCWSTR old_str, char * new_str, size_t max_len)
{
	/*WideCharToMultiByte(CP_ACP, WC_DEFAULTCHAR, old_str, -1,
			new_str, max_len, "X", NULL);*/
	int i;

	if (old_str == NULL) {
		*new_str = 0;
		return;
	}

	for (i = 0; i < (int) max_len - 1 && old_str[i]; ++i) {
		if (old_str[i] > 127) {
			new_str[i] = '?';
		}
		else {
			new_str[i] = (char ) old_str[i];
		}
	}

	new_str[i] = 0;
}

static const SPPHRASEPROPERTY * scanForProperty(const SPPHRASEPROPERTY * root, ULONG id_to_find)
{
	const SPPHRASEPROPERTY * prop;

	for (prop = root; prop != NULL; prop = prop->pNextSibling) {
		if (prop->ulId == id_to_find) {
			return prop;
		}
	}

	return NULL;
}

static void parseCreatureId(const SPPHRASEPROPERTY * prop, char * str, size_t max_len)
{
	prop = scanForProperty(prop, VID_Creature);
	toKeeperString(prop->pszValue, str, max_len);
}

static int parseRoomId(const SPPHRASEPROPERTY * prop)
{
	prop = scanForProperty(prop, VID_Room);
	return prop->vValue.lVal;
}

static void parsePowerId(const SPPHRASEPROPERTY * prop, char * str, size_t max_len)
{
	prop = scanForProperty(prop, VID_Power);
	toKeeperString(prop->pszValue, str, max_len);
}

static void handleRecognition(ISpPhrase * p)
{
	SPPHRASE * phrase;
	
	HRESULT res;
	KEEPERSPEECH_EVENT * ev;

	res = p->GetPhrase(&phrase);
	if (FAILED(res)) {
		return;
	}

	//parsing results below
	switch (phrase->Rule.ulId) {
	/*case VID_Slap:
		pushEvent(KS_SLAP);
		break;
	case VID_PickUp:
		pushEvent(KS_PICKUP);
		break;
	case VID_Drop:
		pushEvent(KS_DROP);
		break;*/
	case VID_HandChoose:
		pushEvent(KS_HAND_CHOOSE);
		break;
	case VID_HandAction:
		pushEvent(KS_HAND_ACTION); //no more dirty jokes plz
		break;
	case VID_ViewInfo:
		pushEvent(KS_VIEW_INFO);
		break;
	case VID_ViewRooms:
		pushEvent(KS_VIEW_ROOMS);
		break;
	case VID_ViewPowers:
		pushEvent(KS_VIEW_POWERS);
		break;
	case VID_ViewTraps:
		pushEvent(KS_VIEW_TRAPS);
		break;
	case VID_ViewCreatures:
		pushEvent(KS_VIEW_CREATURES);
		break;
	case VID_PickUpIdle:
		ev = pushEvent(KS_PICKUP_IDLE);
		parseCreatureId(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
		break;
	case VID_PickUpWorking:
		ev = pushEvent(KS_PICKUP_WORKING);
		parseCreatureId(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
		break;
	case VID_PickUpFighting:
		ev = pushEvent(KS_PICKUP_FIGHTING);
		parseCreatureId(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
		break;
	case VID_PickUpAny:
		ev = pushEvent(KS_PICKUP_ANY);
		parseCreatureId(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
		break;
	case VID_SelectRoom:
		ev = pushEvent(KS_SELECT_ROOM);
		ev->u.room.id = parseRoomId(phrase->pProperties);
		break;
	case VID_SelectPower:
		ev = pushEvent(KS_SELECT_POWER);
		parsePowerId(phrase->pProperties, ev->u.power.model_name, sizeof(ev->u.power.model_name));
		break;
	}

	CoTaskMemFree(phrase);
}

static void __stdcall recognitionCallback(WPARAM wParam, LPARAM lParam)
{
	CSpEvent ev;
	while (ev.GetFrom(state.recog) == S_OK) {
		if (ev.eEventId == SPEI_RECOGNITION) {
			handleRecognition(ev.RecoResult());
		}
	}
}

KEEPERSPEECH_API KEEPERSPEECH_REASON __cdecl KeeperSpeechInit(void)
{
	HRESULT res;
	KEEPERSPEECH_REASON reason;
	
	reason = KSR_NOT_KNOWN;

	//"loop" for error handling purposes (break on error)
	for (;;) {
		res = state.engine.CoCreateInstance(CLSID_SpSharedRecognizer);
		if (FAILED(res)) {
			reason = KSR_CREATE_ENGINE;
			break;
		}

		res = state.engine->CreateRecoContext(&state.recog);
		if (FAILED(res)) {
			reason = KSR_CREATE_RECOG_CONTEXT;
			break;
		}

		res = state.recog->SetNotifyCallbackFunction(recognitionCallback, 0, 0);
		if (FAILED(res)) {
			reason = KSR_SET_NOTIFY;
			break;
		}

		res = state.recog->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
		if (FAILED(res)) {
			reason = KSR_SET_INTEREST;
			break;
		}

		res = state.recog->CreateGrammar(1, &state.grammar);
		if (FAILED(res)) {
			reason = KSR_CREATE_GRAMMAR;
			break;
		}

		res = state.grammar->LoadCmdFromResource(hModule, MAKEINTRESOURCEW(IDR_COMMAND_GRAMMAR),
			L"SRGRAMMAR", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SPLO_DYNAMIC);
		if (FAILED(res)) {
			reason = KSR_LOAD_GRAMMAR;
			break;
		}

		res = state.grammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
		if (FAILED(res)) {
			reason = KSR_ACTIVATE_GRAMMAR;
			break;
		}

		return KSR_OK;
	}

	KeeperSpeechExit();
	return reason;
}

KEEPERSPEECH_API const char * __cdecl KeeperSpeechErrorMessage(KEEPERSPEECH_REASON reason)
{
	switch (reason) {
	case KSR_OK:						return "Not an error";
	case KSR_CREATE_ENGINE:			return "Error creating engine";
	case KSR_CREATE_RECOG_CONTEXT:	return "Error creating recognition context";
	case KSR_SET_NOTIFY:				return "Error setting notification callback";
	case KSR_SET_INTEREST:			return "Error setting what recognition events interest us";
	case KSR_CREATE_GRAMMAR:			return "Error creating grammar";
	case KSR_LOAD_GRAMMAR:			return "Error loading grammar";
	case KSR_NOMOREEVENTS:			return "No more events in queue";
	case KSR_ACTIVATE_GRAMMAR:		return "Error activating grammar rules";
	default:							return "Unknown error";
	}
}

KEEPERSPEECH_API void __cdecl KeeperSpeechExit(void)
{
	if (state.grammar) {
		state.grammar.Release();
	}

	if (state.recog) {
		state.recog->SetNotifySink(NULL);
		state.recog.Release();
	}

	if (state.engine) {
		state.engine.Release();
	}
}

KEEPERSPEECH_API KEEPERSPEECH_REASON __cdecl KeeperSpeechPopEvent(KEEPERSPEECH_EVENT * ev)
{
	int i = state.next_event;
	do {
		if (state.events[i].type != KS_UNUSED) {
			memcpy(ev, &state.events[i], sizeof(*ev));
			state.events[i].type = KS_UNUSED;
			return KSR_OK;
		}

		i = (i + 1) % MAX_EVENTS;
	} while (i != state.next_event);

	return KSR_NOMOREEVENTS;
}

KEEPERSPEECH_API void __cdecl KeeperSpeechClearEvents(void)
{
	int i;

	for (i = 0; i < MAX_EVENTS; ++i) {
		state.events[i].type = KS_UNUSED;
	}
}
