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
#include "AuiManagerPanel.h"

#include "AuiDockArt.h"
#include "MainFrame.h"
#include "Settings.h"
#include "Utils.h"

#include <wx/settings.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guAuiManagerPanel::guAuiManagerPanel( wxWindow * parent ) :
    wxPanel( parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER )
{
    m_MenuBar = nullptr;

    m_AuiManager.SetManagedWindow( this );
    m_AuiManager.SetArtProvider( new guAuiDockArt() );
    m_AuiManager.SetFlags( wxAUI_MGR_ALLOW_FLOATING |
                           wxAUI_MGR_TRANSPARENT_DRAG |
                           wxAUI_MGR_TRANSPARENT_HINT );

    wxAuiDockArt * AuiDockArt = m_AuiManager.GetArtProvider();

    wxColour BaseColor = guCOLOR_BASE;
    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR,      guCOLOR_CAPTION_TEXT_INACTIVE );
    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR,        guCOLOR_CAPTION_TEXT_ACTIVE );
    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR,             guCOLOR_CAPTION_ACTIVE );
    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR,    guCOLOR_CAPTION_GRADIENT_ACTIVE );
    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR,           guCOLOR_CAPTION_INACTIVE );
    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR,  guCOLOR_CAPTION_INACTIVE );
    AuiDockArt->SetColour( wxAUI_DOCKART_SASH_COLOUR,                       guCOLOR_SASH );

    AuiDockArt->SetMetric( wxAUI_DOCKART_CAPTION_SIZE,                      guSIZE_CAPTION );
    AuiDockArt->SetMetric( wxAUI_DOCKART_PANE_BORDER_SIZE,                  guSIZE_BORDER );
    AuiDockArt->SetMetric( wxAUI_DOCKART_SASH_SIZE,                         guSIZE_SASH );
    AuiDockArt->SetMetric( wxAUI_DOCKART_GRADIENT_TYPE,                     guGRADIENT_TYPE );

    m_AuiManager.Bind( wxEVT_AUI_PANE_CLOSE, &guAuiManagerPanel::OnPaneClose, this );
}

// -------------------------------------------------------------------------------- //
guAuiManagerPanel::~guAuiManagerPanel()
{
    m_AuiManager.Unbind( wxEVT_AUI_PANE_CLOSE, &guAuiManagerPanel::OnPaneClose, this );
    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::ShowPanel( const int panelid, bool show )
{
    int PanelIndex = m_PanelIds.Index( panelid );
    if ( PanelIndex != wxNOT_FOUND )
    {
        wxString PaneName = m_PanelNames[ PanelIndex ];

        wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( PaneName );
        if ( PaneInfo.IsOk() )
        {
            if( show )
                PaneInfo.Show();
            else
                PaneInfo.Hide();

            m_AuiManager.Update();
        }

        if ( show )
            m_VisiblePanels |= panelid;
        else
            m_VisiblePanels ^= panelid;

        if ( !m_MenuBar )
        {
            guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
            m_MenuBar = MainFrame->GetMenuBar();
        }
        if ( m_MenuBar )
        {
            wxMenuItem * MenuItem = m_MenuBar->FindItem( m_PanelCmdIds[ PanelIndex ] );
            if ( MenuItem )
                MenuItem->Check( show );
        }

        guLogMessage( wxT( "Id: %i Pane: %s Show:%i  Flags:%08X" ), panelid, PaneName.c_str(), show, m_VisiblePanels );
    }
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::OnPaneClose( wxAuiManagerEvent &event )
{
    wxAuiPaneInfo * PaneInfo = event.GetPane();
    int PanelIndex = m_PanelNames.Index( PaneInfo->name );
    if ( PanelIndex != wxNOT_FOUND )
    {
        guLogMessage( wxT( "OnPaneClose: %s  %i" ), m_PanelNames[ PanelIndex ].c_str(), m_PanelCmdIds[ PanelIndex ] );
        ShowPanel( m_PanelIds[ PanelIndex ], false );
    }

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::LoadPerspective( const wxString &layoutstr, const unsigned int visiblepanels )
{
    int Count = m_PanelIds.Count();
    for ( int Index = 0; Index < Count; Index++ )
    {
        int PanelId = m_PanelIds[ Index ];
        if ( ( visiblepanels & PanelId ) != ( m_VisiblePanels & PanelId ) )
            ShowPanel( PanelId, ( visiblepanels & PanelId ) );
    }

    m_AuiManager.LoadPerspective( layoutstr, true );
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::SaveLayout( wxXmlNode * xmlnode, const wxString &name )
{
    auto * XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, name );

    wxXmlAttribute * Property = new wxXmlAttribute(
            wxT( "panels" ),
            wxString::Format( wxT( "%d" ), VisiblePanels() ),
            new wxXmlAttribute( wxT( "layout" ), SavePerspective(), NULL ) );

    XmlNode->SetAttributes( Property );

    wxXmlNode * Columns = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "columns" ) );
    int Count = GetListViewColumnCount();
    for ( int Index = 0; Index < Count; Index++ )
    {
        int  ColumnPos;
        int  ColumnWidth;
        bool ColumnEnabled;

        wxXmlNode * Column = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "column" ) );

        GetListViewColumnData( Index, &ColumnPos, &ColumnWidth, &ColumnEnabled );

        Property = new wxXmlAttribute( wxT( "id" ), wxString::Format( wxT( "%d" ), Index ),
                   new wxXmlAttribute( wxT( "pos" ), wxString::Format( wxT( "%d" ), ColumnPos ),
                   new wxXmlAttribute( wxT( "width" ), wxString::Format( wxT( "%d" ), ColumnWidth ),
                   new wxXmlAttribute( wxT( "enabled" ), wxString::Format( wxT( "%d" ), ColumnEnabled ),
                   NULL ) ) ) );
        Column->SetAttributes( Property );
        Columns->AddChild( Column );
    }

    XmlNode->AddChild( Columns );
    xmlnode->AddChild( XmlNode );
}

// -------------------------------------------------------------------------------- //
void guAuiManagerPanel::LoadLayout( wxXmlNode * xmlnode )
{
    long VisiblePanels;
    wxString Field, LayoutStr;

    xmlnode->GetAttribute( wxT( "panels" ), &Field );
    Field.ToLong( &VisiblePanels );
    xmlnode->GetAttribute( wxT( "layout" ), &LayoutStr );

    wxXmlNode * Columns = xmlnode->GetChildren();
    if ( Columns && ( Columns->GetName() == wxT( "columns" ) ) )
    {
        wxXmlNode * Column = Columns->GetChildren();
        while ( Column && ( Column->GetName() == wxT( "column" ) ) )
        {
            long ColumnId;
            long ColumnPos;
            long ColumnWidth;
            long ColumnEnabled;

            Column->GetAttribute( wxT( "id" ), &Field );
            Field.ToLong( &ColumnId );
            Column->GetAttribute( wxT( "pos" ), &Field );
            Field.ToLong( &ColumnPos );
            Column->GetAttribute( wxT( "width" ), &Field );
            Field.ToLong( &ColumnWidth );
            Column->GetAttribute( wxT( "enabled" ), &Field );
            Field.ToLong( &ColumnEnabled );

            SetListViewColumnData( ColumnId, ColumnPos, ColumnWidth, ColumnEnabled );

            Column = Column->GetNext();
        }
    }

    LoadPerspective( LayoutStr, VisiblePanels );
}

}
