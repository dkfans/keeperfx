/******************************************************************************/
// Game Launcher for KeeperFX - free implementation of Dungeon Keeper.
/******************************************************************************/
/** @file wxCheckRadioBox.cpp
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
#include "wxCheckRadioBox.hpp"

#include <wx/wx.h>

BEGIN_EVENT_TABLE(wxCheckRadioBox, wxStaticBox)
// catch checkbox events
    EVT_CHECKBOX(eventID_Checkbox, wxCheckRadioBox::OnCheckButton)
END_EVENT_TABLE()


wxCheckRadioBox::wxCheckRadioBox(wxWindow *parent, wxWindowID id,
    const wxString& label,
    const wxString *values_arr, size_t values_num, size_t custom_num,
    const wxPoint& pos, const wxSize& size,
    long style, const wxString& name)
    : wxStaticBox(parent, id, label, pos, size, style, name)
{
    select_limit = 1;
    select_num = 0;
    rbValues.resize(values_num);
    rbPanel = new wxPanel(this, wxID_ANY);
    {
        wxGridSizer *rbPanelSizer = new wxGridSizer(0, 3, 2, 2);
        size_t i;
        for(i=0; i < values_num-custom_num; i++)
        {
            wxCheckBox *check;
            rbValues[i] = values_arr[i];
            check = new wxCheckBox(rbPanel, eventID_Checkbox, rbValues[i]);
            check->SetValue(false);
            rbPanelSizer->Add(check, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
            rbCheckboxes.push_back(check);
        }
        for(; i < values_num; i++)
        {
            wxPanel *resIngameCustPanel = new wxPanel(rbPanel, wxID_ANY);
            wxBoxSizer *rbCustPanelSizer = new wxBoxSizer( wxHORIZONTAL );
            {
                wxCheckBox *check;
                rbValues[i] = values_arr[i];
                check = new wxCheckBox(resIngameCustPanel, eventID_Checkbox, _T("cust:"));
                check->SetValue(false);
                rbCustPanelSizer->Add(check, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
                rbCheckboxes.push_back(check);
                wxTextCtrl *txtCtrl = new wxTextCtrl(resIngameCustPanel, wxID_ANY, rbValues[i], wxDefaultPosition, wxDefaultSize);
                rbCustPanelSizer->Add(txtCtrl, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
                rbTextCtrls.push_back(txtCtrl);
            }
            resIngameCustPanel->SetSizer(rbCustPanelSizer);
            rbPanelSizer->Add(resIngameCustPanel, 1, wxEXPAND);
        }
        rbPanel->SetSizer(rbPanelSizer);
        rbPanelSizer->SetMinSize(386, 64);
    }
}

void wxCheckRadioBox::SetToolTip( const wxString &normal_tip, const wxString &custom_tip )
{
    size_t i;
    wxStaticBox::SetToolTip(normal_tip);
    rbPanel->SetToolTip(normal_tip);
    size_t norm_num = rbCheckboxes.size() - rbTextCtrls.size();
    for (i=0; i < norm_num; i++)
    {
        rbCheckboxes[i]->SetToolTip(normal_tip);
    }
    for (; i < rbCheckboxes.size(); i++)
    {
        rbCheckboxes[i]->SetToolTip(custom_tip);
    }
    for (i=0; i < rbTextCtrls.size(); i++)
    {
        rbTextCtrls[i]->SetToolTip(custom_tip);
    }
}

int wxCheckRadioBox::OptionIndexInCheckboxes(const wxString &option)
{
    size_t i;
    size_t norm_num = rbValues.size();
    for (i=0; i < norm_num; i++)
    {
        if (option.CmpNoCase(rbValues[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

void wxCheckRadioBox::OnCheckButton(wxCommandEvent& event)
{
    if (event.IsChecked())
    {
        select_num++;
        if (select_num >= select_limit) {
            SetAllUncheckedEnabled(false);
        }
    } else {
        select_num--;
        if (select_num < select_limit) {
            SetAllUncheckedEnabled(true);
        }
    }
}

void wxCheckRadioBox::SetAllUncheckedEnabled(bool nstate)
{
    size_t i;
    for (i=0; i < rbCheckboxes.size(); i++)
    {
        if (!rbCheckboxes[i]->GetValue()) {
            rbCheckboxes[i]->Enable(nstate);
        }
    }
}

void wxCheckRadioBox::SetSelected(size_t max_selected, const wxString *sel_options, size_t sel_options_num)
{
    size_t i;
    int index;
    // Uncheck all
    for (i=0; i < rbCheckboxes.size(); i++)
    {
        rbCheckboxes[i]->SetValue(false);
    }
    select_limit = max_selected;
    select_num = 0;
    // Check those which were supplied in "options"
    size_t norm_num = rbCheckboxes.size() - rbTextCtrls.size();
    for (i=0; i < sel_options_num; i++)
    {
        index = OptionIndexInCheckboxes(sel_options[i]);
        if (index >= 0)
        {
            // Found the index in constant values - check it
            rbCheckboxes[index]->SetValue(true);
        } else
        {
            // Index not found in constant values - update custom res.
            size_t k;
            for (k=0; k < rbTextCtrls.size(); k++)
            {
                index = norm_num+k;
                if (rbCheckboxes[index]->GetValue())
                    continue;
                rbTextCtrls[k]->SetValue(sel_options[i]);
                rbCheckboxes[index]->SetValue(true);
                break;
            }
        }
    }
    // Update number of selected boxes
    for (i=0; i < rbCheckboxes.size(); i++)
    {
        if (rbCheckboxes[i]->GetValue()) {
            select_num++;
        }
    }
}

void wxCheckRadioBox::GetSelected(wxString *sel_options, size_t &sel_options_num)
{
    size_t i,k;
    size_t opts_set;
    size_t norm_num = rbCheckboxes.size() - rbTextCtrls.size();
    opts_set = 0;
    for (i=0; i < norm_num; i++)
    {
        if (rbCheckboxes[i]->GetValue())
        {
            sel_options[opts_set] = rbValues[i];
            opts_set++;
        }
    }
    for (k = 0; i < rbCheckboxes.size(); i++,k++)
    {
        if (rbCheckboxes[i]->GetValue())
        {
            sel_options[opts_set] = rbTextCtrls[k]->GetValue();
            opts_set++;
        }
    }
    sel_options_num = opts_set;
}
/******************************************************************************/
