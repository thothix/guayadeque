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
#ifndef __AUIMANAGEDPANEL_H__
#define __AUIMANAGEDPANEL_H__

#include <wx/wx.h>
#include <wx/aui/aui.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guAuiManagedPanel : public wxPanel
{
  protected :
    wxAuiManager *  m_Manager;

    void            OnChangedSize( wxSizeEvent &event );

  public :
    guAuiManagedPanel( wxWindow * parent, wxAuiManager * manager );
    ~guAuiManagedPanel();

    virtual void    CreateShowHideMenu( wxMenu * menu ) {}

};

}

#endif
// -------------------------------------------------------------------------------- //
