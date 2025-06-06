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
#include "Magnatune.h"

#include "LyricsPanel.h"
#include "MainFrame.h"
#include "SelCoverFile.h"
#include "StatusBar.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/tokenzr.h>
#include <wx/zstream.h>
#include <wx/xml/xml.h>

#include <id3v1genres.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guMagnatuneLibrary
// -------------------------------------------------------------------------------- //
guMagnatuneLibrary::guMagnatuneLibrary( const wxString &dbname ) :
    guDbLibrary( dbname )
{
}

// -------------------------------------------------------------------------------- //
guMagnatuneLibrary::~guMagnatuneLibrary()
{
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::UpdateArtistsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ArIndex;
  int           ArCount = labelsets.Count();
  for( ArIndex = 0; ArIndex < ArCount; ArIndex++ )
  {
    wxArrayInt ArLabels = labelsets[ ArIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt Artists;
    Artists.Add( labelsets[ ArIndex ].GetId() );
    SetArtistsLabels( Artists, ArLabels );
  }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::UpdateSongsLabels( const guArrayListItems &labelsets )
{
  guListItems LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int ItemIndex;
  int ItemCount = labelsets.Count();

    for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetSongsLabels( ItemIds, ItemLabels );
  }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::UpdateAlbumsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int ItemIndex;
  int ItemCount = labelsets.Count();

    for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetAlbumsLabels( ItemIds, ItemLabels );
  }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::CreateNewSong( guTrack * track, const wxString &albumsku, const wxString &coverlink )
{
    wxString query;
    //wxSQLite3ResultSet dbRes;

    track->m_SongId = FindTrack( track->m_ArtistName, track->m_SongName );
    if( track->m_SongId != wxNOT_FOUND )
    {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', " \
                                 "song_genreid = %u, song_genre = '%s', " \
                                 "song_artistid = %u, song_artist = '%s', " \
                                 "song_albumid = %u, song_album = '%s', " \
                                 "song_pathid = %u, song_path = '%s', " \
                                 "song_filename = '%s', " \
                                 "song_number = %u, song_year = %u, " \
                                 "song_length = %u, " \
                                 "song_albumsku = '%s', " \
                                 "song_coverlink = '%s' " \
                                 "WHERE song_id = %u;" ),
                    escape_query_str( track->m_SongName ).c_str(),
                    track->m_GenreId,
                    escape_query_str( track->m_GenreName ).c_str(),
                    track->m_ArtistId,
                    escape_query_str( track->m_ArtistName ).c_str(),
                    track->m_AlbumId,
                    escape_query_str( track->m_AlbumName ).c_str(),
                    track->m_PathId,
                    escape_query_str( track->m_Path ).c_str(),
                    escape_query_str( track->m_FileName ).c_str(),
                    track->m_Number,
                    track->m_Year,
                    track->m_Length,
                    escape_query_str( albumsku ).c_str(),
                    escape_query_str( coverlink ).c_str(),
                    track->m_SongId );

        ExecuteUpdate( query );
    }
    else
    {
        wxString query = wxString::Format( wxT( "INSERT INTO songs( " \
                    "song_id, song_playcount, song_addedtime, " \
                    "song_name, song_genreid, song_genre, song_artistid, song_artist, " \
                    "song_albumid, song_album, song_pathid, song_path, song_filename, song_format, song_number, song_year, " \
                    "song_coverid, song_disk, song_length, song_offset, song_bitrate, song_rating, " \
                    "song_filesize, song_albumsku, song_coverlink ) VALUES( NULL, 0, %lu, '%s', %u, '%s', %u, '%s', %u, '%s', " \
                    "%u, '%s', '%s', 'mp3,ogg', %u, %u, %u, '%s', %u, 0, 0, -1, 0, '%s', '%s' )" ),
                    wxDateTime::GetTimeNow(),
                    escape_query_str( track->m_SongName ).c_str(),
                    track->m_GenreId,
                    escape_query_str( track->m_GenreName ).c_str(),
                    track->m_ArtistId,
                    escape_query_str( track->m_ArtistName ).c_str(),
                    track->m_AlbumId,
                    escape_query_str( track->m_AlbumName ).c_str(),
                    track->m_PathId,
                    escape_query_str( track->m_Path ).c_str(),
                    escape_query_str( track->m_FileName ).c_str(),
                    track->m_Number,
                    track->m_Year,
                    track->m_CoverId,
                    escape_query_str( track->m_Disk ).c_str(),
                    track->m_Length,
                    escape_query_str( albumsku ).c_str(),
                    escape_query_str( coverlink ).c_str() );

        //guLogMessage( wxT( "%s" ), query.c_str() );
        ExecuteUpdate( query );
    }

//    guLogMessage( wxT( "%s/%s/%s/%i - %s" ),
//                track->m_GenreName.c_str(),
//                track->m_ArtistName.c_str(),
//                track->m_AlbumName.c_str(),
//                track->m_Number,
//                track->m_SongName.c_str() );
}

// -------------------------------------------------------------------------------- //
int guMagnatuneLibrary::GetTrackId( const wxString &url, guTrack * track )
{
    wxString query;
    wxSQLite3ResultSet dbRes;
    int RetVal = wxNOT_FOUND;

    query = GU_TRACKS_QUERYSTR +
            wxString::Format( wxT( " WHERE song_filename = '%s' LIMIT 1;" ),
                    escape_query_str( url.BeforeLast( '.' ) ).c_str() );
    //guLogMessage( wxT( "Searching:\n%s" ), query.c_str() );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = dbRes.GetInt( 0 );
      if (track)
          FillTrackFromDb( track, &dbRes );
    }
    dbRes.Finalize();

    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guMagnatuneLibrary::GetAlbumSku( const int albumid )
{
    wxString query;
    wxSQLite3ResultSet dbRes;
    wxString RetVal;

    query = wxString::Format( wxT( "SELECT song_albumsku FROM songs WHERE song_albumid = %i LIMIT 1;" ), albumid );
    //guLogMessage( wxT( "Searching:\n%s" ), query.c_str() );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
      RetVal = dbRes.GetString( 0 );

    dbRes.Finalize();
    return RetVal;
}


// -------------------------------------------------------------------------------- //
// guMagnatunePanel
// -------------------------------------------------------------------------------- //
guMagnatunePanel::guMagnatunePanel( wxWindow * parent, guMediaViewerMagnatune * mediaviewer ) :
    guLibPanel( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guMagnatunePanel::OnDownloadAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM );
    Bind( wxEVT_MENU, &guMagnatunePanel::OnDownloadTrackAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
guMagnatunePanel::~guMagnatunePanel()
{
    Unbind( wxEVT_MENU, &guMagnatunePanel::OnDownloadAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM );
    Unbind( wxEVT_MENU, &guMagnatunePanel::OnDownloadTrackAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnDownloadAlbum( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    ((guMediaViewerMagnatune *) m_MediaViewer)->DownloadAlbums(Albums);
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );

    wxArrayInt Albums;
    int Count = Tracks.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( Albums.Index( Tracks[ Index ].m_AlbumId ) == wxNOT_FOUND )
            Albums.Add( Tracks[ Index ].m_AlbumId );
    }

    ( ( guMediaViewerMagnatune * ) m_MediaViewer )->DownloadAlbums( Albums );
}


// -------------------------------------------------------------------------------- //
// guMagnatuneAlbumBrowser
// -------------------------------------------------------------------------------- //
guMagnatuneAlbumBrowser::guMagnatuneAlbumBrowser( wxWindow * parent, guMediaViewer * mediaviewer ) :
    guAlbumBrowser( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guMagnatuneAlbumBrowser::OnDownloadAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM  );
}

// -------------------------------------------------------------------------------- //
guMagnatuneAlbumBrowser::~guMagnatuneAlbumBrowser()
{
    Unbind( wxEVT_MENU, &guMagnatuneAlbumBrowser::OnDownloadAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM  );
}

// -------------------------------------------------------------------------------- //
void guMagnatuneAlbumBrowser::OnDownloadAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadAlbum" ) );
    if( m_LastAlbumBrowserItem )
    {
        wxArrayInt Albums;
        Albums.Add( m_LastAlbumBrowserItem->m_AlbumId );
        ( ( guMediaViewerMagnatune * ) m_MediaViewer )->DownloadAlbums( Albums );
    }
}


// -------------------------------------------------------------------------------- //
// guMagnatuneTreePanel
// -------------------------------------------------------------------------------- //
guMagnatuneTreePanel::guMagnatuneTreePanel( wxWindow * parent, guMediaViewer * mediaviewer ) :
    guTreeViewPanel( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guMagnatuneTreePanel::OnDownloadAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM  );
    Bind( wxEVT_MENU, &guMagnatuneTreePanel::OnDownloadTrackAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM  );
}

// -------------------------------------------------------------------------------- //
guMagnatuneTreePanel::~guMagnatuneTreePanel()
{
    Unbind( wxEVT_MENU, &guMagnatuneTreePanel::OnDownloadAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM  );
    Unbind( wxEVT_MENU, &guMagnatuneTreePanel::OnDownloadTrackAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM  );
}

// -------------------------------------------------------------------------------- //
void guMagnatuneTreePanel::OnDownloadAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadAlbum" ) );
    const wxTreeItemId &CurItemId = m_TreeViewCtrl->GetSelection();
    guTreeViewData * TreeViewData = ( guTreeViewData * ) m_TreeViewCtrl->GetItemData( CurItemId );
    int ItemType = TreeViewData->GetType();

    if( ItemType == guLIBRARY_ELEMENT_ALBUMS )
    {
        wxArrayInt Albums;
        Albums.Add( TreeViewData->m_Id );
        ( ( guMediaViewerMagnatune * ) m_MediaViewer )->DownloadAlbums( Albums );
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneTreePanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadTrackAlbum" ) );

    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );

    wxArrayInt Albums;
    int Count = Tracks.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( Albums.Index( Tracks[ Index ].m_AlbumId ) == wxNOT_FOUND )
            Albums.Add( Tracks[ Index ].m_AlbumId );
    }

    ( ( guMediaViewerMagnatune * ) m_MediaViewer )->DownloadAlbums( Albums );
}


// -------------------------------------------------------------------------------- //
// guMagnatunePlayListPanel
// -------------------------------------------------------------------------------- //
guMagnatunePlayListPanel::guMagnatunePlayListPanel( wxWindow * parent, guMediaViewer * mediaviewer ) :
    guPlayListPanel( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guMagnatunePlayListPanel::OnDownloadTrackAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
guMagnatunePlayListPanel::~guMagnatunePlayListPanel()
{
    Unbind( wxEVT_MENU, &guMagnatunePlayListPanel::OnDownloadTrackAlbum, this, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePlayListPanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadTrackAlbum" ) );

    guTrackArray Tracks;
    m_PLTracksListBox->GetSelectedSongs( &Tracks );

    wxArrayInt Albums;
    int Count = Tracks.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( Albums.Index( Tracks[ Index ].m_AlbumId ) == wxNOT_FOUND )
            Albums.Add( Tracks[ Index ].m_AlbumId );
    }

    ( ( guMediaViewerMagnatune * ) m_MediaViewer )->DownloadAlbums( Albums );
}


// -------------------------------------------------------------------------------- //
// guMagnatuneDownloadThread
// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::guMagnatuneDownloadThread( guMediaViewerMagnatune * mediaviewer,
                const int albumid, const wxString &artist, const wxString &album )
{
    m_MediaViewer = mediaviewer;
    m_Db = ( guMagnatuneLibrary * ) mediaviewer->GetDb();
    m_ArtistName = artist;
    m_AlbumName = album;
    m_AlbumId = albumid;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::~guMagnatuneDownloadThread()
{
}

// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::ExitCode guMagnatuneDownloadThread::Entry()
{
    wxFileName CoverFile = wxFileName( guPATH_MAGNATUNE_COVERS +
                 wxString::Format( wxT( "%s-%s.jpg" ), m_ArtistName.c_str(), m_AlbumName.c_str() ) );

    if( CoverFile.Normalize( wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_CASE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT | wxPATH_NORM_ENV_VARS ) )
    {
        wxString CoverUrl = wxString::Format( wxT( "http://he3.magnatune.com/music/%s/%s/cover_600.jpg" ),
                guURLEncode( m_ArtistName, false ).c_str(),
                guURLEncode( m_AlbumName, false ).c_str() );

        if( !wxDirExists( wxPathOnly( CoverFile.GetFullPath() ) + wxT( "/" ) ) )
            wxMkdir( wxPathOnly( CoverFile.GetFullPath() ) + wxT( "/" ), 0770 );

        if( !CoverFile.FileExists() )
        {
            guLogMessage( wxT( "Downloading: '%s'" ), CoverUrl.c_str() );
            DownloadImage( CoverUrl, CoverFile.GetFullPath(), 300 );
        }

        if( CoverFile.FileExists() )
        {
            int CoverId = m_Db->AddCoverFile( CoverFile.GetFullPath() );

            wxString query = wxString::Format( wxT( "UPDATE songs SET song_coverid = %u WHERE song_albumid = %u" ),
                                CoverId, m_AlbumId );

            m_Db->ExecuteUpdate( query );

            // Notify the panel that the cover is downloaded
            wxCommandEvent event( wxEVT_MENU, ID_MAGNATUNE_COVER_DOWNLAODED );
            event.SetInt( m_AlbumId );
            wxPostEvent( m_MediaViewer, event );
        }
        else
            guLogMessage( wxT( "Could not get the magnatune cover art %s" ), CoverFile.GetFullPath().c_str() );

    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guMagnatuneUpdateThread
// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::guMagnatuneUpdateThread( guMediaViewer * mediaviewer, int action, int gaugeid )
{
    m_MediaViewer = mediaviewer;
    m_Db = ( guMagnatuneLibrary * ) mediaviewer->GetDb();
    m_MainFrame = mediaviewer->GetMainFrame();
    m_Action = action;
    m_GaugeId = gaugeid;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_AllowedGenres = Config->ReadAStr( CONFIG_KEY_MAGNATUNE_GENRES_GENRE, wxEmptyString, CONFIG_PATH_MAGNATUNE_GENRES );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::~guMagnatuneUpdateThread()
{
    wxCommandEvent event( wxEVT_MENU, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( m_MainFrame, event );

    if( !TestDestroy() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxDateTime Now = wxDateTime::Now();
            Config->WriteNum( CONFIG_KEY_MAGNATUNE_LAST_UPDATE, Now.GetTicks(), CONFIG_PATH_MAGNATUNE );
            Config->Flush();
        }

        event.SetId( ID_MAGNATUNE_UPDATE_FINISHED );
        event.SetEventObject( ( wxObject * ) this );
        wxPostEvent( m_MediaViewer, event );
    }
}

#if 0
<AllAlbums>
    <Album>
        <artist>Dr Kuch</artist>
		<artistdesc>Fun electro-poppy dance up and down tempo chill out</artistdesc>
		<cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
        <artistphoto>http://magnatune.com//artists/img/drkuch_cover_small.jpg</artistphoto>
		<albumname>We Can&#39;t Stop Progress</albumname>
		<year>2009</year>
		<album_notes></album_notes>
		<mp3genre>(52)</mp3genre>
		<home>http://magnatune.com/artists/dr_kuch</home>
		<buy>https://magnatune.com/artists/buy_album?artist=Dr+Kuch&amp;album=We+Can%27t+Stop+Progress&amp;genre=Electronica</buy>
		<magnatunegenres>Electronica,Euro-Techno</magnatunegenres>
		<launchdate>2009-10-08</launchdate>
		<albumsku>drkuch-progress</albumsku>
        <Track>
            <artist>Dr Kuch</artist>
            <artistphoto>http://magnatune.com//artists/img/drkuch_cover_small.jpg</artistphoto>
            <artistdesc>Fun electro-poppy dance up and down tempo chill out</artistdesc>
            <albumname>We Can&#39;t Stop Progress</albumname>
            <trackname>Intro</trackname>
            <tracknum>1</tracknum>
            <year>2009</year>
            <mp3genre>(52)</mp3genre>
            <magnatunegenres>Electronica,Euro-Techno</magnatunegenres>
            <license>http://creativecommons.org/licenses/by-nc-sa/1.0/</license>
            <seconds>18</seconds>
            <url>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch.mp3</url>
            <mp3lofi>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch-lofi.mp3</mp3lofi>
            <oggurl>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch.ogg</oggurl>
            <buy>https://magnatune.com/artists/buy_album?artist=Dr+Kuch&amp;album=We+Can%27t+Stop+Progress&amp;genre=Electronica</buy>
            <home>http://magnatune.com/artists/dr_kuch</home>
            <launchdate>2009-10-08</launchdate>
            <cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
            <albumsku>drkuch-progress</albumsku>
        </Track>
        ...
    </Album>
    ...
</AllAlbums>
#endif

// -------------------------------------------------------------------------------- //
bool inline IsGenreEnabled( const wxArrayString &genrelist, const wxString &current )
{
    wxArrayString CurrentGenres = wxStringTokenize( current, wxT( "," ) );
    int Count = CurrentGenres.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( genrelist.Index( CurrentGenres[ Index ] ) != wxNOT_FOUND )
            return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guMagnatuneUpdateThread::AddGenres( const wxString &genres )
{
    wxArrayString Genres = wxStringTokenize( genres, wxT( "," ) );
    int Count = Genres.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( m_GenreList.Index( Genres[ Index ] ) == wxNOT_FOUND )
            m_GenreList.Add( Genres[ Index ]  );
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneUpdateThread::ReadMagnatuneXmlTrack( wxXmlNode * xmlnode )
{
    long Id;
    while( xmlnode && !TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "trackname" ) )
            m_CurrentTrack.m_SongName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
        else if( ItemName == wxT( "tracknum" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Number = Id;
        }
        else if( ItemName == wxT( "year" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Year = Id;
        }
        else if( ItemName == wxT( "seconds" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Length = Id * 1000;
        }
        else if( ItemName == wxT( "url" ) )
        {
            m_CurrentTrack.m_FileName = xmlnode->GetNodeContent();
            m_CurrentTrack.m_FileName.Replace( wxT( ".mp3" ), wxT( "" ) );
        }
        else if( ItemName == wxT( "magnatunegenres" ) )
        {
            m_CurrentTrack.m_GenreName = xmlnode->GetNodeContent();
            m_CurrentTrack.m_GenreId = m_Db->GetGenreId( m_CurrentTrack.m_GenreName );
            AddGenres( m_CurrentTrack.m_GenreName );
        }
        else if( ItemName == wxT( "cover_small" ) )
        {
            //<cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
            m_CoverLink = xmlnode->GetNodeContent();
            m_CoverLink.Replace( wxT( "cover_200" ), wxT( "cover_600" ) );
        }
        else if( ItemName == wxT( "albumsku" ) )
            m_AlbumSku = xmlnode->GetNodeContent();

        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneUpdateThread::ReadMagnatuneXmlAlbum( wxXmlNode * xmlnode )
{
    long Id;
    while( xmlnode && !TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "artist" ) )
        {
            m_CurrentTrack.m_ArtistName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
            m_CurrentTrack.m_ArtistId = m_Db->GetArtistId( m_CurrentTrack.m_ArtistName );
        }
        else if( ItemName == wxT( "albumname" ) )
        {
            m_CurrentTrack.m_AlbumName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
            m_CurrentTrack.m_AlbumId = 0;
            //guLogMessage( wxT( "%s" ), m_CurrentTrack.m_AlbumName.c_str() );
        }
        else if( ItemName == wxT( "year" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Year = Id;
        }
        else if( ItemName == wxT( "magnatunegenres" ) )
        {
            m_CurrentTrack.m_GenreName = xmlnode->GetNodeContent();
            m_CurrentTrack.m_GenreId = m_Db->GetGenreId( m_CurrentTrack.m_GenreName );
            AddGenres( m_CurrentTrack.m_GenreName );
        }
        else if( ItemName == wxT( "Track" ) )
        {
            m_AlbumSku = wxEmptyString;
            m_CoverLink = wxEmptyString;
            if( !m_CurrentTrack.m_AlbumId )
            {
                m_CurrentTrack.m_Path = m_CurrentTrack.m_GenreName + wxT( "/" ) +
                                m_CurrentTrack.m_ArtistName + wxT( "/" ) +
                                m_CurrentTrack.m_AlbumName + wxT( "/" );
                m_CurrentTrack.m_PathId = m_Db->GetPathId( m_CurrentTrack.m_Path );
                m_CurrentTrack.m_AlbumId = m_Db->GetAlbumId( m_CurrentTrack.m_AlbumName, m_CurrentTrack.m_ArtistId, m_CurrentTrack.m_PathId, m_CurrentTrack.m_Path );
            }
            ReadMagnatuneXmlTrack( xmlnode->GetChildren() );

            if( IsGenreEnabled( m_AllowedGenres, m_CurrentTrack.m_GenreName ) )
                m_Db->CreateNewSong( &m_CurrentTrack, m_AlbumSku, m_CoverLink );
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guMagnatuneUpdateThread::UpgradeDatabase()
{
    if( DownloadFile( guMAGNATUNE_DATABASE_DUMP_URL, guPATH_MAGNATUNE wxT( "album_info_xml.gz" ) ) )
    {
        wxFileInputStream Ins( guPATH_MAGNATUNE wxT( "album_info_xml.gz" ) );
        if( Ins.IsOk() )
        {
            wxZlibInputStream ZIn( Ins );
            if( ZIn.IsOk() )
            {
                wxFileOutputStream ZOuts( guPATH_MAGNATUNE wxT( "album_info.xml" ) );
                if( ZOuts.IsOk() )
                {
                    ZIn.Read( ZOuts );
                    return true;
                }
            }
        }
        else
            guLogError( wxT( "Could not open the Magnatune local database copy" ) );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::ExitCode guMagnatuneUpdateThread::Entry()
{
    wxString query;

    wxCommandEvent evtup( wxEVT_MENU, ID_STATUSBAR_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_MENU, ID_STATUSBAR_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    if (m_Action == guMAGNATUNE_ACTION_UPDATE && !wxFileExists(guPATH_MAGNATUNE wxT("album_info.xml")))
        m_Action = guMAGNATUNE_ACTION_UPGRADE;

    guLogMessage( wxT( "Starting the Magnatune Update process..." ) );
    if( !TestDestroy() && ( m_Action == guMAGNATUNE_ACTION_UPDATE || UpgradeDatabase() ) )
    {
        wxFile XmlFile( guPATH_MAGNATUNE wxT( "album_info.xml" ), wxFile::read );
        if( XmlFile.IsOpened() )
        {
            guListItems CurrentGenres;
            m_Db->GetGenres( &CurrentGenres, true );

            wxArrayInt GenresToDel;
            int Count = CurrentGenres.Count();
            for( int Index = 0; Index < Count; Index++ )
            {
                if( !IsGenreEnabled( m_AllowedGenres, CurrentGenres[ Index ].m_Name ) )
                    GenresToDel.Add( CurrentGenres[ Index ].m_Id );
            }

            query = wxT( "BEGIN TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            if( GenresToDel.Count() )
            {
                query = wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( GenresToDel, wxT( "song_genreid" ) );
                //guLogMessage( wxT( "%s" ), query.c_str() );
                m_Db->ExecuteUpdate( query );
            }

            evtmax.SetExtraLong( XmlFile.Length() );
            wxPostEvent( guMainFrame::GetMainFrame(), evtmax );

            wxFileOffset CurPos;

            if( m_AllowedGenres.Count() )
            {
                wxString AlbumChunk = guGetNextXMLChunk( XmlFile, CurPos, "<Album>", "</Album>", wxConvISO8859_1 );
                while( !TestDestroy() && !AlbumChunk.IsEmpty() )
                {
//                    wxHtmlEntitiesParser EntitiesParser;
//                    AlbumChunk = EntitiesParser.Parse( AlbumChunk );
//                    AlbumChunk.Replace( wxT( "&" ), wxT( "&amp;" ) );

                    wxStringInputStream Ins( AlbumChunk );
                    wxXmlDocument XmlDoc( Ins );
                    if( XmlDoc.IsOk() )
                    {
                        wxXmlNode * XmlNode = XmlDoc.GetRoot();
                        if( XmlNode && XmlNode->GetName() == wxT( "Album" ) )
                            ReadMagnatuneXmlAlbum( XmlNode->GetChildren() );
                    }
                    else
                        guLogMessage( wxT( "Error in album chunk:\n%s" ), AlbumChunk.c_str() );

                    AlbumChunk = guGetNextXMLChunk( XmlFile, CurPos, "<Album>", "</Album>", wxConvISO8859_1 );

                    //guLogMessage( wxT( "Chunk : '%s ... %s'" ), AlbumChunk.Left( 35 ).c_str(), AlbumChunk.Right( 35 ).c_str() );

                    evtup.SetExtraLong( CurPos );
                    wxPostEvent( guMainFrame::GetMainFrame(), evtup );
                }

                if( m_GenreList.Count() )
                {
                    guConfig * Config = ( guConfig * ) guConfig::Get();
                    Config->WriteAStr( CONFIG_KEY_MAGNATUNE_GENRES_GENRE, m_GenreList, CONFIG_PATH_MAGNATUNE_GENRELIST );
                    Config->Flush();
                }
            }

            query = wxT( "END TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            XmlFile.Close();
        }
    }

    return 0;
}


// -------------------------------------------------------------------------------- //
// guMediaViewerMagnatune
// -------------------------------------------------------------------------------- //
guMediaViewerMagnatune::guMediaViewerMagnatune( wxWindow * parent, guMediaCollection &mediacollection,
        const int basecmd, guMainFrame * mainframe, const int mode, guPlayerPanel * playerpanel ) :
    guMediaViewer( parent, mediacollection, basecmd, mainframe, mode, playerpanel )
{
//    m_DownloadThread = nullptr;
    m_ContextMenuFlags = ( guCONTEXTMENU_DOWNLOAD_COVERS | guCONTEXTMENU_LINKS );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_Membership = Config->ReadNum( CONFIG_KEY_MAGNATUNE_MEMBERSHIP, 0, CONFIG_PATH_MAGNATUNE );
    m_UserName = Config->ReadStr( CONFIG_KEY_MAGNATUNE_USERNAME, wxEmptyString, CONFIG_PATH_MAGNATUNE );
    m_Password = Config->ReadStr( CONFIG_KEY_MAGNATUNE_PASSWORD, wxEmptyString, CONFIG_PATH_MAGNATUNE );

    Bind( wxEVT_MENU, &guMediaViewerMagnatune::OnCoverDownloaded, this, ID_MAGNATUNE_COVER_DOWNLAODED  );
    Bind( wxEVT_MENU, &guMediaViewerMagnatune::OnUpdateFinished, this, ID_MAGNATUNE_UPDATE_FINISHED  );

    InitMediaViewer( mode );
}

// -------------------------------------------------------------------------------- //
guMediaViewerMagnatune::~guMediaViewerMagnatune()
{
    Unbind( wxEVT_MENU, &guMediaViewerMagnatune::OnCoverDownloaded, this, ID_MAGNATUNE_COVER_DOWNLAODED  );
    Unbind( wxEVT_MENU, &guMediaViewerMagnatune::OnUpdateFinished, this, ID_MAGNATUNE_UPDATE_FINISHED  );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::LoadMediaDb()
{
    guLogMessage( wxT( "LoadMediaDb... MAGNATUNE..." ) );
    m_Db = new guMagnatuneLibrary( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId + wxT( "/guayadeque.db" ) );
    m_Db->SetMediaViewer( this );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::OnConfigUpdated( wxCommandEvent &event )
{
    guMediaViewer::OnConfigUpdated( event );

    if( event.GetInt() & guPREFERENCE_PAGE_FLAG_MAGNATUNE )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_Membership = Config->ReadNum( CONFIG_KEY_MAGNATUNE_MEMBERSHIP, guMAGNATUNE_MEMBERSHIP_FREE, CONFIG_PATH_MAGNATUNE );
        m_UserName = Config->ReadStr( CONFIG_KEY_MAGNATUNE_USERNAME, wxEmptyString, CONFIG_PATH_MAGNATUNE );
        m_Password = Config->ReadStr( CONFIG_KEY_MAGNATUNE_PASSWORD, wxEmptyString, CONFIG_PATH_MAGNATUNE );
        if( m_UserName.IsEmpty() || m_Password.IsEmpty() )
            m_Membership = guMAGNATUNE_MEMBERSHIP_FREE;

        if( Config->ReadBool( CONFIG_KEY_MAGNATUNE_NEED_UPGRADE, false, CONFIG_PATH_MAGNATUNE ) )
            UpdateLibrary();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::UpdateLibrary()
{
    int GaugeId;
    GaugeId = m_MainFrame->AddGauge( m_MediaCollection->m_Name );

    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guMagnatuneUpdateThread( this, guMAGNATUNE_ACTION_UPDATE, GaugeId );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::UpgradeLibrary()
{
    int GaugeId;
    GaugeId = m_MainFrame->AddGauge( m_MediaCollection->m_Name );

    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guMagnatuneUpdateThread( this, guMAGNATUNE_ACTION_UPGRADE, GaugeId );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Count;
    if( tracks && ( Count = tracks->Count() ) )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        int AudioFormat = Config->ReadNum( CONFIG_KEY_MAGNATUNE_AUDIO_FORMAT, 1, CONFIG_PATH_MAGNATUNE );
        for( int Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = &( * tracks )[ Index ];
            //guLogMessage( wxT( "'%s'" ), Track->m_FileName.c_str() );
            guLogMessage( wxT( "The AlbumName: '%s'" ), Track->m_AlbumName.c_str() );
            if( !Track->m_FileName.EndsWith( AudioFormat ? guMAGNATUNE_STREAM_FORMAT_OGG : guMAGNATUNE_STREAM_FORMAT_MP3 ) )
            {
                Track->m_FileName = Track->m_FileName.Mid( Track->m_FileName.Find( wxT( "http://he3." ) ) );
                if( m_Membership > guMAGNATUNE_MEMBERSHIP_FREE ) // Streaming or Downloading
                {
                    Track->m_FileName.Replace( wxT( "//he3." ), wxT( "//" ) + m_UserName + wxT( ":" ) +
                        m_Password + ( ( m_Membership == guMAGNATUNE_MEMBERSHIP_STREAM ) ? wxT( "@stream." ) : wxT( "@download." ) ) );

                    Track->m_FileName += wxT( "_nospeech" );
                }

                Track->m_FileName += AudioFormat ? guMAGNATUNE_STREAM_FORMAT_OGG : guMAGNATUNE_STREAM_FORMAT_MP3;
                Track->m_Type = guTRACK_TYPE_MAGNATUNE;
                if( isdrag )
                    Track->m_FileName.Replace( wxT( "http://" ), wxT( "/" ) );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::AddDownload( const int albumid, const wxString &artist, const wxString &album )
{
    guMagnatuneDownloadThread * DownloadThread = new guMagnatuneDownloadThread( this, albumid, artist, album );

    if( !DownloadThread )
        guLogMessage( wxT( "Could not create the magnatune download thread" ) );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::OnCoverDownloaded( wxCommandEvent &event )
{
    AlbumCoverChanged( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::EndUpdateThread()
{
    m_UpdateThread = nullptr;
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::OnUpdateFinished( wxCommandEvent &event )
{
    EndUpdateThread();
    LibraryUpdated();
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::DownloadAlbumCover( const int albumid )
{
    wxString Artist;
    wxString Album;
    if( m_Db->GetAlbumInfo( albumid, &Album, &Artist, nullptr ) )
    {
        guLogMessage( wxT( "Starting download for %i '%s' '%s'" ), albumid, Artist.c_str(), Album.c_str() );
        AddDownload( albumid, Artist, Album );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu * Menu = new wxMenu();
    wxMenuItem * MenuItem;

    int BaseCommand = GetBaseCommand();

    if( m_Membership == guMAGNATUNE_MEMBERSHIP_DOWNLOAD )
    {
        if( ( windowid == guLIBRARY_ELEMENT_ALBUMS ) )
        {
            MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
            Menu->Append( MenuItem );
            Menu->AppendSeparator();
        }
        else if( ( windowid == guLIBRARY_ELEMENT_TRACKS ) )
        {
            MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
            Menu->Append( MenuItem );
            Menu->AppendSeparator();
        }
    }

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY, _( "Update" ), _( "Update the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_RESCAN_LIBRARY, _( "Rescan" ), _( "Rescan the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_SEARCH_COVERS, _( "Search Covers" ), _( "Search the collection missing covers" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_VIEW_PROPERTIES, _( "Properties" ), _( "Show collection properties" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( Menu, wxT( "Magnatune" ) );
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerMagnatune::CreateLibraryView()
{
    guLogMessage( wxT( "CreateLibraryView... Magnatune...") );
    m_LibPanel = new guMagnatunePanel( this, this );
    m_LibPanel->SetBaseCommand( m_BaseCommand + 1 );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerMagnatune::CreateAlbumBrowserView()
{
    m_AlbumBrowser = new guMagnatuneAlbumBrowser( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerMagnatune::CreateTreeView()
{
    m_TreeViewPanel = new guMagnatuneTreePanel( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerMagnatune::CreatePlayListView()
{
    m_PlayListPanel = new guMagnatunePlayListPanel( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMediaViewerMagnatune::GetAlbumCover( const int albumid, int &coverid,
            wxString &coverpath, const wxString &artistname, const wxString &albumname )
{
    wxImage * CoverImage = guMediaViewer::GetAlbumCover( albumid, coverid, coverpath, artistname, albumname );
    if( CoverImage )
        return CoverImage;

    wxFileName CoverFile = wxFileName( guPATH_MAGNATUNE_COVERS +
                 wxString::Format( wxT( "%s-%s.jpg" ), artistname.c_str(), albumname.c_str() ) );

    if( CoverFile.Normalize( wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_CASE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT | wxPATH_NORM_ENV_VARS ) )
    {
        if( CoverFile.FileExists() )
        {
            wxImage * CoverImage = new wxImage( CoverFile.GetFullPath(), wxBITMAP_TYPE_JPEG );
            if( CoverImage )
            {
                if( CoverImage->IsOk() )
                {
                    coverpath = CoverFile.GetFullPath();
                    SetAlbumCover( albumid, guPATH_MAGNATUNE_COVERS, coverpath );
                    coverid = m_Db->GetAlbumCoverId( albumid );

                    return CoverImage;
                }
                delete CoverImage;
            }
        }
    }

    AddDownload( albumid, artistname, albumname );

    return nullptr;
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewerMagnatune::GetCoverName( const int albumid )
{
    wxString Artist;
    wxString Album;
    if( m_Db->GetAlbumInfo( albumid, &Album, &Artist, nullptr ) )
        return wxString::Format( wxT( "%s-%s" ), Artist.c_str(), Album.c_str() );
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::SelectAlbumCover( const int albumid )
{
    guSelCoverFile * SelCoverFile = new guSelCoverFile( this, this, albumid );
    if( SelCoverFile )
    {
        if( SelCoverFile->ShowModal() == wxID_OK )
        {
            wxString CoverFile = SelCoverFile->GetSelFile();
            if( !CoverFile.IsEmpty() )
            {
                if( SetAlbumCover( albumid, guPATH_MAGNATUNE_COVERS, CoverFile ) )
                {
                    if( SelCoverFile->EmbedToFiles() )
                        EmbedAlbumCover( albumid );
                }
            }
        }
        SelCoverFile->Destroy();
    }
}

//<RESULT>
//  <CC_AMOUNT>$</CC_AMOUNT>
//  <CC_TRANSID></CC_TRANSID>
//  <DL_PAGE>http://download.magnatune.com/artists/albums/satori-rainsleep/download</DL_PAGE>
//  <URL_WAVZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=wav.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/wav.zip</URL_WAVZIP>
//  <URL_128KMP3ZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=mp3.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/mp3.zip</URL_128KMP3ZIP>
//  <URL_OGGZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=Satori%20-%20Sleep%20Under%20The%20Rain%20-%20ogg.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/satori-rainsleep-ogg.zip</URL_OGGZIP>
//  <URL_VBRZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=Satori%20-%20Sleep%20Under%20The%20Rain%20-%20vbr.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/satori-rainsleep-vbr.zip</URL_VBRZIP>
//  <URL_FLACZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=Satori%20-%20Sleep%20Under%20The%20Rain%20-%20flac.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/satori-rainsleep-flac.zip</URL_FLACZIP>
//  <DL_MSG>
//Go here to download your music:
//http://download.magnatune.com/artists/albums/satori-rainsleep/download
//----------
//Give this album away for free!
//You can pass these download instructions on to 3 friends. It's completely legal
//and you'll be helping us fight the evil music industry!  Complete details here:
//http://magnatune.com/info/give
//Help spread the word of Magnatune, with our free recruiting cards!
//http://magnatune.com/cards
//Podcasters, use our music:<br>
//http://magnatune.com/info/podcast
//To send us email:
//https://magnatune.com/info/email_us
//  </DL_MSG>
//</RESULT>

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::DownloadAlbums( const wxArrayInt &albumids )
{
    wxString StartLabel[] = {
        wxT( "<DL_PAGE>" ),
        wxT( "<URL_VBRZIP>" ),
        wxT( "<URL_128KMP3ZIP>" ),
        wxT( "<URL_OGGZIP>" ),
        wxT( "<URL_FLACZIP>" ),
        wxT( "<URL_WAVZIP>" ),
    };

    int Count;
    if( ( Count = albumids.Count() ) )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        int DownloadFormat = Config->ReadNum( CONFIG_KEY_MAGNATUNE_DOWNLOAD_FORMAT, 0, CONFIG_PATH_MAGNATUNE );
        //guLogMessage( wxT( "DownloadFormat: %i %s" ), DownloadFormat, StartLabel[ DownloadFormat ].c_str() );
        if( ( DownloadFormat < 0 ) || ( DownloadFormat > 5 ) )
            DownloadFormat = 0;

        for( int Index = 0; Index < Count; Index++ )
        {
            wxString DownloadUrl = wxString::Format( guMAGNATUNE_DOWNLOAD_URL,
                                m_UserName.c_str(), m_Password.c_str(),
                                ( ( guMagnatuneLibrary * ) m_Db )->GetAlbumSku( albumids[ Index ] ).c_str() );

            //guLogMessage( wxT( "Trying to download %s" ), DownloadUrl.c_str() );
            wxString Content = GetUrlContent( DownloadUrl );
            if( !Content.IsEmpty() )
            {
                //guLogMessage( wxT( "Result:\n%s" ), Content.c_str() );

                DownloadUrl = DoExtractTag( Content, StartLabel[ DownloadFormat ] );
                //guLogMessage( wxT( "Extracted url: %s" ), DownloadUrl.c_str() );

                if( !DownloadUrl.IsEmpty() )
                {
                    DownloadUrl = wxString::Format( wxT( "http://%s:%s@" ), m_UserName.c_str(), m_Password.c_str() ) + DownloadUrl.Mid( 7 );
                    //guLogMessage( wxT( "Trying to download the url : '%s'" ), DownloadUrl.c_str() );
                    guWebExecute( DownloadUrl );
                }
            }
            else
                guLogError( wxT( "Could not get the album download info" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerMagnatune::FindMissingCover( const int albumid, const wxString &artistname, const wxString &albumname, const wxString &albumpath )
{
    AddDownload( albumid, artistname, albumname );
    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaViewerMagnatune::EditProperties()
{
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_MAGNATUNE );
    wxPostEvent( m_MainFrame, CmdEvent );
}

}
