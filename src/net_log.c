#include "pre_inc.h"
#include "bflib_basics.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
#include <SDL3/SDL.h>
#include <inttypes.h>
#include <string.h>

#include "post_inc.h"

static const unsigned char ipv4_prefix[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 255};

static int32_t log_address_id(const unsigned char *address)
{
    static SDL_SpinLock lock;
    static SDL_PropertiesID addresses;
    static int32_t count;
    char key[INET6_ADDRSTRLEN];
    if (!inet_ntop(AF_INET6, address, key, sizeof(key))) {
        return 0;
    }
    SDL_LockSpinlock(&lock);
    if (!addresses) {
        addresses = SDL_CreateProperties();
    }
    int32_t id = SDL_GetNumberProperty(addresses, key, 0);
    if (!id && count < INT32_MAX && SDL_SetNumberProperty(addresses, key, count + 1)) {
        id = ++count;
    }
    SDL_UnlockSpinlock(&lock);
    return id;
}

char *LbLogSanitizeAddresses(const char *message)
{
    size_t length = strlen(message);
    if (length > (SIZE_MAX - 1) / 8) {
        return NULL;
    }
    char *output = SDL_malloc(length * 8 + 1);
    if (!output) {
        return NULL;
    }
    char *destination = output;
    while (*message) {
        size_t token_length = strspn(message, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.:_-");
        if (!token_length) {
            *destination++ = *message++;
            continue;
        }
        char token[64];
        size_t address_length = token_length;
        unsigned char address[16];
        int parsed = 0;
        if (token_length < sizeof(token)) {
            memcpy(token, message, token_length);
            token[token_length] = '\0';
            while (address_length > 0 && token[address_length - 1] == '.') {
                token[--address_length] = '\0';
            }
            char *colon = strchr(token, ':');
            if (colon && strchr(colon + 1, ':') == NULL) {
                address_length = colon - token;
                *colon = '\0';
            }
            if (inet_pton(AF_INET6, token, address) == 1) {
                parsed = 1;
            } else if (inet_pton(AF_INET, token, address + 12) == 1) {
                memcpy(address, ipv4_prefix, sizeof(ipv4_prefix));
                parsed = 1;
            }
        }
        if (parsed) {
            int32_t id = log_address_id(address);
            if (id) {
                const char *family = "IPv6";
                if (memcmp(address, ipv4_prefix, sizeof(ipv4_prefix)) == 0) {
                    family = "IPv4";
                }
                destination += snprintf(destination, 16, "%s#%" PRId32, family, id);
            } else {
                memcpy(destination, "IP#redacted", 11);
                destination += 11;
            }
        } else {
            address_length = token_length;
            memcpy(destination, message, address_length);
            destination += address_length;
        }
        message += address_length;
    }
    *destination = '\0';
    return output;
}
