/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file CommandOptions.hpp
 *     Header file for CommandOptions.cpp.
 * @par Purpose:
 *     A frame which allows changing command line options.
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
#ifndef COMMANDOPTIONS_HPP_
#define COMMANDOPTIONS_HPP_
#include <wx/wx.h>

#include <string>
#include <vector>

class wxCheckRadioBox;

class CommandOptions : public wxDialog
{
    enum {
        eventID_ChangeOption = wxID_HIGHEST+10,
        eventID_CmdUpdate,
    };

    wxBoxSizer * topsizer;

    wxTextCtrl *cmdLineCtrl;
    wxRadioBox * execKindRadio;
    wxCheckRadioBox * cmdFlagsBox;
    wxCheckBox * gameSpeedChkBx;
    wxTextCtrl * gameSpeedTxtCtrl;
    wxCheckBox * humanPlayerChkBx;
    wxTextCtrl * humanPlayerTxtCtrl;
    wxTextCtrl * vidDriverTxtCtrl;
    wxCheckBox * netSessionsChkBx;
    wxTextCtrl * netSessionsTxtCtrl;
    wxRadioButton * packetActionNoRadio;
    wxRadioButton * packetActionLdRadio;
    wxRadioButton * packetActionSvRadio;
    wxTextCtrl * packetFileTxtCtrl;


public:
    CommandOptions(wxFrame *parent);
    virtual ~CommandOptions();

    // event handlers
    void OnClose(wxCloseEvent& event);
    void OnShow(wxShowEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnCmdRefresh(wxCommandEvent& event);
    void OnCmdKeyRefresh(wxKeyEvent& event);
    void OnCmdUpdate(wxCommandEvent& event);

    void readOptions(void);
    void storeOptions(void);
    std::wstring getCommandLine(void);
private:
    std::wstring optionsToCommandLine(void);
    void commandLineToOptions(const std::wstring & cmdLn);
    void readOptionsFile(void);
    void storeOptionsFile(void);
    std::wstring storedCommandLine;
    std::wstring unrecognizedOptions;
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
private:
};


/******************************************************************************/
#endif /* COMMANDOPTIONS_HPP_ */
