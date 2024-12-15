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
#include "TagInfo.h"
#include "Utils.h"

#include "GstTypes.h"
#include "GstTypeFinder.h"
#include "MainFrame.h"
#include "TrackEdit.h"

#include <wx/base64.h>
#include <wx/mstream.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>

#include <asfattribute.h>
#include <popularimeterframe.h>
#include <id3v1tag.h>

#include <gst/pbutils/pbutils.h>

namespace Guayadeque {

wxArrayString guSupportedFormats;
wxMutex       guSupportedFormatsMutex;


// -------------------------------------------------------------------------------- //
bool guIsGStreamerExt( const wxString &ext )
{
    return guGstTypeFinder::getGTF().GetExtensions().Index( ext ) != wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
bool guIsValidAudioFile( const wxString &filename )
{
    wxMutexLocker Lock(guSupportedFormatsMutex);

    if( !guSupportedFormats.Count() )
    {
        guSupportedFormats.Add( wxT( "mp3"  ) );

        guSupportedFormats.Add( wxT( "flac" ) );

        guSupportedFormats.Add( wxT( "ogg"  ) );
        guSupportedFormats.Add( wxT( "oga"  ) );

        guSupportedFormats.Add( wxT( "mp4"  ) );  // MP4 files
        guSupportedFormats.Add( wxT( "m4a"  ) );
        guSupportedFormats.Add( wxT( "m4b"  ) );
        guSupportedFormats.Add( wxT( "m4p"  ) );
        guSupportedFormats.Add( wxT( "aac"  ) );

        guSupportedFormats.Add( wxT( "wma"  ) );
        guSupportedFormats.Add( wxT( "asf"  ) );

        guSupportedFormats.Add( wxT( "ape"  ) );

        guSupportedFormats.Add( wxT( "wav"  ) );
        guSupportedFormats.Add( wxT( "aif"  ) );

        guSupportedFormats.Add( wxT( "wv"   ) );

        guSupportedFormats.Add( wxT( "tta"  ) );

        guSupportedFormats.Add( wxT( "mpc"  ) );

        //guSupportedFormats.Add( wxT( "rmj"  ) );
        guSupportedFormats.Add( wxT( "opus"  ) );
    }

    wxString file_ext = filename.Lower().AfterLast( wxT( '.' ) );
    int Position = guSupportedFormats.Index( file_ext );

    if( (Position == wxNOT_FOUND) && guIsGStreamerExt( file_ext ) )
        Position = INT_MAX;

    return ( Position != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
guTagInfo * guGetTagInfoHandler( const wxString &filename )
{
    wxString file_ext = filename.Lower().AfterLast( wxT( '.' ) );

    guSupportedFormatsMutex.Lock();
    int FormatIndex = guSupportedFormats.Index( file_ext );
    guSupportedFormatsMutex.Unlock();

    switch( FormatIndex )
    {
        case  0 : return new guMp3TagInfo( filename );

        case  1 : return new guFlacTagInfo( filename );

        case  2 :
        case  3 :
        case 17 : return new guOggTagInfo( filename );

        case  4 :
        case  5 :
        case  6 :
        case  7 : return new guMp4TagInfo( filename );

        case  8 : return new guTagInfo( filename );     // aac

        case  9 :
        case 10 : return new guASFTagInfo( filename );

#ifdef TAGLIB_WITH_APE_SUPPORT
        case 11 : return new guApeTagInfo( filename );
#endif

        case 12 :
        case 13 : return new guTagInfo( filename );

        case 14 : return new guWavPackTagInfo( filename );

        case 15 : return new guTrueAudioTagInfo( filename );

        case 16 : return new guMpcTagInfo( filename );

        default :
            break;
    }

    if( guIsGStreamerExt( file_ext ) )
        return new guGStreamerTagInfo( filename );

    return nullptr;
}

// -------------------------------------------------------------------------------- //
TagLib::ID3v2::PopularimeterFrame * GetPopM( TagLib::ID3v2::Tag * tag, const TagLib::String &email )
{
    TagLib::ID3v2::FrameList PopMList = tag->frameList( "POPM" );
    for(auto & it : PopMList)
    {
        auto * PopMFrame = dynamic_cast<TagLib::ID3v2::PopularimeterFrame *>( it );
        //guLogMessage( wxT( "PopM e: '%s'  r: %i  c: %i" ), TStringTowxString( PopMFrame->email() ).c_str(), PopMFrame->rating(), PopMFrame->counter() );
        if( email.isEmpty() || ( PopMFrame->email() == email ) )
            return PopMFrame;
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
int inline guPopMToRating( const int rating )
{
    if( rating < 0 )
        return 0;
    if( rating == 0 )
        return 0;
    if( rating < 64 )
        return 1;
    if( rating < 128 )
        return 2;
    if( rating < 192 )
        return 3;
    if( rating < 255 )
        return 4;
    return 5;
}

// -------------------------------------------------------------------------------- //
int inline guWMRatingToRating( const int rating )
{
    if( rating <= 0 )
        return 0;
    if( rating < 25 )
        return 1;
    if( rating < 50 )
        return 2;
    if( rating < 75 )
        return 3;
    if( rating < 99 )
        return 4;
    return 5;
}

// -------------------------------------------------------------------------------- //
int inline guRatingToPopM( const int rating )
{
    int Ratings[] = { 0, 0, 1, 64, 128, 192, 255 };
    guLogMessage( wxT( "Rating: %i => %i" ), rating, Ratings[ rating + 1 ] );
    return Ratings[ rating + 1 ];
}


// -------------------------------------------------------------------------------- //
wxImage *GetID3v2ImageType(TagLib::ID3v2::FrameList &framelist,
         ID3v2::AttachedPictureFrame::Type frametype = ID3v2::AttachedPictureFrame::Type::FrontCover);

wxImage *GetID3v2ImageType(TagLib::ID3v2::FrameList &framelist,
         ID3v2::AttachedPictureFrame::Type frametype)
{
    TagLib::ID3v2::AttachedPictureFrame * PicFrame;
    for (auto & iter : framelist)
    {
        PicFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(iter);

        if ((frametype == ID3v2::AttachedPictureFrame::Type::Other) || (PicFrame->type() == frametype))
        {
            int ImgDataSize = PicFrame->picture().size();

            if (ImgDataSize > 0)
            {
                //guLogMessage( wxT( "ID3v2 header contains APIC frame with %u bytes." ), ImgDataSize );
                wxMemoryOutputStream ImgOutStream;
                ImgOutStream.Write( PicFrame->picture().data(), ImgDataSize );
                wxMemoryInputStream ImgInputStream( ImgOutStream );
                wxString ImgHandler = wxString( PicFrame->mimeType().toCString( true ), wxConvUTF8 );
                ImgHandler.Replace( wxT( "/jpg" ), wxT( "/jpeg" ) );

                auto * CoverImage = new wxImage( ImgInputStream, ImgHandler );
                if (CoverImage)
                {
                    if (CoverImage->IsOk())
                        return CoverImage;
                    delete CoverImage;
                }
    //		    wxFileOutputStream FOut( wxT( "~/test.jpg" ) );
    //		    FOut.Write( PicFrame->picture().data(), ImgDataSize );
    //		    FOut.Close();
            }
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
wxImage * GetID3v2Image( ID3v2::Tag * tagv2 )
{
    TagLib::ID3v2::FrameList FrameList = tagv2->frameListMap()["APIC"];

    wxImage * CoverImage = GetID3v2ImageType( FrameList, TagLib::ID3v2::AttachedPictureFrame::FrontCover );

    if( !CoverImage )
    {
        CoverImage = GetID3v2ImageType( FrameList, TagLib::ID3v2::AttachedPictureFrame::Other );
        if( !CoverImage )
            CoverImage = GetID3v2ImageType( FrameList, ID3v2::AttachedPictureFrame::Type::FileIcon );
    }

	return CoverImage;
}

// -------------------------------------------------------------------------------- //
void SetID3v2Image( ID3v2::Tag * tagv2, const wxImage * image )
{
    TagLib::ID3v2::AttachedPictureFrame * PicFrame;

    TagLib::ID3v2::FrameList FrameList = tagv2->frameListMap()["APIC"];
    for (auto & iter : FrameList)
    {
        PicFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(iter);

        // TODO : Ppl should be able to select which image types want guayadeque to remove from the audio files
        if ((PicFrame->type() == ID3v2::AttachedPictureFrame::Type::FrontCover) ||
            (PicFrame->type() == ID3v2::AttachedPictureFrame::Type::Other))
            tagv2->removeFrame(PicFrame, true);
    }

    if( image )
    {
        PicFrame = new TagLib::ID3v2::AttachedPictureFrame;
        PicFrame->setMimeType( "image/jpeg" );
        PicFrame->setType( TagLib::ID3v2::AttachedPictureFrame::FrontCover );
        wxMemoryOutputStream ImgOutputStream;
        if( image->SaveFile( ImgOutputStream, wxBITMAP_TYPE_JPEG ) )
        {
            ByteVector ImgData( (uint) ImgOutputStream.GetSize() );
            ImgOutputStream.CopyTo( ImgData.data(), ImgOutputStream.GetSize() );
            PicFrame->setPicture( ImgData );
            tagv2->addFrame( PicFrame );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString GetID3v2Lyrics( ID3v2::Tag * tagv2 )
{
	TagLib::ID3v2::FrameList frameList = tagv2->frameList( "USLT" );
	if( !frameList.isEmpty() )
	{
		TagLib::ID3v2::UnsynchronizedLyricsFrame * LyricsFrame = static_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame * >( frameList.front() );
        if( LyricsFrame )
        {
            //guLogMessage( wxT( "Found lyrics" ) );
            return TStringTowxString( LyricsFrame->text() );
        }
	}
	return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void SetID3v2Lyrics( ID3v2::Tag * tagv2, const wxString &lyrics )
{
    //guLogMessage( wxT( "Saving lyrics..." ) );
    TagLib::ID3v2::UnsynchronizedLyricsFrame * LyricsFrame;

    TagLib::ID3v2::FrameList FrameList = tagv2->frameListMap()["USLT"];
    for (auto & iter : FrameList)
    {
        LyricsFrame = dynamic_cast<TagLib::ID3v2::UnsynchronizedLyricsFrame*>(iter);
        tagv2->removeFrame(LyricsFrame, true);
    }

    if (!lyrics.IsEmpty())
    {
        LyricsFrame = new TagLib::ID3v2::UnsynchronizedLyricsFrame( TagLib::String::UTF8 );
        LyricsFrame->setText(wxStringToTString( lyrics));
        tagv2->addFrame(LyricsFrame);
    }
}

// -------------------------------------------------------------------------------- //
wxImage * GetXiphCommentCoverArt( Ogg::XiphComment * xiphcomment )
{
    if (xiphcomment && xiphcomment->contains("COVERART"))
    {
        wxString CoverMime = TStringTowxString(xiphcomment->fieldListMap()["COVERARTMIME"].front());

        wxString CoverEncData = TStringTowxString(xiphcomment->fieldListMap()["COVERART"].front());

        //guLogMessage( wxT( "Image:\n%s\n" ), CoverEncData.c_str() );

        wxMemoryBuffer CoverDecData = wxBase64Decode(CoverEncData);

        //guLogMessage( wxT( "Image Decoded Data : (%i) %i bytes" ), CoverDecData.GetBufSize(), CoverDecData.GetDataLen() );

        //wxFileOutputStream FOut( wxT( "/home/jrios/test.jpg" ) );
        //FOut.Write( CoverDecData.GetData(), CoverDecData.GetDataLen() );
        //FOut.Close();

        wxMemoryInputStream ImgInputStream(CoverDecData.GetData(), CoverDecData.GetDataLen());

        wxImage * CoverImage = new wxImage(ImgInputStream, CoverMime);
        if (CoverImage)
        {
            if (CoverImage->IsOk())
                return CoverImage;
            delete CoverImage;
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
bool SetXiphCommentCoverArt( Ogg::XiphComment * xiphcomment, const wxImage * image )
{
    if (!xiphcomment)
        return false;

    if (xiphcomment->contains("COVERART"))
    {
        xiphcomment->removeFields("COVERARTMIME");
        xiphcomment->removeFields("COVERART");
    }
    if (!image)
        return true;

    wxMemoryOutputStream ImgOutputStream;
    if (image->SaveFile(ImgOutputStream, wxBITMAP_TYPE_JPEG))
    {
        //ByteVector ImgData( ( TagLib::uint ) ImgOutputStream.GetSize() );
        //ImgOutputStream.CopyTo( ImgData.data(), ImgOutputStream.GetSize() );
        char * ImgData = (char *) malloc(ImgOutputStream.GetSize());
        if (ImgData)
        {
            ImgOutputStream.CopyTo(ImgData, ImgOutputStream.GetSize());
            xiphcomment->addField("COVERARTMIME", "image/jpeg");
            xiphcomment->addField("COVERART", wxStringToTString(wxBase64Encode(ImgData, ImgOutputStream.GetSize())));
            free(ImgData);
            return true;
        }
        guLogMessage(wxT("Couldnt allocate memory saving the image to ogg"));
    }
    return false;
}

// -------------------------------------------------------------------------------- //
wxString GetXiphCommentLyrics( Ogg::XiphComment * xiphcomment )
{
    if( xiphcomment && xiphcomment->contains( "LYRICS" ) )
        return TStringTowxString( xiphcomment->fieldListMap()[ "LYRICS" ].front() );
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool SetXiphCommentLyrics( Ogg::XiphComment * xiphcomment, const wxString &lyrics )
{
    if (xiphcomment)
    {
        while (xiphcomment->contains("LYRICS"))
            xiphcomment->removeFields("LYRICS");

        if (!lyrics.IsEmpty())
            xiphcomment->addField("LYRICS", wxStringToTString(lyrics));
        return true;
    }
    return false;
}

#ifdef TAGLIB_WITH_MP4_COVERS
// -------------------------------------------------------------------------------- //
wxImage * GetMp4Image( TagLib::MP4::Tag * mp4tag )
{
    if( mp4tag && mp4tag->itemMap().contains( "covr" ) )
    {
        TagLib::MP4::CoverArtList Covers = mp4tag->itemMap()[ "covr" ].toCoverArtList();

        for (const auto & Cover : Covers)
        {
            wxBitmapType ImgType = wxBITMAP_TYPE_INVALID;
            if (Cover.format() == TagLib::MP4::CoverArt::Format::PNG)
                ImgType = wxBITMAP_TYPE_PNG;
            else if (Cover.format() == TagLib::MP4::CoverArt::Format::JPEG)
                ImgType = wxBITMAP_TYPE_JPEG;

            wxMemoryOutputStream ImgOutStream;
            ImgOutStream.Write(Cover.data().data(), Cover.data().size());
            wxMemoryInputStream ImgInputStream(ImgOutStream);

            auto * CoverImage = new wxImage(ImgInputStream, ImgType);
            if( CoverImage )
            {
                if( CoverImage->IsOk() )
                    return CoverImage;
                else
                    delete CoverImage;
            }
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
bool SetMp4Image( TagLib::MP4::Tag * mp4tag, const wxImage * image )
{
    if (!mp4tag)
        return false;

    if (mp4tag->itemMap().contains( "covr"))
        mp4tag->removeItem("covr");

    if (!image)
        return true;

    wxMemoryOutputStream ImgOutputStream;
    if (image && image->SaveFile(ImgOutputStream, wxBITMAP_TYPE_JPEG))
    {
        ByteVector ImgData((uint) ImgOutputStream.GetSize());
        ImgOutputStream.CopyTo(ImgData.data(), ImgOutputStream.GetSize());

        TagLib::MP4::CoverArtList CoverList;
        TagLib::MP4::CoverArt Cover(TagLib::MP4::CoverArt::JPEG, ImgData);
        CoverList.append(Cover);
        mp4tag->setItem("covr", CoverList);
        return true;
    }
    return false;
}
#endif

// -------------------------------------------------------------------------------- //
wxString GetMp4Lyrics( TagLib::MP4::Tag * mp4tag )
{
    if (mp4tag && mp4tag->itemMap().contains("\xa9lyr"))
        return TStringTowxString( mp4tag->itemMap()[ "\xa9lyr" ].toStringList().front() );
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool SetMp4Lyrics( TagLib::MP4::Tag * mp4tag, const wxString &lyrics )
{
    if (!mp4tag)
        return false;

    if (mp4tag->itemMap().contains("\xa9lyr"))
        mp4tag->removeItem("\xa9lyr");

    if (!lyrics.IsEmpty())
    {
        const TagLib::String Lyrics = wxStringToTString(lyrics);
        mp4tag->setItem("\xa9lyr", TagLib::StringList(Lyrics));
    }
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * GetApeItemImage( const TagLib::APE::Item &item )
{
    if (item.type() == TagLib::APE::Item::ItemTypes::Binary)
    {
        TagLib::ByteVector CoverData = item.binaryData();

        if (CoverData.size())
        {
            wxMemoryOutputStream ImgOutStream;
            ImgOutStream.Write(CoverData.data(), CoverData.size());
            wxMemoryInputStream ImgInputStream(ImgOutStream);
            auto * CoverImage = new wxImage(ImgInputStream, wxBITMAP_TYPE_JPEG);
            if (CoverImage)
            {
                if (CoverImage->IsOk())
                    return CoverImage;
                delete CoverImage;
            }
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
wxImage * GetApeImage( TagLib::APE::Tag * apetag )
{
    if( apetag )
    {
        if( apetag->itemListMap().contains( "Cover Art (front)" ) )
            return GetApeItemImage( apetag->itemListMap()[ "Cover Art (front)" ] );
        else if( apetag->itemListMap().contains( "Cover Art (other)" ) )
            return GetApeItemImage( apetag->itemListMap()[ "Cover Art (other)" ] );
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
bool SetApeImage( TagLib::APE::Tag * apetag, const wxImage * image )
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxString GetApeLyrics( APE::Tag * apetag )
{
    if( apetag )
    {
        if( apetag->itemListMap().contains( "LYRICS" ) )
            return TStringTowxString( apetag->itemListMap()[ "LYRICS" ].toString() );
        else if( apetag->itemListMap().contains( "UNSYNCED LYRICS" ) )
            return TStringTowxString( apetag->itemListMap()[ "UNSYNCED LYRICS" ].toString() );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool SetApeLyrics( APE::Tag * apetag, const wxString &lyrics )
{
    if (!apetag)
        return false;

    if( apetag->itemListMap().contains( "LYRICS" ) )
        apetag->removeItem( "LYRICS" );
    if( apetag->itemListMap().contains( "UNSYNCED LYRICS" ) )
        apetag->removeItem( "UNSYNCED LYRICS" );
    if( !lyrics.IsEmpty() )
    {
        const TagLib::String Lyrics = wxStringToTString( lyrics );
        apetag->addValue( "Lyrics", Lyrics );
    }
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * GetASFImage( ASF::Tag * asftag )
{
    if (asftag)
    {
        if (asftag->attributeListMap().contains("WM/Picture"))
        {
            ByteVector PictureData = asftag->attributeListMap()[ "WM/Picture" ].front().toByteVector();
            TagLib::ID3v2::AttachedPictureFrame * PicFrame;
            PicFrame = ( TagLib::ID3v2::AttachedPictureFrame * ) PictureData.data();
            int ImgDataSize = PicFrame->picture().size();

            if (ImgDataSize > 0)
            {
                //guLogMessage( wxT( "ASF header contains APIC frame with %u bytes." ), ImgDataSize );
                wxMemoryOutputStream ImgOutStream;
                ImgOutStream.Write( PicFrame->picture().data(), ImgDataSize );
                wxMemoryInputStream ImgInputStream( ImgOutStream );
                wxString ImgHandler = wxString( PicFrame->mimeType().toCString( true ), wxConvUTF8 );
                ImgHandler.Replace( wxT( "/jpg" ), wxT( "/jpeg" ) );
                auto CoverImage = new wxImage(ImgInputStream, ImgHandler);
                if (CoverImage)
                {
                    if (CoverImage->IsOk())
                        return CoverImage;
                    delete CoverImage;
                }
            }
        }
    }
    return nullptr;
}

// -------------------------------------------------------------------------------- //
bool SetASFImage( ASF::Tag * asftag, const wxImage * image )
{
    return false;
}


// -------------------------------------------------------------------------------- //
// guTagInfo
// -------------------------------------------------------------------------------- //
guTagInfo::guTagInfo( const wxString &filename )
{
    m_TagFile = nullptr;
    m_Tag = nullptr;

    SetFileName( filename );

    m_Track = 0;
    m_Year = 0;
    m_Length = 0;
    m_Bitrate = 0;
    m_Rating = wxNOT_FOUND;
    m_PlayCount = 0;
    m_Compilation = false;
}

// -------------------------------------------------------------------------------- //
guTagInfo::~guTagInfo()
{
    if (m_TagFile)
        delete m_TagFile;
}

// -------------------------------------------------------------------------------- //
void guTagInfo::SetFileName( const wxString &filename )
{
    m_FileName = filename;
    if( !filename.IsEmpty() )
        m_TagFile = new TagLib::FileRef( filename.mb_str( wxConvFile ), true, TagLib::AudioProperties::Fast );

    if( m_TagFile && !m_TagFile->isNull() )
    {
        m_Tag = m_TagFile->tag();
        if( !m_Tag )
            guLogWarning( wxT( "Cant get tag object from '%s'" ), filename.c_str() );
    }
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Read( )
{
    AudioProperties * apro;
    if( m_Tag )
    {
        m_TrackName = TStringTowxString( m_Tag->title() );
        m_ArtistName = TStringTowxString( m_Tag->artist() );
        m_AlbumName = TStringTowxString( m_Tag->album() );
        m_GenreName = TStringTowxString( m_Tag->genre() );
        m_Comments = TStringTowxString( m_Tag->comment() );
        m_Track = m_Tag->track();
        m_Year = m_Tag->year();
    }

    if( m_TagFile && m_Tag && ( apro = m_TagFile->audioProperties() ) )
    {
        m_Length = apro->lengthInMilliseconds();
        m_Bitrate = apro->bitrate();
        //m_Samplerate = apro->sampleRate();
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::Write( const int changedflag )
{
    if( m_Tag && ( changedflag & guTRACK_CHANGED_DATA_TAGS ) )
    {
        m_Tag->setTitle( wxStringToTString( m_TrackName ) );
        m_Tag->setArtist( wxStringToTString( m_ArtistName ) );
        m_Tag->setAlbum( wxStringToTString( m_AlbumName ) );
        m_Tag->setGenre( wxStringToTString( m_GenreName ) );
        m_Tag->setComment( wxStringToTString( m_Comments ) );
        m_Tag->setTrack( m_Track ); // set the id3v1 track
        m_Tag->setYear( m_Year );
    }

    if( !m_TagFile->save() )
    {
      guLogWarning( wxT( "Tags Save failed for file '%s'" ), m_FileName.c_str() );
      return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::CanHandleImages()
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxImage * guTagInfo::GetImage()
{
	return nullptr;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::SetImage( const wxImage * image )
{
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::CanHandleLyrics()
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxString guTagInfo::GetLyrics()
{
	return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::SetLyrics( const wxString &lyrics )
{
    return false;
}

// -------------------------------------------------------------------------------- //
void ID3v2_CheckLabelFrame( ID3v2::Tag * tagv2, const char * description, const wxString &value )
{
    ID3v2::UserTextIdentificationFrame * frame;
    //guLogMessage( wxT( "USERTEXT[ '%s' ] = '%s'" ), wxString( description, wxConvUTF8 ).c_str(), value.c_str() );
    frame = ID3v2::UserTextIdentificationFrame::find( tagv2, description );
    if( !frame )
    {
        frame = new ID3v2::UserTextIdentificationFrame( TagLib::String::UTF8 );
        tagv2->addFrame( frame );
        //frame->setDescription( TagLib::String( description, TagLib::String::UTF8 ) );
        frame->setDescription( description );
    }

    if( frame )
        frame->setText( wxStringToTString( value ) );
}

// -------------------------------------------------------------------------------- //
void Xiph_CheckLabelFrame( Ogg::XiphComment * xiphcomment, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if (xiphcomment->fieldListMap().contains(description ))
    {
        if (!value.IsEmpty())
            xiphcomment->addField(description, wxStringToTString(value));
        else
            xiphcomment->removeFields(description);
    }
    else if(!value.IsEmpty())
        xiphcomment->addField(description, wxStringToTString(value));
}

// -------------------------------------------------------------------------------- //
bool guStrDiskToDiskNum( const wxString &diskstr, int &disknum, int &disktotal )
{
    unsigned long Number;
    disknum = 0;
    disktotal = 0;
    wxString DiskNum = diskstr.BeforeFirst(wxT('/'));
    if (!DiskNum.IsEmpty() && DiskNum.ToULong(&Number))
    {
        disknum = Number;
        if (diskstr.Find(wxT("/")))
        {
            DiskNum = diskstr.AfterFirst(wxT('/'));
            if (DiskNum.ToULong(&Number))
                disktotal = Number;
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void Mp4_CheckLabelFrame( TagLib::MP4::Tag * mp4tag, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if( mp4tag->itemMap().contains( description ) )
    {
        if( !value.IsEmpty() )
            mp4tag->setItem(description, TagLib::MP4::Item( TagLib::StringList( wxStringToTString( value ) ) ));
        else
            mp4tag->removeItem(description);
    }
    else if( !value.IsEmpty() )
        mp4tag->setItem(description, TagLib::MP4::Item(TagLib::StringList(wxStringToTString(value))));
}

// -------------------------------------------------------------------------------- //
void Ape_CheckLabelFrame( TagLib::APE::Tag * apetag, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if (apetag->itemListMap().contains(description))
        apetag->removeItem(description);
    if (!value.IsEmpty())
        apetag->addValue( description, wxStringToTString( value ) );
}

// -------------------------------------------------------------------------------- //
void ASF_CheckLabelFrame( ASF::Tag * asftag, const char * description, const wxString &value )
{
    //guLogMessage( wxT( "USERTEXT[ %s ] = '%s'" ), wxString( description, wxConvISO8859_1 ).c_str(), value.c_str() );
    if( asftag->attributeListMap().contains( description ) )
        asftag->removeItem( description );
    if( !value.IsEmpty() )
        asftag->setAttribute( description, wxStringToTString( value ) );
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( ID3v2::Tag * tag )
{
    if( tag )
    {
        if( tag->frameListMap().contains( "TPOS" ) )
            m_Disk = TStringTowxString( tag->frameListMap()[ "TPOS" ].front()->toString() );

        if( tag->frameListMap().contains( "TCOM" ) )
            m_Composer = TStringTowxString( tag->frameListMap()[ "TCOM" ].front()->toString() );

        if( tag->frameListMap().contains( "TPE2" ) )
            m_AlbumArtist = TStringTowxString( tag->frameListMap()[ "TPE2" ].front()->toString() );

        if( tag->frameListMap().contains( "TCMP" ) )
            m_Compilation = TStringTowxString( tag->frameListMap()[ "TCMP" ].front()->toString() ) == wxT( "1" );

        TagLib::ID3v2::PopularimeterFrame * PopMFrame = nullptr;

        PopMFrame = GetPopM( tag, "Guayadeque" );
        if( !PopMFrame )
            PopMFrame = GetPopM( tag, "" );

        if( PopMFrame )
        {
            m_Rating = guPopMToRating( PopMFrame->rating() );
            m_PlayCount = PopMFrame->counter();
        }


        if( m_TrackLabels.Count() == 0 )
        {
            ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tag, "TRACK_LABELS" );
            if( !Frame )
                Frame = ID3v2::UserTextIdentificationFrame::find( tag, "guTRLABELS" );
            if( Frame )
            {
                //guLogMessage( wxT( "*Track Label: '%s'" ), TStringTowxString( Frame->fieldList()[ 1 ] ).c_str() );
                // [guTRLABELS] guTRLABELS labels
                StringList TrLabelsList = Frame->fieldList();
                if( TrLabelsList.size() )
                {
                    m_TrackLabelsStr = TStringTowxString( TrLabelsList[ 1 ] );
                    m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
                }
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tag, "ARTIST_LABELS" );
            if( !Frame )
                Frame = ID3v2::UserTextIdentificationFrame::find( tag, "guARLABELS" );
            if( Frame )
            {
                //guLogMessage( wxT( "*Artist Label: '%s'" ), TStringTowxString( Frame->fieldList()[ 1 ] ).c_str() );
                StringList ArLabelsList = Frame->fieldList();
                if( ArLabelsList.size() )
                {
                    m_ArtistLabelsStr = TStringTowxString( ArLabelsList[ 1 ] );
                    m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
                }
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            ID3v2::UserTextIdentificationFrame * Frame = ID3v2::UserTextIdentificationFrame::find( tag, "ALBUM_LABELS" );
            if( !Frame )
                Frame = ID3v2::UserTextIdentificationFrame::find( tag, "guALLABELS" );
            if( Frame )
            {
                //guLogMessage( wxT( "*Album Label: '%s'" ), TStringTowxString( Frame->fieldList()[ 1 ] ).c_str() );
                StringList AlLabelsList = Frame->fieldList();
                if( AlLabelsList.size() )
                {
                    m_AlbumLabelsStr = TStringTowxString( AlLabelsList[ 1 ] );
                    m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
                }
            }
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( ID3v2::Tag * tag, const int changedflag ) const
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            TagLib::ID3v2::TextIdentificationFrame * frame;
            tag->removeFrames( "TPOS" );
            frame = new TagLib::ID3v2::TextIdentificationFrame( "TPOS" );
            frame->setText( wxStringToTString( m_Disk ) );
            tag->addFrame( frame );

            tag->removeFrames( "TCOM" );
            frame = new TagLib::ID3v2::TextIdentificationFrame( "TCOM" );
            frame->setText( wxStringToTString( m_Composer ) );
            tag->addFrame( frame );

            tag->removeFrames( "TPE2" );
            frame = new TagLib::ID3v2::TextIdentificationFrame( "TPE2" );
            frame->setText( wxStringToTString( m_AlbumArtist ) );
            tag->addFrame( frame );

            //tag->removeFrames( "TCMP" );
            //frame = new TagLib::ID3v2::TextIdentificationFrame( "TCMP" );
            //frame->setText( wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
            //tag->addFrame( frame );

            // I have found several TRCK fields in the mp3s
            tag->removeFrames( "TRCK" );
            tag->setTrack( m_Track );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            guLogMessage( wxT( "Writing ratings and playcount..." ) );
            TagLib::ID3v2::PopularimeterFrame * PopMFrame = GetPopM( tag, "Guayadeque" );
            if( !PopMFrame )
            {
                PopMFrame = new TagLib::ID3v2::PopularimeterFrame();
                tag->addFrame( PopMFrame );
                PopMFrame->setEmail( "Guayadeque" );
            }
            PopMFrame->setRating( guRatingToPopM( m_Rating ) );
            PopMFrame->setCounter( m_PlayCount );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            ID3v2_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            ID3v2_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            ID3v2_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( Ogg::XiphComment * tag )
{
    if( tag )
    {
        if( tag->fieldListMap().contains( "COMPOSER" ) )
            m_Composer = TStringTowxString( tag->fieldListMap()["COMPOSER"].front() );

        if( tag->fieldListMap().contains( "DISCNUMBER" ) )
            m_Disk = TStringTowxString( tag->fieldListMap()["DISCNUMBER"].front() );

        if( tag->fieldListMap().contains( "COMPILATION" ) )
            m_Compilation = TStringTowxString( tag->fieldListMap()["COMPILATION"].front() ) == wxT( "1" );

        if( tag->fieldListMap().contains( "ALBUMARTIST" ) )
            m_AlbumArtist = TStringTowxString( tag->fieldListMap()["ALBUMARTIST"].front() );
        else if( tag->fieldListMap().contains( "ALBUM ARTIST" ) )
            m_AlbumArtist = TStringTowxString( tag->fieldListMap()["ALBUM ARTIST"].front() );

        // Rating
        if( tag->fieldListMap().contains( "RATING" ) )
        {
            long Rating = 0;
            if( TStringTowxString( tag->fieldListMap()["RATING"].front() ).ToLong( &Rating ) )
            {
                if( Rating )
                {
                    if( Rating > 5 )
                        m_Rating = guPopMToRating( Rating );
                    else
                        m_Rating = Rating;
                }
            }
        }

        if( tag->fieldListMap().contains( "PLAY_COUNTER" ) )
        {
            long PlayCount = 0;
            if( TStringTowxString( tag->fieldListMap()["PLAY_COUNTER"].front() ).ToLong( &PlayCount ) )
                m_PlayCount = PlayCount;
        }

        // Labels
        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->fieldListMap().contains( "TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->fieldListMap()["TRACK_LABELS"].front() );
                //guLogMessage( wxT( "*Track Label: '%s'\n" ), m_TrackLabelsStr.c_str() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->fieldListMap().contains( "ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->fieldListMap()["ARTIST_LABELS"].front() );
                //guLogMessage( wxT( "*Artist Label: '%s'\n" ), m_ArtistLabelsStr.c_str() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->fieldListMap().contains( "ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->fieldListMap()["ALBUM_LABELS"].front() );
                //guLogMessage( wxT( "*Album Label: '%s'\n" ), m_AlbumLabelsStr.c_str() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }

        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( Ogg::XiphComment * tag, const int changedflag ) const
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->addField( "DISCNUMBER", wxStringToTString( m_Disk ) );
            tag->addField( "COMPOSER", wxStringToTString( m_Composer ) );
            tag->addField( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
            tag->addField( "ALBUMARTIST", wxStringToTString(  m_AlbumArtist ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            tag->addField( "RATING", wxStringToTString( wxString::Format( wxT( "%u" ), guRatingToPopM( m_Rating ) ) ) );
            tag->addField( "PLAY_COUNTER", wxStringToTString( wxString::Format( wxT( "%u" ), m_PlayCount ) ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            Xiph_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            Xiph_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            Xiph_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( MP4::Tag * tag )
{
    if( tag )
    {
        if( tag->itemMap().contains( "aART" ) )
            m_AlbumArtist = TStringTowxString( tag->itemMap()["aART"].toStringList().front() );

        if( tag->itemMap().contains( "\xA9wrt" ) )
            m_Composer = TStringTowxString( tag->itemMap()["\xa9wrt"].toStringList().front() );

        if( tag->itemMap().contains( "disk" ) )
        {
            m_Disk = wxString::Format( wxT( "%i/%i" ),
                tag->itemMap()["disk"].toIntPair().first,
                tag->itemMap()["disk"].toIntPair().second );

        }

        if( tag->itemMap().contains( "cpil" ) )
            m_Compilation = tag->itemMap()["cpil"].toBool();

        // Rating
        if( tag->itemMap().contains( "----:com.apple.iTunes:RATING" ) )
        {
            long Rating = 0;
            if( TStringTowxString( tag->itemMap()["----:com.apple.iTunes:RATING"].toStringList().front() ).ToLong( &Rating ) )
            {
                if( Rating )
                {
                    if( Rating > 5 )
                        m_Rating = guPopMToRating( Rating );
                    else
                        m_Rating = Rating;
                }
            }
        }

        if( tag->itemMap().contains( "----:com.apple.iTunes:PLAY_COUNTER" ) )
        {
            long PlayCount = 0;
            if( TStringTowxString( tag->itemMap()["----:com.apple.iTunes:PLAY_COUNTER"].toStringList().front()  ).ToLong( &PlayCount ) )
                m_PlayCount = PlayCount;
        }

        // Labels
        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->itemMap().contains( "----:com.apple.iTunes:TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->itemMap()["----:com.apple.iTunes:TRACK_LABELS"].toStringList().front() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->itemMap().contains( "----:com.apple.iTunes:ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->itemMap()["----:com.apple.iTunes:ARTIST_LABELS"].toStringList().front() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->itemMap().contains( "----:com.apple.iTunes:ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->itemMap()["----:com.apple.iTunes:ALBUM_LABELS"].toStringList().front() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }

        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( MP4::Tag * tag, const int changedflag ) const
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->setItem("aART", TagLib::StringList( wxStringToTString( m_AlbumArtist ) ));
            tag->setItem("\xA9wrt", TagLib::StringList( wxStringToTString( m_Composer ) ));
            int first;
            int second;
            guStrDiskToDiskNum( m_Disk, first, second );
            tag->setItem("disk", TagLib::MP4::Item( first, second ));
            tag->setItem("cpil", TagLib::MP4::Item( m_Compilation ));
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            tag->setItem("----:com.apple.iTunes:RATING", TagLib::MP4::Item( wxStringToTString( wxString::Format( wxT( "%u" ), guRatingToPopM( m_Rating ) ) ) ));
            tag->setItem("----:com.apple.iTunes:PLAY_COUNTER", TagLib::MP4::Item( wxStringToTString( wxString::Format( wxT( "%u" ), m_PlayCount ) ) ));
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            Mp4_CheckLabelFrame( tag, "----:com.apple.iTunes:ARTIST_LABELS", m_ArtistLabelsStr );
            Mp4_CheckLabelFrame( tag, "----:com.apple.iTunes:ALBUM_LABELS", m_AlbumLabelsStr );
            Mp4_CheckLabelFrame( tag, "----:com.apple.iTunes:TRACK_LABELS", m_TrackLabelsStr );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( APE::Tag * tag )
{
    if( tag )
    {
        if( tag->itemListMap().contains( "COMPOSER" ) )
            m_Composer = TStringTowxString( tag->itemListMap()["COMPOSER"].toString() );

        if( tag->itemListMap().contains( "DISCNUMBER" ) )
            m_Disk = TStringTowxString( tag->itemListMap()["DISCNUMBER"].toString() );

        if( tag->itemListMap().contains( "COMPILATION" ) )
            m_Compilation = TStringTowxString( tag->itemListMap()["COMPILATION"].toString() ) == wxT( "1" );

        if( tag->itemListMap().contains( "ALBUM ARTIST" ) )
            m_AlbumArtist = TStringTowxString( tag->itemListMap()["ALBUM ARTIST"].toString() );
        else if( tag->itemListMap().contains( "ALBUMARTIST" ) )
            m_AlbumArtist = TStringTowxString( tag->itemListMap()["ALBUMARTIST"].toString() );

        // Rating
        if( tag->itemListMap().contains( "RATING" ) )
        {
            long Rating = 0;
            if( TStringTowxString( tag->itemListMap()["RATING"].toString() ).ToLong( &Rating ) )
            {
                if( Rating )
                {
                    if( Rating > 5 )
                        m_Rating = guPopMToRating( Rating );
                    else
                        m_Rating = Rating;
                }
            }
        }

        if( tag->itemListMap().contains( "PLAY_COUNTER" ) )
        {
            long PlayCount = 0;
            if( TStringTowxString( tag->itemListMap()["PLAY_COUNTER"].toString() ).ToLong( &PlayCount ) )
                m_PlayCount = PlayCount;
        }

        // Labels
        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->itemListMap()["TRACK_LABELS"].toString() );
                //guLogMessage( wxT( "*Track Label: '%s'\n" ), m_TrackLabelsStr.c_str() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->itemListMap()["ARTIST_LABELS"].toString() );
                //guLogMessage( wxT( "*Artist Label: '%s'\n" ), m_ArtistLabelsStr.c_str() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->itemListMap().contains( "ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->itemListMap()["ALBUM_LABELS"].toString() );
                //guLogMessage( wxT( "*Album Label: '%s'\n" ), m_AlbumLabelsStr.c_str() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( APE::Tag * tag, const int changedflag ) const
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->addValue( "COMPOSER", wxStringToTString( m_Composer ) );
            tag->addValue( "DISCNUMBER", wxStringToTString( m_Disk ) );
            tag->addValue( "COMPILATION", wxStringToTString( wxString::Format( wxT( "%u" ), m_Compilation ) ) );
            tag->addValue( "ALBUM ARTIST", wxStringToTString( m_AlbumArtist ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
            tag->addValue( "RATING", wxStringToTString( wxString::Format( wxT( "%u" ), guRatingToPopM( m_Rating ) ) ) );
            tag->addValue( "PLAY_COUNTER", wxStringToTString( wxString::Format( wxT( "%u" ), m_PlayCount ) ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            Ape_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            Ape_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            Ape_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::ReadExtendedTags( ASF::Tag * tag )
{
    if( tag )
    {
        if( tag->attributeListMap().contains( "WM/PartOfSet" ) )
            m_Disk = TStringTowxString( tag->attributeListMap()[ "WM/PartOfSet" ].front().toString() );

        if( tag->attributeListMap().contains( "WM/Composer" ) )
            m_Composer = TStringTowxString( tag->attributeListMap()[ "WM/Composer" ].front().toString() );

        if( tag->attributeListMap().contains( "WM/AlbumArtist" ) )
            m_AlbumArtist = TStringTowxString( tag->attributeListMap()[ "WM/AlbumArtist" ].front().toString() );

        long Rating = 0;
        if( tag->attributeListMap().contains( "WM/SharedUserRating" ) )
            TStringTowxString( tag->attributeListMap()[ "WM/SharedUserRating" ].front().toString() ).ToLong( &Rating );

        if( !Rating && tag->attributeListMap().contains( "Rating" ) )
            TStringTowxString( tag->attributeListMap()[ "Rating" ].front().toString() ).ToLong( &Rating );

        if( Rating )
        {
            if( Rating > 5 )
                m_Rating = guWMRatingToRating( Rating );
            else
                m_Rating = Rating;
        }

        if( m_TrackLabels.Count() == 0 )
        {
            if( tag->attributeListMap().contains( "TRACK_LABELS" ) )
            {
                m_TrackLabelsStr = TStringTowxString( tag->attributeListMap()[ "TRACK_LABELS" ].front().toString() );
                m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
            }
        }

        if( m_ArtistLabels.Count() == 0 )
        {
            if( tag->attributeListMap().contains( "ARTIST_LABELS" ) )
            {
                m_ArtistLabelsStr = TStringTowxString( tag->attributeListMap()[ "ARTIST_LABELS" ].front().toString() );
                m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
            }
        }

        if( m_AlbumLabels.Count() == 0 )
        {
            if( tag->attributeListMap().contains( "ALBUM_LABELS" ) )
            {
                m_AlbumLabelsStr = TStringTowxString( tag->attributeListMap()[ "ALBUM_LABELS" ].front().toString() );
                m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
            }
        }
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTagInfo::WriteExtendedTags( ASF::Tag * tag, const int changedflag ) const
{
    if( tag )
    {
        if( changedflag & guTRACK_CHANGED_DATA_TAGS )
        {
            tag->removeItem( "WM/PartOfSet" );
            tag->setAttribute( "WM/PartOfSet", wxStringToTString( m_Disk ) );

            tag->removeItem( "WM/Composer" );
            tag->setAttribute( "WM/Composer", wxStringToTString( m_Composer ) );

            tag->removeItem( "WM/AlbumArtist" );
            tag->setAttribute( "WM/AlbumArtist", wxStringToTString( m_AlbumArtist ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_RATING )
        {
             tag->removeItem( "WM/SharedUserRating" );
             int WMRatings[] = { 0, 0, 1, 25, 50, 75, 99 };
             tag->setAttribute( "WM/SharedUserRating", wxStringToTString( wxString::Format( wxT( "%i" ), WMRatings[ m_Rating + 1 ] ) ) );
        }

        if( changedflag & guTRACK_CHANGED_DATA_LABELS )
        {
            // The Labels
            ASF_CheckLabelFrame( tag, "ARTIST_LABELS", m_ArtistLabelsStr );
            ASF_CheckLabelFrame( tag, "ALBUM_LABELS", m_AlbumLabelsStr );
            ASF_CheckLabelFrame( tag, "TRACK_LABELS", m_TrackLabelsStr );
        }
        return true;
    }
    return false;
}


// -------------------------------------------------------------------------------- //
// guMp3TagInfo
// -------------------------------------------------------------------------------- //
guMp3TagInfo::guMp3TagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if( m_TagFile && !m_TagFile->isNull() )
        m_TagId3v2 = ( ( TagLib::MPEG::File * ) m_TagFile->file() )->ID3v2Tag();
    else
        m_TagId3v2 = nullptr;
}

// -------------------------------------------------------------------------------- //
guMp3TagInfo::~guMp3TagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::Read()
{
    if( guTagInfo::Read() )
    {
        // If its a ID3v2 Tag try to load the labels
        if( m_TagId3v2 )
            ReadExtendedTags( m_TagId3v2 );
    }
    else
      guLogError( wxT( "Could not read tags from file '%s'" ), m_FileName.c_str() );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::Write( const int changedflag )
{
    if( m_TagId3v2 )
        WriteExtendedTags( m_TagId3v2, changedflag );

    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMp3TagInfo::GetImage()
{
    if( m_TagId3v2 )
        return GetID3v2Image( m_TagId3v2 );
    return nullptr;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::SetImage( const wxImage * image )
{
    if (!m_TagId3v2)
        return false;

    SetID3v2Image( m_TagId3v2, image );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMp3TagInfo::GetLyrics()
{
    if (m_TagId3v2)
        return GetID3v2Lyrics( m_TagId3v2  );
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guMp3TagInfo::SetLyrics( const wxString &lyrics )
{
    if (!m_TagId3v2)
        return false;

    SetID3v2Lyrics( m_TagId3v2, lyrics );
    return true;
}


// -------------------------------------------------------------------------------- //
// guFlacTagInfo
// -------------------------------------------------------------------------------- //
guFlacTagInfo::guFlacTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if (m_TagFile && !m_TagFile->isNull())
        m_XiphComment = ((TagLib::FLAC::File *) m_TagFile->file())->xiphComment();
    else
        m_XiphComment = nullptr;
}

// -------------------------------------------------------------------------------- //
guFlacTagInfo::~guFlacTagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::Read()
{
    if (!guTagInfo::Read())
        return false;

    ReadExtendedTags(m_XiphComment);
    return true;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_XiphComment, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * GetFlacImage( const List<FLAC::Picture *>& Pictures, TagLib::FLAC::Picture::Type imagetype )
{
    wxImage * CoverImage = nullptr;

    for(auto Pic : Pictures)
    {
        if( Pic->type() == imagetype )
        {
            wxBitmapType ImgType = wxBITMAP_TYPE_INVALID;
            if( Pic->mimeType() == "image/png" )
                ImgType = wxBITMAP_TYPE_PNG;
            else if( Pic->mimeType() == "image/jpeg" )
                ImgType = wxBITMAP_TYPE_JPEG;

            wxMemoryOutputStream ImgOutStream;
            ImgOutStream.Write( Pic->data().data(), Pic->data().size() );
            wxMemoryInputStream ImgInputStream( ImgOutStream );
            auto * CoverFlacImage = new wxImage(ImgInputStream, ImgType );
            if (CoverFlacImage)
            {
                if (CoverFlacImage->IsOk())
                    return CoverFlacImage;
                delete CoverFlacImage;
            }
        }
    }
    return CoverImage;
}

// -------------------------------------------------------------------------------- //
wxImage * guFlacTagInfo::GetImage()
{
    wxImage * CoverImage = nullptr;

    if( m_TagFile )
    {
        List<FLAC::Picture *> Pictures = ( ( FLAC::File * ) m_TagFile->file() )->pictureList();
        if( Pictures.size() > 0 )
        {
            CoverImage = GetFlacImage( Pictures, TagLib::FLAC::Picture::FrontCover );
            if( !CoverImage )
                CoverImage = GetFlacImage( Pictures, TagLib::FLAC::Picture::Other );
        }
    }
    return CoverImage;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::SetImage( const wxImage * image )
{
    bool RetVal = false;
    if( m_TagFile )
    {
        auto * FlacFile = ( FLAC::File * ) m_TagFile->file();

        FlacFile->removePictures();

        if( image )
        {
            wxMemoryOutputStream ImgOutputStream;
            if( image && image->SaveFile( ImgOutputStream, wxBITMAP_TYPE_JPEG ) )
            {
                ByteVector ImgData( ( uint ) ImgOutputStream.GetSize() );
                ImgOutputStream.CopyTo( ImgData.data(), ImgOutputStream.GetSize() );

                auto * Pic = new FLAC::Picture();
                if( Pic )
                {
                    Pic->setData( ImgData );
                    //Pic->setDescription( "" );
                    Pic->setMimeType( "image/jpeg" );
                    Pic->setType( TagLib::FLAC::Picture::FrontCover );
                    FlacFile->addPicture( Pic );
                    return true;
                }
            }
            return false;
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guFlacTagInfo::GetLyrics()
{
    return GetXiphCommentLyrics( m_XiphComment );
}

// -------------------------------------------------------------------------------- //
bool guFlacTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetXiphCommentLyrics( m_XiphComment, lyrics );
}


// -------------------------------------------------------------------------------- //
// guOggTagInfo
// -------------------------------------------------------------------------------- //
guOggTagInfo::guOggTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if (m_TagFile && !m_TagFile->isNull())
        m_XiphComment = ((TagLib::Ogg::Vorbis::File *) m_TagFile->file())->tag();
    else
        m_XiphComment = nullptr;
}

// -------------------------------------------------------------------------------- //
guOggTagInfo::~guOggTagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::Read()
{
    if (!guTagInfo::Read())
        return false;

    ReadExtendedTags( m_XiphComment );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_XiphComment, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guOggTagInfo::GetImage()
{
    return GetXiphCommentCoverArt( m_XiphComment );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::SetImage( const wxImage * image )
{
    return SetXiphCommentCoverArt( m_XiphComment, image );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guOggTagInfo::GetLyrics()
{
    return GetXiphCommentLyrics( m_XiphComment );
}

// -------------------------------------------------------------------------------- //
bool guOggTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetXiphCommentLyrics( m_XiphComment, lyrics );
}


// -------------------------------------------------------------------------------- //
// guMp4TagInfo
// -------------------------------------------------------------------------------- //
guMp4TagInfo::guMp4TagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if (m_TagFile && !m_TagFile->isNull())
        m_Mp4Tag = ((TagLib::MP4::File *) m_TagFile->file())->tag();
    else
        m_Mp4Tag = nullptr;
}

// -------------------------------------------------------------------------------- //
guMp4TagInfo::~guMp4TagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::Read()
{
    if (!guTagInfo::Read())
        return false;

    ReadExtendedTags( m_Mp4Tag );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_Mp4Tag, changedflag );
    return guTagInfo::Write( changedflag );
}

#ifdef TAGLIB_WITH_MP4_COVERS
// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMp4TagInfo::GetImage()
{
    return GetMp4Image( m_Mp4Tag );
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::SetImage( const wxImage * image )
{
    return SetMp4Image( m_Mp4Tag, image );
}
#endif

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMp4TagInfo::GetLyrics()
{
    if (m_TagFile)
    {
        auto * TagFile = (TagLib::MP4::File *) m_TagFile->file();
        if (TagFile)
            return GetMp4Lyrics(TagFile->tag());
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guMp4TagInfo::SetLyrics( const wxString &lyrics )
{
    if (m_TagFile)
    {
        auto * TagFile = (TagLib::MP4::File *) m_TagFile->file();
        if (TagFile)
            return SetMp4Lyrics(TagFile->tag(), lyrics);
    }
    return false;
}


// -------------------------------------------------------------------------------- //
// guMpcTagInfo
// -------------------------------------------------------------------------------- //
guMpcTagInfo::guMpcTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if (m_TagFile && !m_TagFile->isNull())
        m_ApeTag = ((TagLib::MPC::File *) m_TagFile->file())->APETag();
    else
        m_ApeTag = nullptr;
}

// -------------------------------------------------------------------------------- //
guMpcTagInfo::~guMpcTagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::Read()
{
    if (guTagInfo::Read())
        return false;

    ReadExtendedTags(m_ApeTag);
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ApeTag, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMpcTagInfo::GetImage()
{
    return GetApeImage( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guMpcTagInfo::SetImage( const wxImage * image )
{
    //return m_ApeTag && SetApeImage( m_ApeTag, image ) && Write();
    return m_ApeTag && SetApeImage( m_ApeTag, image );
}


// -------------------------------------------------------------------------------- //
// guWavPackTagInfo
// -------------------------------------------------------------------------------- //
guWavPackTagInfo::guWavPackTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if (m_TagFile && !m_TagFile->isNull())
        m_ApeTag = ((TagLib::WavPack::File *) m_TagFile->file())->APETag();
    else
        m_ApeTag = nullptr;
}

// -------------------------------------------------------------------------------- //
guWavPackTagInfo::~guWavPackTagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::Read()
{
    if (!guTagInfo::Read())
        return false;

    ReadExtendedTags(m_ApeTag);
    return true;
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ApeTag, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guWavPackTagInfo::GetImage()
{
    return GetApeImage( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::SetImage( const wxImage * image )
{
    return m_ApeTag && SetApeImage( m_ApeTag, image );
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guWavPackTagInfo::GetLyrics()
{
    return GetApeLyrics( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guWavPackTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetApeLyrics( m_ApeTag, lyrics );
}


#ifdef TAGLIB_WITH_APE_SUPPORT
// -------------------------------------------------------------------------------- //
// guApeTagInfo
// -------------------------------------------------------------------------------- //
guApeTagInfo::guApeTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    m_TagId3v1 = nullptr;
    m_ApeTag = nullptr;
    if (m_TagFile && !m_TagFile->isNull())
    {
        m_TagId3v1 = ((TagLib::APE::File *) m_TagFile->file())->ID3v1Tag();
        m_ApeTag = ((TagLib::APE::File *) m_TagFile->file())->APETag();
    }
}

// -------------------------------------------------------------------------------- //
guApeTagInfo::~guApeTagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Read()
{
    if (!guTagInfo::Read())
        return false;

    ReadExtendedTags( m_ApeTag );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ApeTag, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guApeTagInfo::GetLyrics()
{
    return GetApeLyrics( m_ApeTag );
}

// -------------------------------------------------------------------------------- //
bool guApeTagInfo::SetLyrics( const wxString &lyrics )
{
    return SetApeLyrics( m_ApeTag, lyrics );
}
#endif


// -------------------------------------------------------------------------------- //
// guTrueAudioTagInfo
// -------------------------------------------------------------------------------- //
guTrueAudioTagInfo::guTrueAudioTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if (m_TagFile && !m_TagFile->isNull())
        m_TagId3v2 = ((TagLib::TrueAudio::File *) m_TagFile->file())->ID3v2Tag();
    else
        m_TagId3v2 = nullptr;
}

// -------------------------------------------------------------------------------- //
guTrueAudioTagInfo::~guTrueAudioTagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::Read()
{
    if (guTagInfo::Read())
    {
        // If its a ID3v2 Tag try to load the labels
        ReadExtendedTags(m_TagId3v2);
        return true;
    }

    guLogError(wxT("Could not read tags from file '%s'"), m_FileName.c_str());
    return false;
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_TagId3v2, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guTrueAudioTagInfo::GetImage()
{
    if (m_TagId3v2)
        return GetID3v2Image(m_TagId3v2);
    return nullptr;
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::SetImage( const wxImage * image )
{
    if (!m_TagId3v2)
        return false;

    SetID3v2Image( m_TagId3v2, image );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guTrueAudioTagInfo::GetLyrics()
{
    if (m_TagId3v2)
        return GetID3v2Lyrics(m_TagId3v2);

    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guTrueAudioTagInfo::SetLyrics( const wxString &lyrics )
{
	if (!m_TagId3v2)
        return false;

    SetID3v2Lyrics( m_TagId3v2, lyrics );
    return true;
}


// -------------------------------------------------------------------------------- //
// guASFTagInfo
// -------------------------------------------------------------------------------- //
guASFTagInfo::guASFTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    if (m_TagFile && !m_TagFile->isNull())
        m_ASFTag = ((TagLib::ASF::File *) m_TagFile->file())->tag();
    else
        m_ASFTag = nullptr;
}

// -------------------------------------------------------------------------------- //
guASFTagInfo::~guASFTagInfo() = default;

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::Read()
{
    if (guTagInfo::Read())
    {
        ReadExtendedTags( m_ASFTag );
        return true;
    }

    guLogError(wxT("Could not read tags from file '%s'"), m_FileName.c_str());
    return false;
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::Write( const int changedflag )
{
    WriteExtendedTags( m_ASFTag, changedflag );
    return guTagInfo::Write( changedflag );
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::CanHandleImages()
{
    return false;
}

// -------------------------------------------------------------------------------- //
wxImage * guASFTagInfo::GetImage()
{
    if (m_ASFTag)
        return GetASFImage( m_ASFTag );
    return nullptr;
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::SetImage( const wxImage * image )
{
    if (!m_ASFTag)
        return false;

    SetASFImage( m_ASFTag, image );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::CanHandleLyrics()
{
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guASFTagInfo::GetLyrics()
{
    if (m_ASFTag && m_ASFTag->attributeListMap().contains("WM/Lyrics"))
        return TStringTowxString(m_ASFTag->attributeListMap()["WM/Lyrics"].front().toString());
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
bool guASFTagInfo::SetLyrics( const wxString &lyrics )
{
	if (!m_ASFTag)
        return false;

    m_ASFTag->removeItem( "WM/Lyrics" );
    m_ASFTag->setAttribute( "WM/Lyrics", wxStringToTString( lyrics ) );
    return true;
}


// -------------------------------------------------------------------------------- //
// guGStreamerTagInfo
// -------------------------------------------------------------------------------- //
guGStreamerTagInfo::guGStreamerTagInfo( const wxString &filename ) : guTagInfo( filename )
{
    guLogDebug("guGStreamerTagInfo::guGStreamerTagInfo");
    if( m_TagFile && !m_TagFile->isNull() )
        ReadGStreamerTags( m_TagFile->file()->name() );
}

// -------------------------------------------------------------------------------- //
guGStreamerTagInfo::~guGStreamerTagInfo()
{
    guLogDebug("guGStreamerTagInfo::~guGStreamerTagInfo");
    if ( m_GstTagList != nullptr )
        gst_tag_list_unref( (GstTagList *)m_GstTagList );
}

// -------------------------------------------------------------------------------- //
wxString guGStreamerTagInfo::GetGstStrTag( const gchar * tag )
{
    gchar * gc_val;
    if ((m_GstTagList != nullptr) && gst_tag_list_get_string(m_GstTagList, tag, &gc_val)) 
    {
        wxString res = wxString::FromUTF8(gc_val);
        g_free(gc_val);
        return res;
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
int guGStreamerTagInfo::GetGstIntTag( const gchar * tag )
{
    gint gc_val;
    if ((m_GstTagList != nullptr) && gst_tag_list_get_int(m_GstTagList, tag, &gc_val)) 
        return gc_val;
    return 0;
}

// -------------------------------------------------------------------------------- //
bool guGStreamerTagInfo::GetGstBoolTag( const gchar * tag )
{
    gboolean gc_val;
    if ((m_GstTagList != nullptr) && gst_tag_list_get_boolean(m_GstTagList, tag, &gc_val)) 
        return gc_val;
    return false;
}

// -------------------------------------------------------------------------------- //
GDateTime * guGStreamerTagInfo::GetGstTimeTag( const gchar * tag )
{
    if( m_GstTagList == nullptr )
        return nullptr;

    GstDateTime *dt;
    GDateTime *res = nullptr;
    GDate *gd;
    if( gst_tag_list_get_date_time( m_GstTagList, tag, &dt ) ) {
        res = gst_date_time_to_g_date_time(dt);
        gst_date_time_unref(dt);
    }
    else if( gst_tag_list_get_date( m_GstTagList, tag, &gd ) )
    {
        res = g_date_time_new_local( gd->year, gd->month, gd->day, 0, 0, 0 );
        g_free(gd);
    }
    return res;
}

// -------------------------------------------------------------------------------- //
bool guGStreamerTagInfo::Read()
{
    guLogDebug("guGStreamerTagInfo::Read");

    if (!ReadGStreamerTags(m_FileName))
        return false;

    m_TrackName = GetGstStrTag(GST_TAG_TITLE);
    m_ArtistName = GetGstStrTag(GST_TAG_ARTIST);
    m_AlbumName = GetGstStrTag(GST_TAG_ALBUM);
    m_GenreName = GetGstStrTag(GST_TAG_GENRE);
    m_Comments = GetGstStrTag(GST_TAG_COMMENT);
    m_Track = GetGstIntTag(GST_TAG_TRACK_NUMBER);
    m_AlbumArtist = GetGstStrTag(GST_TAG_ALBUM_ARTIST);
    m_Composer = GetGstStrTag(GST_TAG_COMPOSER);
    GDateTime * gd = GetGstTimeTag(GST_TAG_DATE);

    if (gd != nullptr)
        gd = GetGstTimeTag(GST_TAG_DATE_TIME);

    if (gd != nullptr)
    {
        m_Year = g_date_time_get_year(gd);
        g_date_time_unref(gd);
    }

    m_Rating = GetGstIntTag(GST_TAG_USER_RATING);
    return true;
}

// -------------------------------------------------------------------------------- //
bool guGStreamerTagInfo::CanHandleImages()
{
    return true;
}

// -------------------------------------------------------------------------------- //
bool guGStreamerTagInfo::ReadGStreamerTags( const wxString &filename )
{
    guLogDebug("guGStreamerTagInfo::ReadGStreamerTags");

    gchar *uri;
    if( gst_uri_is_valid ( filename.c_str() ) )
        uri = g_strdup( ( const gchar * ) filename.c_str() );
    else
        uri = gst_filename_to_uri( filename.c_str(), nullptr );

    GError * err = nullptr;
    GstDiscoverer * dis = nullptr;
    GstDiscovererInfo * info = nullptr;

    dis = gst_discoverer_new( 10 * GST_SECOND, &err );

    if( dis == nullptr )
    {
        guLogWarning( wxT("gst_discoverer_new error: %s"), err->message );
        g_free(uri);
        return false;
    }

    info = gst_discoverer_discover_uri( dis, uri, &err );
    g_free(uri);

    if( info == nullptr )
    {
        guLogWarning( wxT("gst_discoverer_discover_uri error: %s"), err->message );
        return false;
    }

    m_Length = gst_discoverer_info_get_duration( info ) / 1000000;
    guLogDebug("guGStreamerTagInfo::ReadGStreamerTags length: %u", m_Length);

    GList *l, *slist = gst_discoverer_info_get_streams( info, g_type_from_name( "GstDiscovererAudioInfo" ) );
    for( l = slist; l != nullptr; l = l->next )
    {
        if ( !m_Bitrate )
            m_Bitrate = gst_discoverer_audio_info_get_bitrate((const GstDiscovererAudioInfo*)l->data);
        if ( !m_Bitrate )
            m_Bitrate = gst_discoverer_audio_info_get_max_bitrate((const GstDiscovererAudioInfo*)l->data);
        guLogDebug( "guGStreamerTagInfo::ReadGStreamerTags bitrate: %u", m_Bitrate );
    }
    gst_discoverer_stream_info_list_free(slist);

    if ( m_GstTagList != nullptr )
        gst_tag_list_unref  ( (GstTagList *)m_GstTagList );

    m_GstTagList = gst_discoverer_info_get_tags( info );

    if ( m_GstTagList != nullptr )
    {
        gchar * str_tags = gst_tag_list_to_string( m_GstTagList );
        guLogDebug( "guGStreamerTagInfo::ReadGStreamerTags got tags: '%s'", str_tags );
        g_free( str_tags );
        return !( gst_tag_list_is_empty( m_GstTagList ) );
    }
    else
        guLogDebug( "guGStreamerTagInfo::ReadGStreamerTags tags not found" );

    return false;
}

// -------------------------------------------------------------------------------- //
wxString    guGStreamerTagInfo::GetLyrics()
{
    return GetGstStrTag( GST_TAG_LYRICS );
}

// -------------------------------------------------------------------------------- //
wxImage *guGStreamerTagInfo::GetImage()
{
    guLogDebug("guGStreamerTagInfo::GetImage");

    if( m_GStreamerImage != nullptr )
        return m_GStreamerImage;

    const char *uri, *param = (const char*)m_FileName.mb_str();

    if( gst_uri_is_valid( param ) )
        uri = param;
    else
        uri = gst_filename_to_uri( param, nullptr );

    wxString m_line = "uridecodebin uri=" + wxString( uri ) +
                      " ! jpegenc snapshot=TRUE quality=100 " +
                      " ! fakesink sync=false enable-last-sample=true name=sink";

    guGstElementStatePtr pipeline_gp( gst_parse_launch( (const char *)m_line.mb_str(), nullptr ) );
    GstElement *pipeline = pipeline_gp.ptr;

    if( pipeline == nullptr )
        return nullptr;

    // smrt ptr
    guGstPtr<GstElement> sink_gp( gst_bin_get_by_name( GST_BIN( pipeline ), "sink" ) );
    GstElement *sink = sink_gp.ptr;

    if( sink == nullptr )
        return nullptr;

    GstStateChangeReturn ret = gst_element_set_state( pipeline, GST_STATE_PLAYING );
    if( ret == GST_STATE_CHANGE_FAILURE )
        return nullptr;

    // smrt ptr
    guGstPtr<GstBus> bus_gp( gst_element_get_bus( pipeline ) );
    GstBus *bus = bus_gp.ptr;

    if( bus == nullptr )
        return nullptr;

    // weak ref for scope-limited msg
    GstMessage * msg_wref;
    do
    {
        // no msg in 5 sec => we just fail
        guGstMessagePtr msg_gp( gst_bus_timed_pop( bus, 5 * GST_SECOND ) );
        GstMessage *msg = msg_gp.ptr;

        if( msg != nullptr )
        {
            guLogDebug( "guGStreamerTagInfo::GetImage message type <%s>", GST_MESSAGE_TYPE_NAME( msg ) );
            switch( GST_MESSAGE_TYPE( msg ) )
            {
                case GST_MESSAGE_STATE_CHANGED:
                    #ifdef GU_DEBUG
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed( msg, &old_state, &new_state, &pending_state);
                    guLogDebug( "guGStreamerTagInfo::GetImage %s state change %s -> %s:\n",
                        GST_OBJECT_NAME( GST_MESSAGE_SRC( msg ) ),
                        gst_element_state_get_name( old_state ),
                        gst_element_state_get_name( new_state )
                        );
                    #endif
                    break;
                case GST_MESSAGE_ERROR:
                case GST_MESSAGE_EOS:
                case GST_MESSAGE_ASYNC_DONE:
                    msg = nullptr;
                    break;
                default:
                    guLogDebug( "guGStreamerTagInfo::GetImage unknown message: %s", GST_MESSAGE_TYPE_NAME( msg ) );
                    break;
            }
        }
        msg_wref = msg;
    }
    while( msg_wref != nullptr );

    GstSample * spl;
    g_object_get( G_OBJECT( sink ), "last-sample", &spl, nullptr) ;
    // unref:g_object_unref( spl )

    if( spl != nullptr )
    {
        guLogDebug( "guGStreamerTagInfo::GetImage got the last sample" );
        GstBuffer * buf = gst_sample_get_buffer( spl );
        if( buf != nullptr )
        {
            guLogDebug( "guGStreamerTagInfo::GetImage buff size: %lu",
                gst_buffer_get_size( buf ) );
            GstMapInfo gmi;
            if( gst_buffer_map( buf, &gmi, GST_MAP_READ ) )
            {
                guLogDebug( "guGStreamerTagInfo::GetImage map ok" );
                wxMemoryInputStream mis( gmi.data, gmi.size );
                m_GStreamerImage = new wxImage( mis, wxBITMAP_TYPE_JPEG );
            }
        }
        if( G_IS_OBJECT( spl ) )
            g_object_unref( spl );
    }

    guLogDebug( "guGStreamerTagInfo::GetImage ret" );

    if (m_GStreamerImage != nullptr)
        return m_GStreamerImage;
    return nullptr;
}


// -------------------------------------------------------------------------------- //
// Other functions
// -------------------------------------------------------------------------------- //
wxImage * guTagGetPicture( const wxString &filename )
{
    wxImage * RetVal = nullptr;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleImages() )
            RetVal = TagInfo->GetImage();
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guTagSetPicture( const wxString &filename, wxImage * picture, const bool forcesave )
{
    auto * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();

    const guCurrentTrack * CurrentTrack = MainFrame->GetCurrentTrack();
    if( !forcesave && CurrentTrack && CurrentTrack->m_Loaded )
    {
        if( CurrentTrack->m_FileName == filename )
        {
            // Add the pending track change to MainFrame
            MainFrame->AddPendingUpdateTrack( filename, picture, wxEmptyString, guTRACK_CHANGED_DATA_IMAGES );
            return true;
        }
    }

    bool RetVal = false;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleImages() )
            RetVal = TagInfo->SetImage( picture ) && TagInfo->Write( guTRACK_CHANGED_DATA_IMAGES );
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guTagSetPicture( const wxString &filename, const wxString &imagefile, const bool forcesave )
{
    wxImage Image( imagefile );
    if( Image.IsOk() )
        return guTagSetPicture( filename, &Image, forcesave );
    return false;
}

// -------------------------------------------------------------------------------- //
wxString guTagGetLyrics( const wxString &filename )
{
    wxString RetVal = wxEmptyString;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleLyrics() )
            RetVal = TagInfo->GetLyrics();
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guTagSetLyrics( const wxString &filename, const wxString &lyrics, const bool forcesave )
{
    auto * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();

    const guCurrentTrack * CurrentTrack = MainFrame->GetCurrentTrack();
    if( !forcesave && CurrentTrack && CurrentTrack->m_Loaded )
    {
        if( CurrentTrack->m_FileName == filename )
        {
            // Add the pending track change to MainFrame
            MainFrame->AddPendingUpdateTrack( filename, nullptr, lyrics, guTRACK_CHANGED_DATA_LYRICS );
            return true;
        }
    }

    bool RetVal = false;
    guTagInfo * TagInfo = guGetTagInfoHandler( filename );
    if( TagInfo )
    {
        if( TagInfo->CanHandleLyrics() )
            RetVal = TagInfo->SetLyrics( lyrics ) && TagInfo->Write( guTRACK_CHANGED_DATA_LYRICS );
        delete TagInfo;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guUpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                    const wxArrayString &lyrics, const wxArrayInt &changedflags, const bool forcesave )
{
    guMainFrame * MainFrame = guMainFrame::GetMainFrame();

    // Process each Track
    int Count = tracks.Count();
    for (int Index = 0; Index < Count; Index++)
    {
        // If there is nothign to change continue with next one
        int ChangedFlag = changedflags[Index];
        if (!ChangedFlag)
            continue;

        const guTrack &Track = tracks[Index];

        // Dont allow to edit tags from Cue files tracks
        if (Track.m_Offset)
            continue;

        if (!wxFileExists(Track.m_FileName))
        {
            guLogMessage(wxT("File not found for edition: '%s'"), Track.m_FileName.c_str());
            continue;
        }

        // Prevent write to the current playing file in order to avoid segfaults specially with flac and wma files
        const guCurrentTrack * CurrentTrack = MainFrame->GetCurrentTrack();
        if (!forcesave && CurrentTrack && CurrentTrack->m_Loaded)
        {
            if (CurrentTrack->m_FileName == Track.m_FileName)
            {
                // Add the pending track change to MainFrame
                MainFrame->AddPendingUpdateTrack(
                        Track,
                        Index < (int) images.Count() ? images[Index] : nullptr,
                        Index < (int) lyrics.Count() ? lyrics[Index] : wxT(""),
                        changedflags[Index]);
                continue;
            }
        }

        guTagInfo * TagInfo = guGetTagInfoHandler(Track.m_FileName);

        if (!TagInfo)
        {
            guLogError(wxT("There is no handler for the file '%s'"), Track.m_FileName.c_str());
            return;
        }

        if (ChangedFlag & guTRACK_CHANGED_DATA_TAGS)
        {
            TagInfo->m_TrackName = Track.m_SongName;
            TagInfo->m_AlbumArtist = Track.m_AlbumArtist;
            TagInfo->m_ArtistName = Track.m_ArtistName;
            TagInfo->m_AlbumName = Track.m_AlbumName;
            TagInfo->m_GenreName = Track.m_GenreName;
            TagInfo->m_Track = Track.m_Number;
            TagInfo->m_Year = Track.m_Year;
            TagInfo->m_Composer = Track.m_Composer;
            TagInfo->m_Comments = Track.m_Comments;
            TagInfo->m_Disk = Track.m_Disk;
        }

        if (ChangedFlag & guTRACK_CHANGED_DATA_RATING)
        {
            TagInfo->m_Rating = Track.m_Rating;
            TagInfo->m_PlayCount = Track.m_PlayCount;
        }

        if ((ChangedFlag & guTRACK_CHANGED_DATA_LYRICS) && TagInfo->CanHandleLyrics())
            TagInfo->SetLyrics(lyrics[Index]);

        if ((ChangedFlag & guTRACK_CHANGED_DATA_IMAGES) && TagInfo->CanHandleImages())
            TagInfo->SetImage(images[Index]);

        TagInfo->Write(ChangedFlag);
        delete TagInfo;
    }
}

// -------------------------------------------------------------------------------- //
void guUpdateImages( const guTrackArray &songs, const guImagePtrArray &images, const wxArrayInt &changedflags )
{
    int Count = images.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( !songs[ Index ].m_Offset && ( changedflags[ Index ] & guTRACK_CHANGED_DATA_IMAGES ) )
            guTagSetPicture( songs[ Index ].m_FileName, images[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
void guUpdateLyrics( const guTrackArray &songs, const wxArrayString &lyrics, const wxArrayInt &changedflags )
{
    int Count = lyrics.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        if( !songs[ Index ].m_Offset && ( changedflags[ Index ] & guTRACK_CHANGED_DATA_LYRICS ) )
            guTagSetLyrics( songs[ Index ].m_FileName, lyrics[ Index ] );
    }
}

}
