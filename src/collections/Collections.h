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
#ifndef __COLLECTIONS_H__
#define __COLLECTIONS_H__

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>
#include <wx/menu.h>

namespace Guayadeque {

#define GU_COLLECTION_DUMMY_ROOTDIR     "/$GUAYADEQUE$COLLECTION_DUMMY_ROOTDIR/"

#define guCOLLECTION_UPDATE_ON_START  "UpdateOnStart"
#define guCOLLECTION_DIRECTORY_PATH   "DirectoryPath"

enum guMediaCollectionType {
    guMEDIA_COLLECTION_TYPE_NORMAL,
    guMEDIA_COLLECTION_TYPE_JAMENDO_REMOVED,       // DONT REMOVE (for .config compatibility)
    guMEDIA_COLLECTION_TYPE_MAGNATUNE,
    guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE,
    guMEDIA_COLLECTION_TYPE_IPOD
};

// -------------------------------------------------------------------------------- //
class guMediaCollection
{
  public :
    wxString        m_UniqueId;
    int             m_Type;
    wxString        m_Name;
    wxArrayString   m_Paths;
    wxArrayString   m_CoverWords;
    bool            m_UpdateOnStart;
    bool            m_ScanPlaylists;
    bool            m_ScanFollowSymLinks;
    bool            m_ScanEmbeddedCovers;
    bool            m_EmbeddMetadata;
    wxString        m_DefaultCopyAction;
    int             m_LastUpdate;
    wxString        m_DirectoryPath;

    guMediaCollection( const int type = guMEDIA_COLLECTION_TYPE_NORMAL );
    ~guMediaCollection();

    bool            CheckPaths( void );
};
WX_DECLARE_OBJARRAY( guMediaCollection, guMediaCollectionArray );

// -------------------------------------------------------------------------------- //
class guManagedCollection : public guMediaCollection
{
  protected :
    bool            m_Enabled;
    wxMenu *        m_MenuItem;

  public :
    guManagedCollection( void );
    ~guManagedCollection();

};
WX_DECLARE_OBJARRAY( guManagedCollection, guManagedCollectionArray );

}

#endif
// -------------------------------------------------------------------------------- //
