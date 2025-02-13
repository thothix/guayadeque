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
#include "NewChannel.h"

#include "Config.h"
#include "Images.h"
#include "MainFrame.h"
#include "Settings.h"
#include "Utils.h"

#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/arrimpl.cpp>

namespace Guayadeque {

#define guDIGITALPODCAST_OPML       wxT( "http://www.digitalpodcast.com/opml/digitalpodcast.opml" )

WX_DEFINE_OBJARRAY( guNewPodcastItemArray )
WX_DEFINE_OBJARRAY( guNewPodcastChannelArray )


// -------------------------------------------------------------------------------- //
// guPodcastTreeCtrl
// -------------------------------------------------------------------------------- //
guPodcastTreeCtrl::guPodcastTreeCtrl( wxWindow * parent, guNewPodcastChannelArray * newpodcasts ) :
    wxTreeCtrl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_FULL_ROW_HIGHLIGHT|wxTR_SINGLE|wxSUNKEN_BORDER )
{
    m_NewPodcasts = newpodcasts;
    m_NewItemsCount = 0;

    m_RootId   = AddRoot( wxT( "Channels" ), -1, -1, NULL );

    SetIndent( 5 );

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPodcastTreeCtrl::~guPodcastTreeCtrl()
{
}

// -------------------------------------------------------------------------------- //
void guPodcastTreeCtrl::ReloadItems( void )
{
    DeleteChildren( m_RootId );

    int CountCh = m_NewPodcasts->Count();
    for( int IndexCh = 0; IndexCh < CountCh; IndexCh++ )
    {
        guNewPodcastCategory * NewPodcastChannel = &m_NewPodcasts->Item( IndexCh );

        wxTreeItemId LastItemId = AppendItem( m_RootId, NewPodcastChannel->m_Name, -1, -1, NULL );
        int CountIt = NewPodcastChannel->m_Items.Count();
        for( int IndexIt = 0; IndexIt < CountIt; IndexIt++ )
        {
            AppendItem( LastItemId, NewPodcastChannel->m_Items[ IndexIt ].m_Name, -1, -1,
                             new guNewPodcastItem( NewPodcastChannel->m_Items[ IndexIt ] ) );
        }
        m_NewItemsCount += CountIt;
    }
}

// -------------------------------------------------------------------------------- //
// guNewPodcastChannelSelector
// -------------------------------------------------------------------------------- //
guNewPodcastChannelSelector::guNewPodcastChannelSelector( wxWindow * parent ) :
    wxDialog( parent, wxID_ANY, _( "New Podcast Channel" ), wxDefaultPosition, wxSize( 550,410 ), wxDEFAULT_DIALOG_STYLE )
{
    wxStaticText * UrlLabel;
    wxStdDialogButtonSizer * m_StandardButtons;
    wxButton * m_StandardButtonsOK;
    wxButton * m_StandardButtonsCancel;

	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * DirectoryMainSizer;
	DirectoryMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* DirectoryStaticSizer;
	DirectoryStaticSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY,  wxString::Format(" %s ", _("Digital Podcast Directory"))), wxVERTICAL );

	wxBoxSizer* DirectoryTopSizer;
	DirectoryTopSizer = new wxBoxSizer( wxHORIZONTAL );

    //("Directory: 14 categories, 175 channels")
	m_DirectoryInfoStaticText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DirectoryInfoStaticText->Wrap( -1 );
	DirectoryTopSizer->Add( m_DirectoryInfoStaticText, 1, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxLEFT, 5 );

	m_FilterDirectory = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_FilterDirectory->SetToolTip( _( "Search in the podcast directory" ) );
	DirectoryTopSizer->Add( m_FilterDirectory, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	m_DirectoryReload = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_reload ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DirectoryReload->SetToolTip( _( "Refresh the podcast directory list" ) );
	DirectoryTopSizer->Add( m_DirectoryReload, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	DirectoryStaticSizer->Add( DirectoryTopSizer, 0, wxEXPAND, 5 );

	m_DirectoryTreeCtrl = new guPodcastTreeCtrl( this, &m_NewPodcasts );
	DirectoryStaticSizer->Add( m_DirectoryTreeCtrl, 1, wxEXPAND|wxALL, 5 );

	wxBoxSizer* UrlSizer;
	UrlSizer = new wxBoxSizer( wxHORIZONTAL );

	UrlLabel = new wxStaticText( this, wxID_ANY, wxT("Url:"), wxDefaultPosition, wxDefaultSize, 0 );
	UrlLabel->Wrap( -1 );
	UrlSizer->Add( UrlLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_UrlTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	UrlSizer->Add( m_UrlTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	DirectoryStaticSizer->Add( UrlSizer, 0, wxEXPAND, 5 );

	DirectoryMainSizer->Add( DirectoryStaticSizer, 1, wxEXPAND|wxALL, 5 );

	m_StandardButtons = new wxStdDialogButtonSizer();
	m_StandardButtonsOK = new wxButton( this, wxID_OK );
	m_StandardButtons->AddButton( m_StandardButtonsOK );
	m_StandardButtonsCancel = new wxButton( this, wxID_CANCEL );
	m_StandardButtons->AddButton( m_StandardButtonsCancel );
	m_StandardButtons->SetAffirmativeButton( m_StandardButtonsOK );
	m_StandardButtons->SetCancelButton( m_StandardButtonsCancel );
	m_StandardButtons->Realize();
	DirectoryMainSizer->Add( m_StandardButtons, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( DirectoryMainSizer );
	this->Layout();

	m_StandardButtonsOK->SetDefault();

	// Bind Events
    m_FilterDirectory->Bind( wxEVT_BUTTON, &guNewPodcastChannelSelector::OnFilterDirectoryClicked, this );
    m_DirectoryReload->Bind( wxEVT_BUTTON, &guNewPodcastChannelSelector::OnReloadDirectoryClicked, this );
    m_DirectoryTreeCtrl->Bind( wxEVT_TREE_ITEM_ACTIVATED, &guNewPodcastChannelSelector::OnDirectoryItemSelected, this );
    m_DirectoryTreeCtrl->Bind( wxEVT_TREE_SEL_CHANGED, &guNewPodcastChannelSelector::OnDirectoryItemChanged, this );


	//
	LoadPodcastDirectory();

	m_UrlTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
guNewPodcastChannelSelector::~guNewPodcastChannelSelector()
{
    // Unbind events
    m_FilterDirectory->Unbind( wxEVT_BUTTON, &guNewPodcastChannelSelector::OnFilterDirectoryClicked, this );
    m_DirectoryReload->Unbind( wxEVT_BUTTON, &guNewPodcastChannelSelector::OnReloadDirectoryClicked, this );
    m_DirectoryTreeCtrl->Unbind( wxEVT_TREE_ITEM_ACTIVATED, &guNewPodcastChannelSelector::OnDirectoryItemSelected, this );
    m_DirectoryTreeCtrl->Unbind( wxEVT_TREE_SEL_CHANGED, &guNewPodcastChannelSelector::OnDirectoryItemChanged, this );
}

// -------------------------------------------------------------------------------- //
wxString guNewPodcastChannelSelector::GetValue( void )
{
    return m_UrlTextCtrl->GetValue();
}

// -------------------------------------------------------------------------------- //
int guNewPodcastChannelSelector::ReadNewPodcastChannel( wxXmlNode * XmlNode, guNewPodcastCategory * podcastchannel )
{
    while( XmlNode && XmlNode->GetName() == wxT( "outline" ) )
    {
        guNewPodcastItem * NewPodcastItem = new guNewPodcastItem();
        if( !NewPodcastItem )
            return 0;
        XmlNode->GetAttribute( wxT( "text" ), &NewPodcastItem->m_Name );
        XmlNode->GetAttribute( wxT( "url" ), &NewPodcastItem->m_Url );
        int Count = m_Filters.Count();
        if( Count )
        {
            bool ItemFound = false;
            for( int Index = 0; Index < Count; Index++ )
            {
                if( NewPodcastItem->m_Name.Lower().Find( m_Filters[ Index ].Lower() ) != wxNOT_FOUND )
                {
                    //guLogMessage( wxT( "Found item %s" ), NewPodcastItem->m_Name.c_str() );
                    podcastchannel->m_Items.Add( NewPodcastItem );
                    ItemFound = true;
                    break;
                }
            }
            if( !ItemFound )
                delete NewPodcastItem;
        }
        else
        {
            podcastchannel->m_Items.Add( NewPodcastItem );
        }
        XmlNode = XmlNode->GetNext();
    }
    return podcastchannel->m_Items.Count();
}

// -------------------------------------------------------------------------------- //
int guNewPodcastChannelSelector::ReadNewPodcastChannels( wxXmlNode * XmlNode )
{
    guNewPodcastCategory * NewPodcastChannel = NULL;

    while( XmlNode && XmlNode->GetName() == wxT( "outline" ) )
    {
//        if( !NewPodcastChannel )
//            return 0;
        if( m_Filters.Count() )
        {
            if( !m_NewPodcasts.Count() )
            {
                NewPodcastChannel =  new guNewPodcastCategory();
                NewPodcastChannel->m_Name = _( "Filtered channels" );
                m_NewPodcasts.Add( NewPodcastChannel );
            }
        }
        else
        {
            NewPodcastChannel =  new guNewPodcastCategory();
            XmlNode->GetAttribute( wxT( "text" ), &NewPodcastChannel->m_Name );
        }

        if( !NewPodcastChannel )
            return 0;

        if( ReadNewPodcastChannel( XmlNode->GetChildren(), NewPodcastChannel ) )
        {
            if( !m_Filters.Count() )
                m_NewPodcasts.Add( NewPodcastChannel );
        }
        else
        {
            if( !m_Filters.Count() )
                delete NewPodcastChannel;
        }
        XmlNode = XmlNode->GetNext();
    }
    return m_NewPodcasts.Count();
}

// -------------------------------------------------------------------------------- //
int guNewPodcastChannelSelector::LoadNewPodcastsFromXml( const wxString &filename )
{
    wxFFileInputStream ins( filename );
    wxXmlDocument XmlDoc( ins );
    wxXmlNode * XmlNode = XmlDoc.GetRoot();
    if( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "opml" ) )
        {
            XmlNode = XmlNode->GetChildren();
            while( XmlNode )
            {
                if( XmlNode->GetName() == wxT( "body" ) )
                {
                    ReadNewPodcastChannels( XmlNode->GetChildren() );
                }
                XmlNode = XmlNode->GetNext();
            }
        }
    }
    return m_NewPodcasts.Count();
}


// -------------------------------------------------------------------------------- //
void guNewPodcastChannelSelector::LoadPodcastDirectory( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    wxString PodcastsPath = Config->ReadStr( CONFIG_KEY_PODCASTS_PATH,
                                             guPATH_PODCASTS,
                                             CONFIG_PATH_PODCASTS );

    wxSetCursor( * wxHOURGLASS_CURSOR );
    wxTheApp->Yield();

    if( wxFileExists( PodcastsPath + wxT( "/Podcast.Directory.xml" ) ) ||
        DownloadFile( guDIGITALPODCAST_OPML, PodcastsPath + wxT( "/Podcast.Directory.xml" ) ) )
    {
        m_NewPodcasts.Empty();

        if( LoadNewPodcastsFromXml( PodcastsPath + wxT( "/Podcast.Directory.xml" ) ) )
        {
            m_DirectoryTreeCtrl->ReloadItems();
            if( m_Filters.Count() )
            {
                m_DirectoryInfoStaticText->SetLabel( wxString::Format( _( "%u Channels" ),
                    m_DirectoryTreeCtrl->GetItemsCount() ) );
            }
            else
            {
                m_DirectoryInfoStaticText->SetLabel( wxString::Format( _( "%u Categories, %u Channels" ),
                    m_DirectoryTreeCtrl->GetCategoryCount(),
                    m_DirectoryTreeCtrl->GetItemsCount() ) );
            }
            //guLogMessage( wxT( "%s" ), m_DirectoryInfoStaticText->GetLabel().c_str() );
        }
    }
    else
        guLogWarning( wxT( "Could not download the Podcast Directory xml file" ) );

    if( m_Filters.Count() )
        m_DirectoryTreeCtrl->ExpandAll();
    else
        m_DirectoryTreeCtrl->ExpandRoot();

    wxSetCursor( wxNullCursor );
}

// -------------------------------------------------------------------------------- //
void guNewPodcastChannelSelector::OnDirectoryItemChanged( wxTreeEvent &event )
{
    guNewPodcastItem * ItemData = ( guNewPodcastItem * ) m_DirectoryTreeCtrl->GetItemData( event.GetItem() );
    if( ItemData )
        m_UrlTextCtrl->SetValue( ItemData->m_Url );
    else
        m_UrlTextCtrl->SetValue( wxEmptyString );
}

// -------------------------------------------------------------------------------- //
void guNewPodcastChannelSelector::OnDirectoryItemSelected( wxTreeEvent &event )
{
    guNewPodcastItem * ItemData = ( guNewPodcastItem * ) m_DirectoryTreeCtrl->GetItemData( event.GetItem() );
    if( ItemData )
    {
        m_UrlTextCtrl->SetValue( ItemData->m_Url );
        EndModal( wxID_OK );
    }
    else
        event.Skip();
}

// -------------------------------------------------------------------------------- //
void guNewPodcastChannelSelector::OnFilterDirectoryClicked( wxCommandEvent &event )
{
    wxString FilterText = wxEmptyString;
    int Count = m_Filters.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            FilterText += m_Filters[ Index ] + wxT( " " );
        }
        FilterText.RemoveLast();
    }
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog(guMainFrame::GetMainFrame(), wxString::Format("%s: ", _("Filter Text")), _("Enter the text to filter podcasts channels"), FilterText);
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        m_Filters = wxStringTokenize( EntryDialog->GetValue(), wxT( "\t\r\n " ) );
        //guLogMessage( wxT( "Filter Text : '%s' %u items" ), EntryDialog->GetValue().c_str(), m_Filters.Count() );
        LoadPodcastDirectory();
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guNewPodcastChannelSelector::OnReloadDirectoryClicked( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    wxString PodcastsPath = Config->ReadStr( CONFIG_KEY_PODCASTS_PATH,
                                             guPATH_PODCASTS,
                                             CONFIG_PATH_PODCASTS );

    wxRemoveFile( PodcastsPath + wxT( "/Podcast.Directory.xml" ) );

    LoadPodcastDirectory();
}

}

// -------------------------------------------------------------------------------- //
