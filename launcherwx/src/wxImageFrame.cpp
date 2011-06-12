/******************************************************************************/
// Game Launcher for KeeperFX - free implementation of Dungeon Keeper.
/******************************************************************************/
/** @file wxImageFrame.cpp
 *     A panel with scalable image in background.
 * @par Purpose:
 *     Prepares a panel which will be the loader background.
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
#include "wxImageFrame.hpp"

BEGIN_EVENT_TABLE(wxImageFrame, wxFrame)
// catch paint events
EVT_PAINT(wxImageFrame::paintEvent)
//Size event
EVT_SIZE(wxImageFrame::OnSize)
END_EVENT_TABLE()


wxImageFrame::wxImageFrame(wxWindow* parent, wxWindowID id, const wxString& title, wxString file, wxBitmapType format) :
wxFrame(parent, id, title, wxDefaultPosition, wxSize (480, 360), wxCLIP_CHILDREN)
{
    // load the file... ideally add a check to see if loading was successful
    image.LoadFile(file, format);
    w = -1;
    h = -1;
}

/*
 * Called by the system of by wxWidgets when the panel needs
 * to be redrawn. You can also trigger this call by
 * calling Refresh()/Update().
 */

void wxImageFrame::paintEvent(wxPaintEvent & WXUNUSED(evt))
{
    // depending on your system you may need to look at double-buffered dcs
    wxPaintDC dc(this);
    render(dc);
}

/*
 * Alternatively, you can use a clientDC to paint on the panel
 * at any time. Using this generally does not free you from
 * catching paint events, since it is possible that e.g. the window
 * manager throws away your drawing when the window comes to the
 * background, and expects you will redraw it when the window comes
 * back (by sending a paint event).
 */
void wxImageFrame::paintNow()
{
    // depending on your system you may need to look at double-buffered dcs
    wxClientDC dc(this);
    render(dc);
}

/*
 * Here we do the actual rendering. I put it in a separate
 * method so that it can work no matter what type of DC
 * (e.g. wxPaintDC or wxClientDC) is used.
 */
void wxImageFrame::render(wxDC&  dc)
{
    int neww, newh;
    dc.GetSize( &neww, &newh );

    if( (neww != w) || (newh != h) )
    {
        resized = wxBitmap( image.Scale( neww, newh ) );
        w = neww;
        h = newh;
        dc.DrawBitmap( resized, 0, 0, false );
    } else {
        dc.DrawBitmap( resized, 0, 0, false );
    }
}

/*
 * Here we call refresh to tell the panel to draw itself again.
 * So when the user resizes the image panel the image should be resized too.
 */
void wxImageFrame::OnSize(wxSizeEvent& event)
{
    Refresh();
    //skip the event.
    event.Skip();
}
/******************************************************************************/
