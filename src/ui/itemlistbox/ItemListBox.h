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
#ifndef __ITEMLISTBOX_H__
#define __ITEMLISTBOX_H__

#include "ListView.h"
#include "DbLibrary.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guListBox : public guListView
{
  protected :
    guDbLibrary *       m_Db;
    guListItems *       m_Items;

    virtual wxString    OnGetItemText( const int row, const int col ) const { return GetItemName( row ); }
    virtual wxString    GetSearchText( const int item ) const { return wxEmptyString; }

  public :
    guListBox( wxWindow * parent, guDbLibrary * db, const wxString &label = wxEmptyString,
                    int flags = wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG );
    ~guListBox();

    virtual wxString inline GetItemName( const int item ) const
    {
        return ( * m_Items )[ item ].m_Name;
    }

    virtual int inline      GetItemId( const int item ) const
    {
        return ( * m_Items )[ item ].m_Id;
    }

    virtual void            ReloadItems( bool reset = true );

//    virtual void            SetSelectedItems( const wxArrayInt &selection );

    virtual int             FindItemId( const int id );

};

}

#endif
// -------------------------------------------------------------------------------- //
