#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include "api.h"
#include "lvl_script.h"
#include "post_inc.h"

#define PIPE_NAME "\\\\.\\pipe\\keeperfx"
#define PIPE_BUFFER_SIZE 1024

// Global variables
HANDLE hPipe;
HANDLE hThread;
DWORD dwThreadId;

// Function prototypes
DWORD WINAPI api_server_thread(LPVOID lpParam);
void close_api_server();
void init_api_server();
void log_last_api_error();

// Initialize API server using a thread and named pipe
void init_api_server()
{
    hThread = CreateThread(NULL, 0, api_server_thread, NULL, 0, &dwThreadId);
    if (hThread == NULL)
    {
        WARNLOG("Failed to create a thread for the API server");
        CloseHandle(hPipe);
        return;
    }
}

// Thread that listens using the named pipe
DWORD WINAPI api_server_thread(LPVOID lpParam)
{
    // CHAR buffer[1024];
    CHAR buffer[PIPE_BUFFER_SIZE];
    DWORD dwRead;
    BOOL bConnected;

    // Create named pipe
    hPipe = CreateNamedPipe(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        PIPE_BUFFER_SIZE,
        PIPE_BUFFER_SIZE,
        NMPWAIT_WAIT_FOREVER,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        log_last_api_error();
        return 1;
    }

    JUSTLOG("API server listening on: %s", PIPE_NAME);

    // Continuously accept client connections and handle data
    while (1)
    {
        // Wait for client to connect
        bConnected = ConnectNamedPipe(hPipe, NULL);
        if (!bConnected)
        {
            // Check if the client disconnected
            DWORD dwError = GetLastError();
            if (dwError == ERROR_PIPE_CONNECTED)
            {
                // Client connected before ConnectNamedPipe was called
                bConnected = true;
            }
            else
            {
                // Handle error
                log_last_api_error();
                // CloseHandle(hPipe);
                // return dwError;
                continue;
            }
        }

        if (bConnected)
        {
            SYNCDBG(1, "Client connected...");

            // Read data from the pipe
            while (ReadFile(hPipe, buffer, sizeof(buffer), &dwRead, NULL) && dwRead > 0)
            {
                // Null-terminate the string
                // This is required because ReadFile does not do this automatically
                buffer[dwRead] = '\0';

                JUSTLOG("Received message from client: %s", buffer);

                // Handle a MAP SCRIPT COMMAND
                script_scan_line(buffer, false);

                // Clear the buffer after we are done with it
                // memset(buffer, 0, sizeof(buffer));

                DWORD dwBytesWritten;
                char response[512] = "Test reply";

                // Write to the pipe
                if (!WriteFile(hPipe, response, sizeof(response), &dwBytesWritten, NULL))
                {
                    log_last_api_error();
                    break;
                }
            }

            // Disconnect the client when ReadFile fails (e.g., client disconnects)
            DisconnectNamedPipe(hPipe);
            SYNCDBG(1, "Client disconnected...");
        }
    }
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

// Log the last API error
void log_last_api_error()
{
    // Get last error
    DWORD errorMessageID = GetLastError();

    // Check if there was an error
    if (errorMessageID == 0)
    {
        return;
    }

    // Create a readable string from the error code
    LPSTR messageBuffer = NULL;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    // Log the error
    if (size != 0)
    {
        WARNLOG("%s(Error Code: %lu)", messageBuffer, errorMessageID);
        LocalFree(messageBuffer);
    }
    else
    {
        WARNLOG("Unknown error (Error Code: %lu)", errorMessageID);
    }
}