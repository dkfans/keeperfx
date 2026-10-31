/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file net_steam.cpp
 *     Steam network service provider.
 * @par Purpose:
 *     Lets two players find each other through a Steam lobby and play over Steam's relay network.
 * @par Comment:
 *     The Steamworks SDK is not needed to build this: the few flat C API entry points that are used
 *     are declared below and looked up in steam_api.dll at runtime (the DLL is loaded by name, so it
 *     has to sit next to keeperfx.exe together with steam_appid.txt; 480 is Valve's Spacewar test app).
 *
 *     Mapping onto the NetSP interface:
 *       - a NetUserId is a Steam ID (the host is user 0, every other user is assigned by OnNewUser);
 *       - channel 0 of ISteamNetworkingMessages carries the game messages, reliable or unreliable like
 *         the ENet service provider sends them; messages above SPLIT_SIZE are cut into pieces;
 *       - channel 1 carries control messages (kick with a reason, goodbye, keep-alive), because
 *         ISteamNetworkingMessages gives no notification when a peer closes its session;
 *       - a peer that stays silent for PEER_SILENCE_LIMIT_MS is dropped, like an ENet timeout.
 *     Struct layouts follow the Windows 32 bit SDK (callbacks are packed to 8 bytes).
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_steam.h"
#include "net_main.h"
#include "bflib_datetm.h"
#include "bflib_netsession.h"
#include "front_network.h"
#include "globals.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <vector>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <SDL3/SDL.h>
#include "post_inc.h"

TbBool steam_host_requested = false;
char steam_join_lobby[32] = "";
TbBool steam_wait_for_invite = false;

namespace
{
    typedef uint64_t SteamID;
    typedef uint64_t SteamAPICall;
    typedef int32_t HSteamPipe;
    typedef int32_t HSteamUser;

    // ---- Steamworks flat API (steam_api.dll) ----
    enum {
        k_ELobbyTypeFriendsOnly = 1,
        k_ESteamNetworkingIdentityType_SteamID = 16,
        k_nSteamNetworkingSend_Unreliable = 0,
        k_nSteamNetworkingSend_NoNagle = 1,
        k_nSteamNetworkingSend_Reliable = 8,
        k_ESteamNetworkingConfig_SendBufferSize = 9,
        k_ESteamNetworkingConfig_RecvBufferSize = 47,
        k_ESteamNetworkingConfig_RecvMaxMessageSize = 49,
        k_EResultOK = 1,
        k_EChatRoomEnterResponseSuccess = 1,
        k_iSteamFriendsCallbacks = 300,
        k_iSteamMatchmakingCallbacks = 500,
        k_iSteamUtilsCallbacks = 700,
        k_iSteamNetworkingMessagesCallbacks = 1250,
        CB_GameLobbyJoinRequested = k_iSteamFriendsCallbacks + 33,
        CB_GameRichPresenceJoinRequested = k_iSteamFriendsCallbacks + 37,
        CB_LobbyEnter = k_iSteamMatchmakingCallbacks + 4,
        CB_LobbyChatUpdate = k_iSteamMatchmakingCallbacks + 6,
        CB_LobbyCreated = k_iSteamMatchmakingCallbacks + 13,
        CB_SteamAPICallCompleted = k_iSteamUtilsCallbacks + 3,
        CB_SessionRequest = k_iSteamNetworkingMessagesCallbacks + 1,
        CB_SessionFailed = k_iSteamNetworkingMessagesCallbacks + 2,
        // EChatMemberStateChange
        CHATMEMBER_LEFT = 0x2,
        CHATMEMBER_DISCONNECTED = 0x4,
        CHATMEMBER_KICKED = 0x8,
        CHATMEMBER_BANNED = 0x10,
    };

#pragma pack(push, 8)
    struct CallbackMsg { // CallbackMsg_t
        HSteamUser user;
        int32_t callback;
        uint8_t *param;
        int32_t param_size;
    };
    struct APICallCompleted { // SteamAPICallCompleted_t
        SteamAPICall call;
        int32_t callback;
        uint32_t param_size;
    };
    struct LobbyCreatedCb { // LobbyCreated_t
        int32_t result;
        uint64_t lobby;
    };
    struct LobbyEnterCb { // LobbyEnter_t
        uint64_t lobby;
        uint32_t permissions;
        bool locked;
        uint32_t response;
    };
    struct LobbyChatUpdateCb { // LobbyChatUpdate_t
        uint64_t lobby;
        uint64_t user_changed;
        uint64_t making_change;
        uint32_t state_change;
    };
    struct GameLobbyJoinRequestedCb { // GameLobbyJoinRequested_t
        uint64_t lobby;
        uint64_t friend_id;
    };
    struct RichPresenceJoinRequestedCb { // GameRichPresenceJoinRequested_t
        uint64_t friend_id;
        char connect[256];
    };
#pragma pack(pop)

    // SteamNetworkingIdentity is 136 bytes; it is only ever handled through the SDK's own helper functions.
    struct Identity {
        uint8_t raw[160];
    };
    // SteamNetworkingMessage_t, laid out for the MSVC build of steam_api.dll: void *data; int size; uint32 conn; (padding) identity at 16.
    const size_t MSG_OFF_DATA = 0;
    const size_t MSG_OFF_SIZE = 4;
    const size_t MSG_OFF_IDENTITY = 16;
    const size_t MSG_OFF_IDENTITY_ALT = 12; // same struct with 4 byte alignment of 64 bit members

    typedef bool (__cdecl *fn_IsSteamRunning)(void);
    typedef int (__cdecl *fn_InitFlat)(char *errmsg);
    typedef HSteamPipe (__cdecl *fn_GetHSteamPipe)(void);
    typedef void (__cdecl *fn_ManualDispatch_Init)(void);
    typedef void (__cdecl *fn_ManualDispatch_RunFrame)(HSteamPipe);
    typedef bool (__cdecl *fn_ManualDispatch_GetNextCallback)(HSteamPipe, CallbackMsg *);
    typedef void (__cdecl *fn_ManualDispatch_FreeLastCallback)(HSteamPipe);
    typedef bool (__cdecl *fn_ManualDispatch_GetAPICallResult)(HSteamPipe, SteamAPICall, void *, int, int, bool *);
    typedef void *(__cdecl *fn_GetInterface)(void);
    typedef SteamID (__cdecl *fn_User_GetSteamID)(void *);
    typedef bool (__cdecl *fn_User_BLoggedOn)(void *);
    typedef SteamAPICall (__cdecl *fn_Matchmaking_CreateLobby)(void *, int, int);
    typedef SteamAPICall (__cdecl *fn_Matchmaking_JoinLobby)(void *, SteamID);
    typedef void (__cdecl *fn_Matchmaking_LeaveLobby)(void *, SteamID);
    typedef bool (__cdecl *fn_Matchmaking_SetLobbyData)(void *, SteamID, const char *, const char *);
    typedef bool (__cdecl *fn_Matchmaking_SetLobbyJoinable)(void *, SteamID, bool);
    typedef SteamID (__cdecl *fn_Matchmaking_GetLobbyOwner)(void *, SteamID);
    typedef bool (__cdecl *fn_Friends_SetRichPresence)(void *, const char *, const char *);
    typedef void (__cdecl *fn_Friends_ActivateInviteDialog)(void *, SteamID);
    typedef void (__cdecl *fn_Identity_SetSteamID64)(Identity *, uint64_t);
    typedef uint64_t (__cdecl *fn_Identity_GetSteamID64)(const Identity *);
    typedef int (__cdecl *fn_Msgs_Send)(void *, const Identity *, const void *, uint32_t, int, int);
    typedef int (__cdecl *fn_Msgs_Receive)(void *, int, void **, int);
    typedef bool (__cdecl *fn_Msgs_Accept)(void *, const Identity *);
    typedef bool (__cdecl *fn_Msgs_Close)(void *, const Identity *);
    typedef void (__cdecl *fn_Message_Release)(void *);
    typedef bool (__cdecl *fn_Utils_SetGlobalInt32)(void *, int, int32_t);

    struct Api {
        HMODULE lib;
        fn_IsSteamRunning IsSteamRunning;
        fn_InitFlat InitFlat;
        fn_GetHSteamPipe GetHSteamPipe;
        fn_ManualDispatch_Init ManualDispatch_Init;
        fn_ManualDispatch_RunFrame ManualDispatch_RunFrame;
        fn_ManualDispatch_GetNextCallback ManualDispatch_GetNextCallback;
        fn_ManualDispatch_FreeLastCallback ManualDispatch_FreeLastCallback;
        fn_ManualDispatch_GetAPICallResult ManualDispatch_GetAPICallResult;
        fn_User_GetSteamID User_GetSteamID;
        fn_User_BLoggedOn User_BLoggedOn;
        fn_Matchmaking_CreateLobby CreateLobby;
        fn_Matchmaking_JoinLobby JoinLobby;
        fn_Matchmaking_LeaveLobby LeaveLobby;
        fn_Matchmaking_SetLobbyData SetLobbyData;
        fn_Matchmaking_SetLobbyJoinable SetLobbyJoinable;
        fn_Matchmaking_GetLobbyOwner GetLobbyOwner;
        fn_Friends_SetRichPresence SetRichPresence;
        fn_Friends_ActivateInviteDialog ActivateInviteDialog;
        fn_Identity_SetSteamID64 Identity_SetSteamID64;
        fn_Identity_GetSteamID64 Identity_GetSteamID64;
        fn_Msgs_Send SendMessageToUser;
        fn_Msgs_Receive ReceiveMessagesOnChannel;
        fn_Msgs_Accept AcceptSessionWithUser;
        fn_Msgs_Close CloseSessionWithUser;
        fn_Message_Release Message_Release;
        fn_Utils_SetGlobalInt32 SetGlobalConfigValueInt32;
        void *user;
        void *matchmaking;
        void *friends;
        void *msgs;
        void *utils;
    };

    Api api;
    bool api_ready = false;
    bool api_failed = false;
    char last_error[512] = "";

    // ---- session state ----
    const uint8_t CHANNEL_DATA = 0;
    const uint8_t CHANNEL_CONTROL = 1;
    const uint8_t CTRL_KICK = 1;    // + reason byte (NetJoinRejection)
    const uint8_t CTRL_GOODBYE = 2;
    const uint8_t CTRL_PING = 3;
    const uint8_t FRAME_WHOLE = 0;
    const uint8_t FRAME_PART = 1;
    const uint8_t FRAME_LAST = 2;
    const size_t SPLIT_SIZE = 60000;
    const uint32_t PING_INTERVAL_MS = 1000;
    const uint32_t PEER_SILENCE_LIMIT_MS = 20000;
    const uint32_t LOBBY_TIMEOUT_MS = 15000;
    const uint32_t CLOSE_DELAY_MS = 1500;

    struct Peer {
        bool used;
        SteamID sid;
        uint32_t last_rx;
        uint32_t last_ping;
        std::deque<std::vector<char>> queue;
        std::vector<char> partial;
    };

    struct PendingClose {
        SteamID sid;
        uint32_t when;
    };

    NetDropCallback g_drop_callback = nullptr;
    Peer peers[MAX_NET_CONNECTIONS];
    SteamID my_sid = 0;
    SteamID lobby_id = 0;
    char lobby_text[32] = "";
    bool session_active = false;
    bool session_is_host = false;
    std::vector<PendingClose> pending_closes;
    std::vector<PendingClose> rejected;
    uint32_t last_pump_call = 0;
    bool in_pump = false;
    bool error_unreported = false;

    // lobby / call state
    SteamAPICall create_call = 0;
    SteamAPICall join_call = 0;
    int create_state = 0; // 0 waiting, 1 done, -1 failed
    int join_state = 0;
    SteamID join_wait_lobby = 0;
    char join_request_lobby[32] = "";
    bool join_request_pending = false;

    typedef enum NetJoinRejection (*NewUserFn)(NetUserId *assigned_id, enum NetConnectionRole role);

    void set_error(const char *format, ...) __attribute__((format(printf, 1, 2)));
    void set_error(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        vsnprintf(last_error, sizeof(last_error), format, args);
        va_end(args);
        JUSTLOG("Steam: %s", last_error);
    }

    uint32_t now_ms(void)
    {
        return (uint32_t)LbTimerClock();
    }

    int find_peer(SteamID sid)
    {
        for (int i = 0; i < MAX_NET_CONNECTIONS; i++) {
            if (peers[i].used && peers[i].sid == sid)
                return i;
        }
        return -1;
    }

    void clear_peer(NetUserId id)
    {
        peers[id].used = false;
        peers[id].sid = 0;
        peers[id].queue.clear();
        peers[id].partial.clear();
    }

    void make_identity(Identity *ident, SteamID sid)
    {
        memset(ident, 0, sizeof(*ident));
        api.Identity_SetSteamID64(ident, sid);
    }

    template <typename T> bool load(T &fn, const char *name)
    {
        fn = reinterpret_cast<T>(reinterpret_cast<void *>(GetProcAddress(api.lib, name)));
        if (fn == nullptr) {
            set_error("steam_api.dll is too old or incompatible: it lacks %s", name);
            return false;
        }
        return true;
    }

    bool load_api(void)
    {
        api.lib = LoadLibraryA("steam_api.dll");
        if (!api.lib) {
            set_error("steam_api.dll was not found next to keeperfx.exe");
            return false;
        }
        bool ok = load(api.IsSteamRunning, "SteamAPI_IsSteamRunning")
            && load(api.InitFlat, "SteamAPI_InitFlat")
            && load(api.GetHSteamPipe, "SteamAPI_GetHSteamPipe")
            && load(api.ManualDispatch_Init, "SteamAPI_ManualDispatch_Init")
            && load(api.ManualDispatch_RunFrame, "SteamAPI_ManualDispatch_RunFrame")
            && load(api.ManualDispatch_GetNextCallback, "SteamAPI_ManualDispatch_GetNextCallback")
            && load(api.ManualDispatch_FreeLastCallback, "SteamAPI_ManualDispatch_FreeLastCallback")
            && load(api.ManualDispatch_GetAPICallResult, "SteamAPI_ManualDispatch_GetAPICallResult")
            && load(api.User_GetSteamID, "SteamAPI_ISteamUser_GetSteamID")
            && load(api.User_BLoggedOn, "SteamAPI_ISteamUser_BLoggedOn")
            && load(api.CreateLobby, "SteamAPI_ISteamMatchmaking_CreateLobby")
            && load(api.JoinLobby, "SteamAPI_ISteamMatchmaking_JoinLobby")
            && load(api.LeaveLobby, "SteamAPI_ISteamMatchmaking_LeaveLobby")
            && load(api.SetLobbyData, "SteamAPI_ISteamMatchmaking_SetLobbyData")
            && load(api.SetLobbyJoinable, "SteamAPI_ISteamMatchmaking_SetLobbyJoinable")
            && load(api.GetLobbyOwner, "SteamAPI_ISteamMatchmaking_GetLobbyOwner")
            && load(api.SetRichPresence, "SteamAPI_ISteamFriends_SetRichPresence")
            && load(api.ActivateInviteDialog, "SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog")
            && load(api.Identity_SetSteamID64, "SteamAPI_SteamNetworkingIdentity_SetSteamID64")
            && load(api.Identity_GetSteamID64, "SteamAPI_SteamNetworkingIdentity_GetSteamID64")
            && load(api.SendMessageToUser, "SteamAPI_ISteamNetworkingMessages_SendMessageToUser")
            && load(api.ReceiveMessagesOnChannel, "SteamAPI_ISteamNetworkingMessages_ReceiveMessagesOnChannel")
            && load(api.AcceptSessionWithUser, "SteamAPI_ISteamNetworkingMessages_AcceptSessionWithUser")
            && load(api.CloseSessionWithUser, "SteamAPI_ISteamNetworkingMessages_CloseSessionWithUser")
            && load(api.Message_Release, "SteamAPI_SteamNetworkingMessage_t_Release")
            && load(api.SetGlobalConfigValueInt32, "SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueInt32");
        return ok;
    }

    bool get_interface(void *&out, const char *name)
    {
        fn_GetInterface fn = reinterpret_cast<fn_GetInterface>(reinterpret_cast<void *>(GetProcAddress(api.lib, name)));
        if (!fn) {
            set_error("steam_api.dll is too old or incompatible: it lacks %s", name);
            return false;
        }
        out = fn();
        if (!out) {
            set_error("Steam did not provide the interface %s", name);
            return false;
        }
        return true;
    }

    /** Loads steam_api.dll and connects to the running Steam client (once). */
    bool ensure_api(void)
    {
        if (api_ready)
            return true;
        if (api_failed)
            return false;
        api_failed = true; // stays set unless everything below works
        memset(&api, 0, sizeof(api));
        if (!load_api())
            return false;
        // The dll reads the app id from steam_appid.txt in the working directory (480 = Spacewar, Valve's test app).
        FILE *appid = fopen("steam_appid.txt", "rb");
        if (appid) {
            fclose(appid);
        } else if ((appid = fopen("steam_appid.txt", "wb")) != nullptr) {
            fputs("480", appid);
            fclose(appid);
            JUSTLOG("Steam: created steam_appid.txt with the Spacewar app id 480");
        }
        if (!api.IsSteamRunning()) {
            set_error("Steam is not running. Start Steam and log in, then try again.");
            return false;
        }
        if (api.GetHSteamPipe() == 0) { // not initialised yet (the normal startup init happens only when steam_appid.txt existed)
            char errmsg[1024] = "";
            int result = api.InitFlat(errmsg);
            if (result != 0) {
                set_error("Could not connect to Steam (code %d): %s", result, errmsg);
                return false;
            }
        }
        api.ManualDispatch_Init();
        if (!get_interface(api.user, "SteamAPI_SteamUser_v023")
            || !get_interface(api.matchmaking, "SteamAPI_SteamMatchmaking_v009")
            || !get_interface(api.friends, "SteamAPI_SteamFriends_v018")
            || !get_interface(api.msgs, "SteamAPI_SteamNetworkingMessages_SteamAPI_v002")
            || !get_interface(api.utils, "SteamAPI_SteamNetworkingUtils_SteamAPI_v004"))
            return false;
        if (!api.User_BLoggedOn(api.user)) {
            set_error("Steam is offline or not logged in. Log in to Steam and try again.");
            return false;
        }
        my_sid = api.User_GetSteamID(api.user);
        // resync snapshots can be several megabytes
        api.SetGlobalConfigValueInt32(api.utils, k_ESteamNetworkingConfig_SendBufferSize, 32 * 1024 * 1024);
        api.SetGlobalConfigValueInt32(api.utils, k_ESteamNetworkingConfig_RecvBufferSize, 32 * 1024 * 1024);
        api.SetGlobalConfigValueInt32(api.utils, k_ESteamNetworkingConfig_RecvMaxMessageSize, 32 * 1024 * 1024);
        api_failed = false;
        api_ready = true;
        last_error[0] = '\0';
        JUSTLOG("Steam: networking ready, local Steam ID %llu", (unsigned long long)my_sid);
        return true;
    }

    // ---- sending ----
    int send_raw(SteamID sid, const void *data, uint32_t size, int flags, int channel)
    {
        Identity ident;
        make_identity(&ident, sid);
        return api.SendMessageToUser(api.msgs, &ident, data, size, flags, channel);
    }

    void send_control(SteamID sid, uint8_t type, uint8_t arg)
    {
        uint8_t buf[2] = {type, arg};
        send_raw(sid, buf, 2, k_nSteamNetworkingSend_Reliable | k_nSteamNetworkingSend_NoNagle, CHANNEL_CONTROL);
    }

    void send_frame(SteamID sid, uint8_t frame, const char *data, size_t size, bool reliable)
    {
        std::vector<char> wire(size + 1);
        wire[0] = (char)frame;
        if (size)
            memcpy(&wire[1], data, size);
        int flags = k_nSteamNetworkingSend_NoNagle | (reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable);
        for (int attempt = 0; attempt < 600; attempt++) {
            int result = send_raw(sid, wire.data(), (uint32_t)wire.size(), flags, CHANNEL_DATA);
            if (result == k_EResultOK || !reliable)
                return;
            // the send buffer is full (large resync): give Steam time to drain it
            SDL_Delay(5);
        }
        ERRORLOG("Steam: failed to send a %u byte message", (unsigned)size);
    }

    void send_message(SteamID sid, const char *buffer, size_t size, bool reliable)
    {
        if (!reliable || size <= SPLIT_SIZE) {
            send_frame(sid, FRAME_WHOLE, buffer, size, reliable);
            return;
        }
        for (size_t pos = 0; pos < size; pos += SPLIT_SIZE) {
            size_t piece = size - pos;
            bool last = piece <= SPLIT_SIZE;
            if (!last)
                piece = SPLIT_SIZE;
            send_frame(sid, last ? FRAME_LAST : FRAME_PART, buffer + pos, piece, true);
        }
    }

    // ---- peers ----
    void drop_peer(NetUserId id, enum NetDropReason reason, const char *why)
    {
        if (id < 0 || id >= MAX_NET_CONNECTIONS || !peers[id].used)
            return;
        LbNetLog("Steam: user %d (Steam ID %llu) %s\n", (int)id, (unsigned long long)peers[id].sid, why);
        PendingClose close;
        close.sid = peers[id].sid;
        close.when = now_ms() + CLOSE_DELAY_MS;
        pending_closes.push_back(close);
        clear_peer(id);
        if (g_drop_callback)
            g_drop_callback(id, reason);
    }

    void reject_stranger(SteamID sid, enum NetJoinRejection reason)
    {
        LbNetLog("Steam: rejecting Steam ID %llu, reason %d\n", (unsigned long long)sid, (int)reason);
        send_control(sid, CTRL_KICK, (uint8_t)reason);
        PendingClose close;
        close.sid = sid;
        close.when = now_ms() + CLOSE_DELAY_MS;
        pending_closes.push_back(close);
        rejected.push_back(close);
    }

    bool is_recently_rejected(SteamID sid)
    {
        uint32_t now = now_ms();
        for (size_t i = 0; i < rejected.size();) {
            if ((int32_t)(now - rejected[i].when) > 0) {
                rejected.erase(rejected.begin() + i);
                continue;
            }
            if (rejected[i].sid == sid)
                return true;
            i++;
        }
        return false;
    }

    /** Host side: a Steam user we do not know speaks to us. Returns the user id or -1. */
    int admit_stranger(SteamID sid, NewUserFn new_user)
    {
        if (!session_is_host || is_recently_rejected(sid))
            return -1;
        if (!new_user)
            new_user = OnNewUser;
        NetUserId user_id = INVALID_USER_ID;
        enum NetJoinRejection reason = new_user(&user_id, NetRole_Player);
        if (reason != NetJoin_Accepted || user_id <= SERVER_ID || user_id >= MAX_NET_CONNECTIONS) {
            reject_stranger(sid, reason == NetJoin_Accepted ? NetJoin_Full : reason);
            return -1;
        }
        Peer &peer = peers[user_id];
        clear_peer(user_id);
        peer.used = true;
        peer.sid = sid;
        peer.last_rx = now_ms();
        peer.last_ping = peer.last_rx;
        LbNetLog("Steam: user %d is Steam ID %llu, accepted\n", (int)user_id, (unsigned long long)sid);
        return user_id;
    }

    // ---- callbacks ----
    void handle_session_request(const Identity *ident, NewUserFn new_user)
    {
        SteamID sid = api.Identity_GetSteamID64(ident);
        if (sid == 0 || !session_active)
            return;
        int id = find_peer(sid);
        if (id < 0) {
            if (!session_is_host)
                return; // clients only talk to the host
            id = admit_stranger(sid, new_user);
            if (id < 0)
                return;
        }
        api.AcceptSessionWithUser(api.msgs, ident);
    }

    void parse_join_text(const char *text)
    {
        char lobby[32];
        if (steam_net_parse_lobby(text, lobby, sizeof(lobby))) {
            snprintf(join_request_lobby, sizeof(join_request_lobby), "%s", lobby);
            join_request_pending = true;
            JUSTLOG("Steam: join request for lobby %s", lobby);
        }
    }

    void handle_callback(const CallbackMsg &msg, NewUserFn new_user, const void *data, int size)
    {
        switch (msg.callback) {
        case CB_LobbyCreated:
            if (size >= (int)sizeof(LobbyCreatedCb)) {
                const LobbyCreatedCb *cb = static_cast<const LobbyCreatedCb *>(data);
                if (cb->result == k_EResultOK && cb->lobby) {
                    lobby_id = cb->lobby;
                    create_state = 1;
                } else {
                    create_state = -1;
                    set_error("Steam could not create a lobby (result %d)", (int)cb->result);
                }
            }
            break;
        case CB_LobbyEnter:
            if (size >= (int)sizeof(LobbyEnterCb)) {
                const LobbyEnterCb *cb = static_cast<const LobbyEnterCb *>(data);
                if (join_wait_lobby != 0 && cb->lobby == join_wait_lobby) {
                    if (cb->response == k_EChatRoomEnterResponseSuccess) {
                        join_state = 1;
                    } else {
                        join_state = -1;
                        set_error("Steam refused to let you into the lobby (response %u). The lobby may be closed, full or not visible to you (it is friends only).", (unsigned)cb->response);
                    }
                }
            }
            break;
        case CB_LobbyChatUpdate:
            if (size >= (int)sizeof(LobbyChatUpdateCb)) {
                const LobbyChatUpdateCb *cb = static_cast<const LobbyChatUpdateCb *>(data);
                if (cb->lobby == lobby_id && (cb->state_change & (CHATMEMBER_LEFT | CHATMEMBER_DISCONNECTED | CHATMEMBER_KICKED | CHATMEMBER_BANNED))) {
                    int id = find_peer(cb->user_changed);
                    if (id >= 0 && cb->user_changed != my_sid)
                        drop_peer(id, NETDROP_ERROR, "left the lobby");
                }
            }
            break;
        case CB_GameLobbyJoinRequested:
            if (size >= (int)sizeof(GameLobbyJoinRequestedCb)) {
                const GameLobbyJoinRequestedCb *cb = static_cast<const GameLobbyJoinRequestedCb *>(data);
                snprintf(join_request_lobby, sizeof(join_request_lobby), "%llu", (unsigned long long)cb->lobby);
                join_request_pending = true;
                JUSTLOG("Steam: invite accepted for lobby %s", join_request_lobby);
            }
            break;
        case CB_GameRichPresenceJoinRequested:
            if (size >= (int)sizeof(RichPresenceJoinRequestedCb)) {
                const RichPresenceJoinRequestedCb *cb = static_cast<const RichPresenceJoinRequestedCb *>(data);
                char connect[257];
                memcpy(connect, cb->connect, 256);
                connect[256] = '\0';
                parse_join_text(connect);
            }
            break;
        case CB_SessionRequest:
            if (size >= 136)
                handle_session_request(static_cast<const Identity *>(data), new_user);
            break;
        case CB_SessionFailed:
            if (size >= 136) {
                SteamID sid = api.Identity_GetSteamID64(static_cast<const Identity *>(data));
                int id = find_peer(sid);
                if (id >= 0)
                    drop_peer(id, NETDROP_ERROR, "session failed (connection lost)");
            }
            break;
        default:
            break;
        }
    }

    void dispatch_callbacks(NewUserFn new_user)
    {
        HSteamPipe pipe = api.GetHSteamPipe();
        api.ManualDispatch_RunFrame(pipe);
        CallbackMsg msg;
        while (api.ManualDispatch_GetNextCallback(pipe, &msg)) {
            if (msg.callback == CB_SteamAPICallCompleted && msg.param_size >= (int)sizeof(APICallCompleted)) {
                APICallCompleted done;
                memcpy(&done, msg.param, sizeof(done));
                std::vector<uint8_t> result(done.param_size + 8);
                bool failed = false;
                if (api.ManualDispatch_GetAPICallResult(pipe, done.call, result.data(), (int)done.param_size, done.callback, &failed) && !failed) {
                    CallbackMsg inner = msg;
                    inner.callback = done.callback;
                    handle_callback(inner, new_user, result.data(), (int)done.param_size);
                } else if (done.call == create_call) {
                    create_state = -1;
                    set_error("Steam could not create a lobby (is Steam online?)");
                } else if (done.call == join_call) {
                    join_state = -1;
                    set_error("Steam could not join the lobby (is Steam online?)");
                }
            } else {
                handle_callback(msg, new_user, msg.param, msg.param_size);
            }
            api.ManualDispatch_FreeLastCallback(pipe);
        }
    }

    // ---- receiving ----
    bool read_sender(void *message, SteamID *sid)
    {
        const uint8_t *base = static_cast<const uint8_t *>(message);
        for (size_t offset : {MSG_OFF_IDENTITY, MSG_OFF_IDENTITY_ALT}) {
            Identity ident;
            memset(&ident, 0, sizeof(ident));
            memcpy(&ident, base + offset, 136);
            SteamID found = api.Identity_GetSteamID64(&ident);
            if (found != 0) {
                *sid = found;
                return true;
            }
        }
        return false;
    }

    void queue_message(NetUserId id, std::vector<char> &&message)
    {
        Peer &peer = peers[id];
        if (id >= MAX_NET_USERS && (message.size() > NET_MSG_BUFFER_SIZE || peer.queue.size() >= 32)) {
            send_control(peer.sid, CTRL_KICK, (uint8_t)NetJoin_Locked);
            drop_peer(id, NETDROP_MANUAL, "sent too much data");
            return;
        }
        peer.queue.push_back(std::move(message));
        if (peer.queue.size() == 200)
            WARNLOG("Steam: %d queued messages from user %d", (int)peer.queue.size(), (int)id);
    }

    void handle_data(NetUserId id, const uint8_t *data, int size)
    {
        if (size < 1)
            return;
        Peer &peer = peers[id];
        uint8_t frame = data[0];
        if (frame == FRAME_WHOLE) {
            queue_message(id, std::vector<char>(reinterpret_cast<const char *>(data) + 1, reinterpret_cast<const char *>(data) + size));
            return;
        }
        peer.partial.insert(peer.partial.end(), reinterpret_cast<const char *>(data) + 1, reinterpret_cast<const char *>(data) + size);
        if (frame == FRAME_LAST) {
            std::vector<char> whole;
            whole.swap(peer.partial);
            queue_message(id, std::move(whole));
        }
    }

    void handle_control(NetUserId id, const uint8_t *data, int size)
    {
        if (size < 1)
            return;
        if (data[0] == CTRL_KICK) {
            if (!session_is_host && size >= 2 && data[1] >= NetJoin_InGame && data[1] <= NetJoin_SpectatorsUnsupported)
                net_join_rejection = (enum NetJoinRejection)data[1];
            drop_peer(id, NETDROP_ERROR, "removed us from the game");
        } else if (data[0] == CTRL_GOODBYE) {
            drop_peer(id, NETDROP_ERROR, "disconnected (clean)");
        }
    }

    void receive_channel(int channel, NewUserFn new_user)
    {
        void *messages[64];
        for (int round = 0; round < 16; round++) {
            int count = api.ReceiveMessagesOnChannel(api.msgs, channel, messages, 64);
            for (int i = 0; i < count; i++) {
                void *message = messages[i];
                SteamID sid = 0;
                const uint8_t *data = nullptr;
                int size = 0;
                memcpy(&data, static_cast<uint8_t *>(message) + MSG_OFF_DATA, sizeof(data));
                memcpy(&size, static_cast<uint8_t *>(message) + MSG_OFF_SIZE, sizeof(size));
                if (read_sender(message, &sid) && session_active) {
                    int id = find_peer(sid);
                    if (id < 0 && channel == CHANNEL_DATA)
                        id = admit_stranger(sid, new_user);
                    if (id >= 0) {
                        peers[id].last_rx = now_ms();
                        if (channel == CHANNEL_DATA)
                            handle_data(id, data, size);
                        else
                            handle_control(id, data, size);
                    }
                }
                api.Message_Release(message);
            }
            if (count < 64)
                break;
        }
    }

    void housekeeping(uint32_t now)
    {
        uint32_t gap = now - last_pump_call;
        last_pump_call = now;
        for (int i = 0; i < MAX_NET_CONNECTIONS; i++) {
            Peer &peer = peers[i];
            if (!peer.used)
                continue;
            // we did not look at the network for a while (level loading), so silence from the peer proves nothing
            if (gap > 2000)
                peer.last_rx = now;
            if (now - peer.last_ping >= PING_INTERVAL_MS) {
                peer.last_ping = now;
                send_control(peer.sid, CTRL_PING, 0);
            }
            if (now - peer.last_rx > PEER_SILENCE_LIMIT_MS) {
                drop_peer(i, NETDROP_ERROR, "timed out");
            }
        }
        for (size_t i = 0; i < pending_closes.size();) {
            if ((int32_t)(now - pending_closes[i].when) > 0) {
                Identity ident;
                make_identity(&ident, pending_closes[i].sid);
                api.CloseSessionWithUser(api.msgs, &ident);
                pending_closes.erase(pending_closes.begin() + i);
            } else {
                i++;
            }
        }
    }

    void pump(NewUserFn new_user)
    {
        if (!api_ready || in_pump)
            return;
        in_pump = true;
        dispatch_callbacks(new_user);
        if (session_active) {
            receive_channel(CHANNEL_CONTROL, new_user);
            receive_channel(CHANNEL_DATA, new_user);
            housekeeping(now_ms());
        }
        in_pump = false;
    }

    bool wait_state(const int &state, uint32_t timeout_ms, bool allow_cancel)
    {
        uint32_t start = now_ms();
        while (state == 0 && now_ms() - start < timeout_ms) {
            pump(nullptr);
            if (state != 0)
                break;
            if (allow_cancel && attempting_to_join_cancel_requested())
                return false;
            SDL_Delay(5);
        }
        return state == 1;
    }

    // ---- NetSP ----
    TbError steam_sp_init(NetDropCallback drop_callback)
    {
        if (!ensure_api())
            return Lb_FAIL;
        g_drop_callback = drop_callback;
        return Lb_OK;
    }

    void end_session(void)
    {
        if (api_ready && session_active) {
            for (int i = 0; i < MAX_NET_CONNECTIONS; i++) {
                if (peers[i].used)
                    send_control(peers[i].sid, CTRL_GOODBYE, 0);
            }
            SDL_Delay(100); // let the goodbyes leave before the sessions are closed
            for (int i = 0; i < MAX_NET_CONNECTIONS; i++) {
                if (peers[i].used) {
                    Identity ident;
                    make_identity(&ident, peers[i].sid);
                    api.CloseSessionWithUser(api.msgs, &ident);
                }
            }
            for (size_t i = 0; i < pending_closes.size(); i++) {
                Identity ident;
                make_identity(&ident, pending_closes[i].sid);
                api.CloseSessionWithUser(api.msgs, &ident);
            }
            if (lobby_id) {
                api.LeaveLobby(api.matchmaking, lobby_id);
                api.SetRichPresence(api.friends, "connect", nullptr);
                api.SetRichPresence(api.friends, "status", nullptr);
            }
        }
        for (int i = 0; i < MAX_NET_CONNECTIONS; i++)
            clear_peer(i);
        pending_closes.clear();
        rejected.clear();
        lobby_id = 0;
        lobby_text[0] = '\0';
        session_active = false;
        session_is_host = false;
        create_state = join_state = 0;
        create_call = join_call = 0;
        join_wait_lobby = 0;
    }

    void steam_sp_exit(void)
    {
        end_session();
        g_drop_callback = nullptr;
    }

    TbError steam_sp_host(const char *, void *)
    {
        if (!ensure_api())
            return Lb_FAIL;
        end_session();
        create_state = 0;
        create_call = api.CreateLobby(api.matchmaking, k_ELobbyTypeFriendsOnly, MAX_NET_USERS);
        if (create_call == 0) {
            set_error("Steam did not accept the request to create a lobby");
            return Lb_FAIL;
        }
        if (!wait_state(create_state, LOBBY_TIMEOUT_MS, false)) {
            if (create_state == 0)
                set_error("Steam did not answer the request to create a lobby");
            return Lb_FAIL;
        }
        session_active = true;
        session_is_host = true;
        last_pump_call = now_ms();
        snprintf(lobby_text, sizeof(lobby_text), "%llu", (unsigned long long)lobby_id);
        api.SetLobbyData(api.matchmaking, lobby_id, "game", "KeeperFX Archon co-op");
        char connect[64];
        snprintf(connect, sizeof(connect), "+connect_lobby %s", lobby_text);
        api.SetRichPresence(api.friends, "connect", connect);
        api.SetRichPresence(api.friends, "status", "Hosting KeeperFX co-op");
        JUSTLOG("Steam: lobby created, id %s", lobby_text);
        SDL_SetClipboardText(lobby_text);
        api.ActivateInviteDialog(api.friends, lobby_id);
        return Lb_OK;
    }

    TbError steam_sp_join(const char *session, void *)
    {
        if (!ensure_api())
            return Lb_FAIL;
        end_session();
        char *end = nullptr;
        SteamID lobby = strtoull(session ? session : "", &end, 10);
        if (lobby == 0 || (end && *end != '\0')) {
            set_error("'%s' is not a Steam lobby id", session ? session : "");
            return Lb_FAIL;
        }
        join_wait_lobby = lobby;
        join_state = 0;
        join_call = api.JoinLobby(api.matchmaking, lobby);
        if (join_call == 0) {
            set_error("Steam did not accept the request to join the lobby");
            return Lb_FAIL;
        }
        // the lobby join runs while the "attempting to join" screen is up, so it can be cancelled
        session_active = false;
        if (!wait_state(join_state, LOBBY_TIMEOUT_MS, true)) {
            if (join_state == 0)
                set_error("Steam did not answer the request to join lobby %llu", (unsigned long long)lobby);
            join_wait_lobby = 0;
            return Lb_FAIL;
        }
        SteamID owner = api.GetLobbyOwner(api.matchmaking, lobby);
        if (owner == 0) {
            set_error("The Steam lobby has no owner");
            api.LeaveLobby(api.matchmaking, lobby);
            join_wait_lobby = 0;
            return Lb_FAIL;
        }
        lobby_id = lobby;
        join_wait_lobby = 0;
        session_active = true;
        session_is_host = false;
        Peer &host = peers[SERVER_ID];
        clear_peer(SERVER_ID);
        host.used = true;
        host.sid = owner;
        host.last_rx = host.last_ping = now_ms();
        last_pump_call = host.last_rx;
        LbNetLog("Steam: joined lobby %llu, host is Steam ID %llu\n", (unsigned long long)lobby, (unsigned long long)owner);
        return Lb_OK;
    }

    void steam_sp_update(NewUserFn new_user)
    {
        pump(new_user);
    }

    void steam_sp_sendmsg_single(NetUserId destination, const char *buffer, size_t size)
    {
        if (!session_active)
            return;
        if (!session_is_host)
            destination = SERVER_ID;
        if (destination < 0 || destination >= MAX_NET_CONNECTIONS || !peers[destination].used)
            return;
        send_message(peers[destination].sid, buffer, size, true);
    }

    void steam_sp_sendmsg_single_unsequenced(NetUserId destination, const char *buffer, size_t size)
    {
        if (!session_active)
            return;
        if (!session_is_host)
            destination = SERVER_ID;
        if (destination < 0 || destination >= MAX_NET_CONNECTIONS || !peers[destination].used)
            return;
        send_message(peers[destination].sid, buffer, size, false);
    }

    void steam_sp_sendmsg_all(const char *buffer, size_t size)
    {
        if (!session_active)
            return;
        for (int i = 0; i < MAX_NET_CONNECTIONS; i++) {
            if (peers[i].used)
                send_message(peers[i].sid, buffer, size, true);
        }
    }

    bool wait_for_message(NetUserId source, unsigned timeout)
    {
        if (source < 0 || source >= MAX_NET_CONNECTIONS)
            return false;
        if (!peers[source].queue.empty())
            return true;
        uint32_t start = now_ms();
        while (true) {
            pump(nullptr);
            if (!peers[source].queue.empty())
                return true;
            if (timeout == 0 || now_ms() - start >= timeout || !api_ready)
                return false;
            SDL_Delay(1);
        }
    }

    size_t steam_sp_msgready(NetUserId source, unsigned timeout)
    {
        if (!wait_for_message(source, timeout))
            return 0;
        return peers[source].queue.front().size();
    }

    size_t steam_sp_peekmsg(NetUserId source, const char **buffer)
    {
        *buffer = nullptr;
        if (!wait_for_message(source, 0))
            return 0;
        *buffer = peers[source].queue.front().data();
        return peers[source].queue.front().size();
    }

    size_t steam_sp_readmsg(NetUserId source, char *buffer, size_t max_size)
    {
        if (!wait_for_message(source, 0))
            return 0;
        std::vector<char> &front = peers[source].queue.front();
        size_t copy_size = front.size() < max_size ? front.size() : max_size;
        if (copy_size)
            memcpy(buffer, front.data(), copy_size);
        peers[source].queue.pop_front();
        return copy_size;
    }

    void steam_sp_drop_user(NetUserId id, enum NetJoinRejection reason)
    {
        LbNetLog("Steam: dropping user %d\n", (int)id);
        if (id >= 0 && id < MAX_NET_CONNECTIONS && peers[id].used) {
            if (session_is_host)
                send_control(peers[id].sid, CTRL_KICK, (uint8_t)reason);
            drop_peer(id, NETDROP_MANUAL, "dropped");
        } else if (g_drop_callback) {
            g_drop_callback(id, NETDROP_MANUAL);
        }
    }
}

extern "C" {

struct NetSP *InitSteamSP(void)
{
    static struct NetSP ret = {
        steam_sp_init,
        steam_sp_exit,
        steam_sp_host,
        steam_sp_join,
        steam_sp_update,
        steam_sp_sendmsg_single,
        steam_sp_sendmsg_single_unsequenced,
        steam_sp_sendmsg_all,
        steam_sp_msgready,
        steam_sp_peekmsg,
        steam_sp_readmsg,
        steam_sp_drop_user,
    };
    return &ret;
}

const char *steam_net_last_error(void)
{
    return last_error[0] ? last_error : "Steam networking is not available.";
}

const char *steam_net_lobby_text(void)
{
    return lobby_text;
}

TbBool steam_net_parse_lobby(const char *text, char *lobby_out, size_t lobby_out_size)
{
    if (!text || !lobby_out || lobby_out_size < 2)
        return false;
    const char *key = strstr(text, "connect_lobby");
    const char *p = key ? key + strlen("connect_lobby") : text;
    while (*p == ' ' || *p == '=' || *p == '\t')
        p++;
    size_t length = 0;
    while (p[length] >= '0' && p[length] <= '9')
        length++;
    if (length < 5 || length >= lobby_out_size)
        return false;
    memcpy(lobby_out, p, length);
    lobby_out[length] = '\0';
    return true;
}

const char *steam_net_take_error(void)
{
    if (!error_unreported)
        return nullptr;
    error_unreported = false;
    return steam_net_last_error();
}

TbBool steam_net_poll_join_request(char *lobby_out, size_t lobby_out_size)
{
    if (!steam_wait_for_invite && !api_ready)
        return false;
    if (!api_ready) {
        if (!ensure_api()) {
            steam_wait_for_invite = false; // reported once, do not retry every frame
            error_unreported = true;
            return false;
        }
    }
    if (!session_active)
        pump(nullptr);
    if (!join_request_pending)
        return false;
    join_request_pending = false;
    snprintf(lobby_out, lobby_out_size, "%s", join_request_lobby);
    return true;
}

}
