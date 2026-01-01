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
#ifndef __ALLISTBOX_H__
#define __ALLISTBOX_H__

#include "DbLibrary.h"
#include "ItemListBox.h"

namespace Guayadeque {

class guLibPanel;

// -------------------------------------------------------------------------------- //
class guAlListBox : public  guListView
{
  private :
    guDbLibrary *       m_Db;
    guAlbumItems *      m_Items;
    guLibPanel *        m_LibPanel;
    int                 m_AlbumsOrder;
    wxString            m_ConfigPath;
    int                 m_SysFontPointSize;

    void                OnAlbumListActivated( wxListEvent &event );
    void                OnAlbumListSelected( wxListEvent &event );
    virtual void        DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    wxCoord             OnMeasureItem( size_t n ) const;
    virtual void        CreateContextMenu( wxMenu * menu ) const;

    virtual void        GetItemsList( void ) { m_Db->GetAlbums( ( guAlbumItems * ) m_Items ); }
    void                OnSearchLinkClicked( wxCommandEvent &event );
    void                OnCommandClicked( wxCommandEvent &event );
    void                OnOrderSelected( wxCommandEvent &event );
    wxString            GetSearchText( int item ) const;

    void                OnConfigUpdated( wxCommandEvent &event );
    void                CreateAcceleratorTable();

  public :
                        guAlListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label );
                        ~guAlListBox();
    bool                SelectAlbumName( const wxString &AlbumName );
    virtual int         GetSelectedSongs( guTrackArray * tracks, const bool isdrag = false ) const;
    virtual wxString    GetItemName( const int item ) const { return ( * m_Items )[ item ].m_Name; }

    virtual int         GetItemId( const int item ) const { return ( * m_Items )[ item ].m_Id; }
    virtual void        ReloadItems( bool reset = true );

    int                 FindAlbum( const int albumid );


};

}

#endif
// -------------------------------------------------------------------------------- //
