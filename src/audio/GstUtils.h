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
#ifndef __GSTUTILS_H__
#define __GSTUTILS_H__

#include <gst/gst.h>

#include "Utils.h"

namespace Guayadeque {

// debugging routines

#ifdef GU_DEBUG

// log pad info
void guLogGstPadData( const char * msg, GstPad *pad );

#else

#define guLogGstPadData(...)

#endif
// GU_DEBUG

// get actual peer of the pad avoiding proxy pads
GstPad * guGetPeerPad( GstPad * pad );

// check if any of element pads is linked to another element
bool guIsGstElementLinked( GstElement *element );

// set element state to NULL if unlinked
bool guGstStateToNullIfUnlinked( GstElement *element );

// set element state to NULL and unref
bool guGstStateToNullAndUnref( GstElement *element );


}

#endif
// -------------------------------------------------------------------------------- //

