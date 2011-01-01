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
#define KEEPERSPEECH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	KS_UNUSED = 0,

	//while playing
	KS_VIEW_INFO, //info tab
	KS_VIEW_ROOMS, //room tab
	KS_VIEW_SPELLS, //spell tab
	KS_VIEW_TRAPS, //trap tab
	KS_CREATURES, //creature tab
	KS_SELECT_ROOM,
	KS_SELECT_SPELL,
	KS_SELECT_TRAP,
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
	} u;
} KEEPERSPEECH_EVENT;

KEEPERSPEECH_API int			KeeperSpeechInit(void);
KEEPERSPEECH_API const char *	KeeperSpeechErrorMessage(int reason);
KEEPERSPEECH_API void			KeeperSpeechExit(void);
KEEPERSPEECH_API int			KeeperSpeechPopEvent(KEEPERSPEECH_EVENT * ev);
KEEPERSPEECH_API void			KeeperSpeechClearEvents(void);

#ifdef __cplusplus
};
#endif

#endif //KEEPERSPEECH_H
