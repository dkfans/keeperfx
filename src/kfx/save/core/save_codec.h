/******************************************************************************/
/** @file save_codec.h
 *     KFXS container: file header, chunks, CRC and zlib. No game dependencies.
 */
/******************************************************************************/
#ifndef KFX_SAVE_CODEC_H
#define KFX_SAVE_CODEC_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define SAVE_FORMAT_MAJOR 1
#define SAVE_FORMAT_MINOR 0
#define SAVE_MIN_READABLE_MAJOR 1
#define SAVE_MAGIC_SIZE 8
#define SAVE_HEADER_SIZE 20
#define SAVE_CHUNK_HEADER_SIZE 20
#define SAVE_MAX_RAW_LEN (256u << 20)

#define SAVE_FOURCC(a, b, c, d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b) << 8) | \
    ((uint32_t)(uint8_t)(c) << 16) | ((uint32_t)(uint8_t)(d) << 24))

/** The id of a chunk: four characters, stored in the file in this order. */
enum SaveChunkId {
    /** The title record (CatalogueEntry) of a savegame or replay. */
    SCID_Meta    = SAVE_FOURCC('M','E','T','A'),
    /** The schema: every struct and field this file's records are made of. */
    SCID_Schema  = SAVE_FOURCC('S','C','H','M'),
    /** The code names of the config tables the saved indices refer to. */
    SCID_Names   = SAVE_FOURCC('N','A','M','S'),
    /** The whole game state (struct Game). */
    SCID_Game    = SAVE_FOURCC('G','A','M','E'),
    /** The config values the level changed from what its config files give. */
    SCID_Config  = SAVE_FOURCC('C','O','N','F'),
    /** The campaign progress data (IntralevelData). */
    SCID_Level   = SAVE_FOURCC('I','L','V','L'),
    /** The serialised Lua state. */
    SCID_Lua     = SAVE_FOURCC('L','U','A',' '),
    /** A count-prefixed run of same-typed records, concatenated with no per-record framing (decode
     *  advances by each record's own decoded byte count). Used by the smaller files that hold an
     *  array of one struct type, such as the high scores. */
    SCID_Records = SAVE_FOURCC('R','E','C','S'),
    /** Replay (.pck) files only: a schema-encoded PacketSaveHead record. */
    SCID_PacketHead = SAVE_FOURCC('P','H','D','R'),
    /** Replay (.pck) files only: a zero-length marker. Everything in the file after this chunk is the
     *  raw, streamed, per-turn packet codec output, not another KFXS chunk. */
    SCID_TurnData = SAVE_FOURCC('T','U','R','N')
};

/** What a KFXS file holds, stored in its header as four characters. A reader refuses a file of another kind. */
enum SaveFileKind {
    /** A savegame. */
    SFK_Save = SAVE_FOURCC('S','A','V','E'),
    /** A continue file: the link to the last save, or the progress of a campaign. */
    SFK_Continue = SAVE_FOURCC('C','O','N','T'),
    /** A replay (.pck). */
    SFK_Replay = SAVE_FOURCC('R','E','P','L'),
    /** The high score table of a campaign. */
    SFK_HighScore = SAVE_FOURCC('H','S','C','R'),
    /** The network config. */
    SFK_NetConfig = SAVE_FOURCC('N','C','F','G'),
    /** Reserved for the game state sent to a player that fell out of sync. Nothing writes a file of this kind. BUT HEY, WE'LL SEE */
    SFK_Resync = SAVE_FOURCC('R','S','Y','N')
};

/** Flags of a chunk, stored in its header. */
enum SaveChunkFlags {
    /** The stored bytes are zlib compressed. Set by the writer, only when it makes the chunk smaller. */
    SCF_Zlib      = 0x01,
    /** A build that doesn't know the chunk must refuse the file, not skip the chunk. */
    SCF_Required  = 0x02,
    /** Reserved. Nothing sets or reads it. */
    SCF_PerRecord = 0x04,
    /** The chunk holds raw memory of the machine that wrote it (union data that has no field table), which
     *  is stored in that machine's byte order. A reader with the other byte order can't use those bytes. */
    SCF_RawBytes  = 0x08,
    /** With SCF_RawBytes: the machine that wrote the chunk was big endian. */
    SCF_RawBigEndian = 0x10
};

/** The outcome of a codec call. Anything but SVR_Ok comes with a message in the SaveError. */
enum SaveResult {
    /** It worked. */
    SVR_Ok,
    /** The file doesn't exist, or the end of the chunks was reached. */
    SVR_NotFound,
    /** The file is truncated, a checksum fails, or it doesn't follow the format. */
    SVR_Damaged,
    /** The file was made by a newer build than this one. */
    SVR_TooNew,
    /** The file was made before the save format changed, or by a build too old to read. */
    SVR_TooOld,
    /** The content is well formed but this build can't take it, for example a value that doesn't fit. */
    SVR_Unsupported,
    /** An allocation failed. */
    SVR_NoMemory
};

struct SaveError {
    enum SaveResult result;
    char message[256];
};

struct SaveBuffer {
    uint8_t *data;
    uint32_t len;
    uint32_t cap;
    /** Set by the encoder when the bytes hold raw memory (see SCF_RawBytes). */
    bool has_raw_bytes;
};

struct SaveChunk {
    uint32_t id;
    uint32_t flags;
    uint32_t stored_len;
    uint32_t raw_len;
    uint32_t crc32;
    const uint8_t *body;
};

struct SaveReader {
    const uint8_t *data;
    uint32_t len;
    uint32_t pos;
    uint16_t major;
    uint16_t minor;
    uint32_t kind;
};
/******************************************************************************/
enum SaveResult save_fail(struct SaveError *err, enum SaveResult result, const char *fmt, ...);

/** Whether the first SAVE_MAGIC_SIZE bytes (len of them are available) are the magic of a KFXS file. The magic
 *  has a zero byte and bytes that are not valid UTF-8, so that a KFXS file is never taken for text. */
bool save_magic_matches(const void *bytes, uint32_t len);

void save_buf_free(struct SaveBuffer *buffer);
int save_buf_append(struct SaveBuffer *buffer, const void *data, uint32_t len);
int save_buf_u8(struct SaveBuffer *buffer, uint8_t value);
int save_buf_u16(struct SaveBuffer *buffer, uint16_t value);
int save_buf_u32(struct SaveBuffer *buffer, uint32_t value);

/** Writes the file header; returns 0 on success. */
int save_file_begin(struct SaveBuffer *buffer, uint32_t kind);
/** Appends a chunk, zlib level 1 when asked and when it makes the chunk smaller. */
int save_chunk_add(struct SaveBuffer *buffer, uint32_t id, uint32_t flags, const void *raw, uint32_t raw_len, int compress);
/** Appends the contents of an encoder's buffer as a chunk, with SCF_RawBytes set (and SCF_RawBigEndian when
 *  this machine is big endian) if the encoder wrote raw memory into it. */
int save_chunk_add_buffer(struct SaveBuffer *buffer, uint32_t id, uint32_t flags, const struct SaveBuffer *payload,
    int compress);
/** Whether this machine stores the low byte of a number first. */
bool save_host_is_little_endian(void);

enum SaveResult save_reader_open(struct SaveReader *reader, const void *data, uint32_t len, struct SaveError *err);
/** Parses the 20 bytes of a chunk header at p and checks them against the bytes left in the file after
 *  the header. chunk->body is left NULL. */
enum SaveResult save_chunk_head(const uint8_t *bytes, uint32_t after, struct SaveChunk *chunk, struct SaveError *err);
/** SVR_NotFound means the end of the file was reached. */
enum SaveResult save_reader_next(struct SaveReader *reader, struct SaveChunk *chunk, struct SaveError *err);
/** Decompresses and CRC-checks a chunk into a malloc'd buffer the caller frees. */
enum SaveResult save_chunk_unpack(const struct SaveChunk *chunk, uint8_t **out, uint32_t *out_len, struct SaveError *err);
/******************************************************************************/
#ifdef __cplusplus
}

/** A SaveBuffer that frees itself. */
struct SaveBufferOwner : SaveBuffer {
    SaveBufferOwner() : SaveBuffer() {}
    ~SaveBufferOwner() { save_buf_free(this); }
    SaveBufferOwner(const SaveBufferOwner &) = delete;
    SaveBufferOwner &operator=(const SaveBufferOwner &) = delete;
};
#endif
#endif
