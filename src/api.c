#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <SDL2/SDL_net.h>
#include "api.h"
#include "lvl_script.h"
#include "post_inc.h"

#define API_SERVER_PORT 5599
#define API_SERVER_BUFFER 1024

struct ApiGlobals
{
    TCPsocket serverSocket;
    TCPsocket activeSocket; // Only one client each time
    SDLNet_SocketSet socketSet;
} api;

// Initialize the TCP API server
int api_init_server()
{
    if (SDLNet_Init() < 0)
    {
        JUSTLOG("SDLNet could not initialize! SDLNet_Error: %s", SDLNet_GetError());
        return 1;
    }

    memset(&api, 0, sizeof(api));

    api.socketSet = SDLNet_AllocSocketSet(1);
    if (!api.socketSet)
    {
        JUSTLOG("SDLNet_AllocSocketSet failed! SDLNet_Error: %s", SDLNet_GetError());
        api_close_server();
        return 1;
    }

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, NULL, API_SERVER_PORT) < 0)
    {
        JUSTLOG("SDLNet_ResolveHost failed! SDLNet_Error: %s", SDLNet_GetError());
        api_close_server();
        return 1;
    }

    api.serverSocket = SDLNet_TCP_Open(&ip);
    if (!api.serverSocket)
    {
        JUSTLOG("SDLNet_TCP_Open failed! SDLNet_Error: %s", SDLNet_GetError());
        api_close_server();
        return 1;
    }

    if (SDLNet_TCP_AddSocket(api.socketSet, api.serverSocket) == -1)
    {
        JUSTLOG("SDLNet_TCP_AddSocket failed! SDLNet_Error: %s", SDLNet_GetError());
        api_close_server();
        return 1;
    }

    return 0;
}

void api_update_server()
{
    if (api.serverSocket == 0) // It seems we don't wait anyone
        return;

    char buffer[API_SERVER_BUFFER];
    memset(buffer, 0, API_SERVER_BUFFER);

    JUSTLOG("API Server started. Waiting for connections...");

    int numReady;
    do
    {
        numReady = SDLNet_CheckSockets(api.socketSet, 0);
        if (numReady < 0)
        {
            JUSTLOG("SDLNet_CheckSockets failed! SDLNet_Error: %s", SDLNet_GetError());
            break;
        }

        if (numReady > 0)
        {
            if (SDLNet_SocketReady(api.serverSocket))
            {
                if (api.activeSocket != 0)
                {
                    TCPsocket tmp = SDLNet_TCP_Accept(api.serverSocket);
                    SDLNet_TCP_Close(tmp);
                    WARNLOG("Got another connection while Api connection is still active");
                }
                else
                {
                    api.activeSocket = SDLNet_TCP_Accept(api.serverSocket);
                    if (!api.activeSocket)
                    {
                        continue;
                    }
                    JUSTLOG("Client connected");
                    if (SDLNet_TCP_AddSocket(api.socketSet, api.activeSocket) == -1)
                    {
                        JUSTLOG("SDLNet_TCP_AddSocket failed! SDLNet_Error: %s", SDLNet_GetError());
                        SDLNet_TCP_Close(api.activeSocket);
                        api.activeSocket = 0;
                        continue;
                    }
                }
            } // \serverSocket
            if (SDLNet_SocketReady(api.activeSocket))
            {
                int received = SDLNet_TCP_Recv(api.activeSocket, buffer, API_SERVER_BUFFER);
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

                    SDLNet_TCP_Send(api.activeSocket, buffer, received); // Echo back
                }
                else
                {
                    WARNLOG("Api connection closed");
                    SDLNet_TCP_DelSocket(api.socketSet, api.activeSocket);
                    SDLNet_TCP_Close(api.activeSocket);
                    api.activeSocket = 0;
                }
            } // \activeSocket
        }
    } while (numReady > 0); // To have break instead of goto
}

void api_close_server()
{
    JUSTLOG("API server stopped listening to messages");
    if (api.socketSet)
    {
        SDLNet_FreeSocketSet(api.socketSet);
        api.socketSet = 0;
    }

    if (api.activeSocket)
    {
        SDLNet_TCP_Close(api.activeSocket);
        api.activeSocket = 0;
    }

    if (api.serverSocket)
    {
        SDLNet_TCP_Close(api.serverSocket);
        api.serverSocket = 0;
    }

    SDLNet_Quit();
}