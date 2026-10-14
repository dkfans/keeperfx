/******************************************************************************/
/** @file SaveCatalogue.cpp
 *     The savegame catalogue: one entry per slot the load and save menus range over.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "globals.h"
#include "bflib_basics.h"
#include "config.h"
#include "config_campaigns.h"
#include "front_highscore.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "kfx/platform/IPlatform.h"
#include "post_inc.h"

#include <algorithm>
#include <cstring>

#include "kfx/save/SaveManager_Internal.h"

bool SaveManager::EnsureSlot(long slot)
{
    if ((slot < 0) || (slot >= SAVE_SLOTS_LIMIT))
        return false;
    if (slot >= static_cast<long>(catalogue_.size()))
        catalogue_.resize(slot + 1);    // new entries are zeroed: not in use
    if (slot >= slot_count_)
        slot_count_ = slot + 1;
    return true;
}

CatalogueEntry *SaveManager::Entry(int slot)
{
    if ((slot < 0) || (slot >= slot_count_))
        return nullptr;
    return &catalogue_[slot];
}

int SaveManager::CountSavedGames()
{
    int count = 0;
    for (long index = 0; index < slot_count_; index++)
    {
        if ((catalogue_[index].flags & CEF_InUse) != 0)
            count++;
    }
    return count;
}

bool SaveManager::FillEntry(CatalogueEntry *centry, const char *textname)
{
    centry->level_num = get_loaded_level_number();
    snprintf(centry->textname, SAVE_TEXTNAME_LEN, "%s", textname);
    snprintf(centry->campaign_name, LINEMSG_SIZE, "%s", campaign.name);
    const char *cmpgn_pfx = "";
    for (int index = 0; index < CampgnT_COUNT; index++) {
        if ((cmpgn_fgroup[index] == campaign.fgroup) && (cmpgn_prefix[index] != nullptr)) {
            cmpgn_pfx = cmpgn_prefix[index];
            break;
        }
    }
    snprintf(centry->campaign_fname, DISKPATH_SIZE, "%s%s", cmpgn_pfx, campaign.fname);
    snprintf(centry->player_name, PLAYER_NAME_LENGTH, "%s", high_score_entry);
    set_flag(centry->flags, CEF_InUse);
    clear_flag(centry->flags, CEF_Unloadable);
    centry->game_ver_major = VER_MAJOR;
    centry->game_ver_minor = VER_MINOR;
    centry->game_ver_release = VER_RELEASE;
    centry->game_ver_build = VER_BUILD;
    return true;
}

bool SaveManager::FillSlot(int slot, const char *textname)
{
    if (!EnsureSlot(slot))
    {
        SAVEERR("can't name slot %d: slot index is out of range", slot);
        return false;
    }
    return FillEntry(&catalogue_[slot], textname);
}

bool SaveManager::DisableSlot(unsigned int slot)
{
    if (slot >= static_cast<unsigned long>(slot_count_))
        return false;
    clear_flag(catalogue_[slot].flags, CEF_InUse);
    return true;
}

bool SaveManager::LoadCatalogue()
{
    IFileSystem &filesystem = *GetPlatform()->GetFileSystem();
    // Scan the save directory to find the highest existing slot index, so the catalogue
    // can be sized to exactly the saves that exist plus one free slot to save into.
    long highest = -1;
    for (const std::string &name : filesystem.Find(prepare_file_path(FGrp_Save, "fx1g*.sav")))
    {
        const int idx = savegame::SlotFromFilename(name.c_str());
        highest = std::max<long>(idx, highest);
    }
    long needed = highest + 2;               // used slots + one free slot to save into
    if (needed < SAVE_SLOTS_MIN)
        needed = SAVE_SLOTS_MIN;
    if (needed > SAVE_SLOTS_LIMIT)
        needed = SAVE_SLOTS_LIMIT;
    if (!EnsureSlot(needed - 1))
        return false;
    slot_count_ = needed;                    // logical length the menus range over

    // (Re)load the title and loadability of every slot in range; missing files leave a zeroed (free) entry.
    long saves_found = 0;
    for (long slot_num = 0; slot_num < slot_count_; slot_num++)
    {
        CatalogueEntry *centry = &catalogue_[slot_num];
        memset(centry, 0, sizeof(CatalogueEntry));
        int title_ok;
        const enum SaveCheckResult check_result = savegame::CheckFile(filesystem, prepare_file_fmtpath(FGrp_Save, SAVED_GAME_FILENAME, static_cast<int>(slot_num)),
            centry, &title_ok, nullptr, 0);
        if (check_result == SvChk_Missing)
            continue;
        if (!title_ok)
        {
            // Too damaged to read a title: list it by its file name so it's visible, and refused when picked
            memset(centry, 0, sizeof(CatalogueEntry));
            snprintf(centry->textname, SAVE_TEXTNAME_LEN, SAVED_GAME_FILENAME, static_cast<int>(slot_num));
        }
        set_flag(centry->flags, CEF_InUse);
        clear_flag(centry->flags, CEF_Unloadable);
        if (check_result != SvChk_Loadable)
            set_flag(centry->flags, CEF_Unloadable);
        saves_found++;
    }
    return (saves_found > 0);
}

bool SaveManager::InitialiseSlots()
{
    LoadCatalogue();
    saved_games_ = CountSavedGames();
    return (saved_games_ > 0);
}
