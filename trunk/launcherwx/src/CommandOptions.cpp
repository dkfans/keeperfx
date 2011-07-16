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

BEGIN_EVENT_TABLE(CommandOptions, wxDialog)
    //EVT_CLOSE(CommandOptions::OnClose)
    EVT_SHOW(CommandOptions::OnShow)
END_EVENT_TABLE()

CommandOptions::CommandOptions(wxFrame *parent)
    : wxDialog (parent, -1, wxT("Command line options"), wxDefaultPosition, wxSize(460, 480))

{
    topsizer = new wxBoxSizer( wxVERTICAL );

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
