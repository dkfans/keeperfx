/******************************************************************************/
/** @file save_schema.h
 *     Field tables, the schema stored in files, and the record encoder/decoder.
 */
/******************************************************************************/
#ifndef KFX_SAVE_SCHEMA_H
#define KFX_SAVE_SCHEMA_H

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include "save_codec.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** How a field is stored in a file, whatever its type in memory is. A field keeps its stored type when the
 *  C type changes, so a file doesn't change with the compiler. */
enum SaveStoredType {
    SV_INVALID,
    /** Unsigned and signed integers of 8, 16, 32 and 64 bits, little endian. */
    SV_U8, SV_I8, SV_U16, SV_I16, SV_U32, SV_I32, SV_U64, SV_I64,
    /** IEEE floats of 32 and 64 bits. */
    SV_F32, SV_F64,
    /** A boolean, stored as one byte. */
    SV_BOOL8,
    /** A fixed-size char array holding a string; the array's length is the string's capacity. */
    SV_STR,
    /** A struct with a field table of its own (SaveFieldDesc::sub). */
    SV_STRUCT,
    /** A union: a code for the member in use, then that member (SaveFieldDesc::uni). */
    SV_UNION
};

/** How a field takes part in saving. A field with none of the SAVE, SYNC, DERIVED and RUNTIME bits is
 *  saved and synced. */
enum SaveFieldFlags {
    /** Stored in a savegame. */
    SVF_SAVE    = 0x01,
    /** Sent to a player that fell out of sync in a network game. */
    SVF_SYNC    = 0x02,
    /** Not stored: the load works it out again from the stored fields. */
    SVF_DERIVED = 0x04,
    /** Not stored: the load keeps the value the running game has. */
    SVF_RUNTIME = 0x08,
    /** A value that doesn't fit the type it is converted to is cut to the nearest value that does, instead
     *  of the conversion being refused. */
    SVF_CLAMP   = 0x10,
    /** The field is raw memory with no field table. It is stored as it is in memory, so it can't be read
     *  by a machine with the other byte order (the chunk is flagged SCF_RawBytes). */
    SVF_RAW     = 0x20
};

/** Which fields a record holds. */
enum SaveMode {
    /** A savegame: the fields with SVF_SAVE. */
    SVM_Save,
    /** A network resync: the fields with SVF_SYNC. */
    SVM_Sync
};

/** The config table a field's value is an index into (SaveFieldDesc::name_domain). The save keeps the code
 *  name of every entry of the table, so a load can tell when the indices mean something else in this
 *  build or mod set, and refuses the save then. */
enum SaveNameDomain {
    /** The field is not such an index. */
    SND_None = 0,
    SND_CreatureModel,
    SND_RoomKind,
    SND_Trap,
    SND_Door,
    SND_Object,
    SND_Spell,
    SND_Power,
    SND_Shot,
    SND_Effect,
    SND_EffectElement,
    SND_EffectGenerator,
    SND_Instance,
    /** Thing.model: which table it indexes depends on the thing's class. */
    SND_ByThingClass,
    /** Dungeon.manufacture_kind: which table it indexes depends on the manufacture class. */
    SND_ByManufactureClass,
    /** The number of domains; not a domain. */
    SND_COUNT
};

struct SaveStructDesc;

#define SAVE_UNION_MAX_VALUES 8
#define SAVE_UNION_MAX_MEMBERS 254

struct SaveUnionMember {
    const char *name;
    const struct SaveStructDesc *sub;
    uint32_t raw_size;
    uint8_t is_default;
    uint8_t nvalues;
    int32_t values[SAVE_UNION_MAX_VALUES]; 
};

struct SaveUnionDesc {
    /** The integer field of the same struct that tells which member is in use. NULL when a selector does. */
    const char *disc;
    /** The name of the function, registered with save_register_union_selector, that tells which member is in
     *  use from the whole struct. NULL when a field does. */
    const char *selector;
    uint32_t member_count;
    const struct SaveUnionMember *members;
};

struct SaveFieldDesc {
    const char *name;
    uint32_t offset;
    uint32_t mem_size;            /* sizeof the whole member */
    uint32_t dims[3];             /* 0 = unused; none means a single element */
    uint8_t stored_type;
    uint8_t flags;                /* no save/sync/derived/runtime bit means save + sync */
    const struct SaveStructDesc *sub;
    const struct SaveUnionDesc *uni;
    const char *const *aliases;   /* previous names, NULL-terminated */
    int32_t default_value;
    uint16_t name_domain;
    const char *rebuilt_by;
};

struct SaveStructDesc {
    const char *name;
    uint32_t size;
    uint32_t field_count;
    const struct SaveFieldDesc *fields;
};

#define SAVE_DECLARE(Name) extern const struct SaveStructDesc save_desc_##Name
#define SAVE_DECLARE_UNION(Name) extern const struct SaveUnionDesc save_ud_##Name

/* In every field macro below, T is the full type of the containing struct (struct Thing) and f the member. */

#define SAVE_MEMBER(T, f, ST) \
    .name = #f, .offset = (uint32_t)offsetof(T, f), .mem_size = (uint32_t)sizeof(((T *)0)->f), .stored_type = (ST)

/* A plain member; _EX adds designated initialisers, for example .flags or .aliases. */
#define SAVE_FIELD_EX(T, f, ST, ...) { SAVE_MEMBER(T, f, ST), __VA_ARGS__ }
#define SAVE_FIELD(T, f, ST) SAVE_FIELD_EX(T, f, ST, .flags = 0)

/* A member that is a struct with its own table. */
#define SAVE_SUB_EX(T, f, Sub, ...) { SAVE_MEMBER(T, f, SV_STRUCT), .sub = &save_desc_##Sub, __VA_ARGS__ }
#define SAVE_SUB(T, f, Sub) SAVE_SUB_EX(T, f, Sub, .flags = 0)

/* Array dimensions, taken from the member's own type so a changed #define shows up in the schema. */
#define SAVE_DIM1(T, f) ((uint32_t)(sizeof(((T *)0)->f) / sizeof(((T *)0)->f[0])))
#define SAVE_DIM2(T, f) ((uint32_t)(sizeof(((T *)0)->f[0]) / sizeof(((T *)0)->f[0][0])))

/* Arrays of one or two dimensions of a plain member (_ARRAY) or of structs (_SUB_ARRAY). */
#define SAVE_ARRAY_EX(T, f, ST, ...) SAVE_FIELD_EX(T, f, ST, .dims = { SAVE_DIM1(T, f) }, __VA_ARGS__)
#define SAVE_ARRAY(T, f, ST) SAVE_FIELD_EX(T, f, ST, .dims = { SAVE_DIM1(T, f) })
#define SAVE_ARRAY2_EX(T, f, ST, ...) SAVE_FIELD_EX(T, f, ST, .dims = { SAVE_DIM1(T, f), SAVE_DIM2(T, f) }, __VA_ARGS__)
#define SAVE_ARRAY2(T, f, ST) SAVE_FIELD_EX(T, f, ST, .dims = { SAVE_DIM1(T, f), SAVE_DIM2(T, f) })
#define SAVE_SUB_ARRAY_EX(T, f, Sub, ...) SAVE_SUB_EX(T, f, Sub, .dims = { SAVE_DIM1(T, f) }, __VA_ARGS__)
#define SAVE_SUB_ARRAY(T, f, Sub) SAVE_SUB_EX(T, f, Sub, .dims = { SAVE_DIM1(T, f) })
#define SAVE_SUB_ARRAY2(T, f, Sub) SAVE_SUB_EX(T, f, Sub, .dims = { SAVE_DIM1(T, f), SAVE_DIM2(T, f) })

/* A char array holding one string. */
#define SAVE_STRING(T, f) SAVE_ARRAY(T, f, SV_STR)

/* Members the format doesn't store: rebuilt after a load (fn names what rebuilds it), or kept from the running game. */
#define SAVE_DERIVED(T, f, fn) { SAVE_MEMBER(T, f, SV_U8), .flags = SVF_DERIVED, .rebuilt_by = (fn) }
#define SAVE_RUNTIME(T, f) { SAVE_MEMBER(T, f, SV_U8), .flags = SVF_RUNTIME }

/* Unions: SAVE_UNION_DEF (a field tells the member) or SAVE_UNION_SELECTED_DEF (a function does) lists the members,
   SAVE_UNION puts the union in a struct table. A member is a struct table (_SUB), a table that several members of the
   same layout share (_SHARED, covers names them), raw bytes (_RAW, covers names the members the bytes stand for) or
   nothing (_NONE). */
#define SAVE_OPAQUE(T, at, label, bytes) { .name = (label), .offset = (uint32_t)offsetof(T, at), \
    .mem_size = (bytes), .dims = { (bytes) }, .stored_type = SV_U8, .flags = SVF_SAVE | SVF_SYNC | SVF_RAW }
#define SAVE_UNION_MEMBER_SUB(label, Name, ...) { .name = (label), .sub = &save_desc_##Name, __VA_ARGS__ }
#define SAVE_UNION_MEMBER_SHARED(label, Name, covers, ...) { .name = (label), .sub = &save_desc_##Name, __VA_ARGS__ }
#define SAVE_UNION_MEMBER_RAW(label, bytes, covers, ...) { .name = (label), .raw_size = (bytes), __VA_ARGS__ }
#define SAVE_UNION_MEMBER_NONE(label, ...) { .name = (label), __VA_ARGS__ }
#define SAVE_UNION_DEF(Name, discname, ...) \
    static const struct SaveUnionMember save_um_##Name[] = { __VA_ARGS__ }; \
    const struct SaveUnionDesc save_ud_##Name = { .disc = discname, \
        .member_count = (uint32_t)(sizeof(save_um_##Name) / sizeof(save_um_##Name[0])), .members = save_um_##Name }
#define SAVE_UNION_SELECTED_DEF(Name, selector_name, ...) \
    static const struct SaveUnionMember save_um_##Name[] = { __VA_ARGS__ }; \
    const struct SaveUnionDesc save_ud_##Name = { .selector = selector_name, \
        .member_count = (uint32_t)(sizeof(save_um_##Name) / sizeof(save_um_##Name[0])), .members = save_um_##Name }
#define SAVE_UNION(T, at, label, bytes, Name) { .name = (label), .offset = (uint32_t)offsetof(T, at), \
    .mem_size = (bytes), .stored_type = SV_UNION, .uni = &save_ud_##Name, .flags = 0 }

/* The table of one struct. */
#define SAVE_STRUCT(Name, T, ...) \
    static const struct SaveFieldDesc save_fields_##Name[] = { __VA_ARGS__ }; \
    const struct SaveStructDesc save_desc_##Name = { #Name, (uint32_t)sizeof(T), \
        (uint32_t)(sizeof(save_fields_##Name) / sizeof(save_fields_##Name[0])), save_fields_##Name }

/** Tells which member of a union is in use, as the integer that the members' values list. owner is the struct
 *  that holds the union. */
typedef int64_t (*SaveUnionSelector)(const void *owner);
/** Makes a selector available to the unions that name it. Done once, at startup, before anything is saved. A union
 *  whose selector isn't registered stores its default member. */
int save_register_union_selector(const char *name, SaveUnionSelector selector);
/** The member of a union field of owner (a struct described by desc) that is in use: its index in the union, or
 *  -1 when none is. */
enum SaveResult save_union_pick(const struct SaveStructDesc *desc, const struct SaveFieldDesc *field, const void *owner,
    int *pick, struct SaveError *err);
/** The offset, inside the struct, just past the last byte that the fields of a table cover. */
uint32_t save_struct_extent(const struct SaveStructDesc *desc);

uint32_t save_stored_size(uint8_t type);
const char *save_stored_name(uint8_t type);
uint32_t save_field_count(const struct SaveFieldDesc *field);
uint32_t save_field_dim_count(const struct SaveFieldDesc *field);
uint8_t save_field_flags(const struct SaveFieldDesc *field);
int save_field_included(const struct SaveFieldDesc *field, enum SaveMode mode);

void save_copy_runtime(const struct SaveStructDesc *desc, void *dst, const void *src);

struct SaveStructList {
    const struct SaveStructDesc **structs;
    uint32_t count;
    uint32_t capacity;
};
enum SaveResult save_schema_collect(struct SaveStructList *list, const struct SaveStructDesc *const *roots, uint32_t count,
    enum SaveMode mode, struct SaveError *err);

/******************************************************************************/
/* Schema as stored in the SCHM chunk. */
struct SaveFileMember {
    char *name;
    char *sub_type;
    uint32_t raw_size;
};

struct SaveFileField {
    char *disc;
    uint32_t member_count;
    struct SaveFileMember *members;
    char *name;
    uint8_t stored_type;
    uint8_t flags;
    uint8_t dim_count;
    uint32_t dims[3];
    char *sub_type;
};

struct SaveFileStruct {
    char *name;
    uint32_t field_count;
    struct SaveFileField *fields;
};

struct SaveFileSchema {
    uint32_t struct_count;
    struct SaveFileStruct *structs;
};

enum SaveResult save_schema_encode(struct SaveBuffer *out, const struct SaveStructDesc *const *roots,
    uint32_t root_count, enum SaveMode mode, struct SaveError *err);
enum SaveResult save_schema_decode(const uint8_t *data, uint32_t len, struct SaveFileSchema *schema, struct SaveError *err);
void save_schema_free(struct SaveFileSchema *schema);

struct SaveStashEntry {
    char *path;
    const struct SaveFileField *field;
    uint8_t *bytes;
    uint32_t len;
    uint8_t used;
};

struct SaveDecoder {
    const struct SaveFileSchema *schema;
    enum SaveMode mode;
    struct SaveStashEntry *stash;
    uint32_t stash_count;
    void *priv;
    /** The record being decoded has raw memory made by a machine with the other byte order. */
    int foreign_raw_bytes;
};

enum SaveResult save_encode_record(struct SaveBuffer *out, const struct SaveStructDesc *desc, const void *src,
    enum SaveMode mode, struct SaveError *err);

void save_decoder_init(struct SaveDecoder *decoder, const struct SaveFileSchema *schema, enum SaveMode mode);
/** Tells the decoder which chunk its next records come from. Raw memory in a chunk that was written by a
 *  machine with the other byte order is then refused instead of decoded wrongly. */
void save_decoder_set_chunk_flags(struct SaveDecoder *decoder, uint32_t chunk_flags);
void save_decoder_free(struct SaveDecoder *decoder);

enum SaveResult save_decode_record(struct SaveDecoder *decoder, const struct SaveStructDesc *desc,
    const uint8_t *data, uint32_t len, uint32_t *used, void *dst, struct SaveError *err);

struct SaveValue {
    int is_float;
    int neg;
    uint64_t mag;
    double real;
};

int save_value_load_le(const uint8_t *bytes, uint8_t type, struct SaveValue *value);
int save_value_load_mem(const void *memory, uint32_t size, uint8_t type, struct SaveValue *value);
int save_value_store_le(uint8_t *bytes, uint8_t type, const struct SaveValue *value, int clamp);
int save_value_store_mem(void *memory, uint32_t size, uint8_t type, const struct SaveValue *value, int clamp);
int save_value_is_zero(const struct SaveValue *value);
/******************************************************************************/
#ifdef __cplusplus
}

/** A schema read from a file, the decoder over it, and a list of structs, that free themselves. Declare the schema first. */
struct SaveFileSchemaOwner : SaveFileSchema {
    SaveFileSchemaOwner() : SaveFileSchema() {}
    ~SaveFileSchemaOwner() { save_schema_free(this); }
    SaveFileSchemaOwner(const SaveFileSchemaOwner &) = delete;
    SaveFileSchemaOwner &operator=(const SaveFileSchemaOwner &) = delete;
};
struct SaveStructListOwner : SaveStructList {
    SaveStructListOwner() : SaveStructList() {}
    ~SaveStructListOwner() { free(static_cast<void *>(structs)); }
    SaveStructListOwner(const SaveStructListOwner &) = delete;
    SaveStructListOwner &operator=(const SaveStructListOwner &) = delete;
};
struct SaveDecoderOwner : SaveDecoder {
    SaveDecoderOwner() : SaveDecoder() {}
    ~SaveDecoderOwner() { save_decoder_free(this); }
    SaveDecoderOwner(const SaveDecoderOwner &) = delete;
    SaveDecoderOwner &operator=(const SaveDecoderOwner &) = delete;
};
#endif
#endif
