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
#include "PlayList.h"

#include "Accelerators.h"
#include "Config.h"
#include "EventCommandIds.h"
#include "dbus/mpris.h"
#include "Images.h"
#include "LabelEditor.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "PlayerPanel.h"
#include "PlayListAppend.h"
#include "PlayListFile.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Utils.h"

#include <wx/types.h>
#include <wx/uri.h>
#include <wx/busyinfo.h>

#define SAVE_PLAYLIST_TIMEOUT   60000

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
bool isTopPlayingEnabled(int curItem, wxArrayInt selectedItems)
{
    bool topPlayingEnabled = false;
    int selCount = selectedItems.Count();
    for (int index = 0; index < selCount; index++)
        if (selectedItems[index] > (curItem + 1) || selectedItems[index] < curItem)
        {
            topPlayingEnabled = true;
            break;
        }
    return topPlayingEnabled;
}


// -------------------------------------------------------------------------------- //
// guPlayerPlayList - Playlist Panel
// -------------------------------------------------------------------------------- //
guPlayerPlayList::guPlayerPlayList( wxWindow * parent, guDbLibrary * db, wxAuiManager * manager ) :
    guAuiManagedPanel( parent, manager )
{
    MainSizer = new wxBoxSizer(wxVERTICAL);

    // Playlist
    m_PlayListCtrl = new guPlayList(this, db, nullptr, (guMainFrame *) parent);
    m_PlayListCtrl->SetPlayerPlayList(this);

    // Toolbar
    wxBoxSizer *BarSizer;
    BarSizer = new wxBoxSizer(wxHORIZONTAL);

    m_TopPlayingButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_tiny_start), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_TopPlayingButton->SetToolTip(_("Set as Next Track"));
    BarSizer->Add(m_TopPlayingButton, 0, wxALIGN_LEFT | wxLEFT, 2);

    m_TopButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_go_top), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_TopButton->SetToolTip(_("Move the selected tracks to the top"));
    BarSizer->Add(m_TopButton, 0, wxALIGN_LEFT | wxLEFT, 0);

    m_PrevButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_go_up), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_PrevButton->SetToolTip(_( "Move the selected tracks up"));
    BarSizer->Add(m_PrevButton, 0,wxALIGN_LEFT | wxLEFT, 0);

    m_NextButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_go_down), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_NextButton->SetToolTip(_("Move the selected tracks down"));
    BarSizer->Add(m_NextButton, 0,wxALIGN_LEFT | wxLEFT, 0);

    m_BottomButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_go_bottom), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_BottomButton->SetToolTip(_("Move the selected tracks to the bottom"));
    BarSizer->Add(m_BottomButton, 0, wxALIGN_LEFT | wxLEFT, 0);

    BarSizer->Add( 10, 0, 0 );

    m_ShuffleButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_tiny_shuffle), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_ShuffleButton->SetToolTip(_("Shuffle the tracks in the playlist"));
    BarSizer->Add(m_ShuffleButton, 0, wxALIGN_LEFT | wxLEFT, 0);

    m_RemoveButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_tiny_del), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_RemoveButton->SetToolTip(_("Remove the selected tracks from playlist"));
    BarSizer->Add(m_RemoveButton, 0, wxALIGN_LEFT | wxLEFT, 0);

    m_ClearPlaylistButton = new wxBitmapButton(this, wxID_ANY, guImage(guIMAGE_INDEX_tiny_edit_clear), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
    m_ClearPlaylistButton->SetToolTip(_("Clear the Playlist"));
    BarSizer->Add(m_ClearPlaylistButton, 0, wxALIGN_LEFT | wxLEFT, 0);

    // Update layout
    MainSizer->Add(BarSizer, 0, wxEXPAND| wxLEFT, 0);
    MainSizer->Add(m_PlayListCtrl, 1, wxEXPAND, 5);

    SetSizer(MainSizer);
    Layout();
    MainSizer->Fit(this);

    m_TopPlayingButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnTopPlayingBtnClick, this);
    m_TopButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnTopBtnClick, this);
    m_PrevButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnPrevBtnClick, this);
    m_NextButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnNextBtnClick, this);
    m_BottomButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnBottomBtnClick, this);
    m_ShuffleButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnShuffleBtnClick, this);
    m_RemoveButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnRemoveBtnClick, this);
    m_ClearPlaylistButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnClearPlaylistBtnClick, this);
}

// -------------------------------------------------------------------------------- //
guPlayerPlayList::~guPlayerPlayList()
{
    m_TopPlayingButton->Unbind(wxEVT_BUTTON, &guPlayerPlayList::OnTopPlayingBtnClick, this);
    m_TopButton->Unbind(wxEVT_BUTTON, &guPlayerPlayList::OnTopBtnClick, this);
    m_PrevButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnPrevBtnClick, this);
    m_NextButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnNextBtnClick, this);
    m_BottomButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnBottomBtnClick, this);
    m_RemoveButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnRemoveBtnClick, this);
    m_ClearPlaylistButton->Bind(wxEVT_BUTTON, &guPlayerPlayList::OnClearPlaylistBtnClick, this);
}

// -------------------------------------------------------------------------------- //
void guPlayerPlayList::SetPlayerPanel( guPlayerPanel * player )
{
    m_PlayListCtrl->SetPlayerPanel( player );
}

// -------------------------------------------------------------------------------- //
void guPlayerPlayList::OnTopPlayingBtnClick(wxCommandEvent& event)
{
    m_PlayListCtrl->SetTopPlayingTracks(event);
}

// -------------------------------------------------------------------------------- //
void guPlayerPlayList::OnTopBtnClick(wxCommandEvent& event)
{
    m_PlayListCtrl->SetTopTracks(event);
}

void guPlayerPlayList::OnPrevBtnClick(wxCommandEvent& event)
{
    m_PlayListCtrl->SetPrevTracks(event);
}

void guPlayerPlayList::OnNextBtnClick(wxCommandEvent& event)
{
    m_PlayListCtrl->SetNextTracks(event);
}

void guPlayerPlayList::OnBottomBtnClick(wxCommandEvent& event)
{
    m_PlayListCtrl->SetBottomTracks(event);
}

void guPlayerPlayList::OnRemoveBtnClick(wxCommandEvent& event)
{
    m_PlayListCtrl->OnRemoveClicked(event);
}

void guPlayerPlayList::OnClearPlaylistBtnClick(wxCommandEvent& event)
{
    m_PlayListCtrl->OnClearClicked(event);
}

void guPlayerPlayList::OnShuffleBtnClick( wxCommandEvent &event )
{
    const guMediaState state = m_PlayListCtrl->m_PlayerPanel->GetState();
    m_PlayListCtrl->Randomize( state == guMEDIASTATE_PLAYING );
    wxCommandEvent evt( wxEVT_MENU, ID_PLAYERPANEL_TRACKLISTCHANGED );
    wxPostEvent( this, evt );
}

// -------------------------------------------------------------------------------- //
void guPlayerPlayList::UpdatePlayListToolbarState(int item, int curItem, int lastItem, wxArrayInt selectedItems)
{
    bool topPlayingEnabled = isTopPlayingEnabled(curItem, selectedItems);
    bool topPrevEnabled = item != 0;
    bool nextBottomEnabled = item != lastItem;

    m_TopPlayingButton->Enable(topPlayingEnabled);

    m_TopButton->Enable(topPrevEnabled);
    m_PrevButton->Enable(topPrevEnabled);

    m_NextButton->Enable(nextBottomEnabled);
    m_BottomButton->Enable(nextBottomEnabled);
}

// -------------------------------------------------------------------------------- //
// guPlayList
// -------------------------------------------------------------------------------- //
guPlayList::guPlayList(wxWindow * parent,
                       guDbLibrary *db,
                       guPlayerPanel *playerpanel,
                       guMainFrame *mainframe) :
    guListView(parent, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG | guLISTVIEW_ALLOWDROP | guLISTVIEW_DRAGSELFITEMS |
               guLISTVIEW_HIDE_HEADER)
{
    wxArrayString Songs;

    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_MainFrame = mainframe;
    m_ItemHeight = 40;
    m_TotalLen = 0;
    m_CurItem = wxNOT_FOUND;
    m_StartPlaying = false;
    m_SavePlaylistTimer = nullptr;
    m_SysFontPointSize = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT ).GetPointSize();

    InsertColumn( new guListViewColumn( _( "Now Playing" ), 0 ) );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_MaxPlayedTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MAX_TRACKS_PLAYED, 15, CONFIG_PATH_PLAYBACK );
    m_MinPlayListTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MIN_TRACKS_PLAY, 4, CONFIG_PATH_PLAYBACK );
    m_DelTracksPlayed = Config->ReadNum( CONFIG_KEY_PLAYBACK_DEL_TRACKS_PLAYED, false, CONFIG_PATH_PLAYBACK );

    m_PlayBitmap = new wxBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    m_StopBitmap = new wxBitmap( guImage( guIMAGE_INDEX_player_tiny_red_stop ) );
    m_NormalStar   = new wxBitmap( guImage( guIMAGE_INDEX_star_normal_tiny ) );
    m_SelectStar = new wxBitmap( guImage( guIMAGE_INDEX_star_highlight_tiny ) );

    Bind( wxEVT_MENU, &guPlayList::OnClearClicked, this, ID_PLAYER_PLAYLIST_CLEAR );
    Bind( wxEVT_MENU, &guPlayList::OnRemoveClicked, this, ID_PLAYER_PLAYLIST_REMOVE );
    Bind( wxEVT_MENU, &guPlayList::OnSaveClicked, this, ID_PLAYER_PLAYLIST_SAVE );
    Bind( wxEVT_MENU, &guPlayList::OnCreateBestOfClicked, this, ID_PLAYER_PLAYLIST_CREATE_BESTOF );
    Bind( wxEVT_MENU, &guPlayList::OnEditLabelsClicked, this, ID_PLAYER_PLAYLIST_EDITLABELS );
    Bind( wxEVT_MENU, &guPlayList::OnEditTracksClicked, this, ID_PLAYER_PLAYLIST_EDITTRACKS );
    Bind( wxEVT_MENU, &guPlayList::OnSearchClicked, this, ID_PLAYER_PLAYLIST_SEARCH );
    Bind( wxEVT_MENU, &guPlayList::OnCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Bind( wxEVT_MENU, &guPlayList::OnStopAtEnd, this, ID_PLAYER_PLAYLIST_STOP_ATEND );
    Bind( wxEVT_MENU, &guPlayList::SetTopPlayingTracks, this, ID_PLAYER_PLAYLIST_SET_NEXT_TRACK );
    Bind( wxEVT_MENU, &guPlayList::OnSelectTrack, this, ID_PLAYER_PLAYLIST_SELECT_TITLE );
    Bind( wxEVT_MENU, &guPlayList::OnSelectArtist, this, ID_PLAYER_PLAYLIST_SELECT_ARTIST );
    Bind( wxEVT_MENU, &guPlayList::OnSelectAlbum, this, ID_PLAYER_PLAYLIST_SELECT_ALBUM );
    Bind( wxEVT_MENU, &guPlayList::OnSelectAlbumArtist, this, ID_PLAYER_PLAYLIST_SELECT_ALBUMARTIST );
    Bind( wxEVT_MENU, &guPlayList::OnSelectComposer, this, ID_PLAYER_PLAYLIST_SELECT_COMPOSER );
    Bind( wxEVT_MENU, &guPlayList::OnSelectYear, this, ID_PLAYER_PLAYLIST_SELECT_YEAR );
    Bind( wxEVT_MENU, &guPlayList::OnSelectGenre, this, ID_PLAYER_PLAYLIST_SELECT_GENRE );

    Bind( wxEVT_MENU, &guPlayList::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Bind( wxEVT_MENU, &guPlayList::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    Bind( guConfigUpdatedEvent, &guPlayList::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_MENU, &guPlayList::OnDeleteFromLibrary, this, ID_PLAYER_PLAYLIST_DELETE_LIBRARY );
    Bind( wxEVT_MENU, &guPlayList::OnDeleteFromDrive, this, ID_PLAYER_PLAYLIST_DELETE_DRIVE );

    Bind( wxEVT_MENU, &guPlayList::OnSetRating, this, ID_PLAYERPANEL_SETRATING_0, ID_PLAYERPANEL_SETRATING_5 );

    Bind( wxEVT_MENU, &guPlayList::OnCreateSmartPlaylist, this, ID_PLAYLIST_SMART_PLAYLIST );

    Bind( wxEVT_MENU, &guPlayList::StartSavePlaylistTimer, this, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    Bind( wxEVT_TIMER, &guPlayList::OnSavePlaylistTimer, this );
    m_ListBox->Bind( wxEVT_LISTBOX, &guPlayList::OnColumnSelected, this );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPlayList::~guPlayList()
{
    // Save the guPlayList so it can be reload next time
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    SavePlaylistTracks();

    if (m_SavePlaylistTimer)
        delete m_SavePlaylistTimer;

    if (m_PlayBitmap)
      delete m_PlayBitmap;

    if (m_StopBitmap)
      delete m_StopBitmap;

    if (m_NormalStar)
      delete m_NormalStar;

    if (m_SelectStar)
      delete m_SelectStar;

    Unbind( wxEVT_MENU, &guPlayList::OnClearClicked, this, ID_PLAYER_PLAYLIST_CLEAR );
    Unbind( wxEVT_MENU, &guPlayList::OnRemoveClicked, this, ID_PLAYER_PLAYLIST_REMOVE );
    Unbind( wxEVT_MENU, &guPlayList::OnSaveClicked, this, ID_PLAYER_PLAYLIST_SAVE );
    Unbind( wxEVT_MENU, &guPlayList::OnCreateBestOfClicked, this, ID_PLAYER_PLAYLIST_CREATE_BESTOF );
    Unbind( wxEVT_MENU, &guPlayList::OnEditLabelsClicked, this, ID_PLAYER_PLAYLIST_EDITLABELS );
    Unbind( wxEVT_MENU, &guPlayList::OnEditTracksClicked, this, ID_PLAYER_PLAYLIST_EDITTRACKS );
    Unbind( wxEVT_MENU, &guPlayList::OnSearchClicked, this, ID_PLAYER_PLAYLIST_SEARCH );
    Unbind( wxEVT_MENU, &guPlayList::OnCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Unbind( wxEVT_MENU, &guPlayList::OnStopAtEnd, this, ID_PLAYER_PLAYLIST_STOP_ATEND );
    Unbind( wxEVT_MENU, &guPlayList::SetTopPlayingTracks, this, ID_PLAYER_PLAYLIST_SET_NEXT_TRACK );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectTrack, this, ID_PLAYER_PLAYLIST_SELECT_TITLE );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectArtist, this, ID_PLAYER_PLAYLIST_SELECT_ARTIST );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectAlbum, this, ID_PLAYER_PLAYLIST_SELECT_ALBUM );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectAlbumArtist, this, ID_PLAYER_PLAYLIST_SELECT_ALBUMARTIST );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectComposer, this, ID_PLAYER_PLAYLIST_SELECT_COMPOSER );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectYear, this, ID_PLAYER_PLAYLIST_SELECT_YEAR );
    Unbind( wxEVT_MENU, &guPlayList::OnSelectGenre, this, ID_PLAYER_PLAYLIST_SELECT_GENRE );

    Unbind( wxEVT_MENU, &guPlayList::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guPlayList::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    Unbind( guConfigUpdatedEvent, &guPlayList::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Unbind( wxEVT_MENU, &guPlayList::OnDeleteFromLibrary, this, ID_PLAYER_PLAYLIST_DELETE_LIBRARY );
    Unbind( wxEVT_MENU, &guPlayList::OnDeleteFromDrive, this, ID_PLAYER_PLAYLIST_DELETE_DRIVE );

    Unbind( wxEVT_MENU, &guPlayList::OnSetRating, this, ID_PLAYERPANEL_SETRATING_0, ID_PLAYERPANEL_SETRATING_5 );

    Unbind( wxEVT_MENU, &guPlayList::OnCreateSmartPlaylist, this, ID_PLAYLIST_SMART_PLAYLIST );

    Unbind( wxEVT_MENU, &guPlayList::StartSavePlaylistTimer, this, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    Unbind( wxEVT_TIMER, &guPlayList::OnSavePlaylistTimer, this );
    m_ListBox->Unbind( wxEVT_LISTBOX, &guPlayList::OnColumnSelected, this );
}

// -------------------------------------------------------------------------------- //
void guPlayList::CreateAcceleratorTable()
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AccelCmds;
    AccelCmds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
    AccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );
    AccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_0 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_1 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_2 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_3 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_4 );
    AccelCmds.Add( ID_PLAYERPANEL_SETRATING_5 );

    if (guAccelDoAcceleratorTable( AccelCmds, AccelCmds, AccelTable))
        SetAcceleratorTable( AccelTable );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_PLAYBACK )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            m_MaxPlayedTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MAX_TRACKS_PLAYED, 15, CONFIG_PATH_PLAYBACK );
            m_MinPlayListTracks = Config->ReadNum( CONFIG_KEY_PLAYBACK_MIN_TRACKS_PLAY, 4, CONFIG_PATH_PLAYBACK );
            m_DelTracksPlayed = Config->ReadNum( CONFIG_KEY_PLAYBACK_DEL_TRACKS_PLAYED, false, CONFIG_PATH_PLAYBACK );
        }
    }

    if (Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS)
        CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
bool inline guIsMagnatuneFile( const wxString &filename )
{
    return filename.Find( wxT( ".magnatune.com/all/" ) ) != wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropBegin()
{
    if( GetItemCount() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config->ReadBool( CONFIG_KEY_GENERAL_DROP_FILES_CLEAR_PLAYLIST, false, CONFIG_PATH_GENERAL ) )
        {
            ClearItems();
            RefreshAll();
            m_DragOverItem = wxNOT_FOUND;
            //m_CurItem = wxNOT_FOUND;
            //guLogMessage( wxT( "ClearPlaylist set on config. Playlist cleared" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropFile( const wxString &filename )
{
    guLogMessage( wxT( "Dropping '%s'" ), filename.c_str() );
    guTrack track;

    if (guIsMagnatuneFile(filename))
        AddPlayListItem(wxT("http:/") + filename, track, guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND);
    else if (guIsValidAudioFile(filename) ||
             guPlaylistFile::IsValidPlayList(filename) ||
             guCuePlaylistFile::IsValidFile(filename))
    {
        AddPlayListItem(filename, track, guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND);
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropTracks( const guTrackArray * tracks )
{
    guLogMessage( wxT( "Dropping tracks" ) );
    AddToPlayList( * tracks );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDropEnd()
{
    m_DragOverItem = wxNOT_FOUND;
    // Once finished send the update guPlayList event to the guPlayList object
    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config->ReadBool( CONFIG_KEY_GENERAL_DROP_FILES_CLEAR_PLAYLIST, false, CONFIG_PATH_GENERAL ) )
        event.SetExtraLong( 1 );

    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
int guPlayList::GetSelectedSongs( guTrackArray * Songs, const bool isdrag ) const
{
    wxArrayInt Selection = GetSelectedItems( false );
    int Count = Selection.Count();
    for( int Index = 0; Index < Count; Index++ )
        Songs->Add( new guTrack( m_Items[ Selection[ Index ] ] ) );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnColumnSelected(wxCommandEvent &event)
{
    m_DragOverItem = event.GetInt();
    UpdatePlaylistToolbar();
    m_DragOverItem = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnRemoveClicked( wxCommandEvent &event )
{
    RemoveSelected();
}

// -------------------------------------------------------------------------------- //
void guPlayList::RemoveItem(int item_num, bool lock)
{
    if (lock)
        wxMutexLocker Lock(m_ItemsMutex);

    int count = m_Items.Count();
    if( count && ( item_num >= 0 ) && ( item_num < count ) )
    {
        m_TotalLen -= m_Items[ item_num ].m_Length;
        m_Items.RemoveAt( item_num );
        if( item_num == m_CurItem )
            m_CurItem = wxNOT_FOUND;
        else if( item_num < m_CurItem )
            m_CurItem--;
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::RemoveSelected()
{
    wxArrayInt selected = GetSelectedItems(false);
    int count = selected.Count();
    if (!count)
        return;

    wxMutexLocker Lock(m_ItemsMutex);

    int selectedItem = selected[0];
    m_DragOverItem = selectedItem;

    for (int index = count - 1; index >= 0; index--)
        RemoveItem(selected[index], false);

    //guLogMessage( wxT( "RemoveSelected %i" ), m_DragOverItem );
    SetItemCount( GetCount() );   // ReloadItems();

    //wxCommandEvent CmdEvent( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATETITLE );
    CmdEvent.SetInt( 0 );
    CmdEvent.SetExtraLong( 0 );
    wxPostEvent( this, CmdEvent );

    CmdEvent.SetId( ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, CmdEvent );

    ClearSelectedItems();
    SetSelection(selectedItem);
    UpdatePlaylistToolbar();

    m_DragOverItem = wxNOT_FOUND;
}

//// -------------------------------------------------------------------------------- //
//static void PrintItems( const guTrackArray &Songs, int IP, int SI, int CI )
//{
//    // PrintItems( m_Items, InsertPos, Selection[ 0 ], m_CurItem );
//    int Count = Songs.Count();
//    printf( "\nSongs: %li  SelItem: %d  InsPos: %d  CurItem: %d\n", Songs.Count(), SI, IP, CI );
//    for( int Index = 0; Index < Count; Index++ )
//        printf( "%02d ", Songs[ Index ].m_Number );
//    printf( "\n" );
//}

// -------------------------------------------------------------------------------- //
void guPlayList::SetDragOverItem(guLISTVIEW_NAVIGATION target, wxArrayInt Selection)
{
    if (target == guLISTVIEW_NAVIGATION_TOP_PLAYING)
        m_DragOverItem = (m_CurItem == wxNOT_FOUND) ? 0 : m_CurItem + 1;
    else if (target == guLISTVIEW_NAVIGATION_TOP)
        m_DragOverItem = 0;
    else if (target == guLISTVIEW_NAVIGATION_PREVIOUS)
        m_DragOverItem = Selection[0] - 1;
    else if (target == guLISTVIEW_NAVIGATION_NEXT)
    {
        int selectionCount = Selection.Count();
        int cur_selected = Selection[0];
        if (selectionCount > 1)
        {
            for (int Index = 1; Index < selectionCount; Index++)
            {
                if (Selection[Index] != cur_selected + 1)
                    break;
                cur_selected = Selection[Index];
            }
        }
        m_DragOverItem = cur_selected + 1;
    }
    else if (target == guLISTVIEW_NAVIGATION_BOTTOM)
    {
        int lastItem = m_Items.Count() - 1;
        m_DragOverItem = lastItem;
    }
    else if (target == guLISTVIEW_NAVIGATION_ITEM)
    {  // Use current m_DragOverItem
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::MoveSelection()
{
    MoveSelection(guLISTVIEW_NAVIGATION_ITEM);
}

// -------------------------------------------------------------------------------- //
void guPlayList::MoveSelection(guLISTVIEW_NAVIGATION target)
{
    //guLogMessage( wxT( "guPlayList::MoveSelection %i" ), m_DragOverItem );

    // Move the Selected Items to the DragOverItem and DragOverFirst
    int InsertPos;
    bool CurItemSet = false;
    guTrackArray MoveItems;
    wxArrayInt   target_items;

    // How Many elements to move
    wxArrayInt Selection = GetSelectedItems(false);
    int selectionCount = Selection.Count();
    if (!selectionCount)
        return;

    wxMutexLocker Lock(m_ItemsMutex);

    SetDragOverItem(target, Selection);

    // Where are the items to be moved
    InsertPos = m_DragOverAfter ? m_DragOverItem + 1 : m_DragOverItem;
    // PrintItems(m_Items, InsertPos, Selection[0], m_CurItem);

    // Get a copy of every element to move
    for (int Index = 0; Index < selectionCount; Index++)
        MoveItems.Add(m_Items[Selection[Index ]]);

    // Remove the Items and move CurItem and InsertPos
    // We move from last (bigger) to first
    for (int Index = selectionCount - 1; Index >= 0; Index--)
    {
        // guLogMessage( wxT( "Index %i: CurItem: %i InsPos: %i" ), Index, m_CurItem, InsertPos );
        m_Items.RemoveAt( Selection[ Index ] );
        if ( Selection[ Index ] < InsertPos )
            InsertPos--;

        if ( Selection[ Index ] < m_CurItem )
            m_CurItem--;
        else if ( Selection[ Index ] == m_CurItem )
        {
            m_CurItem = InsertPos + Index;
            CurItemSet = true;
        }
    }
    // PrintItems( m_Items, InsertPos, Selection[ 0 ], m_CurItem );

    // Insert every element at the InsertPos
    for ( int Index = 0; Index < selectionCount; Index++ )
    {
        m_Items.Insert( MoveItems[ Index ], InsertPos );
        target_items.Add(InsertPos);
        if ( !CurItemSet && ( InsertPos <= m_CurItem ) )
            m_CurItem++;
        InsertPos++;
    }

    // PrintItems( m_Items, InsertPos, Selection[ 0 ], m_CurItem );
    ClearSelectedItems();
    SetSelectedItems(target_items);

    SetDragOverItem(target, Selection);
    UpdatePlaylistToolbar();

    m_DragOverItem = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
void guPlayList::UpdatePlaylistToolbar()
{
    int lastItem = m_Items.Count() - 1;
    wxArrayInt selectedItems = GetSelectedItems( false );

    m_PlayerPlayList->UpdatePlayListToolbarState(m_DragOverItem, m_CurItem, lastItem, selectedItems);
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetTopPlayingTracks(wxCommandEvent &event)
{
    m_DragOverAfter = false;
    MoveSelection(guLISTVIEW_NAVIGATION_TOP_PLAYING);
    m_ListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetTopTracks(wxCommandEvent &event)
{
    m_DragOverAfter = false;
    MoveSelection(guLISTVIEW_NAVIGATION_TOP);
    m_ListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetPrevTracks( wxCommandEvent &event )
{
    m_DragOverAfter = false;
    MoveSelection(guLISTVIEW_NAVIGATION_PREVIOUS);
    m_ListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetNextTracks( wxCommandEvent &event )
{
    m_DragOverAfter = true;
    MoveSelection(guLISTVIEW_NAVIGATION_NEXT);
    m_ListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetBottomTracks( wxCommandEvent &event )
{
    m_DragOverAfter = true;
    MoveSelection(guLISTVIEW_NAVIGATION_BOTTOM);
    m_ListBox->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        RemoveSelected();
        return;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetPlayList( const guTrackArray &NewItems )
{
    wxMutexLocker Lock( m_ItemsMutex );

    m_Items = NewItems;
    SetSelection( -1 );
    m_CurItem = 0;
    m_TotalLen = 0;

    int Count = m_Items.Count();
    for (int Index = 0; Index < Count; Index++)
      m_TotalLen += m_Items[ Index ].m_Length;

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    guTrack Item;
    wxRect CutRect;
    wxSize TextSize;
    wxString TimeStr;
//    int OffsetSecLine;
//    wxArrayInt Selection;
    int first_line_offset = 2;

    Item = m_Items[ row ];
    m_Attr.m_Font->SetPointSize( m_SysFontPointSize );
    m_Attr.m_Font->SetStyle( wxFONTSTYLE_NORMAL );
    m_Attr.m_Font->SetWeight( wxFONTWEIGHT_BOLD );

    dc.SetFont( * m_Attr.m_Font );
    dc.SetBackgroundMode( wxTRANSPARENT );

    if( IsSelected( row ) )
        dc.SetTextForeground( m_Attr.m_SelFgColor );
    else if( row == m_CurItem )
        dc.SetTextForeground( m_Attr.m_SelBgColor );
    else
    {
        //dc.SetTextForeground( row > m_CurItem ? m_Attr.m_TextFgColor : m_PlayedColor );
        dc.SetTextForeground( m_Attr.m_TextFgColor );
    }

    // Draw the Items Texts
    CutRect = rect;

    // Draw Play bitmap
    if( row == m_CurItem && m_PlayBitmap )
    {
        dc.DrawBitmap( * m_PlayBitmap, CutRect.x + 2, CutRect.y + 10, true );
        CutRect.x += 16;
        CutRect.width -= 16;
    }

    // The NODB Tracks...
    if (Item.m_Type == guTRACK_TYPE_RADIOSTATION)
    {
        dc.DrawText( Item.m_SongName, CutRect.x + 4, CutRect.y + 13 );
        return;
    }

    // The DB Tracks...
    CutRect.width -= ( 50 + 6 + 2 );

    dc.SetClippingRegion( CutRect );

    // Song and number
    //dc.DrawText( ( Item.m_Number ? wxString::Format( wxT( "%02u " ), Item.m_Number ) : wxT( "" ) ) +
    //               Item.m_SongName, CutRect.x + 4, CutRect.y + first_line_offset );

    dc.DrawText(Item.m_SongName, CutRect.x + 24, CutRect.y + first_line_offset);    // Song with Number printed lately
    //dc.DrawText( Item.m_SongName, CutRect.x + 4, CutRect.y + first_line_offset ); // Only song

    //m_Attr.m_Font->SetPointSize( m_SysFontPointSize - 3 );
    m_Attr.m_Font->SetWeight( wxFONTWEIGHT_NORMAL );
    dc.SetFont( * m_Attr.m_Font );

    // Song with Number printed lately
    dc.DrawText((Item.m_Number ? wxString::Format(wxT("%02u "), Item.m_Number) : wxT("")),
                CutRect.x + 4, CutRect.y + first_line_offset);

    // Artist and Album
    m_Attr.m_Font->SetStyle( wxFONTSTYLE_ITALIC );
    m_Attr.m_Font->SetPointSize( m_SysFontPointSize - 2 );
    dc.SetFont( * m_Attr.m_Font );
    dc.DrawText( Item.m_ArtistName + wxT( " - " ) + Item.m_AlbumName, CutRect.x + 4, CutRect.y + m_SecondLineOffset + 2);

    dc.DestroyClippingRegion();

    // Draw the length and rating
    CutRect = rect;
    CutRect.x += ( CutRect.width - ( 50 + 6 ) );
    CutRect.width = ( 50 + 6 );

    dc.SetClippingRegion( CutRect );

    m_Attr.m_Font->SetStyle( wxFONTSTYLE_NORMAL );
    m_Attr.m_Font->SetPointSize( m_SysFontPointSize );
    dc.SetFont( * m_Attr.m_Font );

    int TimeWidth = 56;

    if( Item.m_Type & guTRACK_TYPE_STOP_HERE )
    {
        dc.DrawBitmap( * m_StopBitmap, CutRect.x + 40, CutRect.y + 2, true );
        TimeWidth -= 16;
    }

    // Track length
    TimeStr = LenToString( Item.m_Length );
    TextSize = dc.GetTextExtent( TimeStr );
    TimeWidth -= TextSize.GetWidth();
    if( TimeWidth < 0 )
        TimeWidth = 0;
    dc.DrawText( TimeStr, CutRect.x + TimeWidth, CutRect.y + first_line_offset );
    //guLogMessage( wxT( "%i - %i - %i" ), TimeWidth, TextSize.GetWidth(), TextSize.GetHeight() );

    // The rating
    //OffsetSecLine += 2;
    CutRect.x += 1;
    CutRect.y += 2;
    for( int index = 0; index < 5; index++ )
    {
       dc.DrawBitmap( ( index >= Item.m_Rating ) ? * m_NormalStar : * m_SelectStar,
                      CutRect.x + ( 11 * index ), CutRect.y + m_SecondLineOffset + 2, true );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnMouse( wxMouseEvent &event )
{
    if( event.LeftDown() || event.LeftUp() )
    {
        int x = event.m_x;
        int y = event.m_y;
        wxSize Size = m_ListBox->GetClientSize();
        if( x >= ( Size.GetWidth() - ( 50 + 6 ) ) )
        {
            int Item = HitTest( x, y );
            //if( Item != wxNOT_FOUND && m_Items[ Item ].m_Type == guTRACK_TYPE_DB )
            if( Item != wxNOT_FOUND &&
               ( m_Items[ Item ].m_Type != guTRACK_TYPE_PODCAST ) &&
               ( m_Items[ Item ].m_Type != guTRACK_TYPE_RADIOSTATION ) )
            {
                if( ( size_t ) y > ( ( Item - GetVisibleRowsBegin() ) * m_ItemHeight ) + m_SecondLineOffset + 2 )
                {
                    if( event.LeftDown() )
                    {
                        int Rating;
                        x -= ( Size.GetWidth() - ( 50 + 6 ) );

                        if( x < 3 )
                            Rating = 0;
                        else
                            Rating = wxMin( 5, ( wxMax( 0, x ) / 11 ) + 1 );

                        if( m_Items[ Item ].m_Rating == Rating )
                            Rating = 0;

                        m_Items[ Item ].m_Rating = Rating;
                        RefreshRow( Item );

                        SetTrackRating( m_Items[ Item ], Rating );
                    }
                    return;
                }
            }
        }
    }

    // Do the inherited procedure
    guListView::OnMouse( event );
}

// -------------------------------------------------------------------------------- //
wxCoord guPlayList::OnMeasureItem( size_t n ) const
{
    int Height = 4;
    // Code taken from the generic/listctrl.cpp file
    guPlayList * self = wxConstCast( this, guPlayList );

    wxClientDC dc( self );
    wxFont Font = GetFont();
    Font.SetPointSize( m_SysFontPointSize - 2 );
    dc.SetFont( Font );

    wxCoord y;
    dc.GetTextExtent( wxT( "Hg" ), nullptr, &y );
    Height += y;
    self->m_SecondLineOffset = Height;

//    Font.SetPointSize( m_SysFontPointSize - 3 );
//    dc.SetFont( Font );
//    dc.GetTextExtent( wxT( "Hg" ), nullptr, &y );
    Height += y + 6;

    self->SetItemHeight( Height );
    self->m_ItemHeight = Height;

//    guLogMessage( wxT( "PlayList::OnMeasureItem %i  %i" ), m_SecondLineOffset, Height );

    return Height;
}

// -------------------------------------------------------------------------------- //
void guPlayList::DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    wxRect LineRect;

    if( row == ( int ) m_DragOverItem )
      dc.SetBrush( m_Attr.m_DragBgColor );
    //else if( n == ( size_t ) GetSelection() )
    else if( IsSelected( row ) )
      dc.SetBrush( wxBrush( m_Attr.m_SelBgColor ) );
//    else if( n == ( size_t ) m_CurItem )
//      dc.SetBrush( wxBrush( m_PlayBgColor ) );
    else
      dc.SetBrush( wxBrush( row & 1 ? m_Attr.m_OddBgColor : m_Attr.m_EveBgColor ) );

    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.DrawRectangle( rect );

    if( row == ( int ) m_DragOverItem )
    {
        LineRect = rect;
        if( m_DragOverAfter )
            LineRect.y += ( LineRect.height - 2 );
        LineRect.height = 2;
        dc.SetBrush( * wxBLACK_BRUSH );
        dc.DrawRectangle( LineRect );
    }
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::OnGetItemText( const int row, const int col ) const
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guPlayList::GetItemsList()
{
}

// -------------------------------------------------------------------------------- //
void guPlayList::ReloadItems( bool reset )
{
    SetItemCount( GetCount() );
    RefreshAll( m_CurItem );
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddItem( const guTrack &NewItem, const int pos )
{
    int InsertPos;
    if( m_DragOverItem != wxNOT_FOUND )
    {
        InsertPos = m_DragOverAfter ? m_DragOverItem + 1 : m_DragOverItem;
        if( InsertPos <= m_CurItem )
            m_CurItem++;
        //guLogMessage( wxT( "Inserted at %i %i" ), m_DragOverItem, m_DragOverAfter );
        m_Items.Insert( NewItem, InsertPos );
        if( m_DragOverAfter )
            m_DragOverItem++;
        m_DragOverAfter = true;
    }
    else
    {
        InsertPos = pos;
        if( InsertPos < 0 || InsertPos > ( int ) m_Items.Count() )
            InsertPos = wxNOT_FOUND;
        if( InsertPos != wxNOT_FOUND )
        {
            if( InsertPos <= m_CurItem )
                m_CurItem++;
            m_Items.Insert( NewItem, InsertPos );
        }
        else
            m_Items.Add( NewItem );
    }
}

//// -------------------------------------------------------------------------------- //
//void guPlayList::AddItem( const guTrack * NewItem )
//{
//    AddItem( * NewItem );
//}

// -------------------------------------------------------------------------------- //
void guPlayList::SetCurrent( int curitem, bool delold )
{
    if( delold && ( curitem != m_CurItem ) && ( m_CurItem != wxNOT_FOUND ) &&
        ( ( size_t ) m_CurItem < m_Items.Count() ) )
    {
        m_TotalLen -= m_Items[ m_CurItem ].m_Length;
        m_Items.RemoveAt( m_CurItem );
        if( m_CurItem < curitem )
            curitem--;
        ReloadItems();
    }

    if( curitem >= 0 && curitem <= GetCount() )
        m_CurItem = curitem;
    else
        m_CurItem = wxNOT_FOUND;

    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
int guPlayList::GetCurItem()
{
    return m_CurItem;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetCurrent()
{
//    if( ( CurItem == wxNOT_FOUND ) && Items.Count() )
//        CurItem = 0;
    return GetItem( m_CurItem );
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetNext( const int playmode, const bool forceskip )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( !forceskip && ( playmode == guPLAYER_PLAYMODE_REPEAT_TRACK ) )
            return &m_Items[ m_CurItem ];
        else if( ( m_CurItem < ( ( int ) m_Items.Count() - 1 ) ) )
        {
            if( m_DelTracksPlayed && ( playmode <= guPLAYER_PLAYMODE_SMART ) )
            {
                m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                m_Items.RemoveAt( m_CurItem );
                ReloadItems();
            }
            else
                m_CurItem++;

            return &m_Items[ m_CurItem ];
        }
        else if( playmode == guPLAYER_PLAYMODE_REPEAT_PLAYLIST )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetPrev( const int playmode, const bool forceskip )
{
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
        else if( !forceskip && playmode == guPLAYER_PLAYMODE_REPEAT_TRACK )
            return &m_Items[ m_CurItem ];
        else if( m_CurItem > 0 )
        {
            if( m_DelTracksPlayed && !playmode )
            {
                m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                m_Items.RemoveAt( m_CurItem );
                ReloadItems();
            }
            m_CurItem--;
            return &m_Items[ m_CurItem ];
        }
        else if( playmode == guPLAYER_PLAYMODE_REPEAT_PLAYLIST )
        {
            m_CurItem = m_Items.Count() - 1;
            return &m_Items[ m_CurItem ];
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetNextAlbum( const int playmode, const bool forceskip )
{
    int SaveCurItem = m_CurItem;
    if( m_Items.Count() )
    {
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
//        else if( !forceskip && playloop == guPLAYER_PLAYLOOP_TRACK )
//        {
//            return &m_Items[ m_CurItem ];
//        }
        else if( ( ( size_t ) m_CurItem < ( m_Items.Count() - 1 ) ) )
        {
            int CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;

            while( ( size_t ) m_CurItem < ( m_Items.Count() - 1 ) )
            {
                if( m_DelTracksPlayed && !playmode )
                {
                    m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                    m_Items.RemoveAt( m_CurItem );
                    ReloadItems();
                }
                else
                    m_CurItem++;
                if( m_Items[ m_CurItem ].m_AlbumId != CurAlbumId )
                    break;
            }

            if( ( size_t ) m_CurItem < ( m_Items.Count() - 1 ) )
                return &m_Items[ m_CurItem ];

        }
//        else if( playloop == guPLAYER_PLAYLOOP_PLAYLIST )
//        {
//            m_CurItem = 0;
//            return &m_Items[ m_CurItem ];
//        }
    }
    m_CurItem = SaveCurItem;
    return nullptr;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetPrevAlbum( const int playmode, const bool forceskip )
{
    int SaveCurItem = m_CurItem;
    if( m_Items.Count() )
    {
        //guLogMessage( wxT( "GetPrevAlbum... %i" ), m_CurItem );
        if( m_CurItem == wxNOT_FOUND )
        {
            m_CurItem = 0;
            return &m_Items[ m_CurItem ];
        }
//        else if( !forceskip && playloop == guPLAYER_PLAYLOOP_TRACK )
//        {
//            return &m_Items[ m_CurItem ];
//        }
        else if( m_CurItem > 0 )
        {
            int CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;

            //guLogMessage( wxT( "CurrentAlbum: %i" ), CurAlbumId );
            while( m_CurItem > 0 )
            {
                if( m_DelTracksPlayed && !playmode )
                {
                    m_TotalLen -= m_Items[ m_CurItem ].m_Length;
                    m_Items.RemoveAt( m_CurItem );
                    ReloadItems();
                }
                m_CurItem--;

                //guLogMessage( wxT( "Album %i:  %i" ), m_CurItem, m_Items[ m_CurItem ].m_AlbumId );
                if( m_Items[ m_CurItem ].m_AlbumId != CurAlbumId )
                {
                    CurAlbumId = m_Items[ m_CurItem ].m_AlbumId;
                    while( m_CurItem > 0 && m_Items[ m_CurItem ].m_AlbumId == CurAlbumId )
                    {
                        //guLogMessage( wxT( "New Album %i:  %i" ), m_CurItem, m_Items[ m_CurItem ].m_AlbumId );
                        m_CurItem--;
                    }
                    if( m_Items[ m_CurItem ].m_AlbumId != CurAlbumId )
                        m_CurItem++;
                    break;
                }
            }

            if( m_CurItem >= 0 )
                return &m_Items[ m_CurItem ];
        }
//        else if( playloop == guPLAYER_PLAYLOOP_PLAYLIST )
//        {
//            m_CurItem = m_Items.Count() - 1;
//            return &m_Items[ m_CurItem ];
//        }
    }
    m_CurItem = SaveCurItem;
    return nullptr;
}

// -------------------------------------------------------------------------------- //
guTrack * guPlayList::GetItem( size_t item )
{
    size_t ItemsCount = m_Items.Count();
    if( ItemsCount && item < ItemsCount )
      return &m_Items[ item ];
    return nullptr;
}

// -------------------------------------------------------------------------------- //
long guPlayList::GetLength() const
{
    return m_TotalLen;
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::GetLengthStr() const
{
    return LenToString( m_TotalLen );
}

// -------------------------------------------------------------------------------- //
void guPlayList::ClearItems()
{
    for (int Index = m_Items.Count() - 1; Index >= 0; Index--)
        m_Items.RemoveAt(Index);

    m_CurItem = wxNOT_FOUND;
    m_TotalLen = 0;
    m_PendingLoadIds.Empty();
    m_PendingLoadMediaViewer.clear();
    ClearItemsExtras();
    ClearSelectedItems();
    ReloadItems();
    //PlayerPanel->UpdateTotalLength();
    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST );
    event.SetInt( 0 );
    event.SetExtraLong( 0 );
    wxPostEvent( this, event );

    event.SetId( ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
void guPlayList::Randomize( const bool isplaying )
{
    int pos;
    int newpos;
    int count = m_Items.Count();
    guTrack SavedItem;

    if( count > 2 )
    {
        if( isplaying && m_CurItem > 0 )
        {
            SavedItem = m_Items[ 0 ];
            m_Items[ 0 ] = m_Items[ m_CurItem ];
            m_Items[ m_CurItem ] = SavedItem;
            m_CurItem = 0;
        }
        else if( !isplaying )
        {
            if( m_CurItem != wxNOT_FOUND )
                m_CurItem = wxNOT_FOUND;
        }
        for( int index = 0; index < count; index++ )
        {
            do {
                pos = guRandom( count );
                newpos = guRandom( count );
            } while( ( pos == newpos ) || ( isplaying && ( m_CurItem == 0 ) && ( !pos || !newpos ) ) );
            SavedItem = m_Items[ pos ];
            m_Items[ pos ] = m_Items[ newpos ];
            m_Items[ newpos ] = SavedItem;
//            if( pos == m_CurItem )
//                m_CurItem = newpos;
//            else if( newpos == m_CurItem )
//                m_CurItem = pos;
            //wxMilliSleep( 1 );
           //guLogMessage( wxT( "%u -> %u" ), pos, newpos );
        }
        ClearSelectedItems();
        Refresh( m_CurItem );

        wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
        wxPostEvent( this, event );
    }
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::FindCoverFile( const wxString &dirname )
{
    wxDir           Dir;
    wxString        DirName;
    wxString        FileName;
    wxString        CurFile;
    wxString        RetVal = wxEmptyString;
    wxArrayString   CoverSearchWords;
    wxString        FirstAudioFile;
    wxString        album_name;

    DirName = GetPathAddTrailSep(dirname);

    // Get the SearchCoverWords array
    m_MainFrame->GetCollectionsCoverNames( CoverSearchWords );

    Dir.Open( DirName );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES ) )
        {
            do {
                CurFile = FileName.Lower();
                //guLogMessage( wxT( "Searching %s : %s" ), DirName.c_str(), FileName.c_str() );

                if (guIsValidAudioFile(CurFile))
                {
                    if (FirstAudioFile.IsEmpty() && !m_Db->FindDeletedFile(DirName + FileName, false))
                    {
                        FirstAudioFile = DirName + FileName;
                        guTagInfo *TagInfo = guGetTagInfoHandler(FirstAudioFile);
                        if (TagInfo && TagInfo->ReadAlbumName())
                        {
                            album_name = TagInfo->m_AlbumName.Lower();
                            delete TagInfo;
                        }
                    }
                }
                else if (SearchCoverWords(CurFile, CoverSearchWords, album_name))
                {
                    if (guIsValidImageFile(CurFile))
                    {
                        //printf( "Found Cover: " ); printf( CurFile.char_str() ); printf( "\n" );
                        RetVal = DirName + FileName;
                        break;
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guPlayList::GetPlayListInsertPosition(const int afterCurrent)
{
    int insertPosition = 0;

    wxMutexLocker Lock(m_ItemsMutex);

    switch (afterCurrent)
    {
        case guINSERT_AFTER_CURRENT_NONE :
            insertPosition = m_Items.Count();
            break;

        case guINSERT_AFTER_CURRENT_TRACK :
        {
            if (m_CurItem != wxNOT_FOUND && m_CurItem < (int) m_Items.Count())
            {
                wxString CurFileName = m_Items[m_CurItem].m_FileName;
                insertPosition = m_CurItem + 1;
                while ((insertPosition < (int) m_Items.Count()) &&
                       m_Items[insertPosition].m_FileName == CurFileName)
                {
                    insertPosition++;
                }
            }
            break;
        }

        case guINSERT_AFTER_CURRENT_ALBUM :
        {
            if (m_CurItem != wxNOT_FOUND && m_CurItem < (int) m_Items.Count())
            {
                int CurAlbumId = m_Items[m_CurItem].m_AlbumId;
                insertPosition = m_CurItem + 1;
                if (CurAlbumId)
                {
                    while ((insertPosition < (int) m_Items.Count()) &&
                           m_Items[insertPosition].m_AlbumId == CurAlbumId)
                    {
                        insertPosition++;
                    }
                }
            }
            break;
        }

        case guINSERT_AFTER_CURRENT_ARTIST :
        {
            if (m_CurItem != wxNOT_FOUND && m_CurItem < (int) m_Items.Count())
            {
                int CurArtistId = m_Items[m_CurItem].m_ArtistId;
                insertPosition = m_CurItem + 1;
                if (CurArtistId)
                {
                    while ((insertPosition < (int) m_Items.Count()) &&
                           m_Items[insertPosition].m_ArtistId == CurArtistId)
                    {
                        insertPosition++;
                    }
                }
            }
            break;
        }
    }

    return insertPosition;
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddToPlayList(const guTrackArray &items, const bool deleteOld, const int afterCurrent)
{
    int insertPosition = GetPlayListInsertPosition(afterCurrent);

    int count = items.Count();
    for (int index = 0; index < count; index++)
    {
        AddItem(items[index], insertPosition + index);
        m_TotalLen += items[index].m_Length;
    }

    while (deleteOld && (m_CurItem != 0) && ((m_CurItem) > m_MaxPlayedTracks))
    {
        m_TotalLen -= m_Items[0].m_Length;
        m_Items.RemoveAt(0);
        m_CurItem--;
    }
    ReloadItems();

    wxCommandEvent event(wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER);
    wxPostEvent(this, event);
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddPlayListItem(const wxString &fileName, guTrack track, const int afterCurrent, const int pos, const int index)
{
    wxString newFileName;
    guPodcastItem PodcastItem;

    wxURI Uri(fileName);

    int insertPosition = GetPlayListInsertPosition(afterCurrent);
    //guLogMessage( wxT( "Loading %i %i => %i '%s'" ), aftercurrent, pos, InsertPosition, filename.c_str() );

    if (guCuePlaylistFile::IsValidFile(Uri.GetPath()))   // If its a cue playlist
    {
        int InsertPos = wxMax( pos, 0 );
        guCuePlaylistFile CuePlaylistFile( fileName );

        for (size_t i = 0; i < CuePlaylistFile.m_CueFiles.Count(); i++)
            m_CuePaths.Add(CuePlaylistFile.m_CueFiles[i]);

        int count = CuePlaylistFile.Count();
        if (count)
        {
            for (int index = 0; index < count; index++)
            {
                guCuePlaylistItem &CueItem = CuePlaylistFile.GetItem(index);
                wxString filePath = wxPathOnly(CueItem.m_TrackPath);
                wxString fileNameOnly = CueItem.m_TrackPath.AfterLast(wxT('/'));

                guDbLibrary *Db = m_MainFrame->GetTrackDb(filePath, track.m_MediaViewer);

                // To find the m_SongId
                Db->FindTrackPath(filePath, fileNameOnly, CueItem.m_Name, &track);

                if (!track.m_MediaViewer)
                    AddPendingMediaViewerTrack(Db->GetDbUniqueId(), track);

                track.m_FileName = CueItem.m_TrackPath;
                track.m_SongName = CueItem.m_Name;
                track.m_ArtistName = CueItem.m_ArtistName;
                track.m_Composer = CueItem.m_Composer;
                track.m_Comments = CueItem.m_Comment;
                track.m_AlbumId = 0;
                track.m_AlbumName = CueItem.m_AlbumName;
                track.m_Offset = CueItem.m_Start;
                track.m_Length = CueItem.m_Length;
                track.m_Number = index + 1;
                long Year;
                if (CueItem.m_Year.ToLong(&Year))
                    track.m_Year = Year;
                track.m_Rating = wxNOT_FOUND;

                AddItem(track, insertPosition + InsertPos);
                InsertPos++;
            }
        }
    }
    else if (guPlaylistFile::IsValidPlayList(Uri.GetPath()))   // If its a playlist
    {
        int InsertPos = wxMax( pos, 0 );
        guPlaylistFile PlayList( fileName );
        int Count = PlayList.Count();
        if( Count )
        {
            for (int index = 0; index < Count; index++)
                AddPlayListItem(PlayList.GetItem(index).m_Location, track, afterCurrent, InsertPos++);
        }
    }
    else if (Uri.IsReference())    // Its a file
    {
        if (fileName.StartsWith(wxT("file://")))
            newFileName = wxURI::Unescape(Uri.GetPath());
        else
            newFileName = fileName;

        guLogMessage(wxT("Loading '%s'"), newFileName.c_str());
        if (wxFileExists(newFileName))
        {
            if (guIsValidAudioFile(newFileName))
            {
                bool findResult;
                wxString filePath = wxPathOnly(newFileName);
                wxString fileNameOnly = newFileName.AfterLast(wxT('/'));

                track.m_FileName = newFileName;

                guDbLibrary *Db = m_MainFrame->GetTrackDb(filePath, track.m_MediaViewer);

                if (track.m_Offset)     // Cue sheet track
                    findResult = Db->FindTrackPath(track.m_Path, fileNameOnly, track.m_SongName, &track);
                else
                    findResult = Db->FindTrackFile(newFileName, &track);

                if (!track.m_MediaViewer)
                    AddPendingMediaViewerTrack(Db->GetDbUniqueId(), track);

                if (!findResult)
                {
                    guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                    if (DbPodcasts->GetPodcastItemFile(newFileName, &PodcastItem))
                    {
                        track.m_Type = guTRACK_TYPE_PODCAST;
                        track.m_SongId = PodcastItem.m_Id;
                        track.m_SongName = PodcastItem.m_Title;
                        track.m_ArtistName = PodcastItem.m_Author;
                        track.m_AlbumId = PodcastItem.m_ChId;
                        track.m_AlbumName = PodcastItem.m_Channel;
                        track.m_Length = PodcastItem.m_Length;
                        track.m_PlayCount = PodcastItem.m_PlayCount;
                        track.m_Year = 0;
                        track.m_Rating = wxNOT_FOUND;
                    }
                    else
                    {
                        //guLogMessage( wxT( "Reading tags from the file..." ) );
                        if (track.ReadFromFile(newFileName))
                            track.m_Type = guTRACK_TYPE_NOTDB;
                        else
                        {
                            guLogError(wxT("Could not read tags from file '%s'"), newFileName.c_str());
                            track.m_Type = guTRACK_TYPE_NOTDB;
                            track.m_Number = index;
                            track.m_SongId = index;
                        }
                    }
                }

                m_TotalLen += track.m_Length;
                AddItem(track, insertPosition + wxMax(0, pos));
            }
            else
                guLogError(wxT("It is not an audio file '%s'"), fileName.c_str());
        }
        else if (wxDirExists(newFileName))
        {
            wxDir Dir;
            wxString DirName = GetPathAddTrailSep(newFileName);

            int InsertPos = pos;
            Dir.Open( DirName );
            if( Dir.IsOpened() )
            {
                if (Dir.GetFirst(&newFileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS))
                {
                    do {
                        if ((newFileName[0] != '.'))
                            AddPlayListItem(DirName + newFileName, track, afterCurrent, InsertPos++);
                    } while (Dir.GetNext(&newFileName));
                }
            }
        }
        else
            guLogError(wxT("File doesnt exist '%s'"), fileName.c_str());
    }
    else if (guIsMagnatuneFile(fileName))
    {
        newFileName = fileName;
        newFileName.Replace(wxT(" "), wxT("%20"));
        wxString SearchStr = newFileName;
        int FoundPos;
        if( ( FoundPos = SearchStr.Find( wxT( "@stream.magnatune" ) ) ) != wxNOT_FOUND )
        {
            SearchStr = SearchStr.Mid( FoundPos );
            SearchStr.Replace( wxT( "@stream." ), wxT( "http://he3." ) );
            SearchStr.Replace( wxT( "_nospeech" ), wxEmptyString );
        }
        else if( ( FoundPos = SearchStr.Find( wxT( "@download.magnatune" ) ) ) != wxNOT_FOUND )
        {
            SearchStr = SearchStr.Mid( FoundPos );
            SearchStr.Replace( wxT( "@download." ), wxT( "http://he3." ) );
            SearchStr.Replace( wxT( "_nospeech" ), wxEmptyString );
        }

        guLogMessage( wxT( "Searching for track '%s'" ), SearchStr.c_str() );

        guMediaViewer * MagnatuneMediaViewer = m_MainFrame->FindCollectionMediaViewer( wxT( "Magnatune" ) );
        guMagnatuneLibrary * MagnatuneDb = nullptr;
        if( MagnatuneMediaViewer )
            MagnatuneDb = ( guMagnatuneLibrary * ) MagnatuneMediaViewer->GetDb();
        else if( m_PendingLoadIds.Index( wxT( "Magnatune" ) ) == wxNOT_FOUND )
            m_PendingLoadIds.Add( wxT( "Magnatune" ) );

        if (!MagnatuneDb || (MagnatuneDb->GetTrackId(SearchStr, &track) == wxNOT_FOUND))
        {
            track.m_CoverId  = 0;
            track.m_SongName = newFileName;
            //Track.m_AlbumName = FileName;
            track.m_Length   = 0;
            track.m_Year     = 0;
            track.m_Bitrate  = 0;
            track.m_Rating   = wxNOT_FOUND;
        }
        track.m_FileName = newFileName;
        track.m_Type     = guTRACK_TYPE_MAGNATUNE;
        AddItem(track, insertPosition + wxMax(0, pos));
    }
    else    // This should be a radiostation
    {
        track.m_Type     = guTRACK_TYPE_RADIOSTATION;
        track.m_CoverId  = 0;
        track.m_FileName = fileName;
        track.m_SongName = fileName;
        //Track.m_AlbumName = FileName;
        track.m_Length   = 0;
        track.m_Year     = 0;
        track.m_Bitrate  = 0;
        track.m_Rating   = wxNOT_FOUND;
        AddItem(track, insertPosition + wxMax(0, pos));
    }

    wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
    wxPostEvent( this, event );
}

// -------------------------------------------------------------------------------- //
void guPlayList::AddPendingMediaViewerTrack(const wxString &uniqueid, guTrack &track)
{
    wxArrayInt aItems;

    guPendingLoadMediaViewerHashMap::iterator it = m_PendingLoadMediaViewer.find(uniqueid);
    if (it != m_PendingLoadMediaViewer.end())
        aItems = it->second;

    aItems.Add(track.m_SongId);
    m_PendingLoadMediaViewer[uniqueid] = aItems;
}

// -------------------------------------------------------------------------------- //
void guPlayList::RemoveCueFilesDuplicates()
{
    int count = m_CuePaths.Count();
    if (!count)
        return;

    for (int iCue = 0; iCue < count; iCue++)
    {
        for (size_t iTrack = 0; iTrack < m_Items.Count(); iTrack++)
        {
            if (m_Items[iTrack].m_SongId != wxNOT_FOUND && m_Items[iTrack].m_FileName == m_CuePaths[iCue])
            {
                RemoveItem(iTrack);
                break;
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void AddPlayListCommands( wxMenu * Menu, int SelCount )
{
    if( Menu )
    {
        wxMenuItem * MenuItem;
        wxMenu * SubMenu = new wxMenu();
        wxASSERT( SubMenu );

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
void guPlayList::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int TrackCount = m_Items.Count();
    if( !TrackCount )
    {
        MenuItem = new wxMenuItem( Menu, wxNOT_FOUND, _( "Empty Playlist" ), _( "The playlist is empty" ) );
        Menu->Append( MenuItem );
        return;
    }

    wxArrayInt selectedItems = GetSelectedItems( false );
    int selCount = selectedItems.Count();

    MenuItem = new wxMenuItem(
            Menu, ID_PLAYER_PLAYLIST_EDITLABELS,
            wxString( _( "Edit Labels" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITLABELS ),
            _( "Edit the current selected tracks labels" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem(
            Menu, ID_PLAYER_PLAYLIST_EDITTRACKS,
            wxString( _( "Edit Songs" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
            _( "Edit the current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    if (selCount)
    {
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SET_NEXT_TRACK,
                                _( "Set as Next Track" ),
                                _( "Move the selected tracks to be played next" ) );
        Menu->Append( MenuItem );
        MenuItem->Enable(isTopPlayingEnabled(m_CurItem, selectedItems));

        wxMenu * RatingMenu = new wxMenu();

        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_0, wxT( "☆☆☆☆☆" ), _( "Set the rating to 0" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_1, wxT( "★☆☆☆☆" ), _( "Set the rating to 1" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_2, wxT( "★★☆☆☆" ), _( "Set the rating to 2" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_3, wxT( "★★★☆☆" ), _( "Set the rating to 3" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_4, wxT( "★★★★☆" ), _( "Set the rating to 4" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_5, wxT( "★★★★★" ), _( "Set the rating to 5" ), wxITEM_NORMAL );
        RatingMenu->Append( MenuItem );

        Menu->AppendSubMenu( RatingMenu, _( "Set Rating" ), _( "Set the current selected tracks rating" ) );
        Menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SEARCH,
                            wxString( _( "Search" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SEARCH ),
                            _( "Search a track in the playlist by name" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
    Menu->Append( MenuItem );

    if (selCount == 1)
    {
        wxMenu *     SubMenu;
        SubMenu = new wxMenu();

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_TITLE, _( "Track" ), _( "Selects the current selected track in the library" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_ARTIST, _( "Artist" ), _( "Selects the artist of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_ALBUMARTIST, _( "Album Artist" ), _( "Select the album artist of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_COMPOSER, _( "Composer" ), _( "Select the composer of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_ALBUM, _( "Album" ), _( "Select the album of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_YEAR, _( "Year" ), _( "Select the year of the current song" ) );
        SubMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_SELECT_GENRE, _( "Genre" ), _( "Select the genre of the current song" ) );
        SubMenu->Append( MenuItem );

        Menu->AppendSubMenu( SubMenu, _( "Select" ), _( "Search in the library" ) );
    }

    Menu->AppendSeparator();

//    MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_STOP_ATEND,
//                            wxString( _( "Stop at end" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_STOP_ATEND),
//                            _( "Stop after current playing or selected track" ) );
//    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_stop ) );
//    Menu->Append( MenuItem );
//
//    Menu->AppendSeparator();

    MenuItem = new wxMenuItem(
            Menu, ID_PLAYER_PLAYLIST_SAVE,
            wxString( _( "Save to Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
            _( "Save the selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
    Menu->Append( MenuItem );

    if (selCount == 1)
    {
        MenuItem = new wxMenuItem( Menu, ID_PLAYLIST_SMART_PLAYLIST, _( "Create Smart Playlist" ), _( "Create a smart playlist from this track" ) );
        Menu->Append( MenuItem );
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_CREATE_BESTOF, _( "Create Best of Playlist" ), _( "Create a playlist with the best of this artist" ) );
        Menu->Append( MenuItem );
    }

    MenuItem = new wxMenuItem(
            Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY,
            wxString( _( "Shuffle the Playlist" ) )  + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_RANDOMPLAY ),
            _( "Shuffle the tracks in the playlist" ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem(
            Menu, ID_PLAYER_PLAYLIST_CLEAR,
            wxString( _( "Clear the Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_CLEAR ),
            _( "Remove all tracks from playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
    Menu->Append( MenuItem );

    if (selCount)
    {
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_REMOVE,
                            _( "Remove from Playlist" ),
                            _( "Remove the selected tracks from playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_del ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_DELETE_LIBRARY,
                            _( "Remove from Library" ),
                            _( "Remove the selected tracks from library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_clear ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_DELETE_DRIVE,
                            _( "Delete from Drive" ),
                            _( "Remove the selected tracks from drive" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    m_MainFrame->CreateCopyToMenu( Menu );

    if (selCount == 1 && (m_Items[selectedItems[0]].m_Type < guTRACK_TYPE_RADIOSTATION))
        AddOnlineLinksMenu(Menu);

    AddPlayListCommands(Menu, selCount);
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnClearClicked( wxCommandEvent &event )
{
    ClearItems();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks;
    wxArrayInt SelectedItems = GetSelectedItems( false );
    int count = SelectedItems.Count();
    if( count )
    {
        Tracks = new guTrackArray();
        for( int index = 0; index < count; index++ )
            Tracks->Add( new guTrack( m_Items[ SelectedItems[ index ] ] ) );
    }
    else
        Tracks = new guTrackArray( m_Items );

    int CommandIndex = event.GetId() - ID_COPYTO_BASE;
    if( CommandIndex >= guCOPYTO_DEVICE_BASE )
    {
        CommandIndex -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
        event.SetId( ID_MAINFRAME_COPYTO );

    event.SetInt( CommandIndex );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void inline UpdateTracks( const guTrackArray &tracks, const wxArrayInt &changedflags )
{
    wxArrayPtrVoid MediaViewerPtrs;
    GetMediaViewersList( tracks, MediaViewerPtrs );

    guTrackArray CurrentTracks;
    wxArrayInt   CurrentFlags;
    int Count = MediaViewerPtrs.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        CurrentTracks.Empty();
        CurrentFlags.Empty();
        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];
        GetMediaViewerTracks( tracks, changedflags, MediaViewer, CurrentTracks, CurrentFlags );
        if( CurrentTracks.Count() )
        {
            guDbLibrary * Db = MediaViewer->GetDb();
            Db->UpdateSongs( &CurrentTracks, CurrentFlags );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDeleteFromLibrary( wxCommandEvent &event )
{
    if( GetSelectedCount() )
    {
        if( wxMessageBox( wxT( "Are you sure to remove the selected tracks from your library?" ),
            wxT( "Remove tracks from library" ), wxICON_QUESTION|wxYES|wxNO|wxNO_DEFAULT ) == wxYES )
        {
            guTrackArray SelectedTracks;
            wxArrayInt PodcastsIds;

            wxArrayInt Selected = GetSelectedItems( false );
            int Count = Selected.Count();
            for( int Index = Count - 1; Index >= 0; Index-- )
            {
                const guTrack & Track = m_Items[ Selected[ Index ] ];

                if( Track.m_Type == guTRACK_TYPE_PODCAST )
                    PodcastsIds.Add( Track.m_SongId );
                else if( Track.m_MediaViewer )
                    SelectedTracks.Add( new guTrack( Track ) );

                if( Selected[ Index ] == m_CurItem )
                {
                    m_CurItem--;
                    event.SetId( ID_PLAYERPANEL_NEXTTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                }

                RemoveItem( Selected[ Index ] );
            }

            if( SelectedTracks.Count() )
            {
                wxArrayPtrVoid MediaViewerPtrs;
                GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

                if( ( Count = MediaViewerPtrs.Count() ) )
                {
                    for( int Index = 0; Index < Count; Index++ )
                    {
                        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];
                        guTrackArray MediaViewerTracks;
                        GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );

                        guDbLibrary * Db = MediaViewer->GetDb();
                        Db->DeleteLibraryTracks( &MediaViewerTracks, true );
                    }
                }
            }

            if( ( Count = PodcastsIds.Count() ) )
            {
                guPodcastItemArray Podcasts;
                guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                DbPodcasts->GetPodcastItems( &Podcasts, PodcastsIds, 0, 0 );

                m_MainFrame->RemovePodcastDownloadItems( &Podcasts );

                for( int Index = 0; Index < Count; Index++ )
                    DbPodcasts->SetPodcastItemStatus( PodcastsIds[ Index ], guPODCAST_STATUS_DELETED );
            }

            ClearSelectedItems();
            ReloadItems();

            wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
            wxPostEvent( this, event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnDeleteFromDrive( wxCommandEvent &event )
{
    if( GetSelectedCount() )
    {
        if (wxMessageBox(_("Are you sure to delete the selected tracks from your drive?\nThis will permanently erase the selected tracks."),
            _("Remove tracks from drive"), wxICON_QUESTION|wxYES|wxNO|wxNO_DEFAULT) == wxYES)
        {
            guTrackArray SelectedTracks;
            wxArrayInt PodcastsIds;
            wxArrayInt Selected = GetSelectedItems( false );
            int Count = Selected.Count();
            for( int Index = Count - 1; Index >= 0; Index-- )
            {
                const guTrack & Track = m_Items[ Selected[ Index ] ];

                if( Track.m_Type == guTRACK_TYPE_PODCAST )
                    PodcastsIds.Add( Track.m_SongId );
                else if( Track.m_MediaViewer )
                    SelectedTracks.Add( new guTrack( Track ) );

                if( Track.m_Type != guTRACK_TYPE_RADIOSTATION &&
                    Track.m_Type != guTRACK_TYPE_MAGNATUNE )
                {
                    if( !wxRemoveFile( Track.m_FileName ) )
                        guLogMessage( wxT( "Error deleting '%s'" ), Track.m_FileName.c_str() );
                }

                if( Selected[ Index ] == m_CurItem )
                {
                    m_CurItem--;
                    event.SetId( ID_PLAYERPANEL_NEXTTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                }
                RemoveItem( Selected[ Index ] );
            }

            if( SelectedTracks.Count() )
            {
                wxArrayPtrVoid MediaViewerPtrs;
                GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

                if( ( Count = MediaViewerPtrs.Count() ) )
                {
                    for( int Index = 0; Index < Count; Index++ )
                    {
                        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];
                        guTrackArray MediaViewerTracks;
                        GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );

                        guDbLibrary * Db = MediaViewer->GetDb();

                        Db->DeleteLibraryTracks( &MediaViewerTracks, true );
                    }
                }
            }

            if( ( Count = PodcastsIds.Count() ) )
            {
                guPodcastItemArray Podcasts;
                guDbPodcasts * DbPodcasts = m_MainFrame->GetPodcastsDb();
                DbPodcasts->GetPodcastItems( &Podcasts, PodcastsIds, 0, 0 );

                m_MainFrame->RemovePodcastDownloadItems( &Podcasts );

                for( int Index = 0; Index < Count; Index++ )
                    DbPodcasts->SetPodcastItemStatus( PodcastsIds[ Index ], guPODCAST_STATUS_DELETED );
            }

            ClearSelectedItems();
            ReloadItems();

            wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
            wxPostEvent( this, event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSaveClicked( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    guTrackArray SelectedTracks;

    int Count = SelectedItems.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ SelectedItems[ Index ] ];
            if( Track.m_MediaViewer )
                SelectedTracks.Add( new guTrack( Track ) );
        }
    }
    else
    {
        Count = m_Items.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ Index ];
            if( Track.m_MediaViewer )
                SelectedTracks.Add( new guTrack( Track ) );
        }
    }

    if( SelectedTracks.Count() )
    {
        wxArrayPtrVoid MediaViewerPtrs;
        GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

        if( MediaViewerPtrs.Count() )
        {
            guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ 0 ];
            guTrackArray MediaViewerTracks;
            GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );

            wxArrayInt TrackIds;
            Count = MediaViewerTracks.Count();
            for( int Index = 0; Index < Count; Index++ )
                TrackIds.Add( MediaViewerTracks[ Index ].m_SongId );

            guDbLibrary * Db = MediaViewer->GetDb();
            guListItems PlayLists;

            Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );
            guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( m_MainFrame, Db, &TrackIds, &PlayLists );
            if( PlayListAppendDlg->ShowModal() == wxID_OK )
            {
                int Selected = PlayListAppendDlg->GetSelectedPlayList();
                if( Selected == wxNOT_FOUND )
                {
                    wxString PLName = PlayListAppendDlg->GetPlaylistName();
                    if( PLName.IsEmpty() )
                        PLName = _( "UnNamed" );
                    Db->CreateStaticPlayList( PLName, TrackIds );
                }
                else
                {
                    int PLId = PlayLists[ Selected ].m_Id;
                    wxArrayInt OldSongs;
                    Db->GetPlayListSongIds( PLId, &OldSongs );
                    if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                    {
                        Db->UpdateStaticPlayList( PLId, TrackIds );
                        Db->AppendStaticPlayList( PLId, OldSongs );
                    }
                    else                                                // END
                        Db->AppendStaticPlayList( PLId, TrackIds );
                }

                MediaViewer->UpdatePlaylists();
            }
            PlayListAppendDlg->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCreateBestOfClicked( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( !SelectedItems.IsEmpty() )
    {
        const guTrack &Track = m_Items[ SelectedItems[ 0 ] ];
        if( Track.m_MediaViewer )
            Track.m_MediaViewer->CreateBestOfPlaylist( Track );
        else
            guLogMessage( _( "Can't create a playlist without mediaviewer." ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Songs;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    //
    guTrack * Track;
    wxArrayInt SelectedItems = GetSelectedItems( false );

    int count = SelectedItems.Count();
    if( count )
    {
        for( int index = 0; index < count; index++ )
        {
            Track = &m_Items[ SelectedItems[ index ] ];
            if( !Track->m_Offset && ( Track->m_Type < guTRACK_TYPE_RADIOSTATION ) )
                Songs.Add( new guTrack( * Track ) );
        }
    }
    else
    {
        // If there is no selection then use all songs that are
        // recognized in the database.
        count = m_Items.Count();
        for( int index = 0; index < count; index++ )
        {
            Track = &m_Items[ index ];
            if( !Track->m_Offset && ( Track->m_Type < guTRACK_TYPE_RADIOSTATION ) )
                Songs.Add( new guTrack( * Track ) );
        }
    }

    if( !Songs.Count() )
        return;

    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Songs, &Images, &Lyrics, &ChangedFlags );

    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            guUpdateTracks( Songs, Images, Lyrics, ChangedFlags );

            UpdateTracks( Songs, ChangedFlags );

            // Update the track in database, playlist, etc
            m_MainFrame->UpdatedTracks( guUPDATED_TRACKS_NONE, &Songs );
        }
        guImagePtrArrayClean( &Images );
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnEditLabelsClicked( wxCommandEvent &event )
{
    guTrackArray SelectedTracks;

    //
    wxArrayInt SelectedItems = GetSelectedItems( false );
    int Count = SelectedItems.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ SelectedItems[ Index ] ];
            if( Track.m_MediaViewer )
                SelectedTracks.Add( new guTrack( Track ) );
        }
    }
    else
    {
        // If there is no selection then use all songs that are
        // recognized in the database.
        int Count = m_Items.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_Items[ Index ];
            if( Track.m_MediaViewer )
                SelectedTracks.Add( new guTrack( Track ) );
        }
    }

    if( SelectedTracks.Count() )
    {
        wxArrayPtrVoid MediaViewerPtrs;
        GetMediaViewersList( SelectedTracks, MediaViewerPtrs );

        if( MediaViewerPtrs.Count() )
        {
            guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ 0 ];
            guTrackArray MediaViewerTracks;
            GetMediaViewerTracks( SelectedTracks, MediaViewer, MediaViewerTracks );

            guListItems Tracks;
            wxArrayInt  TrackIds;
            int Count = MediaViewerTracks.Count();
            for( int Index = 0; Index < Count; Index++ )
            {
                const guTrack &Track = MediaViewerTracks[ Index ];
                Tracks.Add( new guListItem( Track.m_SongId, Track.m_SongName ) );
                TrackIds.Add( Track.m_SongId );
            }

            guDbLibrary * Db = MediaViewer->GetDb();
            guArrayListItems LabelSets = Db->GetSongsLabels( TrackIds );

            guLabelEditor * LabelEditor = new guLabelEditor( this, Db, _( "Tracks Labels Editor" ), false, &Tracks, &LabelSets );
            if( LabelEditor )
            {
                if( LabelEditor->ShowModal() == wxID_OK )
                {
                    // Update the labels in the files
                    Db->UpdateSongsLabels( LabelSets );
                }

                LabelEditor->Destroy();

                wxCommandEvent event( wxEVT_MENU, ID_LABEL_UPDATELABELS );
                wxPostEvent( MediaViewer, event );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guPlayList::GetItemSearchText( const int row )
{
    return m_Items[row].m_SongName + m_Items[row].m_ArtistName + m_Items[row].m_AlbumName;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSearchClicked( wxCommandEvent &event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, wxString::Format("%s: ", _("Search")), _( "Please enter the search term" ), m_LastSearch );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        m_LastSearch = EntryDialog->GetValue();
        wxArrayInt Selection = GetSelectedItems();
        long StartItem = 0;
        if( Selection.Count() )
            StartItem = Selection[ 0 ];
        int LastItemFound = FindItem( StartItem, m_LastSearch, true, false );
        SetSelection( LastItemFound );
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectTrack( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_TRACK );
        evt.SetInt( m_Items[ SelItem ].m_SongId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectArtist( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( m_Items[ SelItem ].m_ArtistId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectAlbum( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ALBUM );
        evt.SetInt( m_Items[ SelItem ].m_AlbumId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectAlbumArtist( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ALBUMARTIST );
        evt.SetInt( m_Items[ SelItem ].m_AlbumArtistId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectComposer( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_COMPOSER );
        evt.SetInt( m_Items[ SelItem ].m_ComposerId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectYear( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        int SelYear = m_Items[ SelItem ].m_Year;
        if( SelYear )
        {
            guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

            wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_YEAR );
            evt.SetInt( SelYear );
            evt.SetClientData( MediaViewer );
            evt.SetExtraLong( m_Items[ SelItem ].m_Type );
            wxPostEvent( m_MainFrame, evt );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSelectGenre( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = GetSelectedItems( false );
    if( SelectedItems.Count() )
    {
        int SelItem = SelectedItems[ 0 ];
        guMediaViewer * MediaViewer = m_Items[ SelItem ].m_MediaViewer;

        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_GENRE );
        evt.SetInt( m_Items[ SelItem ].m_GenreId );
        evt.SetClientData( MediaViewer );
        evt.SetExtraLong( m_Items[ SelItem ].m_Type );
        wxPostEvent( m_MainFrame, evt );
    }
}

// -------------------------------------------------------------------------------- //
int guPlayList::GetCaps()
{
//    NONE                  = 0x0000
//    CAN_GO_NEXT           = 0x0001
//    CAN_GO_PREV           = 0x0002
//   *CAN_PAUSE             = 0x0004
//   *CAN_PLAY              = 0x0008
//   *CAN_SEEK              = 0x0010
//    CAN_PROVIDE_METADATA  = 0x0020
//    CAN_HAS_TRACKLIST     = 0x0040
    int Caps = MPRIS_CAPS_NONE;
    if( m_Items.Count() )
    {
        if( m_CurItem < ( int ) m_Items.Count() )
            Caps |= MPRIS_CAPS_CAN_GO_NEXT;
        if( m_CurItem > 0 )
            Caps |= MPRIS_CAPS_CAN_GO_PREV;
        Caps |= ( MPRIS_CAPS_CAN_PAUSE | MPRIS_CAPS_CAN_PLAY | MPRIS_CAPS_CAN_SEEK | MPRIS_CAPS_CAN_PROVIDE_METADATA );
    }
    Caps |= MPRIS_CAPS_CAN_HAS_TRACKLIST;
    return Caps;
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSearchLinkClicked( wxCommandEvent &event )
{
    unsigned long cookie;
    int Item = GetFirstSelected( cookie );
    if( Item != wxNOT_FOUND )
        ExecuteOnlineLink( event.GetId(), GetSearchText( Item ) );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCommandClicked( wxCommandEvent &event )
{
    wxArrayInt Selection = GetSelectedItems( false );
    if( Selection.Count() )
    {
        guConfig * Config = (guConfig *) guConfig::Get();
        if( Config )
        {
            wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
            wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
            wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);
            wxASSERT( Commands.Count() > 0 );

            int CommandIndex = event.GetId() - ID_COMMANDS_BASE;
            wxString CurCmd = Commands[ CommandIndex ];

            const guTrack &Track = m_Items[ Selection[ 0 ] ];

            if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
            {
                //wxString Path = wxT( "\"" ) + wxPathOnly( m_Items[ Selection[ 0 ] ].m_FileName ) + wxT( "\"" );
                wxString Path = wxPathOnly( Track.m_FileName );
                Path.Replace( wxT( " " ), wxT( "\\ " ) );
                CurCmd.Replace( guCOMMAND_ALBUMPATH, Path );
            }

            if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
            {
                int CoverId = Track.m_CoverId;
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 && Track.m_MediaViewer )
                    CoverPath = Track.m_MediaViewer->GetDb()->GetCoverPath( CoverId );
                else
                    CoverPath = FindCoverFile( wxPathOnly( Track.m_FileName ) );

                if( !CoverPath.IsEmpty() )
                    CurCmd.Replace( guCOMMAND_COVERPATH, wxT( "\"" ) + CoverPath + wxT( "\"" ) );
            }

            if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
            {
                wxString SongList = wxEmptyString;
                int count = Selection.Count();
                if( count )
                {
                    for( int index = 0; index < count; index++ )
                        SongList += wxT( " \"" ) + m_Items[ Selection[ index ] ].m_FileName + wxT( "\"" );
                    CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
                }
            }

            //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
            guExecute( CurCmd );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::UpdatedTracks( const guTrackArray * tracks )
{
    // If there are no items in the playlist there is nothing to do
    if( !m_Items.Count() )
        return;

    bool found = false;
    int count = tracks->Count();
    for( int index = 0; index < count; index++ )
    {
        int itemcnt = m_Items.Count();
        for( int item = 0; item < itemcnt; item++ )
        {
            if( ( m_Items[ item ].m_FileName == tracks->Item( index ).m_FileName ) &&
                ( m_Items[ item ].m_Offset == tracks->Item( index ).m_Offset ) )
            {
                m_Items[ item ] = tracks->Item( index );
                found = true;
            }
        }
    }
    if( found )
    {
        RefreshAll();

        wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
        wxPostEvent( this, event );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::UpdatedTrack( const guTrack * track )
{
    // If there are no items in the playlist there is nothing to do
    if( !m_Items.Count() )
        return;

    bool found = false;
    int item;
    int itemcnt = m_Items.Count();
    for( item = 0; item < itemcnt; item++ )
    {
        if( ( m_Items[ item ].m_FileName == track->m_FileName ) &&
            ( m_Items[ item ].m_Offset == track->m_Offset ) )
        {
            m_Items[ item ] = * track;
            found = true;
        }
    }
    if( found )
    {
        RefreshAll();

        wxCommandEvent event( wxEVT_MENU, ID_PLAYER_PLAYLIST_START_SAVETIMER );
        wxPostEvent( this, event );
    }
}

//// -------------------------------------------------------------------------------- //
//wxString inline guPlayList::GetItemName( const int row ) const
//{
//    return m_Items[ row ].m_SongName;
//}
//
//// -------------------------------------------------------------------------------- //
//int inline guPlayList::GetItemId( const int row ) const
//{
//    return row;
//}

// -------------------------------------------------------------------------------- //
wxString guPlayList::GetSearchText( int item ) const
{
    return wxString::Format( wxT( "\"%s\" \"%s\"" ),
        m_Items[ item ].m_ArtistName.c_str(),
        m_Items[ item ].m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
void guPlayList::StopAtEnd()
{
    int ItemToFlag = wxNOT_FOUND;
    wxArrayInt Selection = GetSelectedItems( false );
    int Count = Selection.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            if( Selection[ Index ] > ItemToFlag )
                ItemToFlag = Selection[ Index ];
        }
        //if( ( ItemToFlag != wxNOT_FOUND ) && ( ItemToFlag < ( int ) m_Items.Count() ) ) )
        //    ItemToFlag;
    }
    else
    {
        if( ( m_CurItem >= 0 ) && ( m_CurItem < ( int ) m_Items.Count() ) )
            ItemToFlag = m_CurItem;
    }

    if( ( ItemToFlag >= 0 ) && ( ItemToFlag < ( int ) m_Items.Count() ) )
    {
        m_Items[ ItemToFlag ].m_Type = guTrackType( ( int ) m_Items[ ItemToFlag ].m_Type ^ guTRACK_TYPE_STOP_HERE );
        RefreshAll();
        if( ItemToFlag == m_CurItem )
            m_PlayerPanel->StopAtEnd();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::ClearStopAtEnd()
{
    if( ( m_CurItem >= 0 ) && ( m_CurItem < ( int ) m_Items.Count() ) )
    {
        m_Items[ m_CurItem ].m_Type = guTrackType( ( int ) m_Items[ m_CurItem ].m_Type & 0x7FFFFFFF );
        RefreshAll();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSetRating( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnSetRating( %i )" ), event.GetId() - ID_PLAYERPANEL_SETRATING_0 );
    wxArrayInt Selected = GetSelectedItems( false );
    int Count = Selected.Count();
    if( Count )
    {
        int Rating = event.GetId() - ID_PLAYERPANEL_SETRATING_0;
        guTrackArray UpdatedTracks;

        for( int Index = 0; Index < Count; Index++ )
        {
            int ItemNum = Selected[ Index ];
            m_Items[ ItemNum ].m_Rating = Rating;
            RefreshRow( ItemNum );

            UpdatedTracks.Add( m_Items[ ItemNum ] );
        }

        SetTracksRating( UpdatedTracks, Rating );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetTrackRating( const guTrack &track, const int rating )
{
    guTrackArray Tracks;
    Tracks.Add( track );
    SetTracksRating( Tracks, rating );
}

// -------------------------------------------------------------------------------- //
void guPlayList::SetTracksRating( const guTrackArray &tracks, const int rating )
{
    wxArrayPtrVoid MediaViewerPtrs;
    GetMediaViewersList( tracks, MediaViewerPtrs );

    guTrackArray CurrentTracks;
    wxArrayInt   CurrentFlags;
    int Count = MediaViewerPtrs.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        CurrentTracks.Empty();
        CurrentFlags.Empty();

        guMediaViewer * MediaViewer = ( guMediaViewer * ) MediaViewerPtrs[ Index ];

        GetMediaViewerTracks( tracks, guTRACK_CHANGED_DATA_RATING, MediaViewer, CurrentTracks, CurrentFlags );

        if( MediaViewer->GetEmbeddMetadata() )
        {
            guImagePtrArray Images;
            wxArrayString Lyrics;
            guUpdateTracks( CurrentTracks, Images, Lyrics, CurrentFlags );
        }

        guDbLibrary * Db = MediaViewer->GetDb();
        Db->SetTracksRating( &CurrentTracks, rating );

        MediaViewer->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &CurrentTracks );
        m_PlayerPanel->UpdatedTracks( &CurrentTracks );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::MediaViewerCreated( const wxString &uniqueid, guMediaViewer * mediaviewer )
{
    if (m_PendingLoadIds.Index(uniqueid) != wxNOT_FOUND)
        CheckPendingLoadItems(uniqueid, mediaviewer);
    LoadPendingMediaViewerTracks(uniqueid, mediaviewer);
}

// -------------------------------------------------------------------------------- //
void guPlayList::MediaViewerClosed( guMediaViewer * mediaviewer )
{
    int count = m_Items.Count();
    for (int index = 0; index < count; index++)
    {
        guTrack &track = m_Items[index];
        if (track.m_MediaViewer == mediaviewer)
        {
            track.m_MediaViewer = nullptr;
            AddPendingMediaViewerTrack(mediaviewer->GetDb()->GetDbUniqueId(), track);
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::LoadPendingMediaViewerTracks(const wxString &uniqueid, guMediaViewer *mediaviewer)
{
    guPendingLoadMediaViewerHashMap::iterator it = m_PendingLoadMediaViewer.find(uniqueid);
    if (it == m_PendingLoadMediaViewer.end())
        return;

    wxArrayInt &pendingTracks = it->second;

    int count = m_Items.Count();
    for (int index = 0; index < count; index++)
    {
        guTrack &track = m_Items[index];
        if (!track.m_MediaViewer)
        {
            int ix = pendingTracks.Index(track.m_SongId);
            if (ix != wxNOT_FOUND)
            {
                track.m_MediaViewer = mediaviewer;
                pendingTracks.RemoveAt(ix);
                // it->second = pendingTracks;
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::CheckPendingLoadItems( const wxString &uniqueid, guMediaViewer * mediaviewer )
{
    if( !mediaviewer )
        return;

    int Type;
    if( uniqueid == wxT( "Magnatune" ) )
        Type = guTRACK_TYPE_MAGNATUNE;
    else
        return;

    guDbLibrary * Db = mediaviewer->GetDb();
    if (!Db)
        return;

    wxString filename;
    int count = m_Items.Count();
    for (int index = 0; index < count; index++)
    {
        guTrack &track = m_Items[index];
        if (track.m_Type == Type)
        {
            //guLogMessage( wxT( "Trying: '%s'" ), Track.m_FileName.c_str() );
            filename = track.m_FileName;
            filename.Replace( wxT( " " ), wxT( "%20" ) );
            wxString search_str = filename;
            int found_pos;
            if ((found_pos = search_str.Find(wxT("@stream.magnatune"))) != wxNOT_FOUND)
            {
                search_str = search_str.Mid( found_pos );
                search_str.Replace( wxT( "@stream." ), wxT( "http://he3." ) );
                search_str.Replace( wxT( "_nospeech" ), wxEmptyString );
            }
            else if ((found_pos = search_str.Find(wxT("@download.magnatune"))) != wxNOT_FOUND)
            {
                search_str = search_str.Mid( found_pos );
                search_str.Replace( wxT( "@download." ), wxT( "http://he3." ) );
                search_str.Replace( wxT( "_nospeech" ), wxEmptyString );
            }

            guLogMessage( wxT( "Searching for track '%s'" ), search_str.c_str() );

            ((guMagnatuneLibrary *) Db)->GetTrackId(search_str, &track);
            track.m_Type     = guTRACK_TYPE_MAGNATUNE;
            track.m_FileName = filename;
            track.m_MediaViewer = mediaviewer;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnCreateSmartPlaylist( wxCommandEvent &event )
{
    wxArrayInt Selected = GetSelectedItems( false );
    if( Selected.Count() )
    {
        const guTrack &Track = m_Items[ Selected[ 0 ] ];
        if( Track.m_MediaViewer )
            Track.m_MediaViewer->CreateSmartPlaylist( Track.m_ArtistName, Track.m_SongName );
    }
}

// -------------------------------------------------------------------------------- //
void guPlayList::SavePlaylistTracks()
{
    long item = wxNOT_FOUND;
    guTrackArray tracks;
    guConfig *Config = (guConfig *) guConfig::Get();

    if (Config->ReadBool(CONFIG_KEY_PLAYLIST_SAVE_ON_CLOSE, true, CONFIG_PATH_PLAYLIST))
    {
        int count = m_Items.Count();
        for (int index = 0; index < count; index++)
        {
            if (m_Items[index].m_Type < guTRACK_TYPE_IPOD)
                tracks.Add(new guTrack(m_Items[index]));
        }
        item = m_CurItem;
    }

    Config->SavePlaylistTracks(tracks, item);
}

// -------------------------------------------------------------------------------- //
void guPlayList::LoadPlaylistTracks()
{
    wxBusyInfo BusyInfo(_("Loading tracks. Please wait"));
    wxTheApp->Yield();

    guMainApp *mainApp = (guMainApp *) wxTheApp;
    if (mainApp && mainApp->argc > 1)
    {
        guTrack track;
        int count = mainApp->argc;
        for (int index = 1; index < count; index++)
        {
            //guLogMessage(wxT("%u-%u %s"), index, MainApp->argc, MainApp->argv[index]);
            AddPlayListItem(mainApp->argv[index], track, guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND);
        }
        m_CurItem = wxNOT_FOUND;
        m_StartPlaying = true;
    }
    else
    {
        guTrackArray tracks;
        guConfig *config = (guConfig *) guConfig::Get();

        int curItem = config->LoadPlaylistTracks( tracks );
        int count = tracks.Count();

        wxTheApp->Yield();

        for (int index = 0; index < count; index++)
        {
            if ((tracks[index].m_Type == guTRACK_TYPE_RADIOSTATION))
                m_Items.Add(new guTrack(tracks[index]));
            else
                AddPlayListItem(tracks[index].m_FileName, tracks[index], guINSERT_AFTER_CURRENT_NONE, wxNOT_FOUND, index);
        }

        m_CurItem = curItem > count ? wxNOT_FOUND : curItem;
    }

    wxCommandEvent event(wxEVT_MENU, ID_PLAYER_PLAYLIST_UPDATELIST);
    event.SetInt(1);
    wxPostEvent(this, event);
}

// -------------------------------------------------------------------------------- //
void guPlayList::StartSavePlaylistTimer( wxCommandEvent &event )
{
    if( !m_SavePlaylistTimer )
        m_SavePlaylistTimer = new wxTimer( this );

    m_SavePlaylistTimer->Start( SAVE_PLAYLIST_TIMEOUT, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guPlayList::OnSavePlaylistTimer( wxTimerEvent & )
{
    SavePlaylistTracks();
}

}
