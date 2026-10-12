#ifndef AP_BRIDGE_H
#define AP_BRIDGE_H

void ap_connect();
void ap_socketconnected();
void ap_slot_connected();
void ap_room_update();
void ap_refresh_missing();
void ap_send(int id);
void ap_clear();
bool ap_connection_status();
int ap_getitem_type(int id);

// slot data handling
void OnKeeperFXCreaturesReceived(int val);
void OnKeeperFXDoorsReceived(int val);
void OnKeeperFXSpellsReceived(int val);
void OnKeeperFXTrapsReceived(int val);
void OnNegativeRecipesReceived(int val);
void OnKeeperFXRecipesReceived(int val);
void OnAddHeroesReceived(int val);
void OnKeeperFXHeroesReceived(int val);
void OnIncludeImpsInPoolReceived(int val);
void OnIncludeTunnellersInPoolReceived(int val);
void OnIncludeKnightsInPoolReceived(int val);
void OnIncludeAvatarsInPoolReceived(int val);
void OnShuffleTilesets(int val);
void OnChangePlayerColour(int val);
void OnChangeNeutrals(int val);
void OnSwapWaterAndLava(int val);
void OnRemoveNeutralRooms(int val);
void OnCrueltyMode(int val);
void OnSecretLevels(int val);

#ifdef __cplusplus
extern "C" {
#endif

void ap_bridge_connect(char* ip, char* slot, char* password);
void ap_bridge_location_check(int id);
void ap_bridge_scout_locations(const int *locations, int count);
bool ap_bridge_connection_status(void);
void ap_bridge_refresh_missing(void);
void ap_bridge_send_message(const char* msg);
void ap_process_sacrifice_recipe(struct SacrificeRecipe *sac);

#ifdef __cplusplus
}
#endif
#endif