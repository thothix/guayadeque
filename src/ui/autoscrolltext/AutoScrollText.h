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
#ifndef __AUTOSCROLLTEXT_H__
#define __AUTOSCROLLTEXT_H__

#include <wx/control.h>
#include <wx/string.h>
#include <wx/timer.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guAutoScrollText : public wxControl
{
  protected :
    wxString        m_Label;
    wxSize          m_LabelExtent;
    int             m_VisWidth;
    bool            m_AllowScroll;
    wxTimer         m_StartTimer;
    wxTimer         m_ScrollTimer;
    int             m_ScrollPos;
    int             m_ScrollQuantum;
    wxSize          m_DefaultSize;


    virtual wxSize  DoGetBestSize() const;
    void            OnPaint( wxPaintEvent &event );
    void            OnMouseEvents( wxMouseEvent &event );
    void            CalcTextExtent( void );
    void            OnSize( wxSizeEvent &event );
    void            OnScrollTimer( wxTimerEvent &event );
    void            OnStartTimer( wxTimerEvent &event );

  public :
    guAutoScrollText( wxWindow * parent, const wxString &label, const wxSize &size = wxDefaultSize );
    ~guAutoScrollText();

    void SetLabel( const wxString &label );

  private :

  DECLARE_EVENT_TABLE()

};

}

#endif
