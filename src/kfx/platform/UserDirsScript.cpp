/******************************************************************************/
/** @file UserDirsScript.cpp
 *     The rule for the files a map's Lua script may open or remove.
 */
/******************************************************************************/
#include "kfx/platform/UserDirsScript.h"

#include <cctype>
#include <cstring>

namespace userdirs {

const char *const script_write_folders_text = "levels/, campgns/, multiplayer/ and mods/";

static const char *const script_write_folders[] = {"levels", "campgns", "multiplayer", "mods"};

namespace {

/** A path taken apart: its root ("", "/" or "C:/") and its parts, with "." and ".." resolved. */
struct SplitPath {
    std::string prefix;
    std::vector<std::string> parts;
    /** A ".." went above the start of a relative path. */
    bool escapes = false;
};

std::string Lower(const std::string &s)
{
    std::string out = s;
    for (char &c : out)
        c = (char)tolower((unsigned char)c);
    return out;
}

bool SamePart(const std::string &a, const std::string &b, bool case_insensitive)
{
    return case_insensitive ? (Lower(a) == Lower(b)) : (a == b);
}

/** prefix is "/" for "/x", "C:/" for "C:\x" or "C:/x", "" for a relative path. Fails for "//x" (UNC, devices) and
 *  "C:x" (relative to the drive's current folder). */
bool Split(const std::string &path, SplitPath &out, std::string &error)
{
    std::string p = path;
    for (char &c : p) {
        if (c == '\\')
            c = '/';
    }
    size_t pos = 0;
    if ((p.size() >= 2) && (p[0] == '/') && (p[1] == '/')) {
        error = "network and device paths are not allowed";
        return false;
    }
    if (!p.empty() && (p[0] == '/')) {
        out.prefix = "/";
        pos = 1;
    } else if ((p.size() >= 2) && isalpha((unsigned char)p[0]) && (p[1] == ':')) {
        if ((p.size() < 3) || (p[2] != '/')) {
            error = "a drive without a folder (\"C:x\") is not allowed";
            return false;
        }
        out.prefix = std::string(1, (char)toupper((unsigned char)p[0])) + ":/";
        pos = 3;
    }
    while (pos <= p.size()) {
        size_t end = p.find('/', pos);
        if (end == std::string::npos)
            end = p.size();
        const std::string part = p.substr(pos, end - pos);
        pos = end + 1;
        if (part.empty() || (part == "."))
            continue;
        if (part == "..") {
            if (!out.parts.empty())
                out.parts.pop_back();
            else if (out.prefix.empty())
                out.escapes = true;
            continue;
        }
        out.parts.push_back(part);
    }
    return true;
}

bool IsDeviceName(const std::string &part)
{
    // The name before the first dot, without the spaces Windows ignores: "nul.txt" and "CON " are devices too.
    std::string base = Lower(part.substr(0, part.find('.')));
    while (!base.empty() && (base.back() == ' '))
        base.pop_back();
    static const char *const devices[] = {"con", "prn", "aux", "nul", "conin$", "conout$", "clock$"};
    for (const char *d : devices) {
        if (base == d)
            return true;
    }
    if ((base.compare(0, 3, "com") == 0) || (base.compare(0, 3, "lpt") == 0)) {
        const std::string rest = base.substr(3);
        if ((rest.size() == 1) && isdigit((unsigned char)rest[0]))
            return true;
        // COM¹, COM², COM³ in UTF-8 and in the Western code page.
        if ((rest == "\xc2\xb9") || (rest == "\xc2\xb2") || (rest == "\xc2\xb3") || (rest == "\xb9") || (rest == "\xb2")
            || (rest == "\xb3"))
            return true;
    }
    return false;
}

/** Why Windows would not take part as a plain file or folder name; empty when it would. */
std::string BadPartReason(const std::string &part)
{
    for (unsigned char c : part) {
        if (c < 0x20)
            return "control characters are not allowed in names";
        if (strchr("<>:\"|?*", c) != NULL)
            return std::string("'") + (char)c + "' is not allowed in names";
    }
    const char last = part.back();
    if ((last == '.') || (last == ' '))
        return "a name may not end with '.' or ' '";
    if (IsDeviceName(part))
        return "\"" + part + "\" is the name of a device";
    return std::string();
}

/** The number of parts of root that path starts with, or -1 when path is not in root. */
int MatchRoot(const SplitPath &path, const SplitPath &root, bool case_insensitive)
{
    if (!SamePart(path.prefix, root.prefix, true) || (root.parts.size() > path.parts.size()))
        return -1;
    for (size_t i = 0; i < root.parts.size(); i++) {
        if (!SamePart(path.parts[i], root.parts[i], case_insensitive))
            return -1;
    }
    return (int)root.parts.size();
}

/** The parts after the longest root path is in; false when it is in none. */
bool StripRoot(const SplitPath &path, const std::vector<std::string> &roots, bool case_insensitive, std::vector<std::string> &rest)
{
    int best = -1;
    for (const std::string &r : roots) {
        SplitPath root;
        std::string ignored;
        if (r.empty() || !Split(r, root, ignored) || root.prefix.empty())
            continue;
        const int n = MatchRoot(path, root, case_insensitive);
        if (n > best)
            best = n;
    }
    if (best < 0)
        return false;
    rest.assign(path.parts.begin() + best, path.parts.end());
    return true;
}

std::string JoinParts(const std::vector<std::string> &parts)
{
    std::string out;
    for (const std::string &p : parts) {
        if (!out.empty())
            out += '/';
        out += p;
    }
    return out;
}

} // namespace

ScriptPath ScriptLogicalPath(const std::string &path, const std::vector<std::string> &roots, bool case_insensitive)
{
    ScriptPath res;
    if (path.empty()) {
        res.error = "no file name";
        return res;
    }
    SplitPath split;
    if (!Split(path, split, res.error))
        return res;
    std::vector<std::string> parts;
    if (split.prefix.empty()) {
        if (split.escapes) {
            res.error = "\"..\" may not lead out of the game and data folders";
            return res;
        }
        parts = split.parts;
    } else if (!StripRoot(split, roots, case_insensitive, parts)) {
        res.error = "only files in the game and data folders are allowed";
        return res;
    }
    if (parts.empty()) {
        res.error = "names a folder, not a file";
        return res;
    }
    for (const std::string &part : parts) {
        const std::string why = BadPartReason(part);
        if (!why.empty()) {
            res.error = why;
            return res;
        }
    }
    res.logical = JoinParts(parts);
    res.ok = true;
    return res;
}

bool ScriptMayWrite(const std::string &logical)
{
    const size_t slash = logical.find('/');
    if ((slash == std::string::npos) || (slash + 1 >= logical.size()))
        return false;
    const std::string first = Lower(logical.substr(0, slash));
    for (const char *f : script_write_folders) {
        if (first == f)
            return true;
    }
    return false;
}

std::string RelativeToRoots(const std::string &path, const std::vector<std::string> &roots, bool case_insensitive)
{
    SplitPath split;
    std::string ignored;
    std::vector<std::string> rest;
    if (!Split(path, split, ignored) || split.prefix.empty() || !StripRoot(split, roots, case_insensitive, rest))
        return std::string();
    return JoinParts(rest);
}

} // namespace userdirs
