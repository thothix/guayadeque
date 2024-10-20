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
#include "DiBrowser.h"

#include "Accelerators.h"
#include "FileBrowser.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "Images.h"
#include "PlayListAppend.h"
#include "Utils.h"
#include "LibPanel.h"
#include "MediaViewer.h"

#include "FileRenamer.h"
#include "LibUpdate.h"
#include "MainFrame.h"
#include "TagInfo.h"
#include "TrackEdit.h"

#include <wx/clipbrd.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guDiGenericDirCtrl
// -------------------------------------------------------------------------------- //
guDiGenericDirCtrl::guDiGenericDirCtrl(wxWindow * parent, guLibPanel * libpanel) :
        wxGenericDirCtrl( parent, wxID_ANY, wxDirDialogDefaultFolderStr,
                          wxDefaultPosition, wxDefaultSize, wxDIRCTRL_DIR_ONLY,
                          wxEmptyString, 0, wxTreeCtrlNameStr )
{
    m_LibPanel = libpanel;
    m_MainFrame = m_LibPanel->GetMainFrame();;
    m_DiBrowserCtrl = (guDiBrowser *) parent;

    wxImageList * ImageList = GetTreeCtrl()->GetImageList();
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_library ) );
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_podcast ) );
    ImageList->Add( guImage( guIMAGE_INDEX_tiny_record ) );

    guConfig * Config = (guConfig *) guConfig::Get();
    Config->RegisterObject(this);

    Bind(guConfigUpdatedEvent, &guDiGenericDirCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED);
}

// -------------------------------------------------------------------------------- //
guDiGenericDirCtrl::~guDiGenericDirCtrl()
{
    guConfig * Config = (guConfig *) guConfig::Get();
    Config->UnRegisterObject(this);

    Unbind(guConfigUpdatedEvent, &guDiGenericDirCtrl::OnConfigUpdated, this, ID_CONFIG_UPDATED);
}

// -------------------------------------------------------------------------------- //
void guDiGenericDirCtrl::SetupSections()
{
    const guMediaCollectionArray & Collections = m_MainFrame->GetMediaCollections();
    int Count = Collections.Count();

    //ShowHidden(true);

    for (int Index = 0; Index < Count; Index++)
    {
        const guMediaCollection & Collection = Collections[Index];
        bool is_active = m_MainFrame->IsCollectionActive(Collection.m_UniqueId);
//        guLogMessage(wxT("Collection %s - Libpanel %s - Active %d"),  Collection.m_UniqueId,
//                      this->m_LibPanel->GetMediaViewer()->GetMediaCollection()->m_UniqueId, is_active);

        if (is_active && Collection.m_UniqueId == this->m_LibPanel->GetMediaViewer()->GetMediaCollection()->m_UniqueId)
        {
            int PathIndex;
            int PathCount = Collection.m_Paths.Count();
            m_CollectionId = AddSection(wxT(GU_COLLECTION_DUMMY_ROOTDIR), Collection.m_Name, guDIBROWSER_IMAGE_INDEX_LIBRARY );

            for (PathIndex = 0; PathIndex < PathCount; PathIndex++)
            {
                wxString LibName = Collection.m_Paths[PathIndex];
                RemovePathTrailSep(LibName);
                //AddSection(LibName, wxFileNameFromPath(LibName), guDIBROWSER_IMAGE_INDEX_LIBRARY);

                auto *dir_item = new wxDirItemData(LibName, wxFileNameFromPath(LibName), true);
                wxTreeItemId treeid = AppendItem(m_CollectionId, wxFileNameFromPath(LibName), guDIBROWSER_IMAGE_INDEX_LIBRARY, -1, dir_item);
                GetTreeCtrl()->SetItemHasChildren(treeid);
                GetTreeCtrl()->Expand(treeid);
                guLogMessage(wxT("Add section - %s"), LibName);
            }
        }
    }
    //GetTreeCtrl()->ExpandAll();
}

// -------------------------------------------------------------------------------- //
void guDiGenericDirCtrl::OnConfigUpdated(wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if (Flags & (guPREFERENCE_PAGE_FLAG_LIBRARY | guPREFERENCE_PAGE_FLAG_RECORD | guPREFERENCE_PAGE_FLAG_PODCASTS))
    {
        wxString CurPath = GetPath();
        ReCreateTree();
        SetPath(CurPath);
    }
}


// -------------------------------------------------------------------------------- //
// guDiBrowser
// -------------------------------------------------------------------------------- //
guDiBrowser::guDiBrowser(wxWindow *parent, guLibPanel *libpanel, guDbLibrary *db, guMediaViewer *mediaviewer, const wxString &dirpath) :
        wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER )
{
    m_LibPanel = libpanel;
    m_MediaViewer = mediaviewer;
    m_MainFrame = mediaviewer->GetMainFrame();
    m_DefaultDb = db;
    m_Db = nullptr;
    m_AddingFolder = false;

    auto *Config = (guConfig *) guConfig::Get();
    Config->RegisterObject( this );

    auto *MainSizer = new wxBoxSizer(wxVERTICAL);

    m_DirCtrl = new guDiGenericDirCtrl(this, m_LibPanel);
    m_DirCtrl->ShowHidden(false);
    SetPath(dirpath, m_MediaViewer);

    MainSizer->Add(m_DirCtrl, 1, wxEXPAND, 5);

    this->SetSizer(MainSizer);
    this->Layout();

    CreateAcceleratorTable();

    // Binds
    m_DirCtrl->Bind(wxEVT_TREE_ITEM_MENU, &guDiBrowser::OnContextMenu, this);
    Bind(guConfigUpdatedEvent, &guDiBrowser::OnConfigUpdated, this, ID_CONFIG_UPDATED);

    Bind(wxEVT_MENU, &guDiBrowser::OnFolderPlay, this, ID_FILESYSTEM_FOLDER_PLAY);
    Bind(wxEVT_MENU, &guDiBrowser::OnFolderEnqueue, this, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST);
    Bind(wxEVT_MENU, &guDiBrowser::OnFolderCopy, this, ID_FILESYSTEM_FOLDER_COPY);
    Bind(wxEVT_MENU, &guDiBrowser::OnFolderEditTracks, this, ID_FILESYSTEM_FOLDER_EDITTRACKS);
    Bind(wxEVT_MENU, &guDiBrowser::OnFolderSaveToPlayList, this, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST);
    Bind(wxEVT_MENU, &guDiBrowser::OnFolderUpdate, this, ID_FILESYSTEM_FOLDER_UPDATE);
    m_DirCtrl->Bind( wxEVT_MENU, &guDiBrowser::OnFolderCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    m_DirCtrl->Bind( wxEVT_MENU, &guDiBrowser::OnFolderCommand, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
}

// -------------------------------------------------------------------------------- //
guDiBrowser::~guDiBrowser()
{
    auto *Config = (guConfig *) guConfig::Get();
    Config->UnRegisterObject( this );

    Unbind(wxEVT_MENU, &guDiBrowser::OnFolderPlay, this, ID_FILESYSTEM_FOLDER_PLAY);
    Unbind(wxEVT_MENU, &guDiBrowser::OnFolderEnqueue, this, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST);
    Unbind(wxEVT_MENU, &guDiBrowser::OnFolderCopy, this, ID_FILESYSTEM_FOLDER_COPY);
    Unbind(wxEVT_MENU, &guDiBrowser::OnFolderEditTracks, this, ID_FILESYSTEM_FOLDER_EDITTRACKS);
    Unbind(wxEVT_MENU, &guDiBrowser::OnFolderSaveToPlayList, this, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST);
    Unbind(wxEVT_MENU, &guDiBrowser::OnFolderUpdate, this, ID_FILESYSTEM_FOLDER_UPDATE);
    m_DirCtrl->Unbind( wxEVT_MENU, &guDiBrowser::OnFolderCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    m_DirCtrl->Unbind( wxEVT_MENU, &guDiBrowser::OnFolderCommand, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    m_DirCtrl->Unbind(wxEVT_TREE_ITEM_MENU, &guDiBrowser::OnContextMenu, this );
    Unbind(guConfigUpdatedEvent, &guDiBrowser::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

wxString guDiBrowser::DefaultPath(const wxString &path)
{
    wxString result = m_DirCtrl->GetDefaultPath();
    if (!path.IsEmpty())
        m_DirCtrl->SetDefaultPath(path);
    return result;
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnContextMenu(wxTreeEvent &event)
{
    wxMenu Menu;
    wxMenuItem *MenuItem;

    wxPoint Point = event.GetPoint();

    MenuItem = new wxMenuItem(&Menu, ID_FILESYSTEM_FOLDER_PLAY,
                              wxString(_("Play")) + guAccelGetCommandKeyCodeString(ID_TRACKS_PLAY),
                              _("Play the selected folder"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_player_tiny_light_play));
    Menu.Append(MenuItem);

    MenuItem = new wxMenuItem(&Menu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL,
                              wxString(_("Enqueue")) + guAccelGetCommandKeyCodeString(ID_TRACKS_ENQUEUE_AFTER_ALL),
                              _("Add the selected folder to playlist"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_add));
    Menu.Append(MenuItem);

    // Enqueue After
    wxMenu *EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem(EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_TRACK,
                              wxString(_("Current Track")) +
                              guAccelGetCommandKeyCodeString(ID_TRACKS_ENQUEUE_AFTER_TRACK),
                              _("Add current selected tracks to playlist after the current track"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_add));
    EnqueueMenu->Append(MenuItem);

    MenuItem = new wxMenuItem(EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALBUM,
                              wxString(_("Current Album")) +
                              guAccelGetCommandKeyCodeString(ID_TRACKS_ENQUEUE_AFTER_ALBUM),
                              _("Add current selected tracks to playlist after the current album"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_add));
    EnqueueMenu->Append(MenuItem);

    MenuItem = new wxMenuItem(EnqueueMenu, ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ARTIST,
                              wxString(_("Current Artist")) +
                              guAccelGetCommandKeyCodeString(ID_TRACKS_ENQUEUE_AFTER_ARTIST),
                              _("Add current selected tracks to playlist after the current artist"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_add));
    EnqueueMenu->Append(MenuItem);

    Menu.Append(wxID_ANY, _("Enqueue After"), EnqueueMenu);
    Menu.AppendSeparator();
    // END Enqueue After

    MenuItem = new wxMenuItem(&Menu, ID_FILESYSTEM_FOLDER_EDITTRACKS,
                              wxString(_("Edit Tracks")) +
                              guAccelGetCommandKeyCodeString(ID_PLAYER_PLAYLIST_EDITTRACKS),
                              _("Edit the tracks in the selected folder"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_edit));
    Menu.Append(MenuItem);
    Menu.AppendSeparator();

    MenuItem = new wxMenuItem(&Menu, ID_FILESYSTEM_FOLDER_SAVEPLAYLIST,
                              wxString(_("Save to Playlist")) +
                              guAccelGetCommandKeyCodeString(ID_PLAYER_PLAYLIST_SAVE),
                              _("Add the tracks in the selected folder to a playlist"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_doc_save));
    Menu.Append(MenuItem);
    Menu.AppendSeparator();

    MenuItem = new wxMenuItem(&Menu, ID_FILESYSTEM_FOLDER_COPY,
                              _("Copy"),
                              _("Copy the selected folder to clipboard"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_edit_copy));
    Menu.Append(MenuItem);
    //MenuItem->Enable( false );

    MenuItem = new wxMenuItem(&Menu, ID_FILESYSTEM_FOLDER_UPDATE,
                              _("Update"),
                              _("Update the selected folder"));
    MenuItem->SetBitmap(guImage(guIMAGE_INDEX_tiny_reload));
    Menu.Append(MenuItem);
    Menu.AppendSeparator();

    m_LibPanel->CreateCopyToMenu(&Menu);

    AppendFolderCommands(&Menu);

    PopupMenu(&Menu, Point);
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::CreateAcceleratorTable(void )
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

    if (guAccelDoAcceleratorTable(AliasAccelCmds, RealAccelCmds, AccelTable))
        SetAcceleratorTable(AccelTable);
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnConfigUpdated(wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if (Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS)
        CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::SetMediaViewer(guMediaViewer * mediaviewer)
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : nullptr;
}

// -------------------------------------------------------------------------------- //
wxString guDiBrowser::GetPath()
{
    return GetPathAddTrailSep(m_DirCtrl->GetPath());
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::SetPath(const wxString &path, guMediaViewer * mediaviewer )
{
    guLogMessage(wxT("guDiBrowser::SetPath( %s )"), path.c_str());
    SetMediaViewer(mediaviewer);
    m_CurDir = GetPathAddTrailSep(path);
    m_DirCtrl->SetPath(m_CurDir);
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::LoadPath(const wxString &path, guMediaViewer * mediaviewer, bool recreate)
{
    guLogMessage(wxT("guDiBrowser::SetPath( %s )"), path.c_str());
    SetMediaViewer(mediaviewer);
    m_CurDir = GetPathAddTrailSep(path);
    if (recreate)
    {
        m_DirCtrl->ReCreateTree();
        m_DirCtrl->ExpandPath(GetPathRemoveTrailSep(GU_COLLECTION_DUMMY_ROOTDIR) + m_CurDir);
    }
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::CollectionsUpdated()
{
    LoadPath(GetPath(), m_MediaViewer, true);
//    wxString cur_path = GetPath();
//    guLogMessage(wxT( "guDiBrowser::CollectionsUpdated( %s )" ), cur_path.c_str() );
//    m_DirCtrl->ReCreateTree();
//    SetPath(cur_path, FindMediaViewerByPath(cur_path));
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::ReloadItems()
{
    m_Files.Empty();
    GetItemsList();
    //SetItemCount( m_Files.Count() );
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::GetItemsList()
{
    GetPathSortedItems(m_CurDir, &m_Files);
//    wxCommandEvent event(wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO);
//    AddPendingEvent(event);
}

// -------------------------------------------------------------------------------- //
guMediaViewer * guDiBrowser::FindMediaViewerByPath(const wxString cur_path)
{
    const guMediaCollectionArray &Collections = m_MainFrame->GetMediaCollections();
    int Count = Collections.Count();
    //guLogMessage( wxT( "guDiBrowser FindMediaViewerByPath %s - count: %i" ), cur_path.c_str(), Count );

    for (int Index = 0; Index < Count; Index++)
    {
        const guMediaCollection & Collection = Collections[Index];
        bool is_Active = m_MainFrame->IsCollectionActive(Collection.m_UniqueId);
        //guLogMessage(wxT("guDiBrowser FindMediaViewerByPath %s - %s - active %d"), Collection.m_UniqueId, Collection.m_Name, is_Active);
        if (is_Active)
        {
            int PathIndex;
            int PathCount = Collection.m_Paths.Count();
            for (PathIndex = 0; PathIndex < PathCount; PathIndex++)
            {
                //guLogMessage( wxT("guDiBrowser FindMediaViewerByPath cur_path %s == %s" ), cur_path.c_str(), Collection.m_Paths[ PathIndex ].c_str());
                if (cur_path.StartsWith(Collection.m_Paths[PathIndex]))
                    return m_MainFrame->FindCollectionMediaViewer(Collection.m_UniqueId);
            }
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnFolderPlay(wxCommandEvent &event)
{
    wxArrayString Files = GetAllFiles(true);
    m_LibPanel->GetPlayerPanel()->SetPlayList(Files);
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnFolderEnqueue(wxCommandEvent &event)
{
    wxArrayString Files = GetAllFiles(true);

    guLogMessage(wxT("guDiBrowser::OnFolderEnqueue( %d )"), Files.Count());

    if (Files.Count())
        m_LibPanel->GetPlayerPanel()->AddToPlayList(Files, true, event.GetId() - ID_FILESYSTEM_FOLDER_ENQUEUE_AFTER_ALL);
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnFolderCopy( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnFolderCopy" ) );
    wxTheClipboard->UsePrimarySelection( false );
    if (!wxTheClipboard->Open())
    {
        guLogError(wxT("Could not open the clipboard object"));
        return;
    }

    wxTheClipboard->Clear();
    wxFileDataObject * FileObject = new wxFileDataObject();
    wxTextDataObject * TextObject = new wxTextDataObject();
    wxDataObjectComposite * CompositeObject = new wxDataObjectComposite();

    wxString Path = m_DirCtrl->GetPath();
    Path.Replace( wxT( "#" ), wxT( "%23" ) );

    TextObject->SetText( Path );
    FileObject->AddFile( Path );

    CompositeObject->Add( FileObject );
    CompositeObject->Add( TextObject );

    if( !wxTheClipboard->AddData( CompositeObject ) )
    {
        delete FileObject;
        delete TextObject;
        delete CompositeObject;
        guLogError( wxT( "Can't copy the folder to the clipboard" ) );
    }
    guLogMessage( wxT( "Copied the data to the clipboard..." ) );
    wxTheClipboard->Close();
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnFolderEditTracks(wxCommandEvent &event)
{
    guTrackArray Tracks;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    GetAllSongs(&Tracks);
    if (!Tracks.Count())
        return;

    auto * TrackEditor = new guTrackEditor(this, m_Db ? m_Db : m_DefaultDb, &Tracks, &Images, &Lyrics, &ChangedFlags);
    if (!TrackEditor)
        return;

    if (TrackEditor->ShowModal() == wxID_OK)
    {
        guUpdateTracks(Tracks, Images, Lyrics, ChangedFlags);
        if (m_Db)
            m_Db->UpdateSongs(&Tracks, ChangedFlags);
        else
            m_DefaultDb->UpdateSongs(&Tracks, ChangedFlags);

        // Update the track in database, playlist, etc
        m_MainFrame->UpdatedTracks(guUPDATED_TRACKS_PLAYER_PLAYLIST, &Tracks);
    }
    guImagePtrArrayClean(&Images);
    TrackEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnFolderSaveToPlayList(wxCommandEvent &event)
{
    guTrackArray Tracks;

    GetAllSongs(&Tracks);

    wxArrayInt TrackIds;
    size_t Count = Tracks.Count();
    for (size_t Index = 0; Index < Count; Index++)
        TrackIds.Add(Tracks[Index].m_SongId);

    if (m_Db && TrackIds.Count())
    {
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists,guPLAYLIST_TYPE_STATIC );
        auto * PlayListAppendDlg = new guPlayListAppend( m_MainFrame, m_Db, &TrackIds, &PlayLists );
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
void guDiBrowser::OnFolderUpdate(wxCommandEvent &event)
{
    if (m_MediaViewer)
        m_MediaViewer->UpdateLibrary(m_DirCtrl->GetPath());
}

// -------------------------------------------------------------------------------- //
void guDiBrowser::OnFolderCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    GetAllSongs( Tracks );

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
void guDiBrowser::OnFolderCommand( wxCommandEvent &event )
{
    int CommandId = event.GetId();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        wxArrayString Commands = Config->ReadAStr( CONFIG_KEY_COMMANDS_EXEC, wxEmptyString, CONFIG_PATH_COMMANDS_EXECS );

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
            wxArrayString Files = GetAllFiles( true );
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
}

// -------------------------------------------------------------------------------- //
size_t guDiBrowser::GetAllSongs(guTrackArray * tracks) const
{
    wxArrayString Files = GetAllFiles(true);
    return GetTracksFromFiles(Files, tracks);
}

// -------------------------------------------------------------------------------- //
wxArrayString guDiBrowser::GetAllFiles(const bool recursive ) const
{
    wxArrayString Files;
    size_t Count = m_Files.Count();
    if (!Count)
        return Files;

    for (size_t Index = 0; Index < Count; Index++)
    {
        if (m_Files[Index].m_Name != wxT(".."))
        {
            if (recursive && (m_Files[Index].m_Type == guFILEITEM_TYPE_FOLDER))
            {
                guFileItemArray DirFiles;
                if (GetPathSortedItems(m_CurDir + m_Files[Index].m_Name, &DirFiles, true))
                {
                    size_t FileIndex;
                    size_t FileCount = DirFiles.Count();
                    for (FileIndex = 0; FileIndex < FileCount; FileIndex++)
                        Files.Add(DirFiles[FileIndex].m_Name);
                }
            }
            else
                Files.Add(m_CurDir + m_Files[Index].m_Name);
        }
    }
    return Files;
}

// -------------------------------------------------------------------------------- //
size_t guDiBrowser::GetTracksFromFiles(const wxArrayString &files, guTrackArray * tracks) const
{
    size_t Count = files.Count();
    if( Count )
    {
        for (size_t Index = 0; Index < Count; Index++)
        {
            wxString FileName = files[Index];
            //guLogMessage( wxT( "GetTracksFromFiles: %s" ), FileName.c_str() );
            wxURI Uri(FileName);

            if (  Uri.IsReference())
            {
                if (guIsValidAudioFile(FileName))
                {
                    guTrack * Track = new guTrack();
                    Track->m_FileName = FileName;

                    if( !m_Db || !m_Db->FindTrackFile( FileName, Track ) )
                    {
                        guPodcastItem PodcastItem;
                        auto * MainFrame = (guMainFrame *) guMainFrame::GetMainFrame();
                        guDbPodcasts * DbPodcasts = MainFrame->GetPodcastsDb();
                        if (DbPodcasts->GetPodcastItemFile(FileName, &PodcastItem))
                        {
                            delete Track;
                            continue;
                        }

                        if( Track->ReadFromFile( FileName ) )
                            Track->m_Type = guTRACK_TYPE_NOTDB;
                        else
                            guLogError( wxT( "Could not read tags from file '%s'" ), FileName.c_str() );
                    }
                    tracks->Add( Track );
                }
            }
        }
    }
    return tracks->Count();
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
int guDiBrowser::GetPathSortedItems(const wxString &path, guFileItemArray * items, const bool recursive ) const
{
    wxString Path = path;
    AddPathTrailSep(Path);

    if (path.IsEmpty() || !wxDirExists(Path))
        return items->Count();

    wxDir Dir(Path);
    if (!Dir.IsOpened())
        return items->Count();

    wxString Filename, FullFilename;
    if (Dir.GetFirst(&Filename, wxEmptyString, wxDIR_FILES | wxDIR_DIRS | wxDIR_DOTDOT))
    {
        do {
            if (Filename == wxT("."))
                continue;

            FullFilename = Path + Filename;
            if (recursive && wxDirExists(FullFilename))
            {
                if (Filename != wxT(".."))
                    GetPathSortedItems(FullFilename, items, recursive);
            }
            else
            {
                guFileItem * FileItem = new guFileItem();
                if (recursive)
                    FileItem->m_Name = Path;
                FileItem->m_Name += Filename;

                GetFileDetails(FullFilename, FileItem);
                if (!wxDirExists(FullFilename))
                {
                    if (guIsValidAudioFile(Filename.Lower()))
                        FileItem->m_Type = guFILEITEM_TYPE_AUDIO;
                    else if (guIsValidImageFile(Filename.Lower()))
                        FileItem->m_Type = guFILEITEM_TYPE_IMAGE;
                }
                items->Add(FileItem);
            }
        } while (Dir.GetNext(&Filename));
    }
    items->Sort(CompareFileNameA);

    return items->Count();
}

}
