/*
   Copyright (C) 2008-2023 J.Rios <anonbeat@gmail.com>
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
#ifndef __LABELEDITOR_H__
#define __LABELEDITOR_H__

#include "DbLibrary.h"
#include "DbRadios.h"

#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/panel.h>
#include <wx/checklst.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/splitter.h>
#include <wx/dialog.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// Class guLabelEditor
// -------------------------------------------------------------------------------- //
class guLabelEditor : public wxDialog
{
  protected:
    guDbLibrary *       m_Db;
    guDbRadios *        m_RaDb;
    guListItems         m_Labels;
    guArrayListItems *  m_LabelSets;
    int                 m_SelectedItem;
    int                 m_SelectedLabel;
    bool                m_IsRadioLabel;

    wxSplitterWindow *  m_Splitter;
    wxPanel *           m_ItemsPanel;
    wxListBox *         m_ItemsListBox;
    wxPanel *           m_LabelsPanel;
    wxCheckListBox *    m_LabelsListBox;

    wxBitmapButton *    m_AddButton;
    wxBitmapButton *    m_DelButton;
    wxBitmapButton *    m_CopyButton;

    void                OnItemSelected( wxCommandEvent& event );
    void                OnLabelSelected( wxCommandEvent& event );
    void                OnLabelChecked( wxCommandEvent& event );
    void                OnLabelDoubleClicked( wxCommandEvent& event );
    void                OnAddLabelClicked( wxCommandEvent& event );
    void                OnDelLabelClicked( wxCommandEvent& event );
    void                OnCopyLabelsClicked( wxCommandEvent& event );

    void                OnIdle( wxIdleEvent &event );

    void                ClearCheckedItems( void );
    void                CheckLabelItems( const wxArrayInt &checkeditems );

    void                AddToAllItems( const int labelid );
    void                DelToAllItems( const int labelid );

  public:
	guLabelEditor( wxWindow * parent, guDbLibrary * db, const wxString &title,
	    const bool isradiolabel, const guListItems * items, guArrayListItems * labelsets );
	~guLabelEditor();

};

}

#endif
// -------------------------------------------------------------------------------- //
