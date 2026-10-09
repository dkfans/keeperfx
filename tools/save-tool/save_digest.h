// State checksum for kfx-savetool digest. No game or SDL code.
#ifndef KFX_SAVE_DIGEST_H
#define KFX_SAVE_DIGEST_H

#include "kfx/save/core/save_schema.h"

struct SaveDigest;

/** schema is the one stored in the file; it must outlive the digest. */
struct SaveDigest *save_digest_new(const struct SaveFileSchema *schema);
void save_digest_free(struct SaveDigest *d);

/** Adds one loaded record. The file's own schema decides which fields are read and how they are
 *  encoded; a field the build no longer has is left out. root_name is the struct's name in the
 *  file, bs the build's table for it, and src the state after loading. */
enum SaveResult save_digest_add(struct SaveDigest *d, const char *root_name, const struct SaveStructDesc *bs,
    const void *src, struct SaveError *err);

/** One "path  crc32" line per field path, sorted. */
enum SaveResult save_digest_text(const struct SaveDigest *d, struct SaveBuffer *out, struct SaveError *err);

#endif
