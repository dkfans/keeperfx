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
    void api_var_update(signed char plyr_idx, unsigned char valtype, unsigned char validx, long new_val);

#ifdef __cplusplus
}
#endif

#endif