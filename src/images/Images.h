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
#ifndef __IMAGES_H__
#define __IMAGES_H__

#include <wx/bitmap.h>
#include <wx/image.h>

namespace Guayadeque {

typedef enum {
    guIMAGE_INDEX_add = 0,
    guIMAGE_INDEX_blank_cd_cover,
    guIMAGE_INDEX_bookmark,
    guIMAGE_INDEX_default_lastfm_image,
    guIMAGE_INDEX_del,
    guIMAGE_INDEX_doc_new,
    guIMAGE_INDEX_doc_save,
    guIMAGE_INDEX_download_covers,
    guIMAGE_INDEX_down,
    guIMAGE_INDEX_edit_clear,
    guIMAGE_INDEX_edit_copy,
    guIMAGE_INDEX_edit_delete,
    guIMAGE_INDEX_edit,
    guIMAGE_INDEX_exit,
    guIMAGE_INDEX_filter,
    guIMAGE_INDEX_guayadeque,
    guIMAGE_INDEX_guayadeque_taskbar,
    guIMAGE_INDEX_lastfm_as_off,
    guIMAGE_INDEX_lastfm_as_on,
    guIMAGE_INDEX_lastfm_on,
    guIMAGE_INDEX_left,
    guIMAGE_INDEX_net_radio,
    guIMAGE_INDEX_no_cover,
    guIMAGE_INDEX_no_photo,
    guIMAGE_INDEX_numerate,
    guIMAGE_INDEX_right,
    guIMAGE_INDEX_search,
    guIMAGE_INDEX_splash,
    guIMAGE_INDEX_system_run,
    guIMAGE_INDEX_tags,
    guIMAGE_INDEX_tiny_accept,
    guIMAGE_INDEX_tiny_add,
    guIMAGE_INDEX_tiny_del,
    guIMAGE_INDEX_up,
    guIMAGE_INDEX_track,
    guIMAGE_INDEX_tiny_search,
    guIMAGE_INDEX_search_engine,
    guIMAGE_INDEX_musicbrainz,
    guIMAGE_INDEX_tiny_edit,
    guIMAGE_INDEX_tiny_edit_copy,
    guIMAGE_INDEX_tiny_filter,
    guIMAGE_INDEX_tiny_search_again,
    guIMAGE_INDEX_tiny_numerate,
    guIMAGE_INDEX_tiny_edit_clear,
    guIMAGE_INDEX_podcast,
    guIMAGE_INDEX_mid_podcast,
    guIMAGE_INDEX_tiny_podcast,
    guIMAGE_INDEX_tiny_status_pending,
    guIMAGE_INDEX_tiny_status_error,
    guIMAGE_INDEX_tiny_doc_save,
    guIMAGE_INDEX_tiny_reload,
    guIMAGE_INDEX_tiny_shoutcast,
    guIMAGE_INDEX_tiny_tunein,
    guIMAGE_INDEX_tiny_net_radio,
    guIMAGE_INDEX_tiny_left,
    guIMAGE_INDEX_tiny_right,
    guIMAGE_INDEX_tiny_search_engine,
    guIMAGE_INDEX_tiny_library,
    guIMAGE_INDEX_tiny_record,
    guIMAGE_INDEX_pref_commands,
    guIMAGE_INDEX_pref_copy_to,
    guIMAGE_INDEX_pref_general,
    guIMAGE_INDEX_pref_last_fm,
    guIMAGE_INDEX_pref_library,
    guIMAGE_INDEX_pref_links,
    guIMAGE_INDEX_pref_lyrics,
    guIMAGE_INDEX_pref_online_services,
    guIMAGE_INDEX_pref_playback,
    guIMAGE_INDEX_pref_podcasts,
    guIMAGE_INDEX_pref_record,
    guIMAGE_INDEX_pref_crossfader,
    guIMAGE_INDEX_pref_jamendo,
    guIMAGE_INDEX_pref_magnatune,
    guIMAGE_INDEX_pref_accelerators,
    //
    guIMAGE_INDEX_loc_library,
    guIMAGE_INDEX_loc_portable_device,
    guIMAGE_INDEX_loc_net_radio,
    guIMAGE_INDEX_loc_podcast,
    guIMAGE_INDEX_loc_magnatune,
    guIMAGE_INDEX_loc_jamendo,
    guIMAGE_INDEX_loc_lastfm,
    guIMAGE_INDEX_loc_lyrics,
    //
    guIMAGE_INDEX_tiny_close_normal,
    guIMAGE_INDEX_tiny_close_highlight,
    //
    guIMAGE_INDEX_player_highlight_equalizer,
    guIMAGE_INDEX_player_highlight_muted,
    guIMAGE_INDEX_player_highlight_next,
    guIMAGE_INDEX_player_highlight_pause,
    guIMAGE_INDEX_player_highlight_play,
    guIMAGE_INDEX_player_highlight_prev,
    guIMAGE_INDEX_player_highlight_record,
    guIMAGE_INDEX_player_highlight_repeat,
    guIMAGE_INDEX_player_highlight_repeat_single,
    guIMAGE_INDEX_player_highlight_search,
    guIMAGE_INDEX_player_highlight_setup,
    guIMAGE_INDEX_player_highlight_smart,
    guIMAGE_INDEX_player_highlight_stop,
    guIMAGE_INDEX_player_highlight_vol_hi,
    guIMAGE_INDEX_player_highlight_vol_low,
    guIMAGE_INDEX_player_highlight_vol_mid,
    guIMAGE_INDEX_player_highlight_love,
    guIMAGE_INDEX_player_highlight_ban,
    guIMAGE_INDEX_player_highlight_crossfading,
    guIMAGE_INDEX_player_highlight_gapless,
    guIMAGE_INDEX_player_light_equalizer,
    guIMAGE_INDEX_player_light_muted,
    guIMAGE_INDEX_player_light_next,
    guIMAGE_INDEX_player_light_pause,
    guIMAGE_INDEX_player_light_play,
    guIMAGE_INDEX_player_light_prev,
    guIMAGE_INDEX_player_light_record,
    guIMAGE_INDEX_player_light_repeat,
    guIMAGE_INDEX_player_light_repeat_single,
    guIMAGE_INDEX_player_light_search,
    guIMAGE_INDEX_player_light_setup,
    guIMAGE_INDEX_player_light_smart,
    guIMAGE_INDEX_player_light_stop,
    guIMAGE_INDEX_player_light_vol_hi,
    guIMAGE_INDEX_player_light_vol_low,
    guIMAGE_INDEX_player_light_vol_mid,
    guIMAGE_INDEX_player_light_love,
    guIMAGE_INDEX_player_light_ban,
    guIMAGE_INDEX_player_light_crossfading,
    guIMAGE_INDEX_player_light_gapless,
    guIMAGE_INDEX_player_normal_equalizer,
    guIMAGE_INDEX_player_normal_muted,
    guIMAGE_INDEX_player_normal_next,
    guIMAGE_INDEX_player_normal_pause,
    guIMAGE_INDEX_player_normal_play,
    guIMAGE_INDEX_player_normal_prev,
    guIMAGE_INDEX_player_normal_record,
    guIMAGE_INDEX_player_normal_repeat,
    guIMAGE_INDEX_player_normal_repeat_single,
    guIMAGE_INDEX_player_normal_search,
    guIMAGE_INDEX_player_normal_setup,
    guIMAGE_INDEX_player_normal_smart,
    guIMAGE_INDEX_player_normal_stop,
    guIMAGE_INDEX_player_normal_vol_hi,
    guIMAGE_INDEX_player_normal_vol_low,
    guIMAGE_INDEX_player_normal_vol_mid,
    guIMAGE_INDEX_player_normal_love,
    guIMAGE_INDEX_player_normal_ban,
    guIMAGE_INDEX_player_normal_crossfading,
    guIMAGE_INDEX_player_normal_gapless,
    guIMAGE_INDEX_player_tiny_light_play,
    guIMAGE_INDEX_player_tiny_light_stop,
    guIMAGE_INDEX_player_tiny_red_stop,
    guIMAGE_INDEX_star_normal_tiny,
    guIMAGE_INDEX_star_normal_mid,
    guIMAGE_INDEX_star_normal_big,
    guIMAGE_INDEX_star_highlight_tiny,
    guIMAGE_INDEX_star_highlight_mid,
    guIMAGE_INDEX_star_highlight_big,
    //
    guIMAGE_INDEX_tiny_mv_library,
    guIMAGE_INDEX_tiny_mv_albumbrowser,
    guIMAGE_INDEX_tiny_mv_treeview,
    guIMAGE_INDEX_tiny_mv_playlists,
    guIMAGE_INDEX_tiny_start,
    guIMAGE_INDEX_go_top,
    guIMAGE_INDEX_go_up,
    guIMAGE_INDEX_go_down,
    guIMAGE_INDEX_go_bottom,
    guIMAGE_INDEX_tiny_shuffle,
    guIMAGE_INDEX_COUNT

} guIMAGE_INDEX;


// -------------------------------------------------------------------------------- //
wxBitmap guBitmap( guIMAGE_INDEX imageindex );
wxImage guImage( guIMAGE_INDEX imageindex );

}

#endif
// -------------------------------------------------------------------------------- //
