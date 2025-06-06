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
#include "PlayListFile.h"

#include "Utils.h"
#include "TagInfo.h"

#include <wx/arrimpl.cpp>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

WX_DEFINE_OBJARRAY( guPlaylistItemArray )
WX_DEFINE_OBJARRAY( guCuePlaylistItemArray )

// -------------------------------------------------------------------------------- //
// guPlaylistItem
// -------------------------------------------------------------------------------- //
wxString guPlaylistItem::GetLocation( const bool relative, const wxString &pathbase )
{
    if( relative )
    {
        wxFileName FileName( m_Location );
        if( FileName.MakeRelativeTo( pathbase ) )
            return FileName.GetFullPath();
    }
    return m_Location;
}


// -------------------------------------------------------------------------------- //
// guPlaylistFile
// -------------------------------------------------------------------------------- //
guPlaylistFile::guPlaylistFile( const wxString &uri )
{
    if( !uri.IsEmpty() )
        Load( uri );
}

// -------------------------------------------------------------------------------- //
guPlaylistFile::~guPlaylistFile()
{
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::IsValidPlayList( const wxString &uri )
{
    wxURI Uri( uri );

    wxString PLPath = Uri.GetPath().Lower();

    return  PLPath.EndsWith( wxT( ".pls" ) ) ||
            PLPath.EndsWith( wxT( ".m3u" ) ) ||
            PLPath.EndsWith( wxT( ".m3u8" ) ) ||
            PLPath.EndsWith( wxT( ".xspf" ) ) ||
            PLPath.EndsWith( wxT( ".asx" ) );
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::Load( const wxString &uri )
{
    guLogMessage( wxT( "guPlaylistFile::Load '%s'" ), uri.c_str() );
    wxURI Uri( uri );
    wxString LowerPath = Uri.GetPath().Lower();

    //guLogMessage( wxT( "'%s' IsReference:%i" ), uri.c_str(), Uri.IsReference() );
    if( Uri.IsReference() )
    {
        wxString FileName;
        if( uri.StartsWith( wxT( "file://" ) ) )
            FileName = wxURI::Unescape( Uri.GetPath() );
        else
            FileName = uri;

        if( LowerPath.EndsWith( wxT( ".pls" ) ) )
            return ReadPlsFile( FileName );
        else if( LowerPath.EndsWith( wxT( ".m3u" ) ) || LowerPath.EndsWith( wxT( ".m3u8" ) ) )
            return ReadM3uFile( FileName );
        else if( LowerPath.EndsWith( wxT( ".xspf" ) ) )
            return ReadXspfFile( FileName );
        else if( LowerPath.EndsWith( wxT( ".asx" ) ) )
            return ReadAsxFile( FileName );
    }
    else
    {
        if( LowerPath.EndsWith( wxT( ".ashx" ) ) )
        {
            wxString Content = GetUrlContent( uri );
            guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
            wxArrayString PlaylistItems = wxStringTokenize( Content, wxT( "\n" ) );
            int Count = PlaylistItems.Count();
            for( int Index = 0; Index < Count; Index++ )
            {
                Load( PlaylistItems[ Index ] );
            }
        }
        else if( !IsValidPlayList( LowerPath ) )
        {
            m_Playlist.Add( new guPlaylistItem( uri, uri ) );
            return true;
        }
        else
        {
            guLogMessage( wxT( "Trying to get the uri: %s" ), uri.c_str() );
            wxString Content = GetUrlContent( uri );
            if( !Content.IsEmpty() )
            {
                wxStringInputStream Ins( Content );

                if( LowerPath.EndsWith( wxT( ".pls" ) ) )
                    return ReadPlsStream( Ins );
                else if( LowerPath.EndsWith( wxT( ".m3u" ) ) || LowerPath.EndsWith( wxT( ".m3u8" ) ) )
                    return ReadM3uStream( Ins );
                else if( LowerPath.EndsWith( wxT( ".xspf" ) ) )
                    return ReadXspfStream( Ins );
                else if( LowerPath.EndsWith( wxT( ".asx" ) ) )
                    return ReadAsxStream( Ins );
            }
            else
                guLogError( wxT( "Could not get the playlist '%s'" ), uri.c_str() );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::Save( const wxString &filename, const bool relative )
{
    if( filename.Lower().EndsWith( wxT( ".pls" ) ) )
        return WritePlsFile( filename, relative );
    else if( filename.Lower().EndsWith( wxT( ".xspf" ) ) )
        return WriteXspfFile( filename, relative );
    else if( filename.Lower().EndsWith( wxT( ".asx" ) ) )
        return WriteAsxFile( filename, relative );
    else
    {
        wxString FileName = filename;
        if( !FileName.Lower().EndsWith( wxT( ".m3u" ) ) )
            FileName += wxT( ".m3u" );
        return WriteM3uFile( FileName, relative );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadPlsStream( wxInputStream &playlist, const wxString &path )
{
    wxFileConfig * PlayList = new wxFileConfig( playlist );
    if( PlayList )
    {
        if( PlayList->HasGroup( wxT( "playlist" ) ) )
        {
            PlayList->SetPath( wxT( "playlist" ) );
            int Count;
            if( PlayList->Read( wxT( "numberofentries" ), &Count ) )
            {
                guLogMessage( wxT( "Found a playlist with %i items" ), Count );
                if( !Count )
                    guLogMessage( wxT( "This station playlist is empty" ) );
                else
                {
                    for( int Index = 1; Index <= Count; Index++ )
                    {
                        wxString Location;
                        wxString Title;

                        if( PlayList->Read( wxString::Format( wxT( "File%u" ), Index ), &Location ) &&
                            !Location.IsEmpty() )
                        {
                            PlayList->Read( wxString::Format( wxT( "Title%u" ), Index ), &Title );

                            wxURI Uri( Location );
                            if( Location.StartsWith( wxT( "/" ) ) || Uri.HasScheme() || path.IsEmpty() )
                                m_Playlist.Add( new guPlaylistItem( Location, Title ) );
                            else
                            {
                                wxFileName FileName( Location );
                                FileName.Normalize( wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_CASE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT | wxPATH_NORM_ENV_VARS, path );

                                m_Playlist.Add( new guPlaylistItem( FileName.GetFullPath(), Title ) );
                            }
                        }
                    }
                    return true;
                }
            }
        }
        else
            guLogError( wxT( "ee: Station Playlist without 'playlist' group" ) );

        delete PlayList;
    }
    else
        guLogError( wxT( "Could not read the playlist stream" ) );

    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadPlsFile( const wxString &filename )
{
    guLogMessage( wxT( "ReadPlsFile( '%s' )" ), filename.c_str() );
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
        return ReadPlsStream( Ins, wxPathOnly( filename ) );
    else
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );

    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadM3uStream( wxInputStream &playlist, const wxString &path )
{
    wxString M3uFile;
    wxStringOutputStream Outs( &M3uFile );
    playlist.Read( Outs );

    if( !M3uFile.IsEmpty() )
    {
        //guLogMessage( wxT( "Content...\n%s" ), M3uFile.c_str() );
        wxString ItemName;
        wxArrayString Lines = wxStringTokenize( M3uFile, wxT( "\n" ) );

        int Count = Lines.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            Lines[ Index ].Trim( false ).Trim( true );
            if( Lines[ Index ].IsEmpty() || ( Lines[ Index ].Find( wxT( "#EXTM3U" ) ) != wxNOT_FOUND ) )
                continue;
            else if( Lines[ Index ].Find( wxT( "#EXTINF" ) ) != wxNOT_FOUND )
            {
                if( Lines[ Index ].Find( wxT( "," ) ) != wxNOT_FOUND )
                    ItemName = Lines[ Index ].AfterLast( wxT( ',' ) );
            }
            else
            {
                wxURI Uri( Lines[ Index ] );
                if( Lines[ Index ].StartsWith( wxT( "/" ) ) || Uri.HasScheme() || path.IsEmpty() )
                    m_Playlist.Add( new guPlaylistItem( Lines[ Index ], ItemName ) );
                else
                {
                    wxFileName FileName( Lines[ Index ] );
                    FileName.Normalize( wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_CASE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT | wxPATH_NORM_ENV_VARS, path );
                    m_Playlist.Add( new guPlaylistItem( FileName.GetFullPath(), ItemName ) );
                }

                //guLogMessage( wxT( "%s - %s -> %s" ), path.c_str(), Lines[ Index ].c_str(), ( FileName.GetPathWithSep() + FileName.GetFullName() ).c_str() );
                ItemName = wxEmptyString;
            }
        }
        return true;
    }
    else
        guLogError( wxT( "Empty playlist file stream" ) );

    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadM3uFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
        return ReadM3uStream( Ins, wxPathOnly( filename ) );
    else
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );

    return false;
}

// -------------------------------------------------------------------------------- //
void guPlaylistFile::ReadXspfTrack( wxXmlNode * XmlNode )
{
    wxString Title;
    wxString Location;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
            Title = XmlNode->GetNodeContent();
        else if( XmlNode->GetName().Lower() == wxT( "location" ) )
            Location = XmlNode->GetNodeContent();
        XmlNode = XmlNode->GetNext();
    }

    if( !Location.IsEmpty() )
        m_Playlist.Add( new guPlaylistItem( Location, Title ) );
}

// -------------------------------------------------------------------------------- //
void guPlaylistFile::ReadXspfTrackList( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "track" ) )
            ReadXspfTrack( XmlNode->GetChildren() );

        XmlNode = XmlNode->GetNext();
    }

}

// -------------------------------------------------------------------------------- //
void guPlaylistFile::ReadXspfPlayList( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
            m_Name = XmlNode->GetNodeContent();
        else if( XmlNode->GetName().Lower() == wxT( "tracklist" ) )
            ReadXspfTrackList( XmlNode->GetChildren() );

        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadXspfStream( wxInputStream &playlist )
{
    wxXmlDocument XmlDoc( playlist );
    wxXmlNode * XmlNode = XmlDoc.GetRoot();
    if( XmlNode && XmlNode->GetName().Lower() == wxT( "playlist" ) )
    {
        ReadXspfPlayList( XmlNode->GetChildren() );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadXspfFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
        return ReadXspfStream( Ins );
    else
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );

    return false;
}

// -------------------------------------------------------------------------------- //
void guPlaylistFile::ReadAsxEntry( wxXmlNode * XmlNode )
{
    wxString Title;
    wxString Location;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
            Title = XmlNode->GetNodeContent();
        else if( XmlNode->GetName().Lower() == wxT( "ref" ) )
            XmlNode->GetAttribute( wxT( "href" ), &Location );
        XmlNode = XmlNode->GetNext();
    }
    if( !Location.IsEmpty() )
        m_Playlist.Add( new guPlaylistItem( Location, Title ) );
}

// -------------------------------------------------------------------------------- //
void guPlaylistFile::ReadAsxPlayList( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
            m_Name = XmlNode->GetNodeContent();
        else if( XmlNode->GetName().Lower() == wxT( "entry" ) )
            ReadAsxEntry( XmlNode->GetChildren() );

        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadAsxStream( wxInputStream &playlist )
{
    wxXmlDocument XmlDoc( playlist );
    wxXmlNode * XmlNode = XmlDoc.GetRoot();
    if( XmlNode && XmlNode->GetName().Lower() == wxT( "asx" ) )
    {
        ReadAsxPlayList( XmlNode->GetChildren() );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::ReadAsxFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
        return ReadAsxStream( Ins );
    else
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );

    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::WritePlsFile( const wxString &filename, const bool relative )
{
    wxFile PlsFile;
    if( PlsFile.Create( filename, true ) && PlsFile.IsOpened() )
    {
        PlsFile.Write( wxT( "[playlist]\n" ) );
        int Count = m_Playlist.Count();
        PlsFile.Write( wxString::Format( wxT( "numberofentries=%u\n" ), Count ) );
        for( int Index = 0; Index < Count; Index++ )
        {
            PlsFile.Write( wxString::Format( wxT( "File%u=%s\n" ), Index + 1,
                                m_Playlist[ Index ].GetLocation( relative, wxPathOnly( filename ) ).c_str() ) );
            PlsFile.Write( wxString::Format( wxT( "Title%u=%s\n" ), Index + 1,
                                m_Playlist[ Index ].m_Name.c_str() ) );
        }
        PlsFile.Close();
    }
    else
        guLogError( wxT( "Could not open the plsfile '%s'" ), filename.c_str() );

    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::WriteM3uFile( const wxString &filename, const bool relative )
{
    wxFile M3uFile;
    if( M3uFile.Create( filename, true ) && M3uFile.IsOpened() )
    {
        M3uFile.Write( wxT( "#EXTM3U\n" ) );
        int Count = m_Playlist.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            M3uFile.Write( wxString::Format( wxT( "#EXTINF:%i,%s\n" ), m_Playlist[ Index ].m_Length, m_Playlist[ Index ].m_Name.c_str() ) );
            M3uFile.Write( m_Playlist[ Index ].GetLocation( relative, wxPathOnly( filename ) ) );
            M3uFile.Write( wxT( "\n" ) );
        }
        M3uFile.Close();
    }
    else
        guLogError( wxT( "Could not open the m3ufile '%s'" ), filename.c_str() );

    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::WriteXspfFile( const wxString &filename, const bool relative )
{
    wxXmlDocument OutXml;
    //OutXml.SetRoot(  );
    wxXmlNode * RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "playlist" ) );

    wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
    wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_Name );
    TitleNode->AddChild( TitleNodeVal );
    RootNode->AddChild( TitleNode );

    wxXmlNode * TrackListNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "trackList" ) );

    int Count = m_Playlist.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        wxXmlNode * TrackNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "track" ) );

        wxXmlNode * LocationNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "location" ) );
        wxXmlNode * LocationNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "location" ), m_Playlist[ Index ].GetLocation( relative, wxPathOnly( filename ) ) );
        LocationNode->AddChild( LocationNodeVal );

        wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
        wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_Playlist[ Index ].m_Name );
        TitleNode->AddChild( TitleNodeVal );

        TrackNode->AddChild( LocationNode );
        TrackNode->AddChild( TitleNode );

        TrackListNode->AddChild( TrackNode );
    }
    RootNode->AddChild( TrackListNode );
    OutXml.SetRoot( RootNode );
    return OutXml.Save( filename );
}

// -------------------------------------------------------------------------------- //
bool guPlaylistFile::WriteAsxFile( const wxString &filename, const bool relative )
{
    wxXmlDocument OutXml;
    //OutXml.SetRoot(  );
    wxXmlNode * RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "asx" ) );

    wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
    wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_Name );
    TitleNode->AddChild( TitleNodeVal );
    RootNode->AddChild( TitleNode );

    int Count = m_Playlist.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        wxXmlNode * EntryNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "entry" ) );

        wxXmlNode * RefNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "ref" ) );
        wxXmlAttribute * RefNodeVal = new wxXmlAttribute( wxT( "href" ), m_Playlist[ Index ].GetLocation( relative, wxPathOnly( filename ) ), NULL );
        RefNode->SetAttributes( RefNodeVal );

        wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
        wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_Playlist[ Index ].m_Name );
        TitleNode->AddChild( TitleNodeVal );

        EntryNode->AddChild( RefNode );
        EntryNode->AddChild( TitleNode );

        RootNode->AddChild( EntryNode );
    }
    OutXml.SetRoot( RootNode );
    return OutXml.Save( filename );
}


// -------------------------------------------------------------------------------- //
// guCuePlaylistFile
// -------------------------------------------------------------------------------- //
guCuePlaylistFile::guCuePlaylistFile( const wxString &location )
{
    m_Location = location;
    m_TrackLength = 0;

    Load( location );
}

// -------------------------------------------------------------------------------- //
unsigned int RedBookToMTime( const wxString &rbtime )
{
    guLogMessage( wxT( "RedBookToMTime( '%s' )" ), rbtime.c_str() );
    // 01:23:45
    wxArrayString Sections = wxStringTokenize( rbtime, wxT( ":" ) );
    if( Sections.Count() != 3 )
        return 0;

    unsigned long Mins;
    unsigned long Secs;
    unsigned long Frames;
    Sections[ 0 ].ToULong( &Mins );
    Sections[ 1 ].ToULong( &Secs );
    Sections[ 2 ].ToULong( &Frames );

    return ( Mins * 60 * 1000 ) +
           ( Secs * 1000 ) +
           ( Frames * 1000 / 75 );
}

// -------------------------------------------------------------------------------- //
inline wxString RemoveQuotationMark( const wxString &text )
{
    //guLogMessage( wxT( "RemoveQuotationMark: '%s'" ), text.c_str() );
    if( text.StartsWith( wxT( "\"" ) ) )
        return text.Mid( 1, text.Length() - 2 );

    return text;
}

// -------------------------------------------------------------------------------- //
inline wxString GetKeyValue( const wxString &line, const wxString &key )
{
    //guLogMessage( wxT( "GetKeyValue: '%s' ==> '%s'" ), line.c_str(), key.c_str() );
    int Pos = line.Find( key );
    if( Pos != wxNOT_FOUND )
        return line.Mid( Pos + key.Length() ).Strip( wxString::both );

    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guCuePlaylistFile::LoadFromText( const wxString &content )
{
    if (content.IsEmpty())
    {
        guLogError( wxT( "Empty Cue sheet '%s'" ), m_Location.c_str() );
        return false;
    }

    wxArrayString lines = wxStringTokenize( content, wxT( "\n" ) );
    int currentTrack = wxNOT_FOUND;
    int fileTrack = wxNOT_FOUND;
    int count = lines.Count();
    int index;
    m_CueFiles.Empty();

    for (index = 0; index < count; index++)
    {
        lines[index].Trim(false).Trim(true);
        wxString line = lines[index];
        //guLogMessage( wxT( "'%s'" ), Line.c_str() );
        wxArrayString Keys = wxStringTokenize(line, wxT(" "));

        if (!Keys.Count())
            continue;

        if (Keys[0] == wxT("FILE"))
        {
            // Set the length of the last track of the previous file
            if (currentTrack != wxNOT_FOUND)
                m_PlaylistItems[currentTrack].m_Length = m_TrackLength - m_PlaylistItems[currentTrack].m_Start;

            m_TrackPath = RemoveQuotationMark(GetKeyValue(line, wxT("FILE")).BeforeLast(wxT(' ')));
            if (!m_TrackPath.StartsWith(wxT("/")))
                m_TrackPath = wxPathOnly(m_Location) + wxT("/") + m_TrackPath;

            m_CueFiles.Add(m_TrackPath);
            fileTrack = wxNOT_FOUND;

            guTagInfo *TagInfo = guGetTagInfoHandler(m_TrackPath);
            if (TagInfo)
            {
                if (TagInfo->Read())
                {
                    m_TrackLength = TagInfo->m_Length;
                    if (!TagInfo->m_GenreName.IsEmpty())
                        m_Genre = TagInfo->m_GenreName;
                    if (!TagInfo->m_AlbumName.IsEmpty())
                        m_AlbumName = TagInfo->m_AlbumName;
                    if (TagInfo->m_Year)
                        m_Year = wxString::Format("%i", TagInfo->m_Year);
                }
                delete TagInfo;
            }
        }
        else if (Keys[0] == wxT("TRACK"))
        {
            m_PlaylistItems.Add(new guCuePlaylistItem());
            fileTrack++;
            currentTrack++;
            guCuePlaylistItem &PlaylistItem  = m_PlaylistItems[currentTrack];
            PlaylistItem.m_Genre = m_Genre;
            PlaylistItem.m_AlbumName = m_AlbumName;
            PlaylistItem.m_Comment = m_Comment;
            PlaylistItem.m_ArtistName = m_ArtistName;
            PlaylistItem.m_Year = m_Year;
            PlaylistItem.m_TrackPath = m_TrackPath;
        }
        else if (Keys[0] == wxT("TITLE"))
        {
            if (fileTrack == wxNOT_FOUND)
                m_AlbumName = RemoveQuotationMark(GetKeyValue(line, wxT("TITLE")));
            else
                m_PlaylistItems[currentTrack].m_Name = RemoveQuotationMark(GetKeyValue(line, wxT("TITLE")));
        }
        else if (Keys[0] == wxT("INDEX"))
        {
            if (Keys[1] == wxT("01"))
            {
                m_PlaylistItems[currentTrack].m_Start = RedBookToMTime(GetKeyValue(line, wxT("01")));
                if (!m_PlaylistItems[currentTrack].m_Start)  // the first track starts at 1ms to make m_Offset = 1
                    m_PlaylistItems[currentTrack].m_Start++;
                if (fileTrack > 0)
                    m_PlaylistItems[currentTrack - 1].m_Length = m_PlaylistItems[currentTrack].m_Start -
                        m_PlaylistItems[currentTrack - 1].m_Start;
            }
        }
        else if (Keys[0] == wxT("PERFORMER"))
        {
            if (fileTrack == wxNOT_FOUND)
                m_ArtistName = RemoveQuotationMark(GetKeyValue(line, wxT("PERFORMER")));
            else
            {
                m_PlaylistItems[currentTrack].m_ArtistName = RemoveQuotationMark(GetKeyValue(line, wxT("PERFORMER")));
                if (!m_ArtistName.IsEmpty())
                    m_PlaylistItems[currentTrack].m_AlbumArtist = m_ArtistName;
            }
        }
        else if (Keys[0] == wxT("SONGWRITER"))
        {
            if (fileTrack == wxNOT_FOUND)
                m_Composer = RemoveQuotationMark(GetKeyValue(line, wxT("SONGWRITER")));
            else
                m_PlaylistItems[currentTrack].m_Composer = RemoveQuotationMark(GetKeyValue(line, wxT("SONGWRITER")));
        }
        else if (Keys[0] == wxT("REM"))
        {
            if (Keys[1] == wxT("GENRE"))
            {
                if (fileTrack == wxNOT_FOUND)
                {
                    m_Genre = RemoveQuotationMark(GetKeyValue(line, wxT("GENRE")));
                    guLogMessage(wxT("Genre  : '%s'"), m_Genre.c_str());
                }
                else
                {
                    m_PlaylistItems[currentTrack].m_Genre = RemoveQuotationMark(GetKeyValue(line, wxT("GENRE")));
                    guLogMessage(wxT("Genre %i: '%s'"), currentTrack, m_Genre.c_str());
                }
            }
            else if (Keys[1] == wxT("DATE"))
            {
                if (fileTrack == wxNOT_FOUND)
                    m_Year = RemoveQuotationMark(GetKeyValue(line, wxT("DATE")));
                else
                    m_PlaylistItems[currentTrack].m_Year = RemoveQuotationMark(GetKeyValue(line, wxT("DATE")));
            }
            else if (Keys[1] == wxT("COMMENT"))
            {
                if (fileTrack == wxNOT_FOUND)
                    m_Comment = RemoveQuotationMark(GetKeyValue(line, wxT("COMMENT")));
                else
                    m_PlaylistItems[currentTrack].m_Comment = RemoveQuotationMark(GetKeyValue(line, wxT("COMMENT")));
            }
        }
    }

    // Set the length of the last track of the last file
    if (currentTrack != wxNOT_FOUND)
        m_PlaylistItems[currentTrack].m_Length = m_TrackLength - m_PlaylistItems[currentTrack].m_Start;

    return false;
}

// -------------------------------------------------------------------------------- //
bool guCuePlaylistFile::Load( const wxString &location )
{
    wxString Content;

    m_Location = location;

    if (location.IsEmpty())
        return false;

    wxURI Uri( location );
    if( location.StartsWith( wxT( "file://" ) ) )
        m_Location = wxURI::Unescape( Uri.GetPath() );

    if( Uri.IsReference() )
    {
        //guLogMessage( wxT( "Cue sheet from file : '%s'" ), m_Location.c_str() );

        wxFile PlaylistFile( m_Location, wxFile::read );

        if( !PlaylistFile.IsOpened() )
        {
            guLogMessage( wxT( "Could not open '%s'" ), m_Location.c_str() );
            return false;
        }

        int DataSize = PlaylistFile.Length();
        if( !DataSize )
        {
            guLogMessage( wxT( "Cue sheet '%s' with 0 length" ), m_Location.c_str() );
            return false;
        }

        char * Buffer = ( char * ) malloc( DataSize + 1 );
        if( Buffer )
        {
            if( PlaylistFile.Read( Buffer, DataSize ) == DataSize )
            {
                Content = wxString( Buffer, wxConvAuto() );
                if( Content.IsEmpty() )
                {
                    Content = wxString( Buffer, wxConvUTF8 );
                    if( Content.IsEmpty() )
                    {
                        Content = wxString( Buffer, wxConvISO8859_1 );
                        if( Content.IsEmpty() )
                        {
                            for( int Index = 0; Index < DataSize; Index++ )
                            {
                                Content += Buffer[Index];
                            }
                        }
                    }
                }

            }
            else
                guLogMessage( wxT( "Could not read '%s' %u bytes" ), location.c_str(), DataSize );

            free( Buffer );
        }

        guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    }
    else
        Content = GetUrlContent( location );

    return LoadFromText( Content );
}

}
