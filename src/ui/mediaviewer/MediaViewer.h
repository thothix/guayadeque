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
#ifndef __MEDIAVIEWER_H__
#define __MEDIAVIEWER_H__

#include "AlbumBrowser.h"
#include "AuiNotebook.h"
#include "Collections.h"
#include "LibPanel.h"
#include "PlayerPanel.h"
#include "PlayListPanel.h"
#include "Preferences.h"
#include "TreePanel.h"

#include <wx/dynarray.h>

namespace Guayadeque {

enum guMediaViewerMode {
    guMEDIAVIEWER_MODE_NONE = -1,
    guMEDIAVIEWER_MODE_LIBRARY,
    guMEDIAVIEWER_MODE_ALBUMBROWSER,
    guMEDIAVIEWER_MODE_TREEVIEW,
    guMEDIAVIEWER_MODE_PLAYLISTS
};

enum guMediaViewerCommand {
    guMEDIAVIEWER_SHOW_LIBRARY,
    guMEDIAVIEWER_SHOW_ALBUMBROWSER,
    guMEDIAVIEWER_SHOW_TREEVIEW,
    guMEDIAVIEWER_SHOW_PLAYLISTS,
    guMEDIAVIEWER_SHOW_TEXTSEARCH,
    guMEDIAVIEWER_SHOW_LABELS,
    guMEDIAVIEWER_SHOW_GENRES,
    guMEDIAVIEWER_SHOW_ARTISTS,
    guMEDIAVIEWER_SHOW_COMPOSERS,
    guMEDIAVIEWER_SHOW_ALBUMARTISTS,
    guMEDIAVIEWER_SHOW_ALBUMS,
    guMEDIAVIEWER_SHOW_YEARS,
    guMEDIAVIEWER_SHOW_RATINGS,
    guMEDIAVIEWER_SHOW_PLAYCOUNTS,
    guMEDIAVIEWER_SHOW_DIRECTORIES
};

enum guMediaViewerSelect {
    guMEDIAVIEWER_SELECT_TRACK,
    guMEDIAVIEWER_SELECT_ARTIST,
    guMEDIAVIEWER_SELECT_ALBUM,
    guMEDIAVIEWER_SELECT_ALBUMARTIST,
    guMEDIAVIEWER_SELECT_COMPOSER,
    guMEDIAVIEWER_SELECT_YEAR,
    guMEDIAVIEWER_SELECT_GENRE,
    guMEDIAVIEWER_SELECT_DIRECTORY
};

#define     guCONTEXTMENU_EDIT_TRACKS       ( 1 << 0 )
#define     guCONTEXTMENU_DOWNLOAD_COVERS   ( 1 << 1 )
#define     guCONTEXTMENU_EMBED_COVERS      ( 1 << 2 )
#define     guCONTEXTMENU_COPY_TO           ( 1 << 3 )
#define     guCONTEXTMENU_LINKS             ( 1 << 4 )
#define     guCONTEXTMENU_COMMANDS          ( 1 << 5 )
#define     guCONTEXTMENU_DELETEFROMLIBRARY ( 1 << 6 )

#define     guCONTEXTMENU_DEFAULT           ( guCONTEXTMENU_EDIT_TRACKS | guCONTEXTMENU_DOWNLOAD_COVERS |\
                                              guCONTEXTMENU_EMBED_COVERS | guCONTEXTMENU_COPY_TO |\
                                              guCONTEXTMENU_LINKS | guCONTEXTMENU_COMMANDS |\
                                              guCONTEXTMENU_DELETEFROMLIBRARY )

class guMainFrame;
class guLibUpdateThread;
class guLibCleanThread;
class guCopyToAction;
class guUpdateCoversThread;

// -------------------------------------------------------------------------------- //
class guMediaViewer : public wxPanel
{
  protected :
    bool                    m_IsDefault;
    guMediaCollection *     m_MediaCollection;
    guDbLibrary *           m_Db;
    guMainFrame *           m_MainFrame;
    guPlayerPanel *         m_PlayerPanel;
    int                     m_BaseCommand;
    int                     m_ViewMode;
    wxThread *              m_UpdateThread;
    guLibCleanThread *      m_CleanThread;
    wxBoxSizer *            m_FiltersSizer;
    guCopyToPattern *       m_CopyToPattern;
    guUpdateCoversThread *  m_UpdateCoversThread;

    wxChoice *              m_FilterChoice;
    wxBitmapButton *        m_AddFilterButton;
    wxBitmapButton *        m_DelFilterButton;
    wxBitmapButton *        m_EditFilterButton;
    wxSearchCtrl *          m_SearchTextCtrl;
    wxBitmapButton *        m_LibrarySelButton;
    wxBitmapButton *        m_AlbumBrowserSelButton;
    wxBitmapButton *        m_TreeViewSelButton;
    wxBitmapButton *        m_PlaylistsSelButton;

    guLibPanel *            m_LibPanel;
    guAlbumBrowser *        m_AlbumBrowser;
    guTreeViewPanel *       m_TreeViewPanel;
    guPlayListPanel *       m_PlayListPanel;

    wxTimer                 m_TextChangedTimer;
    bool                    m_DoneClearSearchText;
    bool                    m_InstantSearchEnabled;
    bool                    m_EnterSelectSearchEnabled;

    wxString                m_ConfigPath;
    wxArrayString           m_DynFilterArray;
    wxString                m_SearchText;

    int                     m_ContextMenuFlags;

    wxString                m_SmartPlaylistName;
    int                     m_SmartPlaylistId;
    wxArrayInt              m_SmartTracksList;
    wxArrayString           m_SmartArtistsList;

    void                    OnViewChanged( wxCommandEvent &event );

    // Search Str events
    void                    OnSearchActivated( wxCommandEvent &event );
    void                    OnSearchCancelled( wxCommandEvent &event );
    void                    OnSearchSelected( wxCommandEvent &event );
    void                    OnTextChangedTimer( wxTimerEvent &event );
    virtual bool            DoTextSearch();


    virtual void            CreateControls();

    virtual void            PlayAllTracks( const bool enqueue );

    virtual void            OnConfigUpdated( wxCommandEvent &event );

    void                    OnAddFilterClicked( wxCommandEvent &event );
    void                    OnDelFilterClicked( wxCommandEvent &event );
    void                    OnEditFilterClicked( wxCommandEvent &event );
    void                    OnFilterSelected( wxCommandEvent &event );
    void                    SetFilter( const wxString &filter );

    void                    OnCleanFinished( wxCommandEvent &event ) { CleanFinished(); }
    void                    OnLibraryUpdated( wxCommandEvent &event ) { LibraryUpdated(); }

    void                    OnAddPath();

    virtual void            LoadMediaDb();

    virtual void            CreateAcceleratorTable();

    virtual void            OnGenreSetSelection( wxCommandEvent &event );
    virtual void            OnAlbumArtistSetSelection( wxCommandEvent &event );
    virtual void            OnComposerSetSelection( wxCommandEvent &event );
    virtual void            OnArtistSetSelection( wxCommandEvent &event );
    virtual void            OnAlbumSetSelection( wxCommandEvent &event );

    virtual void            OnUpdateLabels( wxCommandEvent &event );

    virtual void            OnSmartAddTracks( wxCommandEvent &event );

  public :
    guMediaViewer( wxWindow * parent, guMediaCollection & mediacollection, const int basecommand, guMainFrame * mainframe, const int mode, guPlayerPanel * playerpanel );
    ~guMediaViewer();

    virtual void            InitMediaViewer( const int mode );

    virtual wxString        ConfigPath() { return m_ConfigPath; }

    int                     GetBaseCommand() { return m_BaseCommand; }

    bool                    IsDefault() { return m_IsDefault; }
    void                    SetDefault( const bool isdefault ) { m_IsDefault = isdefault; }

    void                    ClearSearchText();

    void                    GoToSearch();

    virtual int             GetContextMenuFlags() { return m_ContextMenuFlags; }
    virtual void            CreateContextMenu( wxMenu * menu, const int windowid = wxNOT_FOUND );
    virtual void            CreateCopyToMenu( wxMenu * menu );

    int                     GetViewMode() { return m_ViewMode; }
    virtual void            SetViewMode( const int mode );

    guDbLibrary *           GetDb() { return m_Db; }
    guPlayerPanel *         GetPlayerPanel() { return m_PlayerPanel; }
    void                    SetPlayerPanel( guPlayerPanel * playerpanel );
    guMainFrame *           GetMainFrame() { return m_MainFrame; }
    guLibPanel *            GetLibPanel() { return m_LibPanel; }
    guAlbumBrowser *        GetAlbumBrowser() { return m_AlbumBrowser; }
    guTreeViewPanel *       GetTreeViewPanel() { return m_TreeViewPanel; }
    guPlayListPanel *       GetPlayListPanel() { return m_PlayListPanel; }

    virtual bool            CreateLibraryView();
    virtual bool            CreateAlbumBrowserView();
    virtual bool            CreateTreeView();
    virtual bool            CreatePlayListView();

    virtual void            SetMenuState( const bool enabled = true );
    virtual void            ShowPanel( const int id, const bool enabled );

    virtual void            HandleCommand( const int command );

    // Collections related
    guMediaCollection *     GetMediaCollection() { return m_MediaCollection; }
    virtual void            SetCollection( guMediaCollection &collection, const int basecommand );
    virtual wxString        GetUniqueId() { return m_MediaCollection->m_UniqueId; }
    virtual int             GetType() { return m_MediaCollection->m_Type; }
    virtual wxString        GetName() { return m_MediaCollection->m_Name; }
    virtual wxArrayString   GetPaths() { return m_MediaCollection->m_Paths; }
    virtual wxArrayString   GetCoverWords() { return m_MediaCollection->m_CoverWords; }
    virtual bool            GetUpdateOnStart() { return m_MediaCollection->m_UpdateOnStart; }
    virtual bool            GetScanPlaylists() { return m_MediaCollection->m_ScanPlaylists; }
    virtual bool            GetScanFollowSymLinks() { return m_MediaCollection->m_ScanFollowSymLinks; }
    virtual bool            GetScanEmbeddedCovers() { return m_MediaCollection->m_ScanEmbeddedCovers; }
    virtual bool            GetEmbeddMetadata() { return m_MediaCollection->m_EmbeddMetadata; }
    virtual wxString        GetDefaultCopyAction() { return m_MediaCollection->m_DefaultCopyAction; }
    virtual int             GetLastUpdate() { return m_MediaCollection->m_LastUpdate; }
    virtual void            SetLastUpdate();

    void                    UpdateLibrary( const wxString &path = wxEmptyString );
    virtual void            UpdateFinished();
    virtual void            UpgradeLibrary();
    virtual void            CleanLibrary();
    virtual void            CleanFinished();

    virtual void            UpdatePlaylists();

    virtual void            LibraryUpdated();

    virtual void            UpdateCovers();
    virtual void            UpdateCoversFinished();

    virtual void            ImportFiles() { ImportFiles( new guTrackArray() ); }
    virtual void            ImportFiles( guTrackArray * tracks );
    virtual void            ImportFiles( const wxArrayString &files );

    virtual void            SaveLayout( wxXmlNode * xmlnode );
    virtual void            LoadLayout( wxXmlNode * xmlnode );

    virtual wxString        GetSelInfo();

    virtual void            UpdatedTrack( const int updatedby, const guTrack * track );
    virtual void            UpdatedTracks( const int updatedby, const guTrackArray * tracks );

    virtual int             CopyTo( guTrack * track, guCopyToAction &copytoaction, wxString &filename, const int index );

    virtual void            NormalizeTracks( guTrackArray * tracks, const bool isdrag = false ) {}

    virtual void            DownloadAlbumCover( const int albumid );
    virtual void            SelectAlbumCover( const int albumid );
    virtual void            EmbedAlbumCover( const int albumid );
    virtual bool            SetAlbumCover( const int albumid, const wxString &coverpath, const bool update = true );
    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg );
    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath );
    virtual void            DeleteAlbumCover( const int albumid );
    virtual void            DeleteAlbumCover( const wxArrayInt &albumids );
    virtual void            AlbumCoverChanged( const int album, const bool deleted = false );

    virtual wxString        GetCoverName( const int albumid );
    virtual wxBitmapType    GetCoverType() { return wxBITMAP_TYPE_JPEG; }
    virtual int             GetCoverMaxSize() { return 0; }

    virtual wxImage *       GetAlbumCover( const int albumid, int &coverid, wxString &coverpath,
                                           const wxString &artistname = wxEmptyString, const wxString &albumname = wxEmptyString );
    virtual int             GetAlbumCoverId( const int albumid ) { return m_Db->GetAlbumCoverId( albumid ); }

    virtual bool            FindMissingCover( const int albumid, const wxString &artistname,
                                              const wxString &albumname, const wxString &albumpath  );

    virtual void            SetSelection( const int type, const int id );

    virtual void            PlayListUpdated();

    virtual void            EditProperties();

    virtual void            DeleteTracks( const guTrackArray * tracks );

    virtual void            UpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                                          const wxArrayString &lyrics, const wxArrayInt &changedflags );

    virtual void            SetTracksRating( guTrackArray &tracks, const int rating );

    // Copy to support functions
    virtual wxString        AudioPath();
    virtual wxString        Pattern();
    virtual int             AudioFormats();
    virtual int             TranscodeFormat();
    virtual int             TranscodeScope();
    virtual int             TranscodeQuality();
    virtual bool            MoveFiles();
    virtual int             PlaylistFormats();
    virtual wxString        PlaylistPath();
    virtual int             CoverFormats() { return 2; } //guPORTABLEMEDIA_COVER_FORMAT_JPEG
    virtual wxString        CoverName() { return GetCoverName( wxNOT_FOUND ); }
    virtual int             CoverSize() { return 0; }

    virtual void            CreateSmartPlaylist( const wxString &artistname, const wxString &trackname );
    virtual void            CreateBestOfPlaylist( const guTrack &track );
    virtual void            CreateBestOfPlaylist( const wxString &artistname );

};
WX_DEFINE_ARRAY_PTR( guMediaViewer *, guMediaViewerArray );

// -------------------------------------------------------------------------------- //
class guUpdateCoversThread : public wxThread
{
  private:
    guMediaViewer *     m_MediaViewer;
    int                 m_GaugeId;

  public:
    guUpdateCoversThread( guMediaViewer * mediaviewer, int gaugeid );
    ~guUpdateCoversThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guMediaViewerDropTarget : public wxDropTarget
{
  protected :
    guMediaViewer * m_MediaViewer;

  public :
    guMediaViewerDropTarget( guMediaViewer * libpanel );
    ~guMediaViewerDropTarget();

    virtual wxDragResult OnData( wxCoord x, wxCoord y, wxDragResult def );
};

}

#endif
