/** @file DaiPool.h
 *  A fixed pool of values with monotone ids (02 section 6.2): allocated with the keeper, never grown; ids are never
 *  reused within a level and 0 means none; iteration is in slot order, which depends only on history (principle 6). */
#ifndef DAIMON_DAIPOOL_H
#define DAIMON_DAIPOOL_H

#include <array>
#include <cstdint>

/** T has a `uint32_t id` member; a slot whose id is 0 is free. */
template <typename T, int64_t N>
class DaiPool {
public:
    /** A new value in the first free slot, with the next id; nullptr if the pool is full. */
    T *Add()
    {
        for (T &v : m_slots) {
            if (v.id != 0) continue;
            v = T{};
            v.id = m_next_id++;
            m_count++;
            return &v;
        }
        return nullptr;
    }
    T *Find(uint32_t id)
    {
        if (id == 0) return nullptr;
        for (T &v : m_slots) if (v.id == id) return &v;
        return nullptr;
    }
    const T *Find(uint32_t id) const { return const_cast<DaiPool *>(this)->Find(id); }
    void Remove(uint32_t id)
    {
        T *v = Find(id);
        if (v == nullptr) return;
        *v = T{};
        m_count--;
    }
    /** Every value goes; ids keep counting (a level's ids stay unique). */
    void Clear()
    {
        for (T &v : m_slots) v = T{};
        m_count = 0;
    }
    template <typename F> void ForEach(F f) { for (T &v : m_slots) if (v.id != 0) f(v); }
    template <typename F> void ForEach(F f) const { for (const T &v : m_slots) if (v.id != 0) f(v); }
    int64_t Count() const { return m_count; }
    static constexpr int64_t Capacity() { return N; }
    uint32_t NextId() const { return m_next_id; }
    /** A slot as it is (id 0: free): a saved game keeps slot positions, so iteration order survives a load. */
    const T &SlotAt(int64_t i) const { return m_slots[(size_t)i]; }
    /** A loaded value back into its slot: false if the slot is out of range or taken, the id is 0, not below the
     *  next id, or already in use. */
    bool RestoreAt(int64_t i, const T &v)
    {
        if ((i < 0) || (i >= N) || (v.id == 0) || (v.id >= m_next_id) || (m_slots[(size_t)i].id != 0) || (Find(v.id) != nullptr))
            return false;
        m_slots[(size_t)i] = v;
        m_count++;
        return true;
    }
    /** The loaded next id, before any RestoreAt (must be above every restored id). */
    void SetNextId(uint32_t id) { m_next_id = (id == 0) ? 1 : id; }
private:
    std::array<T, (size_t)N> m_slots{};
    uint32_t m_next_id = 1;
    int64_t m_count = 0;
};

#endif
