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
#ifndef __FILEBROWSER_H__
#define __FILEBROWSER_H__

#include "AuiManagerPanel.h"
#include "AuiNotebook.h"
#include "DbLibrary.h"
#include "PlayerPanel.h"
#include "Utils.h"

#include <wx/colour.h>
#include <wx/dirctrl.h>
#include <wx/dynarray.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/tglbtn.h>
#include <wx/gtk/tglbtn.h>

namespace Guayadeque {

#define guPANEL_FILEBROWSER_DIRCTRL             ( 1 << 0 )
#define guPANEL_FILEBROWSER_FILECTRL            ( 1 << 1 )
#define guPANEL_FILEBROWSER_FILEDETAILS         ( 1 << 2 )

#define guPANEL_FILEBROWSER_VISIBLE_DEFAULT     ( guPANEL_FILEBROWSER_DIRCTRL | guPANEL_FILEBROWSER_FILECTRL |\
                                                      guPANEL_FILEBROWSER_FILEDETAILS )
#define guFILEBROWSER_SHOWPATH_SYSTEM           0
#define guFILEBROWSER_SHOWPATH_LOCATIONS        1

#define guFILEBROWSER_IMAGE_INDEX_FOLDER        0
#define guFILEBROWSER_IMAGE_INDEX_OTHER         7
#define guFILEBROWSER_IMAGE_INDEX_LIBRARY       9
#define guFILEBROWSER_IMAGE_INDEX_PODCASTS      10
#define guFILEBROWSER_IMAGE_INDEX_RECORDS       11
#define guFILEBROWSER_IMAGE_INDEX_AUDIO         12
#define guFILEBROWSER_IMAGE_INDEX_IMAGE         13

#define guFILEBROWSER_COLUMN_NAME       0
#define guFILEBROWSER_COLUMN_SIZE       1
#define guFILEBROWSER_COLUMN_TIME       2
#define guFILEBROWSER_COLUMN_COUNT      3

// -------------------------------------------------------------------------------- //
guMediaViewer * FindMediaViewerByPath( guMainFrame * mainframe, const wxString curpath );
void AppendFolderCommands( wxMenu * menu );
bool RemoveDirItems( const wxString &path, wxArrayString * deletefiles );

class guFileBrowserDirCtrl;

// -------------------------------------------------------------------------------- //
class guGenericDirCtrl : public wxGenericDirCtrl
{
  protected :
    guMainFrame *           m_MainFrame;
    guFileBrowserDirCtrl *  m_FileBrowserDirCtrl;
    wxString                m_RenameName;
    wxTreeItemId            m_RenameItemId;
    int                     m_ShowPaths;

    void            OnBeginRenameDir( wxTreeEvent &event );
    void            OnEndRenameDir( wxTreeEvent &event );

  public :
    guGenericDirCtrl() : wxGenericDirCtrl() { m_FileBrowserDirCtrl = nullptr; }
    guGenericDirCtrl(wxWindow * parent, guMainFrame * mainframe, int showpaths);
    ~guGenericDirCtrl() override;

    void            OnConfigUpdated( wxCommandEvent &event );

    void            SetupSections() override;

    void            FolderRename();

    void            SetShowPaths( int showpaths ) { m_ShowPaths = showpaths; }
    int             GetShowPaths() { return m_ShowPaths; }

    DECLARE_EVENT_TABLE()
};

// -------------------------------------------------------------------------------- //
class guFileBrowserDirCtrl : public wxPanel
{
  protected :
    guMainFrame *       m_MainFrame;
    guDbLibrary *       m_DefaultDb;
    guDbLibrary *       m_Db;
    guMediaViewer *     m_MediaViewer;
    guGenericDirCtrl *  m_DirCtrl;
    bool                m_AddingFolder;
    wxBitmapToggleButton *  m_ShowLibPathsBtn;

    void                OnShowLibPathsClick( wxCommandEvent& event );

    wxImageList *       GetImageList( void ) { return m_DirCtrl->GetTreeCtrl()->GetImageList(); }

    void                OnContextMenu( wxTreeEvent &event );

    void                OnConfigUpdated( wxCommandEvent &event );
    void                CreateAcceleratorTable();
    static void         AppendFolderCommands( wxMenu * menu );

    bool                CheckClipboardForValidFile() const;

  public :
    guFileBrowserDirCtrl( wxWindow * parent, guMainFrame * mainframe, guDbLibrary * db, const wxString &dirpath );
    ~guFileBrowserDirCtrl() override;

    wxString            GetPath() { return GetPathAddTrailSep(m_DirCtrl->GetPath()); }

    void                SetPath( const wxString &path, guMediaViewer * mediaviewer );
    void                SetMediaViewer( guMediaViewer * mediaviewer );

    void                MoveDir( const wxString &oldname, const wxString &newname );
    void                RenameDir( const wxString &oldname, const wxString &newname );
    void                FolderRename() { m_DirCtrl->FolderRename(); }
    void                FolderNew();
    void                FolderDelete();
    bool                ExpandPath( const wxString &path ) { return m_DirCtrl->ExpandPath( path ); }
    bool                CollapsePath( const wxString &path ) { return m_DirCtrl->CollapsePath( path ); }

    void                CollectionsUpdated();

    int                 GetShowPaths() { return m_DirCtrl->GetShowPaths(); }

  friend class guFileBrowserFileCtrl;
  friend class guFileBrowser;
};

// -------------------------------------------------------------------------------- //
class guFilesListBox : public guListView
{
  protected :
    guDbLibrary *               m_DefaultDb;
    guDbLibrary *               m_Db;
    wxString                    m_CurDir;
    guFileItemArray             m_Files;
    wxImageList *               m_TreeImageList;
    int                         m_Order;
    bool                        m_OrderDesc;
    guMediaViewer *             m_MediaViewer;

    void                        CreateContextMenu( wxMenu * Menu ) const override;
    wxString                    OnGetItemText( const int row, const int column ) const override;
    void                        DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const override;
    void                        GetItemsList() override;
    int                         GetSelectedSongs( guTrackArray * tracks, const bool isdrag = false ) const override;
    int                         GetAllSongs( guTrackArray * tracks ) const;
    int                         GetTracksFromFiles( const wxArrayString &files, guTrackArray * tracks ) const;

    size_t                      GetDragFiles( guDataObjectComposite * files ) override;

    wxArrayString               GetColumnNames( void );

    size_t                      GetPathSortedItems(const wxString &path, guFileItemArray * items,
                                                   const int order, const bool orderdesc,
                                                   const bool recursive = false) const;

    void                        OnConfigUpdated( wxCommandEvent &event );
    void                        CreateAcceleratorTable();

  public :
    guFilesListBox( wxWindow * parent, guDbLibrary * db );
    ~guFilesListBox() override;

    void                        ReloadItems( bool reset = true ) override;

    wxString inline             GetItemName( int item ) const override;
    int inline                  GetItemId( int item ) const override;

    void                        SetOrder( int order );
    void                        SetPath( const wxString &path, guMediaViewer * mediaviewer );
    wxString                    GetPath(int item, bool absolute = true ) const;
    int                         GetType( int item ) const;
    void                        SetTreeImageList( wxImageList * imagelist ) { m_TreeImageList = imagelist; }

    bool                        GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size );

    wxArrayString               GetSelectedFiles( bool recursive = false ) const;
    wxArrayString               GetAllFiles( bool recursive = false ) const;

  friend class guFileBrowserFileCtrl;
};

// -------------------------------------------------------------------------------- //
class guFileBrowserFileCtrl : public wxPanel
{
  protected :
    guDbLibrary *           m_DefaultDb;
    guDbLibrary *           m_Db;
    guFilesListBox *        m_FilesListBox;
    guFileBrowserDirCtrl *  m_DirCtrl;
    guMediaViewer *         m_MediaViewer;

  public :
    guFileBrowserFileCtrl( wxWindow * parent, guDbLibrary * db, guFileBrowserDirCtrl * dirctrl );
    ~guFileBrowserFileCtrl() override;

    void                    SetPath( const wxString &path, guMediaViewer * mediaviewer );
    const wxString          GetPath( const int item, const bool absolute = true ) const { return m_FilesListBox->GetPath( item, absolute ); }
    int                     GetType( const int item ) const { return m_FilesListBox->GetType( item ); }
    wxArrayInt              GetSelectedItems( const bool convertall = true ) { return m_FilesListBox->GetSelectedItems( convertall ); }
    wxArrayString           GetSelectedFiles( const bool recursive = false ) { return m_FilesListBox->GetSelectedFiles( recursive ); }
    wxArrayString           GetAllFiles( const bool recursive = false ) { return m_FilesListBox->GetAllFiles( recursive ); }
    int                     GetSelectedSongs( guTrackArray * tracks ) { return m_FilesListBox->GetSelectedSongs( tracks ); }
    int                     GetAllSongs( guTrackArray * tracks ) { return m_FilesListBox->GetAllSongs( tracks ); }
    void                    SetOrder( const int order ) { m_FilesListBox->SetOrder( order ); }

    bool                    GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
    {
        return m_FilesListBox->GetCounters( count, len, size );
    }

    bool                    GetColumnData( const int id, int * index, int * width, bool * enabled ) { return m_FilesListBox->GetColumnData( id, index, width, enabled ); }
    bool                    SetColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return m_FilesListBox->SetColumnData( id, index, width, enabled, refresh ); }

};

// -------------------------------------------------------------------------------- //
class guFileBrowser : public guAuiManagerPanel
{
  protected :
    guMainFrame *           m_MainFrame;
	guDbLibrary *           m_DefaultDb;
	guDbLibrary *           m_Db;
	guMediaViewer *         m_MediaViewer;
	guPlayerPanel *         m_PlayerPanel;

	guFileBrowserDirCtrl *  m_DirCtrl;
	guFileBrowserFileCtrl * m_FilesCtrl;

    void                    OnDirItemChanged( wxTreeEvent &event );
    void                    OnFileItemActivated( wxCommandEvent &Event );
    void                    OnFilesColClick( wxListEvent &event );
    void                    OnDirBeginDrag( wxTreeEvent &event );

    void                    OnFolderPlay( wxCommandEvent &event );
    void                    OnFolderEnqueue( wxCommandEvent &event );
    void                    OnFolderNew( wxCommandEvent &event );
    void                    OnFolderRename( wxCommandEvent &event );
    void                    OnFolderDelete( wxCommandEvent &event );
    void                    OnFolderCopy( wxCommandEvent &event );
    void                    OnFolderPaste( wxCommandEvent &event );
    void                    OnFolderMove( wxCommandEvent &event );
    void                    OnFolderEditTracks( wxCommandEvent &event );
    void                    OnFolderSaveToPlayList( wxCommandEvent &event );
    void                    OnFolderUpdate( wxCommandEvent &event );
    void                    OnFolderCopyTo( wxCommandEvent &event );
    void                    OnFolderCommand( wxCommandEvent &event );

    void                    OnItemsPlay( wxCommandEvent &event );
    void                    OnItemsEnqueue( wxCommandEvent &event );
    void                    OnItemsEditTracks( wxCommandEvent &event );
    void                    OnItemsSaveToPlayList( wxCommandEvent &event );
    void                    OnItemsCopyTo( wxCommandEvent &event );
    void                    OnItemsRename( wxCommandEvent &event );
    void                    OnItemsDelete( wxCommandEvent &event );
    void                    OnItemsCommand( wxCommandEvent &event );
    void                    OnItemsCopy( wxCommandEvent &event );
    void                    OnItemsPaste( wxCommandEvent &event );

    DECLARE_EVENT_TABLE()

  public :
    guFileBrowser( wxWindow * parent, guMainFrame * mainframe, guDbLibrary * db, guPlayerPanel * playerpanel );
    ~guFileBrowser() override;

    //virtual void            InitPanelData();
    bool    GetCounters(wxLongLong * count, wxLongLong * len, wxLongLong * size) { return m_FilesCtrl->GetCounters(count, len, size); }

    bool    GetListViewColumnData(const int id, int * index, int * width, bool * enabled) override
    {
        return m_FilesCtrl->GetColumnData(id, index, width, enabled);
    }

    bool    SetListViewColumnData(const int id, const int index, const int width, const bool enabled, const bool refresh = false) override
    {
        return m_FilesCtrl->SetColumnData(id, index, width, enabled, refresh);
    }

    virtual void CollectionsUpdated() {
        m_DirCtrl->CollectionsUpdated();
    }
};

}

#endif
