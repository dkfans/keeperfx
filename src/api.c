#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include "api.h"
#include "lvl_script.h"
#include "post_inc.h"

#define PIPE_NAME "\\\\.\\pipe\\KEEPERFX"

// Global variables
HANDLE hPipe;
HANDLE hThread;

// Function prototypes
DWORD WINAPI api_server_thread(LPVOID lpParam);
void close_api_server();
void init_api_server();

// Function to initialize the named pipe server and create the thread
void init_api_server()
{
    DWORD dwThreadId;

    // Create the named pipe
    hPipe = CreateNamedPipe(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        1024,
        1024,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {

        DWORD errorMessageID = GetLastError();
        if (errorMessageID == 0)
            return; // No error message, don't print anything

        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        if (size != 0)
        {
            WARNLOG("Failed to setup named pipe: %s (%lu)\n", messageBuffer, errorMessageID);
            LocalFree(messageBuffer);
        }
        else
        {
            WARNLOG("Failed to setup named pipe: %lu\n", errorMessageID);
        }

        return;
    }

    JUSTLOG("Named API pipe server listening for client...\n");

    // Spawn a thread to handle incoming messages
    hThread = CreateThread(NULL, 0, api_server_thread, (LPVOID)hPipe, 0, &dwThreadId);
    if (hThread == NULL)
    {
        WARNLOG("Failed to create a thread for the API server");
        CloseHandle(hPipe);
        return;
    }
}

// Thread function to handle incoming messages
DWORD WINAPI api_server_thread(LPVOID lpParam)
{
    hPipe = (HANDLE)lpParam;
    BOOL bConnected;
    DWORD dwRead;
    CHAR buffer[1024];

    // Wait for a client to connect
    bConnected = ConnectNamedPipe(hPipe, NULL);
    if (!bConnected)
    {
        JUSTLOG("Failed to connect to client. Error code: %d\n", GetLastError());
        return 1;
    }

    JUSTLOG("Client connected. Listening for messages...\n");

    // Read data from the pipe
    while (1)
    {
        if (ReadFile(hPipe, buffer, sizeof(buffer), &dwRead, NULL))
        {
            JUSTLOG("Received message from client: %s\n", buffer);

            // Execute commands here...
            // script_process_value() ??
            // parse_txt_data(buffer, sizeof(buffer));

            // script_scan_line(buffer, false);
        }
        else
        {
            WARNLOG("Failed to read from pipe. Error code: %d\n", GetLastError());
            break;
        }
    }

    WARNLOG("Named API pipe thread terminating...");

    // Close the pipe handle
    CloseHandle(hPipe);

    return 0;
}

// Function to close the pipe thread
void close_api_server()
{
    // Terminate the thread
    JUSTLOG("Terminating Named API pipe thread...");
    TerminateThread(hThread, 0);

    // Close the pipe handle
    JUSTLOG("Terminating Named API pipe handle...");
    CloseHandle(hPipe);

    JUSTLOG("API server closed");
}