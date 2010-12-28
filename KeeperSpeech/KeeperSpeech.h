// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KEEPERSPEECH_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KEEPERSPEECH_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef KEEPERSPEECH_EXPORTS
#define KEEPERSPEECH_API __declspec(dllexport)
#else
#define KEEPERSPEECH_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	enum
	{
		//while playing
		KEEPERSPEECH_INFO, //info tab
		KEEPERSPEECH_ROOMS, //room tab
		KEEPERSPEECH_SPELLS, //spell tab
		KEEPERSPEECH_TRAPS, //trap tab
		KEEPERSPEECH_CREATURES, //creature tab
		KEEPERSPEECH_SELECT_ROOM,
		KEEPERSPEECH_SELECT_SPELL,
		KEEPERSPEECH_SELECT_TRAP,
		KEEPERSPEECH_SELECT_SELL,
		KEEPERSPEECH_SELECT_INFO,
		KEEPERSPEECH_FLEE, //do our creatures flee at low HP?
		KEEPERSPEECH_PRISONERS, //shall we take prisoners?
		KEEPERSPEECH_PICKUP, //with hand at current location
		KEEPERSPEECH_DROP, //with hand
		KEEPERSPEECH_SLAP, //with hand
		KEEPERSPEECH_PICKUP_IDLE, //idle creature at any loc
		KEEPERSPEECH_PICKUP_WORKING, //working creature at any loc
		KEEPERSPEECH_PICKUP_FIGHTING, //fighting creature at any loc
		KEEPERSPEECH_PICKUP_ANY, //any creature at any loc
		KEEPERSPEECH_CHAT, //open chat
		KEEPERSPEECH_ESCAPE, //stop whatever we're doing when it makes sense
		KEEPERSPEECH_MENU, //open menu
	} type;

	int param1;
} KEEPERSPEECH_EVENT;

KEEPERSPEECH_API int			KeeperSpeechInit(void);
KEEPERSPEECH_API const char *	KeeperSpeechErrorMessage(int reason);
KEEPERSPEECH_API void			KeeperSpeechExit(void);
KEEPERSPEECH_API int			KeeperSpeechPopEvent(KEEPERSPEECH_EVENT * ev);

#ifdef __cplusplus
};
#endif