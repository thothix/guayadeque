/*
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
#ifndef __DIBROWSER_H__
#define __DIBROWSER_H__

#include "AccelListBox.h"

#include "Utils.h"

#include <wx/dirctrl.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/tglbtn.h>

namespace Guayadeque {

#define guDIBROWSER_IMAGE_INDEX_FOLDER        0
#define guDIBROWSER_IMAGE_INDEX_OTHER         7
#define guDIBROWSER_IMAGE_INDEX_LIBRARY       9
#define guDIBROWSER_IMAGE_INDEX_PODCASTS      10
#define guDIBROWSER_IMAGE_INDEX_RECORDS       11
#define guDIBROWSER_IMAGE_INDEX_AUDIO         12
#define guDIBROWSER_IMAGE_INDEX_IMAGE         13

#define guDIBROWSER_COLUMN_NAME       0
#define guDIBROWSER_COLUMN_SIZE       1
#define guDIBROWSER_COLUMN_TIME       2
#define guDIBROWSER_COLUMN_COUNT      3

class guDiBrowser;

// -------------------------------------------------------------------------------- //
class guDiGenericDirCtrl : public wxGenericDirCtrl
{
protected :
    guMainFrame           * m_MainFrame;
    guLibPanel            * m_LibPanel;
    guDiBrowser           * m_DiBrowserCtrl;
    wxTreeItemId            m_CollectionId;

public :
    guDiGenericDirCtrl() : wxGenericDirCtrl() { m_DiBrowserCtrl = nullptr; }
    guDiGenericDirCtrl(wxWindow * parent, guLibPanel * libpanel);
    ~guDiGenericDirCtrl() override;

    void                    OnConfigUpdated(wxCommandEvent &event);
    virtual void            SetupSections();

    wxTreeItemId            GetCollectionId() { return m_CollectionId; }

    //DECLARE_EVENT_TABLE()
};

// -------------------------------------------------------------------------------- //
class guDiBrowser : public wxPanel
{
protected :
    guMainFrame *           m_MainFrame;
    guLibPanel *            m_LibPanel;
    guDbLibrary *           m_DefaultDb;
    guDbLibrary *           m_Db;
    guMediaViewer *         m_MediaViewer;
    guDiGenericDirCtrl    * m_DirCtrl;
    bool                    m_AddingFolder;
    wxBitmapToggleButton  * m_ShowLibPathsBtn;

    wxString                m_CurDir;
    guFileItemArray         m_Files;

    wxImageList *           GetImageList() { return m_DirCtrl->GetTreeCtrl()->GetImageList(); }

    void                    OnContextMenu(wxTreeEvent &event);
    void                    OnConfigUpdated(wxCommandEvent &event);
    void                    CreateAcceleratorTable();

    virtual void            GetItemsList();

    int                     GetPathSortedItems(const wxString &path, guFileItemArray * items,
                                               const bool recursive = false) const;

public :
    guDiBrowser(wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, guMediaViewer * mediaviewer, const wxString &dirpath);
    ~guDiBrowser() override;

    guDiGenericDirCtrl    * GetDirCtrl() { return m_DirCtrl; }
    wxTreeItemId            GetCollectionId() { return m_DirCtrl->GetCollectionId(); }

    void                    SetMediaViewer(guMediaViewer * mediaviewer);
    guMediaViewer         * FindMediaViewerByPath(const wxString cur_path);

    wxString                GetPath();
    void                    SetPath(const wxString &path, guMediaViewer * mediaviewer);
    void                    LoadPath(const wxString &path, guMediaViewer * mediaviewer, bool recreate = false);
    wxString                DefaultPath(const wxString &path = wxEmptyString);
    void                    SelectPath(const wxString &path, bool select = true) { m_DirCtrl->SelectPath(path, select); }
    bool                    ExpandPath(const wxString &path) { return m_DirCtrl->ExpandPath(path); }
    bool                    CollapsePath(const wxString &path) { return m_DirCtrl->CollapsePath(path); }

    void                    CollectionsUpdated();

    virtual void            ReloadItems();
    size_t                  GetAllSongs(guTrackArray * tracks) const;
    wxArrayString           GetAllFiles(const bool recursive = false) const;
    size_t                  GetTracksFromFiles(const wxArrayString &files, guTrackArray * tracks) const;

    void                    OnFolderPlay(wxCommandEvent &event);
    void                    OnFolderEnqueue(wxCommandEvent &event);
    void                    OnFolderCopy( wxCommandEvent &event );
    void                    OnFolderEditTracks( wxCommandEvent &event );
    void                    OnFolderSaveToPlayList( wxCommandEvent &event );
    void                    OnFolderUpdate( wxCommandEvent &event );
    void                    OnFolderCopyTo( wxCommandEvent &event );
    void                    OnFolderCommand( wxCommandEvent &event );

    friend class guFileBrowserFileCtrl;
    friend class guFileBrowser;
};

}

#endif
