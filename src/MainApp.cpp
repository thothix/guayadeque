// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
//	anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2, or (at your option)
//    any later version.
//
//    This Program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; see the file LICENSE.  If not, write to
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "MainApp.h"

#include "Images.h"
#include "MainFrame.h"
#include "Config.h"
#include "Utils.h"
#include "mpris.h"

#include "taglib-extras/tplugins.h"

#include "wx/clipbrd.h"
#include <wx/curl/base.h>
#include <wx/image.h>
#include <wx/tooltip.h>
#include <wx/stdpaths.h>

IMPLEMENT_APP(guMainApp);

// -------------------------------------------------------------------------------- //
guMainApp::guMainApp() : wxApp()
{
    if( !wxDirExists( wxGetHomeDir() + wxT( "/.guayadeque" ) ) )
    {
        wxMkdir( wxGetHomeDir() + wxT( "/.guayadeque" ), 0770 );
        guLogMessage( wxT( "Created the configuration directory" ) );
    }
    if( !wxFileExists( wxGetHomeDir() + wxT( "/.guayadeque/guayadeque.conf" ) ) )
    {
        if( wxFileExists( wxT( "/usr/share/guayadeque/guayadeque.default.conf" ) ) )
        {
            wxCopyFile( wxT( "/usr/share/guayadeque/guayadeque.default.conf" ),
                        wxGetHomeDir() + wxT( "/.guayadeque/guayadeque.conf" ), false );
        }
        else if( wxFileExists( wxT( "/usr/local/share/guayadeque/guayadeque.default.conf" ) ) )
        {
            wxCopyFile( wxT( "/usr/local/share/guayadeque/guayadeque.default.conf" ),
                        wxGetHomeDir() + wxT( "/.guayadeque/guayadeque.conf" ), false );
        }
        guLogMessage( wxT( "Created the default configuration file" ) );
    }


    m_Config = new guConfig();
    guConfig::Set( m_Config );
}

// -------------------------------------------------------------------------------- //
guMainApp::~guMainApp()
{
    // config
    if( m_Config )
      delete m_Config;

    if( m_SingleInstanceChecker )
        delete m_SingleInstanceChecker;
}


// -------------------------------------------------------------------------------- //
bool SendFilesByMPRIS( const int argc, wxChar * argv[] )
{
    DBusError dberr;
    DBusConnection * dbconn;
    DBusMessage * dbmsg, * dbreply;
    DBusMessageIter dbiter;

    dbus_error_init( &dberr );
    dbconn = dbus_bus_get( DBUS_BUS_SESSION, &dberr );

    if( dbus_error_is_set( &dberr ) )
    {
         printf( "getting session bus failed: %s\n", dberr.message );
         dbus_error_free( &dberr );
         return false;
    }

    dbmsg = dbus_message_new_method_call( GUAYADEQUE_MPRIS_SERVICENAME,
                                          GUAYADEQUE_MPRIS_TRACKLIST_PATH,
                                          GUAYADEQUE_MPRIS_INTERFACE,
                                          "AddTrack" );
    if( dbmsg == NULL )
    {
         guLogError( wxT( "Couldn’t create a DBusMessage" ) );
         return false;
    }

    wxString FilePath;
    bool PlayTrack = false;
    int index;
    for( index = 1; index < argc; index++ )
    {
        FilePath = argv[ index ];
        //guLogMessage( wxT( "Trying to add file '%s'" ), argv[ index ] );

        dbus_message_iter_init_append( dbmsg, &dbiter );

        dbus_message_iter_append_basic( &dbiter, DBUS_TYPE_STRING, &FilePath.char_str() );
        dbus_message_iter_append_basic( &dbiter, DBUS_TYPE_BOOLEAN, &PlayTrack );

        dbus_error_init( &dberr );

        dbreply = dbus_connection_send_with_reply_and_block( dbconn, dbmsg, 5000, &dberr );
        if( dbus_error_is_set( &dberr ) )
        {
              guLogMessage( wxT( "Error adding file %s" ), FilePath.c_str() );
              printf( "Error getting a reply: %s\n", dberr.message );
              dbus_message_unref( dbmsg );
              dbus_error_free( &dberr );
              return false;
        }

        dbus_message_unref( dbreply );

        /* Don’t need this anymore */
        dbus_message_unref( dbmsg );
    }

    dbus_connection_close( dbconn );
    dbus_connection_unref( dbconn );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMainApp::OnInit()
{
    guRandomInit();

    wxLog::SetActiveTarget( new wxLogStderr );

    const wxString AppName = wxString::Format( wxT( "guayadeque-%s" ), wxGetUserId().c_str() );
    m_SingleInstanceChecker = new wxSingleInstanceChecker( AppName );
    if( m_SingleInstanceChecker->IsAnotherRunning() )
    {
        if( argc > 1 )
        {
            wxMilliSleep( 100 );
            int RetryCnt = 0;
            while( RetryCnt < 20 )
            {
                if( SendFilesByMPRIS( argc, argv ) )
                    break;
                wxMilliSleep( 100 );
            }
        }

        guLogError( wxT( "Another program instance is already running, aborting." ) );
        return false;
    }

    // Use the primary clipboard which is shared with other applications
    wxTheClipboard->UsePrimarySelection( false );


    // Init all image handlers
    wxInitAllImageHandlers();

    // Init the wxCurl Lib
    wxCurlBase::Init();

    // Register the asf plugin for taglib
    registerTaglibPlugins();


    //
    if( m_Locale.Init( wxLANGUAGE_DEFAULT,
                     /*wxLOCALE_LOAD_DEFAULT |*/ wxLOCALE_CONV_ENCODING ) )
    {
        m_Locale.AddCatalogLookupPathPrefix( wxT( "/usr/share/locale" ) );
        m_Locale.AddCatalog( wxT( "guayadeque" ) );
        guLogMessage( wxT( "Initialized locale ( %s )" ), m_Locale.GetName().c_str() );
    }
    else
    {
        int LangId = wxLocale::GetSystemLanguage();
        const wxLanguageInfo * LangInfo = wxLocale::GetLanguageInfo( LangId );
        if( LangInfo )
        {
            guLogError( wxT( "Could not initialize the translations engine for ( %s )" ), LangInfo->CanonicalName.c_str() );
        }
        else
        {
            guLogError( wxT( "Could not initialize the translations engine for (%d)" ), LangId );
        }
        wxStandardPaths StdPaths;
        guLogError( wxT( "Locale directory '%s'" ), StdPaths.GetLocalizedResourcesDir( wxT( "es_ES" ), wxStandardPaths::ResourceCat_Messages).c_str() );
    }

    // Enable tooltips
    wxToolTip::Enable( true );


    // If enabled Show the Splash Screen on Startup
    guSplashFrame * SplashFrame = NULL;
    if( m_Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "General" ) ) )
    {
        SplashFrame = new guSplashFrame( 0 );
        if( !SplashFrame )
            guLogError( wxT( "Could not create splash object" ) );
        SplashFrame->Show( true );
        wxYield();
    }

    // Initialize the MainFrame object
    guMainFrame* Frame = new guMainFrame( 0 );
    wxIcon MainIcon;
    MainIcon.CopyFromBitmap( guImage( guIMAGE_INDEX_guayadeque ) );
    Frame->SetIcon( MainIcon );

    // If Minimize is enabled minimized or hide it if Taskbar Icon is enabled
    if( m_Config->ReadBool( wxT( "StartMinimized" ), false, wxT( "General" ) ) )
    {
        if( m_Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "General" ) ) &&
            m_Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) )
        {
            Frame->Show( false );
            //Frame->Hide();
        }
        else
        {
            Frame->Show();
            Frame->Iconize( true );
        }
    }
    else
    {
        Frame->Show();
    }

    SetTopWindow( Frame );

    return true;
}

// -------------------------------------------------------------------------------- //
int guMainApp::OnExit()
{
    // Shutdown the wxCurl Lib
    wxCurlBase::Shutdown();

    return 0;
}

// -------------------------------------------------------------------------------- //
