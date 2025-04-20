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
#ifndef __PODCASTS_H__
#define __PODCASTS_H__

#include <wx/dynarray.h>
#include <wx/xml/xml.h>
#include <wx/event.h>

namespace Guayadeque {

class guDbPodcasts;

// -------------------------------------------------------------------------------- //
typedef enum {
    guPODCAST_STATUS_NORMAL,
    guPODCAST_STATUS_PENDING,
    guPODCAST_STATUS_DOWNLOADING,
    guPODCAST_STATUS_READY,
    guPODCAST_STATUS_DELETED,
    guPODCAST_STATUS_ERROR
} guPodcastStatus;

typedef enum {
    guPODCAST_DOWNLOAD_MANUALLY,
    guPODCAST_DOWNLOAD_FILTER,
    guPODCAST_DOWNLOAD_ALL
} guPodcastDownload;

typedef enum {
    guPODCAST_UPDATE_HOUR,
    guPODCAST_UPDATE_DAY,
    guPODCAST_UPDATE_WEEK,
    guPODCAST_UPDATE_MONTH
} guPodcastUpdatePeriod;

typedef enum {
    guPODCAST_DELETE_DAY,
    guPODCAST_DELETE_WEEK,
    guPODCAST_DELETE_MONTH
} guPodcastDeletePeriod;

// -------------------------------------------------------------------------------- //
class guPodcastItem
{
  protected :
    void ReadXml( wxXmlNode * XmlNode );

  public :
    int             m_Id;
    wxString        m_Title;
    wxString        m_Author;
    int             m_ChId;
    wxString        m_Channel;
    //wxString        m_Link;
    wxString        m_Summary;
    wxString        m_Enclosure;
    int             m_Time;
    unsigned int    m_Length;
    wxString        m_FileName;
    unsigned int    m_FileSize;
    wxString        m_Category;

    int             m_PlayCount;
    int             m_LastPlay;
    int             m_AddedDate;
    int             m_Status;

    guPodcastItem()
    {
        m_Id = 0;
        m_ChId = 0;
        m_Time = 0;
        m_Length = 0;
        m_PlayCount = 0;
        m_LastPlay = 0;
        m_Status = 0;
    }

    guPodcastItem( wxXmlNode * XmlNode );

};
WX_DECLARE_OBJARRAY(guPodcastItem, guPodcastItemArray);

class guDbLibrary;
class guMainFrame;

// -------------------------------------------------------------------------------- //
class guPodcastChannel
{
  protected :
    bool        ReadContent();
    bool        ReadXmlImage( wxXmlNode * XmlNode );
    bool        ReadXml( wxXmlNode * XmlNode );
    void        ReadXmlOwner( wxXmlNode * XmlNode );

    int         GetUpdateItems( guDbPodcasts * db, guPodcastItemArray * items );
    int         GetPendingChannelItems( guDbPodcasts * db, int channelid, guPodcastItemArray * items );
    void        CheckDir();

  public :
    int                 m_Id;
    wxString            m_Url;
    wxString            m_Title;
    wxString            m_Link;
    wxString            m_Description;
    wxString            m_Lang;
    wxString            m_Summary;
    wxString            m_Category;
    wxString            m_Image;
    wxString            m_Author;
    wxString            m_OwnerName;
    wxString            m_OwnerEmail;
    guPodcastItemArray  m_Items;
    int                 m_DownloadType;
    wxString            m_DownloadText;
    bool                m_AllowDelete;

                guPodcastChannel() {}
                guPodcastChannel( const wxString &url );
    void        Update( guDbPodcasts * db, guMainFrame * mainframe );
    void        CheckLogo();
    int         CheckDownloadItems( guDbPodcasts * db, guMainFrame * mainframe );
    void        CheckDeleteItems( guDbPodcasts * db );

};
WX_DECLARE_OBJARRAY(guPodcastChannel, guPodcastChannelArray);


wxDECLARE_EVENT( guPodcastEvent, wxCommandEvent );

// -------------------------------------------------------------------------------- //
class guPodcastDownloadQueueThread : public wxThread
{
  protected :
    guMainFrame *       m_MainFrame;
    wxString            m_PodcastsPath;
    guPodcastItemArray  m_Items;
    wxMutex             m_ItemsMutex;
    int                 m_CurPos;
    int                 m_GaugeId;

    void SendUpdateEvent( guPodcastItem * podcastitem );
    int  FindPodcastItem( guPodcastItem * podcastitem );

  public :
    guPodcastDownloadQueueThread( guMainFrame * mainframe );
    ~guPodcastDownloadQueueThread();

    ExitCode Entry();
    void AddPodcastItems( guPodcastItemArray * items, bool priority = false );
    void RemovePodcastItems( guPodcastItemArray * items );
    void inline Lock() { m_ItemsMutex.Lock(); }
    void inline Unlock() { m_ItemsMutex.Unlock(); }
    int GetCount();

};

}

#endif
