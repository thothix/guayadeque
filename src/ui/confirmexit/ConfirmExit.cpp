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
#include "ConfirmExit.h"
#include "Images.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guExitConfirmDlg::guExitConfirmDlg( wxWindow * parent ) :
    wxDialog( parent, wxID_ANY, _( "Please confirm" ), wxDefaultPosition, wxSize( -1, 160 ), wxDEFAULT_DIALOG_STYLE )
{
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    MainSizer->Add( 0, 20, 0, wxEXPAND, 5 );

    wxBoxSizer * TopSizer = new wxBoxSizer( wxHORIZONTAL );

    wxStaticBitmap * ExitBitmap = new wxStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_exit ), wxDefaultPosition, wxDefaultSize, 0 );
    TopSizer->Add( ExitBitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    wxStaticText * MessageString = new wxStaticText( this, wxID_ANY, _("Are you sure you want to exit the application?"), wxDefaultPosition, wxDefaultSize, 0 );
    MessageString->Wrap( -1 );
    TopSizer->Add( MessageString, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    MainSizer->Add( TopSizer, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

    m_AskAgainCheckBox = new wxCheckBox( this, wxID_ANY, _("Don't ask again"), wxDefaultPosition, wxDefaultSize, 0 );
    MainSizer->Add( m_AskAgainCheckBox, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

    MainSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    wxStdDialogButtonSizer * ButtonsSizer = new wxStdDialogButtonSizer();

    wxButton * ButtonsSizerOK = new wxButton( this, wxID_OK );
    ButtonsSizer->AddButton( ButtonsSizerOK );

    wxButton * ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
    ButtonsSizer->AddButton( ButtonsSizerCancel );

    ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
    ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
    ButtonsSizer->Realize();
    MainSizer->Add( ButtonsSizer, 0, wxALL|wxEXPAND, 5 );

    SetSizer( MainSizer );
    Layout();
    MainSizer->Fit(this);

    ButtonsSizerCancel->SetDefault();

    m_AskAgainCheckBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
guExitConfirmDlg::~guExitConfirmDlg()
{
}

// -------------------------------------------------------------------------------- //
bool guExitConfirmDlg::GetConfirmChecked( void )
{
    return m_AskAgainCheckBox->IsChecked();
}

}

// -------------------------------------------------------------------------------- //
