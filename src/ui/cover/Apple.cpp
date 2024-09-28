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
#include <memory>

#include "Apple.h"
#include "CoverEdit.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/base64.h>
#include <wx/statline.h>
#include <wx/sstream.h>

#include "json/json.h"

#define APPLE_SEARCH_URL       wxT( "https://itunes.apple.com/search?term=" )
#define APPLE_SEARCH_PARAMS    wxT( "&limit=30" )

namespace Guayadeque {


// -------------------------------------------------------------------------------- //
guAppleCoverFetcher::guAppleCoverFetcher( guFetchCoverLinksThread * mainthread,
                                          guArrayStringArray * coverlinks,
                                          const wxChar * artist,
                                          const wxChar * album ) :
    guCoverFetcher( mainthread, coverlinks, artist, album )
{
}

// -------------------------------------------------------------------------------- //
int guAppleCoverFetcher::ExtractImagesInfo( wxString &content )
{
    int RetVal = 0;
    Json::Value RespObject;
    //Json::Reader reader;
    //reader.parse( std::string( content.mb_str() ), RespObject );
    JSONCPP_STRING err;
    Json::CharReaderBuilder builder;

    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(content.c_str(), content.c_str() + content.length(), &RespObject , &err))
    {
        guLogMessage( wxT( "guAppleCoverFetcher::ExtractImagesInfo Error: %s" ), err );
        return EXIT_FAILURE;
    }

    if( RespObject.isMember( "results" ) )
    {
        //guLogMessage( wxT( "Got the results..." ) );
        Json::Value Results = RespObject[ "results" ];
        if( Results.isArray() )
         {
             int Count = Results.size();
             //guLogMessage( wxT( "Results is array....%d" ), Count );

             for( int Index = 0; Index < Count; Index++ )
             {
                 Json::Value Result = Results[ Index ];
                 //guLogMessage( "Object in result %s", Result.asString() );
                 if( Result.isMember( "artworkUrl100" ) )
                 {
                    wxString CoverLink = wxString( Result[ "artworkUrl100" ].asString() );
                    if( !CoverLink.IsEmpty() )
                    {
                        CoverLink.Replace( wxT( "100x100" ), wxT( "500x500" ) );
                        if( !CoverLinkExist( CoverLink ) )
                        {
                            wxArrayString CoverItem;
                            CoverItem.Add( CoverLink );
                            CoverItem.Add( wxT( "500x500" ) );
                            m_CoverLinks->Add( CoverItem );
                            RetVal++;
                        }
                    }
                 }
             }
         }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guAppleCoverFetcher::AddCoverLinks( int pagenum )
{
    wxString SearchParams = guURLEncode( m_Artist + wxT( " " ) + m_Album ) +
                            APPLE_SEARCH_PARAMS;

    SearchParams.Replace( wxT( "," ), wxT( "%2C" ) );

    wxString SearchUrl = APPLE_SEARCH_URL + SearchParams;
    //guLogMessage( wxT( "URL: %u %s" ), pagenum, SearchUrl.c_str() );
    if( !m_MainThread->TestDestroy() )
    {
        //printf( "Buffer:\n%s\n", Buffer );
        wxString Content = GetUrlContent( SearchUrl );
        //Content = http.GetContent( SearchUrl, 60 );
        //guLogMessage( wxT( "Amazon Response:\n%s" ), Content.c_str() );
        if( Content.Length() )
        {
            if( !m_MainThread->TestDestroy() )
                return ExtractImagesInfo( Content );
        }
        else
            guLogError( wxT( "Could not get the remote data from connection" ) );
    }
    return 0;
}

}

// -------------------------------------------------------------------------------- //
