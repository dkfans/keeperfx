/******************************************************************************/
// Game Launcher for KeeperFX - free implementation of Dungeon Keeper.
/******************************************************************************/
/** @file wxImagePanel.hpp
 *     Header file for wxImagePanel.cpp.
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
#ifndef wxImagePanel_HPP
#define wxImagePanel_HPP
#include <wx/wx.h>
#include <wx/sizer.h>

class wxImagePanel : public wxPanel
{
    wxImage image;
    wxBitmap resized;
    int w, h;

public:
    wxImagePanel(wxFrame* parent, wxString file, wxBitmapType format);

    void paintEvent(wxPaintEvent & evt);
    void paintNow();
    void OnSize(wxSizeEvent& event);
    void render(wxDC& dc);

    DECLARE_EVENT_TABLE()
};

#endif // wxImagePanel_HPP
