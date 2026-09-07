#ifndef AP_DATA_H
#define AP_DATA_H


#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AP_LOCATION_NO 166
extern struct APState g_ap_state;

struct APState
{
int checked_locations[AP_LOCATION_NO];
int missing_locations[AP_LOCATION_NO];
int items_recieved[AP_LOCATION_NO];   
int items_count; 
int locations_count;
};

struct AP_LocationInfo
{
    long long item;
    long long location;
    int player;
    int flags;

    char item_name[256];
    char location_name[256];
    char player_name[256];
};

void ap_state_init(struct APState* ap);
void ap_state_update_items(struct APState* ap, int itemid);
void ap_state_update_locations(struct APState* ap, int locationid);
void ap_location_info_init(void);

void ap_location_info_update(
    long long item,
    long long location,
    int player,
    int flags,
    const char *item_name,
    const char *location_name,
    const char *player_name
);

const struct AP_LocationInfo *ap_location_info_get(long long location);

void ap_location_info_clear(void);

#ifdef __cplusplus
}
#endif
#endif
