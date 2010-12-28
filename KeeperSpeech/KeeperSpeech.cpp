// KeeperSpeech.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "KeeperSpeech.h"


// This is an example of an exported variable
//KEEPERSPEECH_API int nKeeperSpeech=0;

struct State
{
	CComPtr<ISpRecognizer> engine;
	CComPtr<ISpRecoContext> recog;
} static state;

enum Reason
{
	REASON_OK = 0,
	REASON_NOT_KNOWN,
	REASON_CREATE_ENGINE,
	REASON_CREATE_RECOG_CONTEXT,
	REASON_SET_NOTIFY,
	REASON_SET_INTEREST,
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

		return REASON_OK;
	}

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
	default:							return "Unknown error";
	}
}

KEEPERSPEECH_API void __cdecl KeeperSpeechExit(void)
{
	if (state.recog) {
		state.recog->SetNotifySink(NULL);
		state.recog.Release();
	}

	if (state.engine) {
		state.engine.Release();
	}
}
