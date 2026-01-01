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
#ifndef __ROUNDBUTTON_H__
#define __ROUNDBUTTON_H__

#include <wx/control.h>
#include <wx/bitmap.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guRoundButton : public wxControl
{
  protected :
    wxBitmap            m_Bitmap;
    wxBitmap            m_HoverBitmap;
    wxBitmap            m_DisBitmap;
    wxRegion            m_Region;
    bool                m_MouseIsOver;
    bool                m_IsClicked;

    DECLARE_EVENT_TABLE()

  protected :
    virtual wxSize      DoGetBestSize() const;
    virtual void        OnPaint( wxPaintEvent &event );
    virtual void        OnMouseEvents( wxMouseEvent &event );

    void                CreateRegion( void );

public :
    guRoundButton( wxWindow * parent, const wxImage &image, const wxImage &selimage );
    virtual ~guRoundButton();

    virtual void        SetBitmapLabel( const wxImage &image );
    virtual void        SetBitmapHover( const wxImage &image );
    virtual void        SetBitmapDisabled( const wxImage &image );

};

}

#endif
// -------------------------------------------------------------------------------- //
