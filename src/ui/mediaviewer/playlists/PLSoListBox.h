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
#ifndef __PLSOLISTBOX_H__
#define __PLSOLISTBOX_H__

#include "SoListBox.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guPLSoListBox : public guSoListBox
{
  protected :
    wxArrayInt          m_PLIds;    // The PlayLists ids
    wxArrayInt          m_PLTypes;  // The PlayLists types
    wxArrayInt          m_PLSetIds; // The array containing the list of plset_id
    wxArrayInt          m_DropIds;  // The array containing the id of the songs dropped
    wxLongLong          m_TracksSize;
    wxLongLong          m_TracksLength;
    bool                m_DisableSorting;

    virtual void        GetItemsList( void );
    virtual void        CreateContextMenu( wxMenu * Menu ) const;

    virtual void        OnKeyDown( wxKeyEvent &event );

    virtual void        OnDropFile( const wxString &filename );
    virtual void        OnDropEnd( void );
    virtual void        MoveSelection( void );

    virtual void        ItemsCheckRange( const int start, const int end ) { m_ItemsFirst = 0; m_ItemsLast = 0; }

    virtual void        CreateAcceleratorTable();
    virtual wxString    GetSearchText( int item ) const;

    void                OnRandomizeTracks( wxCommandEvent &event ) { RandomizeTracks(); }

  public :
    guPLSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, int style = 0 );
    ~guPLSoListBox();

    void                SetPlayList( int plid, int pltype );
    void                SetPlayList( const wxArrayInt &ids, const wxArrayInt &types );
    int                 GetPlayListSetIds( wxArrayInt * setids ) const;

    virtual int         GetSelectedSongs( guTrackArray * Songs, const bool isdrag = false ) const;
    virtual void        GetAllSongs( guTrackArray * Songs );

    virtual int         GetItemId( const int row ) const;
    virtual wxString    GetItemName( const int row ) const;

    void                GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
    {
        * count = GetItemCount();
        * len   = m_TracksLength;
        * size  = m_TracksSize;
    }

    virtual void        SetTracksOrder( const int order );

    virtual void        RandomizeTracks( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
