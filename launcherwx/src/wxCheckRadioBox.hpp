/******************************************************************************/
// Game Launcher for KeeperFX - free implementation of Dungeon Keeper.
/******************************************************************************/
/** @file wxCheckRadioBox.hpp
 *     Header file for wxCheckRadioBox.cpp.
 * @par Purpose:
 *     A panel with scalable image in background.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     25 May 2011 - 07 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef WXCHECKRADIOBOX_HPP
#define WXCHECKRADIOBOX_HPP
#include <wx/wx.h>
#include <wx/sizer.h>
#include <vector>

class wxCheckRadioBox : public wxStaticBox
{
public:
    std::vector<wxCheckBox *> rbCheckboxes;
    std::vector<wxTextCtrl *> rbTextCtrls;
    std::vector<wxString> rbValues;
    wxPanel *rbPanel;
    long select_limit;
public:
    wxCheckRadioBox(wxWindow *parent, wxWindowID id,
        const wxString& label,
        const wxString *values_arr, size_t values_num, size_t custom_num = 0,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = 0,
        const wxString& name = wxStaticBoxNameStr);

    void SetToolTip( const wxString &normal_tip, const wxString &custom_tip );

    void SetSelected(size_t max_selected, const wxString *sel_options, size_t sel_options_num);
    void GetSelected(wxString *sel_options, size_t &sel_options_num);

    DECLARE_EVENT_TABLE()

private:
    int OptionIndexInCheckboxes(const wxString &option);
};

#endif // WXCHECKRADIOBOX_HPP
