#!/usr/bin/env python3
"""Compare an -archontest run between host and client.

usage: compare_logs.py <host_dir> <client_dir>

Each directory must hold archontest_<user>.log and keeperfx.log from one client.
Checks that both bots finished with result=PASS and that the game checksums
logged for the same turn match (bot CKSUM lines and the engine's
update_turn_checksums lines from -mplog). Exits 0 when everything matches.
"""
import glob
import os
import re
import sys


def bot_log(path):
    sums, done = {}, None
    with open(path, errors="replace") as f:
        for line in f:
            m = re.match(r"CKSUM turn=(\d+) sum=(\w+)", line)
            if m:
                sums[int(m.group(1))] = m.group(2)
            if line.startswith("ARCHONTEST DONE"):
                done = line.strip()
    return sums, done


def engine_log(path):
    sums = {}
    with open(path, errors="replace") as f:
        for line in f:
            m = re.search(r"update_turn_checksums: turn=(\d+) checksum=(\w+)", line)
            if m:
                # after a resync a turn is computed again; the last value is the one sent
                sums[int(m.group(1))] = m.group(2)
    return sums


def compare(name, a, b):
    common = sorted(set(a) & set(b))
    bad = [t for t in common if a[t] != b[t]]
    if not common:
        print(f"{name}: no common turns")
        return False
    print(f"{name}: {len(common)} common turns {common[0]}..{common[-1]}, mismatches {len(bad)}"
          + (f" first at turn {bad[0]}: host {a[bad[0]]} client {b[bad[0]]}" if bad else ""))
    return not bad


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    host_dir, client_dir = sys.argv[1], sys.argv[2]
    ok = True
    logs = []
    for d in (host_dir, client_dir):
        found = sorted(glob.glob(os.path.join(d, "archontest_*.log")))
        if not found:
            print(f"no archontest log in {d}")
            return 1
        logs.append(bot_log(found[0]))
        print(f"{found[0]}: {logs[-1][1]}")
        ok &= logs[-1][1] is not None and "result=PASS" in logs[-1][1]
    levels = [re.search(r"level=(\d+)", d or "") for _, d in logs]
    if all(levels) and levels[0].group(1) != levels[1].group(1):
        print(f"level mismatch: host {levels[0].group(1)} client {levels[1].group(1)}")
        ok = False
    ok &= compare("bot checksums", logs[0][0], logs[1][0])
    ok &= compare("engine checksums", engine_log(os.path.join(host_dir, "keeperfx.log")),
                  engine_log(os.path.join(client_dir, "keeperfx.log")))
    print("COMPARE PASS" if ok else "COMPARE FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
