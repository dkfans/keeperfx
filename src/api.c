#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <SDL2/SDL_net.h>
#include "api.h"
#include "lvl_script.h"
#include "post_inc.h"

#define API_SERVER_PORT 5599
#define API_SERVER_BUFFER 1024

// Thread vars
DWORD dwThreadId;
HANDLE hThread;

// Function prototypes
DWORD WINAPI api_server_thread(LPVOID lpParam);
void close_api_server();
void init_api_server();

// Initialize API server using a thread and named pipe
void init_api_server()
{
    // Create the API thread
    hThread = CreateThread(NULL, 0, api_server_thread, NULL, 0, &dwThreadId);
    if (hThread == NULL)
    {
        ERRORLOG("Failed to create a thread for the API server");
        return;
    }
}

// Thread that listens using the named pipe
DWORD WINAPI api_server_thread(LPVOID lpParam)
{

    if (SDLNet_Init() < 0)
    {
        JUSTLOG("SDLNet could not initialize! SDLNet_Error: %s", SDLNet_GetError());
        return 1;
    }

    SDLNet_SocketSet serverSocketSet = SDLNet_AllocSocketSet(1);
    if (!serverSocketSet)
    {
        JUSTLOG("SDLNet_AllocSocketSet failed! SDLNet_Error: %s", SDLNet_GetError());
        return 1;
    }

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, NULL, API_SERVER_PORT) < 0)
    {
        JUSTLOG("SDLNet_ResolveHost failed! SDLNet_Error: %s", SDLNet_GetError());
        return 1;
    }

    TCPsocket serverSocket = SDLNet_TCP_Open(&ip);
    if (!serverSocket)
    {
        JUSTLOG("SDLNet_TCP_Open failed! SDLNet_Error: %s", SDLNet_GetError());
        return 1;
    }

    if (SDLNet_TCP_AddSocket(serverSocketSet, serverSocket) == -1)
    {
        JUSTLOG("SDLNet_TCP_AddSocket failed! SDLNet_Error: %s", SDLNet_GetError());
        return 1;
    }

    char buffer[API_SERVER_BUFFER];
    memset(buffer, 0, API_SERVER_BUFFER);

    JUSTLOG("API Server started. Waiting for connections...");

    while (1)
    {
        int numReady = SDLNet_CheckSockets(serverSocketSet, -1);

        JUSTLOG("numReady %d", numReady);

        if (numReady < 0)
        {
            JUSTLOG("SDLNet_CheckSockets failed! SDLNet_Error: %s", SDLNet_GetError());
            break;
        }

        if (numReady > 0)
        {
            if (SDLNet_SocketReady(serverSocket))
            {
                TCPsocket clientSocket = SDLNet_TCP_Accept(serverSocket);
                if (clientSocket)
                {
                    JUSTLOG("Client connected");

                    while (1)
                    {
                        int received = SDLNet_TCP_Recv(clientSocket, buffer, API_SERVER_BUFFER);
                        if (received > 0)
                        {
                            // Remove any possible trailing newline from the data
                            // This makes it work with a Telnet connection as well
                            if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
                            {
                                buffer[strlen(buffer) - 1] = '\0';
                            }

                            JUSTLOG("Received message from client: %s", buffer);

                            // Run the received data as a MAP SCRIPT COMMAND
                            script_scan_line(buffer, false);
                        }

                        SDLNet_TCP_Send(clientSocket, buffer, received); // Echo back
                    }
                }
            }
        }

        SDL_Delay(100); // Adjust as needed for performance
    }

    JUSTLOG("API server stopped listening to messages");

    SDLNet_FreeSocketSet(serverSocketSet);
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}

void close_api_server()
{
    // Terminate the thread
    JUSTLOG("Terminating API server...");
    TerminateThread(hThread, 0);
}