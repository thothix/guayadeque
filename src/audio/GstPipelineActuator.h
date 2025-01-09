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
#ifndef __GSTPIPELINEACTUATOR_H__
#define __GSTPIPELINEACTUATOR_H__

#include "Utils.h"
#include "GstTypes.h"
#include "GstUtils.h"

namespace Guayadeque {

/*

Pipeline actuator class to support adding & removing pipeline elements on the fly

*/
// -------------------------------------------------------------------------------- //
class guGstPipelineActuator
{
  private :
    guGstElementsChain              * m_Chain           = NULL;
    bool                            m_PrivateChain      = false;
    guGstResultHandler              * m_ResultHandler   = NULL;

  public :
    guGstPipelineActuator() {};
    guGstPipelineActuator( GstElement *element );
    guGstPipelineActuator( guGstElementsChain *chain );
    ~guGstPipelineActuator();

    void SetHandler( guGstResultHandler *rhandler ) { m_ResultHandler = rhandler; }

    bool Enable( GstElement *element, void * new_data = NULL );
    bool Enable( int element_nr, void * new_data = NULL ) { return Enable( m_Chain->at( element_nr ), new_data ); }
    bool Enable() { return Enable(0); }

    bool Disable( GstElement *element, void * new_data = NULL );
    bool Disable( int element_nr, void * new_data = NULL ) { return Disable( m_Chain->at( element_nr ), new_data ); }
    bool Disable() { return Disable(0); }

    bool Toggle( GstElement *element, void * new_data = NULL );
    bool Toggle( int element_nr, void * new_data = NULL ) { return Toggle( m_Chain->at( element_nr ), new_data ); }
    bool Toggle()  { return Toggle(0); }

};

}

#endif
// -------------------------------------------------------------------------------- //
