#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <SDL2/SDL_net.h>
#include "api.h"
#include <json.h>
#include <json-dom.h>
#include "config.h"
#include "config_campaigns.h"
#include "lvl_script.h"
#include "lvl_script_lib.h"
#include "lvl_script_value.h"
#include "dungeon_data.h"
#include "player_data.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "console_cmd.h"
#include "post_inc.h"
#include "value_util.h"
#include "keeperfx.hpp"

#define API_SERVER_BUFFER 1024

/**
 * @brief Structure to hold API global variables.
 *
 * This structure defines global variables related to the API, including the server socket,
 * active client socket (only one client at a time), and a socket set for managing sockets.
 */
struct ApiGlobals
{
    TCPsocket serverSocket;     /**< Server socket for API communication. */
    TCPsocket activeSocket;     /**< Active client socket (only one client at a time). */
    SDLNet_SocketSet socketSet; /**< Socket set for managing sockets. */
} api = {0};                    /**< Global instance of the API global variables initialized with zeros. */

/**
 * @brief Structure to hold the state of a dump buffer.
 *
 * This structure holds the state of a dump buffer, which is used by functions
 * for writing JSON data. It includes a pointer to the output buffer and the
 * remaining space available in the buffer.
 */
struct dump_buf_state
{
    char *out;     /**< Pointer to the output buffer. */
    int out_space; /**< Remaining space available in the output buffer. */
};

/**
 * @brief Callback function for writing JSON value dump.
 *
 * This function is a callback used by the JSON library for writing JSON value dump.
 * It copies the JSON data into a buffer, tracking the buffer space available.
 *
 * @param str Pointer to the buffer containing the JSON data.
 * @param size Size of the JSON data in bytes.
 * @param dbs Pointer to the dump buffer state structure.
 *            It holds information about the output buffer and available space.
 *
 * @return 0 on success, -1 if the buffer is too small.
 */
static int json_value_dump_writer(const unsigned char *str, size_t size, void *dbs)
{
    // @author: https://github.com/wolfSSL/wolfsentry/blob/857c85d1b3a6c7b297efa2bbb6ea89817aea7b4b/src/kv.c#L395

    // Check if buffer is too small
    if (size > (size_t)((struct dump_buf_state *)dbs)->out_space)
    {
        JUSTLOG("buffer too small");
        return -1;
    }

    // Copy data into current part of buffer
    memcpy(((struct dump_buf_state *)dbs)->out, str, size);
    ((struct dump_buf_state *)dbs)->out += size;
    ((struct dump_buf_state *)dbs)->out_space -= (int)size;

    return 0;
}

/**
 * @brief Initialize the TCP API server.
 *
 * This function initializes the TCP API server by opening a socket on the specified port.
 * It also initializes SDLNet library and sets up necessary data structures.
 * If the server is already active or the API is not enabled, it does nothing.
 *
 * @return 0 on success, 1 on failure.
 */
int api_init_server()
{
    // Ignore if server is already active
    if (api.serverSocket)
    {
        return 0;
    }

    // Check if API is enabled
    if (api_enabled != true)
    {
        return 0;
    }
    else
    {
        JUSTLOG("API server starting on port: %ld", api_port);
    }

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
    if (SDLNet_ResolveHost(&ip, NULL, api_port) < 0)
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

    JUSTLOG("API server active");

    return 0;
}

/**
 * @brief Send an API event message.
 *
 * This function sends an event message to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 *
 * @param event_name A null-terminated string representing the name of the event to be sent.
 */
void api_event(const char *event_name)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Create the JSON response and send it to the client
    char buf[512];
    int len = snprintf(buf, sizeof(buf) - 1, "{\"event\":\"%s\"}\n", event_name);
    SDLNet_TCP_Send(api.activeSocket, buf, len);
}

/**
 * @brief Send an API error message.
 *
 * This function sends an error message to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 *
 * @param err A null-terminated string representing the error message to be sent.
 */
static void api_err(const char *err)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Create JSON response object
    VALUE json_root_real;
    VALUE *json_root = &json_root_real;
    value_init_dict(json_root);

    // Create success key
    VALUE *val_success = value_dict_add(json_root, "success");
    value_init_bool(val_success, false);

    // Create error key
    VALUE *val_err = value_dict_add(json_root, "error");
    value_init_string(val_err, err);

    // Create JSON response
    char json_string[1024];
    struct dump_buf_state dump_state = {json_string, sizeof(json_string) - 1};
    int json_dump_return_value = json_dom_dump(json_root, json_value_dump_writer, &dump_state, 0, JSON_DOM_DUMP_MINIMIZE);

    // *dump_state.out = 0;
    if (json_dump_return_value != 0)
    {
        api_err("failed to create json response");
        value_fini(json_root);
        return;
    }

    SDLNet_TCP_Send(api.activeSocket, json_string, dump_state.out - json_string);
    value_fini(json_root);
}

/**
 * @brief Send an API success message.
 *
 * This function sends a success message to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 */
static void api_ok()
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // We can simply send back this data as a string without using JSON functions.
    const char msg[] = "{\"success\":true}\n";
    SDLNet_TCP_Send(api.activeSocket, msg, strlen(msg));
}

/**
 * @brief Return data to the API client.
 *
 * This function takes ownership of the provided value and constructs a JSON response
 * indicating the success status along with the provided value.
 *
 * @param success The success status of the operation, true for success, false for failure.
 * @param value The value to be returned to the API client.
 *              Ownership is transferred to this function.
 */
static void api_return_data(TbBool success, VALUE value)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Create JSON response object
    VALUE json_root_real;
    VALUE *json_root = &json_root_real;
    value_init_dict(json_root);

    // Create success key
    VALUE *val_success = value_dict_add(json_root, "success");
    value_init_bool(val_success, success);

    // Create data key
    VALUE *val_data = value_dict_add(json_root, "data");
    *val_data = value;

    // Create JSON response
    char json_string[1024];
    struct dump_buf_state dump_state = {json_string, sizeof(json_string) - 1};
    int json_dump_return_value = json_dom_dump(json_root, json_value_dump_writer, &dump_state, 0, JSON_DOM_DUMP_MINIMIZE);

    // *dump_state.out = 0;
    if (json_dump_return_value != 0)
    {
        api_err("failed to create json response");
        value_fini(json_root);
        return;
    }

    SDLNet_TCP_Send(api.activeSocket, json_string, dump_state.out - json_string);
    value_fini(json_root);
}

/**
 * @brief Send a string data response to the API client.
 *
 * This function sends a string data response to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 *
 * @param data The string data to be sent to the API client.
 */
static void api_return_data_string(const char *data)
{
    // Do nothing if the API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Create value to send back
    VALUE dataValue, *value = &dataValue;
    value_init_string(value, data);

    // Send the data
    api_return_data(true, dataValue);
}

/**
 * @brief Send a long integer data response to the API client.
 *
 * This is useful for sending numeric values.
 *
 * This function sends a long integer data response to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 *
 * @param data The long integer data to be sent to the API client.
 */
static void api_return_data_number(long data)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Send back the JSON as a string. A number should never be able to break the syntax.
    char buf[256];
    int len = snprintf(buf, sizeof(buf) - 1, "{\"success\":true,\"data\":%ld}\n", data);
    SDLNet_TCP_Send(api.activeSocket, buf, len);
}

/**
 * @brief Process the incoming buffer from the API client.
 *
 * This function processes the incoming buffer from the API client, parsing the JSON object
 * and executing the corresponding action. It handles various actions such as map commands,
 * console commands, getting player flags, reading and setting variables, and retrieving
 * level information.
 *
 * @param buffer The buffer containing the JSON data sent by the client.
 * @param buf_size The size of the buffer.
 */
static void api_process_buffer(const char *buffer, size_t buf_size)
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
        player_id = get_id(player_desc, (char *)value_string(player));
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

        // If the console-prefix-character is at the start of the string we'll ignore that char
        if (console_command[0] == cmd_char)
        {
            console_command += 1;
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

    // Handle get all player flags command
    if (strcasecmp("get_all_player_flags", action) == 0)
    {

        // Create flag data to return to client
        VALUE flag_data_real;
        VALUE *flag_data = &flag_data_real;
        value_init_dict(flag_data);

        for (int player_index = 0; player_index < ALL_PLAYERS; player_index++)
        {
            char *player_string[32];

            // Create the player string for this player
            if (player_index == 4)
            {
                strcpy(player_string, "PLAYER_GOOD");
            }
            else if (player_index == 5)
            {
                strcpy(player_string, "PLAYER_NEUTRAL");
            }
            else
            {

                // After PLAYER_GOOD and PLAYER_NEUTRAL we have to move the number back to PLAYER4
                int player_string_number = player_index;
                if (player_index > 5)
                {
                    player_string_number -= 2;
                }

                // Create string
                char *player_new_string[8];
                snprintf(player_new_string, sizeof(player_new_string), "PLAYER%d", player_string_number);
                strcpy(player_string, player_new_string);
            }

            // Create object for this player
            VALUE *player_info = value_dict_add(flag_data, player_string);
            value_init_dict(player_info);

            for (int flag_index = 0; flag_index < 8; flag_index++)
            {
                // Get flag value
                long flag_value = get_condition_value(player_id, SVar_FLAG, flag_index);

                // Add flag to player flag
                char *flag_string[6];
                snprintf(flag_string, sizeof(flag_string), "FLAG%d", flag_index);
                value_init_int32(value_dict_add(player_info, flag_string), flag_value);
            }
        }

        // Return data to client
        api_return_data(true, flag_data_real);

        // End
        value_fini(&data);
        return;
    }

    // Handle read var command
    if (strcasecmp("read_var", action) == 0)
    {
        // Get variable name
        char *variable_name = (char *)value_string(value_dict_get(value, "var"));
        if (variable_name == NULL || strlen(variable_name) < 1)
        {
            api_err("a 'var' must be given when using the 'read_var' action");
            value_fini(&data);
            return;
        }

        // Recognize variable
        long variable_id, variable_type;
        if (parse_get_varib(variable_name, &variable_id, &variable_type) == false)
        {
            api_err("unknown variable");
            value_fini(&data);
            return;
        }

        // Get the variable
        long variable_value = get_condition_value(player_id, variable_type, variable_id);

        // Return the variable to the user
        api_return_data_number(variable_value);

        // End
        value_fini(&data);
        return;
    }

    // Handle set var command
    if (strcasecmp("set_var", action) == 0)
    {
        // Get variable name
        char *variable_name = (char *)value_string(value_dict_get(value, "var"));
        if (variable_name == NULL || strlen(variable_name) < 1)
        {
            api_err("a 'var' must be given when using the 'set_var' action");
            value_fini(&data);
            return;
        }

        // Recognize variable
        long variable_id, variable_type;
        if (parse_get_varib(variable_name, &variable_id, &variable_type) == false)
        {
            api_err("unknown variable");
            value_fini(&data);
            return;
        }

        // Check if this type of variable can be set dynamically
        if (
            variable_type != SVar_FLAG &&
            variable_type != SVar_CAMPAIGN_FLAG &&
            variable_type != SVar_BOX_ACTIVATED &&
            variable_type != SVar_SACRIFICED &&
            variable_type != SVar_REWARDED)
        {
            api_err("setting this variable is not possible");
            value_fini(&data);
            return;
        }

        // Get the new value
        VALUE *new_value = value_dict_get(value, "value");
        if (new_value == NULL || value_type(new_value) != VALUE_INT32)
        {
            api_err("a 'value' of type int must be given when using the 'set_var' action");
            value_fini(&data);
            return;
        }

        // Set the variable
        set_variable(player_id, variable_type, variable_id, value_int32(new_value));

        // Return success
        api_ok();

        // End
        value_fini(&data);
        return;
    }

    // Handle get level info command
    if (strcasecmp("get_level_info", action) == 0 || strcasecmp("get_map_info", action) == 0)
    {
        // Get level info and name
        const char *lv_name = NULL;
        LevelNumber lv_number = get_loaded_level_number();
        struct LevelInformation *lv_info = get_level_info(lv_number);
        if (lv_info != NULL)
        {
            if (lv_info->name_stridx > 0)
            {
                lv_name = get_string(lv_info->name_stridx);
            }
            else
            {
                lv_name = lv_info->name;
            }
        }
        else if (is_multiplayer_level(lv_number))
        {
            lv_name = (const char *)level_name;
        }

        // Create level data to return to client
        VALUE data_level_info_real;
        VALUE *data_level_info = &data_level_info_real;
        value_init_dict(data_level_info);

        // Add stuff to level data
        value_init_string(value_dict_add(data_level_info, "level_name"), lv_name);
        value_init_int32(value_dict_add(data_level_info, "level_number"), lv_number);
        value_init_int32(value_dict_add(data_level_info, "players"), lv_info->players);
        value_init_int32(value_dict_add(data_level_info, "mapsize_x"), lv_info->mapsize_x);
        value_init_int32(value_dict_add(data_level_info, "mapsize_y"), lv_info->mapsize_y);
        value_init_bool(value_dict_add(data_level_info, "is_multiplayer"), is_multiplayer_level(lv_number));

        // Create campaign data and add to level data
        VALUE *data_campaign_info = value_dict_add(data_level_info, "campaign");
        value_init_dict(data_campaign_info);

        // Add stuff to campaign data
        value_init_string(value_dict_add(data_campaign_info, "campaign_name"), campaign.name);
        value_init_string(value_dict_add(data_campaign_info, "campaign_display_name"), campaign.display_name);
        value_init_string(value_dict_add(data_campaign_info, "campaign_fname"), campaign.fname);
        value_init_bool(value_dict_add(data_campaign_info, "is_map_pack"), is_map_pack());

        // Return data to client
        api_return_data(true, data_level_info_real);

        // End
        value_fini(&data);
        return;
    }

    // Handle get level info command
    if (strcasecmp("get_current_game_info", action) == 0)
    {
        // Create level data to return to client
        VALUE data_current_game_info_real;
        VALUE *data_current_game_info = &data_current_game_info_real;
        value_init_dict(data_current_game_info);

        // Add stuff to level data
        value_init_int32(value_dict_add(data_current_game_info, "game_turn"), get_gameturn());

        // Return data to client
        api_return_data(true, data_current_game_info_real);

        // End
        value_fini(&data);
        return;
    }

    // Handle get level info command
    if (strcasecmp("get_kfx_info", action) == 0)
    {
        // Create level data to return to client
        VALUE data_kfx_info_real;
        VALUE *data_kfx_info = &data_kfx_info_real;
        value_init_dict(data_kfx_info);

        // Add stuff to level data
        value_init_string(value_dict_add(data_kfx_info, "kfx_version"), VER_STRING);

        // Return data to client
        api_return_data(true, data_kfx_info_real);

        // End
        value_fini(&data);
        return;
    }

    // Return unknown action
    api_err("unknown action");
    value_fini(&data);
}

/**
 * @brief Update the API server and handle all pending packets.
 *
 * This function updates the API server by checking for incoming connections and messages.
 * It accepts new client connections, processes incoming messages, and handles disconnections.
 */
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
                    api_process_buffer(buffer, received);
                }
                else
                {
                    WARNLOG("API connection closed");
                    SDLNet_TCP_DelSocket(api.socketSet, api.activeSocket);
                    SDLNet_TCP_Close(api.activeSocket);
                    api.activeSocket = 0;
                }

                // Clear buffer
                memset(buffer, 0, API_SERVER_BUFFER);

            } // \activeSocket
        }
    } while (numReady > 0); // To have break instead of goto
}

/**
 * @brief Close the API server.
 *
 * This function stops the API server by closing the server socket and active client socket,
 * and frees the socket set. It also shuts down the SDLNet library.
 */
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
