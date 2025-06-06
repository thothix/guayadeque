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
#ifndef __COVEREDIT_H__
#define __COVEREDIT_H__

#include "ArrayStringArray.h"
#include "AutoPulseGauge.h"
#include "ThreadArray.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/image.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/choice.h>
#include <wx/checkbox.h>

#define GUCOVERINFO_LINK    0
#define GUCOVERINFO_SIZE    1

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guCoverImage
{
  public:
    wxString    m_Link;
    wxString    m_SizeStr;
    wxImage *   m_Image;

    guCoverImage()
    {
        m_Link = wxEmptyString;
        m_Image = NULL;
    };

    guCoverImage( const wxString &link, const wxString size, wxImage * image )
    {
        m_Link = link;
        m_SizeStr = size;
        m_Image = image;
    };

    ~guCoverImage()
    {
        if( m_Image )
            delete m_Image;
    };
};
WX_DECLARE_OBJARRAY(guCoverImage, guCoverImageArray);

class guCoverEditor;
class guCoverFetcher;

// -------------------------------------------------------------------------------- //
class guFetchCoverLinksThread : public wxThread
{
  private:
    guCoverEditor *     m_CoverEditor;
    guCoverFetcher *    m_CoverFetcher;
    guArrayStringArray  m_CoverLinks;
    int                 m_CurrentPage;
    int                 m_LastDownload;
    wxString            m_Artist;
    wxString            m_Album;
    int                 m_EngineIndex;

  public:
    guFetchCoverLinksThread( guCoverEditor * owner, const wxChar * artist, const wxChar * album, int engineindex );
    ~guFetchCoverLinksThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guDownloadCoverThread : public wxThread
{
  private:
    guCoverEditor * m_CoverEditor;
    wxString        m_UrlStr;
    wxString        m_SizeStr;

  public:
    guDownloadCoverThread( guCoverEditor * Owner, const wxArrayString * CoverInfo );
    ~guDownloadCoverThread();

    virtual ExitCode Entry();
};

class guCoverFetcher;

// -------------------------------------------------------------------------------- //
// Class guCoverEditor
// -------------------------------------------------------------------------------- //
class guCoverEditor : public wxDialog
{
  private:
    wxString                    m_AlbumPath;

    wxTextCtrl *                m_ArtistTextCtrl;
    wxTextCtrl *                m_AlbumTextCtrl;
    wxChoice *                  m_EngineChoice;

    wxCheckBox *                m_EmbedToFilesChkBox;
    wxBitmapButton *            m_CoverFindAgainButton;
    wxBitmapButton *            m_CoverSelectButton;
    wxBitmapButton *            m_CoverDownloadButton;

    wxStaticBitmap *            m_CoverBitmap;
    wxBitmapButton *            m_PrevButton;
    wxBitmapButton *            m_NextButton;
    wxButton *                  m_ButtonsSizerOK;
    wxButton *                  m_ButtonsSizerCancel;
    wxBoxSizer *                m_SizeSizer;
    wxStaticText *              m_SizeStaticText;

    wxMutex                     m_DownloadThreadMutex;
    wxMutex                     m_DownloadEventsMutex;

    guAutoPulseGauge *          m_Gauge;
    wxStaticText *              m_InfoTextCtrl;

    guCoverImageArray           m_AlbumCovers;
    guFetchCoverLinksThread *   m_DownloadCoversThread;
    guThreadArray               m_DownloadThreads;
    int                         m_CurrentImage;
    int                         m_EngineIndex;

    void OnInitDialog( wxInitDialogEvent &event );
    void OnCoverFindAgainClick( wxCommandEvent &event );
    void OnEngineChanged( wxCommandEvent &event );
    void OnCoverLeftDClick( wxMouseEvent &event );
    void OnCoverLeftClick( wxMouseEvent &event );
    void OnPrevButtonClick( wxCommandEvent &event );
    void OnNextButtonClick( wxCommandEvent &event );
    void OnAddCoverImage( wxCommandEvent &event );
    void UpdateCoverBitmap();
    void EndDownloadLinksThread();
    void EndDownloadCoverThread( guDownloadCoverThread * DownloadCoverThread );
    void OnDownloadedLinks( wxCommandEvent &event );

    void OnCoverSelectClick( wxCommandEvent &event );
    void OnCoverDownloadClick( wxCommandEvent &event );

    void OnMouseWheel( wxMouseEvent &event );

  public:
    guCoverEditor(wxWindow * parent, const wxString &Artist, const wxString &Album, const wxString &albumpath = wxEmptyString);
    ~guCoverEditor();
    wxString    GetSelectedCoverUrl();
    wxImage *   GetSelectedCoverImage();
    bool        EmbedToFiles() { return m_EmbedToFilesChkBox->IsChecked(); }

  friend class guFetchCoverLinksThread;
  friend class guDownloadCoverThread;
};

}

#endif
