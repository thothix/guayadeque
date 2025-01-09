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
#include "GIO_Volume.h"

#include "EventCommandIds.h"
#include "MainFrame.h"
#include "Utils.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
extern "C" {

static void VolumeAdded( GVolumeMonitor * monitor, GVolume * volume, guGIO_VolumeMonitor * volmon )
{
	volmon->OnVolumeAdded( volume );
}

static void VolumeRemoved( GVolumeMonitor * monitor, GVolume * volume, guGIO_VolumeMonitor * volmon )
{
	volmon->OnVolumeRemoved( volume );
}

static void MountAdded( GVolumeMonitor * monitor, GMount * mount, guGIO_VolumeMonitor * volmon )
{
	volmon->OnMountAdded( mount );
}

static void MountRemoved( GVolumeMonitor * monitor, GMount * mount, guGIO_VolumeMonitor * volmon )
{
	volmon->OnMountRemoved( mount );
}

static void append_mount( GMount * mount, guGIO_VolumeMonitor * volmon )
{
	volmon->OnMountAdded( mount );
}


};


// -------------------------------------------------------------------------------- //
// guGIO_Mount
// -------------------------------------------------------------------------------- //
guGIO_Mount::guGIO_Mount( GMount * mount )
{
    m_Mount = mount;
    g_object_ref( mount );

    char * mount_name = g_mount_get_name( m_Mount );
    if( mount_name )
    {
        m_Name = wxString( mount_name, wxConvUTF8 );
        g_free( mount_name );
    }

    GFile * RootFile = g_mount_get_root( mount );
    if( RootFile)
    {
        char * Path = g_file_get_path( RootFile );
        m_MountPath = wxString( Path, wxConvUTF8 ) + wxT( "/" );
        // If it is a mtp device find the first writeable folder...
        if( m_MountPath.Find( "mtp:" ) != wxNOT_FOUND )
        {
            wxFileName FileName( m_MountPath );

            if( !FileName.IsDirWritable() ) // if cant be created in the default location ...
            {
                wxString WriteableFolder = FindWriteableFolder( m_MountPath );
                if( !WriteableFolder.IsEmpty() )
                {
                    m_MountPath = WriteableFolder;
                }
            }
        }

        guLogMessage( wxT( "Mount Path: %s" ), m_MountPath.c_str() );
        g_free( Path );
        g_object_unref( RootFile );
    }
    GIcon * Icon = g_mount_get_icon( mount );
    if( Icon )
    {
        char * IconStr = g_icon_to_string( Icon );
        if( IconStr )
        {
            m_IconString = wxString( IconStr, wxConvUTF8 );
            guLogMessage( wxT( "IconStr: '%s'" ), m_IconString.c_str() );
            g_free( IconStr );
        }
        g_object_unref( Icon );
    }

    wxFileConfig Config( wxEmptyString, wxEmptyString, m_MountPath + wxT( ".is_audio_player" ) );
    m_Id = Config.Read( wxT( "audio_player_id" ), wxString::Format( wxT( "%08lX" ), wxGetLocalTime() ) );
}

// -------------------------------------------------------------------------------- //
guGIO_Mount::guGIO_Mount( GMount * mount, wxString &mountpath )
{
    m_Mount = mount;
    g_object_ref( mount );
    m_MountPath = mountpath;
    if( !m_MountPath.EndsWith( wxT( "/" ) ) )
        m_MountPath.Append( wxT( "/" ) );
    // If it is a mtp device find the first writeable folder...
    if( m_MountPath.Find( "mtp:" ) != wxNOT_FOUND )
    {
        wxFileName FileName( m_MountPath );

        if( !FileName.IsDirWritable() ) // if cant be created in the default location ...
        {
            wxString WriteableFolder = FindWriteableFolder( m_MountPath );
            if( !WriteableFolder.IsEmpty() )
            {
                m_MountPath = WriteableFolder;
            }
        }
    }

    char * mount_name = g_mount_get_name( m_Mount );
    if( mount_name )
    {
        m_Name = wxString( mount_name, wxConvUTF8 );
        g_free( mount_name );
    }

    guLogMessage( wxT( "Mount Path: %s" ), m_MountPath.c_str() );
    GIcon * Icon = g_mount_get_icon( mount );
    if( Icon )
    {
        char * IconStr = g_icon_to_string( Icon );
        if( IconStr )
        {
            m_IconString = wxString( IconStr, wxConvUTF8 );
            guLogMessage( wxT( "IconStr: '%s'" ), m_IconString.c_str() );
            g_free( IconStr );
        }
        g_object_unref( Icon );
    }

    wxFileConfig Config( wxEmptyString, wxEmptyString, m_MountPath + wxT( ".is_audio_player" ) );
    m_Id = Config.Read( wxT( "audio_player_id" ), wxString::Format( wxT( "%08lX" ), wxGetLocalTime() ) );
}

// -------------------------------------------------------------------------------- //
guGIO_Mount::~guGIO_Mount()
{
}

// -------------------------------------------------------------------------------- //
wxString guGIO_Mount::FindWriteableFolder( const wxString &mountpath )
{
    wxDir       Dir;
    wxString    FoundFile;
    wxString    MountPath = mountpath;
    if( !MountPath.EndsWith( wxT( "/" ) ) )
        MountPath += wxT( "/" );

    Dir.Open( MountPath );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FoundFile, wxEmptyString, wxDIR_DIRS ) )
        {
            do {
                //guLogMessage( wxT( "Found file '%s'" ), FoundFile.c_str() );

                if( FoundFile == wxT( "." ) ||
                    FoundFile == wxT( ".." ) )
                    continue;

                if( wxIsWritable( MountPath + FoundFile + wxT( "/" ) ) )
                    return MountPath + FoundFile + wxT( "/" );

                if( Dir.Exists( MountPath + FoundFile ) )
                {
                    wxString WriteableFolder = FindWriteableFolder( MountPath + FoundFile );
                    if( !WriteableFolder.IsEmpty() )
                    {
                        //guLogMessage( wxT( "Found writeable folder at '%s'" ), WriteableFolder.c_str() );
                        return WriteableFolder;
                    }
                }

            } while( Dir.GetNext( &FoundFile ) );
        }
    }

    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guGIO_Mount::CanUnmount( void )
{
    return g_mount_can_eject( m_Mount );
}

// -------------------------------------------------------------------------------- //
void Unmounted_Device( GObject * object, GAsyncResult * result, guGIO_Mount * mnt )
{
    GError * error = NULL;

    if( G_IS_MOUNT( object ) )
    {
        GMount * mount = ( GMount * ) object;
        g_mount_eject_with_operation_finish( mount, result, &error);
    }

    if( error )
    {
		if( !g_error_matches( error, G_IO_ERROR, G_IO_ERROR_FAILED_HANDLED ) )
		{
			guLogError( wxT( "Unable to eject %s" ), error->message );
		}
		else
		{
			guLogMessage( wxT( "Eject was already done" ) );
		}
		g_error_free (error);
	}

}

// -------------------------------------------------------------------------------- //
void guGIO_Mount::Unmount( void )
{
    g_mount_eject_with_operation( m_Mount, G_MOUNT_UNMOUNT_NONE, NULL, NULL, GAsyncReadyCallback( Unmounted_Device ), this );
}

// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
guGIO_VolumeMonitor::guGIO_VolumeMonitor( guMainFrame * mainframe )
{
    m_MainFrame = mainframe;
    m_MountedVolumes = new guGIO_MountArray();

    m_VolumeMonitor = g_volume_monitor_get();
    if( m_VolumeMonitor )
    {
        m_VolumeAddedId = g_signal_connect( m_VolumeMonitor, "volume-added", G_CALLBACK( VolumeAdded ), this );
        m_VolumeRemovedId = g_signal_connect( m_VolumeMonitor, "volume-removed", G_CALLBACK( VolumeRemoved ), this );
        m_MountAddedId = g_signal_connect( m_VolumeMonitor, "mount-added", G_CALLBACK( MountAdded ), this );
        m_MountPreUnmountId = g_signal_connect( m_VolumeMonitor, "mount-pre-unmount", G_CALLBACK( MountRemoved ), this );
        m_MountRemovedId = g_signal_connect( m_VolumeMonitor, "mount-removed", G_CALLBACK( MountRemoved ), this );
    }

    GetCurrentMounts();
}

// -------------------------------------------------------------------------------- //
guGIO_VolumeMonitor::~guGIO_VolumeMonitor( void )
{
    guLogMessage( wxT( "Destroying the volume monitor object..." ) );
    if( m_VolumeMonitor )
    {
		g_signal_handler_disconnect( m_VolumeMonitor, m_VolumeAddedId );
		g_signal_handler_disconnect( m_VolumeMonitor, m_VolumeRemovedId );
		g_signal_handler_disconnect( m_VolumeMonitor, m_MountRemovedId );
		g_signal_handler_disconnect( m_VolumeMonitor, m_MountPreUnmountId );
		g_signal_handler_disconnect( m_VolumeMonitor, m_MountAddedId );

        while( m_MountedVolumes->Count() )
        {
            delete ( * m_MountedVolumes )[ 0 ];
            m_MountedVolumes->RemoveAt( 0 );
        }
        if( m_MountedVolumes )
        {
            delete m_MountedVolumes;
        }

        guLogMessage( wxT( ">> ~guGIO_VolumeMonitor()" ) );
        g_object_unref( m_VolumeMonitor );
        guLogMessage( wxT( "<< ~guGIO_VolumeMonitor()" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guGIO_VolumeMonitor::OnMountAdded( GMount * mount )
{
    guLogMessage( wxT( "Mount Added..." ) );
    if( FindMount( mount ) == wxNOT_FOUND )
    {
        if( g_mount_is_shadowed( mount ) )
        {
            //g_object_unref( mount );
            guLogMessage( wxT( "ignored shadowed mount" ) );
            return;
        }

        GVolume * Volume = g_mount_get_volume( mount );
        if( !Volume )
        {
            guLogMessage( wxT( "mount without volume?" ) );
            //g_object_unref( mount );
            return;
        }
        g_object_unref( Volume );

        GFile * MountRoot = g_mount_get_root( mount );
        if( MountRoot )
        {
            char * mount_path = g_file_get_path( MountRoot );
            if( mount_path )
            {
                wxString MountPath = wxString( mount_path, wxConvUTF8 );
                guGIO_Mount * Mount = new guGIO_Mount( mount, MountPath );
                if( Mount )
                {
                    m_MountedVolumes->Add( Mount );

                    wxCommandEvent event( wxEVT_MENU, ID_VOLUMEMANAGER_MOUNT_CHANGED );
                    event.SetInt( 1 );
                    wxPostEvent( m_MainFrame, event );
                }

                g_free( mount_path );
            }

            g_object_unref( MountRoot );
        }
        else
        {
            guLogMessage( wxT( "ignored mount without mount root" ) );
            g_object_unref( mount );
            return;
        }
    }
    else
    {
        guLogMessage( wxT( "Mount already added?" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guGIO_VolumeMonitor::OnMountRemoved( GMount * mount )
{
    guLogMessage( wxT( "Mount Removed..." ) );
    int MountIndex;
    if( ( MountIndex = FindMount( mount ) ) != wxNOT_FOUND )
    {
        //guGIO_Mount * Mount = m_MountedVolumes->Item( MountIndex );
        m_MountedVolumes->RemoveAt( MountIndex );

        wxCommandEvent event( wxEVT_MENU, ID_VOLUMEMANAGER_MOUNT_CHANGED );
        event.SetClientData( ( void * ) mount );
        event.SetInt( 0 );
        wxPostEvent( m_MainFrame, event );
        //guLogMessage( wxT( "Posted mount changed event..." ) );
    }
}

// -------------------------------------------------------------------------------- //
void guGIO_VolumeMonitor::CheckAudioCDVolume( GVolume * volume, const bool adding )
{
    GFile * activation_root = g_volume_get_activation_root( volume );
    if( activation_root )
    {
        char * root_uri = g_file_get_uri( activation_root );
        if( root_uri )
        {
            wxString VolumeUri = wxString::FromUTF8( root_uri );
            guLogMessage( wxT( "Uri: %s" ), VolumeUri.c_str() );
            if( VolumeUri.StartsWith( "cdda" ) ) // If it is a audio cd
            {
                wxCommandEvent event( wxEVT_MENU, ID_VOLUMEMANAGER_AUDIOCD_CHANGED );
                event.SetInt( adding );
                wxPostEvent( m_MainFrame, event );
            }
            g_free( root_uri );
        }
        g_object_unref( activation_root );
    }
}

// -------------------------------------------------------------------------------- //
void guGIO_VolumeMonitor::OnVolumeAdded( GVolume * volume )
{
    guLogMessage( wxT( "Volume Added..." ) );
    CheckAudioCDVolume( volume, true );
}

// -------------------------------------------------------------------------------- //
void guGIO_VolumeMonitor::OnVolumeRemoved( GVolume * volume )
{
    guLogMessage( wxT( "Volume Removed..." ) );
    CheckAudioCDVolume( volume, false );
}

// -------------------------------------------------------------------------------- //
int guGIO_VolumeMonitor::FindMount( GMount * mount )
{
    int Count = m_MountedVolumes->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( ( * m_MountedVolumes )[ Index ]->IsMount( mount ) )
            return Index;
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
wxArrayString guGIO_VolumeMonitor::GetMountNames( void )
{
    wxArrayString RetVal;
    int Count = m_MountedVolumes->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guGIO_Mount * Mount = m_MountedVolumes->Item( Index );
        guLogMessage( wxT( "... '%s'" ), Mount->GetName().c_str() );
        RetVal.Add( Mount->GetName() );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guGIO_VolumeMonitor::GetCurrentMounts( void )
{
    GList * Mounts = g_volume_monitor_get_mounts( m_VolumeMonitor );
    if( Mounts )
    {
        g_list_foreach( Mounts, GFunc( append_mount ), this );
        g_list_free( Mounts );
    }
}

// -------------------------------------------------------------------------------- //
guGIO_Mount * guGIO_VolumeMonitor::GetMountById( const wxString &id )
{
    int Count = m_MountedVolumes->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guGIO_Mount * Mount = ( * m_MountedVolumes )[ Index ];
        if( Mount->GetId() == id )
            return Mount;
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guGIO_Mount * guGIO_VolumeMonitor::GetMountByPath( const wxString &path )
{
    int Count = m_MountedVolumes->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guGIO_Mount * Mount = ( * m_MountedVolumes )[ Index ];
        if( Mount->GetMountPath() == path )
            return Mount;
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
guGIO_Mount * guGIO_VolumeMonitor::GetMountByName( const wxString &name )
{
    int Count = m_MountedVolumes->Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        guGIO_Mount * Mount = ( * m_MountedVolumes )[ Index ];
        if( Mount->GetName() == name )
            return Mount;
    }
    return NULL;
}

}

// -------------------------------------------------------------------------------- //
