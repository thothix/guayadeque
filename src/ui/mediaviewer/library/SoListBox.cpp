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
#include "SoListBox.h"

#include "Accelerators.h"
#include "Config.h"
#include "EventCommandIds.h"
#include "Images.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "Utils.h"
#include "RatingCtrl.h"
#include "MediaViewer.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guSoListBox - Media Library Songs/Tracks Panel
// -------------------------------------------------------------------------------- //
guSoListBox::guSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, long style ) :
    guListView( parent, style|wxLB_MULTIPLE|guLISTVIEW_ALLOWDRAG|guLISTVIEW_COLUMN_CLICK_EVENTS,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer->GetDb();
    m_ConfName = confname;
    m_ItemsFirst = wxNOT_FOUND;
    m_ItemsLast = wxNOT_FOUND;
    m_LastColumnRightClicked = wxNOT_FOUND;

    m_TracksOrder = Config->ReadNum( wxT( "TracksOrder" ), 0, confname );
    m_TracksOrderDesc = Config->ReadBool( wxT( "TracksOrderDesc" ), 0, confname );

    m_Db->SetTracksOrder( m_TracksOrder );
    m_Db->SetTracksOrderDesc( m_TracksOrderDesc );

    m_ColumnNames.Add( wxT( "#" ) );
    m_ColumnNames.Add( _( "Title" ) );
    m_ColumnNames.Add( _( "Artist" ) );
    m_ColumnNames.Add( _( "Album Artist" ) );
    m_ColumnNames.Add( _( "Album" ) );
    m_ColumnNames.Add( _( "Genre" ) );
    m_ColumnNames.Add( _( "Composer" ) );
    m_ColumnNames.Add( _( "Disk" ) );
    m_ColumnNames.Add( _( "Length" ) );
    m_ColumnNames.Add( _( "Year" ) );
    m_ColumnNames.Add( _( "Bit Rate" ) );
    m_ColumnNames.Add( _( "Rating" ) );
    m_ColumnNames.Add( _( "Plays" ) );
    m_ColumnNames.Add( _( "Last Played" ) );
    m_ColumnNames.Add( _( "Added" ) );
    m_ColumnNames.Add( _( "Format" ) );
    m_ColumnNames.Add( _( "Path" ) );
    m_ColumnNames.Add( _( "Offset" ) );

    int ColId;
    wxString ColName;
    int Count = m_ColumnNames.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "id%u" ), Index ), Index, m_ConfName + wxT( "/columns/ids" ) );

        ColName = m_ColumnNames[ ColId ];

        if( style & guLISTVIEW_COLUMN_SORTING )
            ColName += ( ( ColId == m_TracksOrder ) ? ( m_TracksOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "width%u" ), Index ), 80, m_ConfName + wxT( "/columns/widths" ) ),
            Config->ReadBool( wxString::Format( wxT( "show%u" ), Index ), true, m_ConfName + wxT( "/columns/shows" ) )
            );
        InsertColumn( Column );
    }

    m_NormalStar   = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_normal_tiny + GURATING_STYLE_MID ) ) );
    m_SelectStar = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_highlight_tiny + GURATING_STYLE_MID ) ) );

    Bind( wxEVT_MENU, &guSoListBox::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Bind( wxEVT_MENU, &guSoListBox::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
    Bind( guEVT_LISTBOX_ITEM_COL_CLICKED, &guSoListBox::OnItemColumnClicked, this );
    Bind( guEVT_LISTBOX_ITEM_COL_RCLICKED, &guSoListBox::OnItemColumnRClicked, this );

    Bind( guConfigUpdatedEvent, &guSoListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_MENU, &guSoListBox::OnCreateSmartPlaylist, this, ID_PLAYLIST_SMART_PLAYLIST );
    Bind( wxEVT_MENU, &guSoListBox::OnCreateBestOfPlaylist, this, ID_PLAYER_PLAYLIST_CREATE_BESTOF );

    CreateAcceleratorTable();

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guSoListBox::~guSoListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( wxT( "TracksOrder" ), m_TracksOrder, m_ConfName );
    Config->WriteBool( wxT( "TracksOrderDesc" ), m_TracksOrderDesc, m_ConfName );

    //int ColId;
    int count = guSONGS_COLUMN_COUNT;
    for( int index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "id%u" ), index ),
                          ( * m_Columns )[ index ].m_Id,
                          m_ConfName + wxT( "/columns/ids" ) );
        Config->WriteNum( wxString::Format( wxT( "width%u" ), index ),
                          ( * m_Columns )[ index ].m_Width,
                          m_ConfName + wxT( "/columns/widths" ) );
        Config->WriteBool( wxString::Format( wxT( "show%u" ), index ),
                           ( * m_Columns )[ index ].m_Enabled,
                           m_ConfName + wxT( "/columns/shows" ) );
    }

    if( m_NormalStar )
        delete m_NormalStar;
    if( m_SelectStar )
        delete m_SelectStar;

    Unbind( wxEVT_MENU, &guSoListBox::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guSoListBox::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
    Unbind( guEVT_LISTBOX_ITEM_COL_CLICKED, &guSoListBox::OnItemColumnClicked, this );
    Unbind( guEVT_LISTBOX_ITEM_COL_RCLICKED, &guSoListBox::OnItemColumnRClicked, this );

    Unbind( guConfigUpdatedEvent, &guSoListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Unbind( wxEVT_MENU, &guSoListBox::OnCreateSmartPlaylist, this, ID_PLAYLIST_SMART_PLAYLIST );
    Unbind( wxEVT_MENU, &guSoListBox::OnCreateBestOfPlaylist, this, ID_PLAYER_PLAYLIST_CREATE_BESTOF );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::CreateAcceleratorTable()
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_TRACKS_SAVETOPLAYLIST );
    RealAccelCmds.Add( ID_TRACKS_EDITLABELS );
    RealAccelCmds.Add( ID_TRACKS_EDITTRACKS );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
    RealAccelCmds.Add( ID_LIBRARY_SEARCH );

    if (guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable))
        SetAcceleratorTable( AccelTable );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::ReloadItems( bool reset )
{
    //guLogMessage( wxT( "guSoListBox::ReloadItems( %i )"), reset );
    wxArrayInt Selection;
    int FirstVisible = GetVisibleRowsBegin();

    if( reset )
        SetSelection( -1 );
    else
        Selection = GetSelectedIndexes( false );

    m_ItemsMutex.Lock();

    m_Items.Empty();
    m_ItemsFirst = -1;
    m_ItemsLast = -1;

    GetItemsList();
    //SetItemCount( m_Items.Count() );

    m_ItemsMutex.Unlock();

    if( !reset )
    {
        SetSelectedIndexes( Selection );
        ScrollToRow( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if (Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS)
        CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
wxString guSoListBox::OnGetItemText( const int row, const int col ) const
{
    guTrack * Track;
    //guLogMessage( wxT( "GetItemText( %i, %i )    %i-%i  %i" ), row, col, m_ItemsFirst, m_ItemsLast, m_Items.Count() );

    Track = &m_Items[row - m_ItemsFirst];

    switch ((* m_Columns)[col].m_Id)
    {
        case guSONGS_COLUMN_NUMBER :
        {
            if (Track->m_Number)
                return wxString::Format(wxT("%u"), Track->m_Number);
            return wxEmptyString;
        }

        case guSONGS_COLUMN_TITLE  :
            return Track->m_SongName;

        case guSONGS_COLUMN_ARTIST :
            return Track->m_ArtistName;

        case guSONGS_COLUMN_ALBUMARTIST :
            return Track->m_AlbumArtist;

        case guSONGS_COLUMN_ALBUM :
            return Track->m_AlbumName;

        case guSONGS_COLUMN_GENRE :
            return Track->m_GenreName;

        case guSONGS_COLUMN_COMPOSER :
            return Track->m_Composer;

        case guSONGS_COLUMN_DISK :
            return Track->m_Disk;

        case guSONGS_COLUMN_LENGTH :
            return LenToString( Track->m_Length );

        case guSONGS_COLUMN_YEAR :
        {
            if (Track->m_Year)
                return wxString::Format(wxT("%u"), Track->m_Year);
            return wxEmptyString;
        }

        case guSONGS_COLUMN_BITRATE :
            return wxString::Format(wxT("%u"), Track->m_Bitrate);

        case guSONGS_COLUMN_RATING :
            return wxString::Format(wxT("%i"), Track->m_Rating);

        case guSONGS_COLUMN_PLAYCOUNT :
        {
            if (Track->m_PlayCount)
                return wxString::Format(wxT("%u"), Track->m_PlayCount);
            break;
        }

        case guSONGS_COLUMN_LASTPLAY :
        {
            if (Track->m_LastPlay)
            {
                wxDateTime LastPlay;
                LastPlay.Set((time_t) Track->m_LastPlay);
                return LastPlay.FormatDate();
            }
            return _("Never");
        }

        case guSONGS_COLUMN_ADDEDDATE :
        {
            wxDateTime AddedDate;
            AddedDate.Set((time_t) Track->m_AddedTime);
            return AddedDate.FormatDate();
        }

        case guSONGS_COLUMN_FORMAT :
            return Track->m_Format;

        case guSONGS_COLUMN_FILEPATH :
            return Track->m_FileName;   // contains both path and filename

        case guSONGS_COLUMN_OFFSET :
        {
            if (Track->m_Offset)
                return LenToString(Track->m_Offset);
            return wxEmptyString;
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guSoListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( ( * m_Columns )[ col ].m_Id == guSONGS_COLUMN_RATING )
    {
        dc.SetBackgroundMode( wxTRANSPARENT );
        int x;
        int w = ( ( GURATING_STYLE_MID * 2 ) + GURATING_IMAGE_SIZE );

        for( x = 0; x < 5; x++ )
        {
           dc.DrawBitmap( ( x >= m_Items[ row - m_ItemsFirst ].m_Rating ) ? * m_NormalStar : * m_SelectStar,
                          rect.x + 3 + ( w * x ), rect.y + 3, true );
        }
    }
    else
        guListView::DrawItem( dc, rect, row, col );
}


// -------------------------------------------------------------------------------- //
void guSoListBox::ItemsCheckRange( const int start, const int end )
{
    //guLogMessage( wxT( "guSoListBox::ItemsCheckRange( %i, %i )" ), start, end );
    if( m_ItemsFirst != start || m_ItemsLast != end )
    {
        m_Items.Empty();
        m_Db->GetSongs( &m_Items, start, end );
        m_ItemsFirst = start;
        m_ItemsLast = end;
    }
    //guLogMessage( wxT( "Updated the items... %i" ), m_Items );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::GetItemsList()
{
    //m_Db->GetSongs( &m_Items, GetVisibleRowsBegin(), GetVisibleRowsEnd() );
    SetItemCount( m_Db->GetSongsCount() );

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
int guSoListBox::GetSelectedSongs( guTrackArray * tracks, const bool isdrag ) const
{
    unsigned long cookie;
    guSoListBox * self = wxConstCast( this, guSoListBox );

    self->m_ItemsMutex.Lock();
    int item = GetFirstSelected( cookie );
    while( item != wxNOT_FOUND )
    {
        guTrack * Track = new guTrack();
        if( m_Db->GetSong( GetItemId( item ), Track ) )
            tracks->Add( Track );
        else
            delete Track;

        item = GetNextSelected( cookie );
    }
    self->m_ItemsMutex.Unlock();

    m_MediaViewer->NormalizeTracks( tracks, isdrag );

    guLogMessage( wxT( "SOListBox::GetSelectedTracks <- %li" ), tracks->Count() );
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
void guSoListBox::GetAllSongs( guTrackArray * tracks )
{
    wxMutexLocker Lock(m_ItemsMutex);

    int count = GetItemCount();
    for( int index = 0; index < count; index++ )
    {
        guTrack * Track = new guTrack();
        if( m_Db->GetSong( GetItemId( index ), Track ) )
            tracks->Add( Track );
        else
            delete Track;
    }
}

// -------------------------------------------------------------------------------- //
wxArrayString guSoListBox::GetColumnNames() const
{
    return m_ColumnNames;
}

// -------------------------------------------------------------------------------- //
void AddSongsCommands( wxMenu * Menu, int SelCount )
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
                if( ( ( Commands[ index ].Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND ) ||
                      ( Commands[ index ].Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND ) )
                    && ( SelCount != 1 ) )
                {
                    continue;
                }
                MenuItem = new wxMenuItem( Menu, ID_COMMANDS_BASE + index, _( Names[ index ] ), _( Commands[ index ] ) );
                SubMenu->Append( MenuItem );
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
void guSoListBox::AppendFastEditMenu( wxMenu * menu, const int selcount ) const
{
    wxMenuItem * MenuItem;
    //guLogMessage( wxT( "guSoLisBox::AppendFastEditMenu %i - %i" ), m_LastColumnRightClicked, m_LastRowRightClicked );
    if (m_LastColumnRightClicked == wxNOT_FOUND || (m_LastColumnRightClicked >= (int) m_Columns->Count()))
        return;

    int ColumnId = GetColumnId(m_LastColumnRightClicked);

    // If its a column not editable
    if (ColumnId == guSONGS_COLUMN_RATING ||
        ColumnId == guSONGS_COLUMN_OFFSET ||
        ColumnId == guSONGS_COLUMN_LENGTH ||
        ColumnId == guSONGS_COLUMN_BITRATE ||
        ColumnId == guSONGS_COLUMN_LASTPLAY ||
        ColumnId == guSONGS_COLUMN_ADDEDDATE ||
        ColumnId == guSONGS_COLUMN_FORMAT ||
        ColumnId == guSONGS_COLUMN_FILEPATH)
        return;

    wxString MenuText = _("Edit");
    MenuText += wxT(" ") + m_ColumnNames[ColumnId].Lower();

    MenuItem = new wxMenuItem(menu, ID_TRACKS_EDIT_COLUMN,  _(MenuText), _("Edit the clicked column for the selected tracks"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_edit));
    menu->Append(MenuItem);

    if (selcount > 1)
    {
        MenuText = _("Set");
        MenuText += wxT(" ") + m_ColumnNames[ColumnId] + wxT(" ");
        MenuText += _("to");
        wxString ItemText = OnGetItemText(m_LastRowRightClicked, m_LastColumnRightClicked);
        ItemText.Replace(wxT("&"), wxT("&&"));
        MenuText += wxT(" '") + ItemText + wxT("'");

        MenuItem = new wxMenuItem(menu, ID_TRACKS_SET_COLUMN,  _(MenuText), _("Set the clicked column for the selected tracks"));
        MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_edit));
        menu->Append(MenuItem);
    }
}

// -------------------------------------------------------------------------------- //
void guSoListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedCount();
    int ContextMenuFlags = m_MediaViewer->GetContextMenuFlags();

    MenuItem = new wxMenuItem( Menu, ID_TRACKS_PLAY,
                            wxString( _( "Play" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                            _( "Play current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( Menu, ID_TRACKS_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                            _( "Add current selected songs to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_TRACKS_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_TRACKS_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_TRACKS_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );
    MenuItem->Enable( SelCount );

    Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

    if (SelCount)
    {
        if (SelCount == 1)
        {
            Menu->AppendSeparator();

            wxMenu *SubMenu;
            SubMenu = new wxMenu();

            MenuItem = new wxMenuItem(Menu, ID_TRACKS_BROWSE_GENRE, _("Genre"),
                                      _("Selects the genre of the current song"));
            SubMenu->Append(MenuItem);

            MenuItem = new wxMenuItem(Menu, ID_TRACKS_BROWSE_ARTIST, _("Artist"),
                                      _("Selects the artist of the current song"));
            SubMenu->Append(MenuItem);

            MenuItem = new wxMenuItem(Menu, ID_TRACKS_BROWSE_ALBUMARTIST, _("Album Artist"),
                                      _("Selects the album artist of the current song"));
            SubMenu->Append(MenuItem);

            MenuItem = new wxMenuItem(Menu, ID_TRACKS_BROWSE_COMPOSER, _("Composer"),
                                      _("Selects the composer of the current song"));
            SubMenu->Append(MenuItem);

            MenuItem = new wxMenuItem(Menu, ID_TRACKS_BROWSE_ALBUM, _("Album"),
                                      _("Select the album of the current song"));
            SubMenu->Append(MenuItem);

            Menu->AppendSubMenu(SubMenu, _("Select"), _("Search in the library"));
        }

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_TRACKS_EDITLABELS,
                                wxString( _( "Edit Labels" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITLABELS ),
                                _( "Edit the labels assigned to the selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
        Menu->Append( MenuItem );

        if( ContextMenuFlags & guCONTEXTMENU_EDIT_TRACKS )
        {
            MenuItem = new wxMenuItem( Menu, ID_TRACKS_EDITTRACKS,
                                wxString( _( "Edit Songs" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                                _( "Edit the songs selected" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu->Append( MenuItem );

            if ( SelCount == 1 )
                AppendFastEditMenu( Menu, SelCount );

            wxMenu * RatingMenu = new wxMenu();

            MenuItem = new wxMenuItem( RatingMenu, ID_TRACKS_SET_RATING_0,
                                    wxT( "☆☆☆☆☆" ),
                                    _( "Set the rating to 0" ), wxITEM_NORMAL );
            RatingMenu->Append( MenuItem );

            MenuItem = new wxMenuItem( RatingMenu, ID_TRACKS_SET_RATING_1,
                                    wxT( "★☆☆☆☆" ),
                                    _( "Set the rating to 1" ), wxITEM_NORMAL );
            RatingMenu->Append( MenuItem );

            MenuItem = new wxMenuItem( RatingMenu, ID_TRACKS_SET_RATING_2,
                                    wxT( "★★☆☆☆" ),
                                    _( "Set the rating to 2" ), wxITEM_NORMAL );
            RatingMenu->Append( MenuItem );

            MenuItem = new wxMenuItem( RatingMenu, ID_TRACKS_SET_RATING_3,
                                    wxT( "★★★☆☆" ),
                                    _( "Set the rating to 3" ), wxITEM_NORMAL );
            RatingMenu->Append( MenuItem );

            MenuItem = new wxMenuItem( RatingMenu, ID_TRACKS_SET_RATING_4,
                                    wxT( "★★★★☆" ),
                                    _( "Set the rating to 4" ), wxITEM_NORMAL );
            RatingMenu->Append( MenuItem );

            MenuItem = new wxMenuItem( RatingMenu, ID_TRACKS_SET_RATING_5,
                                    wxT( "★★★★★" ),
                                    _( "Set the rating to 5" ), wxITEM_NORMAL );
            RatingMenu->Append( MenuItem );

            Menu->AppendSubMenu( RatingMenu, _( "Set Rating" ), _( "Set the current selected tracks rating" ) );
        }

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_TRACKS_SAVETOPLAYLIST,
                                wxString( _( "Save to Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                                _( "Save all selected tracks as a playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        if( SelCount == 1 )
        {
            MenuItem = new wxMenuItem( Menu, ID_PLAYLIST_SMART_PLAYLIST, _( "Create Smart Playlist" ), _( "Create a smart playlist from this track" ) );
            Menu->Append( MenuItem );
            MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_CREATE_BESTOF, _( "Create Best of Playlist" ), _( "Create a playlist with the best of this artist" ) );
            Menu->Append( MenuItem );
        }

        Menu->AppendSeparator();

        if( ContextMenuFlags & guCONTEXTMENU_DELETEFROMLIBRARY )
        {
            MenuItem = new wxMenuItem( Menu, ID_TRACKS_DELETE_LIBRARY, _( "Remove from Library" ), _( "Remove the current selected tracks from library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
            Menu->Append( MenuItem );
        }

        MenuItem = new wxMenuItem( Menu, ID_TRACKS_DELETE_DRIVE, _( "Delete from Drive" ), _( "Remove the current selected tracks from drive" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu->Append( MenuItem );

        if( ( ContextMenuFlags & guCONTEXTMENU_COPY_TO ) ||
            ( ContextMenuFlags & guCONTEXTMENU_LINKS ) ||
            ( ContextMenuFlags & guCONTEXTMENU_COMMANDS ) )
        {
            Menu->AppendSeparator();

            if( ContextMenuFlags & guCONTEXTMENU_COPY_TO )
                m_MediaViewer->CreateCopyToMenu( Menu );

            if( SelCount == 1 && ( ContextMenuFlags & guCONTEXTMENU_LINKS ) )
                AddOnlineLinksMenu( Menu );

            if( ContextMenuFlags & guCONTEXTMENU_COMMANDS )
                AddSongsCommands( Menu, SelCount );
        }
    }

    m_MediaViewer->CreateContextMenu( Menu, guLIBRARY_ELEMENT_TRACKS );
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnSearchLinkClicked( wxCommandEvent &event )
{
    unsigned long cookie;
    int Item = GetFirstSelected(cookie);
    if (Item != wxNOT_FOUND)
        ExecuteOnlineLink(event.GetId(), GetSearchText(Item));
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnCommandClicked( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
            wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
            wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);

            int CommandIndex = event.GetId() - ID_COMMANDS_BASE;
            wxString CurCmd = Commands[ CommandIndex ];

            if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                GetSelectedSongs( &Songs );
                wxArrayInt AlbumList;
                AlbumList.Add( Songs[ 0 ].m_AlbumId );
                wxArrayString AlbumPaths = m_Db->GetAlbumsPaths( AlbumList );
                wxString Paths = wxEmptyString;
                int count = AlbumPaths.Count();
                for( int index = 0; index < count; index++ )
                {
                    AlbumPaths[ index ].Replace( wxT( " " ), wxT( "\\ " ) );
                    Paths += wxT( " " ) + AlbumPaths[ index ];
                }
                CurCmd.Replace( guCOMMAND_ALBUMPATH, Paths.Trim( false ) );
            }

            if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                GetSelectedSongs( &Songs );
                wxArrayInt AlbumList;
                AlbumList.Add( Songs[ 0 ].m_AlbumId );
                int CoverId = m_Db->GetAlbumCoverId( AlbumList[ 0 ] );
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                    CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
                CurCmd.Replace( guCOMMAND_COVERPATH, CoverPath );
            }

            if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                GetSelectedSongs( &Songs );
                wxString SongList = wxEmptyString;
                int count = Songs.Count();
                if( count )
                {
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
wxString guSoListBox::GetSearchText( int item ) const
{
    if( item >= m_ItemsFirst && item < m_ItemsLast )
    {
        return wxString::Format( wxT( "\"%s\" \"%s\"" ),
            m_Items[ item - m_ItemsFirst ].m_ArtistName.c_str(),
            m_Items[ item - m_ItemsFirst ].m_SongName.c_str() );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxString guSoListBox::GetItemName( const int row ) const
{
    if (row >= m_ItemsFirst && row < m_ItemsLast)
        return m_Items[row - m_ItemsFirst].m_SongName;
    return m_Db->GetSongsName(row);
}

// -------------------------------------------------------------------------------- //
int guSoListBox::GetItemId( const int row ) const
{
    if (row >= m_ItemsFirst && row < m_ItemsLast)
        return m_Items[row - m_ItemsFirst].m_SongId;
    return m_Db->GetSongsId(row);
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnItemColumnClicked( wxListEvent &event )
{
    int ColId = GetColumnId( event.m_col );
    if( ColId == guSONGS_COLUMN_RATING )
    {
        //guLogMessage( wxT( "The rating have been clicked... %i" ), event.GetPoint().x );
        int Row = event.GetInt();
        Row -= m_ItemsFirst;
        if( ( Row < 0 ) || ( Row >= ( int ) m_Items.Count() ) )
            return;

        int w = ( ( GURATING_STYLE_MID * 2 ) + GURATING_IMAGE_SIZE );
        int MouseX = event.GetPoint().x;
        int Rating;

        if( MouseX < 3 )
            Rating = 0;
        else
            Rating = wxMin( 5, ( wxMax( 0, MouseX - 3 ) / w ) + 1 );

        if( m_Items[ Row ].m_Rating == Rating )
            Rating = 0;

        wxCommandEvent RatingEvent( wxEVT_MENU, ID_TRACKS_SET_RATING_0 + Rating );
        AddPendingEvent( RatingEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnItemColumnRClicked( wxListEvent &event )
{
    m_LastColumnRightClicked = event.m_col;
    m_LastRowRightClicked = event.GetInt();
    //guLogMessage( wxT( "Column %i Row %i Right Clicked..." ), m_LastColumnRightClicked, event.GetInt() );
}

// -------------------------------------------------------------------------------- //
wxVariant guSoListBox::GetLastDataClicked()
{
    guTrack * Track = &m_Items[ m_LastRowRightClicked - m_ItemsFirst ];

    int ColId = GetColumnId( m_LastColumnRightClicked );
    switch (ColId)
    {
        case guSONGS_COLUMN_NUMBER :
            return wxVariant( ( long ) Track->m_Number );

        case guSONGS_COLUMN_TITLE :
            return wxVariant( Track->m_SongName );

        case guSONGS_COLUMN_ARTIST :
            return wxVariant( Track->m_ArtistName );

        case guSONGS_COLUMN_ALBUMARTIST :
            return wxVariant( Track->m_AlbumArtist );

        case guSONGS_COLUMN_ALBUM :
            return wxVariant( Track->m_AlbumName );

        case guSONGS_COLUMN_GENRE :
            return wxVariant( Track->m_GenreName );

        case guSONGS_COLUMN_COMPOSER :
            return wxVariant( Track->m_Composer );

        case guSONGS_COLUMN_DISK :
            return wxVariant( Track->m_Disk );

        case guSONGS_COLUMN_YEAR :
            return wxVariant( ( long ) Track->m_Year );
    }

    return wxVariant();
}

// -------------------------------------------------------------------------------- //
void guSoListBox::UpdatedTracks( const guTrackArray * tracks )
{
    int ItemCount = m_Items.Count();
    bool ClearSelection = false;

    if( !ItemCount )
        return;

    int TrackIndex;
    int TrackCount = tracks->Count();
    for ( TrackIndex = 0; TrackIndex < TrackCount; TrackIndex++ )
    {
        guTrack &CurTrack = tracks->Item( TrackIndex );

        for ( int ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
        {
            if (((CurTrack.m_SongId == m_Items[ItemIndex].m_SongId) &&
                 (CurTrack.m_Type == guTRACK_TYPE_MAGNATUNE)) ||
                 (CurTrack.m_FileName == m_Items[ItemIndex].m_FileName))
            {
                m_Items[ ItemIndex ] = CurTrack;
                RefreshRow( ItemIndex + m_ItemsFirst );
                if( !ClearSelection )
                    ClearSelection = true;
            }
        }
    }
    if( ClearSelection )
        ClearSelectedItems();
}

// -------------------------------------------------------------------------------- //
void guSoListBox::UpdatedTrack( const guTrack * track )
{
    int ItemIndex;
    int ItemCount = m_Items.Count();

    if( !ItemCount )
        return;

    for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
    {
        if( track->m_FileName == m_Items[ ItemIndex ].m_FileName )
        {
            m_Items[ ItemIndex ] = * track;
            RefreshRow( ItemIndex );
        }
    }
}

// -------------------------------------------------------------------------------- //
int guSoListBox::FindItem( const int trackid )
{
    int Count = m_Items.Count();
    for (int Index = 0; Index < Count; Index++)
    {
        if (m_Items[Index].m_SongId == trackid)
            return Index;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guSoListBox::SetTracksOrder( const int order )
{
    if( m_TracksOrder != order )
    {
        m_TracksOrder = order;
        m_Db->SetTracksOrder( m_TracksOrder );
    }
    else
    {
        m_TracksOrderDesc = !m_TracksOrderDesc;
        m_Db->SetTracksOrderDesc( m_TracksOrderDesc );
    }

    int ColId = m_TracksOrder;

    // Create the Columns
    int CurColId;
    int Count = m_ColumnNames.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        CurColId = GetColumnId( Index );
        SetColumnLabel( Index,
            m_ColumnNames[ CurColId ]  + ( ( ColId == CurColId ) ? ( m_TracksOrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnCreateSmartPlaylist( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedSongs(&Tracks);
    if (Tracks.Count() == 1)
        m_MediaViewer->CreateSmartPlaylist(Tracks[0].m_ArtistName, Tracks[0].m_SongName);
}

// -------------------------------------------------------------------------------- //
void guSoListBox::OnCreateBestOfPlaylist( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedSongs( &Tracks );
    if( !Tracks.IsEmpty() )
    {
        const guTrack &Track = Tracks[ 0 ];
        if( Track.m_MediaViewer )
            m_MediaViewer->CreateBestOfPlaylist( Track );
    }
}

}
