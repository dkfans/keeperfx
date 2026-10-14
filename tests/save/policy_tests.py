#!/usr/bin/env python3
"""Tests for the save format policy scripts: python3 tests/save/policy_tests.py"""
import json
import os
import sys
import tempfile
import unittest

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(here, "..", "..", "tools", "save-tool"))
import policy  # noqa: E402
import policy_comment  # noqa: E402

HEADER = "#define SAVE_MIN_READABLE_MAJOR %d\n"


def side(schema, migrations="", major=1, minor=0, min_readable=1):
    d = tempfile.mkdtemp()
    with open(d + "/schema.txt", "w") as f:
        f.write("format %d.%d\n%s\n" % (major, minor, schema.strip()))
    with open(d + "/migrations.txt", "w") as f:
        f.write(migrations)
    with open(d + "/save_codec.h", "w") as f:
        f.write(HEADER % min_readable)
    return policy.read_side(d)


BASE = """
Thing.health i32 - save,sync
Thing.name str [32] save,sync
Room.slots u8 [4] save,sync
Game.thing struct:Thing - save,sync
"""


def kinds(base, head, labels=()):
    return [(f["kind"], f["args"]) for f in policy.compare(base, head, set(labels))]


class Policy(unittest.TestCase):
    def test_nothing_changed(self):
        self.assertEqual(kinds(side(BASE), side(BASE)), [])

    def test_compatible_changes_are_silent_once_minor_is_raised(self):
        head = BASE + "Thing.mana i32 - save,sync\n"
        head = head.replace("Thing.health i32", "Thing.health i64").replace("[4]", "[8]")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [])

    def test_schema_change_needs_a_minor(self):
        head = BASE + "Thing.mana i32 - save,sync\n"
        self.assertEqual(kinds(side(BASE), side(head)), [("minor_bump", ["0", "1"])])

    def test_plain_removal_is_compatible(self):
        head = BASE.replace("Thing.health i32 - save,sync\n", "")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [])

    def test_rename_without_alias(self):
        head = BASE.replace("Thing.health", "Thing.hp")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [("missing_alias", ["Thing.health", "Thing.hp"])])

    def test_rename_with_alias(self):
        head = BASE.replace("Thing.health i32 - save,sync", "Thing.hp i32 - save,sync aliases=health")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [])

    def test_narrowed(self):
        head = BASE.replace("Thing.health i32", "Thing.health i16")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [("narrowed", ["Thing.health", "i32", "i16"])])

    def test_narrowed_with_clamp_or_migration(self):
        head = BASE.replace("Thing.health i32 - save,sync", "Thing.health i16 - save,sync,clamp")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [])
        head = BASE.replace("Thing.health i32", "Thing.health i16")
        self.assertEqual(kinds(side(BASE), side(head, migrations="Thing 0 narrow_health\n", minor=1)), [])

    def test_kind_changed(self):
        head = BASE.replace("Thing.health i32", "Thing.health str")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [("kind_changed", ["Thing.health", "i32", "str"])])

    def test_array_shrunk(self):
        head = BASE.replace("[4]", "[2]")
        self.assertEqual(kinds(side(BASE), side(head, minor=1)), [("shrunk", ["Room.slots", "[4]", "[2]"])])

    def test_break_needs_the_label(self):
        head = BASE
        found = kinds(side(BASE, "Thing 0 old\n"), side(head, ""))
        self.assertEqual(found, [("break", ["migration"])])
        self.assertEqual(kinds(side(BASE, "Thing 0 old\n"), side(head, ""), ["save-break-accepted"]), [])

    def test_major_change_is_a_break_and_needs_no_minor(self):
        found = kinds(side(BASE), side(BASE + "Thing.mana i32 - save,sync\n", major=2))
        self.assertEqual(found, [("break", ["SAVE_FORMAT_MAJOR"])])

    def test_min_readable_change_is_a_break(self):
        self.assertEqual(kinds(side(BASE), side(BASE, min_readable=2)), [("break", ["SAVE_MIN_READABLE_MAJOR"])])


SHA = "a" * 40


class Comment(unittest.TestCase):
    def result(self, findings):
        return {"pr": 5, "head_sha": SHA, "findings": findings}

    def test_valid_result(self):
        f = [{"kind": "minor_bump", "args": ["0", "1"]}]
        self.assertEqual(policy_comment.validate(self.result(f)), f)
        self.assertEqual(policy_comment.validate(self.result([])), [])

    def test_rejects_what_it_does_not_know(self):
        bad = [
            {"kind": "bogus", "args": []},
            {"kind": "minor_bump", "args": ["0"]},
            {"kind": "minor_bump", "args": ["0", "[x](http://evil)"]},
            {"kind": "break", "args": ["whatever"]},
            {"kind": "narrowed", "args": ["a", "b", "c d"]},
        ]
        for f in bad:
            self.assertIsNone(policy_comment.validate(self.result([f])), f)
        self.assertIsNone(policy_comment.validate({"pr": 5, "head_sha": "zz", "findings": []}))
        self.assertIsNone(policy_comment.validate({"pr": "5", "head_sha": SHA, "findings": []}))
        self.assertIsNone(policy_comment.validate([]))

    def test_every_kind_builds_text(self):
        findings = [
            {"kind": "minor_bump", "args": ["0", "1"]},
            {"kind": "missing_alias", "args": ["A.x", "A.y"]},
            {"kind": "narrowed", "args": ["A.x", "i32", "i16"]},
            {"kind": "kind_changed", "args": ["A.x", "i32", "str"]},
            {"kind": "shrunk", "args": ["A.x", "[4]", "[2]"]},
            {"kind": "break", "args": ["SAVE_FORMAT_MAJOR", "migration"]},
        ]
        body = policy_comment.build_body(policy_comment.validate(self.result(findings)))
        self.assertTrue(body.startswith(policy_comment.MARKER))
        self.assertEqual(body.count("\n- "), 6)

    def test_policy_output_passes_validation(self):
        d = tempfile.mkdtemp()
        head = BASE.replace("Thing.health", "Thing.hp")
        for name, s in (("base", BASE), ("head", head)):
            os.mkdir(d + "/" + name)
            with open(d + "/%s/schema.txt" % name, "w") as f:
                f.write("format 1.0\n" + s.strip() + "\n")
            open(d + "/%s/migrations.txt" % name, "w").close()
            with open(d + "/%s/save_codec.h" % name, "w") as f:
                f.write(HEADER % 1)
        sys.argv = ["policy.py", "--base", d + "/base", "--head", d + "/head", "--pr", "5", "--head-sha", SHA,
                    "--out", d + "/result.json"]
        self.assertEqual(policy.main(), 0)
        with open(d + "/result.json") as f:
            found = policy_comment.validate(json.load(f))
        self.assertEqual([f["kind"] for f in found], ["minor_bump", "missing_alias"])


if __name__ == "__main__":
    unittest.main()
