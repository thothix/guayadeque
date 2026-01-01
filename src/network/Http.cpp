/*
   Copyright (C) 2008-2023 J.Rios <anonbeat@gmail.com>
   Copyright (C) 2024-2026 Tiago T Barrionuevo <thothix@protonmail.com>

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
#include "Http.h"

#include "wx/wx.h"
#include <wx/wfstream.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guHttp::guHttp( const wxString &url ) : guCurl( url )
{
}

// -------------------------------------------------------------------------------- //
guHttp::~guHttp()
{
}

// -------------------------------------------------------------------------------- //
void guHttp::AddHeader( const wxString &key, const wxString &value )
{
    m_ReqHeaders.Add( key + ": " + value );
}

// -------------------------------------------------------------------------------- //
bool guHttp::Post( const char * buffer, size_t size, const wxString &url )
{
    wxMemoryInputStream InStream( buffer, size );
    return Post( InStream, url );
}

// -------------------------------------------------------------------------------- //
bool guHttp::Post( wxInputStream &instream, const wxString &url )
{
    if( instream.IsOk() )
    {
        SetDefaultOptions( url );
        SetHeaders();

        curl_off_t Size = instream.GetSize();
        if( !Size )
            return false;

        SetOption( CURLOPT_POST, 1L );
        SetOption( CURLOPT_POSTFIELDSIZE_LARGE, Size );
        SetStreamReadOption( instream );
        SetStringWriteOption( m_ResData );

        if( Perform() )
        {
            CleanHeaders();
            CleanupHandle();

            return ( m_ResCode >= 200 && m_ResCode < 300 );
        }
        CleanHeaders();
        CleanupHandle();
    }

    return false;
}


// -------------------------------------------------------------------------------- //
bool guHttp::Get( const wxString &filename, const wxString &url )
{
    wxFFileOutputStream outStream( filename );

    return Get( outStream, url );
}

// -------------------------------------------------------------------------------- //
size_t guHttp::Get( char * &buffer, const wxString &url )
{
    buffer = NULL;
    wxMemoryOutputStream outStream;

    if( Get( outStream, url ) )
    {
        size_t Retval = outStream.GetSize();
        buffer = ( char * ) malloc( Retval + 1 );
        if( buffer )
        {
            if( outStream.CopyTo( buffer, Retval ) == Retval )
            {
                buffer[ Retval ] = 0;
                return Retval;
            }
            else
            {
                free( buffer );
                buffer = NULL;
            }
        }
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
bool guHttp::Get( wxOutputStream &buffer, const wxString &url )
{
    if( buffer.IsOk() )
    {
        SetDefaultOptions( url );
        SetHeaders();
        SetOption( CURLOPT_HTTPGET, 1L );
        SetStreamWriteOption( buffer );

        if( Perform() )
        {
            CleanHeaders();
            CleanupHandle();

            return ( m_ResCode >= 200 && m_ResCode < 300 );
        }
        CleanHeaders();
        CleanupHandle();
    }

    return false;
}

}

// -------------------------------------------------------------------------------- //
