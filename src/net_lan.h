#ifndef NET_LAN_H
#define NET_LAN_H

#include "bflib_netsession.h"
#include <stdint.h>

#define LAN_SESSIONS_MAX 8

#ifdef __cplusplus
extern "C" {
#endif

extern struct TbNetworkSessionNameEntry lan_sessions[LAN_SESSIONS_MAX];
extern int lan_session_count;

void lan_host_start(const char *name, uint16_t port);
void lan_host_update(void);
void lan_refresh_sessions(void);
void lan_shutdown(void);
void lan_set_lobby_id(const char *id);

#ifdef __cplusplus
}
#endif

#endif
