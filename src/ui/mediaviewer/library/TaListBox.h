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
#ifndef __TALISTBOX_H__
#define __TALISTBOX_H__

#include "AccelListBox.h"

namespace Guayadeque {

class guLibPanel;

// -------------------------------------------------------------------------------- //
class guTaListBox : public guAccelListBox
{
  protected :
    guLibPanel *    m_LibPanel;

    virtual void    GetItemsList( void );
    virtual void    CreateContextMenu( wxMenu * Menu ) const;
    void            AddLabel( wxCommandEvent &event );
    void            DelLabel( wxCommandEvent &event );
    void            EditLabel( wxCommandEvent &event );

    virtual void    CreateAcceleratorTable( void );

  public :

    guTaListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * NewDb, const wxString &Label );
    ~guTaListBox();

    virtual int GetSelectedSongs( guTrackArray * Songs, const bool isdrag = false ) const;

};

}

#endif
// -------------------------------------------------------------------------------- //
