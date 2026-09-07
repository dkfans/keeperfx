
#ifndef AP_BRIDGE_H
#define AP_BRIDGE_H

void ap_connect();
void ap_socketconnected();
void ap_slot_connected();
void ap_room_update();
void ap_recieve(int id, bool notify);
void ap_send(int id);
void ap_clear();
bool ap_connection_status();
int ap_getitem_type(int id);
#ifdef __cplusplus
extern "C" {
#endif

void ap_bridge_connect(char* ip, char* slot, char* password);
void ap_bridge_location_check(int id);
void ap_bridge_scout_locations(const int *locations, int count);
bool ap_bridge_connection_status(void);

#ifdef __cplusplus
}
#endif
#endif