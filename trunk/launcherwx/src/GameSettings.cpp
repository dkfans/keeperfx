/******************************************************************************/
// Utility routines.
/******************************************************************************/
/** @file GameSettings.cpp
 *     A frame which allows changing game settings file.
 * @par Purpose:
 *     Loads config file, allows changing it via GUI and then saves.
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
#include "GameSettings.hpp"

#include <cstddef>
#include <cstring>
#include <vector>
#include <sys/stat.h>
#include <stdexcept>
#include <wx/wx.h>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>

const char *supported_boolean_code[] = {
    "OFF",
    "ON",
};

const char *supported_scrshotfmt_code[] = {
    "BMP",
    "HSI",
};

wxString supported_scrshotfmt_text[] = {
    _T("Windows bitmap (BMP)"),
    _T("HSI 'mhwanh' (RAW)"),
};

const char *supported_languages_code[] = {
    "ENG",
    "FRE",
    "GER",
    "ITA",
    "SPA",
    "SWE",
    "POL",
    "DUT",
    "HUN",
    "AUS",
    "DAN",
    "NOR",
    "CES",
    "MAG",
    "RUS",
    "JAP",
    "CHI",
    "CHT",
};

wxString supported_languages_text[] = {
    _T("English"),
    _T("Français"),
    _T("Deutsch"),
    _T("Italiano"),
    _T("Español"),
    _T("Svenska"),
    _T("Polski"),
    _T("Nederlands"),
    _T("Magyar"),
    _T("AUS"),
    _T("Dansk"),
    _T("Norsk"),
    _T("Česky"),
    _T("Mag"),
    _T("Русский"),
    _T("にほんご"),
    _T("简化中国"),
    _T("傳統的中國")
};

wxString resolutions_ingame_init[] = {
    _T("320x200x8"),
    _T("640x400x8"),
    _T("640x480x8"),
    _T("320x200x32"),
    _T("640x400x32"),
    _T("640x480x32"),
    _T("1024x768x32"),
    _T("1280x800x32"),
    _T("1280x960x32"),
    _T("1280x1024x32"),
    _T("1440x900x32"),
    _T("1600x900x32"),
    _T("1680x1050x32"),
    _T("1920x1080x32"),
//    _T("2560x1440x32"),
//    _T("2560x1600x32"),
};

std::vector<wxString> resolutions_ingame(resolutions_ingame_init, resolutions_ingame_init+sizeof(resolutions_ingame_init)/sizeof(resolutions_ingame_init[0]));

BEGIN_EVENT_TABLE(GameSettings, wxDialog)
    EVT_CLOSE(GameSettings::OnClose)
END_EVENT_TABLE()

GameSettings::GameSettings(wxFrame *parent)
    : wxDialog (parent, -1, wxT("Settings editor"), wxDefaultPosition, wxSize(400, 464))

{
    topsizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *resIngameBox = new wxStaticBox( this, wxID_ANY, wxT("In-game resolutions") );
    wxStaticBoxSizer* resIngameBoxSizer = new wxStaticBoxSizer( resIngameBox, wxHORIZONTAL );
    wxPanel *resIngamePanel = new wxPanel(resIngameBox, wxID_ANY);
    {
        wxGridSizer *resIngameSizer = new wxGridSizer(0, 3, 2, 2);
        std::vector<wxString>::iterator strIter;
        for(strIter = resolutions_ingame.begin(); strIter != resolutions_ingame.end(); strIter++)
        {
            wxCheckBox *check;
            check = new wxCheckBox(resIngamePanel, wxID_ANY, *strIter);
            check->SetValue(false);
            resIngameSizer->Add(check, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
        }
        resIngamePanel->SetSizer(resIngameSizer);
        resIngameSizer->SetMinSize(386, 64);
    }
    resIngameBoxSizer->Add(resIngamePanel, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance
    topsizer->Add(resIngameBoxSizer, 1, wxEXPAND);

    wxRadioBox * langRadio = new wxRadioBox( this, wxID_ANY, wxT("Language"), wxPoint(10,132), wxDefaultSize,
        WXSIZEOF(supported_languages_text), supported_languages_text, 4, wxRA_SPECIFY_COLS );
    topsizer->Add(langRadio, 1, wxEXPAND);

    wxRadioBox * scrshotRadio = new wxRadioBox( this, wxID_ANY, wxT("Screenshots"), wxPoint(10,170), wxDefaultSize,
        WXSIZEOF(supported_scrshotfmt_text), supported_scrshotfmt_text, 2, wxRA_SPECIFY_COLS );
    topsizer->Add(scrshotRadio, 1, wxEXPAND);

    // Open the Configfile
    conf = new wxFileConfig(wxEmptyString, wxEmptyString, wxT("keeperfx.cfg"),
        wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH);

    wxString value;
    value = conf->Read(wxT("INSTALL_PATH"), wxT("./"));
    value = conf->Read(wxT("INSTALL_TYPE"), wxT("MAX"));
    value = conf->Read(wxT("LANGUAGE"), wxT("ENG"));
    value = conf->Read(wxT("KEYBOARD"), wxT("101"));
    value = conf->Read(wxT("SCREENSHOT"), wxT("BMP"));
    value = conf->Read(wxT("FRONTEND_RES"), wxT("640x480x32 640x480x32 640x480x32"));
    value = conf->Read(wxT("INGAME_RES"), wxT("640x480x32 1024x768x32"));
//    value = conf->Read(wxT("POINTER_SENSITIVITY"), 100);
    value = conf->Read(wxT("CENSORSHIP"), wxT("OFF"));


    //TODO: make showing GameSettings frame which can load, modify and save the file

    SetSizer(topsizer);
    Centre(wxBOTH);
}

GameSettings::~GameSettings()
{
    delete conf;
    conf = NULL;
}

void GameSettings::OnClose(wxCloseEvent& event)
{
    GetParent()->Enable(true);

     SetReturnCode(1);

    while (wxIsBusy()) wxEndBusyCursor();
    event.Skip();
}



/******************************************************************************/
