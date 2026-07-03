#ifndef _DK_THREAD
#define _DK_THREAD

#include <thread>

#if defined(__MINGW32__) && !defined(_GLIBCXX_HAS_GTHREADS)

// MingW32 10 and lower are broken, 13 (and higher?) work out of the box.
// Implementation below is sufficient for our needs.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdexcept>
#include <chrono>

namespace std {
namespace this_thread {

template<class Rep, class Period>
void sleep_for(const chrono::duration<Rep, Period> & rel_time)
{
    Sleep(std::chrono::duration_cast<std::chrono::milliseconds>(rel_time).count());
}

};
};

#endif

#endif
