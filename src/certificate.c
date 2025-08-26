#include "certificate.h"

#include "pre_inc.h"

#ifdef _WIN32
#include <windows.h>
#include <wintrust.h>
#include <softpub.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "post_inc.h"

int verify_certificate(const char* path)
{
#ifndef _WIN32
    return 0;
#else

    // Convert char* to wchar_t*
    int char_len = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    wchar_t* wchar_path = (wchar_t*)malloc(char_len * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wchar_path, char_len);
    if(!wchar_path){
        return 0;
    }

    // Setup the certificate verification
    // We'll use the official Windows root cert for this
    WINTRUST_FILE_INFO fileData = {};
    fileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileData.pcwszFilePath = wchar_path;
    fileData.hFile = NULL;
    fileData.pgKnownSubject = NULL;
    GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA winTrustData = {};
    winTrustData.cbStruct = sizeof(WINTRUST_DATA);
    winTrustData.dwUIChoice = WTD_UI_NONE;            // No UI
    winTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; // No revocation checking
    winTrustData.dwUnionChoice = WTD_CHOICE_FILE;     // We are verifying a file
    winTrustData.pFile = &fileData;
    winTrustData.dwStateAction = WTD_STATEACTION_VERIFY; // Start verification
    winTrustData.dwProvFlags = WTD_SAFER_FLAG;        // Use safer flags

    // Do the verification
    LONG verify_status = WinVerifyTrust(NULL, &policyGUID, &winTrustData);

    // Close state data to free resources
    winTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(NULL, &policyGUID, &winTrustData);

    // Free the wchar path
    free(wchar_path);

    // Return if certificate verification was successful
    return (verify_status == ERROR_SUCCESS) ? 1 : 0;

#endif
}

