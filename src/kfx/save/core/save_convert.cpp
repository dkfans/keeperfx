/******************************************************************************/
/** @file save_convert.cpp
 *     Scalar width, sign and range conversion between memory and stored bytes.
 */
/******************************************************************************/
#include "save_schema.h"

#include <cstring>
/******************************************************************************/
struct IntKind {
    uint32_t bits;
    int is_signed;
    int is_bool;
};

static int int_kind(uint8_t type, struct IntKind *kind)
{
    kind->is_bool = 0;
    switch (type)
    {
    case SV_U8:  case SV_STR: kind->bits = 8;  kind->is_signed = 0; return 0;
    case SV_I8:  kind->bits = 8;  kind->is_signed = 1; return 0;
    case SV_U16: kind->bits = 16; kind->is_signed = 0; return 0;
    case SV_I16: kind->bits = 16; kind->is_signed = 1; return 0;
    case SV_U32: kind->bits = 32; kind->is_signed = 0; return 0;
    case SV_I32: kind->bits = 32; kind->is_signed = 1; return 0;
    case SV_U64: kind->bits = 64; kind->is_signed = 0; return 0;
    case SV_I64: kind->bits = 64; kind->is_signed = 1; return 0;
    case SV_BOOL8: kind->bits = 8; kind->is_signed = 0; kind->is_bool = 1; return 0;
    default: return -1;
    }
}

static uint64_t raw_load(const uint8_t *bytes, uint32_t size, int little)
{
    uint64_t value = 0;
    for (uint32_t byte_index = 0; byte_index < size; byte_index++)
        value |= static_cast<uint64_t>(bytes[little ? byte_index : size - 1 - byte_index]) << (8 * byte_index);
    return value;
}

static void raw_store(uint8_t *bytes, uint32_t size, uint64_t value, int little)
{
    for (uint32_t byte_index = 0; byte_index < size; byte_index++)
        bytes[little ? byte_index : size - 1 - byte_index] = static_cast<uint8_t>(value >> (8 * byte_index));
}

static void value_from_raw(struct SaveValue *value, uint64_t raw, const struct IntKind *kind)
{
    value->is_float = 0;
    value->real = 0;
    if (kind->is_bool)
    {
        value->neg = 0;
        value->mag = (raw != 0);
        return;
    }
    if (kind->is_signed && kind->bits < 64)
    {
        const uint64_t sign = static_cast<uint64_t>(1) << (kind->bits - 1);
        if (raw & sign)
            raw |= ~((static_cast<uint64_t>(1) << kind->bits) - 1);
    }
    if (kind->is_signed && (raw >> 63))
    {
        value->neg = 1;
        value->mag = (~raw) + 1;
    } else
    {
        value->neg = 0;
        value->mag = raw;
    }
}

static int value_to_raw(const struct SaveValue *value, const struct IntKind *kind, int clamp, uint64_t *raw)
{
    int clamped = 0;
    if (value->is_float)
        return -1;
    if (kind->is_bool)
    {
        *raw = (value->mag != 0);
        return 0;
    }
    uint64_t max_pos;
    uint64_t max_neg;
    if (kind->is_signed)
    {
        max_pos = (static_cast<uint64_t>(1) << (kind->bits - 1)) - 1;
        max_neg = static_cast<uint64_t>(1) << (kind->bits - 1);
    } else
    {
        max_pos = (kind->bits == 64) ? UINT64_MAX : ((static_cast<uint64_t>(1) << kind->bits) - 1);
        max_neg = 0;
    }
    uint64_t mag = value->mag;
    const int neg = value->neg && (mag != 0);
    if (!neg && (mag > max_pos))
    {
        if (!clamp)
            return -1;
        mag = max_pos;
        clamped = 1;
    } else if (neg && (mag > max_neg))
    {
        if (!clamp)
            return -1;
        mag = max_neg;
        clamped = 1;
    }
    uint64_t out = neg ? ((~mag) + 1) : mag;
    if (kind->bits < 64)
        out &= (static_cast<uint64_t>(1) << kind->bits) - 1;
    *raw = out;
    return clamped;
}

int save_value_is_zero(const struct SaveValue *value)
{
    return value->is_float ? (value->real == 0.0) : (value->mag == 0);
}

static int load_generic(const uint8_t *bytes, uint32_t size, uint8_t type, int little, struct SaveValue *value)
{
    *value = SaveValue{};
    if ((type == SV_F32) || (type == SV_F64))
    {
        if ((size != 4) && (size != 8))
            return -1;
        value->is_float = 1;
        if (size == 4)
        {
            auto bits = static_cast<uint32_t>(raw_load(bytes, 4, little));
            float single;
            memcpy(&single, &bits, 4);
            value->real = single;
        } else
        {
            uint64_t bits = raw_load(bytes, 8, little);
            double double_value;
            memcpy(&double_value, &bits, 8);
            value->real = double_value;
        }
        return 0;
    }
    struct IntKind kind;
    if ((int_kind(type, &kind) != 0) || (size == 0) || (size > 8))
        return -1;
    kind.bits = size * 8;
    if (type == SV_BOOL8)
        kind.bits = 8;
    value_from_raw(value, raw_load(bytes, size, little), &kind);
    return 0;
}

static int store_generic(uint8_t *bytes, uint32_t size, uint8_t type, int little, const struct SaveValue *value, int clamp)
{
    if ((type == SV_F32) || (type == SV_F64))
    {
        if (!value->is_float || ((size != 4) && (size != 8)))
            return -1;
        if (size == 4)
        {
            auto single = static_cast<float>(value->real);
            uint32_t bits;
            memcpy(&bits, &single, 4);
            raw_store(bytes, 4, bits, little);
        } else
        {
            uint64_t bits;
            memcpy(&bits, &value->real, 8);
            raw_store(bytes, 8, bits, little);
        }
        return 0;
    }
    struct IntKind kind;
    uint64_t raw;
    if ((int_kind(type, &kind) != 0) || (size == 0) || (size > 8))
        return -1;
    if (type != SV_BOOL8)
        kind.bits = size * 8;
    const int failed = value_to_raw(value, &kind, clamp, &raw);
    if (failed < 0)
        return failed;
    raw_store(bytes, size, raw, little);
    return failed;
}

int save_value_load_le(const uint8_t *bytes, uint8_t type, struct SaveValue *value)
{
    return load_generic(bytes, save_stored_size(type), type, 1, value);
}

int save_value_load_mem(const void *memory, uint32_t size, uint8_t type, struct SaveValue *value)
{
    return load_generic(static_cast<const uint8_t *>(memory), size, type, save_host_is_little_endian(), value);
}

int save_value_store_le(uint8_t *bytes, uint8_t type, const struct SaveValue *value, int clamp)
{
    return store_generic(bytes, save_stored_size(type), type, 1, value, clamp);
}

int save_value_store_mem(void *memory, uint32_t size, uint8_t type, const struct SaveValue *value, int clamp)
{
    return store_generic(static_cast<uint8_t *>(memory), size, type, save_host_is_little_endian(), value, clamp);
}
