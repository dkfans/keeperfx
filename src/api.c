#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <SDL2/SDL_net.h>
#include "api.h"
#include <json.h>
#include <json-dom.h>
#include "config.h"
#include "config_campaigns.h"
#include "lvl_script.h"
#include "lvl_script_commands.h"
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

#define API_SERVER_BUFFER 4096

#define API_SUBSCRIBE_LIST_SIZE 256

#define API_SUBSCRIBE_INACTIVE 0
#define API_SUBSCRIBE_EVENT 1
#define API_SUBSCRIBE_VAR 2

/**
 * Structure to hold API global variables.
 *
 * This structure defines global variables related to the API, including the server socket,
 * active client socket (only one client at a time), and a socket set for managing sockets.
 */
struct ApiGlobals
{
    TCPsocket serverSocket;     // Server socket for API communication
    TCPsocket activeSocket;     // Active client socket (only one client at a time)
    SDLNet_SocketSet socketSet; // Socket set for managing sockets
} api = {0};                    // Global instance of the API global variables initialized with zeros

/**
 * Structure representing a subscribed variable.
 *
 * This structure holds information about a variable subscribed by a client, including
 * the player ID, type, and ID of the variable.
 */
struct SubscribedVariable
{
    PlayerNumber player_id;
    char name[COMMAND_WORD_LEN];
    unsigned char type;
    unsigned char id;
    long val;
};

/**
 * Structure representing a subscription slot.
 *
 * This 'slot' can contain either a SubscribedVariable or an event.
 * The type is used to determine which type of subscription is held.
 * It's also possible to have an inactive subscription.
 */
struct Subscription
{
    struct SubscribedVariable var;
    char event[COMMAND_WORD_LEN];
    int type;
} api_subscriptions[API_SUBSCRIBE_LIST_SIZE];

/**
 * Counter for the amount of active subscriptions.
 *
 * We use an int for this so we can stop checking the list of
 * subscription slots when we are sure there's no more subscriptions left.
 * This is done for performance reasons.
 */
int api_sub_count = 0;

/**
 * Structure to hold the state of a dump buffer.
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
 * Callback function for writing JSON value dump.
 *
 * This function is a callback used by the JSON library for writing JSON value dump.
 * It copies the JSON data into a buffer, tracking the buffer space available.
 *
 * @param str Pointer to the buffer containing the JSON data.
 * @param size Size of the JSON data in bytes.
 * @param dbs Pointer to the dump buffer state structure.
 *            It holds information about the output buffer and available space.
 *
 * @return 0 on success, JSON_ERR_OUTOFMEMORY (-2) if the buffer is too small.
 */
static int json_value_dump_writer(const char *str, size_t size, void *dbs)
{
    // @author: https://github.com/wolfSSL/wolfsentry/blob/857c85d1b3a6c7b297efa2bbb6ea89817aea7b4b/src/kv.c#L395

    // Check if buffer is too small
    if (size > (size_t)((struct dump_buf_state *)dbs)->out_space)
    {
        JUSTLOG("buffer too small");
        return JSON_ERR_OUTOFMEMORY;
    }

    // Copy data into current part of buffer
    memcpy(((struct dump_buf_state *)dbs)->out, str, size);
    ((struct dump_buf_state *)dbs)->out += size;
    ((struct dump_buf_state *)dbs)->out_space -= (int)size;

    return 0;
}

/**
 * Function to get the number of max available KeeperFX flags with a name
 *
 * @return size_t Amount of flags
 */
size_t get_max_flags()
{
    size_t num = 0;
    while (flag_desc[num].name != NULL)
    {
        num++;
    }
    return num;
}

/**
 * Initialize the TCP API server.
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
        JUSTLOG("API server starting on port: %u", api_port);
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

    // Initialize all subscription slots
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; ++i)
    {
        api_subscriptions[i].type = API_SUBSCRIBE_INACTIVE;
    }

    JUSTLOG("Allocated %d API subscription slots", API_SUBSCRIBE_LIST_SIZE);

    return 0;
}

/**
 * Send an API error message.
 *
 * This function sends an error message to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 *
 * @param err A null-terminated string representing the error message to be sent.
 */
static void api_err(const char *err, VALUE *ack_id)
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

    // Add ack ID
    if (ack_id != NULL)
    {
        VALUE *val_ack = value_dict_add(json_root, "ack");
        *val_ack = *ack_id;
    }

    // Create success key
    VALUE *val_success = value_dict_add(json_root, "success");
    value_init_bool(val_success, false);

    // Create error key
    VALUE *val_err = value_dict_add(json_root, "error");
    value_init_string(val_err, (char *)err);

    // Create JSON response
    char json_string[1024];
    struct dump_buf_state dump_state = {json_string, sizeof(json_string) - 1};
    int json_dump_return_value = json_dom_dump(json_root, json_value_dump_writer, &dump_state, 0, JSON_DOM_DUMP_MINIMIZE);

    *dump_state.out = 0;
    if (json_dump_return_value != 0)
    {
        value_fini(json_root);
        return;
    }

    // Add newline to end of data
    dump_state.out[0] = '\n';
    dump_state.out++;

    // Send data to client
    SDLNet_TCP_Send(api.activeSocket, json_string, dump_state.out - json_string);
    value_fini(json_root);
}

/**
 * Send an API success message.
 *
 * This function sends a success message to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 */
static void api_ok(VALUE *ack_id)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Check if we can send a success message without an ack
    if (ack_id == NULL)
    {
        // We can send it directly without using JSON functions here
        const char msg[] = "{\"success\":true}\n";
        SDLNet_TCP_Send(api.activeSocket, msg, strlen(msg));
        return;
    }

    // Create JSON response object
    VALUE json_root_real;
    VALUE *json_root = &json_root_real;
    value_init_dict(json_root);

    // Add ack
    VALUE *val_ack = value_dict_add(json_root, "ack");
    *val_ack = *ack_id;

    // Create success key
    VALUE *val_success = value_dict_add(json_root, "success");
    value_init_bool(val_success, true);

    // Create JSON response
    char json_string[1024];
    struct dump_buf_state dump_state = {json_string, sizeof(json_string) - 1};
    int json_dump_return_value = json_dom_dump(json_root, json_value_dump_writer, &dump_state, 0, JSON_DOM_DUMP_MINIMIZE);

    *dump_state.out = 0;
    if (json_dump_return_value != 0)
    {
        value_fini(json_root);
        return;
    }

    // Add newline to end of data
    dump_state.out[0] = '\n';
    dump_state.out++;

    // Send data to client
    SDLNet_TCP_Send(api.activeSocket, json_string, dump_state.out - json_string);
    value_fini(json_root);
}

/**
 * Return data to the API client.
 *
 * This function takes ownership of the provided value and constructs a JSON response
 * indicating the success status along with the provided value.
 *
 * @param success The success status of the operation, true for success, false for failure.
 * @param value The value to be returned to the API client.
 *              Ownership is transferred to this function.
 */
static void api_return_data(TbBool success, VALUE value, VALUE *ack_id)
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

    // Add ack ID
    if (ack_id != NULL)
    {
        VALUE *val_ack = value_dict_add(json_root, "ack");
        *val_ack = *ack_id;
    }

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

    *dump_state.out = 0;
    if (json_dump_return_value != 0)
    {
        api_err("FAILED_TO_CREATE_JSON", ack_id);
        value_fini(json_root);
        return;
    }

    // Add newline to end of data
    dump_state.out[0] = '\n';
    dump_state.out++;

    // Send data to client
    SDLNet_TCP_Send(api.activeSocket, json_string, dump_state.out - json_string);
    value_fini(json_root);
}

/**
 * Send a string data response to the API client.
 *
 * This function sends a string data response to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 *
 * @param data The string data to be sent to the API client.
 */
// static void api_return_data_string(const char *data)
// {
//     // Do nothing if the API server is not active
//     if (!api.activeSocket)
//     {
//         return;
//     }

//     // Create value to send back
//     VALUE dataValue, *value = &dataValue;
//     value_init_string(value, data);

//     // Send the data
//     api_return_data(true, dataValue);
// }

void api_return_var_update(PlayerNumber plyr_idx, const char *var_name, long value)
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

    // Create event key
    VALUE *val_event = value_dict_add(json_root, "event");
    value_init_string(val_event, "VAR_UPDATE");

    // Create var change object
    VALUE *val_var = value_dict_add(json_root, "var");
    value_init_dict(val_var);

    // Add player string
    VALUE *val_var_player = value_dict_add(val_var, "player");
    value_init_string(val_var_player, player_code_name(plyr_idx));

    // Add variable name
    VALUE *val_var_name = value_dict_add(val_var, "name");
    value_init_string(val_var_name, var_name);

    // Add the new value
    VALUE *val_var_new_val = value_dict_add(val_var, "value");
    value_init_int32(val_var_new_val, value);

    // Create JSON response
    char json_string[1024];
    struct dump_buf_state dump_state = {json_string, sizeof(json_string) - 1};
    int json_dump_return_value = json_dom_dump(json_root, json_value_dump_writer, &dump_state, 0, JSON_DOM_DUMP_MINIMIZE);

    *dump_state.out = 0;
    if (json_dump_return_value != 0)
    {
        value_fini(json_root);
        return;
    }

    // Add newline to end of data
    dump_state.out[0] = '\n';
    dump_state.out++;

    // Send data to client
    SDLNet_TCP_Send(api.activeSocket, json_string, dump_state.out - json_string);
    value_fini(json_root);
}

/**
 * Send a long integer data response to the API client.
 *
 * This is useful for sending numeric values.
 *
 * This function sends a long integer data response to the API client over the active socket.
 * If the API server is not active, this function does nothing.
 *
 * @param data The long integer data to be sent to the API client.
 */
static void api_return_data_number(long data, VALUE *ack_id)
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Check if we can send a success message without an ack
    if (ack_id == NULL)
    {
        // Send back the JSON as a string. A number should never be able to break the syntax.
        char buf[256];
        int len = snprintf(buf, sizeof(buf) - 1, "{\"success\":true,\"data\":%ld}\n", data);
        SDLNet_TCP_Send(api.activeSocket, buf, len);
        return;
    }

    // Create JSON response object
    VALUE json_root_real;
    VALUE *json_root = &json_root_real;
    value_init_dict(json_root);

    // Add ack
    VALUE *val_ack = value_dict_add(json_root, "ack");
    *val_ack = *ack_id;

    // Create success key
    VALUE *val_success = value_dict_add(json_root, "success");
    value_init_bool(val_success, true);

    // Create success key
    VALUE *val_data = value_dict_add(json_root, "data");
    value_init_int32(val_data, data);

    // Create JSON response
    char json_string[1024];
    struct dump_buf_state dump_state = {json_string, sizeof(json_string) - 1};
    int json_dump_return_value = json_dom_dump(json_root, json_value_dump_writer, &dump_state, 0, JSON_DOM_DUMP_MINIMIZE);

    *dump_state.out = 0;
    if (json_dump_return_value != 0)
    {
        value_fini(json_root);
        return;
    }

    // Add newline to end of data
    dump_state.out[0] = '\n';
    dump_state.out++;

    // Send data to client
    SDLNet_TCP_Send(api.activeSocket, json_string, dump_state.out - json_string);
    value_fini(json_root);
}

void api_clear_all_subscriptions()
{
    if (api_sub_count == 0)
    {
        return;
    }

    // Loop trough all subscriptions
    // We don't exit the loop earlier just incase
    // This way this function also works as a full subscription list refresh
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {
        // If this subscription slot is inactive we can skip it
        if (api_subscriptions[i].type == API_SUBSCRIBE_INACTIVE)
        {
            continue;
        }

        // Set type as inactive and clear all data
        api_subscriptions[i].type = API_SUBSCRIBE_INACTIVE;
        memset(api_subscriptions[i].event, 0, sizeof(api_subscriptions[i].event));
        memset(&api_subscriptions[i].var, 0, sizeof(struct SubscribedVariable));
    }

    api_sub_count = 0;
}

int api_is_subscribed_to_event(const char *event_name)
{
    // Look up if we are subscribed to this event
    int api_sub_found_count = 0;
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {
        // Cancel this subscription search if we have
        // seen the same amount of subscriptions as we are subscribed to
        if (api_sub_count == api_sub_found_count)
        {
            return false;
        }

        // If this subscription slot is inactive we can skip it
        if (api_subscriptions[i].type == API_SUBSCRIBE_INACTIVE)
        {
            continue;
        }

        api_sub_found_count++;

        // Ignore if this subscription slot is not an event
        if (api_subscriptions[i].type != API_SUBSCRIBE_EVENT)
        {
            continue;
        }

        // Break out of the for loop if this subscription event matches the triggered one
        if (strcmp(api_subscriptions[i].event, event_name) == 0)
        {
            return true;
        }
    }

    return false;
}

int api_subscribe_event(const char *event_name)
{
    // Return if we are already subscribed to this event
    if (api_is_subscribed_to_event(event_name) == true)
    {
        return true;
    }

    // Make sure we have an open subscription slot
    if (api_sub_count >= API_SUBSCRIBE_LIST_SIZE)
    {
        WARNLOG(
            "Tried to register API event '%s' but we are already at the limit of %d subscription slots",
            event_name,
            API_SUBSCRIBE_LIST_SIZE);

        return false;
    }

    // Loop trough the list of subscription slots to find an empty slot and create a subscription
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {

        // If this subscription slot is inactive we'll use it
        if (api_subscriptions[i].type == API_SUBSCRIBE_INACTIVE)
        {
            api_subscriptions[i].type = API_SUBSCRIBE_EVENT;
            strncpy(api_subscriptions[i].event, event_name, sizeof(api_subscriptions[i].event) - 1);
            api_sub_count++;
            return true;
        }
    }

    return false;
}

int api_unsubscribe_event(const char *event_name)
{
    // First make sure we are actually subscribed to this event
    if (api_is_subscribed_to_event(event_name) == false)
    {
        return true;
    }

    // Loop trough the list of subscription slots to find an empty slot and create a subscription
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {

        // If this subscription slot is not an event we'll skip it
        if (api_subscriptions[i].type != API_SUBSCRIBE_EVENT)
        {
            continue;
        }

        // Check if this subscription slot is the event we're unsubscribing from
        if (strcmp(api_subscriptions[i].event, event_name) == 0)
        {
            api_subscriptions[i].type = API_SUBSCRIBE_INACTIVE;
            memset(api_subscriptions[i].event, 0, sizeof(api_subscriptions[i].event));
            api_sub_count--;
            return true;
        }
    }

    return false;
}

int api_is_subscribed_to_var(PlayerNumber plyr_idx, unsigned char valtype, short validx)
{
    // Look up if we are subscribed to updates of this variable
    int api_sub_found_count = 0;
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {
        // Cancel this subscription search if we have
        // seen the same amount of subscriptions as we are subscribed to
        if (api_sub_count == api_sub_found_count)
        {
            return false;
        }

        // If this subscription slot is inactive we can skip it
        if (api_subscriptions[i].type == API_SUBSCRIBE_INACTIVE)
        {
            continue;
        }

        api_sub_found_count++;

        // Ignore if this subscription slot is not for a var
        if (api_subscriptions[i].type != API_SUBSCRIBE_VAR)
        {
            continue;
        }

        // Break out of the for loop if this subscribed var matches the triggered one
        if (api_subscriptions[i].var.player_id == plyr_idx &&
            api_subscriptions[i].var.type == valtype &&
            api_subscriptions[i].var.id == validx)
        {
            return true;
        }
    }

    return false;
}

int api_subscribe_var(PlayerNumber plyr_idx, const char *var_name, unsigned char valtype, short validx)
{
    JUSTLOG("Sub: %d, %d, %d", plyr_idx, valtype, validx);

    // Return if we are already subscribed to this var
    if (api_is_subscribed_to_var(plyr_idx, valtype, validx) == true)
    {
        return true;
    }

    // Make sure we have an open subscription slot
    if (api_sub_count >= API_SUBSCRIBE_LIST_SIZE)
    {
        WARNLOG("Tried to register to update of var but we are already at the limit of %d subscription slots", API_SUBSCRIBE_LIST_SIZE);
        return false;
    }

    // Loop trough the list of subscription slots to find an empty slot and create a subscription
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {

        // If this subscription slot is inactive we'll use it
        if (api_subscriptions[i].type == API_SUBSCRIBE_INACTIVE)
        {

            struct SubscribedVariable sub_var;
            sub_var.player_id = plyr_idx;
            sub_var.type = valtype;
            sub_var.id = validx;
            sub_var.val = get_condition_value(plyr_idx, valtype, validx);
            strncpy(sub_var.name, var_name, sizeof(sub_var.name) - 1);

            api_subscriptions[i].type = API_SUBSCRIBE_VAR;
            api_subscriptions[i].var = sub_var;

            api_sub_count++;
            return true;
        }
    }

    return false;
}

int api_unsubscribe_var(PlayerNumber plyr_idx, unsigned char valtype, short validx)
{
    // First make sure we are actually subscribed to this var
    if (api_is_subscribed_to_var(plyr_idx, valtype, validx) == false)
    {
        return true;
    }

    // Loop trough the list of subscription slots to find an empty slot and create a subscription
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {

        // If this subscription slot is not an event we'll skip it
        if (api_subscriptions[i].type != API_SUBSCRIBE_VAR)
        {
            continue;
        }

        // Check if this subscription slot is the event we're unsubscribing from
        if (api_subscriptions[i].var.player_id == plyr_idx &&
            api_subscriptions[i].var.type == valtype &&
            api_subscriptions[i].var.id == validx)
        {
            api_subscriptions[i].type = API_SUBSCRIBE_INACTIVE;
            memset(&api_subscriptions[i].var, 0, sizeof(struct SubscribedVariable));
            api_sub_count--;
            return true;
        }
    }

    return false;
}

void api_check_var_update()
{
    // Do nothing if API server is not active
    if (!api.activeSocket)
    {
        return;
    }

    // Loop trough all our subscriptions
    int api_sub_found_count = 0;
    for (int i = 0; i < API_SUBSCRIBE_LIST_SIZE; i++)
    {
        // Cancel this subscription search if we have
        // seen the same amount of subscriptions as we are subscribed to
        if (api_sub_count == api_sub_found_count)
        {
            return;
        }

        // If this subscription slot is inactive we can skip it
        if (api_subscriptions[i].type == API_SUBSCRIBE_INACTIVE)
        {
            continue;
        }

        api_sub_found_count++;

        // Ignore if this subscription slot is not for a var
        if (api_subscriptions[i].type != API_SUBSCRIBE_VAR)
        {
            continue;
        }

        // Get the variable value
        long variable_value = get_condition_value(
            api_subscriptions[i].var.player_id,
            api_subscriptions[i].var.type,
            api_subscriptions[i].var.id);

        // Check if variable has changed
        if (api_subscriptions[i].var.val != variable_value)
        {

            // Update the remembered value
            api_subscriptions[i].var.val = variable_value;

            // Send notification to client
            api_return_var_update(
                api_subscriptions[i].var.player_id,
                api_subscriptions[i].var.name,
                api_subscriptions[i].var.val);
        }
    }
}

/**
 * Send an API event message.
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

    // Do nothing if we are not subscribed to this event
    if (api_is_subscribed_to_event(event_name) == false)
    {
        return;
    }

    // Create the JSON response and send it to the client
    char buf[512];
    int len = snprintf(buf, sizeof(buf) - 1, "{\"event\":\"%s\"}\n", event_name);
    SDLNet_TCP_Send(api.activeSocket, buf, len);
}

/**
 * Process the incoming buffer from the API client.
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
    // Acknowledgement ID
    // This is used to link a response to a request.
    // The Ack ID should be sent back exactly like the client sent it.
    // It is useful because some clients handle our packets out of order or in very different scopes.
    VALUE *ack_id;

    // Values for the data of the buffer
    VALUE data, *value = &data;

    // Handle closing null byte
    if (buffer[buf_size - 1] == 0)
    {
        buf_size -= 1;
    }

    // Check if something is actually sent
    if (strlen(buffer) < 1)
    {
        api_err("NO_JSON", NULL);
        return;
    }

    // Decode the json object
    int ret = json_dom_parse(buffer, buf_size, NULL, 0, value, NULL);
    if (ret != 0)
    {
        api_err("INVALID_JSON", NULL);
        return;
    }

    // Make sure we have a json object
    if (value_type(value) != VALUE_DICT)
    {
        api_err("INVALID_JSON_OBJECT", NULL);
        value_fini(&data);
        return;
    }

    // Get ack ID of the packet
    ack_id = value_dict_get(value, "ack");

    // Get the action the user wants to do
    const char *action = value_string(value_dict_get(value, "action"));
    if (action == NULL)
    {
        api_err("MISSING_ACTION", ack_id);
        value_fini(&data);
        return;
    }

    // Get the player id for the action.
    // Falls back to player 0 (default player)
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

    // ==================================================================================================================================
    // Commands that always work
    // ==================================================================================================================================

    // Handle get KeeperFX info command
    if (strcasecmp("get_kfx_info", action) == 0)
    {
        // Create level data to return to client
        VALUE data_kfx_info_real;
        VALUE *data_kfx_info = &data_kfx_info_real;
        value_init_dict(data_kfx_info);

        // Add stuff to level data
        value_init_string(value_dict_add(data_kfx_info, "kfx_version"), VER_STRING);

        // Return data to client
        api_return_data(true, data_kfx_info_real, ack_id);

        // End
        value_fini(&data);
        return;
    }

    // Handle subscribe var command
    if (strcasecmp("subscribe_var", action) == 0)
    {
        // Get variable name
        const char *variable_name = (char *)value_string(value_dict_get(value, "var"));
        if (variable_name == NULL || strlen(variable_name) < 1)
        {
            api_err("MISSING_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Recognize variable
        long variable_id, variable_type;
        if (parse_get_varib(variable_name, &variable_id, &variable_type) == false)
        {
            api_err("UNKNOWN_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Try to subscribe to the variable
        if (api_subscribe_var(player_id, variable_name, variable_type, variable_id))
        {
            api_ok(ack_id);
        }
        else
        {
            api_err("SUB_FAILED", ack_id);
        }

        // End
        value_fini(&data);
        return;
    }

    // Handle subscribe var command
    if (strcasecmp("unsubscribe_var", action) == 0)
    {
        // Get variable name
        char *variable_name = (char *)value_string(value_dict_get(value, "var"));
        if (variable_name == NULL || strlen(variable_name) < 1)
        {
            api_err("MISSING_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Recognize variable
        long variable_id, variable_type;
        if (parse_get_varib(variable_name, &variable_id, &variable_type) == false)
        {
            api_err("UNKNOWN_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Try to subscribe to the variable
        if (api_unsubscribe_var(player_id, variable_type, variable_id))
        {
            api_ok(ack_id);
        }
        else
        {
            api_err("SUB_FAILED", ack_id);
        }

        // End
        value_fini(&data);
        return;
    }

    // Handle subscribe var command
    if (strcasecmp("subscribe_event", action) == 0)
    {
        // Get event name
        char *event_name = (char *)value_string(value_dict_get(value, "event"));
        if (event_name == NULL || strlen(event_name) < 1)
        {
            api_err("MISSING_EVENT", ack_id);
            value_fini(&data);
            return;
        }

        // Make sure event name is not too long
        if (strlen(event_name) > COMMAND_WORD_LEN)
        {
            api_err("STRING_TOO_LONG", ack_id);
            value_fini(&data);
            return;
        }

        // Try to subscribe to the variable
        if (api_subscribe_event(event_name))
        {
            api_ok(ack_id);
        }
        else
        {
            api_err("SUB_FAILED", ack_id);
        }

        // End
        value_fini(&data);
        return;
    }

    // Handle subscribe var command
    if (strcasecmp("unsubscribe_event", action) == 0)
    {
        // Get event name
        char *event_name = (char *)value_string(value_dict_get(value, "event"));
        if (event_name == NULL || strlen(event_name) < 1)
        {
            api_err("MISSING_EVENT", ack_id);
            value_fini(&data);
            return;
        }

        // Try to subscribe to the variable
        if (api_unsubscribe_event(event_name))
        {
            api_ok(ack_id);
        }
        else
        {
            api_err("SUB_FAILED", ack_id);
        }

        // End
        value_fini(&data);
        return;
    }

    // Handle unsubscribe all
    if (strcasecmp("unsubscribe_all", action) == 0)
    {
        // Unsubscribe from every subscriptions
        api_clear_all_subscriptions();
        api_ok(ack_id);

        // End
        value_fini(&data);
        return;
    }

    // ==================================================================================================================================
    // Commands that only work when a map is loaded
    // ==================================================================================================================================

    // At this point our game needs to be a LOCAL game before we do anything
    if (game.game_kind != GKind_LocalGame)
    {
        api_err("NOT_IN_LOCAL_GAME", ack_id);
        value_fini(&data);
        return;
    }

    // Handle map command
    if (strcasecmp("map_command", action) == 0)
    {
        // Do not allow this command when the game is paused
        if ((game.operation_flags & GOF_Paused) != 0)
        {
            api_err("GAME_IS_PAUSED", ack_id);
            value_fini(&data);
            return;
        }

        // Get map command
        char *map_command = (char *)value_string(value_dict_get(value, "command"));
        if (map_command == NULL)
        {
            api_err("MISSING_COMMAND", ack_id);
            value_fini(&data);
            return;
        }

        // Execute map command
        if (script_scan_line(map_command, false, 99)) // Maximum level of a command support
        {
            api_ok(ack_id);
        }
        else
        {
            api_err("FAILED_TO_EXECUTE_MAP_COMMAND", ack_id);
        }

        // End
        value_fini(&data);
        return;
    }

    // Handle console command
    if (strcasecmp("console_command", action) == 0)
    {
        // Do not allow this command when the game is paused
        if ((game.operation_flags & GOF_Paused) != 0)
        {
            api_err("GAME_IS_PAUSED", ack_id);
            value_fini(&data);
            return;
        }

        // Get console command
        char *console_command = (char *)value_string(value_dict_get(value, "command"));
        if (console_command == NULL || strlen(console_command) < 1)
        {
            api_err("MISSING_COMMAND", ack_id);
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
            api_ok(ack_id);
        }
        else
        {
            api_err("FAILED_TO_EXECUTE_CONSOLE_COMMAND", ack_id);
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
            // Create object for this player
            VALUE *player_info = value_dict_add(flag_data, player_code_name(player_index));
            value_init_dict(player_info);

            for (int flag_index = 0; flag_index < get_max_flags(); flag_index++)
            {
                // Get flag value
                long flag_value = get_condition_value(player_id, SVar_FLAG, flag_index);

                // Add flag to player flag
                const char *flag_string = get_conf_parameter_text(flag_desc, flag_index);
                value_init_int32(value_dict_add(player_info, flag_string), flag_value);
            }
        }

        // Return data to client
        api_return_data(true, flag_data_real, ack_id);

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
            api_err("MISSING_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Recognize variable
        long variable_id, variable_type;
        if (parse_get_varib(variable_name, &variable_id, &variable_type) == false)
        {
            api_err("UNKNOWN_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Get the variable
        long variable_value = get_condition_value(player_id, variable_type, variable_id);

        // Return the variable to the user
        api_return_data_number(variable_value, ack_id);

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
            api_err("MISSING_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Recognize variable
        long variable_id, variable_type;
        if (parse_get_varib(variable_name, &variable_id, &variable_type) == false)
        {
            api_err("UNKNOWN_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Check if this type of variable can be set dynamically
        if (
            variable_type != SVar_FLAG &&
            variable_type != SVar_CAMPAIGN_FLAG &&
            variable_type != SVar_BOX_ACTIVATED &&
            variable_type != SVar_TRAP_ACTIVATED &&
            variable_type != SVar_SACRIFICED &&
            variable_type != SVar_REWARDED)
        {
            api_err("UNABLE_TO_SET_VAR", ack_id);
            value_fini(&data);
            return;
        }

        // Get the new value
        VALUE *new_value = value_dict_get(value, "value");
        if (new_value == NULL || value_type(new_value) != VALUE_INT32)
        {
            api_err("VALUE_MUST_BE_INT", ack_id);
            value_fini(&data);
            return;
        }

        // Set the variable
        set_variable(player_id, variable_type, variable_id, value_int32(new_value));

        // Return success
        api_ok(ack_id);

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
        api_return_data(true, data_level_info_real, ack_id);

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
        api_return_data(true, data_current_game_info_real, ack_id);

        // End
        value_fini(&data);
        return;
    }

    // Return unknown action
    // TODO: we should do this check before...
    api_err("UNKNOWN_ACTION", ack_id);
    value_fini(&data);
}

/**
 * Processes a buffer containing concatenated JSON objects, extracting and
 * processing each valid JSON object individually.
 *
 * @param buffer Pointer to the buffer containing concatenated JSON data.
 * @param buf_size Size of the buffer in bytes.
 */
void api_process_multipart_json(const char *buffer, int buf_size)
{
    int start = -1;
    int depth = 0;

    for (int i = 0; i < buf_size; ++i)
    {
        if (buffer[i] == '{')
        {
            if (depth == 0)
            {
                start = i; // Start of a new JSON object
            }
            depth++;
        }
        else if (buffer[i] == '}')
        {
            depth--;
            if (depth == 0 && start != -1)
            {
                // Extract the JSON object from buffer[start] to buffer[i+1]
                int json_length = i - start + 1;
                //char json_string[json_length + 1]; // +1 for null terminator
                char* json_string = (char*)malloc((json_length + 1) * sizeof(char));
                if (!json_string) return;
                strncpy(json_string, buffer + start, json_length);
                json_string[json_length] = '\0';

                // Process the extracted JSON object
                JUSTLOG("Received message from client: %s", json_string);
                api_process_buffer(json_string, json_length);
                free(json_string);
                // Reset start to look for the next JSON object
                start = -1;
            }
        }
    }

    if (depth > 0)
    {
        api_err("INVALID_JSON_IN_PACKET", NULL);
    }
}

/**
 * Update the API server and handle all pending packets.
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

    // Create a buffer to read from the socket
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
                    // TODO: non nullbyte terminated buffers can crash
                    // For example: when pressing Ctrl C when conneted over telnet

                    // Remove any possible trailing newline from the data
                    // This makes it work with a Telnet connection as well
                    if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
                    {
                        buffer[strlen(buffer) - 1] = '\0';
                    }

                    // Process all JSON objects in the buffer
                    api_process_multipart_json(buffer, strlen(buffer));
                }
                else
                {
                    api_clear_all_subscriptions();
                    SDLNet_TCP_DelSocket(api.socketSet, api.activeSocket);
                    SDLNet_TCP_Close(api.activeSocket);
                    api.activeSocket = 0;
                    JUSTLOG("API connection closed");
                }

                // Clear buffer
                memset(buffer, 0, API_SERVER_BUFFER);

            } // \activeSocket
        }
    } while (numReady > 0); // To have break instead of goto

    // Handle variable subscriptions
    api_check_var_update();
}

/**
 * Close the API server.
 *
 * This function stops the API server by closing the server socket and active client socket,
 * and frees the socket set. It also shuts down the SDLNet library.
 */
void api_close_server()
{
    api_clear_all_subscriptions();

    JUSTLOG("API server closing");

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
