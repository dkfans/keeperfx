/** @file DaiRng.h
 *  A keeper's own random stream (02 principle 6): seeded at take, saved with the keeper, never the game's. */
#ifndef DAIMON_DAIRNG_H
#define DAIMON_DAIRNG_H

#include <cstdint>

class DaiRng {
public:
    explicit DaiRng(uint64_t seed = 1) { Seed(seed); }
    /** Restarts the stream; the same seed gives the same numbers on every machine. */
    void Seed(uint64_t seed);
    uint64_t Next();
    /** A number in [0, n); 0 when n <= 0. */
    int64_t Below(int64_t n);
    uint64_t State() const { return m_state; }
    void SetState(uint64_t state) { m_state = state; }
private:
    uint64_t m_state = 0;
};

#endif
