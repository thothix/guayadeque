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
#ifndef __LIBUPDATE_H__
#define __LIBUPDATE_H__

#include "DbLibrary.h"
#include "MainFrame.h"
#include "LibPanel.h"

namespace Guayadeque {

class guMediaViewer;

// -------------------------------------------------------------------------------- //
class guLibUpdateThread : public wxThread
{
  private :
    guMediaViewer *     m_MediaViewer;
    guDbLibrary *       m_Db;
    //guLibPanel *        m_LibPanel;
    guMainFrame *       m_MainFrame;
    wxArrayString       m_TrackFiles;
    wxArrayString       m_ImageFiles;
    wxArrayString       m_PlayListFiles;
    wxArrayString       m_CueFiles;
    int                 m_GaugeId;
    wxArrayString       m_LibPaths;
    int                 m_LastUpdate;
    wxArrayString       m_CoverSearchWords;
    wxString            m_ScanPath;
    bool                m_ScanAddPlayLists;
    bool                m_ScanEmbeddedCovers;
    bool                m_ScanSymlinks;

    int                 ScanDirectory( wxString dirname, bool includedir = false );
//    bool                ReadFileTags( const wxString &filename );
    void                ProcessCovers( void );


  public :
    guLibUpdateThread( guMediaViewer * mediaviewer, int gaugeid, const wxString &scanpath = wxEmptyString );
    ~guLibUpdateThread();

    ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guLibCleanThread : public wxThread
{
  private :
    guMediaViewer *     m_MediaViewer;
    guDbLibrary *       m_Db;
    guLibPanel *        m_LibPanel;
    guMainFrame *       m_MainFrame;
    wxTimer             m_ProgressTimer;

    void                OnTimer( wxTimerEvent &event );

  public :
    guLibCleanThread( guMediaViewer * mediaviewer );
    ~guLibCleanThread();

    ExitCode            Entry();
};

}

#endif
// -------------------------------------------------------------------------------- //
