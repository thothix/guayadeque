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
#ifndef __PLAYLISTPANEL_H__
#define __PLAYLISTPANEL_H__

#include "AuiManagerPanel.h"
#include "DbLibrary.h"
#include "PlayerPanel.h"
#include "PLSoListBox.h"
#include "SoListBox.h"


#include <wx/aui/aui.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/frame.h>
#include <wx/treectrl.h>
#include <wx/imaglist.h>
#include <wx/srchctrl.h>

namespace Guayadeque {

#define     guPANEL_PLAYLIST_TEXTSEARCH        ( 1 << 0 )

#define     guPANEL_PLAYLIST_VISIBLE_DEFAULT   ( 0 )

class guPLNamesDropTarget;
class guPlayListPanel;
class guMediaViewer;

// -------------------------------------------------------------------------------- //
// guPLNamesTreeCtrl
// -------------------------------------------------------------------------------- //
class guPLNamesTreeCtrl : public wxTreeCtrl
{
  protected :
    guDbLibrary *       m_Db;
    guPlayListPanel *   m_PlayListPanel;
    wxArrayString       m_TextSearchFilter;
    wxImageList *       m_ImageList;
    wxTreeItemId        m_RootId;
    wxTreeItemId        m_StaticId;
    wxTreeItemId        m_DynamicId;

    wxTreeItemId    m_DragOverItem;
    wxArrayInt      m_DropIds;

    void            OnContextMenu( wxTreeEvent &event );

    void            OnBeginDrag( wxTreeEvent &event );
    wxDragResult    OnDragOver( const wxCoord x, const wxCoord y );
    void            OnDropFile( const wxString &filename );
    void            OnDropTracks( const guTrackArray * tracks );
    void            OnDropEnd( void );
    void            OnKeyDown( wxKeyEvent &event );

    void            OnConfigUpdated( wxCommandEvent &event );
    void            CreateAcceleratorTable( void );

  public :
    guPLNamesTreeCtrl( wxWindow * parent, guDbLibrary * db, guPlayListPanel * playlistpanel );
    ~guPLNamesTreeCtrl();

    void            ReloadItems( const bool reset = true );

    DECLARE_EVENT_TABLE()

    friend class guPLNamesDropTarget;
    friend class guPLNamesDropFilesThread;
    friend class guPlayListPanel;
};

// -------------------------------------------------------------------------------- //
class guPLNamesDropFilesThread : public wxThread
{
  protected :
    guPLNamesTreeCtrl *     m_PLNamesTreeCtrl;          // To add the files
    guPLNamesDropTarget *   m_PLNamesDropTarget;        // To clear the thread pointer once its finished
    wxArrayString           m_Files;

    void AddDropFiles( const wxString &DirName );

  public :
    guPLNamesDropFilesThread( guPLNamesDropTarget * plnamesdroptarget,
                                 guPLNamesTreeCtrl * plnamestreectrl, const wxArrayString &files );
    ~guPLNamesDropFilesThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guPLNamesDropTarget : public wxDropTarget
{
  private:
    guPLNamesTreeCtrl *             m_PLNamesTreeCtrl;
    guPLNamesDropFilesThread *      m_PLNamesDropFilesThread;

    void ClearPlayListFilesThread( void ) { m_PLNamesDropFilesThread = NULL; }

  public:
    guPLNamesDropTarget( guPLNamesTreeCtrl * plnamestreectrl );
    ~guPLNamesDropTarget();

    virtual bool                    OnDrop( wxCoord x, wxCoord y );
    virtual wxDragResult            OnData( wxCoord x, wxCoord y, wxDragResult def );

    virtual wxDragResult            OnDragOver( wxCoord x, wxCoord y, wxDragResult def );

    friend class guPLNamesDropFilesThread;
};

// -------------------------------------------------------------------------------- //
class guPlayListPanel : public guAuiManagerPanel
{
  protected :
    guMediaViewer *     m_MediaViewer;
    guDbLibrary *       m_Db;
    guPlayerPanel *     m_PlayerPanel;
    wxString            m_ConfigPath;

    wxSplitterWindow *  m_MainSplitter;
    guPLNamesTreeCtrl * m_NamesTreeCtrl;
    guPLSoListBox *     m_PLTracksListBox;

    wxString            m_ExportLastFolder;

    wxString            m_LastSearchString;

    wxTreeItemId        m_LastSelectedItem;
    bool                m_LockSelection;

    void                OnPLNamesSelected( wxTreeEvent &event );
    void                OnPLNamesActivated( wxTreeEvent &event );
    void                OnPLNamesPlay( wxCommandEvent &event );
    void                OnPLNamesEnqueue( wxCommandEvent &event );

    void                OnPLNamesNewPlaylist( wxCommandEvent &event );
    void                OnPLNamesEditPlaylist( wxCommandEvent &event );
    void                OnPLNamesRenamePlaylist( wxCommandEvent &event );
    void                OnPLNamesDeletePlaylist( wxCommandEvent &event );
    void                OnPLNamesCopyTo( wxCommandEvent &event );

    void                OnPLNamesImport( wxCommandEvent &event );
    void                OnPLNamesExport( wxCommandEvent &event );

    virtual void        OnPLTracksActivated( wxCommandEvent &event );
    void                OnPLTracksPlayClicked( wxCommandEvent &event );
    void                OnPLTracksQueueClicked( wxCommandEvent &event );
    void                OnPLTracksDeleteClicked( wxCommandEvent &event );
    void                OnPLTracksEditLabelsClicked( wxCommandEvent &event );
    void                OnPLTracksEditTracksClicked( wxCommandEvent &event );
    void                OnPLTracksCopyToClicked( wxCommandEvent &event );
    void                OnPLTracksSavePlayListClicked( wxCommandEvent &event );
    void                OnPLTracksSetRating( wxCommandEvent &event );
    void                OnPLTracksSetField( wxCommandEvent &event );
    void                OnPLTracksEditField( wxCommandEvent &event );

    void                OnPLTracksSelectGenre( wxCommandEvent &event );
    void                OnPLTracksSelectAlbumArtist( wxCommandEvent &event );
    void                OnPLTracksSelectArtist( wxCommandEvent &event );
    void                OnPLTracksSelectAlbum( wxCommandEvent &event );
    void                OnPLTracksSelectComposer( wxCommandEvent &event );

    void                OnPLTracksColClicked( wxListEvent &event );

    void                DeleteCurrentPlayList( void );

    void                OnPLTracksDeleteLibrary( wxCommandEvent &event );
    void                OnPLTracksDeleteDrive( wxCommandEvent &event );

    virtual void        NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );

    virtual void        SendPlayListUpdatedEvent( void );

    void                OnGoToSearch( wxCommandEvent &event );
    bool                DoTextSearch( const wxString &textsearch );

    void                OnSetAllowDenyFilter( wxCommandEvent &event );

    void                CreateControls( void );

  public :
    guPlayListPanel( wxWindow * parent, guMediaViewer * mediaviewer );
    ~guPlayListPanel();

    virtual void        InitPanelData( void );

    void                PlayListUpdated( void );

    bool                GetPlayListCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size );

    void inline         UpdatedTracks( const guTrackArray * tracks ) { m_PLTracksListBox->UpdatedTracks( tracks ); }
    void inline         UpdatedTrack( const guTrack * track ) { m_PLTracksListBox->UpdatedTrack( track ); }

    virtual int         GetListViewColumnCount( void ) { return guSONGS_COLUMN_COUNT; }
    virtual bool        GetListViewColumnData( const int id, int * index, int * width, bool * enabled ) { return m_PLTracksListBox->GetColumnData( id, index, width, enabled ); }
    virtual bool        SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return m_PLTracksListBox->SetColumnData( id, index, width, enabled, refresh ); }

    void                SetPlayerPanel( guPlayerPanel * playerpanel ) { m_PlayerPanel = playerpanel; }

    void                UpdatePlaylists( void );

    void                GetPlaylistTracks( guTrackArray * tracks ) { m_PLTracksListBox->GetAllSongs( tracks ); }
    int                 GetPlaylistTrackCount( void ) { return m_PLTracksListBox->GetTrackCount(); }

    friend class guPLNamesTreeCtrl;
    friend class guMediaViewer;
};

}

#endif
// -------------------------------------------------------------------------------- //
