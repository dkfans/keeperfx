#include "pre_inc.h"
#include "ap_data.h"
#include <stdbool.h>
#include <stdio.h>
#include "post_inc.h"

struct APState g_ap_state;
static struct AP_LocationInfo ap_location_info[AP_LOCATION_NO];
static int ap_location_info_count = 0;

void ap_state_init(struct APState* ap)
{
    if (!ap) return;

    ap->items_count = 0;
    ap->locations_count = 0;
    for (int i = 0; i < AP_LOCATION_NO; i++) {
        ap->checked_locations[i] = 0;
        ap->missing_locations[i] = 0; 
        ap->items_received[i] = 0;
    }
}

void ap_state_update_items(struct APState* ap, int itemid)
{
    int item_count = ap->items_count;

    for (int i = 0; i < item_count ; i++)
    {
        if (itemid == ap->items_received[i])
        {
            return;
        }
        
    }

    ap->items_received[item_count] = itemid;
    ap->items_count++;
    
}

void ap_state_update_locations(struct APState* ap, int locationid)
{
    int location_count = ap->locations_count;

    for (int i = 0; i < location_count ; i++)
    {
        if (locationid == ap->checked_locations[i])
        {
            return;
        }
        
    }

    ap->checked_locations[location_count] = locationid;
    ap->locations_count++;
    
}

void ap_location_info_init(void)
{
    ap_location_info_count = 0;

    for (int i = 0; i < AP_LOCATION_NO; i++)
    {
        ap_location_info[i].item = 0;
        ap_location_info[i].location = 0;
        ap_location_info[i].player = 0;
        ap_location_info[i].flags = 0;

        ap_location_info[i].item_name[0] = '\0';
        ap_location_info[i].location_name[0] = '\0';
        ap_location_info[i].player_name[0] = '\0';
    }
}

void ap_location_info_update(long long item, long long location, int player, int flags,
    const char *item_name, const char *location_name, const char *player_name)
{
    for (int i = 0; i < ap_location_info_count; i++)
    {
        if (ap_location_info[i].location == location)
        {
            ap_location_info[i].item = item;
            ap_location_info[i].player = player;
            ap_location_info[i].flags = flags;

            snprintf(ap_location_info[i].item_name, sizeof(ap_location_info[i].item_name),
                "%s", item_name ? item_name : "");

            snprintf(ap_location_info[i].location_name, sizeof(ap_location_info[i].location_name),
                "%s", location_name ? location_name : "");

            snprintf(ap_location_info[i].player_name, sizeof(ap_location_info[i].player_name),
                "%s", player_name ? player_name : "");

            return;
        }
    }

    if (ap_location_info_count >= AP_LOCATION_NO)
        return;

    struct AP_LocationInfo *info = &ap_location_info[ap_location_info_count];

    info->item = item;
    info->location = location;
    info->player = player;
    info->flags = flags;

    snprintf(info->item_name, sizeof(info->item_name), "%s", item_name ? item_name : "");
    snprintf(info->location_name, sizeof(info->location_name), "%s", location_name ? location_name : "");
    snprintf(info->player_name, sizeof(info->player_name), "%s", player_name ? player_name : "");

    ap_location_info_count++;
}

const struct AP_LocationInfo *ap_location_info_get(long long location)
{
    for (int i = 0; i < ap_location_info_count; i++)
    {
        if (ap_location_info[i].location == location)
            return &ap_location_info[i];
    }

    return NULL;
}

void ap_location_info_clear(void)
{
    ap_location_info_init();
}