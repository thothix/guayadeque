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
#include "FaderTimeLine.h"

#include "MediaCtrl.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guFaderTimeLine::guFaderTimeLine( const int timeout, wxEvtHandler * parent, guFaderPlaybin * faderplaybin,
    double volstart, double volend ) :
    guTimeLine( timeout, parent )
{
    m_FaderPlayBin = faderplaybin;
    m_VolStart = volstart;
    m_VolEnd = volend;

    if( volstart > volend )
        m_VolStep = volstart - volend;
    else
        m_VolStep = volend - volstart;

    //guLogDebug( wxT( "Created the fader timeline for %i msecs %0.2f -> %0.2f (%0.2f)" ), timeout, volstart, volend, m_VolStep );
}

// -------------------------------------------------------------------------------- //
guFaderTimeLine::~guFaderTimeLine()
{
    //guLogDebug( wxT( "Destroyed the fader timeline" ) );
}

// -------------------------------------------------------------------------------- //
void guFaderTimeLine::ValueChanged( float value )
{
    if( m_Duration )
    {
        if( m_Direction == guTimeLine::Backward )
        {
            m_FaderPlayBin->SetFaderVolume( m_VolEnd + ( value * m_VolStep ) );
        }
        else
        {
            m_FaderPlayBin->SetFaderVolume( m_VolStart + ( value * m_VolStep ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFaderTimeLine::Finished( void )
{
    m_FaderPlayBin->SetFaderVolume( m_VolEnd );
    m_FaderPlayBin->EndFade();
}

// -------------------------------------------------------------------------------- //
static bool TimerUpdated( guFaderTimeLine * timeline )
{
    timeline->TimerEvent();
    return true;
}

// -------------------------------------------------------------------------------- //
int guFaderTimeLine::TimerCreate( void )
{
    return g_timeout_add( m_UpdateInterval, GSourceFunc( TimerUpdated ), this );
}

}

// -------------------------------------------------------------------------------- //
