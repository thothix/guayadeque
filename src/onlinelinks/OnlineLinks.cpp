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
#include "OnlineLinks.h"

#include "EventCommandIds.h"
#include "Config.h"
#include "Images.h"
#include "MainApp.h"
#include "Settings.h"
#include "Utils.h"

#include <wx/uri.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
void AddOnlineLinksMenu( wxMenu * Menu )
{
    wxMenu * SubMenu;
    wxMenuItem * MenuItem;
    if( Menu )
    {
        SubMenu = new wxMenu();

        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Links = Config->ReadAStr( CONFIG_KEY_SEARCHLINKS_LINK, wxEmptyString, CONFIG_PATH_SEARCHLINKS_LINKS );
        wxArrayString Names = Config->ReadAStr( CONFIG_KEY_SEARCHLINKS_NAME, wxEmptyString, CONFIG_PATH_SEARCHLINKS_NAMES );
        int count = Links.Count();
        if( count )
        {
            for( int index = 0; index < count; index++ )
            {
                wxURI Uri( Links[ index ] );
                MenuItem = new wxMenuItem( Menu, ID_LINKS_BASE + index, Names[ index ], Links[ index ] );
                wxString IconFile = guPATH_LINKICONS + Uri.GetServer() + wxT( ".ico" );
                if( wxFileExists( IconFile ) )
                {
                    MenuItem->SetBitmap( wxBitmap( IconFile, wxBITMAP_TYPE_ICO ) );
                }
                else
                {
                    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_search ) );
                }
                SubMenu->Append( MenuItem );
            }

            SubMenu->AppendSeparator();
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, ID_MENU_PREFERENCES_LINKS, _( "Preferences" ), _( "Add search links in preferences" ) );
            SubMenu->Append( MenuItem );
        }
        Menu->AppendSubMenu( SubMenu, _( "Links" ) );
    }
}

// -------------------------------------------------------------------------------- //
void ExecuteOnlineLink( const int linkid, const wxString &text )
{
    int index = linkid - ID_LINKS_BASE;
    //guLogMessage( wxT( "ExecuteOnlineLink( %i, '%s' )" ), index, text.c_str() );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString Links = Config->ReadAStr( CONFIG_KEY_SEARCHLINKS_LINK, wxEmptyString, CONFIG_PATH_SEARCHLINKS_LINKS );

    if( index >= 0 && ( index < ( int ) Links.Count() ) )
    {
        wxString SearchLink = Links[ index ];
        wxString Lang = Config->ReadStr( CONFIG_KEY_LASTFM_LANGUAGE, wxT( "en" ), CONFIG_PATH_LASTFM );
        if( Lang.IsEmpty() )
        {
            Lang = ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().Mid( 0, 2 );
            //guLogMessage( wxT( "Locale: %s" ), ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().c_str() );
        }
        SearchLink.Replace( guLINKS_LANGUAGE, Lang );
        SearchLink.Replace( guLINKS_TEXT, guURLEncode( text ) );
        guWebExecute( SearchLink );
    }
    else
    {
        guLogMessage( wxT( "Online Link out of rante %i" ), index );
    }
}

}

// -------------------------------------------------------------------------------- //
