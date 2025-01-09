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
#include "RatingCtrl.h"

#include "Images.h"
#include "Utils.h"

#include <wx/dcclient.h>

namespace Guayadeque {

wxIMPLEMENT_DYNAMIC_CLASS( guRatingEvent, wxEvent );
wxDEFINE_EVENT( guEVT_RATING_CHANGED, guRatingEvent );

BEGIN_EVENT_TABLE( guRating, wxControl )
    EVT_PAINT( guRating::OnPaint)
    EVT_MOUSE_EVENTS( guRating::OnMouseEvents )
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guRating::guRating( wxWindow * parent, const int style ) :
    wxControl( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE )
{
    m_Rating = wxNOT_FOUND;
    m_Style = style;

    m_NormalStar   = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_normal_tiny + style ) ) );
    m_SelectStar = new wxBitmap( guImage( ( guIMAGE_INDEX ) ( guIMAGE_INDEX_star_highlight_tiny + style ) ) );
}

// -------------------------------------------------------------------------------- //
guRating::~guRating()
{
    if( m_NormalStar )
        delete m_NormalStar;
    if( m_SelectStar )
        delete m_SelectStar;
}

// -------------------------------------------------------------------------------- //
void guRating::SetRating( const int rating )
{
    m_Rating = rating;
    Refresh();
}

// -------------------------------------------------------------------------------- //
int guRating::GetRating( void )
{
    return m_Rating;
}

// -------------------------------------------------------------------------------- //
wxSize guRating::DoGetBestSize( void ) const
{
    wxSize RetVal;
    RetVal.x = 4 + ( 5 * ( ( m_Style * 2 ) + GURATING_IMAGE_SIZE ) );
    RetVal.y = ( m_Style * 2 ) + GURATING_IMAGE_SIZE + 4;
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guRating::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc( this );
    PrepareDC( dc );

    dc.SetBackgroundMode( wxTRANSPARENT );
    int x;
    int w = ( ( m_Style * 2 ) + GURATING_IMAGE_SIZE );

    for( x = 0; x < 5; x++ )
    {
       dc.DrawBitmap( ( x >= m_Rating ) ? * m_NormalStar : * m_SelectStar,
                      3 + ( w * x ), 2, true );
    }
}

// -------------------------------------------------------------------------------- //
void guRating::OnMouseEvents( wxMouseEvent &event )
{
    if( event.RightDown() || event.LeftDown() )
    {
        int SavedRating = m_Rating;
        if( event.RightDown() )
        {
            m_Rating = 0;
        }
        else if( event.LeftDown() )
        {
            int w = ( ( m_Style * 2 ) + GURATING_IMAGE_SIZE );
            if( event.m_x < 3 )
                m_Rating = 0;
            else
                m_Rating = wxMin( 5, ( wxMax( 0, event.m_x - 3 ) / w ) + 1 );
            //guLogMessage( wxT( "Clicked %i %i" ), event.m_x, m_Rating );

        }
        if( SavedRating == m_Rating )
        {
            m_Rating = 0;
        }
        if( SavedRating != m_Rating )
        {
            Refresh();
            guRatingEvent evt( guEVT_RATING_CHANGED );
            //evt.SetClientObject( this );
            evt.SetInt( m_Rating );
            wxPostEvent( this, evt );
        }
    }
}

}

// -------------------------------------------------------------------------------- //
