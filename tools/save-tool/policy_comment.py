#!/usr/bin/env python3
"""Posts, updates or removes the save format comment on a pull request.

    policy_comment.py result.json

Environment: GH_TOKEN, GITHUB_REPOSITORY, RUN_HEAD_SHA (head commit of the workflow run that made
result.json). The result comes from a job that ran the PR's code, so nothing in it is trusted: the
kinds must be known, every value must look like an identifier, and the PR's head must still be the
commit the result was made for. The comment text is built here, not taken from the result.
With findings, one comment is kept up to date; with none, it is removed. Nothing is posted otherwise.
"""
import json
import os
import re
import subprocess
import sys

MARKER = "<!-- save-format-policy -->"
ARG_OK = re.compile(r"^[A-Za-z0-9_.:\[\],=<>-]{1,120}$")
SHA_OK = re.compile(r"^[0-9a-f]{40}$")

MESSAGES = {
    "minor_bump": lambda a: "The save schema changed. Raise `SAVE_FORMAT_MINOR` in `src/kfx/save/core/save_codec.h` from %s to %s." % (a[0], a[1]),
    "missing_alias": lambda a: "`%s` was removed and `%s` added with the same type. If that is a rename, add `.aliases` with the old name to the new field; without it older saves lose the value." % (a[0], a[1]),
    "narrowed": lambda a: "`%s` went from %s to %s. A save holding a value that doesn't fit is refused. Add a migration for that struct, or mark the field `SVF_CLAMP`." % (a[0], a[1], a[2]),
    "kind_changed": lambda a: "`%s` changed from %s to %s, which older saves can't be read into. Add a migration for that struct." % (a[0], a[1], a[2]),
    "shrunk": lambda a: "`%s` shrank from %s to %s. A save that uses entries past the new end is refused. Add a migration if that matters." % (a[0], a[1], a[2]),
    "break": lambda a: "This breaks existing saves (%s). If that is intended, add the `save-break-accepted` label." % ", ".join(BREAK_WHAT[x] for x in a),
}
ARG_COUNT = {"minor_bump": 2, "missing_alias": 2, "narrowed": 3, "kind_changed": 3, "shrunk": 3}
BREAK_WHAT = {
    "SAVE_FORMAT_MAJOR": "`SAVE_FORMAT_MAJOR` changed",
    "SAVE_MIN_READABLE_MAJOR": "`SAVE_MIN_READABLE_MAJOR` changed",
    "migration": "an existing migration changed or was removed",
}


def validate(result):
    """The findings if result is well formed, else None."""
    if not isinstance(result, dict) or not isinstance(result.get("pr"), int) or result["pr"] <= 0:
        return None
    if not isinstance(result.get("head_sha"), str) or not SHA_OK.match(result["head_sha"]):
        return None
    findings = result.get("findings")
    if not isinstance(findings, list) or len(findings) > 200:
        return None
    for f in findings:
        if not isinstance(f, dict) or f.get("kind") not in MESSAGES:
            return None
        args = f.get("args")
        if not isinstance(args, list) or not all(isinstance(x, str) and ARG_OK.match(x) for x in args):
            return None
        if f["kind"] == "break":
            if not args or any(x not in BREAK_WHAT for x in args):
                return None
        elif len(args) != ARG_COUNT[f["kind"]]:
            return None
    return findings


def build_body(findings):
    lines = [MARKER, "**Save format**", ""]
    lines += ["- " + MESSAGES[f["kind"]](f["args"]) for f in findings]
    lines += ["", "This comment updates itself and goes away once nothing is left to do."]
    return "\n".join(lines)


def gh(*args, data=None):
    cmd = ["gh", "api"] + list(args)
    out = subprocess.run(cmd, input=data, capture_output=True, text=True, check=True).stdout
    return json.loads(out) if out.strip() else None


def main():
    with open(sys.argv[1], encoding="utf-8") as f:
        result = json.load(f)
    findings = validate(result)
    if findings is None:
        print("result.json is not valid; leaving the PR alone")
        return 0
    repo = os.environ["GITHUB_REPOSITORY"]
    pr = result["pr"]
    if result["head_sha"] != os.environ["RUN_HEAD_SHA"]:
        print("result is for another commit; leaving the PR alone")
        return 0
    if gh("repos/%s/pulls/%d" % (repo, pr))["head"]["sha"] != result["head_sha"]:
        print("the PR has moved on; a newer run will report")
        return 0

    existing = None
    for c in gh("--paginate", "--slurp", "repos/%s/issues/%d/comments" % (repo, pr)) or []:
        for item in (c if isinstance(c, list) else [c]):
            if item["body"].startswith(MARKER) and item["user"]["type"] == "Bot":
                existing = item["id"]
    if findings:
        body = json.dumps({"body": build_body(findings)})
        if existing:
            gh("-X", "PATCH", "repos/%s/issues/comments/%d" % (repo, existing), "--input", "-", data=body)
        else:
            gh("-X", "POST", "repos/%s/issues/%d/comments" % (repo, pr), "--input", "-", data=body)
    elif existing:
        gh("-X", "DELETE", "repos/%s/issues/comments/%d" % (repo, existing))
    return 0


if __name__ == "__main__":
    sys.exit(main())
