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
#ifndef __MMKEYS_H__
#define __MMKEYS_H__

#include "gudbus.h"
#include "PlayerPanel.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guMMKeys : public guDBusClient
{
  protected :
    guPlayerPanel *                 m_PlayerPanel;

  public :
    guMMKeys( guDBusServer * server, guPlayerPanel * playerpanel );
    ~guMMKeys();

    virtual DBusHandlerResult       HandleMessages( guDBusMessage * msg, guDBusMessage * reply = NULL );

    void                            GrabMediaPlayerKeys( const unsigned int time );
    void                            ReleaseMediaPlayerKeys( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
