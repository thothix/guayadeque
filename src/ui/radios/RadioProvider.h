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
#ifndef __RADIOPROVIDER_H__
#define __RADIOPROVIDER_H__

#include "DbRadios.h"

#include <wx/arrstr.h>
#include <wx/dynarray.h>
#include <wx/event.h>
#include <wx/imaglist.h>
#include <wx/treebase.h>

namespace Guayadeque {

class guRadioPanel;
class guRadioGenreTreeCtrl;
class guDbRadios;

// -------------------------------------------------------------------------------- //
class guRadioProvider : public wxEvtHandler
{
  protected :
    guRadioPanel *      m_RadioPanel;
    guDbRadios *        m_Db;
    wxArrayInt          m_ImageIds;
    wxArrayString       m_PendingItems;

  public :
    guRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios );
    ~guRadioProvider();

    virtual bool            OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations = false, const int selcount = 0 ) { return false; }
    virtual void            Activated( const int id ) {}
    virtual void            SetSearchText( const wxArrayString &texts ) {}
    virtual void            RegisterImages( wxImageList * imagelist ) {}
    virtual void            RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem ) {}
    virtual bool            HasItemId( const wxTreeItemId &itemid ) { return false; }
    virtual int             GetStations( guRadioStations * stations, const long minbitrate ) { return 0; }
    virtual void            CancellSearchStations( void ) {}
    virtual wxArrayString   GetPendingItems( void ) { wxArrayString RetVal = m_PendingItems; m_PendingItems.Empty(); return RetVal; }
    virtual void            AddPendingItem( const wxString &item ) { m_PendingItems.Add( item ); }
    virtual int             GetPendingItemsCount( void ) { return m_PendingItems.Count(); }
    virtual void            SetStationsOrder( const int columnid, const bool desc ) {}
    virtual void            DoUpdate( void ) {}

};
WX_DEFINE_ARRAY_PTR( guRadioProvider *, guRadioProviderArray );

}

#endif
// -------------------------------------------------------------------------------- //
