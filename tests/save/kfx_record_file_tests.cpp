/******************************************************************************/
/** @file kfx_record_file_tests.cpp
 *     Standalone tests for the files of records (network config, high scores, continue files)
 *     and the whole-file helpers, against an in-memory file system.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "front_network.h"
#include "config_campaigns.h"
#include "game_merge.h"
#include "kfx/save/SaveTypes.h"
#include "post_inc.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "MemoryFileSystem.h"
#include "kfx/save/SaveManager_Internal.h"
#include "kfx/save/core/schema/save_tables_decl.h"

// The save manager logs through these; the tests don't need the lines.
extern "C" int LbJustLog(const char *, ...) { return 0; }
extern "C" int LbWarnLog(const char *, ...) { return 0; }
extern "C" int LbErrorLog(const char *, ...) { return 0; }

static int checks;
static int fails;

static void expect(const char *what, bool cond)
{
    checks++;
    if (!cond) {
        printf("FAIL %s\n", what);
        fails++;
    }
}

static bool write_net(MemoryFileSystem &fs, const char *fname, const ConfigInfo &ci)
{
    return savefile::WriteRecord(fs, fname, ci);
}

static enum SaveResult read_net(MemoryFileSystem &fs, const char *fname, ConfigInfo &ci, SaveError &err)
{
    return savefile::ReadRecord(fs, fname, ci, &err);
}

static void test_net_config()
{
    MemoryFileSystem fs;
    SaveError err;
    ConfigInfo ci, back;
    memset(&ci, 0, sizeof(ci));
    strcpy(ci.net_player_name, "Tester");
    strcpy(ci.str_join, "127.0.0.1");
    expect("net config written", write_net(fs, "net.cfg", ci));
    memset(&back, 0x7e, sizeof(back));
    expect("net config read", read_net(fs, "net.cfg", back, err) == SVR_Ok);
    expect("net config equal", memcmp(&ci, &back, sizeof(ci)) == 0);

    memset(&back, 0x7e, sizeof(back));
    expect("missing file is NotFound", read_net(fs, "none.cfg", back, err) == SVR_NotFound);
    expect("output untouched when the file is missing", ((unsigned char *)&back)[0] == 0x7e);

    fs.files["cut.cfg"] = std::vector<uint8_t>(fs.files["net.cfg"].begin(), fs.files["net.cfg"].begin() + 20);
    expect("cut file refused", read_net(fs, "cut.cfg", back, err) != SVR_Ok);
    expect("output untouched when the file is cut", ((unsigned char *)&back)[0] == 0x7e);

    fs.files["empty.cfg"];
    expect("zero-length file refused", read_net(fs, "empty.cfg", back, err) != SVR_Ok);
    expect("output untouched for a zero-length file", ((unsigned char *)&back)[0] == 0x7e);

    fs.fail_writes = true;
    expect("write failure is reported", !write_net(fs, "net2.cfg", ci));
    expect("failed write leaves no file", !fs.Exists("net2.cfg"));
}

static void test_high_scores()
{
    MemoryFileSystem fs;
    SaveError err;
    HighScore hs[5];
    memset(hs, 0, sizeof(hs));
    for (int i = 0; i < 5; i++) {
        hs[i].score = 1000 * (i + 1);
        snprintf(hs[i].name, sizeof(hs[i].name), "player%d", i);
    }
    std::vector<HighScore> back;
    expect("high scores written", savefile::WriteRecords(fs, "scr.dat", hs, 5));
    expect("high scores read", (savefile::ReadRecords(fs, "scr.dat", back, &err) == SVR_Ok) && (back.size() == 5));
    expect("high scores equal", (back.size() == 5) && (memcmp(hs, back.data(), sizeof(hs)) == 0));

    expect("empty table written", savefile::WriteRecords(fs, "scr0.dat", hs, 0));
    expect("empty table read", (savefile::ReadRecords(fs, "scr0.dat", back, &err) == SVR_Ok) && back.empty());

    std::vector<uint8_t> flipped = fs.files["scr.dat"];
    flipped[flipped.size() / 2] ^= 0x40;
    fs.files["bad.dat"] = flipped;
    expect("damaged high scores refused", (savefile::ReadRecords(fs, "bad.dat", back, &err) == SVR_Damaged) && back.empty());

    fs.files["half.dat"] = std::vector<uint8_t>(flipped.begin(), flipped.begin() + flipped.size() / 2);
    expect("half a high score file refused", (savefile::ReadRecords(fs, "half.dat", back, &err) != SVR_Ok) && back.empty());

    ConfigInfo ci;
    expect("a high score file is not a net config", read_net(fs, "scr.dat", ci, err) == SVR_Damaged);
}

static void test_continue_files()
{
    MemoryFileSystem fs;
    SaveError err;
    static IntralevelData il, il2;
    ContinueData cd, cd2;
    memset(&il, 0, sizeof(il));
    memset(&il2, 0x55, sizeof(il2));
    il.continue_level = 7;
    il.levels_completed[2] = 9;
    expect("progress written", savefile::WriteRecord(fs, "p.cid", il));
    expect("progress read", savefile::ReadRecord(fs, "p.cid", il2, &err) == SVR_Ok);
    expect("progress equal", memcmp(&il, &il2, sizeof(il)) == 0);

    memset(&cd, 0, sizeof(cd));
    memset(&cd2, 0, sizeof(cd2));
    strcpy(cd.link_fname, "fx1g0003.sav");
    expect("link written", savefile::WriteRecord(fs, "last.sav", cd));
    expect("link read", (savefile::ReadRecord(fs, "last.sav", cd2, &err) == SVR_Ok)
        && (strcmp(cd2.link_fname, "fx1g0003.sav") == 0));
    expect("a progress file is not a link", savefile::ReadRecord(fs, "p.cid", cd2, &err) != SVR_Ok);
}

static void test_whole_files()
{
    MemoryFileSystem fs;
    SaveError err;
    std::vector<uint8_t> data;
    const char text[] = "abc";
    expect("whole file written", savefile::WriteFile(fs, "a.bin", text, 3));
    expect("whole file read", (savefile::ReadFile(fs, "a.bin", data, &err) == SVR_Ok) && (data.size() == 3) && (memcmp(data.data(), text, 3) == 0));
    expect("rewrite replaces", savefile::WriteFile(fs, "a.bin", text, 1) && (savefile::ReadFile(fs, "a.bin", data, &err) == SVR_Ok) && (data.size() == 1));
    expect("zero-length file reads as empty", savefile::WriteFile(fs, "z.bin", nullptr, 0) && (savefile::ReadFile(fs, "z.bin", data, &err) == SVR_Ok) && data.empty());
    expect("missing whole file is NotFound", savefile::ReadFile(fs, "none.bin", data, &err) == SVR_NotFound);
    fs.fail_writes = true;
    expect("whole file write failure reported", !savefile::WriteFile(fs, "b.bin", text, 3));
}

int main()
{
    test_net_config();
    test_high_scores();
    test_continue_files();
    test_whole_files();
    printf("%d checks, %d failed\n", checks, fails);
    return fails != 0;
}
