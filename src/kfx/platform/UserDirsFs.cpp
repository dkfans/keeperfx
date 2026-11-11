/******************************************************************************/
/** @file UserDirsFs.cpp
 *     The few file system operations the user folders need, on every OS.
 */
/******************************************************************************/
#include "kfx/platform/UserDirsFs.h"

#include <cstdio>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#if defined(__linux__)
#include <sys/vfs.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/param.h>
#include <sys/mount.h>
#endif
#endif

namespace fs = std::filesystem;

namespace userdirs {

#ifdef _WIN32

/** The ANSI form of a wide string; lossless tells whether every character made it. */
static std::string WideToAnsi(const std::wstring &wide, bool &lossless)
{
    lossless = true;
    if (wide.empty())
        return std::string();
    const UINT cp = GetACP();
    // WC_NO_BEST_FIT_CHARS and the default character test are not allowed with UTF-8, which loses nothing anyway.
    const DWORD flags = (cp == CP_UTF8) ? 0 : WC_NO_BEST_FIT_CHARS;
    BOOL used_default = FALSE;
    BOOL *used_default_ptr = (cp == CP_UTF8) ? NULL : &used_default;
    int len = WideCharToMultiByte(cp, flags, wide.c_str(), (int)wide.size(), NULL, 0, NULL, used_default_ptr);
    if (len <= 0) {
        lossless = false;
        return std::string();
    }
    std::string out((size_t)len, '\0');
    WideCharToMultiByte(cp, flags, wide.c_str(), (int)wide.size(), &out[0], len, NULL, used_default_ptr);
    lossless = !used_default;
    return out;
}

std::wstring WidePath(const std::string &path)
{
    if (path.empty())
        return std::wstring();
    int len = MultiByteToWideChar(CP_ACP, 0, path.c_str(), (int)path.size(), NULL, 0);
    if (len <= 0)
        return std::wstring();
    std::wstring out((size_t)len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), (int)path.size(), &out[0], len);
    return out;
}

std::string NarrowPath(const std::wstring &path)
{
    bool lossless;
    std::string narrow = WideToAnsi(path, lossless);
    if (lossless)
        return narrow;
    DWORD len = GetShortPathNameW(path.c_str(), NULL, 0);
    if (len == 0)
        return std::string();
    std::wstring short_path((size_t)len, L'\0');
    len = GetShortPathNameW(path.c_str(), &short_path[0], len);
    if (len == 0)
        return std::string();
    short_path.resize(len);
    narrow = WideToAnsi(short_path, lossless);
    return lossless ? narrow : std::string();
}

fs::path ToPath(const std::string &path)
{
    return fs::path(WidePath(path));
}

std::string FromPath(const fs::path &path)
{
    bool lossless;
    std::string narrow = WideToAnsi(path.wstring(), lossless);
    return lossless ? narrow : std::string();
}

#else

fs::path ToPath(const std::string &path)
{
    return fs::path(path);
}

std::string FromPath(const fs::path &path)
{
    return path.string();
}

#endif

bool MakeDirs(const std::string &dir, std::string &error)
{
    std::error_code ec;
    const fs::path path = ToPath(dir);
    if (path.empty()) {
        error = "\"" + dir + "\" is not a valid path";
        return false;
    }
    if (fs::is_directory(path, ec))
        return true;
    // Collect the missing folders, outermost last, so each one gets created and, on Unix, made private.
    std::vector<fs::path> missing;
    fs::path cur = path;
    while (!cur.empty()) {
        fs::file_status st = fs::status(cur, ec);
        if (fs::exists(st)) {
            if (!fs::is_directory(st)) {
                error = "\"" + FromPath(cur) + "\" exists and is not a folder";
                return false;
            }
            break;
        }
        missing.push_back(cur);
        fs::path parent = cur.parent_path();
        if (parent == cur)
            break;
        cur = parent;
    }
    for (auto it = missing.rbegin(); it != missing.rend(); ++it) {
        ec.clear();
        if (!fs::create_directory(*it, ec) && ec) {
            error = "can't create \"" + FromPath(*it) + "\": " + ec.message();
            return false;
        }
#ifndef _WIN32
        fs::permissions(*it, fs::perms::owner_all, fs::perm_options::replace, ec);
#endif
    }
    return true;
}

bool IsWritableDir(const std::string &dir)
{
#ifdef _WIN32
    char name[64];
    snprintf(name, sizeof(name), ".keeperfx-write-test-%lu", (unsigned long)GetCurrentProcessId());
    const std::wstring probe = WidePath(dir.empty() ? std::string(".") : dir) + L"\\" + WidePath(name);
    // Deleted when closed. Asking Windows to create the file is the only test that sees ACLs, read only media and
    // folders other programs protect; GetFileAttributes' read only flag means nothing for folders.
    HANDLE h = CreateFileW(probe.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW,
                           FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    CloseHandle(h);
    return true;
#else
    const std::string d = dir.empty() ? std::string(".") : dir;
    struct stat st;
    if ((stat(d.c_str(), &st) != 0) || !S_ISDIR(st.st_mode))
        return false;
    // Creating a file is the test: access() doesn't see read only mounts on every system, nor ACLs.
    char name[64];
    snprintf(name, sizeof(name), "/.keeperfx-write-test-%ld", (long)getpid());
    const std::string probe = d + name;
    int fd = open(probe.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0)
        return false;
    close(fd);
    unlink(probe.c_str());
    return true;
#endif
}

#ifdef _WIN32

/** True when the volume of path keeps owners and permissions (NTFS, ReFS), false for FAT and exFAT. */
static bool VolumeKeepsOwners(const std::wstring &path)
{
    wchar_t volume[MAX_PATH + 1];
    if (!GetVolumePathNameW(path.c_str(), volume, MAX_PATH + 1))
        return true;
    DWORD flags = 0;
    if (!GetVolumeInformationW(volume, NULL, 0, NULL, NULL, &flags, NULL, 0))
        return true;
    return (flags & FILE_PERSISTENT_ACLS) != 0;
}

bool FileIsUsers(const std::string &path)
{
    const std::wstring wide = WidePath(path);
    if (!VolumeKeepsOwners(wide)) {
        HANDLE h = CreateFileW(wide.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (h == INVALID_HANDLE_VALUE)
            return false;
        CloseHandle(h);
        return true;
    }
    DWORD needed = 0;
    GetFileSecurityW(wide.c_str(), OWNER_SECURITY_INFORMATION, NULL, 0, &needed);
    if (needed == 0)
        return false;
    std::vector<unsigned char> sd(needed);
    if (!GetFileSecurityW(wide.c_str(), OWNER_SECURITY_INFORMATION, (PSECURITY_DESCRIPTOR)sd.data(), needed, &needed))
        return false;
    PSID owner = NULL;
    BOOL defaulted = FALSE;
    if (!GetSecurityDescriptorOwner((PSECURITY_DESCRIPTOR)sd.data(), &owner, &defaulted) || (owner == NULL))
        return false;
    HANDLE token = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        return false;
    DWORD len = 0;
    GetTokenInformation(token, TokenUser, NULL, 0, &len);
    std::vector<unsigned char> user(len ? len : 1);
    bool same = false;
    if ((len > 0) && GetTokenInformation(token, TokenUser, user.data(), len, &len))
        same = EqualSid(owner, ((TOKEN_USER *)user.data())->User.Sid) != FALSE;
    CloseHandle(token);
    return same;
}

#else

/** True when the file system of path keeps no owners of its own, so the owner stat() gives is the one the whole
 *  mount was given (FAT, exFAT, NTFS, FUSE mounts such as ntfs-3g, SMB shares, WSL's Windows drives). */
static bool FileSystemHasNoOwners(const std::string &path)
{
#if defined(__linux__)
    struct statfs sf;
    if (statfs(path.c_str(), &sf) != 0)
        return false;
    switch ((unsigned long)sf.f_type) {
    case 0x4d44UL:     // FAT (msdos, vfat)
    case 0x2011bab0UL: // exFAT
    case 0x5346544eUL: // NTFS (ntfs)
    case 0x7366746eUL: // NTFS (ntfs3)
    case 0x65735546UL: // FUSE (ntfs-3g, exfat-fuse and other user space file systems)
    case 0xff534d42UL: // CIFS
    case 0xfe534d42UL: // SMB2
    case 0x517bUL:     // SMB
    case 0x01021997UL: // 9P (the Windows drives in WSL 2)
        return true;
    default:
        return false;
    }
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    struct statfs sf;
    if (statfs(path.c_str(), &sf) != 0)
        return false;
    static const char *const kinds[] = {"msdos", "msdosfs", "exfat", "ntfs", "smbfs", "cifs", "fusefs"};
    for (const char *k : kinds) {
        if (strncmp(sf.f_fstypename, k, strlen(k)) == 0)
            return true;
    }
    return false;
#else
    (void)path;
    return false;
#endif
}

bool FileIsUsers(const std::string &path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    if (st.st_uid == geteuid())
        return true;
    return FileSystemHasNoOwners(path) && (access(path.c_str(), R_OK) == 0);
}

#endif

bool SameDir(const std::string &a, const std::string &b)
{
    if (a == b)
        return true;
    std::error_code ec;
    const fs::path pa = ToPath(a);
    const fs::path pb = ToPath(b);
    if (fs::equivalent(pa, pb, ec))
        return true;
    if (!ec)
        return false;
    // One of them doesn't exist yet: compare the paths.
    ec.clear();
    fs::path ca = fs::weakly_canonical(pa, ec);
    if (ec)
        return false;
    fs::path cb = fs::weakly_canonical(pb, ec);
    if (ec)
        return false;
    return ca.lexically_normal() == cb.lexically_normal();
}

std::string AbsolutePath(const std::string &path)
{
    std::error_code ec;
    fs::path abs = fs::absolute(ToPath(path), ec);
    if (ec)
        return path;
    std::string out = FromPath(abs.lexically_normal());
    if (out.empty())
        return path;
    while ((out.size() > 1) && ((out.back() == '/') || (out.back() == '\\')) && (out[out.size() - 2] != ':'))
        out.pop_back();
    return out;
}

} // namespace userdirs
