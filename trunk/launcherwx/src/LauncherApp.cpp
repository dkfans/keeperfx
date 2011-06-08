/******************************************************************************/
// Game Launcher for KeeperFX - free implementation of Dungeon Keeper.
/******************************************************************************/
/** @file LauncherApp.cpp
 *     Application class.
 * @par Purpose:
 *     Defines Application - core class of the program.
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
#include "LauncherApp.hpp"

#include <wx/sizer.h>
#include "wxImagePanel.hpp"

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "../res/keeperfx.xpm"
#endif

// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(LauncherFrame, wxImageFrame)
    EVT_MENU(Minimal_Quit,  LauncherFrame::OnQuit)
    EVT_MENU(Minimal_About, LauncherFrame::OnAbout)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(LauncherApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool LauncherApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // make sure to call this before using images
    wxInitAllImageHandlers();

    //wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    // create the main application window
    frame = new LauncherFrame(_T("KeeperFX Launcher"));

    // then create a panel with image on
    //drawPane = new wxImagePanel( frame, wxT("launchermn.jpg"), wxBITMAP_TYPE_JPEG);
    //sizer->Add(drawPane, 1, wxEXPAND);

    //frame->SetSizer(sizer);

    frame->CentreOnScreen(wxBOTH);

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
LauncherFrame::LauncherFrame(const wxString& title)
       : wxImageFrame(NULL, wxID_ANY, title, wxT("launchermn.jpg"), wxBITMAP_TYPE_JPEG)
{
    // set the frame icon
    SetIcon(wxICON(sample));

#if wxUSE_STATUSBARz
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(_T("Welcome to wxWidgets!"));
#endif // wxUSE_STATUSBAR
}


// event handlers

void LauncherFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void LauncherFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
                    _T("Welcome to %s!\n")
                    _T("\n")
                    _T("This simple application helps to properly execute KeeperFX\n")
                    _T("running under %s."),
                    wxVERSION_STRING,
                    wxGetOsDescription().c_str()
                 ),
                 _T("About KeeperFX Launcher"),
                 wxOK | wxICON_INFORMATION,
                 this);
}
