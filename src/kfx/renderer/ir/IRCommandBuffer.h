#ifndef RENDERER_IR_IRCOMMANDBUFFER_H
#define RENDERER_IR_IRCOMMANDBUFFER_H

#include <cstddef>
#include <vector>

// Append-only per-frame command buffer. Single writer (game thread submit),
// single reader (replay).
template<typename T>
class IRCommandBuffer
{
public:
    void Reserve(size_t n) { m_cmds.reserve(n); }

    T& Append(const T& cmd)
    {
        m_cmds.push_back(cmd);
        return m_cmds.back();
    }

    void Reset() { m_cmds.clear(); }
    void Swap(IRCommandBuffer<T>& other) { m_cmds.swap(other.m_cmds); }

    const T* Data()  const { return m_cmds.data(); }
    size_t   Size()  const { return m_cmds.size(); }
    bool     Empty() const { return m_cmds.empty(); }

private:
    std::vector<T> m_cmds;
};

#endif // RENDERER_IR_IRCOMMANDBUFFER_H
