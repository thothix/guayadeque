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
#ifndef __COVERWINDOW_H__
#define __COVERWINDOW_H__

#include "PlayerPanel.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>

namespace Guayadeque {

class guCoverPanel;

class guCoverWindow : public wxFrame
{
  protected :
    guCoverPanel        * m_CoverPanel;

    int                 m_LastSize;
    wxBitmap            m_CoverImage;
    int                 m_CoverType;
    wxString            m_CoverPath;
    wxMutex             m_CoverImageMutex;
    wxTimer             m_ResizeTimer;
    wxPanel             * m_Panel;

    virtual void        OnSize( wxSizeEvent &event );
    virtual void        OnPaint( wxPaintEvent &event );
    virtual void        OnResizeTimer( wxTimerEvent &event );
    virtual void        OnClick( wxMouseEvent &event );
    virtual void        OnRightClick( wxMouseEvent &event );
    virtual void        OnKey( wxKeyEvent &event );

    void                UpdateImage( void );

  public :
    guCoverWindow( guCoverPanel * parent,
                   wxWindowID id = wxID_ANY,
                   const wxString& title = wxEmptyString,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxSize( 500, 500 ),
                   long style = wxNO_BORDER | wxFRAME_NO_TASKBAR | wxDEFAULT_FRAME_STYLE | wxMAXIMIZE
                 );
    ~guCoverWindow();

    void                OnUpdatedTrack( wxCommandEvent &event );
    void                SetBitmap( const guSongCoverType CoverType, const wxString &CoverPath = wxEmptyString );
};

}

#endif
