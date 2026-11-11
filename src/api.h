#ifndef API_H
#define API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int api_init_server();
    void api_update_server();
    void api_close_server();

    enum ApiEventDataType
    {
        API_EVENT_DATA_INT32,
        API_EVENT_DATA_UINT32,
        API_EVENT_DATA_INT64,
        API_EVENT_DATA_UINT64,
        API_EVENT_DATA_FLOAT,
        API_EVENT_DATA_DOUBLE,
        API_EVENT_DATA_BOOL,
        API_EVENT_DATA_STRING,
    };

    struct ApiEventData
    {
        const char *name;
        enum ApiEventDataType type;
        union
        {
            int32_t int32_value;
            uint32_t uint32_value;
            int64_t int64_value;
            uint64_t uint64_value;
            float float_value;
            double double_value;
            bool bool_value;
            const char *string_value;
        } value;
    };

    void api_event(const char *event_name);
    void api_event_with_data(const char *event_name, const struct ApiEventData *data, size_t data_count);

#ifdef __cplusplus
}
#endif

#endif