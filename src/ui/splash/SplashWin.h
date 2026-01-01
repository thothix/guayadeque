/*
   Copyright (C) 2008-2023 J.Rios <anonbeat@gmail.com>
   Copyright (C) 2024-2026 Tiago T Barrionuevo <thothix@protonmail.com>

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
#ifndef __SPLASHWIN_H__
#define __SPLASHWIN_H__

#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/hyperlink.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/timer.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guSplashFrame : public wxFrame
{
  private:

  protected:
	wxHyperlinkCtrl *   m_Email;
	wxHyperlinkCtrl *   m_Thanks;
	wxStaticText *      m_Version;
    //wxHyperlinkCtrl *   m_HomePage;
    wxHyperlinkCtrl *   m_Donate;
    wxBitmap *          m_Bitmap;
    wxTimer             m_Timer;

	// event handlers, overide them in your derived class
	void OnSplashClick( wxMouseEvent& event );
    void OnEraseBackground( wxEraseEvent &event );

  public:
	guSplashFrame( wxWindow * parent, int timeout = 4000 ); //, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,358 ), long style = 0|wxTAB_TRAVERSAL );
	~guSplashFrame();

    void OnCloseWindow( wxCloseEvent &event );
    void OnTimeout( wxTimerEvent &event );
};

}

#endif
// -------------------------------------------------------------------------------- //
