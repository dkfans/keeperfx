#ifndef API_H
#define API_H
#include "pre_inc.h"
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include "post_inc.h"

void init_api_server();
void close_api_server();
void log_last_api_error();
DWORD WINAPI api_server_thread(LPVOID lpParam);
#endif