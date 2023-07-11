/* This file (toml_conv.c) is included into toml_api.c */

#include <errno.h>
#include "json.h"

typedef const char *toml_raw_t;

/* Scan p for n digits compositing entirely of [0-9] */
static int scan_digits(const char *p, int n)
{
    int ret = 0;
    for (; n > 0 && isdigit(*p); n--, p++)
    {
        ret = 10 * ret + (*p - '0');
    }
    return n ? -1 : ret;
}

static int scan_date(const char *p, uint16_t *YY, uint8_t *MM, uint8_t *DD)
{
    int year, month, day;
    year = scan_digits(p, 4);
    month = (year >= 0 && p[4] == '-') ? scan_digits(p + 5, 2) : -1;
    day = (month >= 0 && p[7] == '-') ? scan_digits(p + 8, 2) : -1;
    if (YY)
        *YY = year;
    if (MM)
        *MM = month;
    if (DD)
        *DD = day;
    return (year >= 0 && month >= 0 && day >= 0) ? 0 : -1;
}

static int scan_time(const char *p, uint8_t *hh, uint8_t *mm, uint8_t *ss)
{
    int hour, minute, second;
    hour = scan_digits(p, 2);
    minute = (hour >= 0 && p[2] == ':') ? scan_digits(p + 3, 2) : -1;
    second = (minute >= 0 && p[5] == ':') ? scan_digits(p + 6, 2) : -1;
    if (hh)
        *hh = hour;
    if (mm)
        *mm = minute;
    if (ss)
        *ss = second;
    return (hour >= 0 && minute >= 0 && second >= 0) ? 0 : -1;
}

/* Raw to boolean */
static int toml_rtob(token_t src, VALUE *value)
{
    if (src.tok != STRING)
    {
        return -1;
    }

    if (0 == strncmp(src.ptr, "true", 4))
    {
        value_init_bool(value, 1);
        return 0;
    }
    if (0 == strncmp(src.ptr, "false", 5))
    {
        value_init_bool(value, 0);
        return 0;
    }
    return -1;
}

/* Raw to integer */
static int toml_is_i(token_t src, char *buf, size_t buflen)
{
    if (src.tok != STRING)
        return -1;

    char *p = buf;
    char *q = p + buflen;
    const char *s = src.ptr;
    const char *e = s + src.len;
    int base = 0;

    /* allow +/- */
    if (s[0] == '+' || s[0] == '-')
        *p++ = *s++;

    /* disallow +_100 */
    if (s[0] == '_')
        return -1;

    /* if 0* ... */
    if ('0' == s[0])
    {
        if (src.len == 1)
            return 0;
        switch (s[1])
        {
            case 'x':
                base = 16;
                s += 2;
                break;
            case 'o':
                base = 8;
                s += 2;
                break;
            case 'b':
                base = 2;
                s += 2;
                break;
            case '\0':
                return 0;
            default:
                /* ensure no other digits after it */
                if (s[1])
                    return -1;
        }
    }

    /* just strip underscores and pass to strtoll */
    while (s < e && p < q)
    {
        int ch = *s++;
        if (ch == '_')
        {
            // disallow '__'
            if (s[0] == '_')
                return -1;
            // numbers cannot end with '_'
            if (s[0] == '\0')
                return -1;
            continue; /* skip _ */
        }
        *p++ = ch;
    }

    // if we ran out of buffer ...
    if (p == q)
        return -1;

    /* cap with NUL */
    *p = 0;

    /* Run strtoll on buf to get the integer */
    char *endp;
    errno = 0;
    strtoll(buf, &endp, base);
    return (errno || *endp) ? -1 : 0;
}

static int toml_is_d(token_t src, char *buf, int buflen)
{
    if (src.tok != STRING)
        return -1;

    char *p = buf;
    char *q = p + buflen;
    const char *s = src.ptr;
    const char *e = s + src.len;

    /* allow +/- */
    if (s[0] == '+' || s[0] == '-')
        *p++ = *s++;

    /* disallow +_1.00 */
    if (s[0] == '_')
        return -1;

    /* decimal point, if used, must be surrounded by at least one digit on each
     * side */
    {
        char *dot = strchr(s, '.');
        if (dot && (dot < e))
        {
            if (dot == s || !isdigit(dot[-1]) || !isdigit(dot[1]))
                return -1;
        }
    }

    /* zero must be followed by . or 'e', or NUL */
    if (s[0] == '0' && s[1] && !strchr("eE.", s[1]))
        return -1;

    /* just strip underscores and pass to strtod */
    while (s < e && p < q)
    {
        int ch = *s++;
        if (ch == '_')
        {
            // disallow '__'
            if (s[0] == '_')
                return -1;
            // disallow last char '_'
            if (s[0] == 0)
                return -1;
            continue; /* skip _ */
        }
        *p++ = ch;
    }
    if (s < e || p == q)
        return -1; /* reached end of string or buffer is full? */

    /* cap with NUL */
    *p = 0;

    /* Run strtod on buf to get the value */
    char *endp;
    errno = 0;
    strtod(buf, &endp);
    return (errno || *endp) ? -1 : 0;
}

static char ptoml_valtype(token_t token, VALUE *value)
{
    char buf[100];
    if (*token.ptr == '\'' || *token.ptr == '"')
        return 's';
    if (0 == toml_rtob(token, value))
        return 'b';
    // TODO: use value
    if (0 == toml_is_i(token, buf, sizeof(buf)))
        return 'i';
    if (0 == toml_is_d(token, buf, sizeof(buf)))
        return 'd';
    return 'u'; /* unknown */
}

static int
init_number(VALUE *v, const char *data, size_t data_size)
{
    int is_int32_compatible;
    int is_uint32_compatible;
    int is_int64_compatible;
    int is_uint64_compatible;

    if ((data_size > 1) && (data[0] == '0') && (data[1] == 'x'))
    {
        char *end;
        if (data_size < 3)
        {
            int32_t val;
            val = strtol(data + 2, &end, 16);
            if (end != data + data_size)
                return -1;
            return value_init_int32(v, val);
        }
        else
        {
            if ((data_size < 10) ||
                ((data_size == 10) && (data[2] > '0' && data[2] < '8')))
            {
                int32_t val;
                val = strtol(data + 2, &end, 16);
                if (end != data + data_size)
                    return -1;
                return value_init_int32(v, val);
            }
            else if (data_size <= 10)
            {
                uint32_t val;
                val = strtoul(data + 2, &end, 16);
                if (end != data + data_size)
                    return -1;
                return value_init_uint32(v, val);
            }
            else if ((data_size < 18) ||
                     ((data_size == 10) && (data[2] > '0' && data[2] < '8')))
            {
                int64_t val;
                val = strtoll(data + 2, &end, 16);
                if (end != data + data_size)
                    return -1;
                return value_init_int64(v, val);
            }
            else if (data_size <= 18)
            {
                uint64_t val;
                val = strtoull(data + 2, &end, 16);
                if (end != data + data_size)
                    return -1;
                return value_init_uint64(v, val);
            }
            else
            {
                return -1;
            }
        }
    }
    else
    {
        json_analyze_number(data, data_size,
                            &is_int32_compatible, &is_uint32_compatible,
                            &is_int64_compatible, &is_uint64_compatible);
    }

    if (is_int32_compatible)
    {
        return value_init_int32(v, json_number_to_int32(data, data_size));
    }
    else if (is_uint32_compatible)
    {
        return value_init_uint32(v, json_number_to_uint32(data, data_size));
    }
    else if (is_int64_compatible)
    {
        return value_init_int64(v, json_number_to_int64(data, data_size));
    }
    else if (is_uint64_compatible)
    {
        return value_init_uint64(v, json_number_to_uint64(data, data_size));
    }
    else
    {
        double d;
        int err;
        err = json_number_to_double(data, data_size, &d);
        if (err != 0)
            return err;
        return value_init_double(v, d);
    }
}