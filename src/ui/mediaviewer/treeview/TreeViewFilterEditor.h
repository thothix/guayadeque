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
#ifndef __TREEVIEWFILTEREDITOR_H__
#define __TREEVIEWFILTEREDITOR_H__

// -------------------------------------------------------------------------------- //
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/dialog.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// Class guTreeViewFilterEditor
// -------------------------------------------------------------------------------- //
class guTreeViewFilterEditor : public wxDialog
{
  protected :
    wxTextCtrl  *       m_NameTextCtrl;
    wxListBox *         m_FiltersListBox;
    wxBitmapButton *    m_UpFilterButton;
    wxBitmapButton *    m_DownFilterButton;
    wxBitmapButton *    m_DelFilterButton;
    wxChoice *          m_FiltersChoice;
    wxBitmapButton *    m_AddFilterButton;
    wxButton *          m_AcceptButton;

    int                 m_CurrentItem;
    wxArrayInt          m_FilterItems;

    virtual void        OnFilterListBoxSelected( wxCommandEvent& event );
    virtual void        OnUpFilterBtnClick( wxCommandEvent& event );
    virtual void        OnDownFilterBtnClick( wxCommandEvent& event );
    virtual void        OnDelFilterBtnClick( wxCommandEvent& event );
    virtual void        OnAddFilterBtnClick( wxCommandEvent& event );
    virtual void        OnCheckAcceptButton( wxCommandEvent& event );

  public :
    guTreeViewFilterEditor( wxWindow * parent, const wxString &filterentry );
    ~guTreeViewFilterEditor();

    wxString            GetTreeViewFilterEntry( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
