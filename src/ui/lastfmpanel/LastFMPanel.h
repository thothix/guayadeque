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
#ifndef __LASTFMPANEL_H__
#define __LASTFMPANEL_H__

#include "AuiNotebook.h"
#include "DbCache.h"
#include "LastFM.h"
#include "PlayerPanel.h"
#include "ThreadArray.h"
#include "TrackChangeInfo.h"

#include <wx/image.h>
#include <wx/panel.h>
#include <wx/html/htmlwin.h>
#include <wx/hyperlink.h>

namespace Guayadeque {

#define GULASTFMINFO_MAXITEMS  12

// -------------------------------------------------------------------------------- //
class guHtmlWindow : public wxHtmlWindow {
 protected :
  void OnChangedSize(wxSizeEvent &event);
  void OnScrollTo(wxCommandEvent &event);

 public :
  explicit guHtmlWindow(wxWindow *parent, wxWindowID id = wxNOT_FOUND, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxHW_DEFAULT_STYLE);
  ~guHtmlWindow() override;
};

// -------------------------------------------------------------------------------- //
class guLastFMInfo {
 public:
  int m_Index{};
  wxImage *m_Image{};
  wxString m_ImageUrl;

  guLastFMInfo() = default;

  explicit guLastFMInfo(int index, wxImage *image = nullptr) {
    m_Index = index;
    m_Image = image;
  }

  ~guLastFMInfo() {
    delete m_Image;
  }
};
WX_DECLARE_OBJARRAY(guLastFMInfo, guLastFMInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMArtistInfo : public guLastFMInfo {
 public:
  guArtistInfo *m_Artist;
  int m_ArtistId;

  guLastFMArtistInfo() {
    m_Artist = nullptr;
    m_ArtistId = wxNOT_FOUND;
  }
  explicit guLastFMArtistInfo(int index, wxImage *image = nullptr, guArtistInfo *artist = nullptr) :
      guLastFMInfo(index, image) {
    m_Artist = artist;
    m_ArtistId = wxNOT_FOUND;
  }

  ~guLastFMArtistInfo() {
    delete m_Artist;
  }
};

// -------------------------------------------------------------------------------- //
class guLastFMSimilarArtistInfo : public guLastFMInfo {
 public:
  guSimilarArtistInfo *m_Artist;
  int m_ArtistId;

  explicit guLastFMSimilarArtistInfo() {
    m_Artist = nullptr;
    m_ArtistId = wxNOT_FOUND;
  }

  explicit guLastFMSimilarArtistInfo(int index, wxImage *image = nullptr,
                                     guSimilarArtistInfo *artist = nullptr) :
      guLastFMInfo(index, image) {
    m_Artist = artist;
    m_ArtistId = wxNOT_FOUND;
  };

  ~guLastFMSimilarArtistInfo() {
    delete m_Artist;
  };
};
WX_DECLARE_OBJARRAY(guLastFMSimilarArtistInfo, guLastFMSimilarArtistInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMAlbumInfo : public guLastFMInfo {
 public:
  guAlbumInfo *m_Album;
  int m_AlbumId;

  explicit guLastFMAlbumInfo() {
    m_Album = nullptr;
    m_AlbumId = wxNOT_FOUND;
  }

  explicit guLastFMAlbumInfo(int index, wxImage *image = nullptr, guAlbumInfo *album = nullptr) :
      guLastFMInfo(index, image) {
    m_Album = album;
    m_AlbumId = wxNOT_FOUND;
  };

  ~guLastFMAlbumInfo() {
    delete m_Album;
  };
};
WX_DECLARE_OBJARRAY(guLastFMAlbumInfo, guLastFMAlbumInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMTrackInfo : public guLastFMInfo {
 public:
  guSimilarTrackInfo *m_Track;
  int m_TrackId;
  int m_ArtistId{};

  explicit guLastFMTrackInfo() {
    m_Track = nullptr;
    m_TrackId = wxNOT_FOUND;
  }

  explicit guLastFMTrackInfo(int index, wxImage *image = nullptr,
                             guSimilarTrackInfo *track = nullptr) : guLastFMInfo(index, image) {
    m_Track = track;
    m_TrackId = wxNOT_FOUND;
    m_ArtistId = wxNOT_FOUND;
  }

  ~guLastFMTrackInfo() {
    delete m_Track;
  }
};
WX_DECLARE_OBJARRAY(guLastFMTrackInfo, guLastFMTrackInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMTopTrackInfo : public guLastFMInfo {
 public:
  guTopTrackInfo *m_TopTrack;
  int m_TrackId;
  int m_ArtistId{};

  explicit guLastFMTopTrackInfo() {
    m_TopTrack = nullptr;
    m_TrackId = wxNOT_FOUND;
  }

  explicit guLastFMTopTrackInfo(int index, wxImage *image = nullptr,
                                guTopTrackInfo *track = nullptr) :
      guLastFMInfo(index, image) {
    m_TopTrack = track;
    m_TrackId = wxNOT_FOUND;
    m_ArtistId = wxNOT_FOUND;
  };

  ~guLastFMTopTrackInfo() {
    delete m_TopTrack;
  };
};
WX_DECLARE_OBJARRAY(guLastFMTopTrackInfo, guLastFMTopTrackInfoArray);

class guLastFMPanel;

// -------------------------------------------------------------------------------- //
class guFetchLastFMInfoThread : public wxThread {
 protected :
  guLastFMPanel *m_LastFMPanel;
  guThreadArray m_DownloadThreads;
  wxMutex m_DownloadThreadsMutex;

  void WaitDownloadThreads();

 public :
  explicit guFetchLastFMInfoThread(guLastFMPanel *lastfmpanel);
  ~guFetchLastFMInfoThread() override;

  friend class guDownloadImageThread;
};

// -------------------------------------------------------------------------------- //
class guDownloadImageThread : public wxThread {
 protected:
  guDbCache *m_DbCache;
  guLastFMPanel *m_LastFMPanel;
  guFetchLastFMInfoThread *m_MainThread;
  int m_CommandId;
  void *m_CommandData;
  wxImage **m_pImage;
  int m_Index;
  wxString m_ImageUrl;
  int m_ImageSize;

 public:
  guDownloadImageThread(guLastFMPanel *lastfmpanel, guFetchLastFMInfoThread *mainthread,
                        guDbCache *dbcache, int index, const wxChar *imageurl, int commandid,
                        void *commanddata, wxImage **pimage, int imagesize = guDBCACHE_TYPE_IMAGE_SIZE_TINY);
  ~guDownloadImageThread() override;

  ExitCode Entry() override;
};

// -------------------------------------------------------------------------------- //
class guFetchAlbumInfoThread : public guFetchLastFMInfoThread {
 protected:
  guDbCache *m_DbCache;
  int m_Start;
  wxString m_ArtistName;

 public:
  guFetchAlbumInfoThread(guLastFMPanel *lastfmpanel, guDbCache *dbcache, const wxChar *artistname, int startpage);
  ~guFetchAlbumInfoThread() override;

  ExitCode Entry() override;

  friend class guDownloadAlbumImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchTopTracksInfoThread : public guFetchLastFMInfoThread {
 protected:
  guDbCache *m_DbCache;
  wxString m_ArtistName;
  int m_ArtistId;
  int m_Start;

 public:
  guFetchTopTracksInfoThread(guLastFMPanel *lastfmpanel, guDbCache *dbcache, const wxChar *artistname, int startpage);
  ~guFetchTopTracksInfoThread() override;

  ExitCode Entry() override;

  friend class guDownloadAlbumImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchArtistInfoThread : public guFetchLastFMInfoThread {
 private:
  guDbCache *m_DbCache;
  wxString m_ArtistName;

 public:
  guFetchArtistInfoThread(guLastFMPanel *lastfmpanel, guDbCache *dbcache, const wxChar *artistname);
  ~guFetchArtistInfoThread() override;

  ExitCode Entry() override;

  friend class guDownloadArtistImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchSimilarArtistInfoThread : public guFetchLastFMInfoThread {
 private:
  guDbCache *m_DbCache;
  wxString m_ArtistName;
  int m_Start;

 public:
  guFetchSimilarArtistInfoThread(guLastFMPanel *lastfmpanel, guDbCache *dbcache, const wxChar *artistname, int startpage);
  ~guFetchSimilarArtistInfoThread() override;

  ExitCode Entry() override;

  friend class guDownloadArtistImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchSimTracksInfoThread : public guFetchLastFMInfoThread {
 private:
  guDbCache *m_DbCache;
  wxString m_ArtistName;
  wxString m_TrackName;
  int m_Start;

 public:
  guFetchSimTracksInfoThread(guLastFMPanel *lastfmpanel, guDbCache *dbcache, const wxChar *artistname, const wxChar *trackname, int stargpage);
  ~guFetchSimTracksInfoThread() override;

  ExitCode Entry() override;

  friend class guDownloadTrackImageThread;
};

// -------------------------------------------------------------------------------- //
class guLastFMInfoCtrl : public wxPanel {
 private:
  void OnCreateControls(wxWindow *parent);

 protected :
  guDbLibrary *m_DefaultDb;
  guDbCache *m_DbCache;
  guPlayerPanel *m_PlayerPanel;
  wxStaticBitmap *m_Bitmap;
  wxStaticText *m_Text;
  wxColour m_NormalColor;
  wxColour m_NotFoundColor;
  guMediaViewer *m_MediaViewer;
  guDbLibrary *m_Db;
  wxMutex m_DbMutex;

  virtual void OnContextMenu(wxContextMenuEvent &event);
  virtual void CreateContextMenu(wxMenu *Menu);
  virtual void OnDoubleClicked(wxMouseEvent &event);
  virtual wxString GetSearchText() { return wxEmptyString; }
  virtual wxString GetItemUrl() { return wxEmptyString; }

  virtual void OnSearchLinkClicked(wxCommandEvent &event);
  virtual void OnCopyToClipboard(wxCommandEvent &event);
  virtual void CreateControls(wxWindow *parent);
  virtual void OnPlayClicked(wxCommandEvent &event);
  virtual void OnEnqueueClicked(wxCommandEvent &event);
  virtual int GetSelectedTracks(guTrackArray *tracks) { return 0; }
  virtual void OnSongSelectName(wxCommandEvent &event) {}
  virtual void OnArtistSelectName(wxCommandEvent &event) {}
  virtual void OnAlbumSelectName(wxCommandEvent &event) {}

  //virtual void OnBitmapMouseOver( wxCommandEvent &event );
  virtual void OnBitmapClicked(wxMouseEvent &event);
  virtual wxString GetBitmapImageUrl();

  virtual void OnMouse(wxMouseEvent &event);

  virtual bool ItemWasFound() { return false; }

 public :
  guLastFMInfoCtrl(wxWindow *parent, guDbLibrary *db, guDbCache *dbcache, guPlayerPanel *playerpanel, bool createcontrols = true);
  ~guLastFMInfoCtrl() override;

  virtual void SetMediaViewer(guMediaViewer *mediaviewer);

  virtual void Clear(guMediaViewer *mediaviewer);
  virtual void SetBitmap(const wxImage *image);
  void SetLabel(const wxString &label) override;

};
WX_DEFINE_ARRAY_PTR(guLastFMInfoCtrl *, guLastFMInfoCtrlArray);

// -------------------------------------------------------------------------------- //
class guArtistInfoCtrl : public guLastFMInfoCtrl {
 private :
  guLastFMArtistInfo *m_Info;
  wxSizer *m_MainSizer;
  wxSizer *m_DetailSizer;
  guHtmlWindow *m_ArtistDetails;
  wxHyperlinkCtrl *m_ShowMoreHyperLink;
  bool m_ShowLongBioText;

  void OnCreateControls(wxWindow *parent);
  void OnContextMenu(wxContextMenuEvent &event) override;
  wxString GetSearchText() override;
  wxString GetItemUrl() override;
  void CreateContextMenu(wxMenu *Menu) override;
  void CreateControls(wxWindow *parent) override;
  void UpdateArtistInfoText();
  void OnShowMoreLinkClicked(wxHyperlinkEvent &event);
  void OnHtmlLinkClicked(wxHtmlLinkEvent &event);
  int GetSelectedTracks(guTrackArray *tracks) override;
  void OnArtistSelectName(wxCommandEvent &event) override;

  wxString GetBitmapImageUrl() override { return m_Info ? m_Info->m_ImageUrl : wxT(""); }

  bool ItemWasFound() override { return m_Info && (m_Info->m_ArtistId != wxNOT_FOUND); }

 protected :
  void OnCopyToClipboard(wxCommandEvent &event) override;

 public :
  guArtistInfoCtrl(wxWindow *parent, guDbLibrary *db, guDbCache *dbcache, guPlayerPanel *playerpanel);
  ~guArtistInfoCtrl() override;

  void SetMediaViewer(guMediaViewer *mediaviewer) override;

  void SetInfo(guLastFMArtistInfo *info);
  void Clear(guMediaViewer *mediaviewer) override;
  void SetBitmap(const wxImage *image) override;
};

// -------------------------------------------------------------------------------- //
class guAlbumInfoCtrl : public guLastFMInfoCtrl {
 private :
  guLastFMAlbumInfo *m_Info;

  void OnContextMenu(wxContextMenuEvent &event) override;
  wxString GetSearchText() override;
  wxString GetItemUrl() override;
  void CreateContextMenu(wxMenu *Menu) override;
  int GetSelectedTracks(guTrackArray *tracks) override;
  void OnAlbumSelectName(wxCommandEvent &event) override;

  wxString GetBitmapImageUrl() override { return m_Info ? m_Info->m_ImageUrl : wxT(""); }

  bool ItemWasFound() override { return m_Info && (m_Info->m_AlbumId != wxNOT_FOUND); }

 public :
  guAlbumInfoCtrl(wxWindow *parent, guDbLibrary *db, guDbCache *dbcache, guPlayerPanel *playerpanel);
  ~guAlbumInfoCtrl() override;

  void SetMediaViewer(guMediaViewer *mediaviewer) override;

  void SetInfo(guLastFMAlbumInfo *info);
  void Clear(guMediaViewer *mediaviewer) override;
};
WX_DEFINE_ARRAY_PTR(guAlbumInfoCtrl *, guAlbumInfoCtrlArray);

// -------------------------------------------------------------------------------- //
class guSimilarArtistInfoCtrl : public guLastFMInfoCtrl {
 private :
  guLastFMSimilarArtistInfo *m_Info;

  void OnContextMenu(wxContextMenuEvent &event) override;
  void CreateContextMenu(wxMenu *Menu) override;
  wxString GetSearchText() override;
  wxString GetItemUrl() override;
  int GetSelectedTracks(guTrackArray *tracks) override;
  void OnArtistSelectName(wxCommandEvent &event) override;
  void OnSelectArtist(wxCommandEvent &event);

  wxString GetBitmapImageUrl() override { return m_Info ? m_Info->m_ImageUrl : wxT(""); }

  bool ItemWasFound() override { return m_Info && (m_Info->m_ArtistId != wxNOT_FOUND); }

 public :
  guSimilarArtistInfoCtrl(wxWindow *parent, guDbLibrary *db, guDbCache *dbcache, guPlayerPanel *playerpanel);
  ~guSimilarArtistInfoCtrl() override;

  void SetMediaViewer(guMediaViewer *mediaviewer) override;

  void SetInfo(guLastFMSimilarArtistInfo *info);
  void Clear(guMediaViewer *mediaviewer) override;
};
WX_DEFINE_ARRAY_PTR(guSimilarArtistInfoCtrl *, guSimilarArtistInfoCtrlArray);

// -------------------------------------------------------------------------------- //
class guTrackInfoCtrl : public guLastFMInfoCtrl {
 private :
  guLastFMTrackInfo *m_Info;

  void OnContextMenu(wxContextMenuEvent &event) override;
  void CreateContextMenu(wxMenu *Menu) override;
  wxString GetSearchText() override;
  wxString GetItemUrl() override;
  int GetSelectedTracks(guTrackArray *tracks) override;
  void OnSongSelectName(wxCommandEvent &event) override;
  void OnArtistSelectName(wxCommandEvent &event) override;

  void OnSelectArtist(wxCommandEvent &event);

  wxString GetBitmapImageUrl() override { return m_Info ? m_Info->m_ImageUrl : wxT(""); }

  bool ItemWasFound() override { return m_Info && (m_Info->m_TrackId != wxNOT_FOUND); }

 public :
  guTrackInfoCtrl(wxWindow *parent, guDbLibrary *db, guDbCache *dbcache, guPlayerPanel *playerpanel);
  ~guTrackInfoCtrl() override;

  void SetMediaViewer(guMediaViewer *mediaviewer) override;

  void SetInfo(guLastFMTrackInfo *info);
  void Clear(guMediaViewer *mediaviewer) override;
};
WX_DEFINE_ARRAY_PTR(guTrackInfoCtrl *, guTrackInfoCtrlArray);

// -------------------------------------------------------------------------------- //
class guTopTrackInfoCtrl : public guLastFMInfoCtrl {
 private :
  guLastFMTopTrackInfo *m_Info;

  void OnContextMenu(wxContextMenuEvent &event) override;
  void CreateContextMenu(wxMenu *Menu) override;
  wxString GetSearchText() override;
  wxString GetItemUrl() override;
  int GetSelectedTracks(guTrackArray *tracks) override;
  void OnSongSelectName(wxCommandEvent &event) override;
  void OnArtistSelectName(wxCommandEvent &event) override;

  void OnSelectArtist(wxCommandEvent &event);

  wxString GetBitmapImageUrl() override { return m_Info ? m_Info->m_ImageUrl : wxT(""); }

  bool ItemWasFound() override { return m_Info && (m_Info->m_TrackId != wxNOT_FOUND); }

 public :
  guTopTrackInfoCtrl(wxWindow *parent, guDbLibrary *db, guDbCache *dbcache, guPlayerPanel *playerpanel);
  ~guTopTrackInfoCtrl() override;

  void SetMediaViewer(guMediaViewer *mediaviewer) override;

  void SetInfo(guLastFMTopTrackInfo *info);
  void Clear(guMediaViewer *mediaviewer) override;
};
WX_DEFINE_ARRAY_PTR(guTopTrackInfoCtrl *, guTopTrackInfoCtrlArray);

// -------------------------------------------------------------------------------- //
class guLastFMPanel : public wxScrolledWindow {
 private :
  guDbLibrary *m_DefaultDb;
  guDbLibrary *m_Db;
  guDbCache *m_DbCache;
  guPlayerPanel *m_PlayerPanel;
  guTrackChangeInfoArray m_TrackChangeItems;
  int m_CurrentTrackInfo;
  wxString m_ArtistName;
  wxString m_LastArtistName;
  wxString m_TrackName;
  wxString m_LastTrackName;
  guMediaViewer *m_MediaViewer;
  bool m_UpdateEnabled;

  guFetchArtistInfoThread *m_ArtistsUpdateThread;
  wxMutex m_ArtistsUpdateThreadMutex;

  guFetchAlbumInfoThread *m_AlbumsUpdateThread;
  wxMutex m_AlbumsUpdateThreadMutex;

  guFetchTopTracksInfoThread *m_TopTracksUpdateThread;
  wxMutex m_TopTracksUpdateThreadMutex;

  guFetchSimilarArtistInfoThread *m_SimArtistsUpdateThread;
  wxMutex m_SimArtistsUpdateThreadMutex;

  guFetchSimTracksInfoThread *m_SimTracksUpdateThread;
  wxMutex m_SimTracksUpdateThreadMutex;

  // TODO : Check if its really necesary
  wxMutex m_UpdateEventsMutex;

  // GUI Elements
  wxBoxSizer *m_MainSizer;

  wxCheckBox *m_UpdateCheckBox;
  wxBitmapButton *m_PrevButton;
  wxBitmapButton *m_NextButton;
  wxBitmapButton *m_ReloadButton;
  wxTextCtrl *m_ArtistTextCtrl;
  wxTextCtrl *m_TrackTextCtrl;
  wxBitmapButton *m_SearchButton;

  bool m_ShowArtistDetails;
  wxStaticText *m_ArtistDetailsStaticText;
  wxBoxSizer *m_ArtistInfoMainSizer;
  wxBoxSizer *m_ArtistDetailsSizer;

  guArtistInfoCtrl *m_ArtistInfoCtrl;

  bool m_ShowAlbums;
  wxStaticText *m_AlbumsStaticText;
  wxGridSizer *m_AlbumsSizer;
  guAlbumInfoCtrlArray m_AlbumsInfoCtrls;
  wxStaticText *m_AlbumsRangeLabel;
  wxBitmapButton *m_AlbumsPrevBtn;
  wxBitmapButton *m_AlbumsNextBtn;
  int m_AlbumsCount;
  int m_AlbumsPageStart;

  bool m_ShowTopTracks;
  wxStaticText *m_TopTracksStaticText;
  wxGridSizer *m_TopTracksSizer;
  guTopTrackInfoCtrlArray m_TopTrackInfoCtrls;
  wxStaticText *m_TopTracksRangeLabel;
  wxBitmapButton *m_TopTracksPrevBtn;
  wxBitmapButton *m_TopTracksNextBtn;
  int m_TopTracksCount;
  int m_TopTracksPageStart;

  bool m_ShowSimArtists;
  wxStaticText *m_SimArtistsStaticText;
  wxGridSizer *m_SimArtistsSizer;
  guSimilarArtistInfoCtrlArray m_SimArtistsInfoCtrls;
  wxStaticText *m_SimArtistsRangeLabel;
  wxBitmapButton *m_SimArtistsPrevBtn;
  wxBitmapButton *m_SimArtistsNextBtn;
  int m_SimArtistsCount;
  int m_SimArtistsPageStart;

  bool m_ShowSimTracks;
  wxStaticText *m_SimTracksStaticText;
  wxGridSizer *m_SimTracksSizer;
  guTrackInfoCtrlArray m_SimTracksInfoCtrls;
  wxStaticText *m_SimTracksRangeLabel;
  wxBitmapButton *m_SimTracksPrevBtn;
  wxBitmapButton *m_SimTracksNextBtn;
  int m_SimTracksCount;
  int m_SimTracksPageStart;

  bool m_ShowEvents;
  wxStaticText *m_EventsStaticText;
  wxGridSizer *m_EventsSizer;
  wxStaticText *m_EventsRangeLabel;
  wxBitmapButton *m_EventsPrevBtn;
  wxBitmapButton *m_EventsNextBtn;
  int m_EventsCount;
  int m_EventsPageStart;

  wxStaticText *m_ContextMenuObject;

  // TODO : Check if its really necesary
  wxMutex m_UpdateInfoMutex;

  void OnSetDropTarget();
  void OnUpdateArtistInfo(wxCommandEvent &event);
  void OnUpdateAlbumItem(wxCommandEvent &event);
  void OnUpdateTopTrackItem(wxCommandEvent &event);
  void OnUpdateArtistItem(wxCommandEvent &event);
  void OnUpdateTrackItem(wxCommandEvent &event);

  void OnArInfoTitleDClicked(wxMouseEvent &event);
  void OnTopAlbumsTitleDClick(wxMouseEvent &event);
  void OnTopTracksTitleDClick(wxMouseEvent &event);
  void OnSimArTitleDClick(wxMouseEvent &event);
  void OnSimTrTitleDClick(wxMouseEvent &event);

  void SetTopAlbumsVisible(bool dolayout = false);
  void SetTopTracksVisible(bool dolayout = false);
  void SetSimArtistsVisible(bool dolayout = false);
  void SetSimTracksVisible(bool dolayout = false);

  void OnUpdateChkBoxClick(wxCommandEvent &event);
  void OnPrevBtnClick(wxCommandEvent &event);
  void OnNextBtnClick(wxCommandEvent &event);
  void OnReloadBtnClick(wxCommandEvent &event);
  void OnTextUpdated(wxCommandEvent &event);
  void OnTextCtrlKeyDown(wxKeyEvent &event);
  void OnSearchSelected(wxCommandEvent &event);

  void UpdateTrackChangeButtons();

  void UpdateAlbumsRangeLabel();
  void OnAlbumsCountUpdated(wxCommandEvent &event);
  void OnAlbumsPrevClicked(wxCommandEvent &event);
  void OnAlbumsNextClicked(wxCommandEvent &event);

  void UpdateTopTracksRangeLabel();
  void OnTopTracksCountUpdated(wxCommandEvent &event);
  void OnTopTracksPrevClicked(wxCommandEvent &event);
  void OnTopTracksNextClicked(wxCommandEvent &event);

  void UpdateSimArtistsRangeLabel();
  void OnSimArtistsCountUpdated(wxCommandEvent &event);
  void OnSimArtistsPrevClicked(wxCommandEvent &event);
  void OnSimArtistsNextClicked(wxCommandEvent &event);

  void UpdateSimTracksRangeLabel();
  void OnSimTracksCountUpdated(wxCommandEvent &event);
  void OnSimTracksPrevClicked(wxCommandEvent &event);
  void OnSimTracksNextClicked(wxCommandEvent &event);

  void OnContextMenu(wxContextMenuEvent &event);

  void OnPlayClicked(wxCommandEvent &event);
  void OnEnqueueClicked(wxCommandEvent &event);
  void OnSaveClicked(wxCommandEvent &event);
  void OnCopyToClicked(wxCommandEvent &event);

  void GetContextMenuTracks(guTrackArray *tracks);

 public :
  guLastFMPanel(wxWindow *parent, guDbLibrary *db,
                guDbCache *dbcache, guPlayerPanel *playerpanel);
  ~guLastFMPanel() override;

  void OnUpdatedTrack(wxCommandEvent &event);
  void AppendTrackChangeInfo(const guTrackChangeInfo *trackchangeinfo);
  void ShowCurrentTrack();
  void SetUpdateEnable(bool value);
  void UpdateLayout() { m_MainSizer->FitInside(this); }
  void OnDropFiles(const wxArrayString &files);
  void OnDropFiles(const guTrackArray *tracks);

  guMediaViewer *GetMediaViewer() { return m_MediaViewer; }
  void SetMediaViewer(guMediaViewer *mediaviewer);

  void MediaViewerClosed(guMediaViewer *mediaviewer);

  friend class guFetchLastFMInfoThread;
  friend class guFetchArtistInfoThread;
  friend class guFetchAlbumInfoThread;
  friend class guFetchTopTracksInfoThread;
  friend class guFetchSimilarArtistInfoThread;
  friend class guFetchSimTracksInfoThread;
  friend class guFetchEventsInfoThread;
  friend class guDownloadImageThread;
};

// -------------------------------------------------------------------------------- //
class guLastFMPanelDropTarget : public wxDropTarget {
 private:
  guLastFMPanel *m_LastFMPanel;

 public:
  explicit guLastFMPanelDropTarget(guLastFMPanel *lastfmpanel);
  ~guLastFMPanelDropTarget() override;

  wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) override;
};

}

#endif
// -------------------------------------------------------------------------------- //
