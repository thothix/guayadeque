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
#include "TaskBar.h"
#include "Images.h"
#include "EventCommandIds.h"
#include "Utils.h"

#include <wx/menu.h>

namespace Guayadeque {

// ---------------------------------------------------------------------- //
// guTaskBarIcon
// ---------------------------------------------------------------------- //
guTaskBarIcon::guTaskBarIcon( guMainFrame * NewMainFrame, guPlayerPanel * NewPlayerPanel ) : wxTaskBarIcon()
{
    m_MainFrame = NewMainFrame;
    m_PlayerPanel = NewPlayerPanel;
    //
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_PLAY );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_STOP );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_NEXTTRACK );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_NEXTALBUM );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_PREVTRACK );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_PREVALBUM );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_MENU_QUIT );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYMODE_SMART );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYMODE_REPEAT_PLAYLIST );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYMODE_REPEAT_TRACK );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYLIST_RANDOMPLAY );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_SETRATING_0, ID_PLAYERPANEL_SETRATING_5 );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_MAINFRAME_SETFORCEGAPLESS );
    Bind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_MAINFRAME_SETAUDIOSCROBBLE );

    Bind( wxEVT_TASKBAR_LEFT_DOWN, &guTaskBarIcon::OnClick, this );
}

// ---------------------------------------------------------------------- //
guTaskBarIcon::~guTaskBarIcon()
{
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_PLAY );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_STOP );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_NEXTTRACK );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_NEXTALBUM );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_PREVTRACK );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_PREVALBUM );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_MENU_QUIT );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYMODE_SMART );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYMODE_REPEAT_PLAYLIST );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYMODE_REPEAT_TRACK );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYER_PLAYLIST_RANDOMPLAY );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_PLAYERPANEL_SETRATING_0, ID_PLAYERPANEL_SETRATING_5 );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_MAINFRAME_SETFORCEGAPLESS );
    Unbind( wxEVT_MENU, &guTaskBarIcon::SendEventToMainFrame, this, ID_MAINFRAME_SETAUDIOSCROBBLE );

    Unbind( wxEVT_TASKBAR_LEFT_DOWN, &guTaskBarIcon::OnClick, this );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::SendEventToMainFrame( wxCommandEvent &event )
{
    wxPostEvent( m_MainFrame, event );
}

// ---------------------------------------------------------------------- //
void guTaskBarIcon::OnClick( wxTaskBarIconEvent &event )
{
    if( m_MainFrame )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( !m_MainFrame->IsShown() )
        {
            m_MainFrame->Show( true );
            if( m_MainFrame->IsIconized() )
                m_MainFrame->Iconize( false );
        }
        else if( Config->ReadBool( CONFIG_KEY_GENERAL_CLOSE_TO_TASKBAR, false, CONFIG_PATH_GENERAL ) )
        {
            m_MainFrame->Show( false );
        }
        else
        {
            m_MainFrame->Iconize( !m_MainFrame->IsIconized() );
        }
    }
}

// ---------------------------------------------------------------------- //
wxMenu * guTaskBarIcon::CreatePopupMenu()
{
    wxMenu * Menu = new wxMenu;
    wxMenuItem * MenuItem;

    if( m_PlayerPanel )
    {
        bool IsPaused = ( m_PlayerPanel->GetState() == guMEDIASTATE_PLAYING );
        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PLAY, IsPaused ? _( "Pause" ) : _( "Play" ), _( "Play current playlist" ) );
        //MenuItem->SetBitmap( guImage( IsPaused ? guIMAGE_INDEX_player_normal_pause : guIMAGE_INDEX_player_normal_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_STOP, _( "Stop" ), _( "Play current playlist" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_stop ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_NEXTTRACK, _( "Next Track" ), _( "Skip to next track in current playlist" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_next ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_NEXTALBUM, _( "Next Album" ), _( "Skip to next album track in current playlist" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_next ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PREVTRACK, _( "Prev Track" ), _( "Skip to previous track in current playlist" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_prev ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PLAYERPANEL_PREVALBUM, _( "Prev Album" ), _( "Skip to previous album track in current playlist" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_normal_prev ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
        wxMenu * RatingMenu = new wxMenu();

        int Rating = m_PlayerPanel->GetRating();

        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_0, wxT( "☆☆☆☆☆" ), _( "Set the rating to 0" ), wxITEM_CHECK );
        RatingMenu->Append( MenuItem );
        MenuItem->Check( Rating <= 0 );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_1, wxT( "★☆☆☆☆" ), _( "Set the rating to 1" ), wxITEM_CHECK );
        RatingMenu->Append( MenuItem );
        MenuItem->Check( Rating == 1 );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_2, wxT( "★★☆☆☆" ), _( "Set the rating to 2" ), wxITEM_CHECK );
        RatingMenu->Append( MenuItem );
        MenuItem->Check( Rating == 2 );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_3, wxT( "★★★☆☆" ), _( "Set the rating to 3" ), wxITEM_CHECK );
        RatingMenu->Append( MenuItem );
        MenuItem->Check( Rating == 3 );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_4, wxT( "★★★★☆" ), _( "Set the rating to 4" ), wxITEM_CHECK );
        RatingMenu->Append( MenuItem );
        MenuItem->Check( Rating == 4 );
        MenuItem = new wxMenuItem( RatingMenu, ID_PLAYERPANEL_SETRATING_5, wxT( "★★★★★" ), _( "Set the rating to 5" ), wxITEM_CHECK );
        RatingMenu->Append( MenuItem );
        MenuItem->Check( Rating == 5 );

        Menu->AppendSubMenu( RatingMenu, _( "Rating" ), _( "Set the current track rating" ) );

        Menu->AppendSeparator();
        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYMODE_SMART, _( "&Smart Play" ), _( "Update playlist based on Last.fm statics" ), wxITEM_CHECK );
        Menu->Append( MenuItem );
        MenuItem->Check( m_PlayerPanel->GetPlaySmart() );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYMODE_REPEAT_PLAYLIST, _( "Repeat the &Playlist" ), _( "Repeat the tracks in the playlist" ), wxITEM_CHECK );
        Menu->Append( MenuItem );
        MenuItem->Check( m_PlayerPanel->GetPlayMode() == guPLAYER_PLAYMODE_REPEAT_PLAYLIST );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYMODE_REPEAT_TRACK, _( "Repeat &Track" ), _( "Repeat the current track in the playlist" ), wxITEM_CHECK );
        Menu->Append( MenuItem );
        MenuItem->Check( m_PlayerPanel->GetPlayMode() == guPLAYER_PLAYMODE_REPEAT_TRACK );

        MenuItem = new wxMenuItem( Menu, ID_PLAYER_PLAYLIST_RANDOMPLAY, _( "Shu&ffle" ), _( "Shuffle the tracks in the playlist" ), wxITEM_NORMAL );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( Menu, ID_MAINFRAME_SETFORCEGAPLESS, _( "Force &Gapless Mode" ), _( "Set playback in gapless mode" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( m_PlayerPanel->GetForceGapless() );

    MenuItem = new wxMenuItem( Menu, ID_MAINFRAME_SETAUDIOSCROBBLE, _( "Audioscrobbling" ), _( "Send played tracks information" ), wxITEM_CHECK );
    Menu->Append( MenuItem );
    MenuItem->Check( m_PlayerPanel->GetAudioScrobbleEnabled() );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_MENU_QUIT, _( "Exit" ), _( "Exit this program" ) );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_stop ) );
    Menu->Append( MenuItem );

    return Menu;
}

}

// ---------------------------------------------------------------------- //
