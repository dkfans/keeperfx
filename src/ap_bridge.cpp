#include "pre_inc.h"
#include "ap_bridge.h"

#include "ap_data.h"
#include "Archipelago.h"
#include "config_terrain.h"
#include "config_campaigns.h"
#include "frontmenu_ingame_tabs.h"
#include "lua_triggers.h"
#include <cstdio>
#include <string>
#include <iostream>
#include <math.h>
#include "frontend.h"
#include "game_legacy.h"
#include "config_rules.h"
#include "gui_msgs.h"
#include "custom_sprites.h"
#include "keeperfx.hpp"
#include "post_inc.h"


void ap_location_info_callback(std::vector<AP_NetworkItem> locations);
void ap_receive(AP_NetworkItem,bool);
void ap_hint_message(AP_HintMessage msg);
void ap_server_chat_message(AP_ServerChatMessage msg);
void ap_chat_message(AP_ChatMessage msg);
void ap_send_message(std::string msg);
void ap_print(std::string text);
void ap_command_result(std::string text);
void set_quick_information_default(short icon_idx, const char* msg_text);

void RedirectStdoutToFile() {
    FILE* fp;
    // Redirects all future printf / stdout calls to ap_debug.log
    freopen_s(&fp, "ap_debug.log", "w", stdout);
    
    // Disable buffering so errors write to disk immediately upon crashing
    setvbuf(stdout, NULL, _IONBF, 0);
}

void ap_connect(char* ip, char* slot, char* password) {

RedirectStdoutToFile();
    if(AP_IsInit())
    {
        AP_Shutdown();
    }

    AP_Init(ip, "Dungeon Keeper", slot, password);
    AP_SetSocketConnectedCallback(ap_socketconnected);
    AP_SetSocketErrorCallback([](std::string err) {
        frontend_archipelago_error(err.c_str());
        JUSTLOG("AP error");
    });
    AP_SetItemClearCallback(ap_clear);
    AP_SetItemRecvCallback(ap_receive);
    AP_SetLocationCheckedCallback(ap_send);
    AP_SetLocationInfoCallback(ap_location_info_callback);    
    AP_SetRoomUpdateCallback(ap_room_update);
    AP_SetSlotConnectedCallback(ap_slot_connected);
    AP_SetChatMessageCallback(ap_chat_message);
    AP_SetServerChatCallback(ap_server_chat_message);
    AP_SetHintCallback(ap_hint_message);
    AP_SetPrintCallback(ap_print);
    AP_SetCmdResultCallback(ap_command_result);
//  Sslot info setup callbacks
    AP_RegisterSlotDataIntCallback("KeeperFXCreatures",        OnKeeperFXCreaturesReceived);
    AP_RegisterSlotDataIntCallback("KeeperFXDoors",            OnKeeperFXDoorsReceived);
    AP_RegisterSlotDataIntCallback("KeeperFXSpells",           OnKeeperFXSpellsReceived);
    AP_RegisterSlotDataIntCallback("KeeperFXTraps",            OnKeeperFXTrapsReceived);
    AP_RegisterSlotDataIntCallback("NegativeRecipes",          OnNegativeRecipesReceived);
    AP_RegisterSlotDataIntCallback("KeeperFXRecipes",          OnKeeperFXRecipesReceived);
    AP_RegisterSlotDataIntCallback("AddHeroes",                OnAddHeroesReceived);
    AP_RegisterSlotDataIntCallback("KeeperFXHeroes",           OnKeeperFXHeroesReceived);
    AP_RegisterSlotDataIntCallback("IncludeImpsInPool",        OnIncludeImpsInPoolReceived);
    AP_RegisterSlotDataIntCallback("IncludeTunnellersInPool",  OnIncludeTunnellersInPoolReceived);
    AP_RegisterSlotDataIntCallback("IncludeKnightsInPool",     OnIncludeKnightsInPoolReceived);
    AP_RegisterSlotDataIntCallback("IncludeAvatarsInPool",     OnIncludeAvatarsInPoolReceived);
    AP_RegisterSlotDataIntCallback("ShuffleTilesets",          OnShuffleTilesets);
    AP_RegisterSlotDataIntCallback("ChangePlayerColour",       OnChangePlayerColour);
    AP_RegisterSlotDataIntCallback("ChangeNeutrals",           OnChangeNeutrals);
    AP_RegisterSlotDataIntCallback("SwapWaterAndLava",         OnSwapWaterAndLava);
    AP_RegisterSlotDataIntCallback("RemoveNeutralRooms",       OnRemoveNeutralRooms);
    AP_RegisterSlotDataIntCallback("CrueltyMode",              OnCrueltyMode);
    AP_RegisterSlotDataIntCallback("SecretLevels",             OnSecretLevels);

    ap_location_info_init();    
    ap_state_init(&g_ap_state);
    AP_Start();
}

void ap_socketconnected(){

    JUSTLOG("AP CONNECTED");
}

void ap_slot_connected(){
    g_ap_state.connected = true;
    frontend_archipelago_connected();
    //callback once connected to AP server, send scounts for all locations not checked yet, so that ap_location_info_callback will be triggered. 
    ap_refresh_missing();
}

void ap_room_update()
{

}

void ap_refresh_missing()
{    
    AP_SendLocationScouts(AP_GetCheckedLocations(),0);
    for (int64_t loc : AP_GetCheckedLocations()) {
        ap_state_update_locations(&g_ap_state, (int)loc);
    }
    
    AP_SendLocationScouts(AP_GetMissingLocations(),0);
    ap_missing_init(&g_ap_state);
    for (int64_t loc : AP_GetMissingLocations()) {
        ap_state_update_missing_locations(&g_ap_state, (int)loc);
    }
}

void ap_receive(AP_NetworkItem item,bool notify)
{
    if(game.game_kind == GKind_LocalGame)
    {
        lua_on_item_received(item.item);
    }

    ap_state_update_items(&g_ap_state, item.item, item.location, item.player, item.flags, item.index);
}

void ap_send(int id)
{
    ap_state_update_locations(&g_ap_state, id);
}

void ap_clear()
{
    ap_location_info_clear();
}

bool ap_connection_status()
{
    return g_ap_state.connected;
}

void ap_location_info_callback(std::vector<AP_NetworkItem> locations)
{
    // store the details of locations remaining so that tooltips can get set etc.
    for (const AP_NetworkItem &info : locations)
    {
        ap_location_info_update(
            info.item,
            info.location,
            info.player,
            info.flags,
            info.itemName.c_str(),
            info.locationName.c_str(),
            info.playerName.c_str()
        );
    }
}

void ap_hint_message(AP_HintMessage msg)
{  
    std::string status = msg.checked ? "Checked" : "Unchecked";    
    std::string combined = msg.recvPlayer + "'s " +msg.item + " is in " + msg.sendPlayer + "'s "+ msg.location + ". it is " + status;
    set_quick_information_default(get_icon_id("ARCHIPELAGO_ICON"),combined.c_str());
}

void ap_command_result(std::string text)
{
    set_quick_information_default(get_icon_id("ARCHIPELAGO_ICON"),text.c_str());
}

void ap_print(std::string text)
{
    //message_add(MsgType_Custom, get_icon_id("ARCHIPELAGO_ICON"), text.c_str());
}

void ap_server_chat_message(AP_ServerChatMessage msg)
{       
    message_add(MsgType_Custom, get_icon_id("ARCHIPELAGO_ICON"), msg.message.c_str());
}

void ap_chat_message(AP_ChatMessage msg)
{
    message_add(MsgType_Custom, get_icon_id("ARCHIPELAGO_ICON"), msg.message.c_str());
}

void ap_send_message(std::string msg)
{
    AP_Say(msg);
}

void ap_bridge_scout_locations(const int *locations, int count)
{
    std::set<int64_t> location_set;

    for (int i = 0; i < count; i++)
    {
        location_set.insert((int64_t)locations[i]);
    }

    if (!location_set.empty())
    {
        AP_SendLocationScouts(location_set, 0);
    }
}


// probably dont need this anymore, was used to get the first digit from received item ids: 1 = room, 2 = spell
int ap_getitem_type(int id)
{
int digits = log10(id);
int itemType = (id / pow(10, digits));

return itemType;
}


void set_quick_information_default(short icon_idx, const char* msg_text)
{    
    int msg_id = rand() % QUICK_MESSAGES_COUNT;
    if (strlen(msg_text) >= MESSAGE_TEXT_LEN)
    {
        SCRPTWRNLOG("Information TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN - 1);
    }
    if ((game.quick_messages[msg_id][0] != '\0') && (strcmp(game.quick_messages[msg_id], msg_text) != 0))
    {
        SCRPTWRNLOG("Quick Message no %d overwritten by different text", msg_id);
    }
    snprintf(game.quick_messages[msg_id], MESSAGE_TEXT_LEN, "%s", msg_text);

    set_quick_information_with_icon(msg_id, 0, 0, 0, 0,icon_idx);
}

// Callback functions for slot data
void OnKeeperFXCreaturesReceived(int val)       { g_ap_state.slot_data.KeeperFXCreatures = val; }
void OnKeeperFXDoorsReceived(int val)           { g_ap_state.slot_data.KeeperFXDoors = val; }
void OnKeeperFXSpellsReceived(int val)          { g_ap_state.slot_data.KeeperFXSpells = val; }
void OnKeeperFXTrapsReceived(int val)           { g_ap_state.slot_data.KeeperFXTraps = val; }
void OnNegativeRecipesReceived(int val)         { g_ap_state.slot_data.NegativeRecipes = val; }
void OnKeeperFXRecipesReceived(int val)         { g_ap_state.slot_data.KeeperFXRecipes = val; }
void OnAddHeroesReceived(int val)               { g_ap_state.slot_data.AddHeroes = val; }
void OnKeeperFXHeroesReceived(int val)          { g_ap_state.slot_data.KeeperFXHeroes = val; }
void OnIncludeImpsInPoolReceived(int val)       { g_ap_state.slot_data.IncludeImpsInPool = val; }
void OnIncludeTunnellersInPoolReceived(int val) { g_ap_state.slot_data.IncludeTunnellersInPool = val; }
void OnIncludeKnightsInPoolReceived(int val)    { g_ap_state.slot_data.IncludeKnightsInPool = val; }
void OnIncludeAvatarsInPoolReceived(int val)    { g_ap_state.slot_data.IncludeAvatarsInPool = val; }
void OnShuffleTilesets(int val)    { g_ap_state.slot_data.ShuffleTilesets = val; }
void OnChangePlayerColour(int val)    { g_ap_state.slot_data.ChangePlayerColour = val; }
void OnChangeNeutrals(int val)    { g_ap_state.slot_data.ChangeNeutrals = val; }
void OnSwapWaterAndLava(int val)    { g_ap_state.slot_data.SwapWaterAndLava = val; }
void OnRemoveNeutralRooms(int val)    { g_ap_state.slot_data.RemoveNeutralRooms = val; }
void OnCrueltyMode(int val)    { g_ap_state.slot_data.CrueltyMode = val; }
void OnSecretLevels(int val)    { g_ap_state.slot_data.SecretLevels = val; }

// Functions below are run through the C compiler so that lua/console can call them

#ifdef __cplusplus
extern "C" {
#endif

void ap_bridge_connect(char* ip, char* slot, char* password)
{
    ap_connect(ip, slot, password);
}

void ap_bridge_location_check(int id)
{
    AP_SendItem(id);
    ap_state_update_locations(&g_ap_state, id);
}

bool ap_bridge_connection_status(void)
{
    return ap_connection_status();
}

void ap_bridge_refresh_missing(void)
{
    ap_refresh_missing();
}

void ap_bridge_send_message(const char* msg)
{
    ap_send_message(msg);
}

void ap_process_sacrifice_recipe(struct SacrificeRecipe *sac)
{
    char recipe_name[128] = {0};
    switch (sac->action)
    { 
        case SacA_MkGoodHero:
        case SacA_MkCreature:
        {
            const char* creature_name = creature_code_name(sac->param);
            snprintf(recipe_name, sizeof(recipe_name), "Recipe %s", creature_name);
            break;
        }
        case SacA_NegSpellAll:
        case SacA_PosSpellAll:{
            const char* spell_name = spell_code_name(sac->param);        
            snprintf(recipe_name, sizeof(recipe_name), "Recipe %s", spell_name);
            break;
        }
        case SacA_NegUniqFunc:
        case SacA_PosUniqFunc:
            switch (sac->param)
            {
                case UnqF_MkAllAngry:
                    strcpy(recipe_name, "Recipe Make Angry");
                    break;
                case UnqF_MkAllVerAngry:
                    strcpy(recipe_name, "Recipe Make Very Angry");
                    break;
                case UnqF_ComplResrch:
                    strcpy(recipe_name, "Recipe Complete Research");
                    break;
                case UnqF_ComplManufc:
                    strcpy(recipe_name, "Recipe Complete Manufacture");
                    break;
                case UnqF_KillChickns:
                    strcpy(recipe_name, "Recipe Kill Chickens");
                    break;
                case UnqF_CheaperImp:                    
                    strcpy(recipe_name, "Recipe Cheaper Imps");
                    break;
                case UnqF_CostlierImp:
                    strcpy(recipe_name, "Recipe Costlier Imps");
                    break;
                case UnqF_MkAllHappy:
                    strcpy(recipe_name, "Recipe Make Happy");
                    break;
                default:
                    break;
            }
        case SacA_CustomReward:
            break;
        case SacA_CustomPunish:
            break;
    }
    if (recipe_name[0] != '\0') {
        const AP_LocationInfo* info = ap_location_info_get_by_name(recipe_name);
        if(info != NULL && ap_location_is_missing(&g_ap_state, info->location)){
            AP_SendItem(info->location);
        }
    }
}

#ifdef __cplusplus
}
#endif