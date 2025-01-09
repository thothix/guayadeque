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
#ifndef __HTTP_H__
#define __HTTP_H__

#include "Curl.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guHttp : public guCurl
{
  public:
    guHttp( const wxString &url = wxEmptyString );
    virtual ~guHttp();

    void                      AddHeader( const wxString &key, const wxString &value );

    bool                      Post( const char * buffer, size_t size, const wxString &url = wxEmptyString );
    bool                      Post( wxInputStream &buffer, const wxString &url = wxEmptyString );

    bool                      Get( const wxString &filename, const wxString &url = wxEmptyString );
    size_t                    Get( char * &buffer, const wxString &url = wxEmptyString );
    bool                      Get( wxOutputStream &buffer, const wxString &url = wxEmptyString );

};

}

#endif
// -------------------------------------------------------------------------------- //
