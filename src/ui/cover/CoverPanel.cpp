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
#include "CoverPanel.h"

#include "Images.h"
#include "TagInfo.h"

#define guCOVERPANEL_RESIZE_TIMER_TIME  250
#define guCOVERPANEL_RESIZE_TIMER_ID    10

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guCoverPanel::guCoverPanel( wxWindow * parent, guPlayerPanel * playerpanel ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 100, 100 ), wxTAB_TRAVERSAL ),
    m_ResizeTimer( this, guCOVERPANEL_RESIZE_TIMER_ID )
{
    m_PlayerPanel = playerpanel;
    m_LastSize = 100;

    Bind( wxEVT_SIZE, &guCoverPanel::OnSize, this );
    Bind( wxEVT_PAINT, &guCoverPanel::OnPaint, this );
    Bind( wxEVT_TIMER, &guCoverPanel::OnResizeTimer, this, guCOVERPANEL_RESIZE_TIMER_ID );
    Bind( wxEVT_LEFT_UP, &guCoverPanel::OnClick, this);
    Bind( wxEVT_RIGHT_UP, &guCoverPanel::OnClick, this);

    wxCommandEvent Event;
    OnUpdatedTrack( Event );
}

// -------------------------------------------------------------------------------- //
guCoverPanel::~guCoverPanel()
{
    Unbind( wxEVT_SIZE, &guCoverPanel::OnSize, this );
    Unbind( wxEVT_PAINT, &guCoverPanel::OnPaint, this );
    Unbind( wxEVT_TIMER, &guCoverPanel::OnResizeTimer, this, guCOVERPANEL_RESIZE_TIMER_ID );
    Unbind( wxEVT_LEFT_UP, &guCoverPanel::OnClick, this);
    Unbind( wxEVT_RIGHT_UP, &guCoverPanel::OnClick, this);
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnPaint( wxPaintEvent &event )
{
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );
    if ( Width && Height )
    {
        wxMutexLocker Lock( m_CoverImageMutex );
        wxPaintDC dc( this );
        dc.DrawBitmap( m_CoverImage, ( Width - m_LastSize ) / 2, ( Height - m_LastSize ) / 2, false );
    }
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnResizeTimer( wxTimerEvent &event )
{
    UpdateImage();
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnClick( wxMouseEvent &event )
{
    if ( m_CoverWindow == NULL )
        m_CoverWindow = new guCoverWindow( this, wxID_ANY, wxEmptyString );
    else
        m_CoverWindow->Raise();

    if ( m_CoverWindow )
    {
        m_CoverWindow->SetBitmap( m_CoverType, m_CoverPath );
        m_CoverWindow->ShowFullScreen( true );
    }
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnSize( wxSizeEvent &event )
{
    wxSize Size = event.GetSize();
    int MinSize = wxMin( Size.GetWidth(), Size.GetHeight() );

    if ( MinSize != m_LastSize )
    {
        m_LastSize = MinSize;
        if ( m_ResizeTimer.IsRunning() )
            m_ResizeTimer.Stop();

        m_ResizeTimer.Start( guCOVERPANEL_RESIZE_TIMER_TIME, wxTIMER_ONE_SHOT );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::UpdateImage( void )
{
    wxImage * CoverImage = NULL;

    switch ( m_CoverType )
    {
        case GU_SONGCOVER_FILE :
            CoverImage = new wxImage( m_CoverPath );
            break;

        case GU_SONGCOVER_ID3TAG :
            CoverImage = guTagGetPicture( m_CoverPath );
            break;

        case GU_SONGCOVER_RADIO :
            CoverImage = new wxImage( guImage( guIMAGE_INDEX_net_radio ) );
            break;

        case GU_SONGCOVER_PODCAST :
            CoverImage = new wxImage( guImage( guIMAGE_INDEX_podcast ) );
            break;

        default :
            break;
    }

    if ( !CoverImage || !CoverImage->IsOk() )
    {
        if( CoverImage )
            delete CoverImage;
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
    }

    if ( CoverImage )
    {
        if ( m_LastSize > 0 )
            CoverImage->Rescale( m_LastSize, m_LastSize, wxIMAGE_QUALITY_HIGH );
        wxMutexLocker Lock( m_CoverImageMutex );
        m_CoverImage = wxBitmap( * CoverImage );
        //Update();
        Refresh();

        delete CoverImage;
    }
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnUpdatedTrack( wxCommandEvent &event )
{
    const guCurrentTrack * CurrentTrack = m_PlayerPanel->GetCurrentTrack();

    if ( CurrentTrack )
    {
        m_CoverType = CurrentTrack->m_CoverType;
        m_CoverPath = CurrentTrack->m_CoverPath;
        if ( m_CoverWindow != NULL )
            m_CoverWindow->SetBitmap( m_CoverType, m_CoverPath );

        guLogMessage( wxT( "Changed image to %i '%s'" ), m_CoverType, m_CoverPath.c_str() );
    }
    else
    {
        m_CoverType = GU_SONGCOVER_NONE;
        m_CoverPath = wxEmptyString;
    }
    UpdateImage();
}

}

// -------------------------------------------------------------------------------- //
