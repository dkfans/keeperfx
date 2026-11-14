/******************************************************************************/
/** @file UserDirsScript.h
 *     The rule for the files a map's Lua script may open or remove.
 * @par Purpose:
 *     Maps come from the internet, so their scripts get to the files of the
 *     game folder and of the user's data folder only, and may write and remove
 *     only in the folders that hold maps: levels/, campgns/, multiplayer/ and
 *     mods/. Paths are taken apart the same way on every OS, so a map behaves
 *     the same everywhere: a name Windows would read as something else (a
 *     drive, a device, a stream) is refused on Linux too.
 * @par Comment:
 *     These functions only look at the strings. That links a path to a file
 *     outside the folders is checked on the disk by IsWithinDir().
 */
/******************************************************************************/
#ifndef KFX_USERDIRSSCRIPT_H
#define KFX_USERDIRSSCRIPT_H

#include <string>
#include <vector>

namespace userdirs {

struct ScriptPath {
    bool ok = false;
    /** The path relative to the game and data folders, with '/' between its parts, e.g. "levels/x/seed.txt". */
    std::string logical;
    /** Why the path was refused, for the script and the log. */
    std::string error;
};

/**
 * The logical path of a file a map script names. A relative path is taken from the game and data folders; "." and
 * ".." parts are resolved, and one that leads out of them is refused. An absolute path is accepted only when it is
 * in one of roots (the absolute game and data folders), and is then taken as the path relative to that folder.
 * '\\' separates parts as '/' does. Refused: UNC and device paths, "C:x", ':' anywhere else (alternate data streams),
 * the names of Windows devices (CON, NUL, COM1, LPT1, ... with any extension), parts ending in '.' or ' ', the
 * characters Windows doesn't allow in names, and control characters. A root is compared without regard to case
 * when case_insensitive is set (Windows, macOS).
 */
ScriptPath ScriptLogicalPath(const std::string &path, const std::vector<std::string> &roots, bool case_insensitive);

/** True when a script may write or remove the file of a logical path: in levels/, campgns/, multiplayer/ or mods/
 *  (in any case), not the folder itself. */
bool ScriptMayWrite(const std::string &logical);

/** The folders ScriptMayWrite() allows, for messages: "levels/, campgns/, multiplayer/ and mods/". */
extern const char *const script_write_folders_text;

/**
 * path relative to the root it is in, the longest of roots when they nest, with '/' separators; compared without
 * regard to case when case_insensitive is set. Empty when path is in none of them. Only the strings are compared.
 */
std::string RelativeToRoots(const std::string &path, const std::vector<std::string> &roots, bool case_insensitive);

} // namespace userdirs

#endif // KFX_USERDIRSSCRIPT_H
