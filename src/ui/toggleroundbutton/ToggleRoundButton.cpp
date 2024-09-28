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
#include "ToggleRoundButton.h"

#include "Images.h"
#include "Utils.h"

#include <wx/dcclient.h>
#include <wx/tglbtn.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guToggleRoundButton::guToggleRoundButton( wxWindow * parent, const wxImage &image,
                                          const wxImage &selimage, const wxImage &hoverimage ) :
    guRoundButton( parent, selimage, hoverimage )
{
    m_DisBitmap = wxBitmap( image );
    m_Value = false;
}

// -------------------------------------------------------------------------------- //
guToggleRoundButton::~guToggleRoundButton()
{
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    dc.SetBackgroundMode( wxTRANSPARENT );
    dc.DrawBitmap( m_MouseIsOver ? m_HoverBitmap :
        ( m_Value ? m_Bitmap : m_DisBitmap ),
        0 + m_IsClicked, 0 + m_IsClicked, true );
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::OnMouseEvents( wxMouseEvent &event )
{
    bool NeedPaint = false;
    if( m_Region.Contains( event.GetPosition() ) )
    {
        if( !m_MouseIsOver )
        {
            m_MouseIsOver = true;
            //Refresh();
            NeedPaint = true;
        }

        //guLogMessage( wxT( "Event %i %i %i" ), event.LeftDown(), event.LeftIsDown(), event.LeftUp() );
        if( m_IsClicked != event.LeftIsDown() )
        {
            m_IsClicked = event.LeftIsDown();
            //Refresh();
            NeedPaint = true;
        }
        if( event.LeftUp() )
        {
            m_Value = !m_Value;
            NeedPaint = true;
            // Send Clicked event
            wxCommandEvent ClickEvent( wxEVT_TOGGLEBUTTON, GetId() );
            ClickEvent.SetEventObject( this );
            ClickEvent.SetInt( m_Value );
            AddPendingEvent( ClickEvent );
        }
    }
    else
    {
        if( m_MouseIsOver )
        {
            m_MouseIsOver = false;
            //Refresh();
            NeedPaint = true;
        }
        if( m_IsClicked )
        {
            m_IsClicked = false;
            //Refresh();
            NeedPaint = true;
        }
    }
    if( NeedPaint )
        Refresh();
}

// -------------------------------------------------------------------------------- //
void guToggleRoundButton::SetValue( bool value )
{
    if( value != m_Value )
    {
        m_Value = value;
        Refresh();
    }
}

}

// -------------------------------------------------------------------------------- //
