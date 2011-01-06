// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KEEPERSPEECH_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KEEPERSPEECH_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifndef KEEPERSPEECH_H
#define KEEPERSPEECH_H

#ifdef KEEPERSPEECH_EXPORTS
#define KEEPERSPEECH_API __declspec(dllexport)
#else
#ifdef __GNUC__ 
#define KEEPERSPEECH_API __cdecl
#else
#define KEEPERSPEECH_API __declspec(dllimport)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	KSR_OK = 0,
	KSR_NOT_KNOWN,
	KSR_CREATE_ENGINE,
	KSR_CREATE_RECOG_CONTEXT,
	KSR_SET_NOTIFY,
	KSR_SET_INTEREST,
	KSR_CREATE_GRAMMAR,
	KSR_LOAD_GRAMMAR,
	KSR_NOMOREEVENTS,
	KSR_ACTIVATE_GRAMMAR,
	KSR_NO_LIB_INSTALLED, //returned if import code can't find a KeeperSpeech module
	KSR_ALREADY_INIT,
} KEEPERSPEECH_REASON;

typedef enum
{
	KS_UNUSED = 0,

	//while playing
	KS_VIEW_INFO, //info tab
	KS_VIEW_ROOMS, //room tab
	KS_VIEW_POWERS, //spell tab
	KS_VIEW_TRAPS, //trap tab
	KS_VIEW_CREATURES, //creature tab
	KS_SELECT_ROOM,
	KS_SELECT_POWER,
	KS_SELECT_TRAP,
	KS_SELECT_DOOR,
	KS_SELECT_SELL,
	KS_SELECT_INFO,
	KS_FLEE, //do our creatures flee at low HP?
	KS_PRISONERS, //shall we take prisoners?
	KS_PICKUP, //with hand at current location //TODO: re-enable when I can get a sensible implementation
	KS_DROP, //with hand //TODO: re-enable when I can get a sensible implementation
	KS_SLAP, //with hand //TODO: re-enable when I can get a sensible implementation
	KS_PICKUP_IDLE, //idle creature at any loc
	KS_PICKUP_WORKING, //working creature at any loc
	KS_PICKUP_FIGHTING, //fighting creature at any loc
	KS_PICKUP_ANY,
	KS_CHAT, //open chat
	KS_ESCAPE, //stop whatever we're doing when it makes sense
	KS_MENU, //open menu
	KS_HAND_CHOOSE, //left click equivalent
	KS_HAND_ACTION, //right click equivalent
} KEEPERSPEECH_EVENT_TYPE;

typedef struct
{
	KEEPERSPEECH_EVENT_TYPE type;
	union
	{
		struct
		{
			char model_name[32];
		} creature;

		struct
		{
			int id;
		} room;

		struct
		{
			char model_name[32];
		} power;

		struct
		{
			char model_name[32];
		} trapdoor;
	} u;
} KEEPERSPEECH_EVENT;

/**
	Macro which in addition to declaring a KEEPERSPEECH_API function also declares a function pointer
	with the same name but with "fp" prepended.
*/
#define KEEPERLIBDECLARE(r, x, p) typedef KEEPERSPEECH_API r (*fp##x)p; KEEPERSPEECH_API r x p

KEEPERLIBDECLARE(KEEPERSPEECH_REASON,	KeeperSpeechInit,			(void));
KEEPERLIBDECLARE(const char *,			KeeperSpeechErrorMessage,	(KEEPERSPEECH_REASON reason));
KEEPERLIBDECLARE(void,					KeeperSpeechExit,			(void));
KEEPERLIBDECLARE(KEEPERSPEECH_REASON,	KeeperSpeechPopEvent,		(KEEPERSPEECH_EVENT * ev));
KEEPERLIBDECLARE(void,					KeeperSpeechClearEvents,	(void));

#ifdef __cplusplus
};
#endif

#endif //KEEPERSPEECH_H
