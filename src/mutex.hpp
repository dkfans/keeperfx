#ifndef _DK_MUTEX_
#define _DK_MUTEX_

#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)

// MingW32 10 and lower are broken, 13 (and higher?) work out of the box.
// Implementation below is sufficient for our needs.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdexcept>

namespace std {

class mutex {
public:
    mutex()
    {
        m_handle = CreateMutex(nullptr, false, nullptr);
        if (!m_handle) {
            throw std::runtime_error("Cannot create mutex");
        }
    }

    ~mutex() noexcept
    {
        CloseHandle(m_handle);
    }

    mutex(const mutex &) = delete;
    mutex & operator=(const mutex &) = delete;

    void lock()
    {
        const auto result = WaitForSingleObject(m_handle, INFINITE);
        if (result != WAIT_OBJECT_0) {
            throw std::runtime_error("Cannot acquire mutex");
        }
    }

    bool try_lock()
    {
        const auto result = WaitForSingleObject(m_handle, 0);
        if (result == WAIT_OBJECT_0) {
            return true;
        } else if (result == WAIT_TIMEOUT) {
            return false;
        }
        throw std::runtime_error("Error acquiring mutex");
    }

    void unlock() noexcept
    {
        ReleaseMutex(m_handle);
    }

protected:
    HANDLE m_handle = nullptr;
};

}

#endif

#include <mutex>

#endif
