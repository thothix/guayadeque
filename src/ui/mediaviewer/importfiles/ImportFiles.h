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
#ifndef __IMPORTFILES_H__
#define __IMPORTFILES_H__

#include "DbLibrary.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/filepicker.h>
#include <wx/listbox.h>
#include <wx/statbox.h>
#include <wx/dialog.h>

namespace Guayadeque {

class guMediaViewer;

// -------------------------------------------------------------------------------- //
class guImportFiles : public wxDialog
{
  protected :
    guMediaViewer *     m_MediaViewer;
    guTrackArray *      m_Tracks;

    wxChoice *          m_CopyToChoice;
    wxBitmapButton *    m_CopyToSetupBtn;
    wxDirPickerCtrl *   m_DestPathDirPicker;
    wxListBox *         m_FilesListBox;
    wxBitmapButton *    m_AddFilesBtn;
    wxBitmapButton *    m_DelFilesBtn;

    wxStaticText *      m_FilesLabel;

    wxButton *          m_DlgButtonsOK;

    void                CreateControls();
    void                OnConfigUpdated( wxCommandEvent &event );
    void                OnCopyToSetupClicked( wxCommandEvent &event );
	void                OnFileSelected( wxCommandEvent &event );
	void                OnAddFilesClicked( wxCommandEvent &event );
	void                OnDelFilesClicked( wxCommandEvent &event );

	void                CheckButtons();
	void                UpdateCounters();

  public :
    guImportFiles( wxWindow * parent, guMediaViewer * mediaviewer, guTrackArray * tracks );
    ~guImportFiles();

    wxString GetCopyToOption() { return m_CopyToChoice->GetStringSelection(); }
    wxString GetCopyToPath() { return m_DestPathDirPicker->GetPath(); }
};

}

#endif
