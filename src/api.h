#include <windows.h>
#include <winbase.h>

void init_api_server();
void close_api_server();
DWORD WINAPI api_server_thread(LPVOID lpParam);