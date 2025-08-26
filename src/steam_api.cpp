/**
 * This file implements the Steam API in KeeperFX.
 *
 * Documentation:
 *  https://partner.steamgames.com/doc/sdk/api
 *  https://partner.steamgames.com/doc/api/steam_api
 *
 * The following files need to be present in the KeeperFX directory:
 * - steam_api.dll
 * - steam_appid.txt
 *
 * 'steam_api.dll' is shipped with the official Steamworks SDK download:
 *  https://partner.steamgames.com/downloads/steamworks_sdk.zip
 *
 * 'steam_appid.txt' is a simple text file containing the Steam App ID of Dungeon Keeper Gold.
 * The current App ID is: 1996630
 * You can simply grab it from the URL of the game in the Steam store:
 *  https://store.steampowered.com/app/1996630/Dungeon_Keeper_Gold/
 *
 */
#include "pre_inc.h"
#include "steam_api.hpp"
#include "certificate.h"

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
static HMODULE steam_lib;
#endif

#include "keeperfx.hpp"
#include "bflib_fileio.h"
#include "post_inc.h"

// Result from the dynamically loaded SteamAPI_Init function
enum ESteamAPIInitResult
{
    k_ESteamAPIInitResult_OK = 0,
    k_ESteamAPIInitResult_FailedGeneric = 1,   // Some other failure
    k_ESteamAPIInitResult_NoSteamClient = 2,   // We cannot connect to Steam, steam probably isn't running
    k_ESteamAPIInitResult_VersionMismatch = 3, // Steam client appears to be out of date
};

// Typedefs for the functions and data types in the Steam API
typedef char SteamErrMsg[1024];
typedef ESteamAPIInitResult(__cdecl *SteamApiInitFunc)(SteamErrMsg *err);
typedef void(__cdecl *SteamApiShutdownFunc)();

// Type Punning union for the Steam API Init function to go from __cdecl to __stdcall
union SteamApiInitUnion
{
    FARPROC farProc;
    SteamApiInitFunc steamApiInitFunc;
};

// Variables
SteamApiInitFunc SteamAPI_Init;
SteamApiShutdownFunc SteamAPI_Shutdown;

/**
 * @brief Initializes the Steam API in KeeperFX.
 *
 * This function loads the Steam API library and initializes it.
 * It performs checks for required files and compatibility and verifies the certificate of the dll file.
 */
void steam_api_init()
{
#ifndef _WIN32
    // On anything but Windows we just return because the API is not supposed to get loaded
    return;
#else

    // Make sure the steam API is not initialized multiple times
    if (steam_lib != NULL || SteamAPI_Init != NULL)
    {
        WARNLOG("Steam API already initialized");
        return;
    }

    // Check if both files are present
    if (LbFileExists("steam_api.dll") == false || LbFileExists("steam_appid.txt") == false)
    {

        // If only one of the 2 required files is present, we'll log a message for the user.
        if (
            (LbFileExists("steam_api.dll") == true && LbFileExists("steam_appid.txt") == false) ||
            (LbFileExists("steam_api.dll") == false && LbFileExists("steam_appid.txt") == true))
        {
            ERRORLOG("The Steam API requires both the 'steam_api.dll' and 'steam_appid.txt' files to be present");
        }

        return;
    }

    JUSTLOG("'steam_api.dll' and 'steam_appid.txt' found");

    // Check if we are running KeeperFX under Wine.
    // The .dll can not connect to Steam running on the Host machine so we'll log a notice.
    // Maybe there's cases where a person would also run Steam under Wine.
    // Even if we instead load 'libsteam_api.so' while in a Wine environment,
    // it will be unable to determine a Steam binary running on the Linux host.
    if (is_running_under_wine == true)
    {
        JUSTLOG("The Steam API under Wine will not be able to connect to Steam on the host machine");
    }

    // Verify certificate
    int verify_status = verify_certificate("steam_api.dll");
    if(verify_status == 1){
        JUSTLOG("'steam_api.dll' certificate successfully verified");
    } else if (verify_status == -1){
        JUSTLOG("'steam_api.dll' certificate verification is disabled");
    } else {
        ERRORLOG("Failed to verify certificate of 'steam_api.dll'");
        return;
    }

    // Load the Steam API library
    steam_lib = LoadLibraryA("steam_api.dll");
    if (!steam_lib) {
        ERRORLOG("Unable to load 'steam_api.dll' library");
        return;
    }

    JUSTLOG("'steam_api.dll' library loaded");

    // Get the address of the Init function
    // The 'Flat' version can be used instead of SteamAPI_Init when dynamically linking to the DLL
    SteamApiInitUnion SteamApiInit;
    SteamApiInit.farProc = GetProcAddress(steam_lib, "SteamAPI_InitFlat");
    if (SteamApiInit.farProc == NULL)
    {
        ERRORLOG("Failed to get proc address for 'SteamAPI_InitFlat' in 'steam_api.dll'");
        FreeLibrary(steam_lib);
        return;
    }

    // Unionize the Init function address type to our local function type
    SteamAPI_Init = SteamApiInit.steamApiInitFunc;

    // Get the address of the Shutdown function
    SteamAPI_Shutdown = reinterpret_cast<SteamApiShutdownFunc>(GetProcAddress(steam_lib, "SteamAPI_Shutdown"));
    if (SteamAPI_Shutdown == NULL)
    {
        ERRORLOG("Failed to get proc address for 'SteamAPI_Shutdown' in 'steam_api.dll'");
        FreeLibrary(steam_lib);
        return;
    }

    // Initialize the Steam API
    // This notifies Steam that we are running the game
    SteamErrMsg error;
    ESteamAPIInitResult result = SteamAPI_Init(&error);

    // Check if initialization is successful
    if (result != ESteamAPIInitResult::k_ESteamAPIInitResult_OK)
    {
        JUSTLOG("Steam API Failure: %s", error);
        FreeLibrary(steam_lib);
        return;
    }

    FreeLibrary(steam_lib);

#endif
}

/**
 * @brief Shuts down the Steam API in KeeperFX.
 *
 * This function shuts down the initialized Steam API and unloads the library.
 */
void steam_api_shutdown()
{
#ifdef _WIN32
    if (SteamAPI_Shutdown != NULL)
    {
        SteamAPI_Shutdown();
    }

    SteamAPI_Shutdown = NULL;
    SteamAPI_Init = NULL;

    if (steam_lib != NULL)
    {
        FreeLibrary(steam_lib);
        steam_lib = NULL;
    }
#endif
}