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
#ifndef __DB_H__
#define __DB_H__

#include "Utils.h"

// wxWidgets
#include <wx/string.h>
#include <wx/utils.h>

// wxSqlite3
#include <wx/wxsqlite3.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
void inline escape_query_str( wxString * Str )
{
  Str->Replace( wxT( "'" ), wxT( "''" ) );
  //Str->Replace( _T( "\"" ), _T( "\"\"" ) );
  //Str->Replace( _T( "\\" ), _T( "\\\\" ) );
}

// -------------------------------------------------------------------------------- //
wxString inline escape_query_str( const wxString &str )
{
    wxString QueryStr = str;
    escape_query_str( &QueryStr );
    //guLogMessage( wxT( "'%s' --> '%s'" ), str.c_str(), QueryStr.c_str() );
    return QueryStr;
}

// -------------------------------------------------------------------------------- //
class guDb
{
  protected :
    wxString                m_DbName;
    wxSQLite3Database  *    m_Db;

  public :
    guDb( void );
    guDb( const wxString &dbname );
    virtual ~guDb();

    int                 Open( const wxString &dbname );
    int                 Close( void );
    wxSQLite3Database * GetDb( void ) { return m_Db; }

    wxSQLite3ResultSet  ExecuteQuery( const wxString &query );
    int                 ExecuteUpdate( const wxString &query );
    wxSQLite3ResultSet  ExecuteQuery( const wxSQLite3StatementBuffer &query );
    int                 ExecuteUpdate( const wxSQLite3StatementBuffer &query );
    int                 GetLastRowId( void ) { return m_Db->GetLastRowId().GetLo(); }

    virtual void        SetInitParams( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
