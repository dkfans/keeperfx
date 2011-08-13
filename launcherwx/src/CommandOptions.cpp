/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file CommandOptions.cpp
 *     A frame which allows changing command line options.
 * @par Purpose:
 *     Generates command line string for running the game.
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
#include "CommandOptions.hpp"

#include <cstddef>
#include <cstring>
#include <sys/stat.h>
#include <stdexcept>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/valnum.h>
#include "wxCheckRadioBox.hpp"

wxString options_exectype_code[] = {
    _T("keeperfx.exe"),
    _T("keeperfx_hvlog.exe"),
};

wxString options_exectype_text[] = {
    _T("Standard"),
    _T("Heavylog"),
};

wxString options_flags_code[] = {
    _T("-nointro"),
    _T("-nocd"),
    _T("-nosound"),
    _T("-usersfont"),
    _T("-alex"),
    _T("-columnconvert"),
    _T("-lightconvert"),
    _T("-vidsmooth"),
    _T("-altinput"),
};

wxString options_flags_text[] = {
    _T("Skip intro"),
    _T("No CD music"),
    _T("Disable sound"),
    _T("No AWE32/64 Banks"),
    _T("Easter Egg"),
    _T("Column convert"),
    _T("Light convert"),
    _T("Smoothen video"),
    _T("Alt. input support"),
};

wxString options_tooltips_eng[] = {
    _T(""),
    _T("Command line to run. Here you can type by hand the parameters you wish to use."),//1
    _T("You usually want standard version, as it is fast and stable. Heavylog version logs huge amount of messages to a file \"keeperfx.log\" while you're playing. This requires a lot more of computation power, so on slower machines it might severely affect gameplay speed. But if the game will crash, the LOG file may help the developers to fix the problem."),//2
    _T("Switches which you can enable or disable. Their function is explained in readme file. If you don't want to see the intro over and over again, select \"Skip intro\". If you're having problems with keyboard or mouse inside the game, select \"Alt. input\"."),//3
    _T("."),//4
    _T("."),//5
    _T("."),//6
    _T("."),//7
    _T("."),//8
    _T("."),//9
    _T("."),//10
    _T("Accept changes."),
    _T("Abandon changes and close the window."),
};

BEGIN_EVENT_TABLE(CommandOptions, wxDialog)
    //EVT_CLOSE(CommandOptions::OnClose)
    EVT_SHOW(CommandOptions::OnShow)
END_EVENT_TABLE()

CommandOptions::CommandOptions(wxFrame *parent)
    : wxDialog (parent, -1, wxT("Command line options"), wxDefaultPosition, wxSize(460, 480))

{
    topsizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *cmdLineBox = new wxStaticBox(this, wxID_ANY, wxT("Command line") );
    cmdLineBox->SetToolTip(options_tooltips_eng[1]);
    wxStaticBoxSizer* cmdLineBoxSizer = new wxStaticBoxSizer(cmdLineBox, wxHORIZONTAL);
    {
        cmdLineCtrl = new wxTextCtrl(cmdLineBox, wxID_ANY, options_exectype_code[0], wxDefaultPosition, wxDefaultSize);
        cmdLineCtrl->SetToolTip(options_tooltips_eng[1]);
        cmdLineBoxSizer->Add(cmdLineCtrl, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
    }
    topsizer->Add(cmdLineBoxSizer, 0, wxEXPAND);

    execKindRadio = new wxRadioBox( this, wxID_ANY, wxT("Executable file"), wxDefaultPosition, wxDefaultSize,
        WXSIZEOF(options_exectype_text), options_exectype_text, 2, wxRA_SPECIFY_COLS );
    execKindRadio->SetToolTip(options_tooltips_eng[2]);
    topsizer->Add(execKindRadio, 0, wxEXPAND);

    cmdFlagsBox = new wxCheckRadioBox(this, wxID_ANY, wxT("Flags"), options_flags_text, WXSIZEOF(options_flags_text), 0 );
    cmdFlagsBox->SetToolTip(options_tooltips_eng[3],options_tooltips_eng[3]);
    wxStaticBoxSizer* cmdFlagsBoxSizer = new wxStaticBoxSizer( cmdFlagsBox, wxHORIZONTAL );
    cmdFlagsBoxSizer->Add(cmdFlagsBox->rbPanel, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance
    topsizer->Add(cmdFlagsBoxSizer, 1, wxEXPAND);

    wxPanel *cmdOtherPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *cmdOtherPanelSizer = new wxBoxSizer( wxHORIZONTAL );
    {
        wxStaticBox *gameSpeedBox = new wxStaticBox( cmdOtherPanel, wxID_ANY, wxT("Game speed") );
        wxStaticBoxSizer* gameSpeedSizer = new wxStaticBoxSizer( gameSpeedBox, wxVERTICAL );
        {
            {
                gameSpeedChkBx = new wxCheckBox(gameSpeedBox, wxID_ANY, wxT("Set turns/sec."));
                gameSpeedChkBx->SetToolTip(options_tooltips_eng[4]);
                gameSpeedChkBx->SetValue(false);
                gameSpeedSizer->Add(gameSpeedChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                wxIntegerValidator<long> gameSpeedVal(NULL, wxNUM_VAL_THOUSANDS_SEPARATOR);
                gameSpeedVal.SetRange(1,10000);
                gameSpeedTxtCtrl = new wxTextCtrl(gameSpeedBox, wxID_ANY, wxT("20"), wxDefaultPosition, wxSize(64, -1), 0, gameSpeedVal);
                gameSpeedTxtCtrl->SetToolTip(options_tooltips_eng[5]);
                gameSpeedSizer->Add(gameSpeedTxtCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
            }
        }
        cmdOtherPanelSizer->Add(gameSpeedSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

        wxStaticBox *humanPlayerBox = new wxStaticBox( cmdOtherPanel, wxID_ANY, wxT("Controlled players") );
        wxStaticBoxSizer* humanPlayerSizer = new wxStaticBoxSizer( humanPlayerBox, wxVERTICAL );
        {
            {
                humanPlayerChkBx = new wxCheckBox(humanPlayerBox, wxID_ANY, wxT("Set human id"));
                humanPlayerChkBx->SetToolTip(options_tooltips_eng[6]);
                humanPlayerChkBx->SetValue(false);
                humanPlayerSizer->Add(humanPlayerChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                wxIntegerValidator<long> humanPlayerVal(NULL, wxNUM_VAL_THOUSANDS_SEPARATOR);
                humanPlayerVal.SetRange(0,5);
                humanPlayerTxtCtrl = new wxTextCtrl(humanPlayerBox, wxID_ANY, wxT("0"), wxDefaultPosition, wxSize(64, -1), 0, humanPlayerVal);
                humanPlayerTxtCtrl->SetToolTip(options_tooltips_eng[7]);
                humanPlayerSizer->Add(humanPlayerTxtCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
            }
        }
        cmdOtherPanelSizer->Add(humanPlayerSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

        wxStaticBox *vidDriverBox = new wxStaticBox( cmdOtherPanel, wxID_ANY, wxT("SDL Video driver") );
        wxStaticBoxSizer* vidDriverSizer = new wxStaticBoxSizer( vidDriverBox, wxVERTICAL );
        {
            {
                vidDriverChkBx = new wxCheckBox(vidDriverBox, wxID_ANY, wxT("Force driver"));
                vidDriverChkBx->SetToolTip(options_tooltips_eng[8]);
                vidDriverChkBx->SetValue(false);
                vidDriverSizer->Add(vidDriverChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                wxIntegerValidator<long> vidDriverVal(NULL, wxNUM_VAL_THOUSANDS_SEPARATOR);
                vidDriverVal.SetRange(0,5);
                vidDriverTxtCtrl = new wxTextCtrl(vidDriverBox, wxID_ANY, wxT("directx"), wxDefaultPosition, wxSize(64, -1), 0, vidDriverVal);
                vidDriverTxtCtrl->SetToolTip(options_tooltips_eng[9]);
                vidDriverSizer->Add(vidDriverTxtCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
            }
        }
        cmdOtherPanelSizer->Add(vidDriverSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

        cmdOtherPanelSizer->SetMinSize(460, 64);
    }
    cmdOtherPanel->SetSizer(cmdOtherPanelSizer);
    topsizer->Add(cmdOtherPanel, 0, wxEXPAND);

    wxPanel *dlgBottomPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *dlgBottomPanelSizer = new wxBoxSizer( wxHORIZONTAL );
    {
        dlgBottomPanelSizer->AddStretchSpacer(1);
        wxButton *saveBtn = new wxButton(dlgBottomPanel, wxID_OK, _T("&Store") );
        //saveBtn->SetToolTip(tooltips_eng[10]);
        dlgBottomPanelSizer->Add(saveBtn, 0, wxEXPAND);
        dlgBottomPanelSizer->AddStretchSpacer(1);
        wxButton *exitBtn = new wxButton(dlgBottomPanel, wxID_CANCEL, _T("&Cancel") );
        //exitBtn->SetToolTip(tooltips_eng[11]);
        dlgBottomPanelSizer->Add(exitBtn, 0, wxEXPAND);
        dlgBottomPanelSizer->AddStretchSpacer(1);
    }
    dlgBottomPanel->SetSizer(dlgBottomPanelSizer);
    topsizer->Add(dlgBottomPanel, 0, wxEXPAND);

    SetSizer(topsizer);
    Centre(wxBOTH);
}

CommandOptions::~CommandOptions()
{
}

void CommandOptions::OnShow(wxShowEvent& event)
{
    if (event.IsShown()) {
        //readOptions();
        return;
    }
    //storeOptions()
}

/******************************************************************************/
