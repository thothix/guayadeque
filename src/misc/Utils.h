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
#ifndef _UTILS_H
#define _UTILS_H

#include <random>
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

#ifdef NDEBUG
    #define guLogDebug(...)
    #define guLogTrace(...) guLogMsgIfDebug( __VA_ARGS__ )
#else
    #define GU_DEBUG
    #define guLogDebug(...) guLogMsgIfDebug( __VA_ARGS__ )
    #define guLogTrace      wxLogMessage
#endif
#define guLogMessage    wxLogMessage
#define guLogWarning    wxLogWarning
#define guLogError      wxLogError

#define guDEFAULT_BROWSER_USER_AGENT    wxT( "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/55.0.2883.87 Chrome/55.0.2883.87 Safari/537.36" )

#if wxCHECK_VERSION(3, 2, 0)
    #define guENSURE_BITMAP(x) (x.GetBitmap(x.GetDefaultSize()))
#else
    #define guENSURE_BITMAP(x) (x)
#endif

#if GSTREAMER_VERSION == 120        // Version >= 1.20
    #define  guGST_ELEMENT_REQUEST_PAD_SIMPLE gst_element_request_pad_simple
#else
    #define  guGST_ELEMENT_REQUEST_PAD_SIMPLE gst_element_get_request_pad
#endif

#define guDESKTOP_MANAGERS  { wxT("gnome"), wxT("kde"), wxT("xfce"), wxT("lxqt") }

enum guFILEITEM_TYPE {
    guFILEITEM_TYPE_FOLDER = 0,
    guFILEITEM_TYPE_AUDIO,
    guFILEITEM_TYPE_IMAGE,
    guFILEITEM_TYPE_FILE
};

class guFileItem
{
public :
    int             m_Type;
    wxString        m_Name;
    wxFileOffset    m_Size;
    int             m_Time;
};
WX_DECLARE_OBJARRAY(guFileItem, guFileItemArray);

class guTrackArray;
class guMediaViewer;
class guTrack;

bool                IsColorDark( const wxColour &color );
wxString            LenToString( wxUint64 len );
wxString            SizeToString( wxFileOffset size );
wxArrayString       guSplitWords( const wxString &InputStr );
wxImage *           guGetRemoteImage( const wxString &url, wxBitmapType &imgtype );
bool                DownloadImage( const wxString &source, const wxString &target, const wxBitmapType imagetype, int maxwidth, int maxheight );
bool                DownloadImage( const wxString &source, const wxString &taget, int maxwidth = -1, int maxheight = -1 );
int                 DownloadFile( const wxString &Source, const wxString &Target );
wxString            RemoveSearchFilters( const wxString &Album );
bool                SearchCoverWords( const wxString &filename, const wxArrayString &Strings, const wxString &album_name = "" );
wxString            guURLEncode( const wxString &url, bool encodespace = true );
wxString            guFileDnDEncode( const wxString &file );
int                 guWebExecute( const wxString &Url );
int                 guExecute( const wxString &Command );
wxFileOffset        guGetFileSize( const wxString &filename );
wxString            GetUrlContent( const wxString &url, const wxString &referer = wxEmptyString, bool encoding = false );
void                CheckSymLinks( wxArrayString &libpaths );
bool                CheckFileLibPath( const wxArrayString &LibPaths, const wxString &filename );
int                 guGetFileMode( const wxString &filepath );
bool                guSetFileMode( const wxString &filepath, int mode, bool adding = false );
bool                guRenameFile( const wxString &oldname, const wxString &newname, bool overwrite = true );
wxString            guGetNextXMLChunk( wxFile &xmlfile, wxFileOffset &CurPos, const char * startstr, const char * endstr, const wxMBConv &conv = wxConvUTF8 );
wxString            guExpandTrackMacros( const wxString &pattern, guTrack * track, const int indexpos = 0 );
bool                guIsValidImageFile( const wxString &filename );
bool                guRemoveDir( const wxString &path );
void                GetMediaViewerTracks( const guTrackArray &sourcetracks, const wxArrayInt &sourceflags,
                                          const guMediaViewer * mediaviewer, guTrackArray &tracks, wxArrayInt &changedflags );
void                GetMediaViewerTracks( const guTrackArray &sourcetracks, const int flags,
                                          const guMediaViewer * mediaviewer, guTrackArray &tracks, wxArrayInt &changedflags );
void                GetMediaViewerTracks( const guTrackArray &sourcetracks, const guMediaViewer * mediaviewer, guTrackArray &tracks );
void                GetMediaViewersList( const guTrackArray &tracks, wxArrayPtrVoid &MediaViewerPtrs );
wxString            ExtractString( const wxString &source, const wxString &start, const wxString &end );
wxString            GetPathAddTrailSep(wxString path);
void                AddPathTrailSep(wxString &path);
wxString            GetPathRemoveTrailSep(wxString path);
void                RemovePathTrailSep(wxString &path);
int wxCMPFUNC_CONV CompareFileNameA( guFileItem ** item1, guFileItem ** item2 );
int wxCMPFUNC_CONV CompareFileNameD( guFileItem ** item1, guFileItem ** item2 );
int wxCMPFUNC_CONV CompareFileSizeA( guFileItem ** item1, guFileItem ** item2 );
int wxCMPFUNC_CONV CompareFileSizeD( guFileItem ** item1, guFileItem ** item2 );
int wxCMPFUNC_CONV CompareFileTimeA( guFileItem ** item1, guFileItem ** item2 );
int wxCMPFUNC_CONV CompareFileTimeD( guFileItem ** item1, guFileItem ** item2 );
int wxCMPFUNC_CONV CompareFileTypeA( guFileItem ** item1, guFileItem ** item2 );
int wxCMPFUNC_CONV CompareFileTypeD( guFileItem ** item1, guFileItem ** item2 );
wxString JoinFromArrayInt(const wxArrayInt &intArray, const wxChar &delimiter = ',');
wxArrayInt SplitToArrayInt(const wxString &data, const wxChar &delimiter = ',');
wxString GetSuperscriptNumber(int number);

std::mt19937        guSRandom();
extern std::mt19937 rng_default_generator;

// As the new CXX11 random generators needs an object to seed instead of simply initialize the library, we use a
// default object created below but you can create a new one in local scope, just use the guSRandom()
#define guRandomInit() (rng_default_generator = guSRandom())
#define guRandom(x)    (rng_default_generator() % x)

static bool         guDebugMode     = std::getenv( "GU_DEBUG" ) != nullptr;
static bool         guGatherStats   = std::getenv( "GU_STATS" ) != nullptr;

template<typename... Args>
static void guLogMsgIfDebug( Args... what )
{
    if( guDebugMode )
        guLogMessage( what... );
}

template<typename... Args>
static void guLogStats( Args... what )
{
    if( guGatherStats )
        guLogMessage( what... );
}

void inline guImageResize( wxImage * image, int maxsize, bool forceresize = false )
{
    int w = image->GetWidth();
    int h = image->GetHeight();

    double ratio = wxMin( static_cast<double>( maxsize ) / h,
                          static_cast<double>( maxsize ) / w );

    if( forceresize || ( ratio < 1 ) )
        image->Rescale( ( w * ratio ) + .5, ( h * ratio ) + .5, wxIMAGE_QUALITY_HIGH );
}

time_t inline GetFileLastChangeTime( const wxString &filename )
{
    wxStructStat St;
    if( wxStat( filename, &St ) )
        return -1;
    return St.st_ctime;
}

bool inline IsFileSymbolicLink( const wxString &filename )
{
    wxStructStat St;
    wxLstat( filename, &St );
    return S_ISLNK( St.st_mode );
}

}

#endif
