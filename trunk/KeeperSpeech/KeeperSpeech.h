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

KEEPERSPEECH_API int			KeeperSpeechInit(void);
KEEPERSPEECH_API const char *	KeeperSpeechErrorMessage(int reason);
KEEPERSPEECH_API void			KeeperSpeechExit(void);

#ifdef __cplusplus
};
#endif