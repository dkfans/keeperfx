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
#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdexcept>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/valnum.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
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
    _T("Skip Intro"),
    _T("Use Music Files"),
    _T("No Sound"),
    _T("No AWE Banks"),
    _T("Allow Easter Egg"),
    _T("Column Convert"),
    _T("Light Convert"),
    _T("Smoothen Video"),
    _T("Uncaptured Cursor"),
};

wxString options_wparam_code[] = {
    _T("-fps"),
    _T("-human"),
    _T("-sessions"),
    _T("-packetload"),
    _T("-packetsave"),
};

wxString options_wparam_text[] = {
    _T("Set turns/sec."),
    _T("Set human id"),
    _T("Force driver"),
    _T("Set TCP/IP sessions"),
    _T("Load"),
    _T("Save"),
};

wxString options_tooltips_eng[] = {
    _T(""),
    _T("Command line to run. Here you can type by hand the parameters you wish to use."),//1
    _T("You usually want standard version, as it is fast and stable. Heavylog version logs huge amount of messages to a file \"keeperfx.log\" while you're playing. This requires a lot more of computation power, so on slower machines it might severely affect gameplay speed. But if the game will crash, the LOG file may help the developers to fix the problem."),//2
    _T("Switches which you can enable or disable. Their function is explained in readme file. If you don't want to see the intro over and over again, select \"Skip intro\". If you're having problems with keyboard or mouse inside the game, select \"Uncaptured Cursor\"."),//3
    _T("Gameplay speed. Increasing amount of turns per second will make the action faster. Note that you can temporarely unlock the speed limiter with Ctrl+'+'."),//4
    _T("Change human player ID. This allows you to play as blue, green or yellow keeper. Use this option for skirmish - single player levels won't work properly with it, unless they were especially designed for human to play as another keeper."),//5
    //_T("Set the video driver to be used by SDL library. Valid options on Windows host are 'directx' and 'windib'. Use this if your system is broken and most games do not work on it."),//6
    _T("Host/peer addresses required to join a TCP/IP game. See 'tcp_readme.txt' to get detailed instructions on making multiplayer work."),//6
    _T("Loads a previously created packet file. Starts the level for which packet file was created, and continues the gameplay. You may exit this mode by pressing Alt+X, or take over the control by pressing Alt+T."),//7
    _T("Writes a packet file (replay file) when playing. After using this option, you must start a new level and play it continuously to create the replay correctly. Exiting the level or loading will stop the writing process and truncate your replay file."),//8
    _T("Packet files (replays) handling. If you wish to save a reply of your game, or load a previously saved one, then use this. Otherwise, set it to 'None' to disable the option. Saved replay will be loadable as long as you won't change any of the game files."),//9
    _T("Accept changes."),
    _T("Abandon changes and close the window."),
};

BEGIN_EVENT_TABLE(CommandOptions, wxDialog)
    //EVT_CLOSE(CommandOptions::OnClose)
    EVT_SHOW(CommandOptions::OnShow)
    EVT_RADIOBOX(eventID_ChangeOption, CommandOptions::OnCmdRefresh)
    EVT_RADIOBUTTON(eventID_ChangeOption, CommandOptions::OnCmdRefresh)
    EVT_CHECKBOX(eventID_ChangeOption, CommandOptions::OnCmdRefresh)
    EVT_BUTTON(eventID_CmdUpdate, CommandOptions::OnCmdUpdate)
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

    execKindRadio = new wxRadioBox( this, eventID_ChangeOption, wxT("Executable file"), wxDefaultPosition, wxDefaultSize,
        WXSIZEOF(options_exectype_text), options_exectype_text, 2, wxRA_SPECIFY_COLS );
    execKindRadio->SetToolTip(options_tooltips_eng[2]);
    topsizer->Add(execKindRadio, 0, wxEXPAND);

    cmdFlagsBox = new wxCheckRadioBox(this, eventID_ChangeOption, wxT("Flags"), options_flags_code, options_flags_text, WXSIZEOF(options_flags_text), 0 );
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
                gameSpeedChkBx = new wxCheckBox(gameSpeedBox, eventID_ChangeOption, options_wparam_text[0]);
                gameSpeedChkBx->SetToolTip(options_tooltips_eng[4]);
                gameSpeedChkBx->SetValue(false);
                gameSpeedSizer->Add(gameSpeedChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                wxIntegerValidator<long> gameSpeedVal(NULL, wxNUM_VAL_THOUSANDS_SEPARATOR);
                gameSpeedVal.SetRange(1,10000);
                gameSpeedTxtCtrl = new wxTextCtrl(gameSpeedBox, eventID_ChangeOption, wxT("20"), wxDefaultPosition, wxSize(64, -1), 0, gameSpeedVal);
                gameSpeedTxtCtrl->SetToolTip(options_tooltips_eng[4]);
                gameSpeedSizer->Add(gameSpeedTxtCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
                gameSpeedTxtCtrl->Connect(wxEVT_KEY_UP, wxKeyEventHandler(CommandOptions::OnCmdKeyRefresh), NULL, this);
            }
        }
        cmdOtherPanelSizer->Add(gameSpeedSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

        wxStaticBox *humanPlayerBox = new wxStaticBox( cmdOtherPanel, wxID_ANY, wxT("Controlled players") );
        wxStaticBoxSizer* humanPlayerSizer = new wxStaticBoxSizer( humanPlayerBox, wxVERTICAL );
        {
            {
                humanPlayerChkBx = new wxCheckBox(humanPlayerBox, eventID_ChangeOption, options_wparam_text[1]);
                humanPlayerChkBx->SetToolTip(options_tooltips_eng[5]);
                humanPlayerChkBx->SetValue(false);
                humanPlayerSizer->Add(humanPlayerChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                wxIntegerValidator<long> humanPlayerVal(NULL, wxNUM_VAL_THOUSANDS_SEPARATOR);
                humanPlayerVal.SetRange(0,5);
                humanPlayerTxtCtrl = new wxTextCtrl(humanPlayerBox, eventID_ChangeOption, wxT("0"), wxDefaultPosition, wxSize(64, -1), 0, humanPlayerVal);
                humanPlayerTxtCtrl->SetToolTip(options_tooltips_eng[5]);
                humanPlayerSizer->Add(humanPlayerTxtCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
                humanPlayerTxtCtrl->Connect(wxEVT_KEY_UP, wxKeyEventHandler(CommandOptions::OnCmdKeyRefresh), NULL, this);
            }
        }
        cmdOtherPanelSizer->Add(humanPlayerSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

        cmdOtherPanelSizer->SetMinSize(460, 64);
    }
    cmdOtherPanel->SetSizer(cmdOtherPanelSizer);
    topsizer->Add(cmdOtherPanel, 0, wxEXPAND);

    wxPanel *cmdOther2Panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *cmdOther2PanelSizer = new wxBoxSizer( wxHORIZONTAL );
    {
        wxStaticBox *netSessionsBox = new wxStaticBox( cmdOther2Panel, wxID_ANY, wxT("Multiplayer peers") );
        wxStaticBoxSizer* netSessionsSizer = new wxStaticBoxSizer( netSessionsBox, wxVERTICAL );
        {
            {
                netSessionsChkBx = new wxCheckBox(netSessionsBox, eventID_ChangeOption, options_wparam_text[3]);
                netSessionsChkBx->SetToolTip(options_tooltips_eng[6]);
                netSessionsChkBx->SetValue(false);
                netSessionsSizer->Add(netSessionsChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                wxTextValidator netSessionsVal(wxFILTER_INCLUDE_CHAR_LIST, NULL);
                netSessionsVal.SetCharIncludes(L"abcdefABCDEF0123456789-_.:/;");
                netSessionsTxtCtrl = new wxTextCtrl(netSessionsBox, eventID_ChangeOption, wxT("192.168.1.1:5555"), wxDefaultPosition, wxSize(216, -1), 0, netSessionsVal);
                netSessionsTxtCtrl->SetToolTip(options_tooltips_eng[6]);
                netSessionsSizer->Add(netSessionsTxtCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
                netSessionsTxtCtrl->Connect(wxEVT_KEY_UP, wxKeyEventHandler(CommandOptions::OnCmdKeyRefresh), NULL, this);
            }
        }
        cmdOther2PanelSizer->Add(netSessionsSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

        wxStaticBox *packetFileBox = new wxStaticBox( cmdOther2Panel, wxID_ANY, wxT("Packet files") );
        wxStaticBoxSizer* packetFileSizer = new wxStaticBoxSizer( packetFileBox, wxVERTICAL );
        {
            wxBoxSizer* packetActionSizer = new wxBoxSizer( wxHORIZONTAL );
            {
                packetActionNoRadio = new wxRadioButton(packetFileBox, eventID_ChangeOption, "None", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
                packetActionNoRadio->SetToolTip(options_tooltips_eng[9]);
                packetActionNoRadio->SetValue(false);
                packetActionSizer->Add(packetActionNoRadio, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                packetActionLdRadio = new wxRadioButton(packetFileBox, eventID_ChangeOption, options_wparam_text[4]);
                packetActionLdRadio->SetToolTip(options_tooltips_eng[7]);
                packetActionLdRadio->SetValue(false);
                packetActionSizer->Add(packetActionLdRadio, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            {
                packetActionSvRadio = new wxRadioButton(packetFileBox, eventID_ChangeOption, options_wparam_text[5]);
                packetActionSvRadio->SetToolTip(options_tooltips_eng[8]);
                packetActionSvRadio->SetValue(false);
                packetActionSizer->Add(packetActionSvRadio, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            packetFileSizer->Add(packetActionSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

            {
                wxTextValidator packetFileVal(wxFILTER_INCLUDE_CHAR_LIST, NULL);
                packetFileVal.SetCharIncludes(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_.:/\\[];");
                packetFileTxtCtrl = new wxTextCtrl(packetFileBox, eventID_ChangeOption, wxT("reply.pck"), wxDefaultPosition, wxSize(216, -1), 0, packetFileVal);
                packetFileTxtCtrl->SetToolTip(options_tooltips_eng[9]);
                packetFileSizer->Add(packetFileTxtCtrl, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL);
                packetFileTxtCtrl->Connect(wxEVT_KEY_UP, wxKeyEventHandler(CommandOptions::OnCmdKeyRefresh), NULL, this);
            }
        }
        cmdOther2PanelSizer->Add(packetFileSizer, 1, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

        cmdOther2PanelSizer->SetMinSize(460, 64);
    }
    cmdOther2Panel->SetSizer(cmdOther2PanelSizer);
    topsizer->Add(cmdOther2Panel, 0, wxEXPAND);

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
    Fit();
    Centre(wxBOTH);
}

CommandOptions::~CommandOptions(void)
{
}

std::wstring CommandOptions::optionsToCommandLine(void)
{
    wxString selected_flags[WXSIZEOF(options_flags_text)+1];
    size_t sel_flags_num,i;
    std::wstring cmd = L"";
    cmd += options_exectype_code[execKindRadio->GetSelection()];
    sel_flags_num = WXSIZEOF(options_flags_text)+1;
    cmdFlagsBox->GetSelected(selected_flags, sel_flags_num);
    for (i=0; i < sel_flags_num; i++) {
        cmd += L" ";
        cmd += selected_flags[i];
    }
    if ((gameSpeedChkBx->GetValue()) && (gameSpeedTxtCtrl->GetValue().Length() > 0)) {
        cmd += L" ";
        cmd += options_wparam_code[0];
        cmd += L" ";
        cmd += gameSpeedTxtCtrl->GetValue();
    }
    if ((humanPlayerChkBx->GetValue()) && (humanPlayerTxtCtrl->GetValue().Length() > 0)) {
        cmd += L" ";
        cmd += options_wparam_code[1];
        cmd += L" ";
        cmd += humanPlayerTxtCtrl->GetValue();
    }
    if ((netSessionsChkBx->GetValue()) && (netSessionsTxtCtrl->GetValue().Length() > 0)) {
        cmd += L" ";
        cmd += options_wparam_code[2];
        cmd += L" ";
        cmd += netSessionsTxtCtrl->GetValue();
    }
    if ((packetActionLdRadio->GetValue()) && (packetFileTxtCtrl->GetValue().Length() > 0)) {
        cmd += L" ";
        cmd += options_wparam_code[3];
        cmd += L" ";
        cmd += packetFileTxtCtrl->GetValue();
    }
    if ((packetActionSvRadio->GetValue()) && (packetFileTxtCtrl->GetValue().Length() > 0)) {
        cmd += L" ";
        cmd += options_wparam_code[4];
        cmd += L" ";
        cmd += packetFileTxtCtrl->GetValue();
    }
    return cmd;
}

std::vector<std::wstring> tokenize(const std::wstring& str,const std::wstring& delimiters)
{
    std::vector<std::wstring> tokens;

    // skip delimiters at beginning.
    std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);

    // find first "non-delimiter".
    std::wstring::size_type pos = str.find_first_of(delimiters, lastPos);

        while (std::wstring::npos != pos || std::wstring::npos != lastPos)
        {
            // found a token, add it to the vector.
            tokens.push_back(str.substr(lastPos, pos - lastPos));

            // skip delimiters.  Note the "not_of"
            lastPos = str.find_first_not_of(delimiters, pos);

            // find next "non-delimiter"
            pos = str.find_first_of(delimiters, lastPos);
        }

    return tokens;
}

void CommandOptions::commandLineToOptions(const std::wstring& cmdln)
{
    int exec_index;
    wxString selected_flags[WXSIZEOF(options_flags_text)+1];
    size_t sel_flags_num;
    wxString selected_wparams[WXSIZEOF(options_wparam_text)+1];
    std::wstring unrecognized;
    exec_index = 0;
    sel_flags_num = 0;
    size_t k;
    std::vector<std::wstring> tok = tokenize(cmdln, L" \t\r\n");
    std::vector<std::wstring>::iterator iter = tok.begin();
    // First item - executable name
    if (iter != tok.end())
    {
        for (k = 0; k < sizeof(options_exectype_code)/sizeof(options_exectype_code[0]); k++)
        {
            if (options_exectype_code[k].compare(*iter) == 0) {
                exec_index = k;
                break;
            }
        }
    }
    // Now parameters
    for (; iter != tok.end(); iter++)
    {
        bool recognized = false;
        for (k = 0; k < sizeof(options_flags_code)/sizeof(options_flags_code[0]); k++)
        {
            if (options_flags_code[k].compare(*iter) == 0) {
                recognized = true;
                selected_flags[sel_flags_num] = *iter;
                sel_flags_num++;
                break;
            }
        }
        if (recognized) continue;
        for (k = 0; k < sizeof(options_wparam_code)/sizeof(options_wparam_code[0]); k++)
        {
            if (options_wparam_code[k].compare(*iter) == 0) {
                recognized = true;
                iter++; if (iter == tok.end()) break;
                selected_wparams[k] = *iter;
                break;
            }
        }
        if (recognized) continue;
        unrecognized += L" ";
        unrecognized += *iter;
    }
    execKindRadio->SetSelection((exec_index>=0)?exec_index:0);
    cmdFlagsBox->SetSelected(WXSIZEOF(options_flags_text)+1, selected_flags, sel_flags_num);
    if (selected_wparams[0].length() > 0) {
        gameSpeedChkBx->SetValue(true);
        gameSpeedTxtCtrl->SetValue(selected_wparams[0]);
    }
    if (selected_wparams[1].length() > 0) {
        humanPlayerChkBx->SetValue(true);
        humanPlayerTxtCtrl->SetValue(selected_wparams[1]);
    }
    if (selected_wparams[3].length() > 0) {
        netSessionsChkBx->SetValue(true);
        netSessionsTxtCtrl->SetValue(selected_wparams[3]);
    }
    if (selected_wparams[4].length() > 0) {
        packetActionLdRadio->SetValue(true);
        packetFileTxtCtrl->SetValue(selected_wparams[4]);
    } else
    if (selected_wparams[5].length() > 0) {
        packetActionSvRadio->SetValue(true);
        packetFileTxtCtrl->SetValue(selected_wparams[5]);
    } else {
        packetActionNoRadio->SetValue(true);
    }
    unrecognizedOptions = unrecognized;
}


void CommandOptions::readOptionsFile(void)
{
    wxString str;
    // open the file
    wxFileInputStream istrm(L"launch.sh");
    wxTextInputStream text(istrm, wxT("\x09"), wxConvUTF8 );
    while(istrm.IsOk() && !istrm.Eof() )
    {
        str = text.ReadLine();
        // skip empty lines and comments
        if ((str.length() > 1) && (str[0]!='#')) {
            break;
        }
    }
    if (str.length() > 1) {
        storedCommandLine = str.ToStdWstring();
    } else {
        storedCommandLine = optionsToCommandLine();
    }
}

void CommandOptions::storeOptionsFile(void)
{
    wxString str;
    // open the file
    wxFileOutputStream ostrm(L"launch.sh");
    wxTextOutputStream text(ostrm, wxEOL_NATIVE, wxConvUTF8 );
    if (!ostrm.IsOk()) {
        return;
    }
    text.WriteString(L"#!/bin/sh\n");
    text.WriteString(storedCommandLine);
    text.Flush();
}

void CommandOptions::readOptions(void)
{
    readOptionsFile();
    cmdLineCtrl->SetValue(storedCommandLine);
    commandLineToOptions(cmdLineCtrl->GetValue().ToStdWstring());
}

void CommandOptions::storeOptions(void)
{
    storedCommandLine = cmdLineCtrl->GetValue().ToStdWstring();
    storeOptionsFile();
}

std::wstring CommandOptions::getCommandLine(void)
{
    readOptionsFile();
    return storedCommandLine;
}

void CommandOptions::OnShow(wxShowEvent& event)
{
    if (event.IsShown()) {
        readOptions();
        return;
    }
    storeOptions();
}

void CommandOptions::OnCmdRefresh(wxCommandEvent& WXUNUSED(event))
{
    cmdLineCtrl->SetValue(optionsToCommandLine());
}

void CommandOptions::OnCmdKeyRefresh(wxKeyEvent& WXUNUSED(event))
{
    cmdLineCtrl->SetValue(optionsToCommandLine());
}

void CommandOptions::OnCmdUpdate(wxCommandEvent& WXUNUSED(event))
{
}
/******************************************************************************/
