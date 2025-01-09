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
#include "CoverFetcher.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guCoverFetcher
// -------------------------------------------------------------------------------- //
guCoverFetcher::guCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                      const wxChar * artist, const wxChar * album )
{
    m_MainThread = mainthread;
    m_CoverLinks = coverlinks;
    m_Artist = wxString( artist );
    m_Album = wxString( album );
}

// -------------------------------------------------------------------------------- //
guCoverFetcher::~guCoverFetcher()
{
}

// -------------------------------------------------------------------------------- //
bool guCoverFetcher::CoverLinkExist( const wxString &coverlink )
{
    int Count = m_CoverLinks->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        const wxArrayString Cover = m_CoverLinks->Item( Index );
        if( Cover[ 0 ] == coverlink )
            return true;
    }
    return false;
}


}

// -------------------------------------------------------------------------------- //
