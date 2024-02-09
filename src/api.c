#include <windows.h>
#include <winbase.h>

#define PIPE_NAME L"\\\\.\\pipe\\KeeperFX-API"

// Global variables
HANDLE hPipe;
HANDLE hThread;

// Function prototypes
DWORD WINAPI PipeThread(LPVOID lpParam);
void ClosePipeThread();
void InitializeServer();

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
        JUSTLOG("Failed to create named pipe. Error code: %d\n", GetLastError());
        exit(1);
    }

    JUSTLOG("Named pipe server listening for clients...\n");

    // Spawn a thread to handle incoming messages
    hThread = CreateThread(NULL, 0, api_server_thread, (LPVOID)hPipe, 0, &dwThreadId);
    if (hThread == NULL)
    {
        JUSTLOG("Failed to create thread. Error code: %d\n", GetLastError());
        CloseHandle(hPipe);
        exit(1);
    }
}

// Thread function to handle incoming messages
DWORD WINAPI api_server_thread(LPVOID lpParam)
{
    HANDLE hPipe = (HANDLE)lpParam;
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
        }
        else
        {
            JUSTLOG("Failed to read from pipe. Error code: %d\n", GetLastError());
            break;
        }
    }

    return 0;
}

// Function to close the pipe thread
void close_api_server()
{
    // Terminate the thread
    TerminateThread(hThread, 0);

    // Close the pipe handle
    CloseHandle(hPipe);

    JUSTLOG("API server closed");
}