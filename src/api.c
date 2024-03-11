#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <SDL2/SDL_net.h>
#include "api.h"
#include <json.h>
#include <json-dom.h>
#include "lvl_script.h"
#include "player_data.h"
#include "console_cmd.h"
#include "post_inc.h"
#include "value_util.h"

#define API_SERVER_PORT 5599
#define API_SERVER_BUFFER 1024
#define API_CONFIG_KEY "api"

struct ApiGlobals
{
    TCPsocket serverSocket;
    TCPsocket activeSocket; // Only one client each time
    SDLNet_SocketSet socketSet;
} api = {0};

// Initialize the TCP API server
int api_init_server()
{
    if (api.serverSocket) // Already inited
        return 0;

    // TODO: where is keeperfx.cfg dict????

    //    if (value_dict_get(global_config, API_CONFIG_KEY) == NULL)
    //        return 0;

    if (SDLNet_Init() < 0)
    {
        JUSTLOG("SDLNet could not initialize! SDLNet_Error: %s", SDLNet_GetError());
        return 1;
    }

    memset(&api, 0, sizeof(api));

    api.socketSet = SDLNet_AllocSocketSet(2);
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

void api_event(const char *eventName)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Create the JSON response and send it to the client
    char buf[512];
    int len = snprintf(buf, sizeof(buf) - 1, "{\"event\":\"%s\"}", eventName);
    SDLNet_TCP_Send(api.activeSocket, buf, len);
}

static void api_err(const char *err)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    char buf[256];
    int len = snprintf(buf, sizeof(buf) - 1, "{\"success\":false,\"error\":\"%s\"}", err);
    SDLNet_TCP_Send(api.activeSocket, buf, len);
}

static void api_ok()
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    const char msg[] = "{\"success\":true}";
    SDLNet_TCP_Send(api.activeSocket, msg, strlen(msg));
}

static void process_buffer(const char *buffer, size_t buf_size)
{
    VALUE data, *value = &data;

    // Handle closing null byte
    if (buffer[buf_size - 1] == 0)
    {
        buf_size -= 1;
    }

    // Check if something is actually sent
    if (strlen(buffer) < 1)
    {
        api_err("no json sent");
        return;
    }

    // Decode the json object
    int ret = json_dom_parse(buffer, buf_size, NULL, 0, value, NULL);
    if (ret != 0)
    {
        api_err("invalid json");
        return;
    }

    // Make sure we have a json object
    if (value_type(value) != VALUE_DICT)
    {
        api_err("invalid json object");
        value_fini(&data);
        return;
    }

    // Get the action the user wants to do
    const char *action = value_string(value_dict_get(value, "action"));
    if (action == NULL)
    {
        api_err("an 'action' must be given");
        value_fini(&data);
        return;
    }

    // Get player for the action (Default is current player)
    PlayerNumber player_id = my_player_number;
    VALUE *player = value_dict_get(value, "player");
    if (value_type(player) == VALUE_INT32)
    {
        player_id = (PlayerNumber)value_int32(player);
    }
    else if (value_type(player) == VALUE_STRING)
    {
        player_id = get_rid(player_desc, (char *)value_string(player));
    }

    // Handle map command
    if (strcasecmp("map_command", action) == 0)
    {
        // Get map command
        char *map_command = (char *)value_string(value_dict_get(value, "command"));
        if (map_command == NULL)
        {
            api_err("a 'command' must be given when using the 'map_command' action");
            value_fini(&data);
            return;
        }

        // Execute map command
        if (script_scan_line(map_command, false, 99)) // Maximum level of a command support
        {
            api_ok();
        }
        else
        {
            api_err("failed to execute map command");
        }

        // End
        value_fini(&data);
        return;
    }

    // Handle console command
    if (strcasecmp("console_command", action) == 0)
    {
        // Get console command
        char *console_command = (char *)value_string(value_dict_get(value, "command"));
        if (console_command == NULL || strlen(console_command) < 1)
        {
            api_err("a 'command' must be given when using the 'console_command' action");
            value_fini(&data);
            return;
        }

        // If exclamation mark at start of string for 'cmd_exec'
        if (console_command[0] == '!')
        {
            console_command += 1;// skip the exclamation mark
        }

        // Execute console command
        if (cmd_exec(player_id, console_command))
        {
            api_ok();
        }
        else
        {
            api_err("failed to execute console command");
        }

        // End
        value_fini(&data);
        return;
    }

    // Return unknown action
    api_err("unknown action");
    value_fini(&data);
}

void api_update_server()
{
    // Return if the TCP server is not listening
    if (api.serverSocket == 0)
    {
        return;
    }

    char buffer[API_SERVER_BUFFER];
    memset(buffer, 0, API_SERVER_BUFFER);

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
                    WARNLOG("Got another connection while API connection is still active");
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
                    process_buffer(buffer, received);
                }
                else
                {
                    WARNLOG("API connection closed");
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
