// KeeperSpeech.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KeeperCommands.h"
#include "KeeperSpeech.h"
#include "resource.h"

extern HMODULE hModule;

struct State
{
	CComPtr<ISpRecognizer> engine;
	CComPtr<ISpRecoContext> recog;
	CComPtr<ISpRecoGrammar> grammar;
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
};

static void __stdcall recognitionCallback(WPARAM wParam, LPARAM lParam)
{
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
