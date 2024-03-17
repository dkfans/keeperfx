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

#ifdef _WIN32
#include <windows.h>
static HMODULE steam_lib;
#endif

#include "keeperfx.hpp"
#include "bflib_fileio.h"
#include "post_inc.h"

typedef int (*SteamApiInitFunc)(char *err);
SteamApiInitFunc SteamAPI_Init;

typedef void (*SteamApiShutdownFunc)();
SteamApiShutdownFunc SteamAPI_Shutdown;

int steam_api_init()
{
#ifndef _WIN32
    // Windows only
    return -1;
#else

    // Make sure the library is not loaded twice
    if (steam_lib != NULL || SteamAPI_Init != NULL)
    {
        return 1;
    }

    // Check if 'steam_api.dll' exists
    if (!LbFileExists("steam_api.dll"))
    {
        return 1;
    }

    JUSTLOG("'steam_api.dll' found");

    // Check if 'steam_appid.txt' exists
    if (!LbFileExists("steam_appid.txt"))
    {
        ERRORLOG("The Steam API requires the 'steam_appid.txt' file to be present");
        return 1;
    }

    // Load the Steam API library
    // This file is included with the Steam SDK download
    try
    {
        steam_lib = LoadLibraryA("steam_api.dll");
        if (!steam_lib)
        {
            ERRORLOG("Unable to load 'steam_api.dll' library");
            return 1;
        }
    }
    catch (...)
    {
        ERRORLOG("Error while loading 'steam_api.dll' library");
        return 1;
    }

    JUSTLOG("'steam_api.dll' library loaded");

    // Get the address of the Init function
    // The 'Flat' version can be used instead of SteamAPI_Init when dynamically linking to the DLL
    FARPROC funcAddressInit = GetProcAddress(steam_lib, "SteamAPI_InitFlat");
    if (funcAddressInit == NULL)
    {
        ERRORLOG("Failed to get proc address for 'SteamAPI_InitFlat' in 'steam_api.dll'");
        FreeLibrary(steam_lib);
        return 1;
    }

    // Get the address of the Shutdown function
    FARPROC funcAddressShutdown = GetProcAddress(steam_lib, "SteamAPI_Shutdown");
    if (funcAddressShutdown == NULL)
    {
        ERRORLOG("Failed to get proc address for 'SteamAPI_Shutdown' in 'steam_api.dll'");
        FreeLibrary(steam_lib);
        return 1;
    }

    // Load the functions so we can use them
    SteamAPI_Init = (SteamApiInitFunc)funcAddressInit;
    SteamAPI_Shutdown = (SteamApiShutdownFunc)funcAddressShutdown;

    // Initialize the Steam API
    // This notifies Steam that we are running the game
    char error[1024];
    int result = SteamAPI_Init(error);

    // SteamAPI_Init return code:
    // - 0 -> OK
    // - 1 -> FailedGeneric: Some other failure
    // - 2 -> NoSteamClient: We cannot connect to Steam, steam probably isn't running
    // - 3 -> VersionMismatch: Steam client appears to be out of date
    if (result != 0)
    {
        JUSTLOG("Steam API Failure: %s", error);
        FreeLibrary(steam_lib);
        return 1;
    }

    FreeLibrary(steam_lib);
    return 0;

#endif
}

void steam_api_shutdown()
{
    if (SteamAPI_Shutdown != NULL)
    {
        SteamAPI_Shutdown();
    }
}