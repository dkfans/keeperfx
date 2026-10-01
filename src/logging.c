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
#include <stdarg.h>
#include <string.h>

#include "globals.h"
#include "bflib_datetm.h"
#include "bflib_fileio.h"
#include "post_inc.h"

char consoleLogArray[MAX_CONSOLE_LOG_COUNT][MAX_TEXT_LENGTH];
size_t consoleLogArraySize = 0;
int debug_display_consolelog = 0;
const char *log_file_name = DEFAULT_LOG_FILENAME;

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

static char *LbLogSanitizeAddresses(const char *message)
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

static short error_log_initialised = 0;
static struct TbLog error_log;
static int LbLog(struct TbLog *log, const char *fmt_str, va_list arg);

int LbErrorLog(const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefix(&error_log, "Error: ");
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbWarnLog(const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefix(&error_log, "Warning: ");
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbNetLog(const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefix(&error_log, "Net: ");
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbSyncLog(const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefix(&error_log, "Sync: ");
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbNaviLog(const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefix(&error_log, "Navi: ");
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

#ifdef FUNCTESTING
int LbFTestLog(const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefix(&error_log, "FTest: ");
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}
#endif

/*
 * Logs script-related message.
 */
int LbScriptLog(int32_t line, const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefixFmt(&error_log, "Script(line %" PRId32 "): ", line);
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

/*
 * Logs config file related message.
 */
int LbConfigLog(int32_t line, const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefixFmt(&error_log, "Config(line %" PRId32 "): ", line);
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbJustLog(const char *format, ...)
{
    if (!error_log_initialised) {
        return -1;
    }
    LbLogSetPrefix(&error_log, "");
    va_list val;
    va_start(val, format);
    int result = LbLog(&error_log, format, val);
    va_end(val);
    return result;
}

int LbErrorLogSetup(const char *directory, const char *filename, TbBool flag)
{
    if (error_log_initialised) {
        return -1;
    }
    if (filename == NULL || strlen(filename) == 0) {
        filename = "error.log";
    }
    char log_filename[DISKPATH_SIZE];
    if (LbFileMakeFullPath(1, directory, filename, log_filename, DISKPATH_SIZE) != 1) {
        return -1;
    }
    ulong flags = (flag == 0) + 1;
    flags |= LbLog_TimeInHeader | LbLog_DateInHeader | 0x04;
    if (LbLogSetup(&error_log, log_filename, flags) != 1) {
        return -1;
    }
    error_log_initialised = 1;
    return 1;
}

int LbErrorLogClose(void)
{
    if (!error_log_initialised) {
        return -1;
    }
    return LbLogClose(&error_log);
}

static FILE *log_file = NULL;

static void write_log_to_array_for_live_viewing(const char *message, const char *add_log_prefix)
{
    if (consoleLogArraySize >= MAX_CONSOLE_LOG_COUNT) {
        // Array is full - so clear it. This is a bit of a stopgap solution, it will lose us the older entries.
        memset(consoleLogArray, 0, sizeof(consoleLogArray));
        consoleLogArraySize = 0;
    }

    char buffer[MAX_TEXT_LENGTH];
    snprintf(buffer, sizeof(buffer), "%s%s", add_log_prefix, message);

    // Add the combined message to the array
    strncpy(consoleLogArray[consoleLogArraySize], buffer, MAX_TEXT_LENGTH);
    consoleLogArray[consoleLogArraySize][MAX_TEXT_LENGTH - 1] = '\0';
    consoleLogArraySize++;
}

static int LbLog(struct TbLog *log, const char *fmt_str, va_list arg)
{
    enum Header {
        NONE = 0,
        CREATE = 1,
        APPEND = 2,
    };
    if (!log->Initialised) {
        return -1;
    }
    if (log->Suspended) {
        return 1;
    }
    char header = NONE;
    short need_initial_newline = false;
    if (!log->Created) {
        if (((log->flags & 0x04) == 0) || LbFileExists(log->filename)) {
            if (((log->flags & 0x01) != 0) && ((log->flags & 0x04) != 0)) {
                header = CREATE;
            } else if (((log->flags & 0x02) != 0) && ((log->flags & 0x08) != 0)) {
                need_initial_newline = true;
                header = APPEND;
            }
        } else {
            header = CREATE;
        }
    }
    const char *accmode;
    if (log->Created || ((log->flags & 0x01) == 0)) {
        accmode = "a";
    } else {
        accmode = "w";
    }
    if (log_file == NULL) {
        log_file = fopen(log->filename, accmode);
        if (log_file == NULL) {
            return -1;
        }
    }
    log->Created = true;
    if (header != NONE) {
        if (need_initial_newline) {
            fprintf(log_file, "\n");
        }
        const char *actn;
        if (header == CREATE) {
            const char *release = "standard";
            if (BFDEBUG_LEVEL > 7) {
                release = "heavylog";
            }
            fprintf(log_file, PROGRAM_NAME " ver " VER_STRING " (%s release) git:%s\n", release, GIT_REVISION);
            actn = "CREATED";
        } else {
            actn = "APPENDED";
        }
        fprintf(log_file, "LOG %s", actn);
        short at_used = 0;
        if ((log->flags & LbLog_TimeInHeader) != 0) {
            struct TbTime curr_time;
            if (LbTime(&curr_time) == Lb_SUCCESS) {
                fprintf(log_file, "  @ %02u:%02u:%02u", curr_time.Hour, curr_time.Minute, curr_time.Second);
                at_used = 1;
            }
        }
        if ((log->flags & LbLog_DateInHeader) != 0) {
            struct TbDate curr_date;
            if (LbDate(&curr_date) == Lb_SUCCESS) {
                const char *sep;
                if (at_used) {
                    sep = " ";
                } else {
                    sep = "  @ ";
                }
                fprintf(log_file, " %s%02u-%02u-%u", sep, curr_date.Day, curr_date.Month, curr_date.Year);
            }
        }
        fprintf(log_file, "\n\n");
    }
    if ((log->flags & LbLog_DateInLines) != 0) {
        struct TbDate curr_date;
        if (LbDate(&curr_date) == Lb_SUCCESS) {
            fprintf(log_file, "%02u-%02u-%u ", curr_date.Day, curr_date.Month, curr_date.Year);
        }
    }
    if ((log->flags & LbLog_TimeInLines) != 0) {
        struct TbTime curr_time;
        if (LbTime(&curr_time) == Lb_SUCCESS) {
            fprintf(log_file, "%02u:%02u:%02u ", curr_time.Hour, curr_time.Minute, curr_time.Second);
        }
    }
    if (log->prefix[0] != '\0') {
        fputs(log->prefix, log_file);
    }

    char *message = NULL;
    if (SDL_vasprintf(&message, fmt_str, arg) < 0) {
        return -1;
    }
    char *sanitized = LbLogSanitizeAddresses(message);
    SDL_free(message);
    if (!sanitized) {
        return -1;
    }
    write_log_to_array_for_live_viewing(sanitized, log->prefix);
    fputs(sanitized, log_file);
    SDL_free(sanitized);
    log->position = ftell(log_file);
    fflush(log_file);
    return 1;
}

int LbLogSetPrefix(struct TbLog *log, const char *prefix)
{
    if (!log->Initialised) {
        return -1;
    }
    snprintf(log->prefix, LOG_PREFIX_LEN, "%s", prefix);
    return 1;
}

int LbLogSetPrefixFmt(struct TbLog *log, const char *format, ...)
{
    if (!log->Initialised) {
        return -1;
    }
    va_list val;
    va_start(val, format);
    vsnprintf(log->prefix, sizeof(log->prefix), format, val);
    va_end(val);
    return 1;
}

int LbLogSetup(struct TbLog *log, const char *filename, ulong flags)
{
    log->Initialised = false;
    memset(log->filename, 0, DISKPATH_SIZE);
    memset(log->prefix, 0, LOG_PREFIX_LEN);
    log->Created = false;
    log->Suspended = false;
    if (strlen(filename) >= DISKPATH_SIZE || strlen(filename) == 0) {
        return -1;
    }
    snprintf(log->filename, DISKPATH_SIZE, "%s", filename);
    log->flags = flags;
    log->Initialised = true;
    log->position = 0;
    return 1;
}

int LbLogClose(struct TbLog *log)
{
    if (!log->Initialised) {
        return -1;
    }
    memset(log->filename, 0, DISKPATH_SIZE);
    memset(log->prefix, 0, LOG_PREFIX_LEN);
    log->flags = 0;
    log->Initialised = false;
    log->Created = false;
    log->Suspended = false;
    log->position = 0;
    return 1;
}
