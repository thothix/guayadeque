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
#ifndef __FADERTIMELINE_H__
#define __FADERTIMELINE_H__

#include "TimeLine.h"

#include <wx/event.h>

namespace Guayadeque {

class guFaderPlaybin;

// -------------------------------------------------------------------------------- //
class guFaderTimeLine : public guTimeLine
{
  protected :
    guFaderPlaybin * m_FaderPlayBin;
    double           m_VolStep;
    double           m_VolStart;
    double           m_VolEnd;

  public :
    guFaderTimeLine( const int timeout = 3000, wxEvtHandler * parent = NULL, guFaderPlaybin * playbin = NULL,
        double volstart = 0.0, double volend = 1.0 );
    virtual ~guFaderTimeLine();

    virtual void    ValueChanged( float value );
    virtual void    Finished( void );
    virtual int     TimerCreate( void );
};

}

#endif
// -------------------------------------------------------------------------------- //
