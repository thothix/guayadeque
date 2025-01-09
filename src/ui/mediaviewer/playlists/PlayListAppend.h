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
#ifndef __PLAYLISTAPPEND_H__
#define __PLAYLISTAPPEND_H__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

#include "DbLibrary.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guPlayListAppend : public wxDialog
{
  protected:
    wxChoice *              m_PosChoice;
    wxComboBox *            m_PlayListComboBox;
    wxStaticText *          m_TracksStaticText;

    guDbLibrary *           m_Db;
    const wxArrayInt *      m_Tracks;
    guListItems *           m_PlayListItems;

  public:
    guPlayListAppend( wxWindow * parent, guDbLibrary * db, const wxArrayInt * songs, guListItems * items );
    ~guPlayListAppend();

    int         GetSelectedPosition( void );
    int         GetSelectedPlayList( void );
    wxString    GetPlaylistName( void );

};

int FindPlayListItem( guListItems * items, const wxString &playlistname );

}

#endif
// -------------------------------------------------------------------------------- //
