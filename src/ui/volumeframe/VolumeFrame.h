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
#ifndef __VOLUMEFRAME_H__
#define __VOLUMEFRAME_H__

#include <wx/wx.h>
#include "PlayerPanel.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guVolumeFrame : public wxFrame
{
  protected:
    guPlayerPanel * m_PlayerPanel;
    wxButton * m_IncVolButton;
    wxSlider * m_VolSlider;
    wxButton * m_DecVolButton;
    wxTimer *  m_MouseTimer;

    // Virtual event handlers, overide them in your derived class
    void VolFrameActivate( wxActivateEvent& event );
    void IncVolButtonClick( wxCommandEvent& event );
    void VolSliderChanged( wxScrollEvent& event );
    void DecVolButtonClick( wxCommandEvent& event );
    void SetVolume( void );
    void OnMouseWheel( wxMouseEvent &event );

    void OnMouse( wxMouseEvent &event );
    void OnTimer( wxTimerEvent &event );


  public:
    guVolumeFrame( guPlayerPanel * Player, wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 26,200 ), long style = 0|wxTAB_TRAVERSAL );
    ~guVolumeFrame();

};

}

#endif
// -------------------------------------------------------------------------------- //
