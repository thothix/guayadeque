/*
   Copyright (C) 2008-2023 J.Rios <anonbeat@gmail.com>
   Copyright (C) 2024-2025 Tiago T Barrionuevo <thothix@protonmail.com>

   This file is part of Guayadeque Music Player.

   Guayadeque is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Guayadeque is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Guayadeque. If not, see <https://www.gnu.org/licenses/>.
*/
#include "RadioEditor.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guRadioEditor::guRadioEditor( wxWindow* parent, const wxString& title, const wxString &name, const wxString &link ) :
    wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize( 400,150 ), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* MainSizer;
    wxStaticText * NameLabel;
    wxStaticText * LinkLabel;
    wxStdDialogButtonSizer * StdBtnSizer;
    wxButton* StdBtnSizerOK;
    wxButton* StdBtnSizerCancel;

    MainSizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer * FlexGridSizer;
    FlexGridSizer = new wxFlexGridSizer( 2, 0, 0 );
    FlexGridSizer->AddGrowableCol( 1 );
    FlexGridSizer->SetFlexibleDirection( wxBOTH );
    FlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    NameLabel = new wxStaticText( this, wxID_ANY, wxString::Format("%s:", _("Name")), wxDefaultPosition, wxDefaultSize, 0 );
    NameLabel->Wrap( -1 );
    FlexGridSizer->Add( NameLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    m_NameTextCtrl = new wxTextCtrl( this, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, 0 );
    m_NameTextCtrl->SetFocus();
    FlexGridSizer->Add( m_NameTextCtrl, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    LinkLabel = new wxStaticText( this, wxID_ANY, _( "Link:" ), wxDefaultPosition, wxDefaultSize, 0 );
    LinkLabel->Wrap( -1 );
    FlexGridSizer->Add( LinkLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    m_LinkTextCtrl = new wxTextCtrl( this, wxID_ANY, link, wxDefaultPosition, wxDefaultSize, 0 );
    FlexGridSizer->Add( m_LinkTextCtrl, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    MainSizer->Add( FlexGridSizer, 1, wxEXPAND, 5 );

    StdBtnSizer = new wxStdDialogButtonSizer();
    StdBtnSizerOK = new wxButton( this, wxID_OK );
    StdBtnSizer->AddButton( StdBtnSizerOK );
    StdBtnSizerCancel = new wxButton( this, wxID_CANCEL );
    StdBtnSizer->AddButton( StdBtnSizerCancel );
    StdBtnSizer->SetAffirmativeButton( StdBtnSizerOK );
    StdBtnSizer->SetCancelButton( StdBtnSizerCancel );
    StdBtnSizer->Realize();
    MainSizer->Add( StdBtnSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

    this->SetSizer( MainSizer );
    this->Layout();

    StdBtnSizerOK->SetDefault();

    m_NameTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
guRadioEditor::~guRadioEditor()
{
}

}

// -------------------------------------------------------------------------------- //
