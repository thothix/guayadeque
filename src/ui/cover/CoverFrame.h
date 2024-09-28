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
#ifndef __COVERFRAME_H__
#define __COVERFRAME_H__

#include "PlayerPanel.h"

#include <wx/wx.h>

#define guCOVERFRAME_NONE       0
#define guCOVERFRAME_DEFAULT    1
#define guCOVERFRAME_CUSTOM     2

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guCoverFrame : public wxFrame
{
  protected:
    wxStaticBitmap * m_CoverBitmap;
    bool             m_CapturedMouse;
    wxTimer *        m_AutoCloseTimer;

    void CoverFrameActivate( wxActivateEvent &event );
    void OnClick( wxMouseEvent &event );
    void OnCaptureLost( wxMouseCaptureLostEvent &event );
    void OnMouse( wxMouseEvent &event );
    void OnTimer( wxTimerEvent &event );

  public:
    guCoverFrame( wxWindow * parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 293, 258 ), long style = 0|wxTAB_TRAVERSAL );
    ~guCoverFrame();
    void SetBitmap( const guSongCoverType CoverType, const wxString &CoverPath = wxEmptyString );

};

}

#endif
// -------------------------------------------------------------------------------- //


