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
#ifndef __DBLISTBOX_H__
#define __DBLISTBOX_H__

// -------------------------------------------------------------------------------- //
guDbListBoxCache : public wxObject
{
  private :
    DbLibrary * Db;
  public :
    guDbListBoxCache( DbLibrary * db );
};

// -------------------------------------------------------------------------------- //
// This is the shown title bar
guDbListBoxHeader : public wxWindow
{
  private :
    wxString LabelFormat;
    wxString LabelStr;
  public :
};

// -------------------------------------------------------------------------------- //
// This is really the listbox conainted into the virtual control
guDbListBoxItems : public wxVListBox
{
  private :
        wxPoint                 DragStart;
        int                     DragCount;

        wxColor                 SelBgColor;
        wxColor                 SelFgColor;
        wxColor                 OddBgColor;
        wxColor                 EveBgColor;
        wxColor                 TextFgColor;
        wxColor                 SepColor;

        void            OnDragOver( const wxCoord x, const wxCoord y );
        void            OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const;
        wxCoord         OnMeasureItem( size_t n ) const;
        void            OnDrawBackground( wxDC &dc, const wxRect &rect, size_t n ) const;
        void            OnKeyDown( wxKeyEvent &event );
        void            OnBeginDrag( wxMouseEvent &event );
        void            OnMouse( wxMouseEvent &event );
        void            OnContextMenu( wxContextMenuEvent& event );

        DECLARE_EVENT_TABLE()

        friend class guAlbumListBoxTimer;
  public :

};

// -------------------------------------------------------------------------------- //
// this is a container class for the header, cache and listbox
guDbListBox : public wxScrolledWindow
{
  private :
    DbLibrary *         Db;
    guDbListBoxHeader * ListBoxHeader;
    guDbListBoxCache *  ListBoxCache;
    guDbListBoxItems *  ListBoxItems;


  public :
                guDbListBox( wxWindow * parent, DbLibrary * db, const wxString &label );
                ~guDbListBox();

    void        ReloadItems( void );

    wxArrayInt  GetSelection( void ) const;

    // Return the Tracks from the library selected by the current selection of this control
    int         GetSelectedTracks( guTrackArray * Tracks ) const;

};


#endif
// -------------------------------------------------------------------------------- //
