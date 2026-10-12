#include "pre_inc.h"
#include "ap_data.h"
#include <stdbool.h>
#include <stdio.h>
#include "post_inc.h"

struct APState g_ap_state;
static struct AP_LocationInfo ap_location_info[AP_LOCATION_NO];
static int ap_location_info_count = 0;
static int ap_current_lvl_remaining_box = 0;
void ap_state_init(struct APState* ap)
{
    if (!ap) return;

    ap->items_count = 0;
    ap->locations_count = 0;
    ap->missing_locations_count = 0;
    for (int i = 0; i < AP_LOCATION_NO; i++) {
        ap->checked_locations[i] = 0;
        ap->missing_locations[i] = 0; 
        memset(&ap->items_received[i], 0, sizeof(ap->items_received[i]));
    }
}

void ap_missing_init(struct APState* ap)
{
    if (!ap) return;
    ap->missing_locations_count = 0;
    for (int i = 0; i < AP_LOCATION_NO; i++) {
        ap->missing_locations[i] = 0; 
    }
}

void ap_state_update_items(struct APState* ap, long long item,
    long long location, int player, int flags, int index)
{
    if (!ap)
        return;

    if (ap->items_count >= AP_RECEIVED_ITEM_NO)
        return;

    struct AP_ReceivedItem *received =
        &ap->items_received[ap->items_count];

    received->item = item;
    received->location = location;
    received->player = player;
    received->flags = flags;
    received->index = index;

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

void ap_state_update_missing_locations(struct APState* ap, int locationid)
{
    int missing_locations_count = ap->missing_locations_count;

    for (int i = 0; i < missing_locations_count ; i++)
    {
        if (locationid == ap->missing_locations[i])
        {
            return;
        }        
    }

    ap->missing_locations[missing_locations_count] = locationid;
    ap->missing_locations_count++;
    
}

bool ap_location_is_missing(struct APState* ap, int locationid)
{
    for (int i = 0; i < ap->missing_locations_count ; i++)
    {
        if (locationid == ap->missing_locations[i])
        {
            return true;
        }        
    }
    return false;
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

const struct AP_LocationInfo *ap_location_info_get_by_name(const char* itm_name)
{
    for (int i = 0; i < ap_location_info_count; i++)
    {
        if (strcmp(ap_location_info[i].location_name,itm_name) == 0)
            return &ap_location_info[i];
    }

    return NULL;
}

void ap_location_info_clear(void)
{
    ap_location_info_init();
}

void ap_update_current_lvl_box_remaining(int cnt)
{
    ap_current_lvl_remaining_box = cnt;
}

void ap_decrease_current_lvl_box_remaining(void)
{
    ap_current_lvl_remaining_box = ap_current_lvl_remaining_box-1;
}
int ap_get_current_lvl_box_remaining(void)
{
    return ap_current_lvl_remaining_box;
}