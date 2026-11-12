#include "pre_inc.h"
#include "daimon/DaiRng.h"
#include "post_inc.h"

void DaiRng::Seed(uint64_t seed)
{
    m_state = seed;
}

uint64_t DaiRng::Next()
{
    // splitmix64: one 64-bit state, fast, and the same everywhere.
    uint64_t z = (m_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

int64_t DaiRng::Below(int64_t n)
{
    if (n <= 0) return 0;
    return (int64_t)(Next() % (uint64_t)n);
}
