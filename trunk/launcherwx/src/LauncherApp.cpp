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
#include "wxImageFrame.hpp"
#include "FilelistChecker.hpp"

//TODO: finish checking files integrity; make integrity check button
//TODO: make launching the game
//TODO: make view readme / view log buttons
//TODO: make command options / settings dialogs
//TODO: make the window movable with mouse
//TODO: make writing configuration

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
    EVT_SHOW(LauncherFrame::OnShow)
    EVT_BUTTON(Event_Quit,  LauncherFrame::OnQuit)
    EVT_BUTTON(Event_About, LauncherFrame::OnAbout)
    EVT_BUTTON(Event_RunGame, LauncherFrame::OnRunGame)
    EVT_BUTTON(Event_Install, LauncherFrame::OnInstall)
    EVT_BUTTON(Event_Settings, LauncherFrame::OnSettings)
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

    quitButton = NULL;
    startButton = NULL;
    configButton = NULL;

    quitButton = new wxButton( this, Event_Quit, _T("E&xit"), wxPoint(360,320), wxSize(96,30), wxNO_BORDER );
    startButton = new wxButton( this, Event_RunGame, _T("Sta&rt game"), wxPoint(24,320), wxSize(96,30), wxNO_BORDER );
    startButton->Disable();
    installButton = new wxButton( this, Event_Install, _T("In&stallation"), wxPoint(192,320), wxSize(96,30), wxNO_BORDER );

    configButton = new wxButton( this, Event_Settings, _T("Se&ttings"), wxPoint(360,40), wxSize(96,30), wxNO_BORDER );

    msgTextCtrl = new wxTextCtrl(this, wxID_ANY, _T("Initializing...\n"), wxPoint(96, 180), wxSize(480-2*96, 120), wxTE_MULTILINE);
    logTarget = wxLog::SetActiveTarget(new wxLogTextCtrl(msgTextCtrl));

    startButton->SetBackgroundColour(wxT("black"));
    startButton->SetForegroundColour(wxT("white"));
    quitButton->SetBackgroundColour(wxT("black"));
    quitButton->SetForegroundColour(wxT("white"));
    configButton->SetBackgroundColour(wxT("black"));
    configButton->SetForegroundColour(wxT("white"));

    installButton->SetBackgroundColour(wxT("black"));
    installButton->SetForegroundColour(wxT("white"));

    msgTextCtrl->SetBackgroundColour(wxT("black"));
    msgTextCtrl->SetForegroundColour(wxT("white"));

    wxGetHomeDir(&installSrcDir);
    fxWorkDir = wxGetCwd();

    flCheck = new FilelistChecker();
}

LauncherFrame::~LauncherFrame()
{
    delete wxLog::SetActiveTarget(logTarget);
}

// event handlers

void LauncherFrame::OnShow(wxShowEvent& WXUNUSED(event))
{
    RecheckBasicFiles();
}

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

void LauncherFrame::OnInstall(wxCommandEvent& WXUNUSED(event))
{
    wxDirDialog dialog(this, _T("Select folder with original Dungeon Keeper files"), installSrcDir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dialog.ShowModal() == wxID_OK)
    {
        int msgRet;
        installSrcDir = dialog.GetPath();
        flCheck->clearResults();
        if (!flCheck->verifyList(installSrcDir.wchar_str(),additional_complete_check)) {
            wxMessageBox(_T("The folder you've selected dosn't seem to contain files needed by KeeperFX.\n")
                _T("Please select the proper folder, or try with another release or Dungeon Keeper."),
                _T("Dungeon Keeper folder not correct"), wxOK | wxICON_WARNING, this);
            return;
        }
        msgRet = wxMessageBox(_T("The files in selected folder have been checked and are correct.\n")
                _T("When copied into KeeperFX folder, they will allow you to play the game. Do you want to copy the files from selected folder?"),
                _T("KeeperFX files installation"), wxYES_NO | wxYES_DEFAULT | wxICON_QUESTION, this);
        if (msgRet != wxYES) {
            wxLogMessage(wxT("Files copy operation canceled."));
            return;
        }
        wxLogMessage(wxT("TODO."));
    }
}

void LauncherFrame::OnSettings(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Unfinished function. Please edit \"keeperfx.cfg\" by hand."),
                 _T("KeeperFX Launcher"), wxOK | wxICON_WARNING, this);
}

void LauncherFrame::OnRunGame(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(_T("Unfinished function."),
                 _T("KeeperFX Launcher"), wxOK | wxICON_WARNING, this);
}

void LauncherFrame::RecheckBasicFiles(void)
{
    bool can_start;
    can_start = true;
    if (can_start)
    {
        flCheck->clearResults();
        if (!flCheck->verifyList(fxWorkDir.wchar_str(),supplied_basic_check)) {
            wxLogMessage(wxT("Files which are supposed to be included in KeeperFX package are not present."));
            can_start = false;
        }
    }
    if (can_start)
    {
        flCheck->clearResults();
        if (!flCheck->verifyList(fxWorkDir.wchar_str(),additional_basic_check)) {
            wxLogMessage(wxT("Game files which have to be copied from original DK are not present."));
            can_start = false;
        }
    }
    if (can_start) {
        startButton->Enable();
        wxLogMessage(wxT("Ready to start the game."));
    } else {
        startButton->Disable();
    }
}
