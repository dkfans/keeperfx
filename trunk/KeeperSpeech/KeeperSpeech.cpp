// KeeperSpeech.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KeeperCommands.h"
#include "KeeperSpeech.h"
#include "resource.h"

#define MAX_EVENTS	16

extern HMODULE hModule;

struct State
{
	CComPtr<ISpRecognizer> engine;
	CComPtr<ISpRecoContext> recog;
	CComPtr<ISpRecoGrammar> grammar;

	int next_event;
	KEEPERSPEECH_EVENT events[MAX_EVENTS];
} static state;

enum Reason
{
	REASON_OK = 0,
	REASON_NOT_KNOWN,
	REASON_CREATE_ENGINE,
	REASON_CREATE_RECOG_CONTEXT,
	REASON_SET_NOTIFY,
	REASON_SET_INTEREST,
	REASON_CREATE_GRAMMAR,
	REASON_LOAD_GRAMMAR,
	REASON_NOMOREEVENTS,
	REASON_ACTIVATE_GRAMMAR,
};

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

static void scanForCreatureProperty(const SPPHRASEPROPERTY * prop, char * str, size_t max_len)
{
	for (; prop != NULL; prop = prop->pNextSibling) {
		if (prop->ulId == VID_Creature) {
			toKeeperString(prop->pszValue, str, max_len);
			return;
		}
	}
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
	case VID_Slap:
		pushEvent(KS_SLAP);
		break;
	case VID_PickUp:
		pushEvent(KS_PICKUP);
		break;
	case VID_Drop:
		pushEvent(KS_DROP);
		break;
	case VID_PickUpIdle:
		ev = pushEvent(KS_PICKUP_IDLE);
		scanForCreatureProperty(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
		break;
	case VID_PickUpWorking:
		ev = pushEvent(KS_PICKUP_WORKING);
		scanForCreatureProperty(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
		break;
	case VID_PickUpFighting:
		ev = pushEvent(KS_PICKUP_FIGHTING);
		scanForCreatureProperty(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
		break;
	case VID_PickUpAny:
		ev = pushEvent(KS_PICKUP_ANY);
		scanForCreatureProperty(phrase->pProperties, ev->u.creature.model_name, sizeof(ev->u.creature.model_name));
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

KEEPERSPEECH_API int __cdecl KeeperSpeechInit(void)
{
	HRESULT res;
	Reason reason;
	
	reason = REASON_NOT_KNOWN;

	//"loop" for error handling purposes (break on error)
	for (;;) {
		res = state.engine.CoCreateInstance(CLSID_SpSharedRecognizer);
		if (FAILED(res)) {
			reason = REASON_CREATE_ENGINE;
			break;
		}

		res = state.engine->CreateRecoContext(&state.recog);
		if (FAILED(res)) {
			reason = REASON_CREATE_RECOG_CONTEXT;
			break;
		}

		res = state.recog->SetNotifyCallbackFunction(recognitionCallback, 0, 0);
		if (FAILED(res)) {
			reason = REASON_SET_NOTIFY;
			break;
		}

		res = state.recog->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
		if (FAILED(res)) {
			reason = REASON_SET_INTEREST;
			break;
		}

		res = state.recog->CreateGrammar(1, &state.grammar);
		if (FAILED(res)) {
			reason = REASON_CREATE_GRAMMAR;
			break;
		}

		res = state.grammar->LoadCmdFromResource(hModule, MAKEINTRESOURCEW(IDR_COMMAND_GRAMMAR),
			L"SRGRAMMAR", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SPLO_DYNAMIC);
		if (FAILED(res)) {
			reason = REASON_LOAD_GRAMMAR;
			break;
		}

		res = state.grammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
		if (FAILED(res)) {
			reason = REASON_ACTIVATE_GRAMMAR;
			break;
		}

		return REASON_OK;
	}

	KeeperSpeechExit();
	return reason;
}

KEEPERSPEECH_API const char * __cdecl KeeperSpeechErrorMessage(int reason)
{
	switch (reason) {
	case REASON_OK:						return "Not an error";
	case REASON_CREATE_ENGINE:			return "Error creating engine";
	case REASON_CREATE_RECOG_CONTEXT:	return "Error creating recognition context";
	case REASON_SET_NOTIFY:				return "Error setting notification callback";
	case REASON_SET_INTEREST:			return "Error setting what recognition events interest us";
	case REASON_CREATE_GRAMMAR:			return "Error creating grammar";
	case REASON_LOAD_GRAMMAR:			return "Error loading grammar";
	case REASON_NOMOREEVENTS:			return "No more events in queue";
	case REASON_ACTIVATE_GRAMMAR:		return "Error activating grammar rules";
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

KEEPERSPEECH_API int __cdecl KeeperSpeechPopEvent(KEEPERSPEECH_EVENT * ev)
{
	int i = state.next_event;
	do {
		if (state.events[i].type != KS_UNUSED) {
			memcpy(ev, &state.events[i], sizeof(*ev));
			state.events[i].type = KS_UNUSED;
			return REASON_OK;
		}

		i = (i + 1) % MAX_EVENTS;
	} while (i != state.next_event);

	return REASON_NOMOREEVENTS;
}

KEEPERSPEECH_API void __cdecl KeeperSpeechClearEvents(void)
{
	int i;

	for (i = 0; i < MAX_EVENTS; ++i) {
		state.events[i].type = KS_UNUSED;
	}
}
