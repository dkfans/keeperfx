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
#include <wx/tokenzr.h>
#include <wx/valnum.h>
#include "wxCheckRadioBox.hpp"

wxString supported_boolean_code[] = {
    _T("OFF"),
    _T("ON"),
};

wxString supported_scrshotfmt_code[] = {
    _T("BMP"),
    _T("HSI"),
};

wxString resolutions_ingame[] = {
    _T("640x480"),
    _T("800x600"),
    _T("1024x768"),
    //_T("1280x800"),
    _T("1280x960"),
    _T("1280x1024"),
    _T("1440x900"),
    _T("1600x900"),
    //_T("1600x1200"),
    _T("1680x1050"),
    _T("1920x1080"),
    _T("1920x1200"),
    _T("2560x1440"),
    _T("2560x1600"),
};

wxString supported_scrshotfmt_text[] = {
    _T("Windows bitmap (BMP)"),
    _T("HSI 'mhwanh' (RAW)"),
};

wxString supported_languages_code[] = {
    _T("ENG"),
    _T("FRE"),
    _T("GER"),
    _T("ITA"),
    _T("SPA"),
    _T("SWE"),
    _T("POL"),
    _T("DUT"),
    //_T("HUN"),
    //_T("KOR"),
    //_T("DAN"),
    //_T("NOR"),
    _T("CZE"),
    //_T("ARA"),
    _T("RUS"),
    _T("JPN"),
    _T("CHI"),
    _T("CHT"),
    //_T("POR"),
    //_T("HIN"),
    //_T("BEN"),
    //_T("JAV"),
    _T("LAT"),
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
    //_T("Hungarian"),
    //_T("Korean"),
    //_T("Dansk"),
    //_T("Norsk"),
    _T("Česky"),
    //_T("Arabic"),
    _T("Русский"),
    _T("日本語"),
    _T("简体中文"),
    _T("繁體中文"),
    //_T("Portuguese"),
    //_T("Hindi"),
    //_T("Bengali"),
    //_T("Javanese"),
    _T("sermo Latinus"),
};

wxString tooltips_eng[] = {
    _T(""),
    _T("Select up to three resolutions. Note that original resolutions are 320x200 and 640x400; select 640x480x32 for best gaming experience. Switch between selected resolutions by pressing Alt+R during the game."),//1
    _T("Here you can type your own resolution. Use \"WIDTHxHEIGHTxCOLOUR\" scheme. Replace last \"x\" with \"w\" for windowed mode. You can select max. 4 resolutions."),// 2, obsolete
    _T("Screen resolution at which movies (ie. intro) will be played. Original is 320x200, and any higher resolution will make movie window smaller. Still, 640x480x32 is recommended, for compatibility."), // 3, obsolete
    _T("Screen resolution at which game menu is displayed. Original is 640x480 and it is recommended to select 640x480x32. Larger resolutions will make the menu smaller at center of the screen."), // 4, obsolete
    _T("Resolution used in case of screen setup failure. If the game cannot enter one of the selected resolutions (ie. in-game resolution), it will try to fall back into this resolution. 640x480x32 is recommended."), // 5, obsolete
    _T("Here you can select your language translation. This will affect the in-game messages, but also speeches during the game. Note that some campaigns may not support your language; in this case default one will be used."),
    _T("Enabling censorship will make only evil creatures to have blood, and will restrict death effect with exploding flesh. Originally, this was enabled in german language version."),
    _T("Increasing sensitivity will speed up the mouse in the game. This may also make the mouse less accurate, so be careful! Default value is 100; you can increase or decrease it at your will, but sometimes it may be better to change this setting in your OS."),
    _T("Captured screens can be saved in \"scrshots\" folder by pressing Shift+C during the game or inside menu. The HSI format isn't very popular nowadays, so you probably want to select BMP, as most graphics tools can open it."),
    _T("Select whether the game should run in full screen, or as a window. Full screen is recommended. If you've chosen window, you may want to modify input options to disallow the game to control the mouse completely."),
    _T("Write changes to \"keeperfx.cfg\" file."),
    _T("Abandon changes and close the window."),
};

int optionIndexInArray(const wxString * arr, size_t arr_len, const wxString &option)
{
    size_t i;
    for (i=0; i < arr_len; i++)
    {
        if (option.CmpNoCase(arr[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

BEGIN_EVENT_TABLE(GameSettings, wxDialog)
    EVT_CLOSE(GameSettings::OnClose)
    EVT_SHOW(GameSettings::OnShow)
    EVT_CHECKBOX(eventID_ScrnCtrlChange, GameSettings::OnScreenCtrlChange)
    EVT_BUTTON(eventID_Save, GameSettings::OnSave)
    EVT_BUTTON(eventID_Cancel, GameSettings::OnCancel)
END_EVENT_TABLE()

GameSettings::GameSettings(wxFrame *parent)
    : wxDialog (parent, -1, wxT("Settings editor"), wxDefaultPosition, wxSize(460, 480))

{
    topsizer = new wxBoxSizer( wxVERTICAL );
    topsizer->AddSpacer(10); // Top padding.
     
    // In game resolutions box
    _create_resolution_box(); 
    topsizer->Add(resolutionBoxSizer, 0, wxEXPAND);

    // Window mode check box
    topsizer->AddSpacer(10);
    wxStaticBox *windowModeBox = new wxStaticBox(this, wxID_ANY, wxT("Game window"));
    wxStaticBoxSizer* windowModeBoxSizer = new wxStaticBoxSizer(windowModeBox, wxVERTICAL);
    scrnControlChkBx = new wxCheckBox(this, eventID_ScrnCtrlChange, wxT("Windowed mode"));
    scrnControlChkBx->SetToolTip(tooltips_eng[10]);
    scrnControlChkBx->SetValue(false);
    windowModeBoxSizer->Add(scrnControlChkBx, 0, wxEXPAND);
    topsizer->Add(windowModeBoxSizer, 0, wxEXPAND);

    // Language ratio box
    topsizer->AddSpacer(10);
    langRadio = new wxRadioBox( this, wxID_ANY, wxT("Language"), wxDefaultPosition, wxSize(-1, -1),
        WXSIZEOF(supported_languages_text), supported_languages_text, 4, wxRA_SPECIFY_COLS);
    langRadio->SetToolTip(tooltips_eng[6]);
    topsizer->Add(langRadio, 0, wxEXPAND);

    // Other settings
    topsizer->AddSpacer(10);
    wxStaticBox *otherSettingsBox = new wxStaticBox( this, wxID_ANY, wxT("Other settings") );
    wxStaticBoxSizer* otherSettingsSizer = new wxStaticBoxSizer( otherSettingsBox, wxVERTICAL );
    {
        {
            censorChkBx = new wxCheckBox(this, wxID_ANY, wxT("Censorship (limit amount of blood and flesh)"));
            censorChkBx->SetToolTip(tooltips_eng[7]);
            censorChkBx->SetValue(false);
            otherSettingsSizer->Add(censorChkBx, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
        }
        {
            wxPanel *mouseSensitivityPanel = new wxPanel(this, wxID_ANY);
            wxBoxSizer *mouseSensitivityPanelSizer = new wxBoxSizer( wxHORIZONTAL );
            {
                wxStaticText *statTxt = new wxStaticText(mouseSensitivityPanel, wxID_ANY, wxT("Mouse sensitivity"));
                statTxt->SetToolTip(tooltips_eng[8]);
                mouseSensitivityPanelSizer->Add(statTxt, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
                mouseSensitivityPanelSizer->AddSpacer(16);
                wxIntegerValidator<long> mouseSensitivityVal(NULL, wxNUM_VAL_THOUSANDS_SEPARATOR);
                mouseSensitivityVal.SetRange(-10000,10000);
                mouseSensitvTxtCtrl = new wxTextCtrl(mouseSensitivityPanel, wxID_ANY, wxT("100"), wxDefaultPosition, wxSize(64, -1), 0, mouseSensitivityVal);
                mouseSensitvTxtCtrl->SetToolTip(tooltips_eng[8]);
                mouseSensitivityPanelSizer->Add(mouseSensitvTxtCtrl, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            }
            mouseSensitivityPanel->SetSizer(mouseSensitivityPanelSizer);
            otherSettingsSizer->Add(mouseSensitivityPanel, 1, wxEXPAND);
        }
        otherSettingsSizer->SetMinSize(386, 64);
    }
    topsizer->Add(otherSettingsSizer, 0, wxEXPAND); // for wxStaticBox, we're adding sizer instead of the actual wxStaticBox instance

    // Screen shot ratio box
    topsizer->AddSpacer(10);
    scrshotRadio = new wxRadioBox( this, wxID_ANY, wxT("Screenshots"), wxDefaultPosition, wxDefaultSize,
        WXSIZEOF(supported_scrshotfmt_text), supported_scrshotfmt_text, 2, wxRA_SPECIFY_COLS );
    scrshotRadio->SetToolTip(tooltips_eng[9]);
    topsizer->Add(scrshotRadio, 0, wxEXPAND);

    wxPanel *dlgBottomPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *dlgBottomPanelSizer = new wxBoxSizer( wxHORIZONTAL );
    {
        dlgBottomPanelSizer->AddStretchSpacer(1);
        wxButton *saveBtn = new wxButton(dlgBottomPanel, eventID_Save, _T("&Save") );
        saveBtn->SetToolTip(tooltips_eng[11]);
        dlgBottomPanelSizer->Add(saveBtn, 0, wxEXPAND);
        dlgBottomPanelSizer->AddStretchSpacer(1);
        wxButton *exitBtn = new wxButton(dlgBottomPanel, eventID_Cancel, _T("&Cancel") );
        exitBtn->SetToolTip(tooltips_eng[12]);
        dlgBottomPanelSizer->Add(exitBtn, 0, wxEXPAND);
        dlgBottomPanelSizer->AddStretchSpacer(1);
    }
    dlgBottomPanel->SetSizer(dlgBottomPanelSizer);
    topsizer->Add(dlgBottomPanel, 0, wxEXPAND);

    SetSizer(topsizer);
    Fit();
    Centre(wxBOTH);
}

GameSettings::~GameSettings()
{
    //SetSizer(NULL);
    delete conf;
    conf = NULL;
}

void GameSettings::ChangeResolutionOptions(int /*scr_ctrl*/)
{
    if (resSecondaryChkBx && resSecondaryCombo)
    {
        if (resSecondaryChkBx->IsChecked())
        {

            resSecondaryCombo->Enable();
            resTertiaryChkBx->Enable();
        }
        else
        {
            resSecondaryCombo->Disable(); 
            resTertiaryChkBx->Disable();
            resTertiaryCombo->Disable();
        }
    }

    if (resTertiaryChkBx && resTertiaryCombo && resTertiaryChkBx->IsEnabled())
    {
        if (resTertiaryChkBx->IsChecked())
        {
            resTertiaryCombo->Enable();
        }
        else
        {
            resTertiaryCombo->Disable();
        }
    }
}

void GameSettings::OnShow(wxShowEvent& event)
{
    if (event.IsShown()) {
        readConfiguration();
        return;
    }
}

void GameSettings::OnClose(wxCloseEvent& event)
{
    int msgRet;
    msgRet = wxMessageBox(_T("You've made changes to the configuration options. Do you want to store these changes?\n")
            _T("If you answer Yes, the previous configuration file will be replaced with the new settings you've selected."),
            _T("Save KeeperFX configuration"), wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION, this);
    switch (msgRet)
    {
      case wxYES:      // Save, then quit dialog
          writeConfiguration();
          wxLogMessage(wxT("Configuration saved."));
        break;
      case wxNO:       // Don't save; just quit dialog
          wxLogMessage(wxT("Changed discarded."));
        break;
      case wxCANCEL:   // Do nothing - so don't quit dialog
      default:
        if (event.CanVeto()) {
            event.Veto();     // Notify the calling code that we didn't agreed to quit
            return;
        }
        break;
    }

    GetParent()->Enable(true);

    event.Skip();
    EndModal(1);
}

void GameSettings::OnScreenCtrlChange(wxCommandEvent& event)
{
    // Change screen control
    ChangeResolutionOptions(event.GetInt());
    Layout();
}

void GameSettings::OnSave(wxCommandEvent& WXUNUSED(event))
{
    // Generate OnClose event
    Close();
}

void GameSettings::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    // End without generating OnClose event
    EndModal(0);
}

void GameSettings::readConfiguration()
{
    wxString value;
    int index;
    // Open the Configfile
    conf = new wxFileConfig(wxEmptyString, wxEmptyString, wxT("keeperfx.cfg"),
        wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH);

    // There's no point in editing these
    value = conf->Read(wxT("INSTALL_PATH"), wxT("./"));
    installPath = value;
    value = conf->Read(wxT("INSTALL_TYPE"), wxT("MAX"));
    installType = value;
    value = conf->Read(wxT("KEYBOARD"), wxT("101"));
    keybLayout = value;

    value = conf->Read(wxT("LANGUAGE"), supported_languages_code[0]);
    index = optionIndexInArray(supported_languages_code, WXSIZEOF(supported_languages_code), value);
    langRadio->SetSelection((index>=0)?index:0);

    value = conf->Read(wxT("SCREENSHOT"), supported_scrshotfmt_code[0]);
    index = optionIndexInArray(supported_scrshotfmt_code, WXSIZEOF(supported_scrshotfmt_code), value);
    scrshotRadio->SetSelection((index>=0)?index:0);

    value = conf->Read(wxT("INGAME_RES"), wxT("640x480 1024x768"));
    {
        int config_index = 0;
        wxStringTokenizer tokenz(value, wxT(" \t\r\n"));

        while ( tokenz.HasMoreTokens() && (config_index < 3) )
        {
            wxString param = tokenz.GetNextToken();
            config_index++;

            wxString resSuffix1 = wxT("w32");
            wxString resSuffix2 = wxT("x32");

            // Try remove suffix
            unsigned int i = param.find(resSuffix1);
            if (i != wxString::npos)
            {
                param.erase(i, param.length());

                // Temporarily we make the 'windowed' setting of the first configration as generalized one.
                if (scrnControlChkBx && (config_index == 1))
                {
                    scrnControlChkBx->SetValue(true);
                }
            }

            i = param.find(resSuffix2);
            if (i != wxString::npos)
            {
                param.erase(i, param.length());
            }

            switch (config_index)
            {
            case 1:
                if (resPrimaryCombo)
                {
                    resPrimaryCombo->SetValue(param);
                }

                break;
            case 2:                
                if (resSecondaryCombo)
                {
                    resSecondaryChkBx->SetValue(true);
                    resSecondaryCombo->SetValue(param);
                }
                break;
            case 3:                
                if (resTertiaryCombo)
                {
                    resTertiaryChkBx->SetValue(true);
                    resTertiaryCombo->SetValue(param);
                }
                break;
            default:
                break;
            }
        }

        // SetSelection() doesn't generate event to update resolution options, so lrt's call it:
        ChangeResolutionOptions(index);
        Layout();
    }

    value = conf->Read(wxT("WINDOWED_MODE"), supported_boolean_code[0]);
    index = optionIndexInArray(supported_boolean_code, WXSIZEOF(supported_boolean_code), value);
    scrnControlChkBx->SetValue((index >= 0) ? index : 0);

    index = conf->Read(wxT("POINTER_SENSITIVITY"), 100);
    value = wxString::Format(wxT("%d"), (int)index);
    mouseSensitvTxtCtrl->SetValue(value);

    value = conf->Read(wxT("CENSORSHIP"), supported_boolean_code[0]);
    index = optionIndexInArray(supported_boolean_code, WXSIZEOF(supported_boolean_code), value);
    censorChkBx->SetValue((index>=0)?index:0);
}

int GameSettings::verifyConfiguration()
{
    return 2;
}

void GameSettings::writeConfiguration()
{
    wxString strValue; 
    conf->Write(wxT("INSTALL_PATH"), installPath);
    conf->Write(wxT("INSTALL_TYPE"), installType);
    conf->Write(wxT("KEYBOARD"), keybLayout);
    conf->Write(wxT("LANGUAGE"), supported_languages_code[langRadio->GetSelection()]);
    conf->Write(wxT("SCREENSHOT"), supported_scrshotfmt_code[scrshotRadio->GetSelection()]);

    wxString resSuffix = wxT(" ");

    // WINDOWED_MODE
    conf->Write(wxT("WINDOWED_MODE"), supported_boolean_code[scrnControlChkBx->GetValue()]);

    // INGAME_RES
    strValue = resPrimaryCombo->GetValue();
    strValue.Append(resSuffix);
    if (resSecondaryChkBx->IsChecked())
    {
        strValue.Append(resSecondaryCombo->GetValue());
        strValue.Append(resSuffix);

        if (resTertiaryChkBx->IsChecked())
        {
            strValue.Append(resTertiaryCombo->GetValue());
            strValue.Append(resSuffix);
        }
    }
    conf->Write(wxT("INGAME_RES"), strValue);

    conf->Write(wxT("POINTER_SENSITIVITY"), mouseSensitvTxtCtrl->GetValue());
    conf->Write(wxT("CENSORSHIP"), supported_boolean_code[censorChkBx->GetValue()]);
    //conf->Save(); -- saving is automatic when the object is destroyed
}

void GameSettings::_create_resolution_box()
{
    // Resolution panel
    wxStaticBox * resolutionBox = new wxStaticBox(this, wxID_ANY, wxT("Game Resolution"));
    resolutionBox->SetToolTip(tooltips_eng[1]);
    resolutionBoxSizer = new wxStaticBoxSizer(resolutionBox, wxVERTICAL);

    wxString * arr_ptr= resolutions_ingame;
    size_t arr_size= WXSIZEOF(resolutions_ingame);

    // Primary resolution panel
    wxPanel *primaryResolutionPanel = new wxPanel(resolutionBox, wxID_ANY);
    wxGridSizer *primaryResolutionPanelSizer = new wxGridSizer(0, 2, 2, 2);
    {
        // Add text for primary resolution
        wxStaticText *statTxt = new wxStaticText(primaryResolutionPanel, wxID_ANY, wxT("Primary Resolution"));
        primaryResolutionPanelSizer->Add(statTxt, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

        wxString resolution = (resPrimaryCombo != NULL) ? resPrimaryCombo->GetValue() : resolutions_ingame[0];

        delete resPrimaryCombo;
        resPrimaryCombo = new wxComboBox(primaryResolutionPanel, wxID_ANY, arr_ptr[0], wxDefaultPosition, wxDefaultSize, arr_size, arr_ptr, wxCB_DROPDOWN);
        resPrimaryCombo->SetToolTip(tooltips_eng[1]);
        primaryResolutionPanelSizer->Add(resPrimaryCombo, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
        resPrimaryCombo->SetValue(resolution);
    }
    primaryResolutionPanel->SetSizer(primaryResolutionPanelSizer);
    resolutionBoxSizer->Add(primaryResolutionPanel, 0, wxEXPAND);

    // Secondary resolution panel
    wxPanel *secondaryResolutionPanel = new wxPanel(resolutionBox, wxID_ANY);
    wxGridSizer *secondaryResolutionPanelSizer = new wxGridSizer(0, 2, 2, 2);
    {
        // Add checkobox for secondary resolution
        resSecondaryChkBx = new wxCheckBox(secondaryResolutionPanel, eventID_ScrnCtrlChange, wxT("Secondary Resolution"));
        resSecondaryChkBx->SetValue(false);
        secondaryResolutionPanelSizer->Add(resSecondaryChkBx, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

        wxString resolution = (resSecondaryCombo != NULL) ? resSecondaryCombo->GetValue() : resolutions_ingame[0];

        delete resSecondaryCombo;
        resSecondaryCombo = new wxComboBox(secondaryResolutionPanel, wxID_ANY, arr_ptr[0], wxDefaultPosition, wxDefaultSize, arr_size, arr_ptr, wxCB_DROPDOWN);
        resSecondaryCombo->Disable();
        resSecondaryCombo->SetToolTip(tooltips_eng[1]);
        secondaryResolutionPanelSizer->Add(resSecondaryCombo, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
        resSecondaryCombo->SetValue(resolution);
    }
    secondaryResolutionPanel->SetSizer(secondaryResolutionPanelSizer);
    resolutionBoxSizer->Add(secondaryResolutionPanel, 0, wxEXPAND);

    // Tertiary resolution panel
    wxPanel *tertiaryResolutionPanel = new wxPanel(resolutionBox, wxID_ANY);
    wxGridSizer *tertiaryResolutionPanelSizer = new wxGridSizer(0, 2, 2, 2);
    {
        // Add checkobox for tertiary resolution
        resTertiaryChkBx = new wxCheckBox(tertiaryResolutionPanel, eventID_ScrnCtrlChange, wxT("Tertiary Resolution"));
        resTertiaryChkBx->SetValue(false);
        tertiaryResolutionPanelSizer->Add(resTertiaryChkBx, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

        wxString resolution = (resTertiaryCombo != NULL) ? resTertiaryCombo->GetValue() : resolutions_ingame[0];

        delete resTertiaryCombo;
        resTertiaryCombo = new wxComboBox(tertiaryResolutionPanel, wxID_ANY, arr_ptr[0], wxDefaultPosition, wxDefaultSize, arr_size, arr_ptr, wxCB_DROPDOWN);
        resTertiaryCombo->Disable();
        resTertiaryCombo->SetToolTip(tooltips_eng[1]);
        tertiaryResolutionPanelSizer->Add(resTertiaryCombo, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
        resTertiaryCombo->SetValue(resolution);
    }
    tertiaryResolutionPanel->SetSizer(tertiaryResolutionPanelSizer);
    resolutionBoxSizer->Add(tertiaryResolutionPanel, 0, wxEXPAND);

    resolutionBoxSizer->SetMinSize(386, 64);
}

/******************************************************************************/
