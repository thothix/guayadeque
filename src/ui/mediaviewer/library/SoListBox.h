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
#ifndef __SOLISTBOX_H__
#define __SOLISTBOX_H__

#include "DbLibrary.h"
#include "ListView.h"
#include "StatusBar.h"

namespace Guayadeque {

enum guSongs_Columns {
    guSONGS_COLUMN_NUMBER,
    guSONGS_COLUMN_TITLE,
    guSONGS_COLUMN_ARTIST,
    guSONGS_COLUMN_ALBUMARTIST,
    guSONGS_COLUMN_ALBUM,
    guSONGS_COLUMN_GENRE,
    guSONGS_COLUMN_COMPOSER,
    guSONGS_COLUMN_DISK,
    guSONGS_COLUMN_LENGTH,
    guSONGS_COLUMN_YEAR,
    guSONGS_COLUMN_BITRATE,
    guSONGS_COLUMN_RATING,
    guSONGS_COLUMN_PLAYCOUNT,
    guSONGS_COLUMN_LASTPLAY,
    guSONGS_COLUMN_ADDEDDATE,
    guSONGS_COLUMN_FORMAT,
    guSONGS_COLUMN_FILEPATH,
    guSONGS_COLUMN_OFFSET,
    guSONGS_COLUMN_COUNT
};

class guLibPanel;

// -------------------------------------------------------------------------------- //
class guSoListBox : public guListView
{
  private:
    void                        HandleMultiColumnOrder(const int order);
    void                        HandleMultiColumnOrderKeepSelections(const int order);

  protected :
    guMediaViewer *             m_MediaViewer;

    guDbLibrary *               m_Db;
    guTrackArray                m_Items;
    wxMutex                     m_ItemsMutex;
    int                         m_ItemsFirst;
    int                         m_ItemsLast;
    wxString                    m_ConfName;
    int                         m_LastColumnRightClicked;
    int                         m_LastRowRightClicked;

    int                         m_TracksOrder;
    bool                        m_TracksOrderDesc;
    wxArrayInt                  m_TracksMultiOrder;
    wxArrayInt                  m_TracksMultiOrderDesc;

    wxBitmap *                  m_NormalStar;
    wxBitmap *                  m_SelectStar;

    wxArrayString               m_ColumnNames;

    virtual void                DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList();

    void                        OnSearchLinkClicked( wxCommandEvent &event );
    void                        OnCommandClicked( wxCommandEvent &event );
    virtual wxString            GetSearchText( int item ) const;

    void                        OnItemColumnClicked( wxListEvent &event );
    void                        OnItemColumnRClicked( wxListEvent &event );

    virtual void                ItemsLock() { m_ItemsMutex.Lock(); }
    virtual void                ItemsUnlock() { m_ItemsMutex.Unlock(); }
    virtual void                ItemsCheckRange( const int start, const int end );

    virtual void                AppendFastEditMenu( wxMenu * menu, const int selcount ) const;

    virtual void                OnConfigUpdated( wxCommandEvent &event );
    virtual void                CreateAcceleratorTable();

    virtual void                OnCreateSmartPlaylist( wxCommandEvent &event );
    virtual void                OnCreateBestOfPlaylist( wxCommandEvent &event );

  public :
    guSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, long style = 0 );
    ~guSoListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual int                 GetSelectedSongs( guTrackArray * Songs, const bool isdrag = false ) const;
    virtual void                GetAllSongs( guTrackArray * Songs );

    virtual int                 GetItemId( const int row ) const;
    virtual wxString            GetItemName( const int row ) const;

    virtual wxArrayString       GetColumnNames() const;

    void                        UpdatedTracks( const guTrackArray * tracks );
    void                        UpdatedTrack( const guTrack * track );
    int                         FindItem( const int trackid );

    int                         GetLastColumnClicked() { return m_LastColumnRightClicked; }
    int                         GetLastRowClicked( ) { return m_LastRowRightClicked; }
    wxVariant                   GetLastDataClicked();

    virtual int                 GetTracksOrder() { return m_TracksOrder; }
    virtual void                SetTracksOrder( const int order );
    virtual bool                GetTracksOrderDesc() { return m_TracksOrderDesc; }
    virtual void                SetTracksOrderDesc( const bool orderdesc ) { m_TracksOrderDesc = orderdesc; }

    virtual int                 GetTrackCount() { return m_Items.Count(); }
};

}

#endif
