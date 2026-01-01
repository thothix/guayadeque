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
#ifndef __USERRADIO_H__
#define __USERRADIO_H__

#include "RadioProvider.h"

namespace Guayadeque {

class guDbRadios;

// -------------------------------------------------------------------------------- //
class guUserRadioProvider : public guRadioProvider
{
  protected :
    wxTreeItemId        m_ManualId;

    void               OnRadioAdd( wxCommandEvent &event );
    void               OnRadioEdit( wxCommandEvent &event );
    void               OnRadioDelete( wxCommandEvent &event );
    void               OnRadioImport( wxCommandEvent &event );
    void               OnRadioExport( wxCommandEvent &event );

  public :
    guUserRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios );
    ~guUserRadioProvider();

    virtual bool        OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations = false, const int selcount = 0 );
    virtual void        RegisterImages( wxImageList * imagelist );
    virtual void        RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem );
    virtual bool        HasItemId( const wxTreeItemId &itemid ) { return itemid == m_ManualId; }
    virtual int         GetStations( guRadioStations * stations, const long minbitrate );


};

}

#endif
// -------------------------------------------------------------------------------- //
