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

class GameSettings : public wxDialog
{

    wxFileConfig * conf;
public:
    GameSettings(wxFrame *parent);
    virtual ~GameSettings();

    // event handlers
    void OnClose(wxCloseEvent& event);

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};


/******************************************************************************/
#endif /* GAMESETTINGS_HPP_ */
