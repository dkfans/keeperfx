/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file GameSettings.hpp
 *     Header file for GameSettings.cpp.
 * @par Purpose:
 *     A frame which allows changing game settings file.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Jun 2011 - 12 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef GAMESETTINGS_HPP_
#define GAMESETTINGS_HPP_
#include <wx/wx.h>
#include <wx/fileconf.h>

#include <string>
#include <vector>

class wxCheckRadioBox;

class GameSettings : public wxDialog
{
    enum {
        eventID_ScrnCtrlChange = wxID_HIGHEST+20,
        eventID_Save,
        eventID_Cancel,
    };

    wxBoxSizer * topsizer;
    wxRadioBox * langRadio;
    wxRadioBox * scrshotRadio;
    wxCheckBox * scrnControlChkBx;
    wxTextCtrl * mouseSensitvTxtCtrl;
    wxCheckBox * censorChkBx;

    // Resolution box
    wxStaticBoxSizer * resolutionBoxSizer;
    wxComboBox * resPrimaryCombo = NULL;
    wxCheckBox * resSecondaryChkBx;
    wxComboBox * resSecondaryCombo = NULL;
    wxCheckBox * resTertiaryChkBx;
    wxComboBox * resTertiaryCombo = NULL;

    // String parameters
    wxString installPath;
    wxString installType;
    wxString keybLayout;

    wxFileConfig * conf;
public:
    GameSettings(wxFrame *parent);
    virtual ~GameSettings();

    // event handlers
    void OnClose(wxCloseEvent& event);
    void OnShow(wxShowEvent& event);
    void OnScreenCtrlChange(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void ChangeResolutionOptions(int scr_ctrl);

    void readConfiguration();
    void writeConfiguration();
    int verifyConfiguration();
private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()

    void _create_resolution_box();
};


/******************************************************************************/
#endif /* GAMESETTINGS_HPP_ */
