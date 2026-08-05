/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file filepaths.h
 *     Header file for filepaths.cpp.
 * @par Purpose:
 *     Resolving file paths for various file groups.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 */
/******************************************************************************/

#ifndef DK_FILEPATHS_H
#define DK_FILEPATHS_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * Scan all static game directories and populate the internal file map.
 * Call once after keeper_runtime_directory and install_info are set.
 */
void build_file_map(void);

/**
 * Scan campaign-specific directories and add them to the file map with
 * campaign priority (overrides base game files with the same name).
 * Call after a campaign is loaded.
 */
void build_campaign_file_map(void);

/**
 * Look up @p fname in the file map for @p fgroup.
 * Falls back to prepare_file_path_buf() if the file is not in the map.
 * Returns a pointer to an internal static buffer.
 */
char *prepare_file_path_prio(short fgroup, const char *fname);

/**
 * printf-style variant of prepare_file_path_m().
 * @p flags is a bitmask of AltDir values; AltDir_Campaign causes the
 * campaign config directory to be checked first.
 * Returns a pointer to an internal static buffer.
 */
char *prepare_file_fmtpath_prio(short fgroup, int flags, const char *fmt_str, ...);

#ifdef __cplusplus
}
#endif
#endif /* DK_FILEPATHS_H */
