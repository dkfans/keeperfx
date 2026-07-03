#ifndef API_H
#define API_H

#ifdef __cplusplus
extern "C"
{
#endif

    int api_init_server();
    void api_update_server();
    void api_close_server();

    void api_event(const char *event_name);

#ifdef __cplusplus
}
#endif

#endif