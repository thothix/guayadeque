/*
   Copyright (C) 2008-2023 J.Rios <anonbeat@gmail.com>
   Copyright (C) 2024 Tiago T Barrionuevo <thothix@protonmail.com>

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
#include "AlListBox.h"

#include "Accelerators.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "Images.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "MediaViewer.h"
#include "OnlineLinks.h"
#include "Utils.h"
#include "Settings.h"
#include "LibPanel.h"

#define ALLISTBOX_ITEM_SIZE  40

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guAlListBox - Albums panel
// -------------------------------------------------------------------------------- //
guAlListBox::guAlListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label ) :
    guListView( parent, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG | guLISTVIEW_HIDE_HEADER )
{
    m_Db = db;
    m_Items = new guAlbumItems();
    m_LibPanel = libpanel;
    m_SysFontPointSize = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT ).GetPointSize();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_ConfigPath = libpanel->GetMediaViewer()->ConfigPath();
    m_AlbumsOrder = Config->ReadNum( wxT( "AlbumsOrder" ), 0, m_ConfigPath );
    m_Db->SetAlbumsOrder( m_AlbumsOrder );

    guListViewColumn * Column = new guListViewColumn( label, 0 );
    InsertColumn( Column );

    Bind( wxEVT_MENU, &guAlListBox::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Bind( wxEVT_MENU, &guAlListBox::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
    Bind( wxEVT_MENU, &guAlListBox::OnOrderSelected, this, ID_ALBUM_ORDER_NAME, ID_ALBUM_ORDER_ARTIST_YEAR_REVERSE );

    Bind( guConfigUpdatedEvent, &guAlListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guAlListBox::~guAlListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( wxT( "AlbumsOrder" ), m_AlbumsOrder, m_ConfigPath );

    if( m_Items )
        delete m_Items;

    Unbind( wxEVT_MENU, &guAlListBox::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guAlListBox::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guAlListBox::OnOrderSelected, this, ID_ALBUM_ORDER_NAME, ID_ALBUM_ORDER_ARTIST_YEAR_REVERSE );

    Unbind( guConfigUpdatedEvent, &guAlListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_ALBUM_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_ALBUM_EDITLABELS );
    RealAccelCmds.Add( ID_ALBUM_EDITTRACKS );
    RealAccelCmds.Add( ID_ALBUM_PLAY );
    RealAccelCmds.Add( ID_ALBUM_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_ALBUM_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_ALBUM_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_ALBUM_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_LIBRARY_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
bool guAlListBox::SelectAlbumName( const wxString &AlbumName )
{
    long item = FindItem( 0, AlbumName, false );
    if( item != wxNOT_FOUND )
    {
        wxArrayInt * Albums = new wxArrayInt();
        Albums->Add( ( * m_Items )[ item ].m_Id );

        wxCommandEvent event( wxEVT_MENU, ID_ALBUM_SETSELECTION );
        event.SetClientData( ( void * ) Albums );
        wxPostEvent( guMainFrame::GetMainFrame(), event );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guAlListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    m_Attr.m_Font->SetPointSize( m_SysFontPointSize );

    guAlbumItem * Item = &( * ( guAlbumItems * ) m_Items )[ row ];

    dc.SetFont( * m_Attr.m_Font );
    dc.SetBackgroundMode( wxTRANSPARENT );

    dc.SetTextForeground( IsSelected( row ) ? m_Attr.m_SelFgColor : m_Attr.m_TextFgColor );

    dc.DrawText( Item->m_Name, rect.x + 45, rect.y + 4 );

    if( Item->m_Year )
    {
        int Pos;
        dc.GetTextExtent( Item->m_Name, &Pos, NULL );
        Pos += rect.x + 45;
        m_Attr.m_Font->SetPointSize( m_SysFontPointSize - 3 );
        dc.SetFont( * m_Attr.m_Font );
        dc.DrawText( wxString::Format( wxT( " (%4u)" ), Item->m_Year ), Pos, rect.y + 7 );
    }


    if( !Item->m_ArtistName.IsEmpty() )
    {
        m_Attr.m_Font->SetPointSize( m_SysFontPointSize - 2 );
        dc.SetFont( * m_Attr.m_Font );
        dc.DrawText( _( "by " ) + Item->m_ArtistName, rect.x + 45, rect.y + 22 );
    }

    if( Item->m_Thumb )
    {
        dc.DrawBitmap( * Item->m_Thumb, rect.x + 1, rect.y + 1, false );
    }
//    else if( Item->m_Thumb )
//    {
//        guLogError( wxT( "Thumb image corrupt or not correctly loaded" ) );
//    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnOrderSelected( wxCommandEvent &event )
{
    m_AlbumsOrder = event.GetId() - ID_ALBUM_ORDER_NAME;
    m_Db->SetAlbumsOrder( m_AlbumsOrder );

    ReloadItems( false );
}

// -------------------------------------------------------------------------------- //
wxCoord guAlListBox::OnMeasureItem( size_t n ) const
{
    // Code taken from the generic/listctrl.cpp file
    guAlListBox * self = wxConstCast( this, guAlListBox );

    self->SetItemHeight( ALLISTBOX_ITEM_SIZE );
    return wxCoord( ALLISTBOX_ITEM_SIZE );
}

// -------------------------------------------------------------------------------- //
int guAlListBox::GetSelectedSongs( guTrackArray * tracks, const bool isdrag ) const
{
    int Count = m_Db->GetAlbumsSongs( GetSelectedItems(), tracks );
    m_LibPanel->NormalizeTracks( tracks, isdrag );
    return Count;
}

// -------------------------------------------------------------------------------- //
void AddAlbumCommands( wxMenu * Menu, int SelCount )
{
    wxMenu * SubMenu;
    wxMenuItem * MenuItem;
    if( Menu )
    {
        SubMenu = new wxMenu();

        guConfig * Config = (guConfig *) guConfig::Get();

        wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
        wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
        wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);
        wxString category_names = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_NAMES, current_desktop);
        wxArrayString Names = Config->ReadAStr(CONFIG_KEY_COMMANDS_NAME, wxEmptyString, category_names);
        int count = Commands.Count();
        if( count )
        {
            for( int index = 0; index < count; index++ )
            {
                if( ( Commands[ index ].Find( guCOMMAND_COVERPATH ) == wxNOT_FOUND ) || ( SelCount == 1 ) )
                {
                    MenuItem = new wxMenuItem( Menu, ID_COMMANDS_BASE + index, _( Names[ index ] ), _( Commands[ index ] ) );
                    SubMenu->Append( MenuItem );
                }
            }

            SubMenu->AppendSeparator();
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, ID_MENU_PREFERENCES_COMMANDS, _( "Preferences" ), _( "Add commands in preferences" ) );
            SubMenu->Append( MenuItem );
        }
        Menu->AppendSubMenu( SubMenu, _( "Commands" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int ContextMenuFlags = m_LibPanel->GetContextMenuFlags();

    int SelCount = GetSelectedCount();
    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_ALBUM_PLAY,
                                wxString( _( "Play" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                                _( "Play current selected albums" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_ENQUEUE_AFTER_ALL,
                                wxString( _( "Enqueue" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                                _( "Add current selected albums to the Playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected albums to the Playlist as Next Tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected albums to the Playlist as Next Tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected albums to the Playlist as Next Tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu, _( "Add the selected albums after" ) );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_EDITLABELS,
                                wxString( _( "Edit Labels" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITLABELS ),
                                _( "Edit the labels assigned to the selected albums" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
        Menu->Append( MenuItem );

        if( ContextMenuFlags & guCONTEXTMENU_EDIT_TRACKS )
        {
            MenuItem = new wxMenuItem( Menu, ID_ALBUM_EDITTRACKS,
                                wxString( _( "Edit Songs" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                                _( "Edit the selected songs" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu->Append( MenuItem );
        }

        Menu->AppendSeparator();
    }

    wxMenu * SubMenu = new wxMenu();

    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_NAME, _( "Name" ), _( "Albums are sorted by name" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_YEAR, _( "Year" ), _( "Albums are sorted by year" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_YEAR_REVERSE, _( "Year Descending" ), _( "Albums are sorted by year descending" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_ARTIST_NAME, _( "Artist, Name" ), _( "Albums are sorted by artist and album name" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_ARTIST_YEAR, _( "Artist, Year" ), _( "Albums are sorted by artist and year" ) );
    SubMenu->AppendRadioItem( ID_ALBUM_ORDER_ARTIST_YEAR_REVERSE, _( "Artist, Year Descending" ), _( "Albums are sorted by artist and year descending" ) );

    MenuItem = SubMenu->FindItemByPosition( m_Db->GetAlbumsOrder() );
    MenuItem->Check( true );

    Menu->Append( wxID_ANY, _( "Sort By" ), SubMenu, _( "Sets the albums order" ) );

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_SAVETOPLAYLIST,
                                wxString( _( "Save to Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                                _( "Save the selected tracks to Playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_CREATE_BESTOF_PLAYLIST,
                                wxString( _( "Create Best of Playlist" ) ),
                                _( "Create a playlist with the best of this artist" ) );
        Menu->Append( MenuItem );

        if( SelCount == 1 && ( ContextMenuFlags & guCONTEXTMENU_DOWNLOAD_COVERS ) )
        {
            Menu->AppendSeparator();

            MenuItem = new wxMenuItem( Menu, ID_ALBUM_MANUALCOVER, _( "Download Cover" ), _( "Download cover for the current selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu->Append( MenuItem );

            MenuItem = new wxMenuItem( Menu, ID_ALBUM_SELECT_COVER, _( "Select Cover" ), _( "Select the cover image file from disk" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu->Append( MenuItem );

            MenuItem = new wxMenuItem( Menu, ID_ALBUM_COVER_DELETE, _( "Delete Cover" ), _( "Delete the cover for the selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
            Menu->Append( MenuItem );
        }

        if( ContextMenuFlags & guCONTEXTMENU_EMBED_COVERS )
        {
            MenuItem = new wxMenuItem( Menu, ID_ALBUM_COVER_EMBED, _( "Embed Cover" ), _( "Embed the current cover to the album files" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
            Menu->Append( MenuItem );
        }

        if( ( ContextMenuFlags & guCONTEXTMENU_COPY_TO ) ||
            ( ContextMenuFlags & guCONTEXTMENU_LINKS ) ||
            ( ContextMenuFlags & guCONTEXTMENU_COMMANDS ) )
        {
            Menu->AppendSeparator();

            if( ( m_LibPanel->GetContextMenuFlags() & guCONTEXTMENU_COPY_TO ) )
            {
                m_LibPanel->CreateCopyToMenu( Menu );
            }

            if( SelCount == 1 && ( ContextMenuFlags & guCONTEXTMENU_LINKS ) )
            {
                AddOnlineLinksMenu( Menu );
            }

            if( ContextMenuFlags & guCONTEXTMENU_COMMANDS )
                AddAlbumCommands( Menu, SelCount );
        }
    }

    m_LibPanel->CreateContextMenu( Menu, guLIBRARY_ELEMENT_ALBUMS );
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    unsigned long cookie;
    int Item = GetFirstSelected( cookie );
    if( Item != wxNOT_FOUND )
    {
        ExecuteOnlineLink( event.GetId(), GetSearchText( Item ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlListBox::OnCommandClicked( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        guConfig * Config = (guConfig *) guConfig::Get();
        if( Config )
        {
            wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
            wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
            wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);

            //guLogMessage( wxT( "CommandId: %u" ), index );
            int CommandIndex = event.GetId() - ID_COMMANDS_BASE;
            wxString CurCmd = Commands[ CommandIndex ];

            if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
            {
                wxArrayString AlbumPaths = m_Db->GetAlbumsPaths( Selection );
                wxString Paths = wxEmptyString;
                int count = AlbumPaths.Count();
                for( int index = 0; index < count; index++ )
                {
                    //Paths += wxT( " \"" ) + AlbumPaths[ index ] + wxT( "\"" );
                    AlbumPaths[ index ].Replace( wxT( " " ), wxT( "\\ " )  );
                    Paths += wxT( " " ) + AlbumPaths[ index ];
                }
                CurCmd.Replace( guCOMMAND_ALBUMPATH, Paths.Trim( false ) );
            }

            if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
            {
                int CoverId = m_Db->GetAlbumCoverId( Selection[ 0 ] );
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                {
                    CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
                }
                CurCmd.Replace( guCOMMAND_COVERPATH, CoverPath );
            }

            if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                wxString SongList = wxEmptyString;
                if( m_Db->GetAlbumsSongs( Selection, &Songs ) )
                {
                    int count = Songs.Count();
                    for( int index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + Songs[ index ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
                }
            }

            //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
            guExecute( CurCmd );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guAlListBox::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Db->GetArtistName( ( * m_Items )[ item ].m_ArtistId ).c_str(),
        ( * m_Items )[ item ].m_Name.c_str() );
}

// -------------------------------------------------------------------------------- //
void guAlListBox::ReloadItems( bool reset )
{
    wxArrayInt Selection;
    int FirstVisible = GetVisibleRowsBegin();

    if( reset )
        SetSelection( -1 );
    else
        Selection = GetSelectedItems( false );

    m_Items->Empty();

    GetItemsList();
    m_Items->Insert( new guAlbumItem( 0, wxString::Format( wxT( "%s (%lu)" ), _( "All" ), m_Items->Count() ) ), 0 );
    SetItemCount( m_Items->Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToRow( FirstVisible );
    }
    RefreshAll();
}

//// -------------------------------------------------------------------------------- //
//void  guAlListBox::SetSelectedItems( const wxArrayInt &selection )
//{
//    guListView::SetSelectedItems( selection );
//
////    wxCommandEvent event( wxEVT_LISTBOX, GetId() );
////    event.SetEventObject( this );
////    event.SetInt( -1 );
////    (void) GetEventHandler()->ProcessEvent( event );
//}

// -------------------------------------------------------------------------------- //
int guAlListBox::FindAlbum( const int albumid )
{
    int Count = m_Items->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( m_Items->Item( Index ).m_Id == albumid )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

}

// -------------------------------------------------------------------------------- //
