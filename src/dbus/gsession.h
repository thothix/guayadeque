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
#ifndef __GSESSION_H__
#define __GSESSION_H__

#include "gudbus.h"

#include <wx/wx.h>

namespace Guayadeque {

typedef enum {
    guGSESSION_STATUS_ERROR = -1,
    guGSESSION_STATUS_REGISTER_CLIENT = 0,
    guGSESSION_STATUS_INITIALIZED,
    guGSESSION_STATUS_QUERY_END_SESSION,
    guGSESSION_STATUS_END_SESSION,
    guGSESSION_STATUS_ENDED_SESSION,
    guGSESSION_STATUS_UNREGISTER
} guGSessionStatus;

// -------------------------------------------------------------------------------- //
class guGSession : public guDBusClient
{
  protected :
    int         m_Status;
    wxString    m_ObjectPath;

  public :
    guGSession( guDBusServer * server );
    ~guGSession();

    virtual DBusHandlerResult   HandleMessages( guDBusMessage * msg, guDBusMessage * reply = NULL );

};

}

#endif
// -------------------------------------------------------------------------------- //
