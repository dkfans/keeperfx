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
#include <sys/stat.h>
#include <stdexcept>
#include <wx/wx.h>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/confbase.h>
#include <wx/fileconf.h>

BEGIN_EVENT_TABLE(GameSettings, wxDialog)
    EVT_CLOSE(GameSettings::OnClose)
END_EVENT_TABLE()

GameSettings::GameSettings(wxFrame *parent)
    : wxDialog (parent, -1, wxT("Settings editor"), wxDefaultPosition, wxSize(400, 290))

{

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
