/** @file DaiLog.h
 *  "DAI:" log lines, the keeper first (02 section 7): `DAI: p2 took slot 4 (default, skirmish)`. While a keeper plans
 *  (12 section 13.5) its lines go to that turn's output instead, through a sink set on the planning thread, and are
 *  written by the main thread when the output is applied: the text is the same either way. */
#ifndef DAIMON_DAILOG_H
#define DAIMON_DAILOG_H

#include "daimon_base_types.h"
#include <cstdint>
#include <string>
#include <vector>

#include "globals.h"

enum class DaiLogLevel : uint8_t { Info, Debug, Warn };

/** A line held for the main thread: a Warn line keeps the turn it was planned at (the game log's turn stamp). */
struct DaiLogLine {
    DaiLogLevel level = DaiLogLevel::Info;
    DaiTurn turn = 0;
    std::string text;           /**< "p2 ..." without the "DAI: " */
};

class DaiLog {
public:
    /** A line the player may want in the log at the normal level (take, hand back, posture). */
    static void Info(PlayerNumber plyr, const char *fmt, ...);
    /** A detail line (jobs, gestures): only with the DEBUG log level. */
    static void Debug(PlayerNumber plyr, const char *fmt, ...);
    static void Warn(PlayerNumber plyr, const char *fmt, ...);

    /** This thread's lines go into `lines` (a Warn stamped `turn`) until EndCapture; nullptr writes them again. */
    static void BeginCapture(std::vector<DaiLogLine> *lines, DaiTurn turn);
    static void EndCapture() { BeginCapture(nullptr, 0); }
    /** Writes a held line as the call that made it would have. */
    static void Write(const DaiLogLine &l);
    /** Whether debug lines are wanted (the engine's debug level). DaiLogSink.cpp, with Write and EngineTurn: the
     *  engine side, which a port writes again. */
    static bool DebugOn();
    /** The game turn a warning written now carries. */
    static DaiTurn EngineTurn();
};

#endif
