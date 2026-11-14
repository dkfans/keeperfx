#include "pre_inc.h"
#include "daimon/DaiLog.h"

#include <cinttypes>
#include <cstdarg>
#include <cstdio>

#include "post_inc.h"

namespace {
thread_local std::vector<DaiLogLine> *t_lines = nullptr;
thread_local DaiTurn t_turn = 0;

void format_line(char *buf, size_t size, PlayerNumber plyr, const char *fmt, va_list ap)
{
    const int n = snprintf(buf, size, "p%d ", (int)plyr);
    if ((n > 0) && ((size_t)n < size))
        vsnprintf(buf + n, size - (size_t)n, fmt, ap);
}

/** Held for the main thread when a capture is on; else written now. */
void put(DaiLogLevel level, const char *buf)
{
    DaiLogLine l;
    l.level = level;
    l.turn = (level == DaiLogLevel::Warn) ? ((t_lines != nullptr) ? t_turn : DaiLog::EngineTurn()) : 0;
    l.text = buf;
    if (t_lines != nullptr) t_lines->push_back(std::move(l));
    else DaiLog::Write(l);
}
}

void DaiLog::Info(PlayerNumber plyr, const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    format_line(buf, sizeof(buf), plyr, fmt, ap);
    va_end(ap);
    put(DaiLogLevel::Info, buf);
}

void DaiLog::Debug(PlayerNumber plyr, const char *fmt, ...)
{
    if (!DaiLog::DebugOn()) return;
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    format_line(buf, sizeof(buf), plyr, fmt, ap);
    va_end(ap);
    put(DaiLogLevel::Debug, buf);
}

void DaiLog::Warn(PlayerNumber plyr, const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    format_line(buf, sizeof(buf), plyr, fmt, ap);
    va_end(ap);
    put(DaiLogLevel::Warn, buf);
}

void DaiLog::BeginCapture(std::vector<DaiLogLine> *lines, DaiTurn turn)
{
    t_lines = lines;
    t_turn = turn;
}

// DaiLog::Write, DebugOn and EngineTurn are the engine side: DaiLogSink.cpp (the seam).
