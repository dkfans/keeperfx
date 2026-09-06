#include "pre_inc.h"
#include "ap_bridge.h"

#include "ap_data.h"
#include "Archipelago.h"
#include "config_terrain.h"
#include "frontmenu_ingame_tabs.h"
#include "lua_triggers.h"
#include <cstdio>
#include <string>
#include <iostream>
#include <math.h>
#include "post_inc.h"


void ap_location_info_callback(std::vector<AP_NetworkItem> locations);

void RedirectStdoutToFile() {
    FILE* fp;
    // Redirects all future printf / stdout calls to ap_debug.log
    freopen_s(&fp, "ap_debug.log", "w", stdout);
    
    // Disable buffering so errors write to disk immediately upon crashing
    setvbuf(stdout, NULL, _IONBF, 0);
}

void ap_connect(char* ip, char* slot) {

RedirectStdoutToFile();
    if(AP_IsInit())
    {
        AP_Shutdown();
    }

    AP_Init(ip, "Dungeon Keeper", slot, "");
    AP_SetSocketConnectedCallback(ap_socketconnected);
    AP_SetSocketErrorCallback([](std::string err) {
    // need to do something with errors, placeholder for now
        JUSTLOG("AP error");
    });
    AP_SetItemClearCallback(ap_clear);
    AP_SetItemRecvCallback(ap_recieve);
    AP_SetLocationCheckedCallback(ap_send);
    AP_SetLocationInfoCallback(ap_location_info_callback);    
    AP_SetRoomUpdateCallback(ap_room_update);
    AP_SetSlotConnectedCallback(ap_slot_connected);
    ap_location_info_init();    
    ap_state_init(&g_ap_state);
    AP_Start();
}

void ap_socketconnected(){

    JUSTLOG("AP CONNECTED");
}

void ap_slot_connected(){
    //callback once connected to AP server, send scounts for all locations not checked yet, so that ap_location_info_callback will be triggered.
    AP_SendLocationScouts(AP_GetMissingLocations(),0);
}

void ap_room_update(){

}

void ap_recieve(int id, bool notify)
{


    lua_on_item_received(id);
    ap_state_update_items(&g_ap_state, id);

 //   pre lua version testing code   
 //   TbBool available = 1;
 //   long roomid = id % 100;    


 //   switch (ap_getitem_type(id))
 //   {
 //   case 1: // Rooms
 //       set_room_available(0, roomid, available, available);
 //       update_room_tab_to_config();
 //       break;  

 //   case 2: // Spells
       // set_power_available(1, spellid, 1, 1);
 //       break;  

 //  default:
 //      break;
 //  }

}

void ap_send(int id)
{
    ap_state_update_locations(&g_ap_state, id);

}

void ap_clear()
{

    ap_location_info_clear();

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

// Functions below are run through the C compiler so that lua/console can call them

#ifdef __cplusplus
extern "C" {
#endif

void ap_bridge_connect(char* ip, char* slot)
{
ap_connect(ip, slot);
}

void ap_bridge_location_check(int id)
{
    AP_SendItem(id);
    ap_state_update_locations(&g_ap_state, id);
}

#ifdef __cplusplus
}
#endif