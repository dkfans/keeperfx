/******************************************************************************/
// Game Launcher for KeeperFX - free implementation of Dungeon Keeper.
/******************************************************************************/
/** @file wxImageFrame.hpp
 *     Header file for wxImageFrame.cpp.
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
#ifndef wxImageFrame_HPP
#define wxImageFrame_HPP
#include <wx/wx.h>
#include <wx/sizer.h>

class wxImageFrame : public wxFrame
{
    wxImage image;
    wxBitmap resized;
    int w, h;

public:
    wxImageFrame(wxWindow* parent, wxWindowID id, const wxString& title, wxString file, wxBitmapType format);

    void paintEvent(wxPaintEvent & evt);
    void paintNow();
    void OnSize(wxSizeEvent& event);
    void render(wxDC& dc);

    DECLARE_EVENT_TABLE()
};

#endif // wxImageFrame_HPP
