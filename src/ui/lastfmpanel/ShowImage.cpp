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
#include "ShowImage.h"

#include "Utils.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guShowImage::guShowImage( wxWindow * parent, wxImage * image, const wxPoint &pos ) :
    wxFrame( parent, wxID_ANY, wxEmptyString, pos, wxSize( image->GetWidth(), image->GetHeight() ), wxNO_BORDER | wxFRAME_NO_TASKBAR | wxTAB_TRAVERSAL )
{
    m_CapturedMouse = false;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition,
        wxSize( image->GetWidth(), image->GetHeight() ), 0 );
	MainSizer->Add( m_Bitmap, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    if( image )
    {
        m_Bitmap->SetBitmap( wxBitmap( image->Copy() ) );
        delete image;
    }

	Bind( wxEVT_ACTIVATE, &guShowImage::FrameActivate, this );
	Bind( wxEVT_LEFT_DOWN, &guShowImage::OnClick, this );
	Bind( wxEVT_RIGHT_DOWN, &guShowImage::OnClick, this );
	Bind( wxEVT_MOUSEWHEEL, &guShowImage::OnClick, this );

	m_Bitmap->Bind( wxEVT_MOTION, &guShowImage::OnMouse, this );
	Bind( wxEVT_MOTION, &guShowImage::OnMouse, this );
	Bind( wxEVT_MOUSE_CAPTURE_LOST, &guShowImage::OnCaptureLost, this );
}

// -------------------------------------------------------------------------------- //
guShowImage::~guShowImage()
{
    if( m_CapturedMouse )
        ReleaseMouse();

    Unbind( wxEVT_ACTIVATE, &guShowImage::FrameActivate, this );
    Unbind( wxEVT_LEFT_DOWN, &guShowImage::OnClick, this );
    Unbind( wxEVT_RIGHT_DOWN, &guShowImage::OnClick, this );
    Unbind( wxEVT_MOUSEWHEEL, &guShowImage::OnClick, this );

    m_Bitmap->Unbind( wxEVT_MOTION, &guShowImage::OnMouse, this );
    Unbind( wxEVT_MOTION, &guShowImage::OnMouse, this );
    Unbind( wxEVT_MOUSE_CAPTURE_LOST, &guShowImage::OnCaptureLost, this );
}

// -------------------------------------------------------------------------------- //
void guShowImage::OnClick( wxMouseEvent &event )
{
    Close();
}

// -------------------------------------------------------------------------------- //
void guShowImage::OnCaptureLost( wxMouseCaptureLostEvent &event )
{
    m_CapturedMouse = false;
    Close();
}

// -------------------------------------------------------------------------------- //
void guShowImage::FrameActivate( wxActivateEvent &event )
{
    if( !event.GetActive() )
      Close();
}

// -------------------------------------------------------------------------------- //
void guShowImage::OnMouse( wxMouseEvent &event )
{
    int MouseX, MouseY;
    wxGetMousePosition( &MouseX, &MouseY );

    wxRect WinRect = m_Bitmap->GetScreenRect();
    //guLogMessage( wxT( "Mouse: %i %i   %i %i %i %i" ), MouseX, MouseY, WinRect.x, WinRect.y, WinRect.width, WinRect.height );
    if( !WinRect.Contains( MouseX, MouseY ) )
    {
        Close();
    }
    else
    {
        if( !m_CapturedMouse )
        {
            m_CapturedMouse = true;
            CaptureMouse();
        }
    }
    event.Skip();
}

}

// -------------------------------------------------------------------------------- //
