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
#ifndef __TUNEINRADIO_H__
#define __TUNEINRADIO_H__

#include "RadioProvider.h"

namespace Guayadeque {

//#define guTUNEIN_PARTNER_ID "k2YHnXyS" "16" "GB-PVR" "ucoRiu0S"

#define guTUNEIN_BASE_URL   "http://opml.radiotime.com/Browse.ashx?partnerId=xwhZkVKi&formats=mp3,aac,wma,ogg"
//http://opml.radiotime.com/Browse.ashx?partnerId=xwhZkVKi&serial=9ffb74a78ce7a5629d80053d4e5cf943&username=&render=json&formats=mp3,aac,wma,wmpro,wmvoice,ogg,qt&locale=en&


class guDbRadios;
class guTuneInRadioProvider;

// -------------------------------------------------------------------------------- //
class guTuneInReadStationsThread : public wxThread
{
  protected :
    guRadioPanel *              m_RadioPanel;
    guTuneInRadioProvider *     m_TuneInProvider;
    wxString                    m_Url;
    guRadioStations *           m_RadioStations;
    wxArrayString               m_MoreStations;
    long                        m_MinBitRate;

    void                       ReadStations( wxXmlNode * xmlnode, guRadioStations * stations, const long minbitrate );
    int                        AddStations( const wxString &url, guRadioStations * stations, const long minbitrate );
    void                       SortStations( void );

  public :
    guTuneInReadStationsThread( guTuneInRadioProvider * tuneinprovider, guRadioPanel * radiopanel, const wxString &url, guRadioStations * stations, const long minbitrate );
    ~guTuneInReadStationsThread();

    virtual ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
class guTuneInRadioProvider : public guRadioProvider
{
  protected :
    wxTreeItemId                        m_TuneInId;
    guTuneInReadStationsThread *        m_ReadStationsThread;
    wxMutex                             m_ReadStationsThreadMutex;
    wxArrayString                       m_SearchTexts;

    void                EndReadStationsThread( void );

  public :
    guTuneInRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios );
    ~guTuneInRadioProvider();

    virtual bool            OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations = false, const int selcount = 0 );
//    virtual void              Activated( const int id );
    virtual void            SetSearchText( const wxArrayString &texts );
    virtual void            RegisterImages( wxImageList * imagelist );
    virtual void            RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem );
    virtual bool            HasItemId( const wxTreeItemId &itemid );
    virtual int             GetStations( guRadioStations * stations, const long minbitrate );
    virtual void            CancellSearchStations( void );
    wxArrayString &           GetSearchTexts( void ) { return m_SearchTexts; }

  friend class guTuneInReadStationsThread;

};

}

#endif
// -------------------------------------------------------------------------------- //
