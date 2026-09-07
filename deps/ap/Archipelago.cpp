#include "Archipelago.h"

#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXUserAgent.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <queue>
#include <random>
#include <fstream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <deque>
#include <set>
#include <string>
#include <chrono>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>
#include <algorithm>
#include <filesystem>

extern const int AP_OFFLINE_SLOT = 1404;
constexpr int AP_OFFLINE_TEAM = 0;
constexpr char const* AP_OFFLINE_NAME = "You";
constexpr AP_NetworkVersion AP_DEFAULT_NETWORK_VERSION = {0,5,1}; // Default for compatibility reasons

//Setup Stuff
std::function<void(std::string)> logfunc = [](std::string log){ std::cout << log << "\n"; };
bool init = false;
bool auth = false;
bool refused = false;
bool multiworld = true;
bool isSSL = true;
bool ssl_success = false;
int ap_player_id;
int ap_player_team;
std::string ap_player_name;
size_t ap_player_name_hash;
std::string ap_ip;
std::string ap_game;
std::string ap_passwd;
std::uint64_t ap_uuid = 0;
std::mt19937_64 rando;
AP_NetworkVersion client_version = AP_DEFAULT_NETWORK_VERSION; 

//Deathlink Stuff
bool deathlinkstat = false;
bool deathlinksupported = false;
bool enable_deathlink = false;
int deathlink_amnesty = 0;
int cur_deathlink_amnesty = 0;

// Message System
std::deque<AP_Message*> messageQueue;
bool queueitemrecvmsg = true;

// Data Maps
std::map<int, AP_NetworkPlayer> map_players;
std::map<std::pair<std::string,int64_t>, std::string> map_location_id_name;
std::map<std::pair<std::string,int64_t>, std::string> map_item_id_name;

// Data Sets
std::set<int> teams_set;

// Location / item state
std::set<int64_t> locations_checked;
std::set<int64_t> locations_scouted;
std::set<int64_t> missing_locations;
std::set<int64_t> checked_locations;
std::map<int64_t, AP_NetworkItem> locations_info;
std::vector<AP_NetworkItem> items_received;
bool receive_own_locations = false;
std::set<int64_t> pending_location_checks;
std::map<int, std::set<int64_t>> pending_location_scouts;
std::vector<std::tuple<int, int64_t, AP_HintStatus>> pending_hint_updates;
std::map<std::pair<int, AP_HintStatus>, std::set<int64_t>> pending_hints;
std::map<int, AP_NetworkSlot> map_slots;
std::map<std::string, std::string> stored_data;
std::set<std::string> stored_data_notification_keys;
int pending_status = -1;

// Callback function pointers
std::function<void()> resetItemValues = nullptr;
std::function<void(int64_t,bool)> getitemfunc = nullptr;
std::function<void(int64_t)> checklocfunc = nullptr;
std::function<void(std::vector<AP_NetworkItem>)> locinfofunc = nullptr;
std::function<void(std::string, std::string)> recvdeath = nullptr;
std::function<void(AP_SetReply)> setreplyfunc = nullptr;
std::function<void(AP_Bounce)> bouncedfunc = nullptr;
std::function<void()> socketconnectedfunc = nullptr;
std::function<void()> socketdisconnectedfunc = nullptr;
std::function<void(std::string)> socketerrorfunc = nullptr;
std::function<void()> slotconnectedfunc = nullptr;
std::function<void()> slotdisconnectedfunc = nullptr;
std::function<void(std::vector<std::string>)> connectionrefusedfunc = nullptr;
std::function<void()> roominfofunc = nullptr;
std::function<void()> roomupdatefunc = nullptr;
std::function<void(std::vector<AP_NetworkItem>)> itemsrecvfunc = nullptr;
std::function<void()> datapkgfunc = nullptr;
std::function<void(std::string)> printfunc = nullptr;
std::function<void(std::string)> printjsonfunc = nullptr;
std::function<void(std::string)> retrievedfunc = nullptr;
std::function<void(std::string, std::string)> stored_data_notifyfunc = nullptr;

// Serverdata Management
std::map<std::string,AP_DataType> map_serverdata_typemanage;
AP_GetServerDataRequest resync_serverdata_request;
uint32_t last_item_idx = 0;

void resolveDataStorageOp(Json::Value op);

// Gifting interop
bool gifting_supported = false;
bool gifting_autoReject = true;
void handleGiftAPISetReply(const AP_SetReply& reply);

// Singleplayer Seed Info
std::string sp_save_path;
Json::Value sp_save_root;

//Misc Data for Clients
AP_RoomInfo lib_room_info;

//Server Data Stuff
std::map<std::string, AP_GetServerDataRequest*> map_server_data;
std::queue<std::pair<Json::Value,AP_RequestStatus*>> queue_server_data;

//Slot Data Stuff
std::map<std::string, std::function<void(int)>> map_slotdata_callback_int;
std::map<std::string, std::function<void(std::string)>> map_slotdata_callback_raw;
std::map<std::string, std::function<void(std::map<int,int>)>> map_slotdata_callback_mapintint;

// Datapackage Stuff
std::filesystem::path datapkg_cache_dir = std::filesystem::current_path() / ".datapkg-cache";
std::set<std::string> datapkg_outdated_games;

ix::WebSocket webSocket;
Json::Reader reader;
Json::FastWriter writer;

Json::Value sp_ap_root;

// PRIV Func Declarations Start
void AP_Init_Generic();
bool parse_response(std::string msg, std::string &request);
void APSend(std::string req);
void WriteFileJSON(Json::Value val, std::string path);
std::string getItemName(std::string game, int64_t id);
std::string getLocationName(std::string game, int64_t id);
AP_NetworkPlayer getPlayer(int team, int slot);
bool loadDataPkg(const std::string& game, const std::string& hash);
void cacheDataPkgs(Json::Value& serverPkgs);
Json::Value getDataPkgRequest(void);
// PRIV Func Declarations End

void AP_Init(const char* ip, const char* game, const char* player_name, const char* passwd) {
    multiworld = true;
    
    uint64_t milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    rando = std::mt19937_64(milliseconds_since_epoch);

    if (!strcmp(ip,"")) {
        ip = "archipelago.gg:38281";
        logfunc("AP: Using default Server Adress: '" + std::string(ip) + "'");
    } else {
        logfunc("AP: Using Server Adress: '" + std::string(ip) + "'");
    }
    ap_ip = ip;
    ap_game = game;
    ap_player_name = player_name;
    ap_passwd = passwd;

    logfunc("AP: Initializing...");

    //Connect to server
    ix::initNetSystem();
    webSocket.setUrl("wss://" + ap_ip);
    webSocket.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg)
        {
            if (msg->type == ix::WebSocketMessageType::Message)
            {
                std::string request;
                if (parse_response(msg->str, request)) {
                    APSend(request);
                }
            }
            else if (msg->type == ix::WebSocketMessageType::Open)
            {
                logfunc("AP: Connected to Archipelago");
                if (socketconnectedfunc) socketconnectedfunc();
            }
            else if (msg->type == ix::WebSocketMessageType::Error || msg->type == ix::WebSocketMessageType::Close)
            {
                bool was_auth = auth;
                auth = false;
                for (auto itr = map_server_data.begin(); itr != map_server_data.end(); ) {
                    itr->second->status = AP_RequestStatus::Error;
                    itr = map_server_data.erase(itr);
                }
                if (msg->type == ix::WebSocketMessageType::Error && socketerrorfunc)
                    socketerrorfunc(msg->errorInfo.reason);
                if (msg->type == ix::WebSocketMessageType::Close) {
                    if (was_auth && slotdisconnectedfunc) slotdisconnectedfunc();
                    if (socketdisconnectedfunc) socketdisconnectedfunc();
                    map_slots.clear();
                    locations_info.clear();
                    items_received.clear();
                    checked_locations.clear();
                    missing_locations.clear();
                }
                logfunc("AP: Error connecting to Archipelago. Retries: " + std::to_string(msg->errorInfo.retries-1));
                if (msg->errorInfo.retries-1 >= 2 && isSSL && !ssl_success) {
                    logfunc("AP: SSL connection failed. Attempting unencrypted...");
                    webSocket.setUrl("ws://" + ap_ip);
                    isSSL = false;
                }
            }
        }
    );
    webSocket.enablePerMessageDeflate();
    webSocket.setPingInterval(45);

    AP_NetworkPlayer archipelago {
        -1,
        0,
        "Archipelago",
        "Archipelago",
        "__Server"

    };
    map_players[0] = archipelago;
    AP_Init_Generic();
}

void AP_Init(const char* filename) {
    multiworld = false;
    std::ifstream mwfile(filename);
    reader.parse(mwfile,sp_ap_root);
    mwfile.close();
    sp_save_path = std::string(filename) + ".save";
    std::ifstream savefile(sp_save_path);
    reader.parse(savefile, sp_save_root);
    savefile.close();
    WriteFileJSON(sp_save_root, sp_save_path);
    ap_player_name = AP_OFFLINE_NAME;
    AP_Init_Generic();
}

void AP_Start() {
    init = true;
    if (multiworld) {
        webSocket.start();
    } else {
        if (!sp_save_root.get("init", false).asBool()) {
            sp_save_root["init"] = true;
            sp_save_root["checked_locations"] = Json::arrayValue;
            sp_save_root["store"] = Json::objectValue;
        }
        // Only game in the data package is our game
        ap_game = sp_ap_root["data_package"]["data"]["games"].getMemberNames()[0];
        Json::Value fake_msg;
        fake_msg[0]["cmd"] = "Connected";
        fake_msg[0]["team"] = AP_OFFLINE_TEAM;
        fake_msg[0]["slot"] = AP_OFFLINE_SLOT;
        fake_msg[0]["slot_info"][std::to_string(AP_OFFLINE_SLOT)]["game"] = ap_game;
        fake_msg[0]["players"] = Json::arrayValue;
        fake_msg[0]["players"][0]["team"] = AP_OFFLINE_TEAM;
        fake_msg[0]["players"][0]["slot"] = AP_OFFLINE_SLOT;
        fake_msg[0]["players"][0]["alias"] = AP_OFFLINE_NAME;
        fake_msg[0]["players"][0]["name"] = AP_OFFLINE_NAME;
        fake_msg[0]["checked_locations"] = sp_save_root["checked_locations"];
        fake_msg[0]["missing_locations"] = Json::arrayValue;
        for (auto location : sp_ap_root["location_to_item"].getMemberNames()) {
            int64_t location_id = std::stoll(location);
            bool checked = false;
            for (auto checked_location : sp_save_root["checked_locations"]) {
                if (checked_location.asInt64() == location_id) {
                    checked = true;
                    break;
                }
            }
            if (!checked) fake_msg[0]["missing_locations"].append(location_id);
        }
        fake_msg[0]["slot_data"] = sp_ap_root["slot_data"];
        std::string req;
        parse_response(writer.write(fake_msg), req);
        fake_msg.clear();
        fake_msg[0]["cmd"] = "DataPackage";
        fake_msg[0]["data"] = sp_ap_root["data_package"]["data"];
        parse_response(writer.write(fake_msg), req);
        fake_msg.clear();
        fake_msg[0]["cmd"] = "ReceivedItems";
        fake_msg[0]["index"] = 0;
        fake_msg[0]["items"] = Json::arrayValue;
        for (unsigned int i = 0; i < sp_save_root["checked_locations"].size(); i++) {
            Json::Value item;
            item["item"] = sp_ap_root["location_to_item"][sp_save_root["checked_locations"][i].asString()].asInt64();
            item["location"] = 0;
            item["player"] = ap_player_id;
            fake_msg[0]["items"].append(item);
        }
        parse_response(writer.write(fake_msg), req);
    }
}

void AP_Shutdown() {
    if (multiworld)
        webSocket.stop();

    // Reset all states
    init = false;
    auth = false;
    refused = false;
    multiworld = true;
    isSSL = true;
    ssl_success = false;
    ap_player_id = 0;
    ap_player_name.clear();
    ap_ip.clear();
    ap_game.clear();
    ap_passwd.clear();
    ap_uuid = 0;
    client_version = AP_DEFAULT_NETWORK_VERSION;
    deathlinkstat = false;
    deathlinksupported = false;
    enable_deathlink = false;
    deathlink_amnesty = 0;
    cur_deathlink_amnesty = 0;
    gifting_autoReject = true;
    gifting_supported = false;
    while (AP_IsMessagePending()) AP_ClearLatestMessage();
    queueitemrecvmsg = true;
    map_players.clear();
    map_slots.clear();
    teams_set.clear();
    locations_checked.clear();
    locations_scouted.clear();
    missing_locations.clear();
    checked_locations.clear();
    locations_info.clear();
    items_received.clear();
    pending_location_checks.clear();
    pending_location_scouts.clear();
    pending_hint_updates.clear();
    pending_hints.clear();
    stored_data.clear();
    stored_data_notification_keys.clear();
    pending_status = -1;
    map_location_id_name.clear();
    map_item_id_name.clear();
    resetItemValues = nullptr;
    getitemfunc = nullptr;
    checklocfunc = nullptr;
    locinfofunc = nullptr;
    socketconnectedfunc = nullptr;
    socketdisconnectedfunc = nullptr;
    socketerrorfunc = nullptr;
    slotconnectedfunc = nullptr;
    slotdisconnectedfunc = nullptr;
    connectionrefusedfunc = nullptr;
    roominfofunc = nullptr;
    roomupdatefunc = nullptr;
    itemsrecvfunc = nullptr;
    datapkgfunc = nullptr;
    printfunc = nullptr;
    printjsonfunc = nullptr;
    retrievedfunc = nullptr;
    stored_data_notifyfunc = nullptr;
    recvdeath = nullptr;
    setreplyfunc = nullptr;
    map_serverdata_typemanage.clear();
    last_item_idx = 0;
    receive_own_locations = false;
    sp_save_path.clear();
    sp_save_root.clear();
    map_server_data.clear(); // Does this leak?
    map_slotdata_callback_int.clear();
    map_slotdata_callback_raw.clear();
    map_slotdata_callback_mapintint.clear();
    sp_ap_root = Json::objectValue;
}

bool AP_IsInit() {
    return init;
}

void AP_SetClientVersion(AP_NetworkVersion* version) {
    client_version.major = version->major;
    client_version.minor = version->minor;
    client_version.build = version->build;
}

void AP_SendItem(int64_t idx) {
    AP_SendItem(std::set<int64_t>{ idx });
}
void AP_SendItem(std::set<int64_t> const& locations) {
    for (int64_t idx : locations) {
        logfunc("AP: Checked '" + getLocationName(ap_game, idx) + "'.");
        locations_checked.insert(idx);
    }
    if (multiworld) {
        if (locations.empty()) return;
        if (webSocket.getReadyState() == ix::ReadyState::Open && auth) {
            Json::Value req_t;
            req_t[0]["cmd"] = "LocationChecks";
            req_t[0]["locations"] = Json::arrayValue;
            for (int64_t loc : locations) {
                req_t[0]["locations"].append(loc);
                if (!receive_own_locations) {
                    checked_locations.insert(loc);
                    missing_locations.erase(loc);
                }
            };
            APSend(writer.write(req_t));
        } else {
            pending_location_checks.insert(locations.begin(), locations.end());
            if (!receive_own_locations) {
                checked_locations.insert(locations.begin(), locations.end());
                for (int64_t loc : locations) missing_locations.erase(loc);
            }
        }
    } else {
        std::set<int64_t> new_locations;
        for (int64_t idx : locations) {
            bool was_previously_checked = false;
            for (auto itr : sp_save_root["checked_locations"]) {
                if (itr.asInt64() == idx) {
                    was_previously_checked = true;
                    break;
                }
            }
            if (!was_previously_checked) new_locations.insert(idx);
        }

        Json::Value fake_msg;
        fake_msg[0]["cmd"] = "ReceivedItems";
        fake_msg[0]["index"] = last_item_idx+1;
        fake_msg[0]["items"] = Json::arrayValue;
        for (int64_t idx : new_locations) {
            int64_t recv_item_id = sp_ap_root["location_to_item"].get(std::to_string(idx), 0).asInt64();
            if (recv_item_id == 0) continue;
            Json::Value item;
            item["item"] = recv_item_id;
            item["location"] = idx;
            item["player"] = ap_player_id;
            fake_msg[0]["items"].append(item);
        }
        std::string req;
        parse_response(writer.write(fake_msg), req);

        fake_msg.clear();
        fake_msg[0]["cmd"] = "RoomUpdate";
        fake_msg[0]["checked_locations"] = Json::arrayValue;
        for (int64_t idx : new_locations) {
            fake_msg[0]["checked_locations"].append(idx);
            sp_save_root["checked_locations"].append(idx);
        }
        WriteFileJSON(sp_save_root, sp_save_path);
        parse_response(writer.write(fake_msg), req);
    }
}
void AP_SendLocationScouts(std::set<int64_t> const& locations, int create_as_hint) {
    locations_scouted.insert(locations.begin(), locations.end());
    if (multiworld) {
        if (webSocket.getReadyState() == ix::ReadyState::Open && auth) {
            Json::Value req_t;
            req_t[0]["cmd"] = "LocationScouts";
            req_t[0]["locations"] = Json::arrayValue;
            for (int64_t loc : locations) req_t[0]["locations"].append(loc);
            req_t[0]["create_as_hint"] = create_as_hint;
            APSend(writer.write(req_t));
        } else {
            pending_location_scouts[create_as_hint].insert(locations.begin(), locations.end());
        }
    } else {
        Json::Value fake_msg;
        fake_msg[0]["cmd"] = "LocationInfo";
        fake_msg[0]["locations"] = Json::arrayValue;
        for (int64_t loc : locations) {
            Json::Value netitem;
            netitem["item"] = sp_ap_root["location_to_item"].get(std::to_string(loc), 0).asInt64();
            netitem["location"] = loc;
            netitem["player"] = ap_player_id;
            netitem["flags"] = 0b001; // Hardcoded for SP seeds.
            fake_msg[0]["locations"].append(netitem);
        }
        std::string req;
        parse_response(writer.write(fake_msg), req);
    }
}
void AP_UpdateTags(std::vector<std::string> const& tags)
{
    lib_room_info.tags = tags;
    if (!multiworld) return;
    Json::Value req_t;
    req_t[0]["cmd"] = "ConnectUpdate";
    req_t[0]["tags"] = Json::arrayValue;
    for (std::string tag : tags) {
        req_t[0]["tags"].append(tag);
    }
    APSend(writer.write(req_t));
}

void AP_StoryComplete() {
    if (!multiworld) return;
    Json::Value req_t;
    req_t[0]["cmd"] = "StatusUpdate";
    req_t[0]["status"] = 30; //CLIENT_GOAL
    APSend(writer.write(req_t));
}


void AP_StatusUpdate(int status) {
    if (!multiworld) return;
    Json::Value req_t;
    req_t[0]["cmd"] = "StatusUpdate";
    req_t[0]["status"] = status;
    if (webSocket.getReadyState() == ix::ReadyState::Open && auth) {
        APSend(writer.write(req_t));
        pending_status = -1;
    } else {
        pending_status = status;
    }
}

void AP_Sync() {
    if (!multiworld) return;
    Json::Value req_t;
    req_t[0]["cmd"] = "Sync";
    APSend(writer.write(req_t));
}

void AP_UpdateHint(int player, int64_t location, AP_HintStatus status) {
    if (multiworld && webSocket.getReadyState() == ix::ReadyState::Open && auth) {
        Json::Value req_t;
        req_t[0]["cmd"] = "UpdateHint";
        req_t[0]["player"] = player;
        req_t[0]["location"] = location;
        req_t[0]["status"] = static_cast<int>(status);
        APSend(writer.write(req_t));
    } else if (multiworld) {
        pending_hint_updates.emplace_back(player, location, status);
    }
}

void AP_CreateHints(std::set<int64_t> const& locations, int player, AP_HintStatus status) {
    if (player < 0) player = ap_player_id;
    if (multiworld && webSocket.getReadyState() == ix::ReadyState::Open && auth) {
        Json::Value req_t;
        req_t[0]["cmd"] = "CreateHints";
        req_t[0]["locations"] = Json::arrayValue;
        req_t[0]["player"] = player;
        for (int64_t location : locations) req_t[0]["locations"].append(location);
        if (status != AP_HintStatus::Unspecified)
            req_t[0]["status"] = static_cast<int>(status);
        APSend(writer.write(req_t));
    } else if (multiworld) {
        pending_hints[{player, status}].insert(locations.begin(), locations.end());
    }
}

const std::vector<AP_NetworkItem>& AP_GetItemsReceived() { return items_received; }
const std::map<int, AP_NetworkPlayer>& AP_GetPlayers() { return map_players; }
bool AP_GetSlot(int slot, AP_NetworkSlot* info) {
    auto itr = map_slots.find(slot);
    if (itr == map_slots.end()) return false;
    if (info != nullptr) *info = itr->second;
    return true;
}

void AP_DeathLinkSend(const std::string &cause) {
    if (!enable_deathlink || !multiworld) return;
    if (cur_deathlink_amnesty > 0) {
        cur_deathlink_amnesty--;
        return;
    }
    cur_deathlink_amnesty = deathlink_amnesty;
    std::chrono::time_point<std::chrono::system_clock> timestamp = std::chrono::system_clock::now();
    AP_Bounce b;
    Json::Value v;
    v["time"] = (int64_t)std::chrono::duration_cast<std::chrono::seconds>(timestamp.time_since_epoch()).count();
    v["source"] = ap_player_name; // Name and Shame >:D
    if (!cause.empty())
    {
        std::string cause_pname = cause;
        constexpr std::string_view pname_you{"%YOU%"};
        size_t pname_marker = cause_pname.find(pname_you);

        if (pname_marker != std::string::npos)
            cause_pname.replace(pname_marker, pname_you.size(), ap_player_name);
        v["cause"] = cause_pname;
    }
    b.data = writer.write(v);
    b.games = nullptr;
    b.slots = nullptr;
    std::vector<std::string> tags = {std::string("DeathLink")};
    b.tags = &tags;
    AP_SendBounce(b);
}

void AP_EnableQueueItemRecvMsgs(bool b) {
    queueitemrecvmsg = b;
}

void AP_SetLoggingCallback(std::function<void(std::string)> f_log) {
    logfunc = f_log;
}

void AP_SetItemClearCallback(std::function<void()> f_itemclr) {
    resetItemValues = f_itemclr;
}

void AP_SetItemRecvCallback(std::function<void(int64_t,bool)> f_itemrecv) {
    getitemfunc = f_itemrecv;
}

void AP_SetLocationCheckedCallback(std::function<void(int64_t)> f_locrecv) {
    checklocfunc = f_locrecv;
}


void AP_SetLocationInfoCallback(std::function<void(std::vector<AP_NetworkItem>)> f_locinfrecv) {
    locinfofunc = f_locinfrecv;
}

void AP_SetSocketConnectedCallback(std::function<void()> f_connected) { socketconnectedfunc = f_connected; }
void AP_SetSocketDisconnectedCallback(std::function<void()> f_disconnected) { socketdisconnectedfunc = f_disconnected; }
void AP_SetSocketErrorCallback(std::function<void(std::string)> f_error) { socketerrorfunc = f_error; }
void AP_SetSlotConnectedCallback(std::function<void()> f_connected) { slotconnectedfunc = f_connected; }
void AP_SetSlotDisconnectedCallback(std::function<void()> f_disconnected) { slotdisconnectedfunc = f_disconnected; }
void AP_SetConnectionRefusedCallback(std::function<void(std::vector<std::string>)> f_refused) { connectionrefusedfunc = f_refused; }
void AP_SetRoomInfoCallback(std::function<void()> f_roominfo) { roominfofunc = f_roominfo; }
void AP_SetRoomUpdateCallback(std::function<void()> f_roomupdate) { roomupdatefunc = f_roomupdate; }
void AP_SetItemsReceivedCallback(std::function<void(std::vector<AP_NetworkItem>)> f_itemsrecv) { itemsrecvfunc = f_itemsrecv; }
void AP_SetDataPackageChangedCallback(std::function<void()> f_datapkg) { datapkgfunc = f_datapkg; }
void AP_SetPrintCallback(std::function<void(std::string)> f_print) { printfunc = f_print; }
void AP_SetPrintJSONCallback(std::function<void(std::string)> f_printjson) { printjsonfunc = f_printjson; }
void AP_SetRetrievedCallback(std::function<void(std::string)> f_retrieved) { retrievedfunc = f_retrieved; }

void AP_SetServerDataNotifyCallback(std::function<void(std::string, std::string)> f_notify) { stored_data_notifyfunc = f_notify; }
bool AP_GetStoredServerData(std::string key, std::string* value) {
    auto itr = stored_data.find(key);
    if (itr == stored_data.end()) return false;
    if (value != nullptr) *value = itr->second;
    return true;
}

void AP_SetReceiveOwnLocations(bool b) { receive_own_locations = b; }
bool AP_GetReceiveOwnLocations() { return receive_own_locations; }
std::set<int64_t> AP_GetLocationsChecked() { return locations_checked; }
std::set<int64_t> AP_GetLocationsScouted() { return locations_scouted; }
std::set<int64_t> AP_GetMissingLocations() { return missing_locations; }
std::set<int64_t> AP_GetCheckedLocations() { return checked_locations; }
std::set<int64_t> AP_GetServerLocations() {
    std::set<int64_t> result = missing_locations;
    result.insert(checked_locations.begin(), checked_locations.end());
    return result;
}
bool AP_GetLocationInfo(int64_t location, AP_NetworkItem* info) {
    auto itr = locations_info.find(location);
    if (itr == locations_info.end()) return false;
    if (info != nullptr) *info = itr->second;
    return true;
}

void AP_SetDeathLinkRecvCallback(std::function<void()> f_deathrecv) {
    recvdeath = [f_deathrecv](std::string, std::string){ f_deathrecv(); };
}
void AP_SetDeathLinkRecvCallback(std::function<void(std::string, std::string)> f_deathrecv) {
    recvdeath = f_deathrecv;
}

void AP_RegisterSlotDataIntCallback(std::string key, std::function<void(int)> f_slotdata) {
    map_slotdata_callback_int[key] = f_slotdata;
}

void AP_RegisterSlotDataRawCallback(std::string key, std::function<void(std::string)> f_slotdata) {
    map_slotdata_callback_raw[key] = f_slotdata;
}

void AP_RegisterSlotDataMapIntIntCallback(std::string key, std::function<void(std::map<int,int>)> f_slotdata) {
    map_slotdata_callback_mapintint[key] = f_slotdata;
}

void AP_SetDeathLinkSupported(bool supdeathlink) {
    deathlinksupported = supdeathlink;
}

bool AP_DeathLinkPending() {
    return deathlinkstat;
}

void AP_DeathLinkClear() {
    deathlinkstat = false;
}

bool AP_IsMessagePending() {
    return !messageQueue.empty();
}

AP_Message* AP_GetLatestMessage() {
    return messageQueue.front();
}

void AP_ClearLatestMessage() {
    if (AP_IsMessagePending()) {
        delete messageQueue.front();
        messageQueue.pop_front();
    }
}

void AP_Say(std::string text) {
    Json::Value req_t;
    req_t[0]["cmd"] = "Say";
    req_t[0]["text"] = text;
    APSend(writer.write(req_t));
}

int AP_GetRoomInfo(AP_RoomInfo* client_roominfo) {
    if (!auth) return 1;
    *client_roominfo = lib_room_info;
    return 0;
}

AP_ConnectionStatus AP_GetConnectionStatus() {
    if (!multiworld && auth) return AP_ConnectionStatus::Authenticated;
    if (refused) {
        return AP_ConnectionStatus::ConnectionRefused;
    }
    if (webSocket.getReadyState() == ix::ReadyState::Open) {
        if (auth) {
            return AP_ConnectionStatus::Authenticated;
        } else {
            return AP_ConnectionStatus::Connected;
        }
    }
    return AP_ConnectionStatus::Disconnected;
}

std::uint64_t AP_GetUUID() {
    return ap_uuid;
}

int AP_GetPlayerID() {
    return ap_player_id;
}

void AP_BulkSetServerData(AP_SetServerDataRequest* request) {
    request->status = AP_RequestStatus::Pending;

    Json::Value req_t;
    req_t["cmd"] = "Set";
    req_t["key"] = request->key;
    switch (request->type) {
        case AP_DataType::Int:
            for (int i = 0; i < request->operations.size(); i++) {
                req_t["operations"][i]["operation"] = request->operations[i].operation;
                req_t["operations"][i]["value"] = *((int*)request->operations[i].value);
            }
            if (request->default_value != nullptr) {
                req_t["default"] = *((int*)request->default_value);
            }
            break;
        case AP_DataType::Double:
            for (int i = 0; i < request->operations.size(); i++) {
                req_t["operations"][i]["operation"] = request->operations[i].operation;
                req_t["operations"][i]["value"] = *((double*)request->operations[i].value);
            }
            if (request->default_value != nullptr) {
                req_t["default"] = *((double*)request->default_value);
            }
            break;
        default:
            for (int i = 0; i < request->operations.size(); i++) {
                req_t["operations"][i]["operation"] = request->operations[i].operation;
                Json::Value data;
                reader.parse((*(std::string*)request->operations[i].value), data);
                req_t["operations"][i]["value"] = data;
            }
            if (request->default_value != nullptr) {
                Json::Value default_val_json;
                reader.parse(*((std::string*)request->default_value), default_val_json);
                req_t["default"] = default_val_json;
            }
            break;
    }
    req_t["want_reply"] = request->want_reply;
    map_serverdata_typemanage[request->key] = request->type;

    queue_server_data.push({req_t,&request->status});
}

void AP_CommitServerData() {
    Json::Value req = Json::arrayValue;
    while (!queue_server_data.empty()) {
        std::pair<Json::Value, AP_RequestStatus*> request = queue_server_data.front();
        req.append(request.first);
        std::string key = req[req.size()-1]["cmd"].asString();
        if (key == "Set" || key == "SetNotify") // Set has local completion at this stage
            *(request.second) = AP_RequestStatus::Done;
        if (!multiworld)
            resolveDataStorageOp(request.first);
        queue_server_data.pop();
    }
    if (multiworld) APSend(writer.write(req));
}

void AP_SetServerData(AP_SetServerDataRequest* request) {
    AP_BulkSetServerData(request);
    AP_CommitServerData();
}

void AP_RegisterSetReplyCallback(std::function<void(AP_SetReply)> f_setreply) {
    setreplyfunc = f_setreply;
}

void AP_SetNotify(std::map<std::string,AP_DataType> keylist, bool requestCurrentValue) {
    for (const auto& keytypepair : keylist)
        stored_data_notification_keys.insert(keytypepair.first);

    Json::Value req_t;
    req_t["cmd"] = "SetNotify";

    std::string zero = "0";
    std::string emptyJson = "{}";

    int i = 0;
    std::vector<AP_SetServerDataRequest> requests;

    for (std::pair<std::string,AP_DataType> keytypepair : keylist) {
        req_t["keys"][i] = keytypepair.first;
        map_serverdata_typemanage[keytypepair.first] = keytypepair.second;

        i++;

        if (requestCurrentValue) {
            AP_SetServerDataRequest setDefaultRequest;
            setDefaultRequest.key = keytypepair.first;
            setDefaultRequest.type = keytypepair.second;
            setDefaultRequest.want_reply = true;
            switch (keytypepair.second) {
                case AP_DataType::Int:
                case AP_DataType::Double:
                    setDefaultRequest.operations = {{"default", &zero}};
                    setDefaultRequest.default_value = &zero;
                    break;
                case AP_DataType::Raw:
                    setDefaultRequest.operations = {{"default", &zero}};
                    setDefaultRequest.default_value = &emptyJson;
                    break;
            }
            
            requests.push_back(setDefaultRequest);
        }
    }

    AP_RequestStatus requestStatus;
    queue_server_data.push({req_t, &requestStatus});

    for (AP_SetServerDataRequest& request : requests)
        AP_BulkSetServerData(&request);

    AP_CommitServerData();
}

void AP_SetNotify(std::string key, AP_DataType type, bool requestCurrentValue) {
    std::map<std::string,AP_DataType> keylist;
    keylist[key] = type;
    AP_SetNotify(keylist, requestCurrentValue);
}

void AP_BulkGetServerData(AP_GetServerDataRequest* request) {
    request->status = AP_RequestStatus::Pending;

    if (map_server_data.find(request->key) != map_server_data.end()) return;

    map_server_data[request->key] = request;

    Json::Value req_t;
    req_t["cmd"] = "Get";
    req_t["keys"][0] = request->key;

    queue_server_data.push({req_t,&request->status});
}

void AP_GetServerData(AP_GetServerDataRequest *request) {
    AP_BulkGetServerData(request);
    AP_CommitServerData();
}

std::string AP_GetPrivateServerDataPrefix() {
    return "APCpp" + std::to_string(ap_player_name_hash) + "APCpp" + std::to_string(ap_player_id) + "APCpp";
}

void AP_SendBounce(AP_Bounce bounce) {
    Json::Value req_t;
    req_t[0]["cmd"] = "Bounce";

    // Add targets for bounce, if requested
    #define ADD_TARGETS( targets ) \
            if (bounce.targets != nullptr && !bounce.targets->empty()) { \
                for (int i = 0; i < bounce.targets->size(); i++) { \
                    req_t[0][#targets].append((*(bounce.targets))[i]); \
                } \
            }
    ADD_TARGETS(games)
    ADD_TARGETS(slots)
    ADD_TARGETS(tags)
    #undef ADD_TARGETS

    Json::Value data;
    reader.parse(bounce.data, data);
    req_t[0]["data"] = data;
    APSend(writer.write(req_t));
}

void AP_RegisterBouncedCallback(std::function<void(AP_Bounce)> f_bounced) {
    bouncedfunc = f_bounced;
}

// PRIV

void AP_Init_Generic() {
    ap_player_name_hash = std::hash<std::string>{}(ap_player_name);
    std::filesystem::create_directory(datapkg_cache_dir);
}

bool parse_response(std::string msg, std::string &request) {
    Json::Value root;
    reader.parse(msg, root);
    for (unsigned int i = 0; i < root.size(); i++) {
        std::string cmd = root[i]["cmd"].asString();
        if (cmd == "RoomInfo") {
            lib_room_info.version.major = root[i]["version"]["major"].asInt();
            lib_room_info.version.minor = root[i]["version"]["minor"].asInt();
            lib_room_info.version.build = root[i]["version"]["build"].asInt();
            lib_room_info.generator_version.major = root[i]["generator_version"]["major"].asInt();
            lib_room_info.generator_version.minor = root[i]["generator_version"]["minor"].asInt();
            lib_room_info.generator_version.build = root[i]["generator_version"]["build"].asInt();
            lib_room_info.games.clear();
            for (auto itr : root[i]["games"]) lib_room_info.games.push_back(itr.asString());
            std::vector<std::string> serv_tags;
            for (auto itr : root[i]["tags"]) {
                serv_tags.push_back(itr.asString());
            }
            lib_room_info.tags = serv_tags;
            lib_room_info.password_required = root[i]["password"].asBool();
            std::map<std::string,int> serv_permissions;
            for (auto itr : root[i]["permissions"].getMemberNames()) {
                serv_permissions[itr] = root[i]["permissions"][itr].asInt();
            }
            lib_room_info.permissions = serv_permissions;
            lib_room_info.hint_cost = root[i]["hint_cost"].asInt();
            lib_room_info.location_check_points = root[i]["location_check_points"].asInt();
            lib_room_info.hint_points = root[i].get("hint_points", 0).asInt();
            lib_room_info.location_count = 0;
            std::map<std::string,std::string> serv_datapkg_checksums;
            for (auto itr : root[i]["datapackage_checksums"].getMemberNames()) {
                serv_datapkg_checksums[itr] = root[i]["datapackage_checksums"][itr].asString();
            }
            lib_room_info.datapackage_checksums = serv_datapkg_checksums;
            lib_room_info.seed_name = root[i]["seed_name"].asString();
            lib_room_info.time = root[i]["time"].asDouble();
            if (roominfofunc) roominfofunc();

            if (!auth) {
                Json::Value req_t;
                ap_uuid = rando();
                req_t[0]["cmd"] = "Connect";
                req_t[0]["game"] = ap_game;
                req_t[0]["name"] = ap_player_name;
                req_t[0]["password"] = ap_passwd;
                req_t[0]["uuid"] = ap_uuid;
                req_t[0]["tags"] = Json::arrayValue;
                req_t[0]["version"]["major"] = client_version.major;
                req_t[0]["version"]["minor"] = client_version.minor;
                req_t[0]["version"]["build"] = client_version.build;
                req_t[0]["version"]["class"] = "Version";
                req_t[0]["items_handling"] = 7; // Full Remote
                request = writer.write(req_t);
                return true;
            }
        } else if (cmd == "Connected") {
            // Avoid inconsistency if we disconnected before
            logfunc("AP: Authenticated");
            ap_player_id = root[i]["slot"].asInt(); // MUST be called before resetitemvalues, otherwise PrivateServerDataPrefix, GetPlayerID return broken values!
            ap_player_team = root[i]["team"].asInt();
            resetItemValues();

            checked_locations.clear();
            missing_locations.clear();
            for (unsigned int j = 0; j < root[i]["checked_locations"].size(); j++) {
                int64_t loc_id = root[i]["checked_locations"][j].asInt64();
                checked_locations.insert(loc_id);
                if (checklocfunc) checklocfunc(loc_id);
            }
            for (unsigned int j = 0; j < root[i]["missing_locations"].size(); j++) {
                missing_locations.insert(root[i]["missing_locations"][j].asInt64());
            }
            lib_room_info.hint_points = root[i].get("hint_points", lib_room_info.hint_points).asInt();
            lib_room_info.location_count = static_cast<int>(checked_locations.size() + missing_locations.size());
            for (unsigned int j = 0; j < root[i]["players"].size(); j++) {
                AP_NetworkPlayer player = {
                    root[i]["players"][j]["team"].asInt(),
                    root[i]["players"][j]["slot"].asInt(),
                    root[i]["players"][j]["name"].asString(),
                    root[i]["players"][j]["alias"].asString(),
                    "PLACEHOLDER"
                };
                player.game = root[i]["slot_info"][std::to_string(player.slot)]["game"].asString();
                map_players[root[i]["players"][j]["slot"].asInt()] = player;
                teams_set.insert(root[i]["players"][j]["team"].asInt());
            }
            map_slots.clear();
            if (root[i]["slot_info"].isObject()) {
                for (auto slot_name : root[i]["slot_info"].getMemberNames()) {
                    int slot = std::stoi(slot_name);
                    const Json::Value& slot_json = root[i]["slot_info"][slot_name];
                    AP_NetworkSlot info;
                    info.name = slot_json.get("name", "").asString();
                    info.game = slot_json.get("game", "").asString();
                    info.type = static_cast<AP_SlotType>(slot_json.get("type", 1).asInt());
                    for (auto member : slot_json.get("group_members", Json::arrayValue))
                        info.members.push_back(member.asInt());
                    map_slots[slot] = info;
                }
            }

            if (slotconnectedfunc) slotconnectedfunc();

            if (gifting_supported) {
                // Order is important, Motherboxes must be retrieved before personal box for auto-rejection reasons, do not combine
                std::map<std::string,AP_DataType> giftMotherBoxKeys;
                for (int team : teams_set)
                    giftMotherBoxKeys.emplace("GiftBoxes;" + std::to_string(team), AP_DataType::Raw); 
                AP_SetNotify(giftMotherBoxKeys, true);
                AP_SetNotify("GiftBox;" + std::to_string(ap_player_team) + ";" + std::to_string(ap_player_id), AP_DataType::Raw, true);
            }

            if ((root[i]["slot_data"].get("death_link", false).asBool() || root[i]["slot_data"].get("DeathLink", false).asBool()) && deathlinksupported) enable_deathlink = true;
            if (root[i]["slot_data"]["death_link_amnesty"] != Json::nullValue)
                deathlink_amnesty = root[i]["slot_data"].get("death_link_amnesty", 0).asInt();
            else if (root[i]["slot_data"]["DeathLink_Amnesty"] != Json::nullValue)
                deathlink_amnesty = root[i]["slot_data"].get("DeathLink_Amnesty", 0).asInt();
            cur_deathlink_amnesty = deathlink_amnesty;
            for (auto slot_itr = root[i]["slot_data"].begin(); slot_itr != root[i]["slot_data"].end(); ++slot_itr) {
                std::string key = slot_itr.key().asString();
                if (map_slotdata_callback_int.count(key)) {
                    map_slotdata_callback_int[key](slot_itr->asInt());
                } else if (map_slotdata_callback_raw.count(key)) {
                    map_slotdata_callback_raw[key](writer.write(*slot_itr));
                } else if (map_slotdata_callback_mapintint.count(key)) {
                    std::map<int,int> out;
                    for (auto map_idx_itr : slot_itr->getMemberNames()) {
                        out[std::stoi(map_idx_itr)] = (*slot_itr)[map_idx_itr].asInt();
                    }
                    map_slotdata_callback_mapintint[key](out);
                } else {
                    if (key != "death_link" && key != "death_link_amnesty" && key != "DeathLink" && key != "DeathLink_Amnesty")
                        logfunc("AP: Warning: Unmapped slot data with key '" + key + "'!");
                }
            }

            resync_serverdata_request.key = "APCppLastRecv" + ap_player_name + std::to_string(ap_player_id);
            resync_serverdata_request.value = &last_item_idx;
            resync_serverdata_request.type = AP_DataType::Int;
            AP_GetServerData(&resync_serverdata_request);

            Json::Value req_t = Json::arrayValue;
            if (pending_status >= 0) {
                Json::Value status_update;
                status_update["cmd"] = "StatusUpdate";
                status_update["status"] = pending_status;
                req_t.append(status_update);
                pending_status = -1;
            }
            if (enable_deathlink && deathlinksupported) {
                Json::Value setdeathlink;
                setdeathlink["cmd"] = "ConnectUpdate";
                setdeathlink["tags"][0] = "DeathLink";
                req_t.append(setdeathlink);
            }

            for (auto &game_pkg : lib_room_info.datapackage_checksums) {
                if (!loadDataPkg(game_pkg.first, game_pkg.second)) {
                    datapkg_outdated_games.insert(game_pkg.first);
                }
            }

            if (!pending_location_checks.empty()) {
                Json::Value checks;
                checks[0]["cmd"] = "LocationChecks";
                checks[0]["locations"] = Json::arrayValue;
                for (int64_t location : pending_location_checks) checks[0]["locations"].append(location);
                pending_location_checks.clear();
                req_t.append(checks[0]);
            }
            if (!pending_location_scouts.empty()) {
                for (auto& pair : pending_location_scouts) {
                    Json::Value scouts;
                    scouts["cmd"] = "LocationScouts";
                    scouts["locations"] = Json::arrayValue;
                    scouts["create_as_hint"] = pair.first;
                    for (int64_t location : pair.second) scouts["locations"].append(location);
                    req_t.append(scouts);
                }
                pending_location_scouts.clear();
            }
            for (auto& hint : pending_hint_updates) {
                Json::Value update;
                update["cmd"] = "UpdateHint";
                update["player"] = std::get<0>(hint);
                update["location"] = std::get<1>(hint);
                update["status"] = static_cast<int>(std::get<2>(hint));
                req_t.append(update);
            }
            pending_hint_updates.clear();
            for (auto& pair : pending_hints) {
                Json::Value hints;
                hints["cmd"] = "CreateHints";
                hints["locations"] = Json::arrayValue;
                hints["player"] = pair.first.first;
                if (pair.first.second != AP_HintStatus::Unspecified)
                    hints["status"] = static_cast<int>(pair.first.second);
                for (int64_t location : pair.second) hints["locations"].append(location);
                req_t.append(hints);
            }
            pending_hints.clear();

            // getDataPkgRequest returns either a Sync or GetDataPackage packet
            req_t.append(getDataPkgRequest());
            request = writer.write(req_t);
            return true;
        } else if (cmd == "DataPackage") {
            cacheDataPkgs(root[i]["data"]);
            if (datapkgfunc && datapkg_outdated_games.empty()) datapkgfunc();
            Json::Value req_t = Json::arrayValue;
            req_t.append(getDataPkgRequest());
            request = writer.write(req_t);
            return true;
        } else if (cmd == "Retrieved") {
            if (retrievedfunc) retrievedfunc(writer.write(root[i]));
            for (auto itr : root[i]["keys"].getMemberNames()) {
                stored_data[itr] = writer.write(root[i]["keys"][itr]);
                if (stored_data_notifyfunc && stored_data_notification_keys.count(itr))
                    stored_data_notifyfunc(itr, stored_data[itr]);
                if (!map_server_data.count(itr)) continue;
                AP_GetServerDataRequest* target = map_server_data[itr];
                switch (target->type) {
                    case AP_DataType::Int:
                        *((int*)target->value) = root[i]["keys"][itr].asInt();
                        break;
                    case AP_DataType::Double:
                        *((double*)target->value) = root[i]["keys"][itr].asDouble();
                        break;
                    case AP_DataType::Raw:
                        *((std::string*)target->value) = writer.write(root[i]["keys"][itr]);
                        break;
                }
                target->status = AP_RequestStatus::Done;
                map_server_data.erase(itr);
            }
        } else if (cmd == "SetReply") {
            if (gifting_supported && root[i]["key"].asString().rfind("GiftBox", 0) == 0) {
                // Reserved by library. Used for Gifting API
                std::string raw_val;
                std::string raw_orig_val;
                AP_SetReply setreply;
                raw_val =  writer.write(root[i]["value"]);
                raw_orig_val = writer.write(root[i]["original_value"]);
                setreply.key = root[i]["key"].asString();
                setreply.value = &raw_val;
                setreply.original_value = &raw_orig_val;
                handleGiftAPISetReply(setreply);
            }
            if (setreplyfunc) {
                int int_val;
                int int_orig_val;
                double dbl_val;
                double dbl_orig_val;
                std::string raw_val;
                std::string raw_orig_val;
                AP_SetReply setreply;
                setreply.key = root[i]["key"].asString();
                switch (map_serverdata_typemanage[setreply.key]) {
                    case AP_DataType::Int:
                        int_val = root[i]["value"].asInt();
                        int_orig_val = root[i]["original_value"].asInt();
                        setreply.value = &int_val;
                        setreply.original_value = &int_orig_val;
                        break;
                    case AP_DataType::Double:
                        dbl_val = root[i]["value"].asDouble();
                        dbl_orig_val = root[i]["original_value"].asDouble();
                        setreply.value = &dbl_val;
                        setreply.original_value = &dbl_orig_val;
                        break;
                    default:
                        raw_val =  writer.write(root[i]["value"]);
                        raw_orig_val = writer.write(root[i]["original_value"]);
                        setreply.value = &raw_val;
                        setreply.original_value = &raw_orig_val;
                        break;
                }
                setreplyfunc(setreply);
            }
        } else if (cmd == "Print") {
            if (printfunc) printfunc(root[i].get("text", "").asString());
        } else if (cmd == "PrintJSON") {
            if (printjsonfunc) printjsonfunc(writer.write(root[i]));
            const std::string printType = root[i].get("type","").asString();
            if (printType == "ItemSend" || printType == "ItemCheat") {
                // Filter out itemrecv messages, which would otherwise be duplicated from the itemrecv callback
                if (getPlayer(0, root[i]["receiving"].asInt()).alias == getPlayer(0, ap_player_id).alias) continue;
                AP_NetworkPlayer recv_player = getPlayer(0, root[i]["receiving"].asInt());
                AP_ItemSendMessage* msg = new AP_ItemSendMessage;
                msg->type = AP_MessageType::ItemSend;
                msg->item = getItemName(recv_player.game, root[i]["item"]["item"].asInt64());
                msg->recvPlayer = recv_player.alias;
                msg->text = msg->item + std::string(" was sent to ") + msg->recvPlayer;
                messageQueue.push_back(msg);
            } else if (printType == "Hint") {
                AP_NetworkPlayer send_player = getPlayer(0, root[i]["item"]["player"].asInt());
                AP_NetworkPlayer recv_player = getPlayer(0, root[i]["receiving"].asInt());
                AP_HintMessage* msg = new AP_HintMessage;
                msg->type = AP_MessageType::Hint;
                msg->item = getItemName(recv_player.game,root[i]["item"]["item"].asInt64());
                msg->sendPlayer = send_player.alias;
                msg->recvPlayer = recv_player.alias;
                msg->location = getLocationName(send_player.game, root[i]["item"]["location"].asInt64());
                msg->checked = root[i]["found"].asBool();
                msg->text = std::string("Item ") + msg->item + std::string(" from ") + msg->sendPlayer + std::string(" to ") + msg->recvPlayer + std::string(" at ") + msg->location + std::string((msg->checked ? " (Checked)" : " (Unchecked)"));
                messageQueue.push_back(msg);
            } else if (printType == "Countdown") {
                AP_CountdownMessage* msg = new AP_CountdownMessage;
                msg->type = AP_MessageType::Countdown;
                if (root[i]["countdown"].isInt()) {
                    msg->timer = root[i]["countdown"].asInt();
                } else {
                    msg->timer = 0;
                }
                msg->text = root[i]["data"][0]["text"].asString();
                messageQueue.push_back(msg);
            } else if (printType == "Chat") {
                AP_NetworkPlayer sender = getPlayer(0, root[i]["slot"].asInt());
                AP_ChatMessage* msg = new AP_ChatMessage;
                msg->type = AP_MessageType::Chat;
                msg->player = sender.alias;
                msg->message = root[i]["message"].asString();
                msg->text = msg->player + ": " + msg->message;
                messageQueue.push_back(msg);
            } else if (printType == "ServerChat") {
                AP_ServerChatMessage* msg = new AP_ServerChatMessage;
                msg->type = AP_MessageType::ServerChat;
                msg->message = root[i]["message"].asString();
                msg->text = "[Server]: " + msg->message;
                messageQueue.push_back(msg);
            } else {
                AP_Message* msg = new AP_Message;
                msg->text = "";
                for (auto itr : root[i]["data"]) {
                    if (itr.get("type","").asString() == "player_id") {
                        msg->text += getPlayer(0, itr["text"].asInt()).alias;
                    } else if (itr.get("text","") != "") {
                        msg->text += itr["text"].asString();
                    }
                }
                messageQueue.push_back(msg);
            }
        } else if (cmd == "LocationInfo") {
            std::vector<AP_NetworkItem> locations;
            for (unsigned int j = 0; j < root[i]["locations"].size(); j++) {
                AP_NetworkItem item;
                item.item = root[i]["locations"][j]["item"].asInt64();
                item.location = root[i]["locations"][j]["location"].asInt64();
                AP_NetworkPlayer player = getPlayer(0, root[i]["locations"][j]["player"].asInt());
                item.player = player.slot;
                item.flags = root[i]["locations"][j].get("flags", 0).asInt();
                item.index = -1;
                item.itemName = getItemName(player.game, item.item);
                item.locationName = getLocationName(ap_game, item.location);
                item.playerName = player.alias;
                locations_info[item.location] = item;
                locations_scouted.insert(item.location);
                locations.push_back(item);
            }
            if (locinfofunc) {
                locinfofunc(locations);
            } else {
                logfunc("AP: Received LocationInfo but no handler registered!");
            }
        } else if (cmd == "ReceivedItems") {
            int item_idx = root[i]["index"].asInt();
            bool notify;
            std::vector<AP_NetworkItem> received;
            int next_idx = item_idx;
            for (unsigned int j = 0; j < root[i]["items"].size(); j++) {
                AP_NetworkItem item;
                item.item = root[i]["items"][j]["item"].asInt64();
                item.location = root[i]["items"][j]["location"].asInt64();
                item.player = root[i]["items"][j]["player"].asInt();
                item.flags = root[i]["items"][j].get("flags", 0).asInt();
                item.index = next_idx++;
                AP_NetworkPlayer sender = getPlayer(0, item.player);
                item.itemName = getItemName(sender.game, item.item);
                item.locationName = getLocationName(sender.game, item.location);
                item.playerName = sender.alias;
                notify = (item_idx == 0 && last_item_idx <= j && multiworld) || item_idx != 0;
                if (getitemfunc) getitemfunc(item.item, notify);
                received.push_back(item);
                if (queueitemrecvmsg && notify) {
                    AP_ItemRecvMessage* msg = new AP_ItemRecvMessage;
                    msg->type = AP_MessageType::ItemRecv;
                    msg->item = item.itemName;
                    msg->sendPlayer = item.playerName;
                    msg->text = std::string("Received ") + msg->item + std::string(" from ") + msg->sendPlayer;
                    messageQueue.push_back(msg);
                }
                if (item.index >= static_cast<int>(items_received.size()))
                    items_received.resize(item.index + 1);
                items_received[item.index] = item;
            }
            if (item_idx == 0) {
                last_item_idx = static_cast<uint32_t>(root[i]["items"].size());
            } else {
                last_item_idx = item_idx + static_cast<uint32_t>(root[i]["items"].size());
            }
            if (itemsrecvfunc && !received.empty()) itemsrecvfunc(received);
            AP_SetServerDataRequest request;
            request.key = "APCppLastRecv" + ap_player_name + std::to_string(ap_player_id);
            AP_DataStorageOperation replac;
            replac.operation = "replace";
            replac.value = &last_item_idx;
            std::vector<AP_DataStorageOperation> operations;
            operations.push_back(replac);
            request.operations = operations;
            request.default_value = 0;
            request.type = AP_DataType::Int;
            request.want_reply = false;
            AP_SetServerData(&request);
        } else if (cmd == "RoomUpdate") {
            std::vector<int64_t> newly_checked;
            if (root[i]["checked_locations"].isArray()) {
                for (unsigned int j = 0; j < root[i]["checked_locations"].size(); j++) {
                    int64_t loc_id = root[i]["checked_locations"][j].asInt64();
                    if (checked_locations.insert(loc_id).second) {
                        missing_locations.erase(loc_id);
                        newly_checked.push_back(loc_id);
                    }
                }
            }
            if (root[i]["players"].isArray()) {
                for (auto itr : root[i]["players"]) {
                    int slot = itr["slot"].asInt();
                    AP_NetworkPlayer player = {
                        itr["team"].asInt(),
                        slot,
                        itr["name"].asString(),
                        itr["alias"].asString(),
                        map_slots.count(slot) ? map_slots[slot].game : (map_players.count(slot) ? map_players[slot].game : "PLACEHOLDER")
                    };
                    map_players[slot] = player;
                    teams_set.insert(player.team);
                }
            }
            if (root[i]["hint_points"].isInt()) lib_room_info.hint_points = root[i]["hint_points"].asInt();
            if (root[i]["permissions"].isObject()) {
                for (auto itr : root[i]["permissions"].getMemberNames())
                    lib_room_info.permissions[itr] = root[i]["permissions"][itr].asInt();
            }
            if (!newly_checked.empty() && checklocfunc) {
                for (int64_t location : newly_checked) checklocfunc(location);
            }
            if (roomupdatefunc) roomupdatefunc();
        } else if (cmd == "ConnectionRefused") {
            auth = false;
            refused = true;
            std::vector<std::string> errors;
            for (auto error : root[i]["errors"]) errors.push_back(error.asString());
            if (connectionrefusedfunc) connectionrefusedfunc(errors);
            logfunc("AP: Archipelago Server has refused connection. Check Password / Name / IP and restart the Game.");
            fflush(stdout);
        } else if (cmd == "Bounced") {
            if (!enable_deathlink && !bouncedfunc) continue;
            if (!bouncedfunc) {
                // Only do native DeathLink handling, client is not interested in bounce packets
                for (unsigned int j = 0; j < root[i]["tags"].size(); j++) {
                    if (root[i]["tags"][j].asString() == "DeathLink") {
                        std::string source = root[i]["data"]["source"].asString();
                        
                        // Suspicions confirmed ;-; But maybe we died, not them?
                        if (source == ap_player_name) break; // We already paid our penance
                        deathlinkstat = true;
                        if (recvdeath) {
                            std::string cause = root[i]["data"]["cause"].isNull() ? "" : root[i]["data"]["cause"].asString();
                            recvdeath(source, cause);
                        }
                        break;
                    }
                }
            } else {
                AP_Bounce bounce;
                std::vector<std::string> games;
                std::vector<int64_t> slots;
                std::vector<std::string> tags;
                // Add targets to bounce package
                #define ADD_TARGETS( targets, accessor ) \
                        if (root[i].isMember(#targets)) { \
                            for (unsigned int j = 0; j < root[i][#targets].size(); j++) { \
                                targets.push_back(root[i][#targets][j].accessor()); \
                            } \
                            bounce.targets = &targets; \
                        }
                ADD_TARGETS(games, asString)
                ADD_TARGETS(slots, asInt64)
                ADD_TARGETS(tags, asString)
                #undef ADD_TARGETS

                bounce.data = writer.write(root[i]["data"]);
                bouncedfunc(bounce);
            }
            
        }
    }
    return false;
}

void APSend(std::string req) {
    if (webSocket.getReadyState() != ix::ReadyState::Open) {
        logfunc("AP: Not Connected. Send will fail.");
        return;
    }
    webSocket.send(req);
}

void WriteFileJSON(Json::Value val, std::string path) {
    std::ofstream out;
    out.open(path);
    out.seekp(0);
    out << writer.write(val).c_str();
    out.flush();
    out.close();
}

std::filesystem::path getDataPkgCachePath(const std::string& game, const std::string& hash)
{
    std::string alphanum_game, alphanum_hash;

    auto alphanum_func = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'); };
    std::copy_if(game.begin(), game.end(), std::back_inserter(alphanum_game), alphanum_func);
    std::copy_if(hash.begin(), hash.end(), std::back_inserter(alphanum_hash), alphanum_func);

    return datapkg_cache_dir / (alphanum_game + "-" + alphanum_hash + ".json");
}

void parseDataPkg(const std::string& game, Json::Value& package)
{
    for (std::string item_name : package["item_name_to_id"].getMemberNames()) {
        map_item_id_name[{game, package["item_name_to_id"][item_name].asInt64()}] = item_name;
    }
    for (std::string location : package["location_name_to_id"].getMemberNames()) {
        map_location_id_name[{game, package["location_name_to_id"][location].asInt64()}] = location;
    }
}

bool loadDataPkg(const std::string& game, const std::string& hash) {
    std::filesystem::path cache_path = getDataPkgCachePath(game, hash);
    std::ifstream cache_file(cache_path);
    if (!cache_file.is_open()) return false;
    try {
        Json::Value datapkg;
        reader.parse(cache_file, datapkg);
        parseDataPkg(game, datapkg);
    }
    catch (...) {
        return false; // Redownload corrupt/malformed Json files
    }
    return true;
}

void cacheDataPkgs(Json::Value& serverPkgs) {
    for (std::string& game : serverPkgs["games"].getMemberNames()) {
        std::string hash = serverPkgs["games"][game]["checksum"].asString();
        std::filesystem::path cache_path = getDataPkgCachePath(game, hash);

        parseDataPkg(game, serverPkgs["games"][game]);
        WriteFileJSON(serverPkgs["games"][game], cache_path.string());

        datapkg_outdated_games.erase(game);
        logfunc("AP: Game Cache updated for " + game);
    }
}

Json::Value getDataPkgRequest(void) {
    Json::Value server_req;

    if (datapkg_outdated_games.empty()) {
        server_req["cmd"] = "Sync";

        auth = true;
        ssl_success = auth && isSSL;
        refused = false;
    }
    else {
        // Fetch multiple games from the server at once to take advantage of compression.
        // Fetch up to 3 games at a time. Except if exactly 4 are left; then do 2 and 2 instead.
        int num_to_fetch = (datapkg_outdated_games.size() == 4 ? 2 : (std::min)(3, (int)datapkg_outdated_games.size()));

        server_req["cmd"] = "GetDataPackage";
        server_req["games"] = Json::arrayValue;
        for (const std::string& game : datapkg_outdated_games) {
            server_req["games"].append(game);
            if (!(--num_to_fetch > 0))
                break;
        }
    }
    return server_req;
}

std::string getItemName(std::string game, int64_t id) {
    std::pair<std::string,int64_t> item = {game,id};
    return map_item_id_name.count(item) ? map_item_id_name.at(item) : std::string("Unknown Item") + std::to_string(id) + " from " + game;
}

std::string getLocationName(std::string game, int64_t id) {
    std::pair<std::string,int64_t> location = {game,id};
    return map_location_id_name.count(location) ? map_location_id_name.at(location) : std::string("Unknown Location") + std::to_string(id) + " from " + game;
}

AP_NetworkPlayer getPlayer(int team, int slot) {
    return map_players[slot];
}

AP_NetworkPlayer getPlayer(int team, std::string name) {
    for (std::pair<int,AP_NetworkPlayer> player : map_players) {
        if (player.second.name == name && player.second.team == team) {
            return player.second;
        }
    }
    static AP_NetworkPlayer ERR_Player = {
        -1,
        -1,
        "ERR",
        "ERR",
        "ERR"
    };
    return ERR_Player;
}
