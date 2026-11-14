/******************************************************************************/
/** @file UserDirsFs.h
 *     The few file system operations the user folders need, on every OS.
 * @par Comment:
 *     The game opens files with the narrow C functions, which on Windows take
 *     paths in the ANSI code page. These helpers keep to that encoding, and turn
 *     wide Windows paths into it without losing characters (see NarrowPath).
 */
/******************************************************************************/
#ifndef KFX_USERDIRSFS_H
#define KFX_USERDIRSFS_H

#include <filesystem>
#include <string>

namespace userdirs {

/** A path in the game's narrow encoding as a std::filesystem path. */
std::filesystem::path ToPath(const std::string &path);
/** A std::filesystem path in the game's narrow encoding. Empty if a character has no narrow form. */
std::string FromPath(const std::filesystem::path &path);

/** Creates a folder and its missing parents. On Unix the folders it creates are private (0700), as the XDG
 *  specification asks. Returns false and a message when it can't. */
bool MakeDirs(const std::string &dir, std::string &error);

/** True when dir is a folder the current user can create files in. Who owns it doesn't matter: on FAT, NTFS and
 *  exFAT mounts, network shares and container volumes the owner the OS reports is often not the user who plays. */
bool IsWritableDir(const std::string &dir);

/** True when the file is the current user's to take along: the user owns it, or it is on a file system that keeps
 *  no owners (FAT, exFAT, NTFS and network shares on Unix, FAT and exFAT on Windows) and the user can read it. */
bool FileIsUsers(const std::string &path);

/** True when path, with every link in it followed, is dir or in dir (dir's links followed too). path need not
 *  exist: the part of it that exists is resolved, the rest compared by name. */
bool IsWithinDir(const std::string &dir, const std::string &path);

/** True when a and b name the same existing folder. */
bool SameDir(const std::string &a, const std::string &b);

/** The absolute form of a path, for the log; the path itself when that fails. */
std::string AbsolutePath(const std::string &path);

#ifdef _WIN32
/** A wide path in the ANSI code page the game's file functions use. When the path has characters that code page
 *  lacks (a Cyrillic user name on a Western Windows, say), it is the short 8.3 form of the path, which is ASCII.
 *  The path must exist for that. Empty when neither works. */
std::string NarrowPath(const std::wstring &path);
std::wstring WidePath(const std::string &path);
#endif

} // namespace userdirs

#endif // KFX_USERDIRSFS_H
