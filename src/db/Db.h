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
#ifndef __DB_H__
#define __DB_H__

#include "Utils.h"

//#include <sqlite3.h>
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <wx/string.h>
#include <wx/utils.h>
#include <wx/wxsqlite3.h>

namespace Guayadeque {

void inline escape_query_str( wxString * Str )
{
    Str->Replace( wxT( "'" ), wxT( "''" ) );
    //Str->Replace( _T( "\"" ), _T( "\"\"" ) );
    //Str->Replace( _T( "\\" ), _T( "\\\\" ) );
}

wxString inline escape_query_str( const wxString &str )
{
    wxString QueryStr = str;
    escape_query_str( &QueryStr );
    //guLogMessage( wxT( "'%s' --> '%s'" ), str.c_str(), QueryStr.c_str() );
    return QueryStr;
}

class guFoldAllTransliterator
{
    public:
        icu::Transliterator  *accentsConverter;

        guFoldAllTransliterator()
        {
            UErrorCode error = U_ZERO_ERROR;
            accentsConverter = icu::Transliterator::createInstance("NFD; [:M:] Remove; Lower; NFC", UTRANS_FORWARD, error);
        }

        icu::UnicodeString   WxStrToICU(const wxString& wxs)
        {
            return icu::UnicodeString::fromUTF32((const UChar32*)wxs.wc_str(), wxs.Length());
        }

        void Fold(icu::UnicodeString& what)
        {
            accentsConverter->transliterate(what);
        }
};

class guDbFoldAllCollation : public wxSQLite3Collation
{
    public:
        guFoldAllTransliterator  transliterator;

        guDbFoldAllCollation() : wxSQLite3Collation()
        {
            transliterator = guFoldAllTransliterator();
        }

        int Compare(const wxString& text1, const wxString& text2)
        {
            guLogDebug("guDbFoldAllCollation::Compare %s %s", text1, text2);
            icu::UnicodeString t1 = transliterator.WxStrToICU(text1), t2 = transliterator.WxStrToICU(text2);
            transliterator.Fold(t1);
            transliterator.Fold(t2);
            return t1.compare(t2);
        }
};

class guDbFoldAllContainsFunction : public wxSQLite3ScalarFunction
{
    public:
        guFoldAllTransliterator  transliterator;

        void Execute( wxSQLite3FunctionContext &    ctx )
        {
            guLogDebug("guDbFoldAllContainsFunction::Execute %s %s", ctx.GetString(0), ctx.GetString(1));
            icu::UnicodeString t1 = transliterator.WxStrToICU(ctx.GetString(0)),
                               t2 = transliterator.WxStrToICU(ctx.GetString(1));
            transliterator.Fold(t1);
            transliterator.Fold(t2);
            ctx.SetResult( t1.indexOf(t2) );
            return;
        }
};

class guDb
{
    protected :
        wxString                       m_DbName;
        wxString                       m_DbUniqueId;
        wxSQLite3Database            * m_Db;
        guDbFoldAllCollation           m_DbFoldAllCollation;
        guDbFoldAllContainsFunction    m_DbFoldAllContainsFunction;

    public :
        guDb();
        guDb( const wxString &dbname );
        virtual ~guDb();

        int                 Open( const wxString &dbname );
        int                 Close();
        wxSQLite3Database * GetDb() { return m_Db; }
        wxString            GetDbName() { return m_DbName; }
        wxString            GetDbUniqueId() { return m_DbUniqueId; }

        wxSQLite3ResultSet  ExecuteQuery( const wxString &query );
        int                 ExecuteUpdate( const wxString &query );
        wxSQLite3ResultSet  ExecuteQuery( const wxSQLite3StatementBuffer &query );
        int                 ExecuteUpdate( const wxSQLite3StatementBuffer &query );
        int                 GetLastRowId() { return m_Db->GetLastRowId().GetLo(); }

        virtual void        SetInitParams();
};

}

#endif
