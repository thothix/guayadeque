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
#include "TuneInRadio.h"

#include "Accelerators.h"
#include "DbCache.h"
#include "DbRadios.h"
#include "Http.h"
#include "Images.h"
#include "RadioPanel.h"
#include "RadioEditor.h"

#include <wx/sstream.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guTuneInRadioProvider::guTuneInRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios ) :
    guRadioProvider( radiopanel, dbradios )
{
    m_ReadStationsThread = NULL;
}

// -------------------------------------------------------------------------------- //
guTuneInRadioProvider::~guTuneInRadioProvider()
{
}

// -------------------------------------------------------------------------------- //
bool guTuneInRadioProvider::OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations, const int selcount )
{
    return true;
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::RegisterImages( wxImageList * imagelist )
{
    imagelist->Add( guImage( guIMAGE_INDEX_tiny_tunein ) );
    m_ImageIds.Add( imagelist->GetImageCount() - 1 );
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem )
{
    guRadioItemData * TuneInData = new guRadioItemData( -1, guRADIO_SOURCE_TUNEIN, wxT( "tunein" ), wxT( guTUNEIN_BASE_URL ), 0 );
    m_TuneInId = genretreectrl->AppendItem( rootitem, wxT( "tunein" ), m_ImageIds[ 0 ], m_ImageIds[ 0 ], TuneInData );
}

// -------------------------------------------------------------------------------- //
bool guTuneInRadioProvider::HasItemId( const wxTreeItemId &itemid )
{
    wxTreeItemId ItemId = itemid;
    while( ItemId.IsOk() )
    {
        if( ItemId == m_TuneInId )
            return true;
        ItemId = m_RadioPanel->GetItemParent( ItemId );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::EndReadStationsThread( void )
{
    wxMutexLocker MutexLocker( m_ReadStationsThreadMutex );
    m_ReadStationsThread = NULL;

//    m_RadioPanel->EndLoadingStations();
    wxCommandEvent Event( wxEVT_MENU, ID_RADIO_LOADING_STATIONS_FINISHED );
    wxPostEvent( m_RadioPanel, Event );
}

// -------------------------------------------------------------------------------- //
int guTuneInRadioProvider::GetStations( guRadioStations * stations, const long minbitrate )
{
    m_PendingItems.Empty();
    guRadioItemData * ItemData = m_RadioPanel->GetSelectedData();
    if( ItemData )
    {
        guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();
        wxTreeItemId SelectedItem = m_RadioPanel->GetSelectedGenre();
        RadioTreeCtrl->DeleteChildren( SelectedItem );

        //AddStations( ItemData->GetUrl(), stations, minbitrate );
        CancellSearchStations();

        wxMutexLocker Lock(m_ReadStationsThreadMutex);

        m_ReadStationsThread = new guTuneInReadStationsThread( this, m_RadioPanel, ItemData->GetUrl(), stations, minbitrate );
    }
    return stations->Count();
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::CancellSearchStations( void )
{
    if( m_ReadStationsThread )
    {
        wxMutexLocker MutexLocker( m_ReadStationsThreadMutex );
        m_ReadStationsThread->Pause();
        m_ReadStationsThread->Delete();
        m_ReadStationsThread = NULL;
    }
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::SetSearchText( const wxArrayString &texts )
{
    m_SearchTexts = texts;
}


// -------------------------------------------------------------------------------- //
guTuneInReadStationsThread::guTuneInReadStationsThread( guTuneInRadioProvider * tuneinprovider,
    guRadioPanel * radiopanel, const wxString &url, guRadioStations * stations, const long minbitrate ) :
    wxThread()
{
    m_TuneInProvider = tuneinprovider;
    m_RadioPanel = radiopanel;
    m_RadioStations = stations;
    m_Url = wxString::FromUTF8( url.char_str() );
    m_MinBitRate = minbitrate;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guTuneInReadStationsThread::~guTuneInReadStationsThread()
{
    if( !TestDestroy() )
    {
        m_TuneInProvider->EndReadStationsThread();
    }
}

// -------------------------------------------------------------------------------- //
wxString GetTuneInUrl( const wxString &url )
{
    guDbCache * DbCache = guDbCache::GetDbCache();
    wxString Content = DbCache->GetContent( url );

    if( Content.IsEmpty() )
    {
        char *      Buffer = NULL;
        guHttp      Http;

        // Only with a UserAgent is accepted the Charset requested
        //http.AddHeader( wxT( "User-Agent: " "Dalvik/1.6.0.(Linux;.U;.Android.4.1.1;.Galaxy.Nexus.Build/JRO03L)" ) );
        Http.AddHeader( wxT( "User-Agent" ), guDEFAULT_BROWSER_USER_AGENT );
        Http.AddHeader( wxT( "Accept" ), wxT( "text/html" ) );
        Http.AddHeader( wxT( "Accept-Charset" ), wxT( "utf-8" ) );
        Http.Get( Buffer, url );
        if( Buffer )
        {
            Content = wxString( Buffer, wxConvUTF8 );

            if( !Content.IsEmpty() )
            {
                DbCache->SetContent( url, Content, guDBCACHE_TYPE_TUNEIN );
            }

            free( Buffer );
        }
    }

    return Content;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNameA( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item1 )->m_Name.Cmp( ( * item2 )->m_Name );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNameD( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item2 )->m_Name.Cmp( ( * item1 )->m_Name );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareBitRateA( guRadioStation ** item1, guRadioStation ** item2 )
{
    if( ( * item1 )->m_BitRate == ( * item2 )->m_BitRate )
        return 0;
    else if( ( * item1 )->m_BitRate > ( * item2 )->m_BitRate )
        return 1;
    else
        return -1;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareBitRateD( guRadioStation ** item1, guRadioStation ** item2 )
{
    if( ( * item1 )->m_BitRate == ( * item2 )->m_BitRate )
        return 0;
    else if( ( * item2 )->m_BitRate > ( * item1 )->m_BitRate )
        return 1;
    else
        return -1;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareTypeA( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item1 )->m_Type.Cmp( ( * item2 )->m_Type );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareTypeD( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item2 )->m_Type.Cmp( ( * item1 )->m_Type );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNowPlayingA( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item1 )->m_NowPlaying.Cmp( ( * item2 )->m_NowPlaying );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNowPlayingD( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item2 )->m_NowPlaying.Cmp( ( * item1 )->m_NowPlaying );
}

// -------------------------------------------------------------------------------- //
void guTuneInReadStationsThread::SortStations( void )
{
    int     StationsOrder = m_RadioPanel->GetStationsOrder();
    bool    StationsOrderDesc = m_RadioPanel->GetStationsOrderDesc();

    switch( StationsOrder )
    {
        case guRADIOSTATIONS_COLUMN_NAME :
            m_RadioStations->Sort( StationsOrderDesc ? CompareNameD : CompareNameA );
            break;

        case guRADIOSTATIONS_COLUMN_BITRATE :
            m_RadioStations->Sort( StationsOrderDesc ? CompareBitRateD : CompareBitRateA );

        case guRADIOSTATIONS_COLUMN_LISTENERS :
            break;

        case guRADIOSTATIONS_COLUMN_TYPE :
            m_RadioStations->Sort( StationsOrderDesc ? CompareTypeD : CompareTypeA );
            break;

        case guRADIOSTATIONS_COLUMN_NOWPLAYING :
            m_RadioStations->Sort( StationsOrderDesc ? CompareNowPlayingD : CompareNowPlayingA );
            break;
    }
}

// -------------------------------------------------------------------------------- //
bool SearchFilterTexts( wxArrayString &texts, const wxString &name )
{
    int Count = texts.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        //guLogMessage( wxT( "%s = > '%s'" ), name.c_str(), texts[ Index ].Lower().c_str() );
        if( name.Find( texts[ Index ].Lower() ) == wxNOT_FOUND )
            return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
void guTuneInReadStationsThread::ReadStations( wxXmlNode * xmlnode, guRadioStations * stations, const long minbitrate )
{
//    wxString MoreStationsUrl;
    while( xmlnode && !TestDestroy() )
    {
        wxString Type;
        wxString Name;
        wxString Url;
        xmlnode->GetAttribute( wxT( "type" ), &Type );
        if( Type == wxT( "" ) )
        {
            ReadStations( xmlnode->GetChildren(), stations, minbitrate );
        }
        else if( Type == wxT( "link" ) )
        {
            xmlnode->GetAttribute( wxT( "text" ), &Name );
            xmlnode->GetAttribute( wxT( "URL" ), &Url );

            //guLogMessage( wxT( "ReadStations -> Type : '%s' Name : '%s' " ), Type.c_str(), Name.c_str() );
            if( Name == wxT( "Find by Name" ) )
            {
            }
            else if( Name == wxT( "More Stations" ) )
            {
                //MoreStationsUrl = Url;
                m_MoreStations.Add( Url );
            }
            else
            {
                //guLogMessage( wxT( "AddPendingItem '%s' '%s'" ), Name.c_str(), Url.c_str() );
                m_TuneInProvider->AddPendingItem( Name + wxT( "|" ) + Url );

                wxCommandEvent Event( wxEVT_MENU, ID_RADIO_CREATE_TREE_ITEM );
                wxPostEvent( m_RadioPanel, Event );
                Sleep( 20 );
            }
        }
        else if( Type == wxT( "audio" ) )
        {
            //    <outline type="audio"
            //        text="Talk Radio Europe (Cartagena)"
            //        URL="http://opml.radiotime.com/Tune.ashx?id=s111270"
            //        bitrate="64"
            //        reliability="96"
            //        guide_id="s111270"
            //        subtext="your voice in spain"
            //        genre_id="g32"
            //        formats="mp3"
            //        item="station"
            //        image="http://radiotime-logos.s3.amazonaws.com/s111270q.png"
            //        now_playing_id="s111270"
            //        preset_id="s111270"/>
            guRadioStation * RadioStation = new guRadioStation();

            long lBitRate = 0;
            wxString BitRate;
            xmlnode->GetAttribute( wxT( "bitrate" ), &BitRate );
            if( !BitRate.IsEmpty() )
            {
                BitRate.ToLong( &lBitRate );
            }
            xmlnode->GetAttribute( wxT( "text" ), &RadioStation->m_Name );
            if( ( BitRate.IsEmpty() || ( lBitRate >= minbitrate ) ) && SearchFilterTexts( m_TuneInProvider->GetSearchTexts(), RadioStation->m_Name.Lower() ) )
            {
                RadioStation->m_BitRate = lBitRate;
                RadioStation->m_Id = -1;
                RadioStation->m_SCId = wxNOT_FOUND;
                xmlnode->GetAttribute( wxT( "URL" ), &RadioStation->m_Link );
                xmlnode->GetAttribute( wxT( "formats" ), &RadioStation->m_Type );
                xmlnode->GetAttribute( wxT( "subtext" ), &RadioStation->m_NowPlaying );
                RadioStation->m_Source = guRADIO_SOURCE_TUNEIN;
                RadioStation->m_Listeners = 0;

                stations->Add( RadioStation );
                //guLogMessage( wxT( "Adding station %s" ), RadioStation->m_Name.c_str() );
            }
            else
            {
                delete RadioStation;
            }
        }

        xmlnode = xmlnode->GetNext();
    }

    if( !TestDestroy() )
    {
        SortStations();

        wxCommandEvent Event( wxEVT_MENU, ID_RADIO_UPDATED );
        wxPostEvent( m_RadioPanel, Event );
    }
}

// -------------------------------------------------------------------------------- //
int guTuneInReadStationsThread::AddStations( const wxString &url, guRadioStations * stations, const long minbitrate )
{
    wxString Content = GetTuneInUrl( url );
    //guLogMessage( wxT( "AddStations: %s" ), url.c_str() );

    if( !Content.IsEmpty() )
    {
        wxStringInputStream Ins( Content );
        wxXmlDocument XmlDoc( Ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "opml" ) )
            {
                XmlNode = XmlNode->GetChildren();
                while( XmlNode && !TestDestroy() )
                {
                    //guLogMessage( wxT( "XmlNode: '%s'" ), XmlNode->GetName().c_str() );
                    if( XmlNode->GetName() == wxT( "outline" ) )
                    {
                        wxString Type;
                        XmlNode->GetAttribute( wxT( "type" ), &Type );
                        if( Type == wxT( "" ) )
                        {
                            ReadStations( XmlNode->GetChildren(), stations, minbitrate );
                        }
                        else
                        {
                            ReadStations( XmlNode, stations, minbitrate );
                            break;
                        }
                    }
                    else if( XmlNode->GetName() == wxT( "body" ) )
                    {
                        XmlNode = XmlNode->GetChildren();
                        continue;
                    }

                    XmlNode = XmlNode->GetNext();
                }
            }
        }
    }

    if( !TestDestroy() )
    {
        SortStations();

        wxCommandEvent Event( wxEVT_MENU, ID_RADIO_UPDATED );
        wxPostEvent( m_RadioPanel, Event );

        return stations->Count();
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
guTuneInReadStationsThread::ExitCode guTuneInReadStationsThread::Entry()
{
    if( TestDestroy() )
        return 0;

    if( !TestDestroy() )
    {
        AddStations( m_Url, m_RadioStations, m_MinBitRate );

        while( !TestDestroy() && m_MoreStations.Count() )
        {
            AddStations( m_MoreStations[ 0 ], m_RadioStations, m_MinBitRate );

            m_MoreStations.RemoveAt( 0 );
        }
    }

    return 0;
}

}

// -------------------------------------------------------------------------------- //
