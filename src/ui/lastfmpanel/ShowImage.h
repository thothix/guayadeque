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
#ifndef __SHOWIMAGE_H__
#define __SHOWIMAGE_H__

#include "DbCache.h"

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
#include <wx/frame.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guShowImage
// -------------------------------------------------------------------------------- //
class guShowImage : public wxFrame
{
  protected:
    wxStaticBitmap * m_Bitmap;
    bool             m_CapturedMouse;

    void FrameActivate( wxActivateEvent &event );
    void OnClick( wxMouseEvent &event );
    void OnCaptureLost( wxMouseCaptureLostEvent &event );
    void OnMouse( wxMouseEvent &event );

  public:
    guShowImage( wxWindow * parent, wxImage * image, const wxPoint &pos = wxDefaultPosition );
    ~guShowImage();

};

}

#endif
// -------------------------------------------------------------------------------- //
