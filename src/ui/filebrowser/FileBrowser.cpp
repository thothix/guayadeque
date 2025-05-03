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
#include "FileBrowser.h"

#include "Accelerators.h"
#include "AuiDockArt.h"
#include "Config.h"
#include "FileRenamer.h"
#include "Images.h"
#include "LibUpdate.h"
#include "MainFrame.h"
#include "PlayListAppend.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Utils.h"

#include <wx/aui/aui.h>
#include <wx/arrimpl.cpp>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/gtk/tglbtn.h>

namespace Guayadeque {

WX_DEFINE_OBJARRAY( guFileItemArray )

// -------------------------------------------------------------------------------- //
guMediaViewer * FindMediaViewerByPath( guMainFrame * mainframe, const wxString curpath )
{
    const guMediaCollectionArray &Collections = mainframe->GetMediaCollections();
    int Count = Collections.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        const guMediaCollection & Collection = Collections[ Index ];
        if( mainframe->IsCollectionActive( Collection.m_UniqueId ) )
        {
            int PathIndex;
            int PathCount = Collection.m_Paths.Count();
            for( PathIndex = 0; PathIndex < PathCount; PathIndex++ )
            {
                guLogMessage(wxT("Filebrowser FindMediaViewerByPath curpath: %s == %s"), curpath.c_str(), Collection.m_Paths[PathIndex].c_str());
                if( curpath.StartsWith( Collection.m_Paths[ PathIndex ] ) )
                    return mainframe->FindCollectionMediaViewer( Collection.m_UniqueId );
            }
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
// guGenericDirCtrl
// -------------------------------------------------------------------------------- //
wxBEGIN_EVENT_TABLE( guGenericDirCtrl, wxGenericDirCtrl )
    EVT_TREE_BEGIN_LABEL_EDIT( wxID_ANY, guGenericDirCtrl::OnBeginRenameDir )
    EVT_TREE_END_LABEL_EDIT( wxID_ANY, guGenericDirCtrl::OnEndRenameDir )
wxEND_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guGenericDirCtrl::guGenericDirCtrl( wxWindow * parent, guMainFrame * mainframe, const int showpaths  ) :
              wxGenericDirCtrl( parent, wxID_ANY, wxDirDialogDefaultFolderStr,
                wxDefaultPosition, wxDefaultSize, wxDIRCTRL_SELECT_FIRST|wxDIRCTRL_DIR_ONLY|wxNO_BORDER|wxDIRCTRL_EDIT_LABELS ,
                wxEmptyString, 0, wxTreeCtrlNameStr )
{
    m_MainFrame = mainframe;
    m_ShowPaths = showpaths;
    m_FileBrowserDirCtrl = ( guFileBrowserDirCtrl * ) parent;

    wxImageList * ImageList = GetTreeCtrl()->GetImageList();
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_library ) );
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_podcast ) );
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_record ) );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    Bind( guConfigUpdatedEvent, &guGenericDirCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
guGenericDirCtrl::~guGenericDirCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Unbind( guConfigUpdatedEvent, &guGenericDirCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & ( guPREFERENCE_PAGE_FLAG_LIBRARY | guPREFERENCE_PAGE_FLAG_RECORD | guPREFERENCE_PAGE_FLAG_PODCASTS ) )
    {
        if( m_ShowPaths != guFILEBROWSER_SHOWPATH_SYSTEM )
        {
            wxString CurPath = GetPath();
            ReCreateTree();
            SetPath( CurPath );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::SetupSections()
{
    auto * Config = ( guConfig * ) guConfig::Get();

    if( m_ShowPaths & guFILEBROWSER_SHOWPATH_LOCATIONS )
    {
        const guMediaCollectionArray &  Collections = m_MainFrame->GetMediaCollections();
        size_t Count = Collections.Count();
        for( size_t Index = 0; Index < Count; Index++ )
        {
            const guMediaCollection & Collection = Collections[ Index ];
            if( m_MainFrame->IsCollectionActive( Collection.m_UniqueId ) )
            {
                size_t PathIndex;
                size_t PathCount = Collection.m_Paths.Count();
                for( PathIndex = 0; PathIndex < PathCount; PathIndex++ )
                {
                    wxString LibName = Collection.m_Paths[ PathIndex ];
                    RemovePathTrailSep(LibName);
                    AddSection(LibName, wxFileNameFromPath( LibName ), guFILEBROWSER_IMAGE_INDEX_LIBRARY );
                    guLogMessage( wxT( "guGenericDirCtrl::SetupSections - %s" ), Collection.m_Paths[ PathIndex ].c_str() );
                }
            }
        }

        wxString Path = Config->ReadStr( CONFIG_KEY_PODCASTS_PATH, wxEmptyString, CONFIG_PATH_PODCASTS );
        if( !Path.IsEmpty() )
        {
            wxString Name = Path;
            RemovePathTrailSep(Name);
            AddSection(Path, wxFileNameFromPath( Name ), guFILEBROWSER_IMAGE_INDEX_PODCASTS );
        }

        Path = Config->ReadStr( CONFIG_KEY_RECORD_PATH, wxEmptyString, CONFIG_PATH_RECORD );
        if( !Path.IsEmpty() )
        {
            wxString Name = Path;
            RemovePathTrailSep(Name);
            AddSection(Path, wxFileNameFromPath( Name ), guFILEBROWSER_IMAGE_INDEX_RECORDS );
        }
    }
    else //if( m_ShowPaths & guFILEBROWSER_SHOWPATH_SYSTEM )
        AddSection(wxT( "/" ), wxT( "/" ), guFILEBROWSER_IMAGE_INDEX_FOLDER );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnBeginRenameDir( wxTreeEvent &event )
{
    wxTreeCtrl * TreeCtrl = GetTreeCtrl();
    m_RenameItemId = TreeCtrl->GetSelection();
    m_RenameName = GetPath();
    OnBeginEditItem( event );
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::OnEndRenameDir( wxTreeEvent &event )
{
    OnEndEditItem(event);
    wxTreeCtrl * TreeCtrl = GetTreeCtrl();
    TreeCtrl->SelectItem(m_RenameItemId);

    bool is_expanded = TreeCtrl->IsExpanded(m_RenameItemId);
    TreeCtrl->Collapse(m_RenameItemId);

    m_FileBrowserDirCtrl->RenameDir(m_RenameName, GetPath());

    if (is_expanded)
        TreeCtrl->Expand(m_RenameItemId);
}

// -------------------------------------------------------------------------------- //
void guGenericDirCtrl::FolderRename( void )
{
    wxTreeCtrl * TreeCtrl = GetTreeCtrl();
    m_RenameItemId = TreeCtrl->GetSelection();
    m_RenameName = GetPath();
    TreeCtrl->EditLabel( m_RenameItemId );
}

// -------------------------------------------------------------------------------- //
// guFileBrowserDirCtrl
// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::guFileBrowserDirCtrl( wxWindow * parent, guMainFrame * mainframe, guDbLibrary * db, const wxString &dirpath ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER )
{
    m_MainFrame = mainframe;
    m_DefaultDb = db;
    m_Db = nullptr;
    m_MediaViewer = nullptr;
    m_AddingFolder = false;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    // Directory viewer
    int ShowPaths = Config->ReadNum( CONFIG_KEY_FILE_BROWSER_SHOW_LIB_PATHS, guFILEBROWSER_SHOWPATH_LOCATIONS, CONFIG_PATH_FILE_BROWSER );
    m_DirCtrl = new guGenericDirCtrl( this, m_MainFrame, ShowPaths );
	m_DirCtrl->ShowHidden( false );
	SetPath( dirpath, FindMediaViewerByPath( m_MainFrame, dirpath ) );

	MainSizer->Add( m_DirCtrl, 1, wxEXPAND, 5 );

    // Directory button
    wxBoxSizer * DirBtnSizer = new wxBoxSizer( wxHORIZONTAL );
    DirBtnSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_ShowLibPathsBtn = new wxBitmapToggleButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_library ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_ShowLibPathsBtn->SetToolTip( ShowPaths == guFILEBROWSER_SHOWPATH_SYSTEM ?
                          _( "See used locations" ) :
                          _( "See system files" ) );
	m_ShowLibPathsBtn->SetValue( ShowPaths & guFILEBROWSER_SHOWPATH_LOCATIONS );
	DirBtnSizer->Add( m_ShowLibPathsBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	MainSizer->Add( DirBtnSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    m_DirCtrl->Bind( wxEVT_TREE_ITEM_MENU, &guFileBrowserDirCtrl::OnContextMenu, this );
    m_ShowLibPathsBtn->Bind( wxEVT_TOGGLEBUTTON, &guFileBrowserDirCtrl::OnShowLibPathsClick, this );
    Bind( guConfigUpdatedEvent, &guFileBrowserDirCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
guFileBrowserDirCtrl::~guFileBrowserDirCtrl()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( CONFIG_KEY_FILE_BROWSER_SHOW_LIB_PATHS, m_ShowLibPathsBtn->GetValue(), CONFIG_PATH_FILE_BROWSER );

    m_DirCtrl->Unbind( wxEVT_TREE_ITEM_MENU, &guFileBrowserDirCtrl::OnContextMenu, this );
    m_ShowLibPathsBtn->Unbind( wxEVT_TOGGLEBUTTON, &guFileBrowserDirCtrl::OnShowLibPathsClick, this );

    Unbind( guConfigUpdatedEvent, &guFileBrowserDirCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
        CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );

    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_SAVEPLAYLIST );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_EDITTRACKS );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_PLAY );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
        SetAcceleratorTable( AccelTable );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::AppendFolderCommands( wxMenu * menu )
{
    wxMenu * SubMenu;
    wxMenuItem * MenuItem;

    SubMenu = new wxMenu();

    auto * Config = (guConfig *) guConfig::Get();

    wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
    wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
    wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);
    wxString category_names = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_NAMES, current_desktop);
    wxArrayString Names = Config->ReadAStr(CONFIG_KEY_COMMANDS_NAME, wxEmptyString, category_names);

    int Count = Commands.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            if( ( Commands[ Index ].Find( guCOMMAND_COVERPATH ) == wxNOT_FOUND ) )
            {
                MenuItem = new wxMenuItem( menu, ID_COMMANDS_BASE + Index, _( Names[ Index ] ), _( Commands[ Index ] ) );
                SubMenu->Append( MenuItem );
            }
        }

        SubMenu->AppendSeparator();
    }
    else
    {
        MenuItem = new wxMenuItem( SubMenu, ID_MENU_PREFERENCES_COMMANDS, _( "Preferences" ), _( "Add commands in preferences" ) );
        SubMenu->Append( MenuItem );
    }
    menu->AppendSubMenu( SubMenu, _( "Commands" ) );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnContextMenu( wxTreeEvent &event )
{
    wxMenu Menu;
    wxMenuItem * MenuItem;

    wxPoint Point = event.GetPoint();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                            _( "Play the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                            _( "Add the selected folder to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu.Append( MenuItem );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    Menu.Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );
    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_EDITTRACKS,
                            wxString( _( "Edit Tracks" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                            _( "Edit the tracks in the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
    Menu.Append( MenuItem );
    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST,
                            wxString( _( "Save to Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                            _( "Add the tracks in the selected folder to a playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    Menu.Append( MenuItem );
    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_COPY,
                            _( "Copy" ),
                            _( "Copy the selected folder to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    //MenuItem->Enable(false);
    Menu.Append( MenuItem );

    bool has_clipboard_data = false;
    wxTheClipboard->UsePrimarySelection(false);
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported(wxDF_FILENAME))
        {
            wxFileDataObject data;
            has_clipboard_data = wxTheClipboard->GetData(data);
        }
        wxTheClipboard->Close();
    }

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_PASTE,
                            _( "Paste" ),
                            _( "Paste to the selected folder" ) );
    MenuItem->Enable(has_clipboard_data);
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_MOVE,
                            _( "Move here" ),
                            _( "Move to the selected folder" ) );
    MenuItem->Enable(has_clipboard_data);
    Menu.Append( MenuItem );

    if( m_DirCtrl->GetShowPaths() & guFILEBROWSER_SHOWPATH_LOCATIONS )
    {
        MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_UPDATE,
                                   _( "Update" ),
                                   _( "Update the selected folder" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_reload ) );
        Menu.Append( MenuItem );
    }
    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_NEW, _( "New Folder" ), _( "Create a new folder" ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_RENAME, _( "Rename" ), _( "Rename the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_FILESYSTEM_FOLDER_DELETE, _( "Remove" ), _( "Remove the selected folder" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
    Menu.Append( MenuItem );
    Menu.AppendSeparator();

    m_MainFrame->CreateCopyToMenu( &Menu );

    AppendFolderCommands( &Menu );

    PopupMenu( &Menu, Point );
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::MoveDir( const wxString &oldname, const wxString &newname )
{
    guLogMessage(wxT("MoveDir: '%s' -> '%s' - (%i)"), oldname.c_str(), newname.c_str(), m_Db != nullptr);

    if (oldname == newname)
        return;

    if (!guRenameFile(oldname, newname))
    {
        guLogError(wxT("Could not move '%s' to '%s'"), oldname.c_str(), newname.c_str());
        return;
    }

    if (m_Db)
        m_Db->UpdatePaths(oldname, newname);
    else
        m_DefaultDb->UpdatePaths(oldname, newname);
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::RenameDir( const wxString &oldname, const wxString &newname )
{
    guLogMessage(wxT( "RenameDir: '%s' -> '%s' - (%i)"), oldname.c_str(), newname.c_str(), m_Db != nullptr);

    if( m_AddingFolder )
    {
        wxTreeCtrl * TreeCtrl = m_DirCtrl->GetTreeCtrl();
        if( newname.IsEmpty() )
            TreeCtrl->Delete( TreeCtrl->GetSelection() );

        //m_DirCtrl->ReCreateTree();
        m_AddingFolder = false;
    }
    else
    {
        if( oldname != newname )
        {
            if( m_Db )
                m_Db->UpdatePaths( oldname, newname );
            else
                m_DefaultDb->UpdatePaths( oldname, newname );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::FolderNew( void )
{
    wxTreeCtrl * TreeCtrl = m_DirCtrl->GetTreeCtrl();
    wxTreeItemId FolderParent = TreeCtrl->GetSelection();
    if( FolderParent.IsOk() )
    {
        m_AddingFolder = true;

        wxString NewDirName = GetPath() + _( "New Folder" );
        int Index = 1;
        while( wxDirExists( NewDirName ) )
        {
            NewDirName = GetPath() + _( "New Folder" );
            NewDirName += wxString::Format( wxT( "%i" ), Index++ );
        }

        if( wxMkdir( NewDirName, 0770 ) )
        {
            TreeCtrl->Collapse( FolderParent );
            //TreeCtrl->Expand( m_AddFolderParent );
            if( m_DirCtrl->ExpandPath( NewDirName ) )
            {
                wxTreeItemId Selected = TreeCtrl->GetSelection();
                if( Selected.IsOk() )
                {
                    wxTextCtrl * TextCtrl = TreeCtrl->EditLabel( Selected );
                    if( TextCtrl )
                        TextCtrl->SetSelection( -1, -1 );
                }
                else
                    guLogMessage( wxT( "No Selected item" ) );
            }
        }
        else
            guLogError( wxT( "Could not create the new directory" ) );
    }
}

// -------------------------------------------------------------------------------- //
bool RemoveDirItems( const wxString &path, wxArrayString * deletefiles )
{
    wxString FileName;
    wxString CurPath = path;

    AddPathTrailSep(CurPath);
    //guLogMessage( wxT( "Deleting folder %s" ), CurPath.c_str() );
    wxDir Dir( CurPath );
    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_HIDDEN | wxDIR_DIRS | wxDIR_DOTDOT ) )
        {
            do {
                if( FileName != wxT( "." ) && FileName != wxT( ".." ) )
                {
                    if( wxDirExists( CurPath + FileName ) )
                    {
                        if( !RemoveDirItems( CurPath + FileName, deletefiles ) )
                            return false;
                        //guLogMessage( wxT( "Removing Dir: %s" ), ( CurPath + FileName ).c_str() );
                        if( !wxRmdir( CurPath + FileName ) )
                            return false;
                    }
                    else
                    {
                        //guLogMessage( wxT( "Removing file: %s" ), ( CurPath + FileName ).c_str() );
                        if( !wxRemoveFile( CurPath + FileName ) )
                            return false;
                        deletefiles->Add( CurPath + FileName );
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::FolderDelete( void )
{
    wxTreeCtrl * TreeCtrl = m_DirCtrl->GetTreeCtrl();
    wxTreeItemId FolderId = TreeCtrl->GetSelection();
    wxDirItemData * FolderData = ( wxDirItemData *  ) TreeCtrl->GetItemData( FolderId );
    if( wxMessageBox( _( "Are you sure to delete the selected path ?" ),
                     _( "Confirm" ),
                     wxICON_QUESTION | wxYES_NO, this ) == wxYES )
    {
        wxArrayString DeleteFiles;
        if( RemoveDirItems( FolderData->m_path, &DeleteFiles ) && wxRmdir( FolderData->m_path ) )
        {
            TreeCtrl->Delete( FolderId );
        }
        else
        {
            wxMessageBox(wxString::Format("%s ", _("Error deleting the folder")) + FolderData->m_path,
                _( "Error" ), wxICON_ERROR | wxOK, this );
        }
        //m_Db->DoCleanUp();
        if( m_Db )
            m_Db->CleanFiles( DeleteFiles );
        else
            m_DefaultDb->CleanFiles( DeleteFiles );
    }
}


// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::OnShowLibPathsClick( wxCommandEvent& event )
{
    int ShowPaths = m_ShowLibPathsBtn->GetValue();

    wxString CurPath = GetPath();
    m_DirCtrl->SetShowPaths( ShowPaths );
    m_DirCtrl->ReCreateTree();
    m_DirCtrl->SetPath( CurPath );

    m_ShowLibPathsBtn->SetToolTip( ShowPaths == guFILEBROWSER_SHOWPATH_SYSTEM ?
                          _( "See used locations" ) :
                          _( "See system files" ) );
}


// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::CollectionsUpdated()
{
    if( m_DirCtrl->GetShowPaths() & guFILEBROWSER_SHOWPATH_LOCATIONS )
    {
        wxString CurPath = GetPath();
        m_DirCtrl->ReCreateTree();
        m_DirCtrl->SetPath( CurPath );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::SetPath( const wxString &path, guMediaViewer * mediaviewer )
{
    //guLogMessage( wxT( "guFileBrowserDirCtrl::SetPath( %s )" ), path.c_str() );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
    m_DirCtrl->SetPath( path );
}

// -------------------------------------------------------------------------------- //
void guFileBrowserDirCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
}

// -------------------------------------------------------------------------------- //
// guFilesListBox
// -------------------------------------------------------------------------------- //
bool guAddDirItems( const wxString &path, wxArrayString &files )
{
    wxString FileName;
    wxString CurPath = path;

    AddPathTrailSep(CurPath);
    guLogMessage( wxT( "Searching in folder %s" ), CurPath.c_str() );
    wxDir Dir( CurPath );
    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_HIDDEN | wxDIR_DIRS | wxDIR_DOTDOT ) )
        {
            do {
                if( FileName != wxT( "." ) && FileName != wxT( ".." ) )
                {
                    if( wxDirExists( CurPath + FileName ) )
                    {
                        if( !guAddDirItems( CurPath + FileName, files ) )
                            return false;
                    }
                    else
                        files.Add( CurPath + FileName );
                }
            } while( Dir.GetNext( &FileName ) );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guFilesListBox::guFilesListBox( wxWindow * parent, guDbLibrary * db ) :
    guListView( parent, wxLB_MULTIPLE | guLISTVIEW_COLUMN_SELECT | guLISTVIEW_COLUMN_SORTING | guLISTVIEW_ALLOWDRAG )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_Order = Config->ReadNum( CONFIG_KEY_FILE_BROWSER_ORDER, 0, CONFIG_PATH_FILE_BROWSER );
    m_OrderDesc = Config->ReadNum( CONFIG_KEY_FILE_BROWSER_ORDERDESC, false, CONFIG_PATH_FILE_BROWSER );

    int ColId;
    wxString ColName;
    wxArrayString ColumnNames = GetColumnNames();
    int count = ColumnNames.Count();
    for( int index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "Id%u" ), index ), index, CONFIG_PATH_FILE_BROWSER_COLUMNS_IDS );

        ColName = ColumnNames[ ColId ];

        ColName += ( ( ColId == m_Order ) ? ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "Width%u" ), index ), 80, CONFIG_PATH_FILE_BROWSER_COLUMNS_WIDTHS ),
            Config->ReadBool( wxString::Format( wxT( "Show%u" ), index ), true, CONFIG_PATH_FILE_BROWSER_COLUMNS_SHOWS )
            );
        InsertColumn( Column );
    }

    Bind( guConfigUpdatedEvent, &guFilesListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guFilesListBox::~guFilesListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    wxArrayString ColumnNames = GetColumnNames();
    int count = ColumnNames.Count();
    for( int index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "Id%u" ), index ),
                          ( * m_Columns )[ index ].m_Id, CONFIG_PATH_FILE_BROWSER_COLUMNS_IDS );
        Config->WriteNum( wxString::Format( wxT( "Width%u" ), index ),
                          ( * m_Columns )[ index ].m_Width, CONFIG_PATH_FILE_BROWSER_COLUMNS_WIDTHS );
        Config->WriteBool( wxString::Format( wxT( "Show%u" ), index ),
                           ( * m_Columns )[ index ].m_Enabled, CONFIG_PATH_FILE_BROWSER_COLUMNS_SHOWS );
    }

    Config->WriteNum( CONFIG_KEY_FILE_BROWSER_ORDER, m_Order, CONFIG_PATH_FILE_BROWSER );
    Config->WriteBool( CONFIG_KEY_FILE_BROWSER_ORDERDESC, m_OrderDesc, CONFIG_PATH_FILE_BROWSER );

    Unbind( guConfigUpdatedEvent, &guFilesListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
        CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SAVE );
    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );

    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_SAVEPLAYLIST );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_EDITTRACKS );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_PLAY );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
        SetAcceleratorTable( AccelTable );
}

// -------------------------------------------------------------------------------- //
wxArrayString guFilesListBox::GetColumnNames( void )
{
    wxArrayString ColumnNames;
    ColumnNames.Add( _( "Name" ) );
    ColumnNames.Add( _( "Size" ) );
    ColumnNames.Add( _( "Modified" ) );
    return ColumnNames;
}

// -------------------------------------------------------------------------------- //
wxString guFilesListBox::OnGetItemText( const int row, const int col ) const
{
//    guLogMessage( wxT( "GetItem: %i  %i" ), row, col );
//    if( row < 0 || col < 0 )
//        return wxEmptyString;

    guFileItem * FileItem;
    FileItem = &m_Files[ row ];
    switch( ( * m_Columns )[ col ].m_Id )
    {
//        case guFILEBROWSER_COLUMN_TYPE :
//            return wxEmptyString;

        case guFILEBROWSER_COLUMN_NAME :
          return FileItem->m_Name;

        case guFILEBROWSER_COLUMN_SIZE :
          return SizeToString( FileItem->m_Size );

        case guFILEBROWSER_COLUMN_TIME :
        {
          wxDateTime FileTime;
          FileTime.Set( ( time_t ) FileItem->m_Time );
          return FileTime.FormatDate();
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( ( * m_Columns )[ col ].m_Id == guFILEBROWSER_COLUMN_NAME )
    {
        guFileItem * FileItem = &m_Files[ row ];
        dc.SetBackgroundMode( wxTRANSPARENT );
        int ImageIndex = guFILEBROWSER_IMAGE_INDEX_OTHER;
        if( FileItem->m_Type == guFILEITEM_TYPE_FOLDER )
            ImageIndex = guFILEBROWSER_IMAGE_INDEX_FOLDER;
        else if( FileItem->m_Type == guFILEITEM_TYPE_AUDIO )
            ImageIndex = guFILEBROWSER_IMAGE_INDEX_AUDIO;
        else if( FileItem->m_Type == guFILEITEM_TYPE_IMAGE )
            ImageIndex = guFILEBROWSER_IMAGE_INDEX_IMAGE;

        m_TreeImageList->Draw( ImageIndex, dc, rect.x + 1, rect.y + 1, wxIMAGELIST_DRAW_TRANSPARENT );
        wxRect TextRect = rect;
        TextRect.x += 20; // 16 + 4
        TextRect.width -= 20;
        guListView::DrawItem( dc, TextRect, row, col );
    }
    else
        guListView::DrawItem( dc, rect, row, col );
}

// -------------------------------------------------------------------------------- //
void inline GetFileDetails( const wxString &filename, guFileItem * fileitem )
{
    wxStructStat St;
    wxStat( filename, &St );
    fileitem->m_Type = ( ( St.st_mode & S_IFMT ) == S_IFDIR ) ? 0 : 3;
    fileitem->m_Size = St.st_size;
    fileitem->m_Time = St.st_ctime;
}

// -------------------------------------------------------------------------------- //
size_t guFilesListBox::GetPathSortedItems(const wxString &path, guFileItemArray *items,
                                          const int order, const bool orderdesc, const bool recursive ) const
{
    wxString v_path = GetPathAddTrailSep(path);

    if (path.IsEmpty() || !wxDirExists(v_path))
        return items->Count();

    wxDir Dir(v_path );
    if (!Dir.IsOpened())
        return items->Count();

    wxString FileName;
    if (Dir.GetFirst(&FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS | wxDIR_DOTDOT))
    {
        do {
            if (FileName == wxT("."))
                continue;

            if (recursive && wxDirExists(v_path + FileName))
            {
                if (FileName != wxT(".."))
                    GetPathSortedItems(v_path + FileName, items, order, orderdesc, recursive);
            }
            else
            {
                auto * FileItem = new guFileItem();
                if (recursive)
                    FileItem->m_Name = v_path;
                FileItem->m_Name += FileName;
                GetFileDetails(v_path + FileName, FileItem );
                if( !wxDirExists(v_path + FileName ) )
                {
                    if( guIsValidAudioFile( FileName.Lower() ) )
                        FileItem->m_Type = guFILEITEM_TYPE_AUDIO;
                    else if( guIsValidImageFile( FileName.Lower() ) )
                        FileItem->m_Type = guFILEITEM_TYPE_IMAGE;
                }
                items->Add( FileItem );
            }
        } while( Dir.GetNext( &FileName ) );
    }

    switch (order)
    {
        case guFILEBROWSER_COLUMN_NAME :
            items->Sort( orderdesc ? CompareFileNameD : CompareFileNameA );
            break;
        case guFILEBROWSER_COLUMN_SIZE :
            items->Sort( orderdesc ? CompareFileSizeD : CompareFileSizeA );
            break;
        case guFILEBROWSER_COLUMN_TIME :
            items->Sort( orderdesc ? CompareFileTimeD : CompareFileTimeA );
            break;
        default :
            items->Sort( orderdesc ? CompareFileNameD : CompareFileNameA );
    }
    return items->Count();
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::GetItemsList()
{
    GetPathSortedItems(m_CurDir, &m_Files, m_Order, m_OrderDesc);
    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::ReloadItems( bool reset )
{
    wxArrayInt Selection;
    int FirstVisible = 0;

    if( reset )
        SetSelection( -1 );
    else
    {
        Selection = GetSelectedItems( false );
        FirstVisible = GetVisibleRowsBegin();
    }

    m_Files.Empty();
    GetItemsList();
    SetItemCount( m_Files.Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToRow( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void AppendItemsCommands( wxMenu * menu, int selcount, int seltype )
{
    wxMenuItem * MenuItem;
    wxMenu * SubMenu = new wxMenu();

    guLogMessage( wxT( "AppendItemCommands: %i  %i" ), selcount, seltype );

    guConfig * Config = (guConfig *) guConfig::Get();

    wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
    wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
    wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);
    wxString category_names = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_NAMES, current_desktop);
    wxArrayString Names = Config->ReadAStr(CONFIG_KEY_COMMANDS_NAME, wxEmptyString, category_names);

    int Count = Commands.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            if( ( Commands[ Index ].Find( guCOMMAND_COVERPATH ) == wxNOT_FOUND ) ||
                ( ( selcount == 1 ) && ( seltype == guFILEITEM_TYPE_IMAGE ) ) )
            {
                MenuItem = new wxMenuItem( menu, ID_COMMANDS_BASE + Index, _( Names[ Index ] ), _( Commands[ Index ] ) );
                SubMenu->Append( MenuItem );
            }
        }

        SubMenu->AppendSeparator();
    }
    else
    {
        MenuItem = new wxMenuItem( SubMenu, ID_MENU_PREFERENCES_COMMANDS, _( "Preferences" ), _( "Add commands in preferences" ) );
        SubMenu->Append( MenuItem );
    }
    menu->AppendSubMenu( SubMenu, _( "Commands" ) );
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    wxArrayInt Selection = GetSelectedItems( false );
    int SelCount;
    if( ( SelCount = Selection.Count() ) )
    {
        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_PLAY,
                            wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                            _( "Play current selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL,
                            wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                            _( "Add current selected files to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );
        MenuItem->Enable( SelCount );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );
    }

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_EDITTRACKS,
                            wxString( _( "Edit Tracks" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                            _( "Edit the current selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_SAVEPLAYLIST,
                            wxString( _( "Save to Playlist" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                            _( "Add the current selected tracks to a playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );
    }

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_COPY, _( "Copy" ), _( "Copy the selected folder to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_PASTE, _( "Paste" ), _( "Paste to the selected dir" ) );
    Menu->Append( MenuItem );
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_FILENAME ) )
        {
            wxFileDataObject data;
            MenuItem->Enable( wxTheClipboard->GetData( data ) );
        }
        wxTheClipboard->Close();
    }

    if( SelCount )
    {
        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_RENAME, _( "Rename Files" ), _( "Rename the current selected file" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_FILESYSTEM_ITEMS_DELETE, _( "Remove" ), _( "Delete the selected files" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
        MainFrame->CreateCopyToMenu( Menu );

        AppendItemsCommands( Menu, SelCount, SelCount ? GetType( Selection[ 0 ] ) : guFILEITEM_TYPE_FILE );
    }
}

// -------------------------------------------------------------------------------- //
int inline guFilesListBox::GetItemId( const int row ) const
{
    return row;
}

// -------------------------------------------------------------------------------- //
wxString inline guFilesListBox::GetItemName( const int row ) const
{
    return m_Files[ row ].m_Name;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::SetOrder( int columnid )
{
    if( m_Order != columnid )
    {
        m_Order = columnid;
        m_OrderDesc = ( columnid != 0 );
    }
    else
        m_OrderDesc = !m_OrderDesc;

    wxArrayString ColumnNames = GetColumnNames();
    int CurColId;
    int count = ColumnNames.Count();
    for( int index = 0; index < count; index++ )
    {
        CurColId = GetColumnId( index );
        SetColumnLabel( index,
            ColumnNames[ CurColId ]  + ( ( CurColId == m_Order ) ?
                ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetSelectedSongs( guTrackArray * tracks, const bool isdrag ) const
{
    wxArrayString Files = GetSelectedFiles( true );
    return GetTracksFromFiles( Files, tracks );
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetAllSongs( guTrackArray * tracks ) const
{
    wxArrayString Files = GetAllFiles( true );
    return GetTracksFromFiles( Files, tracks );
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetTracksFromFiles( const wxArrayString &files, guTrackArray * tracks ) const
{
    int Count = files.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            wxString FileName = files[ Index ];
            //guLogMessage( wxT( "GetTracksFromFiles: %s" ), FileName.c_str() );
            wxURI Uri( FileName );

            if( Uri.IsReference() )
            {
                if( guIsValidAudioFile( FileName ) )
                {
                    guTrack * Track = new guTrack();
                    Track->m_FileName = FileName;

                    if( !m_Db || !m_Db->FindTrackFile( FileName, Track ) )
                    {
                        guPodcastItem PodcastItem;
                        guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
                        guDbPodcasts * DbPodcasts = MainFrame->GetPodcastsDb();
                        if( DbPodcasts->GetPodcastItemFile( FileName, &PodcastItem ) )
                        {
                            delete Track;
                            continue;
                        }
                        else
                        {
                            if( Track->ReadFromFile( FileName ) )
                                Track->m_Type = guTRACK_TYPE_NOTDB;
                            else
                                guLogError( wxT( "Could not read tags from file '%s'" ), FileName.c_str() );
                        }
                    }
                    tracks->Add( Track );
                }
            }
        }
    }
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
wxArrayString guFilesListBox::GetSelectedFiles( const bool recursive ) const
{
    wxArrayString Files;
    wxArrayInt Selection = GetSelectedItems( false );
    int Count = Selection.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
        {
            if( m_Files[ Selection[ Index ] ].m_Name != wxT( ".." ) )
            {
                if( recursive && ( m_Files[ Selection[ Index ] ].m_Type == guFILEITEM_TYPE_FOLDER ) )
                {
                    //guAddDirItems( m_CurDir + m_Files[ Selection[ Index ] ].m_Name, Files );
                    guFileItemArray DirFiles;
                    if(GetPathSortedItems(m_CurDir + m_Files[Selection[Index]].m_Name,
                                          &DirFiles, m_Order, m_OrderDesc, true) )
                    {
                        int FileIndex;
                        int FileCount = DirFiles.Count();
                        for( FileIndex = 0; FileIndex < FileCount; FileIndex++ )
                            Files.Add( DirFiles[ FileIndex ].m_Name );
                    }
                }
                else
                    Files.Add( m_CurDir + m_Files[ Selection[ Index ] ].m_Name );
            }
        }
    }
    return Files;
}

// -------------------------------------------------------------------------------- //
wxArrayString guFilesListBox::GetAllFiles( const bool recursive ) const
{
    wxArrayString Files;
    size_t Count = m_Files.Count();

    if (!Count)
        return Files;

    for (size_t Index = 0; Index < Count; Index++)
    {
        if (m_Files[Index].m_Name == wxT(".."))
            continue;

        if( recursive && ( m_Files[ Index ].m_Type == guFILEITEM_TYPE_FOLDER ) )
        {
            //guAddDirItems( m_CurDir + m_Files[ Selection[ Index ] ].m_Name, Files );
            guFileItemArray DirFiles;
            if(GetPathSortedItems(m_CurDir + m_Files[Index].m_Name,
                                  &DirFiles, m_Order, m_OrderDesc, true) )
            {
                size_t FileIndex;
                size_t FileCount = DirFiles.Count();
                for( FileIndex = 0; FileIndex < FileCount; FileIndex++ )
                    Files.Add( DirFiles[ FileIndex ].m_Name );
            }
        }
        else
            Files.Add( m_CurDir + m_Files[ Index ].m_Name );
    }
    return Files;
}

// -------------------------------------------------------------------------------- //
size_t guFilesListBox::GetDragFiles( guDataObjectComposite * files )
{
    wxArrayString SelectFiles = GetSelectedFiles( true );
    size_t Count = SelectFiles.Count();

    for (size_t Index = 0; Index < Count; Index++)
       SelectFiles[ Index ] = guFileDnDEncode( SelectFiles[ Index ] );

    files->SetFiles( SelectFiles );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guFilesListBox::SetPath( const wxString &path, guMediaViewer * mediaviewer )
{
    //guLogMessage( wxT( "guFilesListBox::SetPath( %s )" ), path.c_str() );
    m_CurDir = GetPathAddTrailSep(path);
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : nullptr;
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
wxString guFilesListBox::GetPath( const int item, const bool absolute ) const
{
    wxString RetVal;
    //guLogMessage( wxT( "GetPath( %i )" ), item );
    if( item >= 0 )
    {
        if( !absolute )
            return m_CurDir + m_Files[ item ].m_Name;
        else
        {
            if( m_Files[ item ].m_Name == wxT( ".." ) )
            {
                RetVal = m_CurDir.BeforeLast( wxT( '/' ) ).BeforeLast( wxT( '/' ) );
                //guLogMessage( wxT( "1) Path : %s" ), RetVal.c_str() );
                return RetVal;
            }

            wxFileName FileName( m_Files[ item ].m_Name );
            FileName.MakeAbsolute( m_CurDir );
            //guLogMessage( wxT( "Path : %s" ), FileName.GetFullPath().c_str() );
            return FileName.GetFullPath();
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
int guFilesListBox::GetType( const int item ) const
{
    if( item >= 0 )
        return m_Files[ item ].m_Type;
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
bool guFilesListBox::GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
    * count = * len = * size = 0;
    int Count = m_Files.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( m_Files[ Index ].m_Type == guFILEITEM_TYPE_FOLDER )
        {
            if( ( m_Files[ Index ].m_Name != wxT( ".." ) ) )
                ( * len )++;
        }
        else
        {
            * size += m_Files[ Index ].m_Size;
            ( * count )++;
        }
    }
    return true;
}


// -------------------------------------------------------------------------------- //
// guFileBrowserFileCtrl
// -------------------------------------------------------------------------------- //
guFileBrowserFileCtrl::guFileBrowserFileCtrl( wxWindow * parent, guDbLibrary * db, guFileBrowserDirCtrl * dirctrl ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxNO_BORDER )
{
    m_DefaultDb = db;
    m_Db = NULL;
    m_MediaViewer = NULL;
    m_DirCtrl = dirctrl;
    wxImageList * ImageList = dirctrl->GetImageList();
    ImageList->Add( wxArtProvider::GetBitmap( wxT( "audio-x-generic" ), wxART_OTHER, wxSize( 16, 16 ) ) );
    ImageList->Add( wxArtProvider::GetBitmap( wxT( "image-x-generic" ), wxART_OTHER, wxSize( 16, 16 ) ) );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_FilesListBox = new guFilesListBox( this, db );
	m_FilesListBox->SetTreeImageList( ImageList );
	MainSizer->Add( m_FilesListBox, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();
}

// -------------------------------------------------------------------------------- //
guFileBrowserFileCtrl::~guFileBrowserFileCtrl()
{
}

// -------------------------------------------------------------------------------- //
void guFileBrowserFileCtrl::SetPath( const wxString &path, guMediaViewer * mediaviewer )
{
    guLogMessage( wxT( "guFileBrowserFileCtrl::SetPath( %s )" ), path.c_str() );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
    m_FilesListBox->SetPath( path, mediaviewer );
}


// -------------------------------------------------------------------------------- //
// guFileBrowser
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE( guFileBrowser, wxPanel )
    EVT_TREE_BEGIN_DRAG( wxID_ANY, guFileBrowser::OnDirBeginDrag)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guFileBrowser::guFileBrowser( wxWindow * parent, guMainFrame * mainframe, guDbLibrary * db, guPlayerPanel * playerpanel ) :
    guAuiManagerPanel( parent )
{
    m_MainFrame = mainframe;
    m_DefaultDb = db;
    m_Db = NULL;
    m_MediaViewer = NULL;
    m_PlayerPanel = playerpanel;

    guConfig *  Config = ( guConfig * ) guConfig::Get();
    m_VisiblePanels = Config->ReadNum( CONFIG_KEY_FILE_BROWSER_VISIBLE_PANELS, guPANEL_FILEBROWSER_VISIBLE_DEFAULT, CONFIG_PATH_FILE_BROWSER );
    wxString LastPath = Config->ReadStr( CONFIG_KEY_FILE_BROWSER_PATH, wxEmptyString, CONFIG_PATH_FILE_BROWSER );

    guLogMessage( wxT( "guFileBrowser LastPath: '%s'" ), LastPath.c_str() );
    m_DirCtrl = new guFileBrowserDirCtrl( this, m_MainFrame, db, LastPath );

    m_AuiManager.AddPane( m_DirCtrl,
            wxAuiPaneInfo().Name( wxT( "FileBrowserDirCtrl" ) ).Caption( _( "Directories" ) ).
            MinSize( 60, 28 ).Row( 0 ).Layer( 0 ).Position( 0 ).
            CloseButton( false ).
            Dockable( true ).Left() );

    m_FilesCtrl = new guFileBrowserFileCtrl( this, db, m_DirCtrl );
    m_FilesCtrl->SetPath( LastPath, NULL );

    m_AuiManager.AddPane( m_FilesCtrl,
            wxAuiPaneInfo().Name( wxT( "FileBrowserFilesCtrl" ) ).
            Dockable( true ).CenterPane() );

    wxString FileBrowserLayout = Config->ReadStr( CONFIG_KEY_FILE_BROWSER_LAST_LAYOUT, wxEmptyString, CONFIG_PATH_FILE_BROWSER );
    if( Config->GetIgnoreLayouts() || FileBrowserLayout.IsEmpty() )
    {
        m_VisiblePanels = guPANEL_FILEBROWSER_VISIBLE_DEFAULT;
        FileBrowserLayout = wxT( "layout2|name=FileBrowserDirCtrl;caption=" ) + wxString( _( "Directories" ) );
        FileBrowserLayout += wxT( ";state=2044;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=60;besth=28;minw=60;minh=28;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        FileBrowserLayout += wxT( "name=FileBrowserFilesCtrl;caption=;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=20;besth=20;minw=-1;minh=-1;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        FileBrowserLayout += wxT( "dock_size(5,0,0)=10|dock_size(4,0,0)=266|" );
        //m_AuiManager.Update();
    }

    m_AuiManager.LoadPerspective( FileBrowserLayout, true );

    m_DirCtrl->Bind( wxEVT_TREE_SEL_CHANGED, &guFileBrowser::OnDirItemChanged, this );
    m_FilesCtrl->Bind( wxEVT_LISTBOX_DCLICK, &guFileBrowser::OnFileItemActivated, this );
    m_FilesCtrl->Bind( wxEVT_LIST_COL_CLICK, &guFileBrowser::OnFilesColClick, this );

    Bind( wxEVT_MENU, &guFileBrowser::OnFolderPlay, this, ID_FILESYSTEM_FOLDER_PLAY );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderEnqueue, this, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderNew, this, ID_FILESYSTEM_FOLDER_NEW );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderRename, this, ID_FILESYSTEM_FOLDER_RENAME );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderDelete, this, ID_FILESYSTEM_FOLDER_DELETE );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderCopy, this, ID_FILESYSTEM_FOLDER_COPY );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderPaste, this, ID_FILESYSTEM_FOLDER_PASTE );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderMove, this, ID_FILESYSTEM_FOLDER_MOVE );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderEditTracks, this, ID_FILESYSTEM_FOLDER_EDITTRACKS );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderSaveToPlayList, this, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST );
    Bind( wxEVT_MENU, &guFileBrowser::OnFolderUpdate, this, ID_FILESYSTEM_FOLDER_UPDATE );

    m_DirCtrl->Bind( wxEVT_MENU, &guFileBrowser::OnFolderCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    m_DirCtrl->Bind( wxEVT_MENU, &guFileBrowser::OnFolderCommand, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
    m_FilesCtrl->Bind( wxEVT_MENU, &guFileBrowser::OnItemsCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    Bind( wxEVT_MENU, &guFileBrowser::OnItemsPlay, this, ID_FILESYSTEM_ITEMS_PLAY );
    Bind( wxEVT_MENU, &guFileBrowser::OnItemsEnqueue, this, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guFileBrowser::OnItemsEditTracks, this, ID_FILESYSTEM_ITEMS_EDITTRACKS );
    Bind( wxEVT_MENU, &guFileBrowser::OnItemsSaveToPlayList, this, ID_FILESYSTEM_ITEMS_SAVEPLAYLIST );
    Bind( wxEVT_MENU, &guFileBrowser::OnItemsRename, this, ID_FILESYSTEM_ITEMS_RENAME );
    Bind( wxEVT_MENU, &guFileBrowser::OnItemsDelete, this, ID_FILESYSTEM_ITEMS_DELETE );
    Bind( wxEVT_MENU, &guFileBrowser::OnItemsCopy, this, ID_FILESYSTEM_ITEMS_COPY );
    Bind( wxEVT_MENU, &guFileBrowser::OnItemsPaste, this, ID_FILESYSTEM_ITEMS_PASTE );

    m_FilesCtrl->Bind( wxEVT_MENU, &guFileBrowser::OnItemsCommand, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
}

// -------------------------------------------------------------------------------- //
guFileBrowser::~guFileBrowser()
{
    guConfig *  Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( CONFIG_KEY_FILE_BROWSER_VISIBLE_PANELS, m_VisiblePanels, CONFIG_PATH_FILE_BROWSER );
    Config->WriteStr( CONFIG_KEY_FILE_BROWSER_LAST_LAYOUT, m_AuiManager.SavePerspective(), CONFIG_PATH_FILE_BROWSER );
    Config->WriteStr( CONFIG_KEY_FILE_BROWSER_PATH, m_DirCtrl->GetPath(), CONFIG_PATH_FILE_BROWSER );

    m_DirCtrl->Unbind( wxEVT_TREE_SEL_CHANGED, &guFileBrowser::OnDirItemChanged, this );
    m_FilesCtrl->Unbind( wxEVT_LISTBOX_DCLICK, &guFileBrowser::OnFileItemActivated, this );
    m_FilesCtrl->Unbind( wxEVT_LIST_COL_CLICK, &guFileBrowser::OnFilesColClick, this );

    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderPlay, this, ID_FILESYSTEM_FOLDER_PLAY );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderEnqueue, this, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderNew, this, ID_FILESYSTEM_FOLDER_NEW );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderRename, this, ID_FILESYSTEM_FOLDER_RENAME );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderDelete, this, ID_FILESYSTEM_FOLDER_DELETE );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderCopy, this, ID_FILESYSTEM_FOLDER_COPY );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderPaste, this, ID_FILESYSTEM_FOLDER_PASTE );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderMove, this, ID_FILESYSTEM_FOLDER_MOVE );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderEditTracks, this, ID_FILESYSTEM_FOLDER_EDITTRACKS );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderSaveToPlayList, this, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST );
    Unbind( wxEVT_MENU, &guFileBrowser::OnFolderUpdate, this, ID_FILESYSTEM_FOLDER_UPDATE );
    m_DirCtrl->Unbind( wxEVT_MENU, &guFileBrowser::OnFolderCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    m_DirCtrl->Unbind( wxEVT_MENU, &guFileBrowser::OnFolderCommand, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsPlay, this, ID_FILESYSTEM_ITEMS_PLAY );
    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsEnqueue, this, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsEditTracks, this, ID_FILESYSTEM_ITEMS_EDITTRACKS );
    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsSaveToPlayList, this, ID_FILESYSTEM_ITEMS_SAVEPLAYLIST );
    m_FilesCtrl->Unbind( wxEVT_MENU, &guFileBrowser::OnItemsCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsRename, this, ID_FILESYSTEM_ITEMS_RENAME );
    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsDelete, this, ID_FILESYSTEM_ITEMS_DELETE );
    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsCopy, this, ID_FILESYSTEM_ITEMS_COPY );
    Unbind( wxEVT_MENU, &guFileBrowser::OnItemsPaste, this, ID_FILESYSTEM_ITEMS_PASTE );

    m_FilesCtrl->Unbind( wxEVT_MENU, &guFileBrowser::OnItemsCommand, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnDirItemChanged( wxTreeEvent &event )
{
    wxString CurPath = m_DirCtrl->GetPath();
    AddPathTrailSep(CurPath);

    guLogMessage( wxT( "guFileBrowser::OnDirItemChanged( '%s' )" ), CurPath.c_str() );

    if( m_DirCtrl->GetShowPaths() & guFILEBROWSER_SHOWPATH_LOCATIONS )
    {
        m_MediaViewer = FindMediaViewerByPath( m_MainFrame, CurPath );
        m_Db = m_MediaViewer ? m_MediaViewer->GetDb() : nullptr;
        if( m_MediaViewer )
            guLogTrace( wxT( "guFileBrowser::OnDirItemChanged MEDIAVIEWER - '%s' ==>> '%i' '%s'" ), CurPath.c_str(), m_MediaViewer != NULL, m_MediaViewer->GetName().c_str() );
        else
            guLogTrace( wxT( "guFileBrowser::OnDirItemChanged '%s' ==>> '%i' ''" ), CurPath.c_str(), m_MediaViewer != NULL );
    }
    else
        m_Db = nullptr;

    m_FilesCtrl->SetPath( CurPath, m_MediaViewer );
    m_DirCtrl->SetMediaViewer( m_MediaViewer );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFileItemActivated( wxCommandEvent &Event )
{
    wxArrayInt Selection = m_FilesCtrl->GetSelectedItems( false );
    if( Selection.Count() )
    {
        if( m_FilesCtrl->GetType( Selection[ 0 ] ) == guFILEITEM_TYPE_FOLDER )
            m_DirCtrl->SetPath( m_FilesCtrl->GetPath( Selection[ 0 ] ), m_MediaViewer );
        else
        {
            wxArrayString Files;
            Files.Add( m_FilesCtrl->GetPath( Selection[ 0 ] ) );

            guConfig * Config = ( guConfig * ) guConfig::Get();
            if( Config )
            {
                if( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false , CONFIG_PATH_GENERAL) )
                    m_PlayerPanel->AddToPlayList( Files );
                else
                    m_PlayerPanel->SetPlayList( Files );
            }

        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFilesColClick( wxListEvent &event )
{
    int col = event.GetColumn();
    if( col < 0 )
        return;
    m_FilesCtrl->SetOrder( col );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnDirBeginDrag( wxTreeEvent &event )
{
    wxFileDataObject Files;

    wxArrayString FolderFiles = m_FilesCtrl->GetAllFiles( true );
    int Count = FolderFiles.Count();
    if( Count )
    {
        for( int Index = 0; Index < Count; Index++ )
            Files.AddFile( FolderFiles[ Index ] );

        wxDropSource source( Files, this );

        wxDragResult Result = source.DoDragDrop();
        if( Result ) {}
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderPlay( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetAllFiles( true );
    m_PlayerPanel->SetPlayList( Files );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderEnqueue( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetAllFiles( true );
    if( Files.Count() )
        m_PlayerPanel->AddToPlayList( Files, true, event.GetId() - ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderNew( wxCommandEvent &event )
{
    m_DirCtrl->FolderNew();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderRename( wxCommandEvent &event )
{
    m_DirCtrl->FolderRename();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderDelete( wxCommandEvent &event )
{
    m_DirCtrl->FolderDelete();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderCopy( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnFolderCopy" ) );
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->Clear();
        wxFileDataObject * FileObject = new wxFileDataObject();
        //wxCustomDataObject * GnomeCopiedObject = new wxCustomDataObject( wxDataFormat( wxT( "x-special/gnome-copied-files" ) ) );
        //wxCustomDataObject * UriListObject = new wxCustomDataObject( wxDataFormat( wxT( "text/uri-list" ) ) );
        wxTextDataObject * TextObject = new wxTextDataObject();
        wxDataObjectComposite * CompositeObject = new wxDataObjectComposite();

        wxString Path = m_DirCtrl->GetPath();
        Path.Replace( wxT( "#" ), wxT( "%23" ) );

        TextObject->SetText( Path );
        FileObject->AddFile( Path );

        //Path = wxT( "file://" ) + Path;
        //UriListObject->SetData( Path.Length(), Path.char_str() );

        //Path = wxT( "copy\n" ) + Path;
        //GnomeCopiedObject->SetData( Path.Length(), Path.char_str() );

        //guLogMessage( wxT( "Copy: '%s'" ), Path.c_str() );

        CompositeObject->Add( FileObject );
        //CompositeObject->Add( GnomeCopiedObject );
        //CompositeObject->Add( UriListObject );
        CompositeObject->Add( TextObject );

        //if( !wxTheClipboard->AddData( CustomObject ) )
        if( !wxTheClipboard->AddData( CompositeObject ) )
        //if( !wxTheClipboard->AddData( TextObject ) )
        {
            delete FileObject;
            delete TextObject;
            //delete GnomeCopiedObject;
            //delete UriListObject;
            delete CompositeObject;
            guLogError( wxT( "Can't copy the folder to the clipboard" ) );
        }
        guLogMessage( wxT( "Copied the data to the clipboard..." ) );
        wxTheClipboard->Close();
    }
    else
        guLogError( wxT( "Could not open the clipboard object" ) );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderMove( wxCommandEvent &event )
{
    guLogMessage(wxT( "OnFolderMove"));
    wxTheClipboard->UsePrimarySelection(false);

    if (!wxTheClipboard->Open())
    {
        guLogError(wxT("Cannot open the clipboard object"));
        return;
    }

    if (wxTheClipboard->IsSupported(wxDF_FILENAME))
    {
        wxFileDataObject FileObject;
        if (wxTheClipboard->GetData(FileObject))
        {
            const wxArrayString sourceFiles = FileObject.GetFilenames();
            const wxString curPath = GetPathRemoveTrailSep(m_DirCtrl->GetPath());
            wxFileName dirname;
            wxString sourceDir;

            int count = sourceFiles.Count();
            for (int index = 0; index < count; index++)
            {
                sourceDir = GetPathRemoveTrailSep(sourceFiles[index]);

                if (wxDirExists(sourceDir))
                {
                    if (curPath != sourceDir)
                    {
                        dirname = wxFileName::DirName(sourceDir);
                        wxString lastDir = dirname.GetDirs()[dirname.GetDirCount() - 1];
                        wxString targetDir = curPath + wxT("/") + lastDir;

                        guLogMessage(wxT("Moving: %s -> %s"), sourceDir.c_str(), targetDir.c_str());
                        m_DirCtrl->MoveDir(sourceDir, targetDir);
                    }
                } else
                    guLogError(wxT("Cannot move '%s' into '%s'"),  sourceDir.c_str(), curPath.c_str());
            }
            m_DirCtrl->CollapsePath(curPath);
            if (!sourceDir.IsEmpty())
            {
                sourceDir = wxPathOnly(sourceDir);
                m_DirCtrl->CollapsePath(sourceDir);
                m_DirCtrl->ExpandPath(sourceDir);
            }
            m_DirCtrl->ExpandPath(curPath);
        }
        else
            guLogError(wxT("Cannot move the data from the clipboard"));
    }
    wxTheClipboard->Close();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderPaste( wxCommandEvent &event )
{
    guLogMessage(wxT( "OnFolderPaste"));
    wxTheClipboard->UsePrimarySelection(false);

    if (!wxTheClipboard->Open())
    {
        guLogError(wxT("Cannot open the clipboard object"));
        return;
    }

//        if( wxTheClipboard->IsSupported( wxDataFormat(  wxT( "text/uri-list" ) ) ) )
//        {
//            guLogMessage( wxT( "Supported format uri-list.." ) );
//            wxCustomDataObject CustomDataObject( wxDataFormat( wxT( "text/uri-list" ) ) );
//            if( wxTheClipboard->GetData( CustomDataObject ) )
//                guLogMessage( wxT( "Custom Data: (%i) '%s'" ), CustomDataObject.GetSize(), wxString( ( const char * ) CustomDataObject.GetData(), wxConvUTF8 ).c_str() );
//        }
//        if( wxTheClipboard->IsSupported( wxDataFormat( wxT( "x-special/gnome-copied-files" ) ) ) )
//        {
//            guLogMessage( wxT( "Supported format x-special..." ) );
//            wxCustomDataObject CustomDataObject( wxDataFormat( wxT( "x-special/gnome-copied-files" ) ) ); //( wxT( "Supported format x-special..." ) );
//            if( wxTheClipboard->GetData( CustomDataObject ) )
//                guLogMessage( wxT( "Custom Data: (%i) '%s'" ), CustomDataObject.GetSize(), wxString( ( const char * ) CustomDataObject.GetData(), wxConvUTF8 ).c_str() );
//        }
//        else
    if (wxTheClipboard->IsSupported(wxDF_FILENAME))
    {
        wxFileDataObject FileObject;
        if (wxTheClipboard->GetData(FileObject))
        {
            const wxArrayString sourceFiles = FileObject.GetFilenames();
            const wxString curPath = GetPathRemoveTrailSep(m_DirCtrl->GetPath());
            wxArrayString fromFiles;
            wxFileName dirname;
            wxString sourceDir;

            guLogMessage(wxT("Pasting: %s"), sourceFiles[0].c_str());
            int count = sourceFiles.Count();
            for (int index = 0; index < count; index++)
            {
                sourceDir = GetPathRemoveTrailSep(sourceFiles[index]);

                if (wxDirExists(sourceDir))
                {
                    dirname = wxFileName::DirName(sourceDir);
                    wxString lastDir = dirname.GetDirs()[dirname.GetDirCount() - 1];
                    wxString targetDir = curPath + wxT("/") + lastDir;
                    wxFileName::Mkdir(targetDir, 0770, wxPATH_MKDIR_FULL);

                    guAddDirItems(sourceDir, fromFiles);

                    int fromIndex;
                    int fromCount = fromFiles.Count();
                    for (fromIndex = 0; fromIndex < fromCount; fromIndex++)
                    {
                        wxString DestName = fromFiles[fromIndex];
                        DestName.Replace(wxPathOnly(sourceDir), curPath);

                        guLogMessage(wxT("Copy file %s to %s"), fromFiles[fromIndex].c_str(), DestName.c_str());
                        wxCopyFile(fromFiles[fromIndex], DestName);
                    }
                }
                else
                    wxCopyFile(sourceDir, m_DirCtrl->GetPath() + wxT("/") + wxFileNameFromPath(sourceDir));
            }

            m_DirCtrl->CollapsePath(curPath);
            if (count > 0)
            {
                sourceDir = wxPathOnly(sourceFiles[0]);
                m_DirCtrl->CollapsePath(sourceDir);
                m_DirCtrl->ExpandPath(sourceDir);
            }
            m_DirCtrl->ExpandPath(curPath);
        }
        else
            guLogError(wxT("Cannot paste the data from the clipboard"));
    }
    wxTheClipboard->Close();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderEditTracks( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    m_FilesCtrl->GetAllSongs( &Tracks );

    if( Tracks.Count() )
    {
        guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db ? m_Db : m_DefaultDb, &Tracks, &Images, &Lyrics, &ChangedFlags );

        if( TrackEditor )
        {
            if( TrackEditor->ShowModal() == wxID_OK )
            {
                guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
                if( m_Db )
                    m_Db->UpdateSongs( &Tracks, ChangedFlags );
                else
                    m_DefaultDb->UpdateSongs( &Tracks, ChangedFlags );
                //guUpdateLyrics( Tracks, Lyrics, ChangedFlags );
                //guUpdateImages( Tracks, Images, ChangedFlags );

                // Update the track in database, playlist, etc
                m_MainFrame->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &Tracks );
            }
            guImagePtrArrayClean( &Images );
            TrackEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderSaveToPlayList( wxCommandEvent &event )
{
    guTrackArray Tracks;

    m_FilesCtrl->GetAllSongs( &Tracks );
    wxArrayInt TrackIds;
    int Count = Tracks.Count();
    for( int Index = 0; Index < Count; Index++ )
        TrackIds.Add( Tracks[ Index ].m_SongId );

    if( m_Db && TrackIds.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists,guPLAYLIST_TYPE_STATIC );
        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( m_MainFrame, m_Db, &TrackIds, &PlayLists );
        if( PlayListAppendDlg->ShowModal() == wxID_OK )
        {
            int Selected = PlayListAppendDlg->GetSelectedPlayList();
            if( Selected == -1 )
            {
                wxString PLName = PlayListAppendDlg->GetPlaylistName();
                if( PLName.IsEmpty() )
                    PLName = _( "UnNamed" );

                m_Db->CreateStaticPlayList( PLName, TrackIds );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, TrackIds );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                    m_Db->AppendStaticPlayList( PLId, TrackIds );
            }
            m_MediaViewer->UpdatePlaylists();
        }
        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderUpdate( wxCommandEvent &event )
{
    if( m_MediaViewer )
        m_MediaViewer->UpdateLibrary( m_DirCtrl->GetPath() );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_FilesCtrl->GetAllSongs( Tracks );

    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
        event.SetId( ID_MAINFRAME_COPYTO );

    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsPlay( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles( true );

    int Count = Files.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guLogMessage( wxT( "File%i: '%s'" ), Index, Files[ Index ].c_str() );
    }
    if( Files.Count() )
    {
        m_PlayerPanel->SetPlayList( Files );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsEnqueue( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles( true );
    if( Files.Count() )
        m_PlayerPanel->AddToPlayList( Files, true, event.GetId() - ID_FILESYSTEM_ITEMS_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsEditTracks( wxCommandEvent &event )
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    m_FilesCtrl->GetSelectedSongs( &Tracks );

    if( Tracks.Count() )
    {
        guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db ? m_Db : m_DefaultDb, &Tracks, &Images, &Lyrics, &ChangedFlags );

        if( TrackEditor )
        {
            if( TrackEditor->ShowModal() == wxID_OK )
            {
                guUpdateTracks( Tracks, Images, Lyrics, ChangedFlags );
                if( m_Db )
                    m_Db->UpdateSongs( &Tracks, ChangedFlags );
                else
                    m_DefaultDb->UpdateSongs( &Tracks, ChangedFlags );
                //guUpdateLyrics( Tracks, Lyrics, ChangedFlags );
                //guUpdateImages( Tracks, Images, ChangedFlags );

                // Update the track in database, playlist, etc
                m_MainFrame->UpdatedTracks( guUPDATED_TRACKS_PLAYER_PLAYLIST, &Tracks );
            }
            guImagePtrArrayClean( &Images );
            TrackEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsSaveToPlayList( wxCommandEvent &event )
{
    guTrackArray Tracks;
    m_FilesCtrl->GetSelectedSongs( &Tracks );

    wxArrayInt TrackIds;
    int Count = Tracks.Count();
    for( int Index = 0; Index < Count; Index++ )
        TrackIds.Add( Tracks[ Index ].m_SongId );

    if( m_Db && TrackIds.Count() )
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );
        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( m_MainFrame, m_Db, &TrackIds, &PlayLists );
        if( PlayListAppendDlg->ShowModal() == wxID_OK )
        {
            int Selected = PlayListAppendDlg->GetSelectedPlayList();
            if( Selected == -1 )
            {
                wxString PLName = PlayListAppendDlg->GetPlaylistName();
                if( PLName.IsEmpty() )
                    PLName = _( "UnNamed" );

                m_Db->CreateStaticPlayList( PLName, TrackIds );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, TrackIds );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                    m_Db->AppendStaticPlayList( PLId, TrackIds );
            }

            m_MediaViewer->UpdatePlaylists();
        }

        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_FilesCtrl->GetSelectedSongs( Tracks );

    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
        event.SetId( ID_MAINFRAME_COPYTO );

    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsRename( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles(true);
    if (!Files.Count())
        return;

    guFileRenamer * FileRenamer = new guFileRenamer(this, m_Db ? m_Db : m_DefaultDb, Files);
    if (!FileRenamer)
        return;

    if (FileRenamer->ShowModal() == wxID_OK)
    {
        wxArrayString RenamedFiles = FileRenamer->GetRenamedNames();
        int Count = RenamedFiles.Count();
        for (int Index = 0; Index < Count; Index++)
        {
            if (Files[Index] != RenamedFiles[Index])
            {
                wxString NewDirName = wxPathOnly(RenamedFiles[Index]);
                if (!wxDirExists(NewDirName))
                    wxFileName::Mkdir(NewDirName, 0770, wxPATH_MKDIR_FULL);

                //if( wxFileExists( Files[ Index ] ) )
                if (!guRenameFile(Files[Index], RenamedFiles[Index]))
                {
                    guLogError(wxT("Could no rename '%s' to '%s'"),
                        Files[Index].c_str(),
                        RenamedFiles[Index].c_str());
                }

                if(m_Db)
                    m_Db->UpdateTrackFileName(Files[Index], RenamedFiles[Index]);
                else
                    m_DefaultDb->UpdateTrackFileName(Files[Index], RenamedFiles[Index]);
            }
        }

        //m_DirCtrl->ExpandPath(m_DirCtrl->GetPath());
        m_FilesCtrl->SetPath(m_DirCtrl->GetPath(), m_MediaViewer);
    }
    FileRenamer->Destroy();
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsDelete( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles();

    bool Error = false;
    int Count = Files.Count();
    if( Count )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected files ?" ),
                         _( "Confirm" ),
                         wxICON_QUESTION | wxYES_NO, this ) == wxYES )
        {
            wxArrayString DeleteFiles;
            for( int Index = 0; Index < Count; Index++ )
            {
                if( wxDirExists( Files[ Index ] ) )
                    Error = !RemoveDirItems( Files[ Index ], &DeleteFiles ) || !wxRmdir( Files[ Index ] );
                else
                {
                    Error = !wxRemoveFile( Files[ Index ] );
                    DeleteFiles.Add( Files[ Index ] );
                }
                if( Error )
                {
                    if (wxMessageBox(
                        wxString::Format("%s ", _("There was an error deleting")) + wxFileNameFromPath(Files[Index]) +
                        _("\nContinue deleting?"),
                        _("Continue"),
                        wxICON_QUESTION | wxYES_NO, this) == wxNO)
                    {
                        break;
                    }
                }
            }
            wxString CurrentFolder = m_DirCtrl->GetPath();
            m_DirCtrl->CollapsePath( CurrentFolder );
            m_DirCtrl->ExpandPath( CurrentFolder );
            //
            //m_Db->DoCleanUp();
            if( m_Db )
                m_Db->CleanFiles( DeleteFiles );
            else
                m_DefaultDb->CleanFiles( DeleteFiles );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnFolderCommand( wxCommandEvent &event )
{
    int CommandId = event.GetId();

    guConfig * Config = (guConfig *) guConfig::Get();
    if (!Config)
        return;

    wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
    wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
    wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);

    CommandId -= ID_COMMANDS_BASE;
    wxString CurCmd = Commands[ CommandId ];
    if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
    {
        wxString DirPath = m_DirCtrl->GetPath();
        DirPath.Replace( wxT( " " ), wxT( "\\ " ) );
        CurCmd.Replace( guCOMMAND_ALBUMPATH, DirPath );
    }

    if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
    {
        wxString SongList;
        wxArrayString Files = m_FilesCtrl->GetAllFiles( true );
        int Count = Files.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            SongList += wxT( " \"" ) + Files[ Index ] + wxT( "\"" );
        }
        CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
    }

    //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
    guExecute( CurCmd );
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsCommand( wxCommandEvent &event )
{
    int CommandId = event.GetId();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        wxString current_desktop = Config->ReadStr(CONFIG_KEY_GENERAL_DESKTOP, wxEmptyString, CONFIG_PATH_GENERAL);
        wxString category_execs = wxString::Format(CONFIG_PATH_COMMANDS_DESKTOP_EXECS, current_desktop);
        wxArrayString Commands = Config->ReadAStr(CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, category_execs);

        CommandId -= ID_COMMANDS_BASE;
        wxString CurCmd = Commands[ CommandId ];
        if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
        {
            wxString DirPath = m_DirCtrl->GetPath();
            DirPath.Replace( wxT( " " ), wxT( "\\ " ) );
            CurCmd.Replace( guCOMMAND_ALBUMPATH, DirPath );
        }

        if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
        {
            wxString SongList;
            wxArrayString Files = m_FilesCtrl->GetSelectedFiles( false );
            int Count = Files.Count();
            for( int Index = 0; Index < Count; Index++ )
                SongList += wxT( " \"" ) + Files[ Index ] + wxT( "\"" );
            CurCmd.Replace( guCOMMAND_COVERPATH, SongList.Trim( false ) );
        }

        if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
        {
            wxString SongList;
            wxArrayString Files = m_FilesCtrl->GetSelectedFiles( true );
            int Count = Files.Count();
            for( int Index = 0; Index < Count; Index++ )
                SongList += wxT( " \"" ) + Files[ Index ] + wxT( "\"" );
            CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
        }

        //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
        guExecute( CurCmd );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsCopy( wxCommandEvent &event )
{
    wxArrayString Files = m_FilesCtrl->GetSelectedFiles( false );
    if( Files.Count() )
    {
        wxTheClipboard->UsePrimarySelection( false );
        if( wxTheClipboard->Open() )
        {
            wxTheClipboard->Clear();
            wxFileDataObject * FileObject = new wxFileDataObject();
            wxTextDataObject * TextObject = new wxTextDataObject();
            wxDataObjectComposite * CompositeObject = new wxDataObjectComposite();

            wxString FilesText;
            int Count = Files.Count();
            for( int Index = 0; Index < Count; Index++ )
            {
                wxString CurFile = Files[ Index ];
                FilesText += ( FilesText.IsEmpty() ? wxT( "" ) : wxT( "\n" ) ) + CurFile;

                CurFile.Replace( wxT( "#" ), wxT( "%23" ) );
                FileObject->AddFile( CurFile );
            }

            TextObject->SetText( FilesText );

            CompositeObject->Add( FileObject );
            CompositeObject->Add( TextObject );

            if( !wxTheClipboard->AddData( CompositeObject ) )
            {
                delete FileObject;
                delete TextObject;
                delete CompositeObject;
                guLogError( wxT( "Can't copy the selected files to the clipboard" ) );
            }
            wxTheClipboard->Close();
        }
        else
            guLogError( wxT( "Could not open the clipboard object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guFileBrowser::OnItemsPaste( wxCommandEvent &event )
{
    wxString DestFolder;
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_FILENAME ) )
        {
            wxArrayString Selection = m_FilesCtrl->GetSelectedFiles( false );
            if( ( Selection.Count() == 1 ) && wxDirExists( Selection[ 0 ] ) )
                DestFolder = Selection[ 0 ];
            else
                DestFolder = m_DirCtrl->GetPath();

            wxFileDataObject FileObject;
            if( wxTheClipboard->GetData( FileObject ) )
            {
                wxArrayString Files = FileObject.GetFilenames();
                wxArrayString FromFiles;
                //guLogMessage( wxT( "Pasted: %s" ), Files[ 0 ].c_str() );
                int Count = Files.Count();
                for( int Index = 0; Index < Count; Index++ )
                {
                    if( wxDirExists( Files[ Index ] ) )
                    {
                        guAddDirItems( Files[ Index ], FromFiles );

                        int FromIndex;
                        int FromCount = FromFiles.Count();
                        for( FromIndex = 0; FromIndex < FromCount; FromIndex++ )
                        {
                            wxString DestName = FromFiles[ FromIndex ];
                            DestName.Replace( wxPathOnly( Files[ Index ] ), DestFolder );
                            wxFileName::Mkdir( wxPathOnly( DestName ), 0770, wxPATH_MKDIR_FULL );
                            guLogMessage( wxT( "Copy file %s to %s" ), FromFiles[ FromIndex ].c_str(), DestName.c_str() );
                            wxCopyFile( FromFiles[ FromIndex ], DestName );
                        }
                    }
                    else
                        wxCopyFile( Files[ Index ], m_DirCtrl->GetPath() + wxT( "/" ) + wxFileNameFromPath( Files[ Index ] ) );
                }
                wxString CurPath = m_DirCtrl->GetPath();
                m_DirCtrl->CollapsePath( CurPath );
                m_DirCtrl->ExpandPath( CurPath );
            }
            else
                guLogError( wxT( "Can't paste the data from the clipboard" ) );
        }
        wxTheClipboard->Close();
    }
    else
        guLogError( wxT( "Could not open the clipboard object" ) );
}

}
