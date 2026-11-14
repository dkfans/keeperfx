#include "pre_inc.h"
#include "daimon/DaiBlob.h"

#include <cstring>

#include "post_inc.h"

void DaiBlobWriter::Bytes(const void *p, size_t n)
{
    if (n == 0) return;
    const uint8_t *b = static_cast<const uint8_t *>(p);
    m_out.insert(m_out.end(), b, b + n);
}

void DaiBlobWriter::Str(const char *s)
{
    const size_t n = (s != nullptr) ? strnlen(s, 255) : 0;
    U8((uint8_t)n);
    Bytes(s, n);
}

void DaiBlobWriter::BeginSection(uint32_t id, uint16_t version)
{
    U32(id);
    U16(version);
    m_section_len_at = m_out.size();
    U32(0);
    m_in_section = true;
}

void DaiBlobWriter::EndSection()
{
    if (!m_in_section) return;
    const uint32_t len = (uint32_t)(m_out.size() - m_section_len_at - 4);
    for (int i = 0; i < 4; i++) m_out[m_section_len_at + (size_t)i] = (uint8_t)(len >> (8 * i));
    m_in_section = false;
}

uint64_t DaiBlobReader::get(int n)
{
    if (!m_ok || ((size_t)n > m_len - m_pos)) {
        Fail("truncated");
        return 0;
    }
    uint64_t v = 0;
    for (int i = 0; i < n; i++) v |= (uint64_t)m_data[m_pos + (size_t)i] << (8 * i);
    m_pos += (size_t)n;
    return v;
}

bool DaiBlobReader::Bool()
{
    const uint8_t v = U8();
    if (v > 1) Fail("not a boolean");
    return v == 1;
}

void DaiBlobReader::Bytes(void *p, size_t n)
{
    if (n == 0) return;
    if (!m_ok || (n > m_len - m_pos)) {
        Fail("truncated");
        memset(p, 0, n);
        return;
    }
    memcpy(p, m_data + m_pos, n);
    m_pos += n;
}

std::string DaiBlobReader::Str()
{
    const size_t n = U8();
    if (!m_ok || (n > m_len - m_pos)) {
        Fail("truncated");
        return std::string();
    }
    std::string s((const char *)m_data + m_pos, n);
    m_pos += n;
    return s;
}

int64_t DaiBlobReader::Count(int64_t max)
{
    const int64_t v = I64();
    if ((v < 0) || (v > max)) {
        Fail("count out of range");
        return 0;
    }
    return v;
}

int64_t DaiBlobReader::Range(int64_t lo, int64_t hi)
{
    const int64_t v = I64();
    if ((v < lo) || (v > hi)) {
        Fail("value out of range");
        return lo;
    }
    return v;
}

bool DaiBlobReader::NextSection(uint32_t *id, uint16_t *version, size_t *length)
{
    if (!m_ok || (m_pos == m_len)) return false;
    *id = U32();
    *version = U16();
    *length = U32();
    if (m_ok && (*length > m_len - m_pos)) Fail("section longer than the data");
    return m_ok;
}

DaiBlobReader DaiBlobReader::Sub(size_t n)
{
    if (!m_ok || (n > m_len - m_pos)) {
        Fail("truncated");
        DaiBlobReader bad(nullptr, 0);
        bad.Fail("truncated");
        return bad;
    }
    DaiBlobReader sub(m_data + m_pos, n);
    m_pos += n;
    return sub;
}
