#ifndef AP_DATA_H
#define AP_DATA_H


#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AP_LOCATION_NO 166
#define AP_RECEIVED_ITEM_NO 512
extern struct APState g_ap_state;

struct AP_ReceivedItem
{
    long long item;
    long long location;
    int player;
    int flags;
    int index;
};

struct AP_SlotData
{
int KeeperFXCreatures;
int KeeperFXDoors;
int KeeperFXSpells;
int KeeperFXTraps;
int NegativeRecipes;
int KeeperFXRecipes;
int AddHeroes;
int KeeperFXHeroes;
int IncludeImpsInPool;
int IncludeTunnellersInPool;
int IncludeKnightsInPool;
int IncludeAvatarsInPool;
int ShuffleTilesets;
int ChangePlayerColour;
int ChangeNeutrals;
int SwapWaterAndLava;
int RemoveNeutralRooms;
int CrueltyMode;
int SecretLevels;
};
struct APState
{
    bool connected;
    int checked_locations[AP_LOCATION_NO];
    int missing_locations[AP_LOCATION_NO];
    struct AP_ReceivedItem items_received[AP_RECEIVED_ITEM_NO];   
    int items_count;
    int locations_count;
    int missing_locations_count;
    struct AP_SlotData slot_data;
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
void ap_state_update_items(struct APState* ap, long long item, long long location, int player, int flags, int index);
void ap_state_update_locations(struct APState* ap, int locationid);
void ap_state_update_missing_locations(struct APState* ap, int locationid);
bool ap_location_is_missing(struct APState* ap, int locationid);
void ap_location_info_init(void);
void ap_missing_init(struct APState* ap);

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
const struct AP_LocationInfo *ap_location_info_get_by_name(const char* itm_name);
void ap_location_info_clear(void);
void ap_update_current_lvl_box_remaining(int cnt);
void ap_decrease_current_lvl_box_remaining(void);
int ap_get_current_lvl_box_remaining(void);

#ifdef __cplusplus
}
#endif
#endif
