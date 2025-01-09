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
#include "UserRadio.h"

#include "Accelerators.h"
#include "DbRadios.h"
#include "Images.h"
#include "RadioPanel.h"
#include "RadioEditor.h"

#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guUserRadioProvider::guUserRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios ) :
    guRadioProvider( radiopanel, dbradios )
{
    radiopanel->Bind( wxEVT_MENU, &guUserRadioProvider::OnRadioAdd, this, ID_RADIO_USER_ADD );
    radiopanel->Bind( wxEVT_MENU, &guUserRadioProvider::OnRadioEdit, this, ID_RADIO_USER_EDIT );
    radiopanel->Bind( wxEVT_MENU, &guUserRadioProvider::OnRadioDelete, this, ID_RADIO_USER_DEL );
    radiopanel->Bind( wxEVT_MENU, &guUserRadioProvider::OnRadioImport, this, ID_RADIO_USER_IMPORT );
    radiopanel->Bind( wxEVT_MENU, &guUserRadioProvider::OnRadioExport, this, ID_RADIO_USER_EXPORT );
}

// -------------------------------------------------------------------------------- //
guUserRadioProvider::~guUserRadioProvider() {}

// -------------------------------------------------------------------------------- //
bool guUserRadioProvider::OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations, const int selcount )
{
    if( selcount )
        menu->AppendSeparator();

    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_RADIO_USER_ADD, _( "Add Radio" ), _( "Create a new radio" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    menu->Append( MenuItem );

    if( forstations && selcount )
    {
        MenuItem = new wxMenuItem( menu, ID_RADIO_USER_EDIT,
                        wxString( _( "Edit Radio" ) ) + guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_EDITTRACKS ),
                        _( "Change the selected radio" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        menu->Append( MenuItem );

        MenuItem = new wxMenuItem( menu, ID_RADIO_USER_DEL, _( "Delete Radio" ), _( "Delete the selected radio" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        menu->Append( MenuItem );
    }

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_RADIO_USER_IMPORT, _( "Import" ), _( "Import the radio stations" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    menu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_RADIO_USER_EXPORT, _( "Export" ), _( "Export all the radio stations" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    menu->Append( MenuItem );

    return true;
}

// -------------------------------------------------------------------------------- //
void guUserRadioProvider::RegisterImages( wxImageList * imagelist )
{
    imagelist->Add( guImage( guIMAGE_INDEX_tiny_net_radio ) );
    m_ImageIds.Add( imagelist->GetImageCount() - 1 );
}

// -------------------------------------------------------------------------------- //
void guUserRadioProvider::RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem )
{
    m_ManualId = genretreectrl->AppendItem( rootitem, _( "User Defined" ), m_ImageIds[ 0 ], m_ImageIds[ 0 ], NULL );
}

// -------------------------------------------------------------------------------- //
int guUserRadioProvider::GetStations( guRadioStations * stations, const long minbitrate )
{
    m_Db->SetRadioSourceFilter( guRADIO_SOURCE_USER );
    m_Db->GetRadioStations( stations );

    m_RadioPanel->EndLoadingStations();

    return stations->Count();
}

// -------------------------------------------------------------------------------- //
void guUserRadioProvider::OnRadioAdd( wxCommandEvent &event )
{
    wxArrayString * Params = ( wxArrayString * ) event.GetClientData();
    wxString Name;
    wxString Link;
    if( Params && ( Params->GetCount() == 2 ) )
    {
        Name = Params->Item( 0 );
        Link = Params->Item( 1 );
    }

    guRadioEditor * RadioEditor = new guRadioEditor( m_RadioPanel, _( "Edit Radio" ), Name, Link );
    if( RadioEditor )
    {
        if( RadioEditor->ShowModal() == wxID_OK )
        {
            guRadioStation RadioStation;
            RadioStation.m_Id = wxNOT_FOUND;
            RadioStation.m_SCId = wxNOT_FOUND;
            RadioStation.m_BitRate = 0;
            RadioStation.m_GenreId = wxNOT_FOUND;
            RadioStation.m_Source = 1;
            RadioStation.m_Name = RadioEditor->GetName();
            RadioStation.m_Link = RadioEditor->GetLink();
            RadioStation.m_Listeners = 0;
            RadioStation.m_Type = wxEmptyString;
            //
            m_Db->SetRadioStation( &RadioStation );
            //
            m_RadioPanel->ReloadStations();
        }
        RadioEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guUserRadioProvider::OnRadioEdit( wxCommandEvent &event )
{
    guRadioStation RadioStation;
    m_RadioPanel->GetSelectedStation( &RadioStation );

    guRadioEditor * RadioEditor = new guRadioEditor( m_RadioPanel, _( "Edit Radio" ), RadioStation.m_Name, RadioStation.m_Link );
    if( RadioEditor )
    {
        if( RadioEditor->ShowModal() == wxID_OK )
        {
            RadioStation.m_Name = RadioEditor->GetName();
            RadioStation.m_Link = RadioEditor->GetLink();
            m_Db->SetRadioStation( &RadioStation );
            m_RadioPanel->ReloadStations();
        }
        RadioEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guUserRadioProvider::OnRadioDelete( wxCommandEvent &event )
{
    guRadioStation RadioStation;
    m_RadioPanel->GetSelectedStation( &RadioStation );
    m_Db->DelRadioStation( RadioStation.m_Id );
    m_RadioPanel->ReloadStations();
}

// -------------------------------------------------------------------------------- //
void ReadXmlRadioStation( wxXmlNode * node, guRadioStation * station )
{
    while( node )
    {
        if( node->GetName() == wxT( "Name" ) )
            station->m_Name = node->GetNodeContent();
        else if( node->GetName() == wxT( "Url" ) )
            station->m_Link = node->GetNodeContent();

        node = node->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlRadioStations( wxXmlNode * node, guRadioStations * stations )
{
    while( node && node->GetName() == wxT( "RadioStation" ) )
    {
        guRadioStation * RadioStation = new guRadioStation();

        RadioStation->m_Id = wxNOT_FOUND;
        RadioStation->m_SCId = wxNOT_FOUND;
        RadioStation->m_BitRate = 0;
        RadioStation->m_GenreId = wxNOT_FOUND;
        RadioStation->m_Source = 1;
        RadioStation->m_Listeners = 0;
        RadioStation->m_Type = wxEmptyString;

        ReadXmlRadioStation( node->GetChildren(), RadioStation );

        if( !RadioStation->m_Name.IsEmpty() && !RadioStation->m_Link.IsEmpty() )
            stations->Add( RadioStation );
        else
            delete RadioStation;

        node = node->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guUserRadioProvider::OnRadioImport( wxCommandEvent &event )
{
    guRadioStations UserStations;

    wxFileDialog * FileDialog = new wxFileDialog( m_RadioPanel,
        wxT( "Select the xml file" ),
        wxGetHomeDir(),
        wxEmptyString,
        wxT( "*.xml;*.xml" ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST );

    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            wxFileInputStream Ins( FileDialog->GetPath() );
            wxXmlDocument XmlDoc( Ins );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName() == wxT( "RadioStations" ) )
            {
                ReadXmlRadioStations( XmlNode->GetChildren(), &UserStations );
                int Count = UserStations.Count();
                if( Count )
                {
                    for( int Index = 0; Index < Count; Index++ )
                    {
                        m_Db->SetRadioStation( &UserStations[ Index ] );
                    }
                    //
                    m_RadioPanel->ReloadStations();
                }
            }
        }
        FileDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guUserRadioProvider::OnRadioExport( wxCommandEvent &event )
{
    guRadioStations UserStations;
    m_Db->GetUserRadioStations( &UserStations );
    int Count = UserStations.Count();
    if( Count )
    {
        wxFileDialog * FileDialog = new wxFileDialog( m_RadioPanel,
            wxT( "Select the output xml filename" ),
            wxGetHomeDir(),
            wxT( "RadioStations.xml" ),
            wxT( "*.xml;*.xml" ),
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

        if( FileDialog )
        {
            if( FileDialog->ShowModal() == wxID_OK )
            {
                wxXmlDocument OutXml;
                //OutXml.SetRoot(  );
                wxXmlNode * RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "RadioStations" ) );

                for( int Index = 0; Index < Count; Index++ )
                {
                    //guLogMessage( wxT( "Adding %s" ), UserStations[ Index ].m_Name.c_str() );
                    wxXmlNode * RadioNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "RadioStation" ) );

                    wxXmlNode * RadioName = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "Name" ) );
                    wxXmlNode * RadioNameVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "Name" ), UserStations[ Index ].m_Name );
                    RadioName->AddChild( RadioNameVal );
                    RadioNode->AddChild( RadioName );

                    wxXmlNode * RadioUrl = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "Url" ) );
                    wxXmlNode * RadioUrlVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "Url" ), UserStations[ Index ].m_Link );
                    RadioUrl->AddChild( RadioUrlVal );
                    RadioNode->AddChild( RadioUrl );

                    RootNode->AddChild( RadioNode );
                }
                OutXml.SetRoot( RootNode );
                OutXml.Save( FileDialog->GetPath() );
            }
            FileDialog->Destroy();
        }
    }
}

}

// -------------------------------------------------------------------------------- //
