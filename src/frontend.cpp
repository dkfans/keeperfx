/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontend.cpp
 *     Frontend menu implementation for Dungeon Keeper.
 * @par Purpose:
 *     Functions to display and maintain the game menu.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2008 - 21 Apr 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "frontend.h"

#include <string.h>
#include "bflib_basics.h"
#include "globals.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_dernc.h"
#include "bflib_datetm.h"
#include "bflib_keybrd.h"
#include "bflib_inputctrl.h"
#include "bflib_sndlib.h"
#include "bflib_mouse.h"
#include "bflib_vidraw.h"
#include "bflib_fileio.h"
#include "bflib_memory.h"
#include "bflib_filelst.h"
#include "bflib_sound.h"
#include "bflib_network.h"
#include "config.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_magic.h"
#include "scrcapt.h"
#include "gui_draw.h"
#include "kjm_input.h"
#include "vidmode.h"
#include "front_simple.h"
#include "front_input.h"
#include "front_fmvids.h"
#include "game_saves.h"
#include "engine_render.h"
#include "engine_redraw.h"
#include "front_landview.h"
#include "front_credits.h"
#include "front_torture.h"
#include "front_highscore.h"
#include "front_lvlstats.h"
#include "front_easter.h"
#include "front_network.h"
#include "frontmenu_net.h"
#include "frontmenu_options.h"
#include "frontmenu_specials.h"
#include "frontmenu_saves.h"
#include "frontmenu_select.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_opts.h"
#include "lvl_filesdk1.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "power_hand.h"
#include "magic.h"
#include "player_instances.h"
#include "player_utils.h"
#include "player_states.h"
#include "gui_frontmenu.h"
#include "gui_frontbtns.h"
#include "gui_soundmsgs.h"
#include "vidfade.h"
#include "config_settings.h"
#include "config_strings.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#include "music_player.h"
#include "custom_sprites.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void __stdcall enum_sessions_callback(struct TbNetworkCallbackData *netcdat, void *ptr);
/******************************************************************************/
TbClockMSec gui_message_timeout = 0;
char gui_message_text[TEXT_BUFFER_LENGTH];

struct GuiButtonInit frontend_main_menu_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,                 0, 999,  26, 999,  26, 371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,       {1},            0, NULL },
  { 0,  0, 0, 0, frontend_start_new_game,NULL,frontend_over_button,     3, 999,  92, 999,  92, 371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,       {2},            0, NULL },
  { 0,  0, 0, 0, frontend_load_continue_game,NULL,frontend_over_button, 0, 999, 138, 999, 138, 371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,       {8},            0, frontend_continue_game_maintain },
  { 0,  0, 0, 0, frontend_load_mappacks,NULL,frontend_over_button,     34, 999, 184, 999, 184, 371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,     {106},            0, frontend_mappacks_maintain },
  { 0,  0, 0, 0, frontend_change_state,NULL, frontend_over_button,    2, 999, 230,   999, 230, 371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,       {3},            0, frontend_main_menu_load_game_maintain },
  { 0,  0, 0, 0, frontend_netservice_change_state,NULL, frontend_over_button,4,999,276,999,276,371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,       {4},            0, frontend_main_menu_netservice_maintain },
  { 0,  0, 0, 0, frontend_change_state,NULL, frontend_over_button,   27, 999, 322,   999, 322, 371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,      {97},            0, NULL },
  { 0,  0, 0, 0, frontend_ldcampaign_change_state,NULL, frontend_over_button,18,999,368,999,368,371,46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,     {104},            0, frontend_main_menu_highscores_maintain },
  { 0,  0, 0, 0, frontend_change_state,NULL, frontend_over_button,      9, 999, 414, 999, 414, 371, 46, frontend_draw_large_menu_button,  0, GUIStr_Empty,  0,       {5},            0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,                 0,   0,   0,   0,   0,   0,  0, NULL,                             0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_statistics_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,                 0, 999,  30, 999,  30,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,      {84},            0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,                 0, 999,  90, 999,  90,450,162, frontstats_draw_main_stats,        0, GUIStr_Empty,  0,       {0},            0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,                 0, 999, 260, 999, 260,450,136, frontstats_draw_scrolling_stats,   0, GUIStr_Empty,  0,       {0},            0, NULL },
  { 0,  0, 0, 0, frontstats_leave,NULL,frontend_over_button,           18, 999, 404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,      {83},            0, NULL },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,                 0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_high_score_score_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,                 0, 999,  30, 999,  30,495, 46, frontend_draw_vlarge_menu_button,  0, GUIStr_Empty,  0,      {85},            0, NULL },
  { 0,  0, 0, 0, NULL,               NULL,        NULL,                 0, 999,  97, 999,  97,450,286, frontend_draw_high_score_table,    0, GUIStr_Empty,  0,       {0},            0, NULL },
  { 0,  0, 0, 0, frontend_quit_high_score_table,NULL,frontend_over_button,3,999,404, 999, 404,371, 46, frontend_draw_large_menu_button,   0, GUIStr_Empty,  0,      {83},            0, frontend_maintain_high_score_ok_button },
  {-1,  0, 0, 0, NULL,               NULL,        NULL,                 0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};

struct GuiButtonInit frontend_error_box_buttons[] = {
  { 0,  0, 0, 0, NULL,               NULL,        NULL,                 0, 999,   0, 999,   0,450, 92, frontend_draw_error_text_box,      0, GUIStr_Empty,  0,{(long)gui_message_text},0, frontend_maintain_error_text_box},
  {-1,  0, 0, 0, NULL,               NULL,        NULL,                 0,   0,   0,   0,   0,  0,  0, NULL,                              0, GUIStr_Empty,  0,       {0},            0, NULL },
};


struct GuiMenu frontend_main_menu =
 { GMnu_FEMAIN,             0, 1, frontend_main_menu_buttons, POS_SCRCTR,POS_SCRCTR, 640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_statistics_menu =
 { GMnu_FESTATISTICS,       0, 1, frontend_statistics_buttons,POS_SCRCTR,POS_SCRCTR, 640, 480, NULL, 0, NULL,    NULL,                    0, 0, 0,};
struct GuiMenu frontend_high_score_table_menu =
 { GMnu_FEHIGH_SCORE_TABLE, 0, 1, frontend_high_score_score_buttons,POS_SCRCTR,POS_SCRCTR, 640, 480, NULL, 0, NULL,NULL,                  0, 0, 0,};
struct GuiMenu frontend_error_box = // Error box has no background defined - the buttons drawing adds it
 { GMnu_FEERROR_BOX,        0, 1, frontend_error_box_buttons,POS_GAMECTR,POS_GAMECTR, 450,  92, NULL,                        0, NULL,    NULL,                    0, 1, 0,};

// Note: update size in .h file when changing this array.
struct GuiMenu *menu_list[] = {
    NULL,
    &main_menu,
    &room_menu,
    &spell_menu,
    &trap_menu,
    &creature_menu,
    &event_menu,
    &query_menu,
    &options_menu,
    &instance_menu,
    &quit_menu,//10
    &load_menu,
    &save_menu,
    &video_menu,
    &sound_menu,
    &error_box,
    &text_info_menu,
    &hold_audience_menu,
    &frontend_main_menu,
    &frontend_load_menu,
    &frontend_net_service_menu,//20
    &frontend_net_session_menu,
    &frontend_net_start_menu,
    &frontend_net_modem_menu,
    &frontend_net_serial_menu,
    &frontend_statistics_menu,
    &frontend_high_score_table_menu,
    &dungeon_special_menu,
    &resurrect_creature_menu,
    &transfer_creature_menu,
    &armageddon_menu,//30
    &creature_query_menu1,
    &creature_query_menu3,
    &creature_query_menu4,
    &battle_menu,
    &creature_query_menu2,
    &frontend_define_keys_menu,
    &autopilot_menu,
    &spell_lost_menu,
    &frontend_option_menu,
    &frontend_select_level_menu,//40
    &frontend_select_campaign_menu,
    &frontend_error_box,
    &frontend_add_session_box,
    &frontend_select_mappack_menu,
    &message_box,
    NULL,
};

/** Array used for mapping buttons to text messages.
 *  Index in this array is accepted as value of button 'content' property.
 *  If adding entries here, you should also update FRONTEND_BUTTON_INFO_COUNT.
 */
struct FrontEndButtonData frontend_button_info[] = {
    {0,   0}, // [0]
    {GUIStr_MnuMainMenu, 0},
    {GUIStr_MnuStartNewGame, 1},
    {GUIStr_MnuLoadGame, 1},
    {GUIStr_MnuMultiplayer, 1},
    {GUIStr_MnuQuit, 1},
    {GUIStr_MnuReturnToMain, 1},
    {GUIStr_MnuLoadGame, 0},
    {GUIStr_MnuContinueGame, 1},
    {GUIStr_MnuPlayIntro, 1},
    {GUIStr_NetServiceMenu, 0}, // [10]
    {GUIStr_NetSessionMenu, 0},
    {GUIStr_MnuGameMenu, 0}, // [12]
    {GUIStr_NetJoinGame, 1}, // [13]
    {GUIStr_NetCreateGame, 1}, // [14]
    {GUIStr_NetStartGame, 1}, // [15]
    {GUIStr_MnuCancel, 1}, // [16]
    {GUIStr_Empty, 1}, // [17]
    {GUIStr_Empty, 1}, // [18]
    {GUIStr_NetName, 1}, // [19]
    {GUIStr_Empty, 1}, // [20]
    {GUIStr_Empty, 1}, // [21]
    {GUIStr_MnuLevel, 1}, // [22]
    {GUIStr_Empty, 1}, // [23]
    {GUIStr_Empty, 1}, // [24]
    {GUIStr_Empty, 1}, // [25]
    {GUIStr_Empty, 1}, // [26]
    {GUIStr_Empty, 1}, // [27]
    {GUIStr_Empty, 1}, // [28]
    {GUIStr_NetSessions, 2}, // [29]
    {GUIStr_MnuGames, 2}, // [30]
    {GUIStr_MnuPlayers, 2},
    {GUIStr_MnuLevels, 2},
    {GUIStr_NetServices, 2},
    {GUIStr_NetMessages, 2},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1}, // [40]
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1}, // [50]
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_NetModemMenu, 0},
    {GUIStr_NetSerialMenu, 0},
    {GUIStr_NetComPort, 2},
    {GUIStr_NetSpeed, 2},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1}, // [60]
    {GUIStr_NetIrq, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_NetInit, 1},
    {GUIStr_NetHangup, 1},
    {GUIStr_NetDial, 1},
    {GUIStr_NetAnswer, 1},
    {GUIStr_Empty, 1}, // [70]
    {GUIStr_NetPhoneNumber, 1},
    {GUIStr_NetContinue, 1},
    {GUIStr_NetContinue, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1}, // [80]
    {GUIStr_Empty, 1},
    {GUIStr_Credits, 1},
    {GUIStr_MnuOk, 1},
    {GUIStr_MnuStatistics, 0},
    {GUIStr_MnuHighScoreTable, 0},
    {GUIStr_TeamChooseGame, 0},
    {GUIStr_TeamGameType, 2},
    {GUIStr_NetStart, 1},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1}, // [90]
    {GUIStr_Empty, 1},
    {GUIStr_DefineKeys, 0},
    {GUIStr_Empty, 1},
    {GUIStr_Empty, 1},
    {GUIStr_DefineKeys, 1},
    {GUIStr_MnuOptions, 0},
    {GUIStr_MnuOptions, 1},
    {GUIStr_MnuRetToOptions, 1},
    {GUIStr_MnuSoundOptions, 1},
    {GUIStr_MouseOptions, 1}, // [100]
    {GUIStr_Sensitivity, 1},
    {GUIStr_MnuInvertMouse, 1},
    {GUIStr_MnuComputer, 1},
    {GUIStr_MnuHighScoreTable, 1},
    {GUIStr_Empty, 0},
    {GUIStr_MnuFreePlayLevels, 1},
    {GUIStr_MnuFreePlayLevels, 0},
    {GUIStr_MnuLandSelection, 0}, // [108]
    {GUIStr_MnuCampaigns, 2}, // [109]
    {GUIStr_MnuAddComputer, 1}, // [110]
    {GUIStr_MnuReturnToFreePlay, 1},
    {GUIStr_MnuMapPacks, 2},
};

// bttn_sprite, tooltip_stridx, msg_stridx, lifespan_turns, turns_between_events, replace_event_kind_button;
struct EventTypeInfo event_button_info[] = {
  {260, GUIStr_Empty,                       GUIStr_Empty,                      1,   1, EvKind_Nothing},
  {260, GUIStr_EventDnHeartAttackedDesc,    GUIStr_EventHeartAttacked,       300, 250, EvKind_Nothing},
  {262, GUIStr_EventFightDesc,              GUIStr_EventFight,                -1,   0, EvKind_FriendlyFight},
  {258, GUIStr_EventObjective,              GUIStr_Empty,                     -1,   0, EvKind_Objective},
  {260, GUIStr_EventBreachDesc,             GUIStr_EventBreach,              300,   0, EvKind_Nothing},
  {250, GUIStr_EventNewRoomResrchDesc,      GUIStr_EventNewRoomResearched,  1200,   0, EvKind_Nothing},
  {256, GUIStr_EventNewCreatureDesc,        GUIStr_EventNewCreature,        1200,   0, EvKind_Nothing},
  {252, GUIStr_EventNewSpellResrchDesc,     GUIStr_EventNewSpellResearched, 1200,   0, EvKind_Nothing},
  {254, GUIStr_EventNewTrapDesc,            GUIStr_EventNewTrap,            1200,   0, EvKind_Nothing},
  {254, GUIStr_EventNewDoorDesc,            GUIStr_EventNewDoor,            1200,   0, EvKind_Nothing},
  {260, GUIStr_EventCreatrScavngDesc,       GUIStr_EventScavengingDetected, 1200,   0, EvKind_Nothing}, // EvKind_CreatrScavenged
  {266, GUIStr_EventTreasrRoomFullDesc,     GUIStr_EventTreasureRoomFull,   1200, 500, EvKind_Nothing},
  {266, GUIStr_EventCreaturePaydayDesc,     GUIStr_EventCreaturePayday,     1200,   0, EvKind_Nothing},
  {266, GUIStr_EventAreaDiscoveredDesc,     GUIStr_EventAreaDiscovered,     1200,   0, EvKind_Nothing},
  {266, GUIStr_EventSpellPickedUpDesc,      GUIStr_EventNewSpellPickedUp,   1200,   0, EvKind_Nothing},
  {266, GUIStr_EventRoomTakenOverDesc,      GUIStr_EventNewRoomTakenOver,   1200,   0, EvKind_Nothing},
  {260, GUIStr_EventCreatrAnnoyedDesc,      GUIStr_EventCreatureAnnoyed,    1200,   0, EvKind_Nothing},
  {260, GUIStr_EventNoMoreLivingSetDesc,    GUIStr_EventNoMoreLivingSpace,  1200, 500, EvKind_Nothing},
  {260, GUIStr_EventAlarmTriggeredDesc,     GUIStr_EventAlarmTriggered,      300, 200, EvKind_Nothing},
  {260, GUIStr_EventRoomUnderAttackDesc,    GUIStr_EventRoomUnderAttack,     300, 250, EvKind_Nothing},
  {260, GUIStr_EventNeedTreasrRoomDesc,     GUIStr_EventTreasureRoomNeeded,  300, 500, EvKind_Nothing}, // EvKind_NeedTreasureRoom
  {268, GUIStr_EventInformationDesc,        GUIStr_Empty,                   1200,   0, EvKind_Nothing},
  {260, GUIStr_EventRoomLostDesc,           GUIStr_EventRoomLost,           1200,   0, EvKind_Nothing},
  {260, GUIStr_EventCreaturesHungryDesc,    GUIStr_EventCreaturesHungry,     300, 500, EvKind_Nothing},
  {266, GUIStr_EventTrapCrateFoundDesc,     GUIStr_EventTrapCrateFound,      300,   0, EvKind_Nothing},
  {266, GUIStr_EventDoorCrateFoundDesc,     GUIStr_EventDoorCrateFound,      300,   0, EvKind_Nothing}, // EvKind_DoorCrateFound
  {266, GUIStr_EventDnSpecialFoundDesc,     GUIStr_EventDnSpecialFound,      300,   0, EvKind_Nothing},
  {268, GUIStr_EventInformationDesc,        GUIStr_Empty,                   1200,   0, EvKind_Nothing},
  {262, GUIStr_EventFightDesc,              GUIStr_EventFight,                -1,   0, EvKind_EnemyFight},
  {260, GUIStr_EventWorkRoomUnreachblDesc,  GUIStr_EventWorkRoomUnreachbl,  1200, 500, EvKind_Nothing}, // EvKind_WorkRoomUnreachable
  {260, GUIStr_EventStorgRoomUnreachblDesc, GUIStr_EventStorgRoomUnreachbl, 1200, 500, EvKind_Nothing}, // EvKind_StorageRoomUnreachable
  {  0, GUIStr_Empty,                       GUIStr_Empty,                     50,  10, EvKind_Nothing}, // EvKind_PrisonerStarving
  {  0, GUIStr_Empty,                       GUIStr_Empty,                   1200,  50, EvKind_Nothing}, // EvKind_TorturedHurt
  {  0, GUIStr_Empty,                       GUIStr_Empty,                   1200,  50, EvKind_Nothing}, // EvKind_EnemyDoor
};

/*
struct DoorDesc doors[] = {
  {102,  13, 102,  20,  97, 155, 0, 0, 0, 0, 200},
  {253,   0, 257,   0, 103, 118, 0, 0, 0, 0, 201},
  {399,   0, 413,   0, 114, 144, 0, 0, 0, 0, 202},
  {511,  65, 546,  85,  94, 160, 0, 0, 0, 0, 203},
  {149, 211, 153, 232,  55,  84, 0, 0, 0, 0, 204},
  {258, 176, 262, 178,  60,  84, 0, 0, 0, 0, 205},
  {364, 183, 375, 191,  70,  95, 0, 0, 0, 0, 206},
  {466, 257, 473, 261,  67,  94, 0, 0, 0, 0, 207},
  {254, 368, 260, 391, 128,  80, 0, 0, 0, 0, 208},
};
*/

const unsigned long alliance_grid[4][4] = {
  {0x00, 0x01, 0x02, 0x04,},
  {0x01, 0x00, 0x08, 0x10,},
  {0x02, 0x08, 0x00, 0x20,},
  {0x04, 0x10, 0x20, 0x00,},
};

#if (BFDEBUG_LEVEL > 0)
// Declarations for font testing screen (debug version only)
struct TbSprite *testfont[TESTFONTS_COUNT];
struct TbSprite *testfont_end[TESTFONTS_COUNT];
unsigned char * testfont_data[TESTFONTS_COUNT];
unsigned char *testfont_palette[3];
long num_chars_in_font = 128;
#endif

int status_panel_width = 140;
// struct MsgBoxInfo MsgBox;

/******************************************************************************/
short menu_is_active(short idx)
{
  return (menu_id_to_number(idx) >= 0);
}

TbBool a_menu_window_is_active(void)
{
  if (no_of_active_menus <= 0)
    return false;
  int i;
  int k;
  for (i=0; i<no_of_active_menus; i++)
  {
      k = menu_stack[i];
      if (!is_toggleable_menu(k))
        return true;
  }
  return false;
}

int frontend_font_char_width(int fnt_idx,char c)
{
    struct TbSprite *fnt;
    int i;
    fnt = frontend_font[fnt_idx];
    i = (unsigned short)c - 31;
    if (i >= 0)
        return fnt[i].SWidth;
    return 0;
}

int frontend_font_string_width(int fnt_idx, const char *str)
{
    LbTextSetFont(frontend_font[fnt_idx]);
    return LbTextStringWidth(str);
}

TbBool frontend_font_string_draw(int scr_x, int scr_y, int dst_width, int dst_height, int fnt_idx, const char *str, unsigned short fdflags)
{
    int units_per_px;
    units_per_px = dst_height * 16 / LbTextLineHeight();
    if (units_per_px < 1)
        units_per_px = 1;
    lbDisplay.DrawFlags = 0;
    LbTextSetFont(frontend_font[fnt_idx]);
    int w;
    int h;
    h = LbTextLineHeight() * units_per_px / 16;
    w = LbTextStringWidth(str) * units_per_px / 16;
    if (w > dst_width) w = dst_width;
    switch (fdflags & 0x03)
    {
    case Fnt_LeftJustify:
        LbTextSetWindow(scr_x, scr_y, w, h);
        break;
    case Fnt_RightJustify:
        LbTextSetWindow(scr_x+dst_width-w, scr_y, w, h);
        break;
    case Fnt_CenterPos:
        LbTextSetWindow(scr_x+((dst_width-w)>>1), scr_y, w, h);
        break;
    }
    return LbTextDrawResized(0, 0, units_per_px, str);
}

void get_player_gui_clicks(void)
{
  struct PlayerInfo *player;
  struct Thing *thing;
  player = get_my_player();

  if ( ((game.operation_flags & GOF_Paused) != 0) && ((game.operation_flags & GOF_WorldInfluence) == 0))
    return;

  switch (player->view_type)
  {
  case PVT_CreaturePasngr:
      if (right_button_released)
      {
        thing = thing_get(player->controlled_thing_idx);
        if (thing->class_id == TCls_Creature)
        {
          if (a_menu_window_is_active())
          {
            game.numfield_D &= ~GNFldD_CreaturePasngr;
            player->allocflags &= ~PlaF_Unknown8;
            turn_off_all_window_menus();
          } else
          {
            game.numfield_D |= GNFldD_CreaturePasngr;
            player->allocflags |= PlaF_Unknown8;
            turn_on_menu(GMnu_QUERY);
          }
        }
      }
      break;
  case PVT_CreatureContrl:
  case PVT_MapScreen:
  case PVT_MapFadeIn:
  case PVT_MapFadeOut:
      break;
  default:
      if (right_button_released)
      {
        if ((player->work_state != PSt_HoldInHand) || power_hand_is_empty(player))
        {
          if ( !turn_off_all_window_menus() )
          {
            if (player->work_state == PSt_CreatrQuery)
            {
              turn_off_query_menus();
              set_players_packet_action(player, PckA_SetPlyrState, PSt_CtrlDungeon, 0, 0, 0);
              right_button_released = 0;
            } else
            if ((player->work_state != PSt_CreatrInfo) && (player->work_state != PSt_CreatrInfoAll) && (player->work_state != PSt_CtrlDungeon))
            {
              set_players_packet_action(player, PckA_SetPlyrState, PSt_CtrlDungeon, 0, 0, 0);
              right_button_released = 0;
            }
          }
        }
      } else
      if (lbKeyOn[KC_ESCAPE])
      {
        lbKeyOn[KC_ESCAPE] = 0;
        if (a_menu_window_is_active())
        {
            turn_off_all_window_menus();
        } else
        {
            turn_on_menu(GMnu_OPTIONS);
        }
      }
      break;
  }

  if ( game_is_busy_doing_gui() )
  {
    set_players_packet_control(player, 0x4000u);
  }
}

void add_message(long plyr_idx, char *msg)
{
    struct NetMessage *nmsg;
    long i;
    long k;
    i = net_number_of_messages;
    if (i >= NET_MESSAGES_COUNT)
    {
      for (k=0; k < (NET_MESSAGES_COUNT-1); k++)
      {
        memcpy(&net_message[k], &net_message[k+1], sizeof(struct NetMessage));
      }
      i = NET_MESSAGES_COUNT-1;
    }
    nmsg = &net_message[i];
    nmsg->plyr_idx = plyr_idx;
    strncpy(nmsg->text, msg, NET_MESSAGE_LEN-1);
    nmsg->text[NET_MESSAGE_LEN-1] = '\0';
    i++;
    net_number_of_messages = i;
    if (net_message_scroll_offset+4 < i)
      net_message_scroll_offset = i-4;
}

/**
 * Checks if all the network players are using compatible version of DK.
 */
TbBool validate_versions(void)
{
    struct PlayerInfo *player;
    long i;
    long ver;
    ver = -1;
    for (i=0; i < NET_PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if ((net_screen_packet[i].field_4 & 0x01) != 0)
      {
        if (ver == -1)
          ver = player->field_4E7;
        if (player->field_4E7 != ver)
          return false;
      }
    }
    return true;
}

void versions_different_error(void)
{
    const char *plyr_nam;
    struct ScreenPacket *nspckt;
    char text[MESSAGE_TEXT_LEN];
    char *str;
    int i;

    NETMSG("Error: Players have different versions of DK");

    if (LbNetwork_Stop())
    {
      ERRORLOG("LbNetwork_Stop() failed");
    }
    lbKeyOn[KC_ESCAPE] = 0;
    lbKeyOn[KC_SPACE] = 0;
    lbKeyOn[KC_RETURN] = 0;
    text[0] = '\0';
    // Preparing message
    for (i=0; i < NET_PLAYERS_COUNT; i++)
    {
      plyr_nam = network_player_name(i);
      nspckt = &net_screen_packet[i];
      if ((nspckt->field_4 & 0x01) != 0)
      {
        str = buf_sprintf("%s(%d.%02d) ", plyr_nam, nspckt->field_6, nspckt->field_8);
        strncat(text, str, MESSAGE_TEXT_LEN-strlen(text));
        text[MESSAGE_TEXT_LEN-1] = '\0';
      }
    }
    // Waiting for users reaction
    while ( 1 )
    {
      if (lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_SPACE] || lbKeyOn[KC_RETURN])
        break;
      LbWindowsControl();
      if (LbScreenLock() == Lb_SUCCESS)
      {
        draw_text_box(text);
        LbScreenUnlock();
      }
      LbScreenSwap();
    }
    // Checking where to go back
    init_menu_state_on_net_stats_exit();
}

/**
 * Makes error box with message from GUI strings collection.
 *
 * @param msg_idx
 */
void create_error_box(TextStringId msg_idx)
{
    if (!game.packet_load_enable)
    {
        //change the length into  when gui_error_text will not be exported
        strncpy(gui_error_text, get_string(msg_idx), sizeof(gui_error_text)-1);
        turn_on_menu(GMnu_ERROR_BOX);
    }
}


void create_message_box(const char *title, const char *line1, const char *line2, const char *line3, const char* line4, const char* line5)
{
    memset(&MsgBox,0, sizeof(MsgBox));
    memcpy(&MsgBox.title, title, sizeof(MsgBox.title)-1);
    memcpy(&MsgBox.line1, line1, sizeof(MsgBox.line1)-1);
    memcpy(&MsgBox.line2, line2, sizeof(MsgBox.line2)-1);
    memcpy(&MsgBox.line3, line3, sizeof(MsgBox.line3)-1);
    memcpy(&MsgBox.line4, line4, sizeof(MsgBox.line4)-1);
    memcpy(&MsgBox.line5, line5, sizeof(MsgBox.line5)-1);
    turn_on_menu(GMnu_MSG_BOX);
}

short game_is_busy_doing_gui(void)
{
    struct PlayerInfo *player;
    player = get_my_player();
    struct DungeonAdd *dungeonadd = get_dungeonadd(player->id_number);
    if (dungeonadd->one_click_lock_cursor)
      return false;
    if (!busy_doing_gui)
      return false;
    if (battle_creature_over <= 0)
      return true;
    PowerKind pwkind;
    pwkind = 0;
    if (player->work_state < PLAYER_STATES_COUNT)
      pwkind = player_state_to_power_kind[player->work_state];
    {
        struct Thing *thing;
        thing = thing_get(battle_creature_over);
        if (can_cast_power_on_thing(player->id_number, thing, pwkind))
            return true;
    }
    return false;
}

TbBool get_button_area_input(struct GuiButton *gbtn, int modifiers)
{
    char *str;
    TbKeyCode key;
    unsigned short outchar;
    TbLocChar vischar[4];
    //return _DK_get_button_area_input(gbtn, a2);
    strcpy(vischar," ");
    str = (char *)gbtn->content;
    key = lbInkey;
    outchar = key_to_ascii(key, key_modifiers);
    vischar[0] = outchar;
    if (key == KC_RETURN)
    {
        if ((str[0] != '\0') || (modifiers == -3))
        {
            gbtn->gbactn_1 = 0;
            (gbtn->click_event)(gbtn);
            input_button = 0;
            if ((gbtn->flags & LbBtnF_Unknown02) != 0)
            {
                struct GuiMenu *gmnu;
                gmnu = get_active_menu(gbtn->gmenu_idx);
                gmnu->visual_state = 3;
                remove_from_menu_stack(gmnu->ident);
            }
        }
    } else
    if (key == KC_ESCAPE)
    { // Stop the input, revert the string to what it was before
        strncpy(str, backup_input_field, gbtn->field_2D);
        input_button = 0;
        input_field_pos = 0;
    } else
    if (key == KC_BACK)
    { // Delete the last char
        if (input_field_pos > 0) {
            input_field_pos--;
            LbLocTextStringDelete(str, input_field_pos, 1);
        }
    } else
    if (key == KC_DELETE)
    { // Delete the next char
        if (input_field_pos < LbLocTextStringLength(str)) {
            LbLocTextStringDelete(str, input_field_pos, 1);
        }
    } else
    if ((key == KC_HOME) || (key == KC_PGUP))
    { // move to first char
        input_field_pos = 0;
    } else
    if ((key == KC_END) || (key == KC_PGDOWN))
    { // move to last char
        input_field_pos = LbLocTextStringLength(str);
    } else
    if (key == KC_LEFT)
    { // move one char left
        if (input_field_pos > 0)
            input_field_pos--;
    } else
    if (key == KC_RIGHT)
    { // move one char left
        if (input_field_pos < LbLocTextStringLength(str))
            input_field_pos++;
    } else
    if (LbLocTextStringSize(str) < abs(gbtn->field_2D))
    {
        // Check if we have printable character
        if (modifiers == -1)
        {
            if (!isprint(vischar[0])) {
                clear_key_pressed(key);
                return false;
            }
        } else
        {
            if (!isalnum(vischar[0]) && (vischar[0] != ' ')) {
                clear_key_pressed(key);
                return false;
            }
        }
        if (LbLocTextStringInsert(str, vischar, input_field_pos, gbtn->field_2D) != NULL) {
            input_field_pos++;
        }
    }
    clear_key_pressed(key);
    return true;
}

void maintain_loadsave(struct GuiButton *gbtn)
{
    if ((game.system_flags & GSF_NetworkActive) == 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void maintain_zoom_to_event(struct GuiButton *gbtn)
{
    struct Dungeon *dungeon;
    struct Event *event;
    dungeon = get_players_num_dungeon(my_player_number);
    if (dungeon->visible_event_idx)
    {
      event = &(game.event[dungeon->visible_event_idx]);
      if ((event->mappos_x != 0) || (event->mappos_y != 0))
      {
        gbtn->flags |= LbBtnF_Enabled;
        return;
      }
    }
    gbtn->flags &= ~LbBtnF_Enabled;
}

void maintain_scroll_up(struct GuiButton *gbtn)
{
    struct TextScrollWindow * scrollwnd;
    //_DK_maintain_scroll_up(gbtn);
    scrollwnd = (struct TextScrollWindow *)gbtn->content;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled * (scrollwnd->start_y < 0)) & LbBtnF_Enabled;
    if (!check_current_gui_layer(GuiLayer_OneClick))
    {
        if (wheel_scrolled_up & lbKeyOn[KC_LCONTROL])
        {
            scrollwnd->action = 1;
        }
    }
}

void maintain_scroll_down(struct GuiButton *gbtn)
{
    struct TextScrollWindow * scrollwnd;
    //_DK_maintain_scroll_down(gbtn);
    scrollwnd = (struct TextScrollWindow *)gbtn->content;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Enabled
        * (scrollwnd->window_height - scrollwnd->text_height + 2 < scrollwnd->start_y)) & LbBtnF_Enabled;
    if (!check_current_gui_layer(GuiLayer_OneClick))
    {
        if (wheel_scrolled_down & lbKeyOn[KC_LCONTROL])
        {
            scrollwnd->action = 2;
        }
    }
}

void frontend_continue_game_maintain(struct GuiButton *gbtn)
{
    if (continue_game_option_available != 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontend_main_menu_load_game_maintain(struct GuiButton *gbtn)
{
    if (number_of_saved_games > 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontend_mappacks_maintain(struct GuiButton *gbtn)
{
    if (mappacks_list.items_num > 0)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void frontend_main_menu_netservice_maintain(struct GuiButton *gbtn)
{
    gbtn->flags |= LbBtnF_Enabled;
}

void frontend_main_menu_highscores_maintain(struct GuiButton *gbtn)
{
    gbtn->flags |= LbBtnF_Enabled;
}

TbBool frontend_should_all_players_quit(void)
{
    return (net_service_index_selected <= 1);
}

TbBool frontend_is_player_allied(long idx1, long idx2)
{
    if (idx1 == idx2)
      return true;
    if ((idx1 < 0) || (idx1 >= PLAYERS_COUNT))
      return false;
    if ((idx2 < 0) || (idx2 >= PLAYERS_COUNT))
      return false;
    return ((frontend_alliances & alliance_grid[idx1][idx2]) != 0);
}

void frontend_set_alliance(long idx1, long idx2)
{
    if (frontend_is_player_allied(idx1, idx2))
      frontend_alliances &= ~alliance_grid[idx1][idx2];
    else
      frontend_alliances |= alliance_grid[idx1][idx2];
}

TbResult frontend_load_data(void)
{
    char *fname;
    TbResult ret;
    long len;
    //return _DK_frontend_load_data();
    ret = Lb_SUCCESS;
    wait_for_cd_to_be_available();
    frontend_background = (unsigned char *)game.map;
#ifdef SPRITE_FORMAT_V2
    fname = prepare_file_fmtpath(FGrp_LoData,"front-%d.raw",64);
#else
    fname = prepare_file_path(FGrp_LoData,"front.raw");
#endif
    len = LbFileLoadAt(fname, frontend_background);
    if (len < 307200) {
        ret = Lb_FAIL;
    }
    if (len > sizeof(game.map)) {
        WARNLOG("Reused memory area exceeded for frontend background.");
    }
    frontend_sprite_data = (unsigned char *)poly_pool;
#ifdef SPRITE_FORMAT_V2
    fname = prepare_file_fmtpath(FGrp_LoData,"frontbit-%d.dat",64);
#else
    fname = prepare_file_path(FGrp_LoData,"frontbit.dat");
#endif
    len = LbFileLoadAt(fname, frontend_sprite_data);
    if (len < 12) {
        frontend_end_sprite_data = frontend_sprite_data;
        ret = Lb_FAIL;
    } else {
        frontend_end_sprite_data = ((unsigned char *)frontend_sprite_data + len);
    }
    frontend_sprite = (struct TbSprite *)frontend_end_sprite_data;
#ifdef SPRITE_FORMAT_V2
    fname = prepare_file_fmtpath(FGrp_LoData,"frontbit-%d.tab",64);
#else
    fname = prepare_file_path(FGrp_LoData,"frontbit.tab");
#endif
    len = LbFileLoadAt(fname, frontend_sprite);
    if (len < 12) {
        frontend_end_sprite = frontend_sprite;
        ret = Lb_FAIL;
    } else {
        frontend_end_sprite = (struct TbSprite *)((unsigned char *)frontend_sprite + len);
    }
    if (((long)frontend_end_sprite - (long)frontend_sprite_data) > sizeof(poly_pool)) {
        WARNLOG("Reused memory area exceeded for frontend sprites.");
    }
    LbSpriteSetup(frontend_sprite, frontend_end_sprite, frontend_sprite_data);
    return ret;
}

void activate_room_build_mode(RoomKind rkind, TextStringId tooltip_id)
{
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_SetPlyrState, PSt_BuildRoom, rkind, 0, 0);
    struct RoomConfigStats *roomst;
    roomst = &slab_conf.room_cfgstats[rkind];
    game.chosen_room_kind = rkind;
    game.chosen_room_spridx = roomst->bigsym_sprite_idx;
    game.chosen_room_tooltip = tooltip_id;
}

long player_state_to_packet(long work_state, PowerKind pwkind, TbBool already_in)
{
    switch (work_state)
    {
    case PSt_CallToArms:
        if (already_in)
            return PckA_PwrCTADis;
        else
            return PckA_SetPlyrState;
    case PSt_SightOfEvil:
        if (already_in)
            return PckA_PwrSOEDis;
        else
            return PckA_SetPlyrState;
    case PSt_CtrlDirect:
    case PSt_FreeCtrlDirect:
    case PSt_CreateDigger:
    case PSt_CaveIn:
    case PSt_Heal:
    case PSt_Lightning:
    case PSt_SpeedUp:
    case PSt_Armour:
    case PSt_Conceal:
    case PSt_CastDisease:
    case PSt_TurnChicken:
    case PSt_DestroyWalls:
    case PSt_TimeBomb:
        return PckA_SetPlyrState;
    case PSt_None:
        switch (pwkind)
        {
        case PwrK_OBEY:
            return PckA_UsePwrObey;
        case PwrK_HOLDAUDNC:
            return PckA_HoldAudience;
        case PwrK_ARMAGEDDON:
            return PckA_UsePwrArmageddon;
        default:
            break;
        }
        return PckA_None;
    default:
        return PckA_None;
    }
}

TbBool set_players_packet_change_spell(struct PlayerInfo *player,PowerKind pwkind)
{
    if (power_is_instinctive(game.chosen_spell_type) && (game.chosen_spell_type != 0))
        return false;
    const struct PowerConfigStats *powerst;
    powerst = get_power_model_stats(pwkind);
    TbBool already_in;
    already_in = (powerst->work_state != PSt_None) && (player->work_state == powerst->work_state);
    int pcktype;
    pcktype = player_state_to_packet(powerst->work_state, pwkind, already_in);
    if (pcktype != PckA_None)
    {
        set_players_packet_action(player, pcktype, powerst->work_state, 0, 0, 0);
        if (!already_in) {
            play_non_3d_sample(powerst->select_sample_idx);
        }
    }
    return true;
}

TbBool is_special_power(PowerKind pwkind)
{
    return ((pwkind == PwrK_HOLDAUDNC) || (pwkind == PwrK_ARMAGEDDON));
}

/**
 * Sets a new chosen special spell (Armageddon or Hold Audience).
 */
void choose_special_spell(PowerKind pwkind, TextStringId tooltip_id)
{
    struct Dungeon *dungeon;
    const struct MagicStats *pwrdynst;

    if (!is_special_power(pwkind)) {
        WARNLOG("Bad power kind");
        return;
    }

    dungeon = get_players_num_dungeon(my_player_number);
    set_chosen_power(pwkind, tooltip_id);
    pwrdynst = get_power_dynamic_stats(pwkind);

    if (dungeon->total_money_owned >= pwrdynst->cost[0]) {
        struct PowerConfigStats *powerst;
        powerst = get_power_model_stats(pwkind);
        play_non_3d_sample_no_overlap(powerst->select_sample_idx); // Play the spell speech
        switch (pwkind)
        {
        case PwrK_ARMAGEDDON:
            turn_on_menu(GMnu_ARMAGEDDON);
            break;
        case PwrK_HOLDAUDNC:
            turn_on_menu(GMnu_HOLD_AUDIENCE);
            break;
        }
    }
}

/**
 * Sets a new chosen spell.
 * Fills packet with the previous spell disable action.
 */
void choose_spell(PowerKind pwkind, TextStringId tooltip_id)
{
    struct PlayerInfo *player;

    pwkind = pwkind % POWER_TYPES_COUNT;

    if (is_special_power(pwkind)) {
        choose_special_spell(pwkind, tooltip_id);
        return;
    }

    player = get_my_player();

    // Disable previous spell
    if (!set_players_packet_change_spell(player, pwkind)) {
        WARNLOG("Inconsistency when switching spell %d to %d",
            (int)game.chosen_spell_type, (int)pwkind);
    }

    set_chosen_power(pwkind, tooltip_id);
}

void frontend_draw_scroll_tab(struct GuiButton *gbtn, long scroll_offset, long first_elem, long last_elem)
{
    struct TbSprite *spr;
    long i;
    long k;
    long n;
    int units_per_px;
    units_per_px = simple_frontend_sprite_width_units_per_px(gbtn, 78, 100);
    spr = &frontend_sprite[78];
    i = last_elem - first_elem;
    k = gbtn->height - spr->SHeight * units_per_px / 16;
    if (i <= 1)
        n = 0;
    else
        n = (scroll_offset * (k << 8) / (i - 1)) >> 8;
    LbSpriteDrawResized(gbtn->scr_pos_x, n+gbtn->scr_pos_y, units_per_px, spr);
}

long frontend_scroll_tab_to_offset(struct GuiButton *gbtn, long scr_pos, long first_elem, long last_elem)
{
    long elem_num;
    elem_num = last_elem - first_elem;
    if (elem_num < 1) {
        return 0;
    }
    long bar_pos;
    bar_pos = scr_pos - gbtn->scr_pos_y;
    if (bar_pos < 0) bar_pos = 0;
    if (bar_pos >= gbtn->height) bar_pos = gbtn->height-1;
    long scroll_offset;
    scroll_offset = bar_pos * elem_num / gbtn->height;
    return scroll_offset;
}

void gui_quit_game(struct GuiButton *gbtn)
{
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_Unknown001, 0, 0, 0, 0);
}

void draw_slider64k(long scr_x, long scr_y, int units_per_px, long width)
{
    draw_bar64k(scr_x, scr_y, units_per_px, width);
    // Inner size
    int base_x;
    int base_y;
    int base_w;
    base_w = width - 64*units_per_px/16;
    base_x = scr_x + 32*units_per_px/16;
    base_y = scr_y + 10*units_per_px/16;
    if (base_w < 72*units_per_px/16) {
        ERRORLOG("Bar is too small");
        return;
    }
    int cur_x;
    int cur_y;
    cur_x = base_x;
    cur_y = base_y;
    int end_x;
    end_x = base_x + base_w - 64*units_per_px/16;
    struct TbSprite *spr;
    spr = &button_sprite[4];
    LbSpriteDrawResized(cur_x, cur_y, units_per_px, spr);
    cur_x += spr->SWidth*units_per_px/16;
    spr = &button_sprite[5];
    while (cur_x < end_x)
    {
        LbSpriteDrawResized(cur_x, cur_y, units_per_px, spr);
        cur_x += spr->SWidth*units_per_px/16;
    }
    cur_x = end_x;
    LbSpriteDrawResized(cur_x/pixel_size, cur_y/pixel_size, units_per_px, spr);
    cur_x += spr->SWidth*units_per_px/16;
    spr = &button_sprite[6];
    LbSpriteDrawResized(cur_x/pixel_size, cur_y/pixel_size, units_per_px, spr);
}

void gui_area_slider(struct GuiButton *gbtn)
{
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        return;
    }
    int units_per_px;
    units_per_px = (gbtn->height*16 + 30/2) / 30;
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, 2, 100);
    draw_slider64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->width);
    int shift_x;
    shift_x = (gbtn->width - 64*units_per_px/16) * gbtn->slide_val >> 8;
    struct TbSprite *spr;
    if (gbtn->flags != 0) {
        spr = &button_sprite[21];
    } else {
        spr = &button_sprite[20];
    }
    LbSpriteDrawResized(gbtn->scr_pos_x + shift_x + 24*units_per_px/16, gbtn->scr_pos_y + 6*units_per_px/16, bs_units_per_px, spr);
}

#if (BFDEBUG_LEVEL > 0)
// Code for font testing screen (debug version only)
TbBool fronttestfont_draw(void)
{
  const struct TbSprite *spr;
  unsigned long i;
  unsigned long k;
  long w;
  long h;
  long x;
  long y;
  SYNCDBG(9,"Starting");
  for (y=0; y < lbDisplay.GraphicsScreenHeight; y++)
    for (x=0; x < lbDisplay.GraphicsScreenWidth; x++)
    {
        lbDisplay.WScreen[y*lbDisplay.GraphicsScreenWidth+x] = 0;
    }
  LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenHeight/pixel_size, MyScreenWidth/pixel_size);
  // Drawing
  w = 32;
  h = 48;
  for (i=31; i < num_chars_in_font+31; i++)
  {
    k = (i-31);
    SYNCDBG(9,"Drawing char %d",i);
    x = (k%32)*w + 2;
    y = (k/32)*h + 2;
    if (lbFontPtr != NULL)
      spr = LbFontCharSprite(lbFontPtr,i);
    else
      spr = NULL;
    if (spr != NULL)
    {
      LbDrawBox(x, y, spr->SWidth+2, spr->SHeight+2, 255);
      LbSpriteDraw(x+1, y+1, spr);
    }
//TODO SPRITES enhance font support
  }
  // Displaying the new frame
  return true;
}

TbBool fronttestfont_input(void)
{
  const unsigned int keys[] = {KC_Z,KC_1,KC_2,KC_3,KC_4,KC_5,KC_6,KC_7,KC_8,KC_9,KC_0};
  int i;
  for (i=0; i < sizeof(keys)/sizeof(keys[0]); i++)
  {
    if (lbKeyOn[keys[i]])
    {
      lbKeyOn[keys[i]] = 0;
      num_chars_in_font = testfont_end[i]-testfont[i];
      SYNCDBG(9,"Characters in font %d: %d",i,num_chars_in_font);
      if (i < 4)
        LbPaletteSet(frontend_palette);//testfont_palette[0]
      else
        LbPaletteSet(testfont_palette[1]);
      LbTextSetFont(testfont[i]);
      return true;
    }
  }
  return false;
}
#endif


void frontend_draw_icon(struct GuiButton *gbtn)
{
    int units_per_px;
    units_per_px = simple_frontend_sprite_width_units_per_px(gbtn, gbtn->sprite_idx, 100);
    const struct TbSprite *spr = get_frontend_sprite(gbtn->sprite_idx);
    LbSpriteDrawResized(gbtn->scr_pos_x, gbtn->scr_pos_y, units_per_px, spr);
}

void frontend_draw_slider(struct GuiButton *gbtn)
{
    //_DK_frontend_draw_slider(gbtn);
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        return;
    }
    int fs_units_per_px;
    fs_units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, 93, 100);
    int scr_x;
    int scr_y;
    scr_x = gbtn->scr_pos_x;
    scr_y = gbtn->scr_pos_y;
    struct TbSprite *spr;
    spr = &frontend_sprite[92];
    LbSpriteDrawResized(scr_x, scr_y, fs_units_per_px, spr);
    scr_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[93];
    LbSpriteDrawResized(scr_x, scr_y, fs_units_per_px, spr);
    scr_x += spr->SWidth * fs_units_per_px / 16;
    LbSpriteDrawResized(scr_x, scr_y, fs_units_per_px, spr);
    scr_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[94];
    LbSpriteDrawResized(scr_x, scr_y, fs_units_per_px, spr);
    int shift_x;
    shift_x = gbtn->slide_val * (gbtn->width - 64*fs_units_per_px/16) >> 8;
    if (gbtn->gbactn_1 != 0) {
        spr = &frontend_sprite[91];
    } else {
        spr = &frontend_sprite[78];
    }
    LbSpriteDrawResized((gbtn->scr_pos_x + shift_x + 24*fs_units_per_px/16) / pixel_size, (gbtn->scr_pos_y + 3*fs_units_per_px/16) / pixel_size, fs_units_per_px, spr);
}

void frontend_draw_small_slider(struct GuiButton *gbtn)
{
    //_DK_frontend_draw_small_slider(gbtn);
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        return;
    }
    int fs_units_per_px;
    fs_units_per_px = simple_frontend_sprite_height_units_per_px(gbtn, 93, 100);
    int scr_x;
    int scr_y;
    scr_x = gbtn->scr_pos_x;
    scr_y = gbtn->scr_pos_y;
    struct TbSprite *spr;
    spr = &frontend_sprite[92];
    LbSpriteDrawResized(scr_x, scr_y, fs_units_per_px, spr);
    scr_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[93];
    LbSpriteDrawResized(scr_x, scr_y, fs_units_per_px, spr);
    scr_x += spr->SWidth * fs_units_per_px / 16;
    spr = &frontend_sprite[94];
    LbSpriteDrawResized(scr_x, scr_y, fs_units_per_px, spr);
    int val;
    val = gbtn->slide_val * (gbtn->width - 64*fs_units_per_px/16) >> 8;
    if (gbtn->gbactn_1 != 0) {
        spr = &frontend_sprite[91];
    } else {
        spr = &frontend_sprite[78];
    }
    LbSpriteDrawResized((gbtn->scr_pos_x + val + 24*fs_units_per_px/16) / pixel_size, (gbtn->scr_pos_y + 3*fs_units_per_px/16) / pixel_size, fs_units_per_px, spr);
}

void gui_area_text(struct GuiButton *gbtn)
{
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        return;
    }
    int bs_units_per_px;
    bs_units_per_px = simple_button_sprite_height_units_per_px(gbtn, 2, 94);
    switch (gbtn->sprite_idx)
    {
    case 1:
        if ( gbtn->gbactn_1 || gbtn->gbactn_2 )
        {
            draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->width);
            draw_lit_bar64k(gbtn->scr_pos_x - 6*units_per_pixel/16, gbtn->scr_pos_y - 6*units_per_pixel/16, bs_units_per_px, gbtn->width + 6*units_per_pixel/16);
        } else
        {
            draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->width);
        }
        break;
    case 2:
        draw_bar64k(gbtn->scr_pos_x, gbtn->scr_pos_y, bs_units_per_px, gbtn->width);
        break;
    }
    if ((gbtn->tooltip_stridx != GUIStr_Empty) && (gbtn->tooltip_stridx != -GUIStr_Empty))
    {
        if (gbtn->tooltip_stridx > 0)
            snprintf(gui_textbuf,sizeof(gui_textbuf), "%s", get_string(gbtn->tooltip_stridx));
        else
            snprintf(gui_textbuf,sizeof(gui_textbuf), "%s", get_string(-gbtn->tooltip_stridx));
        draw_button_string(gbtn, (gbtn->width*32 + 16)/gbtn->height, gui_textbuf);
    } else
    if (gbtn->content != NULL)
    {
        snprintf(gui_textbuf,sizeof(gui_textbuf), "%s", (char *)gbtn->content);
        // Since this button can have various width, but its height is always 32,
        // unscaled width is deduced based on height scale
        draw_button_string(gbtn, (gbtn->width*32 + 16)/gbtn->height, gui_textbuf);
    }
}

void frontend_init_options_menu(struct GuiMenu *gmnu)
{
    //_DK_frontend_init_options_menu(gmnu);
    music_level = settings.redbook_volume;
    sound_level = settings.sound_volume;
    fe_mouse_sensitivity = settings.first_person_move_sensitivity;
}

void frontend_set_player_number(long plr_num)
{
    struct PlayerInfo *player;
    my_player_number = plr_num;
    player = get_my_player();
    player->id_number = plr_num;
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
}

const char *frontend_button_caption_text(const struct GuiButton *gbtn)
{
    unsigned long febtn_idx;
    int text_idx;
    febtn_idx = (unsigned long)gbtn->content;
    if (febtn_idx < FRONTEND_BUTTON_INFO_COUNT)
        text_idx = frontend_button_info[febtn_idx].capstr_idx;
    else
        text_idx = GUIStr_Empty;
    return get_string(text_idx);
}

int frontend_button_caption_font(const struct GuiButton *gbtn, long mouse_over_btn_idx)
{
    unsigned long febtn_idx;
    int font_idx;
    febtn_idx = (unsigned long)gbtn->content;
    if (febtn_idx < FRONTEND_BUTTON_INFO_COUNT)
        font_idx = frontend_button_info[febtn_idx].font_index;
    else
        font_idx = 3;
    if ((febtn_idx != 0) && (mouse_over_btn_idx == febtn_idx))
        font_idx = 2;
    return font_idx;
}

void frontend_draw_text(struct GuiButton *gbtn)
{
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    int font_idx;
    if ((gbtn->flags & LbBtnF_Enabled) == 0)
        font_idx = 3;
    else
        font_idx = frontend_button_caption_font(gbtn, frontend_mouse_over_button);
    LbTextSetFont(frontend_font[font_idx]);
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    LbTextDrawResized(0, 0, tx_units_per_px, frontend_button_caption_text(gbtn));
}

void frontend_change_state(struct GuiButton *gbtn)
{
    frontend_set_state(gbtn->btype_value & LbBFeF_IntValueMask);
}

void frontend_draw_enter_text(struct GuiButton *gbtn)
{
    //_DK_frontend_draw_enter_text(gbtn); return;
    int font_idx;
    font_idx = 1;
    if (gbtn == input_button) {
        font_idx = 2;
    } else
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        font_idx = 3;
    } else
    if ((gbtn->content != NULL) && ((gbtn->btype_value & LbBFeF_IntValueMask) == frontend_mouse_over_button)) {
        font_idx = 2;
    }
    char *srctext;
    srctext = (char *)gbtn->content;
    while (LbTextStringWidth(srctext) > 240)
        srctext[strlen(srctext)-2] = 0;
    char text[2048];
    // Prepare text buffer
    TbBool print_with_cursor = 0;
    if (gbtn == input_button)
    {
        if ((LbTimerClock() / 200 & 1) != 0)
            print_with_cursor = 1;
    }
    snprintf(text, sizeof(text), "%s%s", srctext, print_with_cursor?"_":"");
    LbTextSetFont(frontend_font[font_idx]);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, (240 + LbTextCharWidth('_')) * tx_units_per_px / 16, gbtn->height);
    LbTextDrawResized(0, 0, tx_units_per_px, text);
}

void frontend_draw_small_menu_button(struct GuiButton *gbtn)
{
    //_DK_frontend_draw_small_menu_button(gbtn);
    const char *text;
    text = frontend_button_caption_text(gbtn);
    frontend_draw_button(gbtn, 0, text, Lb_TEXT_HALIGN_CENTER);
}

void frontend_toggle_computer_players(struct GuiButton *gbtn)
{
    //_DK_frontend_toggle_computer_players(gbtn);
    struct ScreenPacket *nspck;
    nspck = &net_screen_packet[my_player_number];
    if ((nspck->field_4 & 0xF8) == 0)
    {
        nspck->field_4 = (nspck->field_4 & 0x07) | 0x38;
        nspck->param1 = (fe_computer_players == 0);
    }
}

void frontend_draw_computer_players(struct GuiButton *gbtn)
{
    //_DK_frontend_draw_computer_players(gbtn);
    int font_idx;
    font_idx = frontend_button_caption_font(gbtn,frontend_mouse_over_button);
    LbTextSetFont(frontend_font[font_idx]);
    const char *text;
    if (fe_computer_players) {
        text = get_string(GUIStr_On);
    } else {
        text = get_string(GUIStr_Off);
    }
    int tx_units_per_px;
    tx_units_per_px = gbtn->height * 16 / LbTextLineHeight();
    int ln_height;
    ln_height = LbTextLineHeight() * tx_units_per_px / 16;
    LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, ln_height);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    LbTextDrawResized(0, 0, tx_units_per_px, frontend_button_caption_text(gbtn));
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_RIGHT;
    LbTextDrawResized(0, 0, tx_units_per_px, text);
    lbDisplay.DrawFlags = 0;
}

void set_packet_start(struct GuiButton *gbtn)
{
    //_DK_set_packet_start(gbtn);
    struct ScreenPacket *nspck;
    nspck = &net_screen_packet[my_player_number];
    if ((nspck->field_4 & 0xF8) == 0)
        nspck->field_4 = (nspck->field_4 & 7) | 0x18;
}

void draw_scrolling_button_string(struct GuiButton *gbtn, const char *text)
{
  struct TextScrollWindow *scrollwnd;
  unsigned short flg_mem;
  long text_height;
  long area_height;
  flg_mem = lbDisplay.DrawFlags;
  lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
  lbDisplay.DrawFlags |= Lb_TEXT_HALIGN_CENTER;
  LbTextSetWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
  scrollwnd = (struct TextScrollWindow *)gbtn->content;
  if (scrollwnd == NULL)
  {
      ERRORLOG("Cannot have a TEXT_SCROLLING box type without a pointer to a TextScrollWindow");
      LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenHeight/pixel_size, MyScreenWidth/pixel_size);
      return;
  }
  area_height = gbtn->height;
  scrollwnd->window_height = area_height;
  text_height = scrollwnd->text_height;
  int tx_units_per_px;
  if (dbc_language > 0)
  {
      tx_units_per_px = scale_value_by_horizontal_resolution((MyScreenWidth >= 640) ? 16 : 32);
  }
  else
  {
      tx_units_per_px = scale_ui_value((MyScreenWidth >= 640) ? 16 : 32);
  }
  if (text_height == 0)
  {
      text_height = text_string_height(tx_units_per_px, text);
      SYNCDBG(18,"Computed message height %ld for \"%s\"",text_height,text);
      scrollwnd->text_height = text_height;
  }
  SYNCDBG(18,"Message h=%ld Area h=%d",text_height,area_height);
  // If the text is smaller that the area we have for it - just place it at center
  if (text_height <= area_height)
  {
    scrollwnd->start_y = (area_height - text_height) / 2;
  } else
  // Otherwise - we must take scrollbars into account
  {
    // Maintain scrolling actions
    switch ( scrollwnd->action )
    {
    case 1:
      scrollwnd->start_y += 12*units_per_pixel/16;
      break;
    case 2:
      scrollwnd->start_y -= 12*units_per_pixel/16;
      break;
    case 3:
      scrollwnd->start_y += area_height;
      break;
    case 4:
    case 5:
      scrollwnd->start_y -= area_height;
      break;
    }
    if (scrollwnd->action == 5)
    {
      if (scrollwnd->start_y < -text_height)
      {
        scrollwnd->start_y = 0;
      }
    } else
    if (scrollwnd->action != 0)
    {
      if (scrollwnd->start_y < gbtn->height-text_height)
      {
        scrollwnd->start_y = gbtn->height-text_height;
      } else
      if (scrollwnd->start_y > 0)
      {
        scrollwnd->start_y = 0;
      }
    }
    scrollwnd->action = 0;
  }
  // Finally, draw the text
  LbTextDrawResized(0, scrollwnd->start_y, tx_units_per_px, text);
  // And restore default drawing options
  LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenHeight/pixel_size, MyScreenWidth/pixel_size);
  lbDisplay.DrawFlags = flg_mem;
}

void gui_area_scroll_window(struct GuiButton *gbtn)
{
    struct TextScrollWindow *scrollwnd;
    char *text;
    //_DK_gui_area_scroll_window(gbtn); return;
    if ((gbtn->flags & LbBtnF_Enabled) == 0) {
        return;
    }
    scrollwnd = (struct TextScrollWindow *)gbtn->content;
    if (scrollwnd == NULL) {
        ERRORLOG("Button doesn't point to a TextScrollWindow data item");
        return;
    }
    text = buf_sprintf("%s", scrollwnd->text);
    draw_scrolling_button_string(gbtn, text);
}

void gui_go_to_event(struct GuiButton *gbtn)
{
    //_DK_gui_go_to_event(gbtn);
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    player = get_my_player();
    dungeon = get_players_dungeon(player);
    if (dungeon->visible_event_idx) {
        set_players_packet_action(player, PckA_Unknown083, dungeon->visible_event_idx, 0, 0, 0);
    }
}

void gui_close_objective(struct GuiButton *gbtn)
{
    //_DK_gui_close_objective(gbtn); return;
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_EventBoxClose, 0, 0, 0, 0);
    // The final effect of this packet should be 3 menus disabled
    /*turn_off_menu(GMnu_TEXT_INFO);
    turn_off_menu(GMnu_BATTLE);
    turn_off_menu(GMnu_DUNGEON_SPECIAL);*/
}

void gui_scroll_text_up(struct GuiButton *gbtn)
{
    //_DK_gui_scroll_text_up(gbtn);
    struct TextScrollWindow *scroll_window;
    scroll_window = (struct TextScrollWindow *)gbtn->content;
    scroll_window->action = 1;
}

void gui_scroll_text_down(struct GuiButton *gbtn)
{
    //_DK_gui_scroll_text_down(gbtn);
    struct TextScrollWindow *scroll_window;
    scroll_window = (struct TextScrollWindow *)gbtn->content;
    scroll_window->action = 2;
}

/**
 * Changes state based on a parameter inside GuiButton.
 * But first, loads the default campaign if no campaign is loaded yet.
 */
void frontend_ldcampaign_change_state(struct GuiButton *gbtn)
{
  if (!is_campaign_loaded())
  {
    if (!change_campaign(""))
      return;
  }
  frontend_change_state(gbtn);
}

/**
 * Changes state based on a parameter inside GuiButton.
 * But first, loads the default campaign if no campaign is loaded,
 * or the loaded one has no MP maps.
 */
void frontend_netservice_change_state(struct GuiButton *gbtn)
{
    TbBool set_cmpg;
    set_cmpg = false;
    if (!is_campaign_loaded())
    {
        set_cmpg = true;
    } else
    if (campaign.multi_levels_count < 1)
    {
        set_cmpg = true;
    }
    if (set_cmpg)
    {
        if (!change_campaign(""))
          return;
    }
    frontend_change_state(gbtn);
}

TbBool frontend_start_new_campaign(const char *cmpgn_fname)
{
    struct PlayerInfo *player;
    int i;
    SYNCDBG(7,"Starting");
    memset(&intralvl, 0, sizeof(struct IntralevelData));
    if (!change_campaign(cmpgn_fname))
        return false;
    set_continue_level_number(first_singleplayer_level());
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        player->flgfield_6 &= ~PlaF6_PlyrHasQuit;
    }
    player = get_my_player();
    clear_transfered_creature();
    calculate_moon_phase(false,false);
    hide_all_bonus_levels(player);
    update_extra_levels_visibility();
    return true;
}

void frontend_start_new_game(struct GuiButton *gbtn)
{
    const char *cmpgn_fname;
    SYNCDBG(6,"Clicked");
    // Check if we can just start the game without campaign selection screen
    if (campaigns_list.items_num < 1)
      cmpgn_fname = lbEmptyString;
    else
    if (campaigns_list.items_num == 1)
      cmpgn_fname = campaigns_list.items[0].fname;
    else
      cmpgn_fname = NULL;
    if (cmpgn_fname != NULL)
    { // If there's only one campaign, then start it
      if (!frontend_start_new_campaign(cmpgn_fname))
      {
        ERRORLOG("Unable to start new campaign");
        return;
      }
      frontend_set_state(FeSt_CAMPAIGN_INTRO);
    } else
    { // If there's more campaigns, go to selection screen
      frontend_set_state(FeSt_CAMPAIGN_SELECT);
    }
}

void frontend_load_mappacks(struct GuiButton *gbtn)
{
    const char *cmpgn_fname;
    SYNCDBG(6,"Clicked");
    // Check if we can show some levels without showing the map pack selection screen
    if (mappacks_list.items_num < 1)
      cmpgn_fname = lbEmptyString;
    else
    if (mappacks_list.items_num == 1)
      cmpgn_fname = mappacks_list.items[0].fname;
    else
      cmpgn_fname = NULL;
    if (cmpgn_fname != NULL)
    { // If there's only one map pack, then just show the levels
      if (!change_campaign(cmpgn_fname))
      {
        ERRORLOG("Unable to load map pack list");
        return;
      }
      frontend_set_state(FeSt_LEVEL_SELECT);
    } else
    { // If there's more map packs, go to selection screen
      frontend_set_state(FeSt_MAPPACK_SELECT);
    }
}

/**
 * Writes the continue game file.
 * If allow_lvnum_grow is true and my_player has won the singleplayer level,
 * then next level is written into continue file. This should be the case
 * if complete_level() wasn't called yet.
 */
short frontend_save_continue_game(short allow_lvnum_grow)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    unsigned short victory_state;
    short flg_mem;
    LevelNumber lvnum;
    lvnum = get_loaded_level_number();
    SYNCDBG(6,"Starting");
    player = get_my_player();
    dungeon = get_players_dungeon(player);
    // Do not allow current level to grow if it wasn't just beaten
    if (get_loaded_level_number() != get_continue_level_number()) {
        allow_lvnum_grow = false;
    }
    // Save some of the data from clearing
    victory_state = player->victory_state;
    memcpy(scratch, &dungeon->lvstats, sizeof(struct LevelStats));
    flg_mem = ((player->additional_flags & PlaAF_UnlockedLordTorture) != 0);
    // clear all data
    clear_game_for_save();
    // Restore saved data
    player->victory_state = victory_state;
    memcpy(&dungeon->lvstats, scratch, sizeof(struct LevelStats));
    set_flag_byte(&player->additional_flags,PlaAF_UnlockedLordTorture,flg_mem);
    // Only save continue if level was won, not a free play level, not a multiplayer level and not in packet mode
    if (((game.system_flags & GSF_NetworkActive) != 0)
     || ((game.operation_flags & GOF_SingleLevel) != 0)
     || (game.packet_load_enable)
     || (is_freeplay_level(lvnum))
     || (is_multiplayer_level(lvnum)))
        return false;
    // Select the continue level (move the campaign forward)
    if ((allow_lvnum_grow) && (player->victory_state == VicS_WonLevel)) {
        // If level number growth makes sense, do it
        SYNCDBG(7,"Progressing the campaign");
        lvnum = move_campaign_to_next_level();
    } else {
        SYNCDBG(7,"No change in campaign position, victory state %d",(int)player->victory_state);
        lvnum = get_continue_level_number();
    }
    return save_continue_game(lvnum);
}

void frontend_load_continue_game(struct GuiButton *gbtn)
{
  if (!load_continue_game())
  {
    continue_game_option_available = 0;
    return;
  }
  frontend_set_state(FeSt_LAND_VIEW);
}

void frontend_load_game_maintain(struct GuiButton *gbtn)
{
    long game_index=load_game_scroll_offset+(long)(gbtn->content)-45;
    if (game_index < number_of_saved_games)
        gbtn->flags |= LbBtnF_Enabled;
    else
        gbtn->flags &= ~LbBtnF_Enabled;
}

void do_button_click_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
    SYNCDBG(9,"Starting for button type %d",(int)gbtn->gbtype);
    //_DK_do_button_click_actions(gbtn, s, callback);
    if (gbtn->gbtype == LbBtnT_RadioBtn)
    {
        //TODO: pointers comparison should be avoided
        if (s == &gbtn->gbactn_2)
            return;
    }
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        switch (gbtn->gbtype)
        {
        case LbBtnT_NormalBtn:
        case LbBtnT_ToggleBtn:
        case LbBtnT_EditBox:
        case LbBtnT_Unknown6:
            *s = 1;
            break;
        case LbBtnT_RadioBtn:
            if ((gbtn->content != NULL) && (!*s))
            {
                unsigned char *rbstate;
                rbstate = (unsigned char *)gbtn->content;
                do_sound_button_click(gbtn);
                struct GuiMenu *amnu;
                amnu = get_active_menu(gbtn->gmenu_idx);
                clear_radio_buttons(amnu);
                *rbstate = 1;
                *s = 1;
                update_radio_button_data(amnu);
            }
            if (callback != NULL) {
                callback(gbtn);
            }
            break;
        }
    }
}

void do_button_press_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
    SYNCDBG(9,"Starting for button type %d",(int)gbtn->gbtype);
    if (gbtn->gbtype == LbBtnT_RadioBtn)
    {
        //TODO: pointers comparison should be avoided
        if (s == &gbtn->gbactn_2)
            return;
    }
    if ((gbtn->flags & LbBtnF_Enabled) != 0)
    {
        switch (gbtn->gbtype)
        {
        case LbBtnT_HoldableBtn:
            if ((*s > 5) && (callback != NULL)) {
                callback(gbtn);
            } else {
                (*s)++;
            }
            break;
        case LbBtnT_Unknown6:
            if (callback != NULL) {
                callback(gbtn);
            }
            break;
        case LbBtnT_NormalBtn:
        case LbBtnT_ToggleBtn:
        case LbBtnT_EditBox:
        case LbBtnT_RadioBtn:
            break;
        }
    }
}

void do_button_release_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
  SYNCDBG(17,"Starting");
  int i;
  struct GuiMenu *gmnu;
  switch ( gbtn->gbtype )
  {
  case LbBtnT_NormalBtn:
  case LbBtnT_HoldableBtn:
      if ((*s != 0) && (callback != NULL))
      {
          do_sound_button_click(gbtn);
          callback(gbtn);
      }
      *s = 0;
      break;
  case LbBtnT_ToggleBtn:
      i = *(unsigned char *)gbtn->content;
      i++;
      if (i > gbtn->field_2D)
          i = 0;
      *(unsigned char *)gbtn->content = i;
      if ((*s != 0) && (callback != NULL))
      {
          do_sound_button_click(gbtn);
          callback(gbtn);
      }
      *s = 0;
      break;
  case LbBtnT_RadioBtn:
      //TODO: pointers comparison should be avoided
      if (s == &gbtn->gbactn_2)
        return;
      break;
  case LbBtnT_EditBox:
      input_button = gbtn;
      setup_input_field(input_button, get_string(GUIStr_MnuUnused));
      break;
  default:
      break;
  }

  if (s == &gbtn->gbactn_1)
  {
    gmnu = get_active_menu(gbtn->gmenu_idx);
    if (gbtn->parent_menu != NULL)
      create_menu(gbtn->parent_menu);
    if ((gbtn->flags & LbBtnF_Unknown02) && (gbtn->gbtype != LbBtnT_EditBox))
    {
      if (callback == NULL)
        do_sound_menu_click();
      gmnu->visual_state = 3;
    }
  }
  SYNCDBG(17,"Finished");
}

/**
 * Returns if the menu is toggleable. If it's not, then it is always
 * visible until it's deleted.
 */
short is_toggleable_menu(short mnu_idx)
{
  switch (mnu_idx)
  {
  case GMnu_MAIN:
  case GMnu_ROOM:
  case GMnu_SPELL:
  case GMnu_TRAP:
  case GMnu_CREATURE:
  case GMnu_EVENT:
  case GMnu_QUERY:
      return true;
  case GMnu_TEXT_INFO:
  case GMnu_DUNGEON_SPECIAL:
  case GMnu_CREATURE_QUERY1:
  case GMnu_CREATURE_QUERY2:
  case GMnu_CREATURE_QUERY3:
  case GMnu_CREATURE_QUERY4:
  case GMnu_BATTLE:
  case GMnu_SPELL_LOST:
      return true;
  case GMnu_OPTIONS:
  case GMnu_INSTANCE:
  case GMnu_QUIT:
  case GMnu_LOAD:
  case GMnu_SAVE:
  case GMnu_VIDEO:
  case GMnu_SOUND:
  case GMnu_ERROR_BOX:
  case GMnu_HOLD_AUDIENCE:
  case GMnu_FEMAIN:
  case GMnu_FELOAD:
  case GMnu_FENET_SERVICE:
  case GMnu_FENET_SESSION:
  case GMnu_FENET_START:
  case GMnu_FENET_MODEM:
  case GMnu_FENET_SERIAL:
  case GMnu_FESTATISTICS:
  case GMnu_FEHIGH_SCORE_TABLE:
  case GMnu_RESURRECT_CREATURE:
  case GMnu_TRANSFER_CREATURE:
  case GMnu_ARMAGEDDON:
  case GMnu_FEDEFINE_KEYS:
  case GMnu_AUTOPILOT:
  case GMnu_FEOPTION:
  case GMnu_FELEVEL_SELECT:
  case GMnu_MAPPACK_SELECT:
  case GMnu_FECAMPAIGN_SELECT:
  case GMnu_FEERROR_BOX:
      return false;
  default:
      return true;
  }
}

int create_button(struct GuiMenu *gmnu, struct GuiButtonInit *gbinit, int units_per_px)
{
    struct GuiButton *gbtn;
    int gidx;
    long i;
    //gidx = _DK_create_button(gmnu, gbinit);
    gidx = guibutton_get_unused_slot();
    if (gidx == -1) {
        // No free buttons
        return -1;
    }
    gbtn = &active_buttons[gidx];
    gbtn->flags |= LbBtnF_Active;
    struct GuiMenu *gmnuinit;
    gmnuinit = gmnu->menu_init;
    gbtn->gmenu_idx = gmnu->number;
    gbtn->gbtype = gbinit->gbtype;
    gbtn->id_num = gbinit->id_num;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Unknown02 * (gbinit->gbifield_5 & 0xff)) & LbBtnF_Unknown02;
    gbtn->click_event = gbinit->click_event;
    gbtn->rclick_event = gbinit->rclick_event;
    gbtn->ptover_event = gbinit->ptover_event;
    gbtn->btype_value = gbinit->btype_value;
    gbtn->width = (gbinit->width * units_per_px + 8) / 16;
    gbtn->height = (gbinit->height * units_per_px + 8) / 16;
    gbtn->draw_call = gbinit->draw_call;
    gbtn->sprite_idx = gbinit->sprite_idx;
    gbtn->tooltip_stridx = gbinit->tooltip_stridx;
    gbtn->parent_menu = gbinit->parent_menu;
    gbtn->content = (unsigned long *)gbinit->content.lptr;
    gbtn->field_2D = gbinit->gbifield_31;
    gbtn->maintain_call = gbinit->maintain_call;
    gbtn->flags |= LbBtnF_Enabled;
    gbtn->flags &= ~LbBtnF_Unknown10;
    gbtn->gbactn_1 = 0;
    gbtn->flags |= LbBtnF_Visible;
    gbtn->flags ^= (gbtn->flags ^ LbBtnF_Unknown20 * (gbinit->gbifield_5 >> 8)) & LbBtnF_Unknown20;
    if ((gbinit->scr_pos_x == 999) || (gbinit->pos_x == 999))
    {
        i = gmnu->pos_x + ((gmnuinit->width >> 1) - (gbinit->width >> 1)) * units_per_px / 16;
        gbtn->scr_pos_x = i;
        gbtn->pos_x = i;
    } else
    {
        gbtn->pos_x = gmnu->pos_x + (gbinit->pos_x * units_per_px + 8) / 16;
        gbtn->scr_pos_x = gmnu->pos_x + (gbinit->scr_pos_x * units_per_px + 8) / 16;
    }
    if ((gbinit->scr_pos_y == 999) || (gbinit->pos_y == 999))
    {
        i = gmnu->pos_y + (((gmnuinit->height >> 1) - (gbinit->height >> 1)) * units_per_px + 8) / 16;
        gbtn->scr_pos_y = i;
        gbtn->pos_y = i;
    } else
    {
        gbtn->pos_y = (gbinit->pos_y * units_per_px + 8) / 16 + gmnu->pos_y;
        gbtn->scr_pos_y = gmnu->pos_y + (gbinit->scr_pos_y * units_per_px + 8) / 16;
    }
    if (gbtn->gbtype == LbBtnT_RadioBtn)
    {
        struct TextScrollWindow *scrollwnd;
        scrollwnd = (struct TextScrollWindow *)gbtn->content;
        if ((scrollwnd != NULL) && (scrollwnd->text[0] == 1))
        {
            gbtn->gbactn_1 = 1;
            gbtn->gbactn_2 = 0;
        } else
        {
            gbtn->gbactn_1 = 0;
            gbtn->gbactn_2 = 0;
        }
    } else
    {
        gbtn->gbactn_1 = 0;
        gbtn->gbactn_2 = 0;
    }
    SYNCDBG(11,"Created button %d at (%d,%d) size (%d,%d)",gidx,
        gbtn->pos_x,gbtn->pos_y,gbtn->width,gbtn->height);
    return gidx;

}

long compute_menu_position_x(long desired_pos,int menu_width, int units_per_px)
{
  struct PlayerInfo *player;
  player = get_my_player();
  long scaled_width;
  scaled_width = (menu_width * units_per_px + 8) / 16;
  long pos;
  switch (desired_pos)
  {
  case POS_MOUSMID: // Place menu centered over mouse
      pos = GetMouseX() - (scaled_width >> 1);
      break;
  case POS_GAMECTR: // Player-based positioning
      pos = (player->engine_window_x) + (player->engine_window_width >> 1) - (scaled_width >> 1);
      break;
  case POS_MOUSPRV: // Place menu centered over previous mouse position
      pos = old_menu_mouse_x - (scaled_width >> 1);
      break;
  case POS_SCRCTR:
      pos = (MyScreenWidth >> 1) - (scaled_width >> 1);
      break;
  case POS_SCRBTM:
      pos = MyScreenWidth - scaled_width;
      break;
  default: // Desired position have direct coordinates
      pos = ((desired_pos*(long)units_per_pixel)>>4)*((long)pixel_size);
      if (pos+scaled_width > lbDisplay.PhysicalScreenWidth*((long)pixel_size))
        pos = lbDisplay.PhysicalScreenWidth*((long)pixel_size)-scaled_width;
/* Helps not to touch left panel - disabling, as needs additional conditions
      if (pos < status_panel_width)
        pos = status_panel_width;
*/
      break;
  }
  // Clipping position X
  if (desired_pos == POS_GAMECTR)
  {
    if (pos+scaled_width > MyScreenWidth)
      pos = MyScreenWidth-scaled_width;
    if (pos < player->engine_window_x)
      pos = player->engine_window_x;
  } else
  {
    if (pos+scaled_width > MyScreenWidth)
      pos = MyScreenWidth-scaled_width;
    if (pos < 0)
      pos = 0;
  }
  return pos;
}

long compute_menu_position_y(long desired_pos,int menu_height, int units_per_px)
{
    struct PlayerInfo *player;
    player = get_my_player();
    long scaled_height;
    scaled_height = (menu_height * units_per_px + 8) / 16;
    long pos;
    switch (desired_pos)
    {
    case POS_MOUSMID: // Place menu centered over mouse
        pos = GetMouseY() - (scaled_height >> 1);
        break;
    case POS_GAMECTR: // Player-based positioning
        pos = (player->engine_window_height >> 1) - ((scaled_height+20*units_per_px/16) >> 1);
        break;
    case POS_MOUSPRV: // Place menu centered over previous mouse position
        pos = old_menu_mouse_y - (scaled_height >> 1);
        break;
    case POS_SCRCTR:
        pos = (MyScreenHeight >> 1) - (scaled_height >> 1);
        break;
    case POS_SCRBTM:
        pos = MyScreenHeight - scaled_height;
        break;
    default: // Desired position have direct coordinates
        pos = ((desired_pos*((long)units_per_pixel))>>4)*((long)pixel_size);
        if (pos+scaled_height > lbDisplay.PhysicalScreenHeight*((long)pixel_size))
          pos = lbDisplay.PhysicalScreenHeight*((long)pixel_size)-scaled_height;
        break;
    }
    // Clipping position Y
    if (pos+scaled_height > MyScreenHeight)
      pos = MyScreenHeight-scaled_height;
    if (pos < 0)
      pos = 0;
    return pos;
}

MenuNumber create_menu(struct GuiMenu *gmnu)
{
    MenuNumber mnu_num;
    struct GuiMenu *amnu;
    Gf_Mnu_Callback callback;
    struct GuiButtonInit *btninit;
    int i;
    SYNCDBG(18,"Starting menu ID %d",gmnu->ident);
    mnu_num = menu_id_to_number(gmnu->ident);
    if (mnu_num >= 0)
    {
        amnu = get_active_menu(mnu_num);
        amnu->visual_state = 1;
        amnu->fade_time = gmnu->fade_time;
        amnu->is_turned_on = ((game.operation_flags & GOF_ShowGui) != 0) || (!is_toggleable_menu(gmnu->ident));
        SYNCDBG(18,"Menu number %d already active",(int)mnu_num);
        return mnu_num;
    }
    add_to_menu_stack(gmnu->ident);
    mnu_num = first_available_menu();
    if (mnu_num == -1)
    {
        ERRORLOG("Too many menus open");
        return -1;
    }
    SYNCDBG(18,"Menu number %d added to stack",(int)mnu_num);
    amnu = get_active_menu(mnu_num);
    amnu->visual_state = 1;
    amnu->number = mnu_num;
    amnu->menu_init = gmnu;
    amnu->ident = gmnu->ident;
    if (amnu->ident == GMnu_MAIN)
    {
        old_menu_mouse_x = GetMouseX();
        old_menu_mouse_y = GetMouseY();
    }
    // Make scale factor
    int units_per_px;
    units_per_px = min((int)units_per_pixel,units_per_pixel_min*16/10);
    // Decrease scale factor if for some reason resulting size would exceed screen (wierd aspec ratio support)
    if (gmnu->width * units_per_px > LbScreenWidth() * 16)
        units_per_px = LbScreenWidth() * 16 / gmnu->width;
    if (gmnu->height * units_per_px > LbScreenHeight() * 16)
        units_per_px = LbScreenHeight() * 16 / gmnu->height;
    // Setting position X
    amnu->pos_x = compute_menu_position_x(gmnu->pos_x,gmnu->width,units_per_px);
    // Setting position Y
    amnu->pos_y = compute_menu_position_y(gmnu->pos_y,gmnu->height,units_per_px);

    amnu->fade_time = gmnu->fade_time;
    if (amnu->fade_time < 1) {
        ERRORLOG("Fade time %d is less than 1.",(int)amnu->fade_time);
    }
    amnu->buttons = gmnu->buttons;
    amnu->width = (gmnu->width * units_per_px + 8) / 16;
    amnu->height = (gmnu->height * units_per_px + 8) / 16;
    amnu->draw_cb = gmnu->draw_cb;
    amnu->create_cb = gmnu->create_cb;
    amnu->is_monopoly_menu = gmnu->is_monopoly_menu;
    amnu->field_1F = gmnu->field_1F;
    amnu->is_turned_on = ((game.operation_flags & GOF_ShowGui) != 0) || (!is_toggleable_menu(gmnu->ident));
    callback = amnu->create_cb;
    if (callback != NULL)
        callback(amnu);
    btninit = gmnu->buttons;
    for (i=0; btninit[i].gbtype != -1; i++)
    {
        if (create_button(amnu, &btninit[i], units_per_px) == -1)
        {
          ERRORLOG("Cannot Allocate button");
          return -1;
        }
    }
    update_radio_button_data(amnu);
    init_slider_bars(amnu);
    init_menu_buttons(amnu);
    SYNCMSG("Created menu ID %d at slot %d, pos (%d,%d) size (%d,%d)",(int)gmnu->ident,
        (int)mnu_num,(int)amnu->pos_x,(int)amnu->pos_y,(int)amnu->width,(int)amnu->height);
    return mnu_num;
}

unsigned long toggle_status_menu(short visible)
{
  static unsigned char room_on = 0;
  static unsigned char spell_on = 0;
  static unsigned char spell_lost_on = 0;
  static unsigned char trap_on = 0;
  static unsigned char creat_on = 0;
  static unsigned char event_on = 0;
  static unsigned char query_on = 0;
  static unsigned char creature_query1_on = 0;
  static unsigned char creature_query2_on = 0;
  static unsigned char creature_query3_on = 0;
  static unsigned char creature_query4_on = 0;
  static unsigned char objective_on = 0;
  static unsigned char battle_on = 0;
  static unsigned char special_on = 0;

  long k;
  unsigned long i;
  k = menu_id_to_number(GMnu_MAIN);
  if (k < 0) return 0;
  // Update pannel width
  status_panel_width = get_active_menu(k)->width;
  i = get_active_menu(k)->is_turned_on;
  if (visible != i)
  {
    if ( visible )
    {
      set_menu_visible_on(GMnu_MAIN);
      if ( room_on )
        set_menu_visible_on(GMnu_ROOM);
      if ( spell_on )
        set_menu_visible_on(GMnu_SPELL);
      if ( spell_lost_on )
        set_menu_visible_on(GMnu_SPELL_LOST);
      if ( trap_on )
        set_menu_visible_on(GMnu_TRAP);
      if ( event_on )
        set_menu_visible_on(GMnu_EVENT);
      if ( query_on )
        set_menu_visible_on(GMnu_QUERY);
      if ( creat_on )
        set_menu_visible_on(GMnu_CREATURE);
      if ( creature_query1_on )
        set_menu_visible_on(GMnu_CREATURE_QUERY1);
      if ( creature_query2_on )
        set_menu_visible_on(GMnu_CREATURE_QUERY2);
      if ( creature_query3_on )
        set_menu_visible_on(GMnu_CREATURE_QUERY3);
      if ( creature_query4_on )
        set_menu_visible_on(GMnu_CREATURE_QUERY4);
      if ( battle_on )
        set_menu_visible_on(GMnu_BATTLE);
      if ( objective_on )
        set_menu_visible_on(GMnu_TEXT_INFO);
      if ( special_on )
        set_menu_visible_on(GMnu_DUNGEON_SPECIAL);
    } else
    {
      set_menu_visible_off(GMnu_MAIN);

      k = menu_id_to_number(GMnu_ROOM);
      if (k >= 0)
        room_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_ROOM);

      k = menu_id_to_number(GMnu_SPELL);
      if (k >= 0)
        spell_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_SPELL);

      k = menu_id_to_number(GMnu_SPELL_LOST);
      if (k >= 0)
        spell_lost_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_SPELL_LOST);

      k = menu_id_to_number(GMnu_TRAP);
      if (k >= 0)
      trap_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_TRAP);

      k = menu_id_to_number(GMnu_CREATURE);
      if (k >= 0)
        creat_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_CREATURE);

      k = menu_id_to_number(GMnu_EVENT);
      if (k >= 0)
        event_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_EVENT);

      k = menu_id_to_number(GMnu_QUERY);
      if (k >= 0)
        query_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_QUERY);

      k = menu_id_to_number(GMnu_CREATURE_QUERY1);
      if (k >= 0)
        creature_query1_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_CREATURE_QUERY1);

      k = menu_id_to_number(GMnu_CREATURE_QUERY2);
      if (k >= 0)
        creature_query2_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_CREATURE_QUERY2);

      k = menu_id_to_number(GMnu_CREATURE_QUERY3);
      if (k >= 0)
        creature_query3_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_CREATURE_QUERY3);

      k = menu_id_to_number(GMnu_CREATURE_QUERY4);
      if (k >= 0)
        creature_query4_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_CREATURE_QUERY4);

      k = menu_id_to_number(GMnu_TEXT_INFO);
      if (k >= 0)
        objective_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_TEXT_INFO);

      k = menu_id_to_number(GMnu_BATTLE);
      if (k >= 0)
        battle_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_BATTLE);

      k = menu_id_to_number(GMnu_DUNGEON_SPECIAL);
      if (k >= 0)
        special_on = get_active_menu(k)->is_turned_on;
      set_menu_visible_off(GMnu_DUNGEON_SPECIAL);
    }
  }
  return i;
}

TbBool toggle_first_person_menu(TbBool visible)
{
  static unsigned char creature_query_on = 0;
  if (visible)
  {
    if (creature_query_on == 1)
        set_menu_visible_on(GMnu_CREATURE_QUERY1);
    else
        if (creature_query_on == 2)
      set_menu_visible_on(GMnu_CREATURE_QUERY2);
    else
        if (creature_query_on == 3)
        set_menu_visible_on(GMnu_CREATURE_QUERY3);
    else
        if (creature_query_on == 4)
        set_menu_visible_on(GMnu_CREATURE_QUERY4);
    else
    {
      WARNMSG("No active query for first person menu; assuming query 1.");
      set_menu_visible_on(GMnu_CREATURE_QUERY1);
    }
    return true;
  } else
  {
    long menu_num;
    // CREATURE_QUERY1
    menu_num = menu_id_to_number(GMnu_CREATURE_QUERY1);
    if (menu_num >= 0)
        creature_query_on = 1;
    set_menu_visible_off(GMnu_CREATURE_QUERY1);
    // CREATURE_QUERY2
    menu_num = menu_id_to_number(GMnu_CREATURE_QUERY2);
    if (menu_num >= 0)
        creature_query_on = 2;
    set_menu_visible_off(GMnu_CREATURE_QUERY2);
    // CREATURE_QUERY3
    menu_num = menu_id_to_number(GMnu_CREATURE_QUERY3);
    if (menu_num >= 0)
        creature_query_on = 3;
    set_menu_visible_off(GMnu_CREATURE_QUERY3);
    // CREATURE_QUERY4
    menu_num = menu_id_to_number(GMnu_CREATURE_QUERY4);
    if (menu_num >= 0)
        creature_query_on = 4;
    set_menu_visible_off(GMnu_CREATURE_QUERY4);
    return true;
  }
}

void set_gui_visible(TbBool visible)
{
  SYNCDBG(6,"Starting");
  set_flag_byte(&game.operation_flags,GOF_ShowGui,visible);
  struct PlayerInfo *player=get_my_player();
  unsigned char is_visbl = ((game.operation_flags & GOF_ShowGui) != 0);
  switch (player->view_type)
  {
  case PVT_CreatureContrl:
      toggle_first_person_menu(is_visbl);
      break;
  case PVT_MapScreen:
  case PVT_MapFadeIn:
  case PVT_MapFadeOut:
      toggle_status_menu(0);
      break;
  case PVT_DungeonTop:
  default:
      toggle_status_menu(is_visbl);
      break;
  }
  if (((game.numfield_D & GNFldD_Unkn20) != 0) && ((game.operation_flags & GOF_ShowGui) != 0))
  {
      setup_engine_window(status_panel_width, 0, MyScreenWidth, MyScreenHeight);
  }
  else
  {
      setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  }
  // Adjust the bounds of zoom of the camera when the side-menu is toggled (in Isometric view) to hide graphical glitches
  // Without the gui sidebar, the camera cannot be zoomed in as much.
  // NOTE: This should be reverted if the render array is ever increased (i.e. can see more things on screen)
  if (player->acamera && player->acamera->view_mode == PVM_IsometricView)
  {
      update_camera_zoom_bounds(player->acamera, CAMERA_ZOOM_MAX, adjust_min_camera_zoom(player->acamera, game.operation_flags & GOF_ShowGui));
      if (is_my_player(player))
      {
        settings.isometric_view_zoom_level = player->acamera->zoom;
        save_settings();
      }
  }
}

void toggle_gui(void)
{
    TbBool visible = ((game.operation_flags & GOF_ShowGui) == 0);
    set_gui_visible(visible);
}

void reinit_all_menus(void)
{
    TbBool visible = ((game.operation_flags & GOF_ShowGui) != 0);
    init_gui();
    reset_gui_based_on_player_mode();
    set_gui_visible(visible);
}

void frontend_load_data_from_cd(void)
{
    LbDataLoadSetModifyFilenameFunction(_DK_mdlf_for_cd);
}

void frontend_load_data_reset(void)
{
  LbDataLoadSetModifyFilenameFunction(_DK_mdlf_default);
}

void initialise_tab_tags(MenuID menu_id)
{
    info_tag =  (menu_id == GMnu_QUERY) || (menu_id == GMnu_CREATURE_QUERY1) ||
        (menu_id == GMnu_CREATURE_QUERY2) || (menu_id == GMnu_CREATURE_QUERY3) || (menu_id == GMnu_CREATURE_QUERY4);
    room_tag = (menu_id == GMnu_ROOM);
    spell_tag = (menu_id == GMnu_SPELL);
    trap_tag = (menu_id == GMnu_TRAP);
    creature_tag = (menu_id == GMnu_CREATURE);
}

void initialise_tab_tags_and_menu(MenuID menu_id)
{
    MenuNumber menu_num;
    initialise_tab_tags(menu_id);
    menu_num = menu_id_to_number(menu_id);
    if (menu_num >= 0) {
        setup_radio_buttons(get_active_menu(menu_num));
    }
}

void init_gui(void)
{
  LbMemorySet(breed_activities, 0, CREATURE_TYPES_COUNT*sizeof(unsigned short));
  LbMemorySet(menu_stack, 0, ACTIVE_MENUS_COUNT*sizeof(unsigned char));
  LbMemorySet(active_menus, 0, ACTIVE_MENUS_COUNT*sizeof(struct GuiMenu));
  LbMemorySet(active_buttons, 0, ACTIVE_BUTTONS_COUNT*sizeof(struct GuiButton));
  breed_activities[0] = get_players_special_digger_model(my_player_number);
  no_of_breeds_owned = 1;
  top_of_breed_list = 0;
  old_menu_mouse_x = -999;
  old_menu_mouse_y = -999;
  drag_menu_x = -999;
  drag_menu_y = -999;
  initialise_tab_tags(GMnu_ROOM);
  new_objective = 0;
  input_button = 0;
  busy_doing_gui = 0;
  no_of_active_menus = 0;
}

void frontend_shutdown_state(FrontendMenuState pstate)
{
    char *fname;
    switch (pstate)
    {
    case FeSt_INITIAL:
        init_gui();
        wait_for_cd_to_be_available();
        fname = prepare_file_path(FGrp_LoData,"front.pal");
        if (LbFileLoadAt(fname, frontend_palette) != PALETTE_SIZE)
            ERRORLOG("Unable to load FRONTEND PALETTE");
        wait_for_cd_to_be_available();
        LbMoveGameCursorToHostCursor(); // set the initial cursor position for the main menu
        update_mouse();
        break;
    case FeSt_MAIN_MENU: // main menu state
        turn_off_menu(GMnu_FEMAIN);
        break;
    case FeSt_FELOAD_GAME:
        turn_off_menu(GMnu_FELOAD);
        break;
    case FeSt_LAND_VIEW:
        frontmap_unload();
        frontend_load_data();
        break;
    case FeSt_NET_SERVICE:
        turn_off_menu(GMnu_FENET_SERVICE);
        break;
    case FeSt_NET_SESSION: // Network play mode
        turn_off_menu(GMnu_FENET_SESSION);
        break;
    case FeSt_NET_START:
        turn_off_menu(GMnu_FENET_START);
        break;
    case FeSt_STORY_POEM:
    case FeSt_STORY_BIRTHDAY:
        frontstory_unload();
        break;
    case FeSt_CREDITS:
        StopMusicPlayer();
        break;
    case FeSt_NET_MODEM:
        turn_off_menu(GMnu_FENET_MODEM);
        frontnet_modem_reset();
        break;
    case FeSt_NET_SERIAL:
        turn_off_menu(GMnu_FENET_SERIAL);
        frontnet_serial_reset();
        break;
    case FeSt_LEVEL_STATS:
        StopStreamedSample();
        turn_off_menu(GMnu_FESTATISTICS);
        break;
    case FeSt_HIGH_SCORES:
        turn_off_menu(GMnu_FEHIGH_SCORE_TABLE);
        break;
    case FeSt_TORTURE:
        set_pointer_graphic_none();
        fronttorture_clear_state();
        fronttorture_unload();
        frontend_load_data();
        break;
    case FeSt_NETLAND_VIEW:
        frontnetmap_unload();
        frontend_load_data();
        break;
    case FeSt_FEDEFINE_KEYS:
        turn_off_menu(GMnu_FEDEFINE_KEYS);
        save_settings();
        break;
    case FeSt_FEOPTIONS:
        turn_off_menu(GMnu_FEOPTION);
        StopMusicPlayer();
        break;
    case FeSt_LEVEL_SELECT:
        turn_off_menu(GMnu_FELEVEL_SELECT);
        frontend_level_list_unload();
        break;
    case FeSt_MAPPACK_SELECT:
        turn_off_menu(GMnu_MAPPACK_SELECT);
        break;
    case FeSt_CAMPAIGN_SELECT:
        turn_off_menu(GMnu_FECAMPAIGN_SELECT);
        break;
    case FeSt_START_KPRLEVEL:
    case FeSt_START_MPLEVEL:
    case FeSt_UNKNOWN09:
    case FeSt_LOAD_GAME:
    case FeSt_INTRO:
    case FeSt_CAMPAIGN_INTRO:
    case FeSt_DEMO: //demo state (intro/credits)
    case FeSt_OUTRO:
    case FeSt_PACKET_DEMO:
        break;
#if (BFDEBUG_LEVEL > 0)
    case FeSt_FONT_TEST:
        free_testfont_fonts();
        break;
#endif
    default:
        ERRORLOG("Unhandled FRONTEND state %d shutdown",(int)pstate);
        break;
    }
}

/**
 * Initializes given front-end state.
 * Loads required assets and initializes variables for each state.
 * Does not do network connection initialization for MP states.
 *
 * @param nstate The new state to be initialized.
 * @return The state which was really initialized.
 */
FrontendMenuState frontend_setup_state(FrontendMenuState nstate)
{
    SYNCDBG(9,"Starting for state %d",(int)nstate);
    switch ( nstate )
    {
      case FeSt_INITIAL:
          set_pointer_graphic_none();
          break;
      case FeSt_MAIN_MENU:
          continue_game_option_available = continue_game_available();
          turn_on_menu(GMnu_FEMAIN);
          last_mouse_x = GetMouseX();
          last_mouse_y = GetMouseY();
          time_last_played_demo = LbTimerClock();
          fe_high_score_table_from_main_menu = true;
          set_flag_byte(&game.system_flags, GSF_NetworkActive, false);
          set_pointer_graphic_menu();
          break;
      case FeSt_FELOAD_GAME:
          turn_on_menu(GMnu_FELOAD);
          set_pointer_graphic_menu();
          break;
      case FeSt_LAND_VIEW:
          set_pointer_graphic_none();
          if ( !frontmap_load() ) {
              // Fallback in case of error
              nstate = FeSt_START_KPRLEVEL;
          }
          break;
      case FeSt_NET_SERVICE:
          turn_on_menu(GMnu_FENET_SERVICE);
          frontnet_service_setup();
          set_pointer_graphic_menu();
          break;
      case FeSt_NET_SESSION:
          turn_on_menu(GMnu_FENET_SESSION);
          frontnet_session_setup();
          set_flag_byte(&game.system_flags, GSF_NetworkActive, false);
          set_pointer_graphic_menu();
          break;
      case FeSt_NET_START:
          turn_on_menu(GMnu_FENET_START);
          frontnet_start_setup();
          set_flag_byte(&game.system_flags, GSF_NetworkActive, true);
          set_pointer_graphic_menu();
          break;
      case FeSt_START_KPRLEVEL:
      case FeSt_UNKNOWN09:
      case FeSt_LOAD_GAME:
      case FeSt_INTRO:
      case FeSt_CAMPAIGN_INTRO:
      case FeSt_DEMO:
      case FeSt_OUTRO:
      case FeSt_PACKET_DEMO:
          fade_palette_in = 0;
          break;
      case FeSt_START_MPLEVEL:
          if ((game.flags_font & FFlg_unk10) != 0)
              LbNetwork_ChangeExchangeTimeout(30);
          fade_palette_in = 0;
          break;
      case FeSt_STORY_POEM:
      case FeSt_STORY_BIRTHDAY:
          set_pointer_graphic_none();
          frontstory_load();
          break;
      case FeSt_CREDITS:
          set_pointer_graphic_none();
          credits_offset = lbDisplay.PhysicalScreenHeight;
          credits_end = 0;
          LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
          lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
          break;
      case FeSt_NET_MODEM:
          turn_on_menu(GMnu_FENET_MODEM);
          frontnet_modem_setup();
          break;
      case FeSt_NET_SERIAL:
          turn_on_menu(GMnu_FENET_SERIAL);
          frontnet_serial_setup();
          break;
      case FeSt_LEVEL_STATS:
          turn_on_menu(GMnu_FESTATISTICS);
          frontstats_set_timer();
          set_pointer_graphic_menu();
          break;
      case FeSt_HIGH_SCORES:
          turn_on_menu(GMnu_FEHIGH_SCORE_TABLE);
          frontstats_save_high_score();
          set_pointer_graphic_menu();
          break;
      case FeSt_TORTURE:
          set_pointer_graphic_none();
          fronttorture_load();
          break;
      case FeSt_NETLAND_VIEW:
          set_pointer_graphic_none();
          frontnet_init_level_descriptions();
          frontnetmap_load();
          break;
      case FeSt_FEDEFINE_KEYS:
          defining_a_key = 0;
          define_key_scroll_offset = 0;
          turn_on_menu(GMnu_FEDEFINE_KEYS);
          break;
      case FeSt_FEOPTIONS:
          turn_on_menu(GMnu_FEOPTION);
          set_pointer_graphic_menu();
          break;
    case FeSt_LEVEL_SELECT:
        turn_on_menu(GMnu_FELEVEL_SELECT);
        frontend_level_list_load();
        set_pointer_graphic_menu();
        break;
    case FeSt_MAPPACK_SELECT:
        turn_on_menu(GMnu_MAPPACK_SELECT);
        frontend_mappack_list_load();
        set_pointer_graphic_menu();
        break;
    case FeSt_CAMPAIGN_SELECT:
        turn_on_menu(GMnu_FECAMPAIGN_SELECT);
        frontend_campaign_list_load();
        set_pointer_graphic_menu();
        break;
  #if (BFDEBUG_LEVEL > 0)
    case FeSt_FONT_TEST:
        fade_palette_in = 0;
        load_testfont_fonts();
        set_pointer_graphic_menu();
        break;
  #endif
      default:
        ERRORLOG("Unhandled FRONTEND new state");
        break;
    }
    return nstate;
}

FrontendMenuState frontend_set_state(FrontendMenuState nstate)
{
    SYNCDBG(8,"State %d will be switched to %d",(int)frontend_menu_state,(int)nstate);
    frontend_shutdown_state(frontend_menu_state);
    if ( frontend_menu_state )
      fade_out();
    fade_palette_in = 1;
    SYNCMSG("Frontend state change from %d into %d",(int)frontend_menu_state,(int)nstate);
    frontend_menu_state = frontend_setup_state(nstate);
    return frontend_menu_state;
}

TbBool frontmainmnu_input(void)
{
    int mouse_x;
    int mouse_y;
    // check if mouse position has changed
    mouse_x = GetMouseX();
    mouse_y = GetMouseY();
    if ((mouse_x != last_mouse_x) || (mouse_y != last_mouse_y))
    {
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        time_last_played_demo = LbTimerClock();
    }
    // Handle key inputs
    if (lbKeyOn[KC_G] && lbKeyOn[KC_LSHIFT])
    {
        lbKeyOn[KC_G] = 0;
        frontend_set_state(FeSt_CREDITS);
        return true;
    }
    if (lbKeyOn[KC_T] && lbKeyOn[KC_LSHIFT])
    {
        if ((game.flags_font & FFlg_AlexCheat) != 0)
        {
            lbKeyOn[KC_T] = 0;
            set_player_as_won_level(get_my_player());
            frontend_set_state(FeSt_TORTURE);
            return true;
        }
    }
#if (BFDEBUG_LEVEL > 0)
    if (lbKeyOn[KC_F] && lbKeyOn[KC_LSHIFT])
    {
        if ((game.flags_font & FFlg_AlexCheat) != 0)
        {
            lbKeyOn[KC_F] = 0;
            frontend_set_state(FeSt_FONT_TEST);
            return true;
        }
    }
#endif
    return false;
}

TbBool front_continue_pressed(TbBool force)
{
    TbBool got_input;
    got_input = force;
    if (lbKeyOn[KC_SPACE])
    {
        lbKeyOn[KC_SPACE] = 0;
        got_input = true;
    }
    if (lbKeyOn[KC_RETURN])
    {
        lbKeyOn[KC_RETURN] = 0;
        got_input = true;
    }
    if (left_button_clicked)
    {
        left_button_clicked = 0;
        got_input = true;
    }
    return got_input;
}

TbBool frontscreen_end_input(TbBool force)
{
    TbBool do_change;
    do_change = force;
    if (lbKeyOn[KC_ESCAPE])
    {
        lbKeyOn[KC_ESCAPE] = 0;
        do_change = true;
    }
    if (right_button_clicked)
    {
        right_button_clicked = 0;
        do_change = true;
    }
    if (do_change)
    {
        FrontendMenuState nstate;
        nstate = get_menu_state_when_back_from_substate(frontend_menu_state);
        if (nstate != frontend_menu_state) {
            frontend_set_state(nstate);
        } else {
            do_change = false;
        }
    }
    return do_change;
}

short get_frontend_global_inputs(void)
{
    if (is_key_pressed(KC_X, KMod_ALT))
    {
        clear_key_pressed(KC_X);
        exit_keeper = true;
    } else {
        return false;
    }
    return true;
}

void frontend_input(void)
{
    SYNCDBG(7,"Starting");
    TbBool input_consumed;
    input_consumed = false;
    switch (frontend_menu_state)
    {
    case FeSt_MAIN_MENU:
        get_gui_inputs(0);
        input_consumed = frontscreen_end_input(false);
        if (input_consumed) {
            break;
        }
        input_consumed = frontmainmnu_input();
        break;
    case FeSt_LAND_VIEW:
        frontmap_input();
        break;
    case FeSt_NET_SESSION:
    case FeSt_NET_MODEM:
    case FeSt_NET_SERIAL:
        get_gui_inputs(0);
        break;
    case FeSt_NET_START:
        get_gui_inputs(0);
        frontnet_start_input();
        break;
    case FeSt_STORY_POEM:
    case FeSt_STORY_BIRTHDAY:
        input_consumed = frontscreen_end_input(front_continue_pressed(false));
        if (input_consumed) {
            break;
        }
        input_consumed = frontstory_input();
        break;
    case FeSt_CREDITS:
        input_consumed = frontscreen_end_input(front_continue_pressed(credits_end));
        if (input_consumed) {
            break;
        }
        frontcredits_input();
        break;
    case FeSt_HIGH_SCORES:
        get_gui_inputs(0);
        if (high_score_entry_input_active < 0) {
            input_consumed = frontscreen_end_input(front_continue_pressed(false));
        }
        if (input_consumed) {
            break;
        }
        input_consumed = frontend_high_score_table_input();
        break;
    case FeSt_TORTURE:
        fronttorture_input();
        break;
    case FeSt_NETLAND_VIEW:
        frontnetmap_input();
        break;
    case FeSt_FEDEFINE_KEYS:
        if (!defining_a_key) {
            get_gui_inputs(0);
            input_consumed = frontscreen_end_input(false);
        } else {
            define_key_input();
            input_consumed = true;
        }
        break;
#if (BFDEBUG_LEVEL > 0)
    case FeSt_FONT_TEST:
        get_gui_inputs(0);
        input_consumed = frontscreen_end_input(false);
        if (input_consumed) {
            break;
        }
        fronttestfont_input();
        break;
#endif
    default:
        get_gui_inputs(0);
        input_consumed = frontscreen_end_input(false);
        break;
    } // end switch
    get_frontend_global_inputs();
    if (!input_consumed) {
        get_screen_capture_inputs();
    }
    SYNCDBG(19,"Finished");
}

void draw_defining_a_key_box(void)
{
    draw_text_box(get_string(GUIStr_PressAKey));
}

char update_menu_fade_level(struct GuiMenu *gmnu)
{
    //return _DK_update_menu_fade_level(gmnu);
    int i;
    switch (gmnu->visual_state)
    {
    case 1:
        if (gmnu->fade_time == 0) {
            gmnu->fade_time = 1;
            return 0;
        }
        gmnu->fade_time--;
        if (gmnu->fade_time != 0) {
            return 0;
        }
        i = gmnu->menu_init->fade_time;
        if (i)
        {
            gmnu->visual_state = 2;
            gmnu->fade_time = i;
        } else
        {
            gmnu->visual_state = 2;
            gmnu->fade_time = 1;
        }
        return 1;
    case 3:
        if (gmnu->fade_time == 0) {
            return -1;
        }
        gmnu->fade_time--;
        if (gmnu->fade_time <= 0) {
            return -1;
        }
        return 0;
    default:
        break;
    }
    return 0;
}

void toggle_gui_overlay_map(void)
{
    toggle_flag_byte(&game.operation_flags,GOF_ShowGui);
}

void draw_menu_buttons(struct GuiMenu *gmnu)
{
    int i;
    struct GuiButton *gbtn;
    Gf_Btn_Callback callback;
    SYNCDBG(18,"Starting phase one");
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        callback = gbtn->draw_call;
        if ((callback != NULL) && (gbtn->flags & LbBtnF_Visible) && (gbtn->flags & LbBtnF_Active) && (gbtn->gmenu_idx == gmnu->number))
        {
          if ( ((gbtn->gbactn_1 == 0) && (gbtn->gbactn_2 == 0)) || (gbtn->gbtype == LbBtnT_HorizSlider) || (callback == gui_area_null) )
            callback(gbtn);
        }
    }
    SYNCDBG(18,"Starting phase two");
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        callback = gbtn->draw_call;
        if ((callback != NULL) && (gbtn->flags & LbBtnF_Visible) && (gbtn->flags & LbBtnF_Active) && (gbtn->gmenu_idx == gmnu->number))
        {
          if (((gbtn->gbactn_1) || (gbtn->gbactn_2)) && (gbtn->gbtype != LbBtnT_HorizSlider) && (callback != gui_area_null))
            callback(gbtn);
        }
    }
    SYNCDBG(19,"Finished");
}

void update_fade_active_menus(void)
{
    SYNCDBG(8,"Starting");
    struct GuiMenu *gmnu;
    int k;
    for (k=0; k < ACTIVE_MENUS_COUNT; k++)
    {
        gmnu = &active_menus[k];
        if (update_menu_fade_level(gmnu) == -1)
        {
            kill_menu(gmnu);
            remove_from_menu_stack(gmnu->ident);
        }
    }
    SYNCDBG(19,"Finished");
}

void draw_active_menus_buttons(void)
{
    struct GuiMenu *gmnu;
    int k;
    long menu_num;
    Gf_Mnu_Callback callback;
    SYNCDBG(8,"Starting with %d active menus",no_of_active_menus);
    for (k=0; k < no_of_active_menus; k++)
    {
        menu_num = menu_id_to_number(menu_stack[k]);
        if (menu_num < 0) continue;
        gmnu = &active_menus[menu_num];
        //SYNCMSG("DRAW menu %d, fields %d, %d",menu_num,gmnu->visual_state,gmnu->is_turned_on);
        if ((gmnu->visual_state != 0) && (gmnu->is_turned_on))
        {
            if ((gmnu->visual_state != 2) && (gmnu->fade_time))
            {
              if (gmnu->menu_init != NULL)
                if (gmnu->menu_init->fade_time)
                  lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
            }
            callback = gmnu->draw_cb;
            if (callback != NULL)
              callback(gmnu);
            if (gmnu->visual_state == 2)
              draw_menu_buttons(gmnu);
            lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
        }
    }
    SYNCDBG(9,"Finished");
}

void spangle_button(struct GuiButton *gbtn)
{
    struct TbSprite *spr;
    spr = &button_sprite[176];
    int bs_units_per_px;
    bs_units_per_px = 50 * units_per_pixel / spr->SHeight;
    long x;
    long y;
    unsigned long i;
    x = gbtn->pos_x + (gbtn->width >> 1)  - ((spr->SWidth*bs_units_per_px/16) / 2);
    y = gbtn->pos_y + (gbtn->height >> 1) - ((spr->SHeight*bs_units_per_px/16) / 2);
    i = 176+((game.play_gameturn >> 1) & 7);
    spr = &button_sprite[i];
    LbSpriteDrawResized(x, y, bs_units_per_px, spr);
}

void draw_menu_spangle(struct GuiMenu *gmnu)
{
    struct GuiButton *gbtn;
    int i;
    if (gmnu->is_turned_on == 0)
      return;
    for (i=0; i<ACTIVE_BUTTONS_COUNT; i++)
    {
        gbtn = &active_buttons[i];
        if ((gbtn->draw_call == NULL) || ((gbtn->flags & LbBtnF_Visible) == 0) || ((gbtn->flags & LbBtnF_Active) == 0) || (game.flash_button_index == 0))
          continue;
        if ((gbtn->id_num > BID_DEFAULT) && (gbtn->id_num == button_designation_to_tab_designation(game.flash_button_index)))
        {
            // Button is a tab header; spangle if the tab is not active
            if (!menu_is_active(gbtn->btype_value&LbBFeF_IntValueMask))
                spangle_button(gbtn);
        } else
        if ((gbtn->id_num > BID_DEFAULT) && (gbtn->id_num == game.flash_button_index))
        {
            spangle_button(gbtn);
        }
    }
}

void draw_active_menus_highlights(void)
{
    struct GuiMenu *gmnu;
    int k;
    SYNCDBG(8,"Starting");
    for (k=0; k<ACTIVE_MENUS_COUNT; k++)
    {
        gmnu = &active_menus[k];
        if ((gmnu->visual_state != 0) && (gmnu->ident == GMnu_MAIN))
          draw_menu_spangle(gmnu);
    }
}

void draw_gui(void)
{
    SYNCDBG(6,"Starting");
    unsigned int flg_mem;
    LbTextSetFont(winfont);
    flg_mem = lbDisplay.DrawFlags;
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    update_fade_active_menus();
    draw_active_menus_buttons();
    if (game.flash_button_index != 0)
    {
        draw_active_menus_highlights();
        if (game.flash_button_gameturns != -1)
        {
            game.flash_button_gameturns--;
            if (game.flash_button_gameturns == -1)
              game.flash_button_index = 0;
        }
    }
    lbDisplay.DrawFlags = flg_mem;
    SYNCDBG(8,"Finished");
}

/**
 * Frontend drawing function.
 * @return Gives 0 if a movie has started, 1 if normal draw occured, 2 on error.
 */
short frontend_draw(void)
{
    short result;
    switch (frontend_menu_state)
    {
    case FeSt_INTRO:
        intro();
        return 0;
    case FeSt_DEMO:
        demo();
        return 0;
    case FeSt_CAMPAIGN_INTRO:
        campaign_intro();
        return 0;
    case FeSt_DRAG:
        drag_video();
        return 0;
    case FeSt_OUTRO:
        campaign_outro();
        return 0;
    }

    if (LbScreenLock() != Lb_SUCCESS)
        return 2;

    result = 1;
    switch ( frontend_menu_state )
    {
    case FeSt_MAIN_MENU:
    case FeSt_FELOAD_GAME:
    case FeSt_NET_SERVICE:
    case FeSt_NET_SESSION:
    case FeSt_NET_MODEM:
    case FeSt_NET_SERIAL:
    case FeSt_NET_START:
    case FeSt_LEVEL_STATS:
    case FeSt_HIGH_SCORES:
    case FeSt_UNKNOWN20:
    case FeSt_FEOPTIONS:
    case FeSt_LEVEL_SELECT:
    case FeSt_MAPPACK_SELECT:
    case FeSt_CAMPAIGN_SELECT:
        frontend_copy_background();
        draw_gui();
        break;
    case FeSt_LAND_VIEW:
        frontmap_draw();
        break;
    case FeSt_STORY_POEM:
        frontstory_draw();
        break;
    case FeSt_CREDITS:
        frontcredits_draw();
        break;
    case FeSt_TORTURE:
        fronttorture_draw();
        break;
    case FeSt_NETLAND_VIEW:
        frontnetmap_draw();
        break;
    case FeSt_FEDEFINE_KEYS:
        frontend_copy_background();
        draw_gui();
        if ( defining_a_key )
            draw_defining_a_key_box();
        break;
    case FeSt_STORY_BIRTHDAY:
        frontbirthday_draw();
        break;
#if (BFDEBUG_LEVEL > 0)
    case FeSt_FONT_TEST:
        fronttestfont_draw();
        break;
#endif
    default:
        break;
    }
    // In-Menu information, for debugging messages
    //char text[255];
    //sprintf(text, "time %7d, mode %d",LbTimerClock(),frontend_menu_state);
    //lbDisplay.DrawFlags=0;LbTextSetWindow(0,0,640,200);LbTextSetFont(frontend_font[0]);
    //LbTextDraw(200/pixel_size, 8/pixel_size, text);text[0]='\0';
    perform_any_screen_capturing();
    LbScreenUnlock();
    return result;
}

void load_game_update(void)
{
    if ((number_of_saved_games>0) && (load_game_scroll_offset>=0))
    {
        if ( load_game_scroll_offset > number_of_saved_games-1 )
          load_game_scroll_offset = number_of_saved_games-1;
    } else
    {
        load_game_scroll_offset = 0;
    }
}

void gui_set_autopilot(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  player = get_my_player();
  int ntype;
  if (game.comp_player_aggressive)
  {
    ntype = 1;
  } else
  if (game.comp_player_defensive)
  {
    ntype = 2;
  } else
  if (game.comp_player_construct)
  {
    ntype = 3;
  } else
  if (game.comp_player_creatrsonly)
  {
    ntype = 4;
  } else
  {
    ERRORLOG("Illegal Autopilot type, resetting to default");
    ntype = 1;
  }
  set_players_packet_action(player, PckA_SetComputerKind, ntype, 0, 0, 0);
}

void set_level_objective(const char *msg_text)
{
    if (msg_text == NULL)
    {
        ERRORLOG("Invalid message pointer");
        return;
    }
    strncpy(game.evntbox_text_objective, msg_text, MESSAGE_TEXT_LEN);
    new_objective = 1;
}

void update_player_objectives(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    SYNCDBG(6,"Starting for player %d",(int)plyr_idx);
    player = get_player(plyr_idx);
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
      if ((!player->field_4EB) && (player->victory_state != VicS_Undecided))
        player->field_4EB = game.play_gameturn+1;
    }
    if (player->field_4EB == game.play_gameturn)
    {
      switch (player->victory_state)
      {
      case VicS_WonLevel:
          if (plyr_idx == my_player_number)
            set_level_objective(get_string(CpgStr_SuccessLandIsYours));
          display_objectives(player->id_number, 0, 0);
          break;
      case VicS_LostLevel:
          if (plyr_idx == my_player_number)
            set_level_objective(get_string(CpgStr_LevelLost));
          display_objectives(player->id_number, 0, 0);
          break;
      }
    }
}

void display_objectives(PlayerNumber plyr_idx, long x, long y)
{
    //_DK_display_objectives(plyr_idx,x,y);
    long cor_x;
    long cor_y;
    cor_y = 0;
    cor_x = 0;
    if ((x > 0) || (y > 0))
    {
        cor_x = subtile_coord_center(x);
        cor_y = subtile_coord_center(y);
    }
    int evbtn_idx;
    for (evbtn_idx=0; evbtn_idx < EVENT_BUTTONS_COUNT+1; evbtn_idx++)
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(plyr_idx);
        EventIndex evidx;
        evidx = dungeon->event_button_index[evbtn_idx];
        struct Event *event;
        event = &game.event[evidx];
        if (event->kind == EvKind_Objective)
        {
            event_create_event_or_update_old_event(cor_x, cor_y, EvKind_Objective, plyr_idx, 0);
            return;
        }
    }
    if ((x == 255) && (y == 255))
    {
        struct Thing *creatng;
        creatng = lord_of_the_land_find();
        if (!thing_is_invalid(creatng))
        {
            cor_x = creatng->mappos.x.val;
            cor_y = creatng->mappos.y.val;
        }
        event_create_event_or_update_nearby_existing_event(cor_x, cor_y, EvKind_Objective, plyr_idx, creatng->index);
    } else
    {
        event_create_event_or_update_nearby_existing_event(cor_x, cor_y, EvKind_Objective, plyr_idx, 0);
    }
}

void frontend_update(short *finish_menu)
{
    SYNCDBG(18,"Starting for menu state %d", (int)frontend_menu_state);
    switch ( frontend_menu_state )
    {
    case FeSt_MAIN_MENU:
        StopMusicPlayer();
        frontend_button_info[8].font_index = (continue_game_option_available?1:3);
        //this uses original timing function for compatibility with frontend_set_state()
        if ( abs(LbTimerClock()-(long)time_last_played_demo) > MNU_DEMO_IDLE_TIME )
          frontend_set_state(FeSt_DEMO);
        break;
    case FeSt_FELOAD_GAME:
        load_game_update();
        break;
    case FeSt_LAND_VIEW:
        *finish_menu = frontmap_update();
        break;
    case FeSt_CAMPAIGN_INTRO:
        break;
    case FeSt_NET_SERVICE:
        frontnet_service_update();
        break;
    case FeSt_NET_SESSION:
        frontnet_session_update();
        break;
    case FeSt_NET_START:
        frontnet_start_update();
        break;
    case FeSt_START_KPRLEVEL:
    case FeSt_START_MPLEVEL:
    case FeSt_LOAD_GAME:
    case FeSt_PACKET_DEMO:
        *finish_menu = 1;
        break;
    case FeSt_UNKNOWN09:
        *finish_menu = 1;
        exit_keeper = 1;
        break;
    case FeSt_CREDITS:
        PlayMusicPlayer(7);
        break;
    case FeSt_NET_MODEM:
        frontnet_modem_update();
        break;
    case FeSt_NET_SERIAL:
        frontnet_serial_update();
        break;
    case FeSt_LEVEL_STATS:
        frontstats_update();
        break;
    case FeSt_TORTURE:
        fronttorture_update();
        break;
    case FeSt_NETLAND_VIEW:
        *finish_menu = frontnetmap_update();
        break;
    case FeSt_FEOPTIONS:
        PlayMusicPlayer(3);
        break;
    case FeSt_LEVEL_SELECT:
        frontend_level_select_update();
        break;
    case FeSt_CAMPAIGN_SELECT:
        frontend_campaign_select_update();
        break;
    case FeSt_MAPPACK_SELECT:
        frontend_mappack_select_update();
        break;
    default:
        break;
    }
    SYNCDBG(17,"Finished");
}

/**
 * Chooses frontend menu state to which we should return when exiting specific level.
 */
FrontendMenuState get_menu_state_based_on_last_level(LevelNumber lvnum)
{
    if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum) || is_extra_level(lvnum))
    {
        return FeSt_LAND_VIEW;
    } else
    if (is_multiplayer_level(lvnum))
    {
        return FeSt_NET_SERVICE;
    } else
    if (is_freeplay_level(lvnum))
    {
        return FeSt_LEVEL_SELECT;
    } else
    {
        return FeSt_MAIN_MENU;
    }
}

/**
 * Chooses frontend menu state to which we should return when exiting another frontend menu.
 * @return The frontend menu state to be used when a substate is cancelled.
 */
FrontendMenuState get_menu_state_when_back_from_substate(FrontendMenuState substate)
{
    LevelNumber lvnum;
    struct PlayerInfo *player;
    switch (substate)
    {
    case FeSt_START_KPRLEVEL:
    case FeSt_START_MPLEVEL:
        lvnum = get_loaded_level_number();
        return get_menu_state_based_on_last_level(lvnum);
    case FeSt_LOAD_GAME:
        return FeSt_START_KPRLEVEL;
    case FeSt_NET_START:
        return FeSt_NET_SESSION;
    case FeSt_NET_SESSION:
    case FeSt_NET_MODEM:
    case FeSt_NET_SERIAL:
    case FeSt_NETLAND_VIEW:
        return FeSt_NET_SERVICE;
    case FeSt_TORTURE:
    case FeSt_CAMPAIGN_INTRO:
        return FeSt_LAND_VIEW;
    case FeSt_OUTRO:
        return FeSt_LEVEL_STATS;
    case FeSt_DRAG:
        return FeSt_TORTURE;
    case FeSt_LEVEL_STATS:
        if ((game.system_flags & GSF_NetworkActive) != 0)
            return FeSt_NET_SESSION;
        player = get_my_player();
        if (player->victory_state == VicS_WonLevel)
            return FeSt_HIGH_SCORES;
        lvnum = get_loaded_level_number();
        return get_menu_state_based_on_last_level(lvnum);
    case FeSt_HIGH_SCORES:
        if (fe_high_score_table_from_main_menu)
            return FeSt_MAIN_MENU;
        lvnum = get_loaded_level_number();
        return get_menu_state_based_on_last_level(lvnum);
    case FeSt_FEDEFINE_KEYS:
        return FeSt_FEOPTIONS;
    case FeSt_CREDITS:
        return FeSt_MAIN_MENU;
    default:
        return FeSt_MAIN_MENU;
    }
}

/**
 * Chooses initial frontend menu state.
 * Used when game is first run, or player exits from gameplay.
 */
FrontendMenuState get_startup_menu_state(void)
{
  struct PlayerInfo *player;
  LevelNumber lvnum;
  if (game_flags2 & GF2_Server)
  {
      game_flags2 &= ~GF2_Server;
      SYNCLOG("Setup server");

      if (setup_network_service(NS_TCP_IP))
      {
          frontnet_service_setup();
          frontnet_session_setup();
          frontnet_session_create(NULL);
          return FeSt_NET_START;
      }
  }
  else if (game_flags2 & GF2_Connect)
  {
      game_flags2 &= ~GF2_Connect;
      SYNCLOG("Setup client");
      if (setup_network_service(NS_TCP_IP))
      {
          frontnet_service_setup();
          frontnet_session_setup();
          net_number_of_sessions = 0;
          LbMemorySet(net_session, 0, sizeof(net_session));
          // TODO: should disable actual network enumerating if either
          if (LbNetwork_EnumerateSessions(enum_sessions_callback, 0))
          {
              ERRORLOG("LbNetwork_EnumerateSessions() failed");
          }
          else
          {
              net_session_index_active = 0;
              frontnet_session_join(NULL);
              return frontend_menu_state;
          }
      }
  }
  else if ((game.flags_cd & MFlg_unk40) != 0)
  { // If starting up the game after intro
    if (is_full_moon)
    {
        SYNCLOG("Full moon state selected");
        return FeSt_STORY_POEM;
    } else
    if (get_team_birthday() != NULL)
    {
        SYNCLOG("Birthday state selected");
        return FeSt_STORY_BIRTHDAY;
    } else
    {
        SYNCLOG("Standard startup state selected");
        return FeSt_MAIN_MENU;
    }
  } else
  {
    player = get_my_player();
    lvnum = get_loaded_level_number();
    if ((game.system_flags & GSF_NetworkActive) != 0)
    { // If played real network game, then resulting screen isn't changed based on victory
        SYNCLOG("Network game summary state selected");
        if ((player->additional_flags & PlaAF_UnlockedLordTorture) != 0)
        { // Player has tortured LOTL - go FeSt_TORTURE before any others
          player->additional_flags &= ~PlaAF_UnlockedLordTorture;
          return FeSt_DRAG;
        } else
        if ((player->flgfield_6 & PlaF6_PlyrHasQuit) == 0)
        {
          return FeSt_LEVEL_STATS;
        } else
        if (setup_old_network_service())
        {
          return FeSt_NET_SESSION;
        } else
        {
          return FeSt_MAIN_MENU;
        }
    } else
    if (((player->flgfield_6 & PlaF6_PlyrHasQuit) != 0) || (player->victory_state == VicS_Undecided))
    {
        SYNCLOG("Undecided victory state selected");
        return get_menu_state_based_on_last_level(lvnum);
    } else
    if (game.flags_cd & MFlg_IsDemoMode)
    { // It wasn't a real game, just a demo - back to main menu
        SYNCLOG("Demo mode state selected");
        game.flags_cd &= ~MFlg_IsDemoMode;
        return FeSt_MAIN_MENU;
    } else
    if (player->victory_state == VicS_WonLevel)
    {
        SYNCLOG("Victory achieved state selected");
        if (is_singleplayer_level(lvnum))
        {
            if (get_continue_level_number() == SINGLEPLAYER_FINISHED)
            {
                return FeSt_OUTRO;
            } else
            if ((player->additional_flags & PlaAF_UnlockedLordTorture) != 0)
            {
                player->additional_flags &= ~PlaAF_UnlockedLordTorture;
                return FeSt_DRAG;
            } else
            {
                return FeSt_LEVEL_STATS;
            }
        } else
        if (is_bonus_level(lvnum) || is_extra_level(lvnum))
        {
            return FeSt_LAND_VIEW;
        } else
        {
            return FeSt_LEVEL_STATS;
        }
    } else
    if (player->victory_state == VicS_State3)
    {
        SYNCLOG("Victory st3 state selected");
        return FeSt_LEVEL_STATS;
    } else
    {
        SYNCLOG("Lost level state selected");
        return get_menu_state_based_on_last_level(lvnum);
    }
  }
  ERRORLOG("Unresolved menu state");
  return FeSt_MAIN_MENU;
}

void try_restore_frontend_error_box()
{
    if (LbTimerClock() < gui_message_timeout)
    {
        turn_on_menu(GMnu_FEERROR_BOX);
    }
}

void create_frontend_error_box(long showTime, const char * text)
{
    strncpy(gui_message_text, text, TEXT_BUFFER_LENGTH-1);
    gui_message_text[TEXT_BUFFER_LENGTH-1] = '\0';
    gui_message_timeout = LbTimerClock()+showTime;
    turn_on_menu(GMnu_FEERROR_BOX);
}

void frontend_draw_error_text_box(struct GuiButton *gbtn)
{
    draw_text_box((char *)gbtn->content);
}

void frontend_maintain_error_text_box(struct GuiButton *gbtn)
{
    if (LbTimerClock() > gui_message_timeout)
    {
        turn_off_menu(GMnu_FEERROR_BOX);
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
