/** @file DaiBlob.h
 *  Daimon's byte codec (02 section 4.15): little-endian, sections of {id, version, length, bytes}. DaiBlobWriter
 *  appends; DaiBlobReader reads untrusted bytes -- every read is bounds-checked, every count a caller reads is checked
 *  against its limit, and the first failure sticks (Ok() is false from then on, every later read gives 0), so a
 *  reader never runs past its buffer and a caller checks once at the end. A base class: each Daimon class saves its
 *  own private state with it; DaiSave puts the keeper together. */
#ifndef DAIMON_DAIBLOB_H
#define DAIMON_DAIBLOB_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "daimon/DaiTypes.h"

/** A section id from four characters ('JOBS'). */
constexpr uint32_t DaiFourCC(char a, char b, char c, char d)
{
    return (uint32_t)(uint8_t)a | ((uint32_t)(uint8_t)b << 8) | ((uint32_t)(uint8_t)c << 16) | ((uint32_t)(uint8_t)d << 24);
}

class DaiBlobWriter {
public:
    void U8(uint8_t v) { m_out.push_back(v); }
    void U16(uint16_t v) { put(v, 2); }
    void U32(uint32_t v) { put(v, 4); }
    void U64(uint64_t v) { put(v, 8); }
    void I64(int64_t v) { put((uint64_t)v, 8); }
    void Bool(bool v) { U8(v ? 1 : 0); }
    void Bytes(const void *p, size_t n);
    /** A string up to 255 bytes (longer is cut). */
    void Str(const char *s);
    /** A thing's reference: its index, then its creation turn (14 K8; the bytes every site wrote by hand). */
    void Ref(const DaiThingRef &r) { I64(r.idx); U64(r.creation_turn); }
    /** Starts a section; its length is filled in by EndSection. Sections do not nest. */
    void BeginSection(uint32_t id, uint16_t version);
    void EndSection();
    const std::vector<uint8_t> &Data() const { return m_out; }
    size_t Size() const { return m_out.size(); }
private:
    void put(uint64_t v, int n) { for (int i = 0; i < n; i++) m_out.push_back((uint8_t)(v >> (8 * i))); }
    std::vector<uint8_t> m_out;
    size_t m_section_len_at = 0;
    bool m_in_section = false;
};

class DaiBlobReader {
public:
    DaiBlobReader(const uint8_t *data, size_t len) : m_data(data), m_len(len) {}
    uint8_t U8() { return (uint8_t)get(1); }
    uint16_t U16() { return (uint16_t)get(2); }
    uint32_t U32() { return (uint32_t)get(4); }
    uint64_t U64() { return get(8); }
    int64_t I64() { return (int64_t)get(8); }
    /** 0 or 1, anything else fails. */
    bool Bool();
    void Bytes(void *p, size_t n);
    std::string Str();
    /** A count, failing when it is above `max`. */
    int64_t Count(int64_t max);
    /** A value, failing when it is outside [lo, hi]. */
    int64_t Range(int64_t lo, int64_t hi);
    /** A thing's reference as Ref wrote it, its index in the game's range. */
    DaiThingRef Ref() { DaiThingRef t; t.idx = (DaiThingIndex)Range(0, INT32_MAX); t.creation_turn = (DaiTurn)U64(); return t; }
    /** The next section's header; false at the end of the data (or on an error). The section's bytes follow: read them
     *  with Sub(), which also moves past them. */
    bool NextSection(uint32_t *id, uint16_t *version, size_t *length);
    /** A reader over the next `n` bytes, and this one moves past them. */
    DaiBlobReader Sub(size_t n);
    void Fail(const char *why) { if (m_why == nullptr) m_why = why; m_ok = false; }
    bool Ok() const { return m_ok; }
    const char *Why() const { return (m_why != nullptr) ? m_why : ""; }
    size_t Left() const { return m_ok ? m_len - m_pos : 0; }
    bool AtEnd() const { return m_ok && (m_pos == m_len); }
private:
    uint64_t get(int n);
    const uint8_t *m_data;
    size_t m_len;
    size_t m_pos = 0;
    bool m_ok = true;
    const char *m_why = nullptr;
};

#endif
