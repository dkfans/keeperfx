#include "pre_inc.h"
#include "ap_bridge.h"
#include "Archipelago.h"
#include "config_terrain.h"
#include "frontmenu_ingame_tabs.h"
#include "lua_triggers.h"
#include <math.h>
#include "post_inc.h"

void ap_connect(char* ip, char* slot) {

    if(AP_IsInit())
    {
        AP_Shutdown();
    }

    AP_Init(ip, "Dungeon Keeper", slot, "");

    AP_SetItemClearCallback(ap_clear);
    AP_SetItemRecvCallback(ap_recieve);
    AP_SetLocationCheckedCallback(ap_send);

    AP_Start();
}

void ap_recieve(int id, bool notify)
{


    lua_on_item_received(id);

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


}

void ap_clear()
{



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
}

#ifdef __cplusplus
}
#endif