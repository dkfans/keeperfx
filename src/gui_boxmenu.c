/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_boxmenu.c
 *     Displaying service menu on screen.
 * @par Purpose:
 *     Functions to maintain on-screen service and cheat menu boxes.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_boxmenu.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "frontend.h"
#include "creature_instances.h"
#include "player_data.h"
#include "player_instances.h"
#include "player_states.h"
#include "player_utils.h"
#include "kjm_input.h"
#include "packets.h"
#include "thing_data.h"
#include "gui_draw.h"
#include "game_legacy.h"
#include "vidfade.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*
extern struct GuiBoxOption gui_main_cheat_list[];
extern struct GuiBoxOption gui_creature_cheat_option_list[];
extern struct GuiBoxOption gui_instance_option_list[];
*/

long gf_change_player_state(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_change_player_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_research_rooms(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_make_everything_free(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_give_all_creatures_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_explore_everywhere(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_research_magic(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_all_researchable(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gfa_can_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag);
long gfa_controlled_creature_has_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag);
long gf_decide_victory(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);

struct GuiBoxOption gui_main_cheat_list[] = { //gui_main_option_list in beta
  {"Null mode",                1,           NULL,      gf_change_player_state, 0, 0, 0,            PSt_None, 0, 0, 0, 0},
  {"Place tunneller mode",     1,           NULL,      gf_change_player_state, 0, 0, 0,    PSt_MkGoodDigger, 0, 0, 0, 0},
  {"Place creature mode",      1,           NULL,      gf_change_player_state, 0, 0, 0,     PSt_MkBadCreatr, 0, 0, 0, 0},
  {"Place hero mode",          1,           NULL,      gf_change_player_state, 0, 0, 0,    PSt_MkGoodCreatr, 0, 0, 0, 0},
  {"Destroy walls mode",       1,           NULL,      gf_change_player_state, 0, 0, 0,PSt_FreeDestroyWalls, 0, 0, 0, 0},
  {"Disease mode",             1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_FreeCastDisease, 0, 0, 0, 0},
  {"Peter mode",               1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_FreeTurnChicken, 0, 0, 0, 0},
  {"Create gold mode",         1,           NULL,      gf_change_player_state, 0, 0, 0,       PSt_MkGoldPot, 0, 0, 0, 0},
  {"Steal room mode",          1,           NULL,      gf_change_player_state, 0, 0, 0,       PSt_StealRoom, 0, 0, 0, 0},
  {"Destroy room mode",        1,           NULL,      gf_change_player_state, 0, 0, 0,     PSt_DestroyRoom, 0, 0, 0, 0},
  {"Steal slab mode",          1,           NULL,      gf_change_player_state, 0, 0, 0,       PSt_StealSlab, 0, 0, 0, 0},
  {"Place terrain mode",       1,           NULL,      gf_change_player_state, 0, 0, 0,    PSt_PlaceTerrain, 0, 0, 0, 0},
  {"",                         2,           NULL,                        NULL, 0, 0, 0,            PSt_None, 0, 0, 0, 0},
  {"Passenger control mode",   1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_FreeCtrlPassngr, 0, 0, 0, 0},
  {"Direct control mode",      1,           NULL,      gf_change_player_state, 0, 0, 0,  PSt_FreeCtrlDirect, 0, 0, 0, 0},
  {"Order creature mode",      1,           NULL,      gf_change_player_state, 0, 0, 0,     PSt_OrderCreatr, 0, 0, 0, 0},
  {"Kill creature mode",       1,           NULL,      gf_change_player_state, 0, 0, 0,      PSt_KillCreatr, 0, 0, 0, 0},
  {"Destroy thing mode",       1,           NULL,      gf_change_player_state, 0, 0, 0,    PSt_DestroyThing, 0, 0, 0, 0},
  {"Turncoat mode",            1,           NULL,      gf_change_player_state, 0, 0, 0,   PSt_ConvertCreatr, 0, 0, 0, 0},
  {"Level up mode",            1,           NULL,      gf_change_player_state, 0, 0, 0, PSt_LevelCreatureUp, 0, 0, 0, 0},
  {"Level down mode",          1,           NULL,    gf_change_player_state, 0, 0, 0, PSt_LevelCreatureDown, 0, 0, 0, 0},
  {"Query mode",               1,           NULL,      gf_change_player_state, 0, 0, 0,  PSt_CreatrQueryAll, 0, 0, 0, 0},
  {"Make happy mode",          1,           NULL,      gf_change_player_state, 0, 0, 0,         PSt_MkHappy, 0, 0, 0, 0},
  {"Make angry mode",          1,           NULL,      gf_change_player_state, 0, 0, 0,         PSt_MkAngry, 0, 0, 0, 0},
  {"",                         2,           NULL,                        NULL, 0, 0, 0,            PSt_None, 0, 0, 0, 0},
  {"Kill player mode",         1,           NULL,      gf_change_player_state, 0, 0, 0,      PSt_KillPlayer, 0, 0, 0, 0},
  {"Edit heart health",        1,           NULL,      gf_change_player_state, 0, 0, 0,     PSt_HeartHealth, 0, 0, 0, 0},
  {"!",                        0,           NULL,                        NULL, 0, 0, 0,            PSt_None, 0, 0, 0, 0},
};

struct GuiBoxOption gui_creature_cheat_option_list[] = {
 {"Everything is free",        1,           NULL,     gf_make_everything_free, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Give controlled creature spells",1,gfa_can_give_controlled_creature_spells,gf_give_controlled_creature_spells, 0, 0, 0, 0, 0, 0, 0, 0},
 {"Give all creatures spells", 1,           NULL,gf_give_all_creatures_spells, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Explore everywhere",        1,           NULL,       gf_explore_everywhere, 0, 0, 0,               0, 0, 0, 0, 0},
 {"All rooms and magic researchable",1,     NULL,         gf_all_researchable, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Research all magic",        1,           NULL,           gf_research_magic, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Research all rooms",        1,           NULL,           gf_research_rooms, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Win the level instantly",   1,           NULL,           gf_decide_victory, 0, 0, 0,               1, 0, 0, 0, 0},
 {"Lose the level instantly",  1,           NULL,           gf_decide_victory, 0, 0, 0,               0, 0, 0, 0, 0},
 {"!",                         0,           NULL,                        NULL, 0, 0, 0,               0, 0, 0, 0, 0},
};

struct GuiBoxOption gui_instance_option_list[] = {
 {"Fireball",1,gfa_controlled_creature_has_instance,gf_change_player_instance, 5, 0, 0,  5, 0, 0, 0, 0},
 {"Meteor",1, gfa_controlled_creature_has_instance, gf_change_player_instance, 6, 0, 0,  6, 0, 0, 0, 0},
 {"Freeze",1, gfa_controlled_creature_has_instance, gf_change_player_instance, 7, 0, 0,  7, 0, 0, 0, 0},
 {"Armour",1, gfa_controlled_creature_has_instance, gf_change_player_instance, 8, 0, 0,  8, 0, 0, 0, 0},
 {"Lightning",1,gfa_controlled_creature_has_instance,gf_change_player_instance,9, 0, 0,  9, 0, 0, 0, 0},
 {"Rebound",1,gfa_controlled_creature_has_instance, gf_change_player_instance,10, 0, 0, 10, 0, 0, 0, 0},
 {"Heal",1,   gfa_controlled_creature_has_instance, gf_change_player_instance,11, 0, 0, 11, 0, 0, 0, 0},
 {"Poison Cloud",1,gfa_controlled_creature_has_instance,gf_change_player_instance,12,0,0,12,0, 0, 0, 0},
 {"Invisibility",1,gfa_controlled_creature_has_instance,gf_change_player_instance,13,0,0,13,0, 0, 0, 0},
 {"Teleport",1,gfa_controlled_creature_has_instance,gf_change_player_instance,14, 0, 0, 14, 0, 0, 0, 0},
 {"Speed", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,15, 0, 0, 15, 0, 0, 0, 0},
 {"Slow",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,16, 0, 0, 16, 0, 0, 0, 0},
 {"Drain", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,17, 0, 0, 17, 0, 0, 0, 0},
 {"Fear",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,18, 0, 0, 18, 0, 0, 0, 0},
 {"Missile",1,gfa_controlled_creature_has_instance, gf_change_player_instance,19, 0, 0, 19, 0, 0, 0, 0},
 {"Homer", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,20, 0, 0, 20, 0, 0, 0, 0},
 {"Breath",1, gfa_controlled_creature_has_instance, gf_change_player_instance,21, 0, 0, 21, 0, 0, 0, 0},
 {"Wind",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,22, 0, 0, 22, 0, 0, 0, 0},
 {"Light", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,23, 0, 0, 23, 0, 0, 0, 0},
 {"Fly",   1, gfa_controlled_creature_has_instance, gf_change_player_instance,24, 0, 0, 24, 0, 0, 0, 0},
 {"Sight", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,25, 0, 0, 25, 0, 0, 0, 0},
 {"Grenade",1,gfa_controlled_creature_has_instance, gf_change_player_instance,26, 0, 0, 26, 0, 0, 0, 0},
 {"Hail",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,27, 0, 0, 27, 0, 0, 0, 0},
 {"WOP",   1, gfa_controlled_creature_has_instance, gf_change_player_instance,28, 0, 0, 28, 0, 0, 0, 0},
 {"Fart",  1, gfa_controlled_creature_has_instance, gf_change_player_instance,29, 0, 0, 29, 0, 0, 0, 0},
 {"Dig",   1, gfa_controlled_creature_has_instance, gf_change_player_instance,39, 0, 0, 39, 0, 0, 0, 0},
 {"Arrow", 1, gfa_controlled_creature_has_instance, gf_change_player_instance, 4, 0, 0,  4, 0, 0, 0, 0},
 {"Group", 1, gfa_controlled_creature_has_instance, gf_change_player_instance,40, 0, 0, 40, 0, 0, 0, 0},
 {"Disease",1,gfa_controlled_creature_has_instance, gf_change_player_instance,41, 0, 0, 41, 0, 0, 0, 0},
 {"Chicken",1,gfa_controlled_creature_has_instance, gf_change_player_instance,42, 0, 0, 42, 0, 0, 0, 0},
 {"!",     0,                          NULL,                             NULL, 0, 0, 0,  0, 0, 0, 0, 0},
};

// Boxes used for service/cheat menu
struct GuiBox *gui_box=NULL;
struct GuiBox *gui_cheat_box=NULL;

struct GuiBox *first_box=NULL;
struct GuiBox *last_box=NULL;
struct GuiBox gui_boxes[3];
//struct TbSprite *font_sprites=NULL;
//struct TbSprite *end_font_sprites=NULL;
//unsigned char *font_data=NULL;
struct DraggingBox dragging_box;

/******************************************************************************/
long gf_change_player_state(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  // Note: reworked from beta and unchecked
  struct PlayerInfo *player=get_my_player();
  set_players_packet_action(player, PckA_SetPlyrState, tag[0], tag[1], 0, 0);
  struct GuiBoxOption* guop = gbox->optn_list;
  while (guop->label[0] != '!')
  {
    guop->active = 0;
    guop++;
  }
  goptn->active = 1;
  return 1;
}

long gf_decide_victory(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
  //TODO PACKET we should use packets! This way is unacceptable!
  struct PlayerInfo* player = get_my_player();
  if (tag[0])
    set_player_as_won_level(player);
  else
    set_player_as_lost_level(player);
  return 1;
}

long gf_change_player_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_CtrlCrtrSetInstnc, *tag, 0, 0, 0);
    return 1;
}

long gf_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    if ((player->controlled_thing_idx <= 0) || (player->controlled_thing_idx >= THINGS_COUNT))
        return 0;
    set_players_packet_action(player, PckA_CheatCrtSpells, 0, 0, 0, 0);
    return 1;
}

long gf_research_rooms(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatAllRooms, 0, 0, 0, 0);
    return 1;
}

long gf_all_researchable(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatAllResrchbl, 0, 0, 0, 0);
    return 1;
}

long gf_make_everything_free(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatAllFree, 0, 0, 0, 0);
    return 1;
}

long gf_give_all_creatures_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatCrAllSpls, 0, 0, 0, 0);
    return 1;
}

long gf_explore_everywhere(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatRevealMap, 0, 0, 0, 0);
    return 1;
}

long gf_research_magic(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatAllMagic, 0, 0, 0, 0);
    return 1;
}

long gfa_can_give_controlled_creature_spells(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    if ((player->controlled_thing_idx <= 0) || (player->controlled_thing_idx >= THINGS_COUNT))
        return false;
    return true;
}

long gfa_controlled_creature_has_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    if ((player->controlled_thing_idx <= 0) || (player->controlled_thing_idx >= THINGS_COUNT))
        return false;
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    return creature_instance_is_available(thing, *tag);
}

void gui_draw_all_boxes(void)
{
  SYNCDBG(5,"Starting");
  lbDisplay.DrawFlags = Lb_TEXT_ONE_COLOR;
  LbTextSetFont(font_sprites);
  struct GuiBox* gbox = gui_get_lowest_priority_box();
  while (gbox != NULL)
  {
    gui_draw_box(gbox);
    gbox = gui_get_next_highest_priority_box(gbox);
  }
}

short gui_box_is_not_valid(struct GuiBox *gbox)
{
  if (gbox == NULL) return true;
  return (gbox->field_0 & 0x01) == 0;
}

void gui_insert_box_at_list_top(struct GuiBox *gbox)
{
  if (gbox->field_0 & 0x02)
  {
    ERRORLOG("GuiBox is already in list");
    return;
  }
  gbox->field_0 |= 0x02;
  gbox->next_box = first_box;
  if (first_box != NULL)
      first_box->prev_box = gbox;
  else
      last_box = gbox;
  first_box = gbox;
}

struct GuiBox *gui_allocate_box_structure(void)
{
    for (int i = 1; i < 3; i++)
    {
        struct GuiBox* gbox = &gui_boxes[i];
        if (gui_box_is_not_valid(gbox))
        {
            gbox->field_1 = i;
            gbox->field_0 |= 0x01;
            gui_insert_box_at_list_top(gbox);
            return gbox;
        }
  }
  return NULL;
}

long gui_calculate_box_width(struct GuiBox *gbox)
{
    int maxw = 0;
    struct GuiBoxOption* goptn = gbox->optn_list;
    while (goptn->label[0] != '!')
    {
        int w = pixel_size * LbTextStringWidth(goptn->label);
        if (w > maxw)
            maxw = w;
        goptn++;
  }
  return maxw+16;
}

long gui_calculate_box_height(struct GuiBox *gbox)
{
    int i = 0;
    struct GuiBoxOption* goptn = gbox->optn_list;
    while (goptn->label[0] != '!')
    {
        i++;
        goptn++;
  }
  return i*(pixel_size*LbTextLineHeight()+2) + 16;
}

void gui_remove_box_from_list(struct GuiBox *gbox)
{
  if ((gbox->field_0 & 0x02) == 0)
  {
    ERRORLOG("Cannot remove box from list when it is not in one!");
    return;
  }
  gbox->field_0 &= 0xFDu;
  if ( gbox->prev_box )
      gbox->prev_box->next_box = gbox->next_box;
  else
      first_box = gbox->next_box;
  if ( gbox->next_box )
      gbox->next_box->prev_box = gbox->prev_box;
  else
      last_box = gbox->prev_box;
  gbox->prev_box = 0;
  gbox->next_box = 0;
}

void gui_delete_box(struct GuiBox *gbox)
{
    gui_remove_box_from_list(gbox);
    LbMemorySet(gbox, 0, sizeof(struct GuiBox));
}

struct GuiBox *gui_create_box(long x, long y, struct GuiBoxOption *optn_list)
{
    struct GuiBox* gbox = gui_allocate_box_structure();
    if (gbox == NULL)
        return NULL;
    // Setting gui font - will be required to properly calculate box dimensions
    LbTextSetFont(font_sprites);
    gbox->optn_list = optn_list;
    gbox->pos_x = x;
    gbox->pos_y = y;
    gbox->width = gui_calculate_box_width(gbox);
    gbox->height = gui_calculate_box_height(gbox);
    return gbox;
}

short gui_move_box(struct GuiBox *gbox, long x, long y, unsigned short fdflags)
{
  short result;
  switch (fdflags)
  {
  case Fnt_LeftJustify:
      gbox->pos_x = x;
      gbox->pos_y = y;
      result = true;
      break;
  case Fnt_RightJustify:
      gbox->pos_x = x - gbox->width;
      gbox->pos_y = y - gbox->height;
      result = true;
      break;
  case Fnt_CenterPos:
      gbox->pos_x = x - (gbox->width >> 1);
      gbox->pos_y = y - (gbox->height >> 1);
      result = true;
      break;
  default:
      result = false;
      break;
  }
  if (gbox->pos_x+gbox->width > MyScreenWidth)
    gbox->pos_x = MyScreenWidth-gbox->width;
  if (gbox->pos_x < 0)
    gbox->pos_x = 0;
  if (gbox->pos_y+gbox->height > MyScreenHeight)
    gbox->pos_y = MyScreenHeight-gbox->height;
  if (gbox->pos_y < 0)
    gbox->pos_y = 0;
  return result;
}

/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * @return Gives true if the menu was toggled, false if cheat is not allowed.
 */
short toggle_main_cheat_menu(void)
{
  long mouse_x = GetMouseX();
  long mouse_y = GetMouseY();
  if ((gui_box==NULL) || (gui_box_is_not_valid(gui_box)))
  {
    if ((game.flags_font & FFlg_AlexCheat) == 0)
      return false;
    gui_box = gui_create_box(mouse_x,mouse_y,gui_main_cheat_list);
    gui_move_box(gui_box, mouse_x, mouse_y, Fnt_CenterPos);
  } else
  {
    gui_delete_box(gui_box);
    gui_box=NULL;
  }
  return true;
}

/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * @return Gives true if the menu was toggled, false if cheat is not allowed.
 */
short toggle_instance_cheat_menu(void)
{
    // Toggle cheat menu
    if ((gui_box==NULL) || (gui_box_is_not_valid(gui_box)))
    {
        if ((game.flags_font & FFlg_AlexCheat) == 0)
            return false;
        gui_box = gui_create_box(200,20,gui_instance_option_list);
/*
          player->unknownbyte  |= 0x08;
          game.unknownbyte |= 0x08;
*/
    } else
    {
        gui_delete_box(gui_box);
        gui_box=NULL;
/*
          player->unknownbyte &= 0xF7;
          game.unknownbyte &= 0xF7;
*/
    }
    return true;
}

/**
 * Opens cheat menu. It should not allow cheats in Network mode.
 * Returns true if the menu was toggled, false if cheat is not allowed.
 */
TbBool open_creature_cheat_menu(void)
{
  if ((game.flags_font & FFlg_AlexCheat) == 0)
    return false;
  if (!gui_box_is_not_valid(gui_cheat_box))
    return false;
  gui_cheat_box = gui_create_box(150,20,gui_creature_cheat_option_list);
  return (!gui_box_is_not_valid(gui_cheat_box));
}

/**
 * Closes cheat menu.
 * Returns true if the menu was closed.
 */
TbBool close_creature_cheat_menu(void)
{
  if (gui_box_is_not_valid(gui_cheat_box))
    return false;
  gui_delete_box(gui_cheat_box);
  gui_cheat_box = NULL;
  return true;
}

/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * Returns true if the menu was toggled, false if cheat is not allowed.
 */
TbBool toggle_creature_cheat_menu(void)
{
  // Cheat sub-menus
  if (gui_box_is_not_valid(gui_cheat_box))
  {
    return open_creature_cheat_menu();
  } else
  {
    return close_creature_cheat_menu();
  }
}


struct GuiBox *gui_get_highest_priority_box(void)
{
  return first_box;
}

struct GuiBox *gui_get_lowest_priority_box(void)
{
  return last_box;
}

struct GuiBox *gui_get_next_highest_priority_box(struct GuiBox *gbox)
{
  return gbox->prev_box;
}

struct GuiBox *gui_get_next_lowest_priority_box(struct GuiBox *gbox)
{
  return gbox->next_box;
}

struct GuiBox *gui_get_box_point_over(long x, long y)
{
    struct GuiBox* gbox = gui_get_highest_priority_box();
    while (gbox != NULL)
    {
        if ((y >= gbox->pos_y) && (y < gbox->pos_y + gbox->height))
            if ((x >= gbox->pos_x) && (x < gbox->pos_x + gbox->width))
                return gbox;
        gbox = gui_get_next_lowest_priority_box(gbox);
  }
  return NULL;
}

/**
 * Returns box option under given position.
 * Requires text font to be set properly before running.
 */
struct GuiBoxOption *gui_get_box_option_point_over(struct GuiBox *gbox, long x, long y)
{
    long sx = gbox->pos_x + 8;
    long sy = gbox->pos_y + 8;
    struct GuiBoxOption* gboptn = gbox->optn_list;
    long lnheight = LbTextLineHeight() * ((long)pixel_size) + 2;
    while (gboptn->label[0] != '!')
    {
        long height = LbTextStringHeight(gboptn->label) * ((long)pixel_size);
        if ((y >= sy) && (y < sy + height))
        {
            long width = LbTextStringWidth(gboptn->label) * ((long)pixel_size);
            if ((x >= sx) && (x < sx + width))
            {
                if ((gboptn->numfield_4 == 2) || (gboptn->field_26 == 0))
                    return NULL;
                return gboptn;
            }
        }
        gboptn++;
        sy += lnheight;
  }
  return NULL;
}

struct GuiBoxOption *gui_move_active_box_option(struct GuiBox *gbox, int val)
{
  if (gbox == NULL)
    return NULL;
  int opt_num = -1;
  int opt_total = 0;
  struct GuiBoxOption* goptn = gbox->optn_list;
  while (goptn->label[0] != '!')
  {
    if (goptn->active)
    {
      opt_num = opt_total;
//      goptn->active = 0;
//TODO GUI: deactivate option
    }
    goptn++;
    opt_total++;
  }
  opt_num += val;
  if ((opt_num >= 0) && (opt_num < opt_total))
  {
    goptn = &gbox->optn_list[opt_num];
    if (goptn->callback != NULL)
      goptn->callback(gbox, goptn, 1, &goptn->field_19);
//TODO GUI: activate option
    return goptn;
  }
  return NULL;
}

void gui_draw_box(struct GuiBox *gbox)
{
    SYNCDBG(6,"Drawing box, first optn \"%s\"",gbox->optn_list->label);
    struct GuiBoxOption *goptn;
    LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    long mouse_x = GetMouseX();
    long mouse_y = GetMouseY();
    struct GuiBoxOption* goptn_over = NULL;
    struct GuiBox* gbox_over = gui_get_box_point_over(mouse_x, mouse_y);
    if (gbox_over != NULL)
    {
      goptn_over = gui_get_box_option_point_over(gbox_over, mouse_x, mouse_y);
    }

    LbTextSetFont(font_sprites);
    long lnheight = pixel_size * LbTextLineHeight() + 2;
    long pos_y = gbox->pos_y + 8;
    long pos_x = gbox->pos_x + 8;
    if (gbox != gui_get_highest_priority_box())
    {
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[6][0][0]);
        if (lbDisplay.DrawFlags & Lb_SPRITE_OUTLINE)
        {
          LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[0][0][0]);
        } else
        {
          lbDisplay.DrawFlags ^= Lb_SPRITE_OUTLINE;
          LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[0][0][0]);
          lbDisplay.DrawFlags ^= Lb_SPRITE_OUTLINE;
        }
        lbDisplay.DrawFlags ^= Lb_SPRITE_TRANSPAR4;
        lbDisplay.DrawColour = colours[3][3][3];
        goptn = gbox->optn_list;
        while (goptn->label[0] != '!')
        {
          if (goptn->active_cb != NULL)
            goptn->field_26 = (goptn->active_cb)(gbox, goptn, &goptn->field_D);
          else
            goptn->field_26 = 1;
          if (!goptn->field_26)
            lbDisplay.DrawColour = colours[0][0][0];
          else
            lbDisplay.DrawColour = colours[3][3][3];
          if (LbScreenIsLocked())
          {
            LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, goptn->label);
          }
          goptn++;
          pos_y += lnheight;
        }
    } else
    {
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[12][0][0]);
        if (lbDisplay.DrawFlags & Lb_SPRITE_OUTLINE)
        {
            LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[2][0][0]);
        } else
        {
            lbDisplay.DrawFlags ^= Lb_SPRITE_OUTLINE;
            LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[2][0][0]);
            lbDisplay.DrawFlags ^= Lb_SPRITE_OUTLINE;
        }
        lbDisplay.DrawFlags ^= Lb_SPRITE_TRANSPAR4;
        goptn = gbox->optn_list;
        while (goptn->label[0] != '!')
        {
            if (goptn->active_cb != NULL)
              goptn->field_26 = (goptn->active_cb)(gbox, goptn, &goptn->field_D);
            else
              goptn->field_26 = 1;
            if (!goptn->field_26)
              lbDisplay.DrawColour = colours[0][0][0];
            else
            if ( ((gbox == gbox_over) && (goptn == goptn_over) && (gbox != dragging_box.gbox)) ||
                 ((gbox != NULL) && (goptn->active != 0)) )
              lbDisplay.DrawColour = colours[15][15][15];
            else
              lbDisplay.DrawColour = colours[9][9][9];
            if (LbScreenIsLocked())
            {
              LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, goptn->label);
            }
            goptn++;
            pos_y += lnheight;
        }
    }
}

TbBool gui_process_option_inputs(struct GuiBox *gbox, struct GuiBoxOption *goptn)
{
  if (left_button_released || right_button_released)
  {
      short button_num;
      if (left_button_released)
      {
          left_button_released = 0;
          button_num = 1;
    } else
    {
      right_button_released = 0;
      button_num = 2;
    }
    if (goptn->numfield_4 == 1)
    {
      if (goptn->callback != NULL)
        goptn->callback(gbox, goptn, button_num, &goptn->field_19);
    }
    return true;
  }
  return false;
}

/**
 * Processes GUI Boxes inputs.
 * @return Returns true if the input event was captured by a GUI Box.
 */
short gui_process_inputs(void)
{
    struct GuiBox *gbox;
    SYNCDBG(8,"Starting");
    long mouse_x = GetMouseX();
    long mouse_y = GetMouseY();
    short result = false;
    struct GuiBox* hpbox = gui_get_highest_priority_box();
    struct GuiBoxOption* goptn = NULL;
    if (dragging_box.gbox != NULL)
    {
      if (left_button_held)
      {
        if (hpbox != dragging_box.gbox)
        {
          gui_remove_box_from_list(dragging_box.gbox);
          gui_insert_box_at_list_top(dragging_box.gbox);
        }
        dragging_box.gbox->pos_x = mouse_x - dragging_box.start_x;
        dragging_box.gbox->pos_y = mouse_y - dragging_box.start_y;
      } else
      {
        dragging_box.gbox = NULL;
        left_button_released = 0;
      }
      result = true;
    } else
    if (left_button_clicked)
    {
      LbTextSetFont(font_sprites);
      gbox = gui_get_box_point_over(left_button_clicked_x, left_button_clicked_y);
      if (gbox != NULL)
      {
        goptn = gui_get_box_option_point_over(gbox, left_button_clicked_x, left_button_clicked_y);
        if (gbox != hpbox)
        {
          gui_remove_box_from_list(gbox);
          gui_insert_box_at_list_top(gbox);
          left_button_clicked = 0;
        } else
        if (goptn == NULL)
        {
          dragging_box.gbox = hpbox;
          dragging_box.start_x = left_button_clicked_x - gbox->pos_x;
          dragging_box.start_y = left_button_clicked_y - gbox->pos_y;
          left_button_clicked = 0;
        }
        result = true;
      }
    } else
    if (left_button_released)
    {
      gbox = gui_get_box_point_over(left_button_clicked_x, left_button_clicked_y);
      if (gbox != NULL)
      {
        LbTextSetFont(font_sprites);
        goptn = gui_get_box_option_point_over(gbox, left_button_clicked_x, left_button_clicked_y);
        if ((gbox == hpbox) && (goptn != NULL))
        {
          gui_process_option_inputs(hpbox, goptn);
        }
        result = true;
      }
    }
    if (right_button_released)
    {
      gbox = gui_get_box_point_over(left_button_clicked_x, left_button_clicked_y);
      if (gbox != NULL)
      {
        LbTextSetFont(font_sprites);
        goptn = gui_get_box_option_point_over(gbox, left_button_clicked_x, left_button_clicked_y);
        if ((gbox == hpbox) && (goptn != NULL))
        {
          gui_process_option_inputs(hpbox, goptn);
        }
        result = true;
      }
    }
/* These are making incorrect mouse function in possesion - thus disabled
    if (hpbox != NULL)
    {
      if (is_key_pressed(KC_UP,KM_NONE))
      {
        goptn = gui_move_active_box_option(hpbox,-1);
        clear_key_pressed(KC_UP);
        result = true;
      } else
      if (is_key_pressed(KC_DOWN,KM_NONE))
      {
        goptn = gui_move_active_box_option(hpbox,1);
        clear_key_pressed(KC_DOWN);
        result = true;
      }
      if (is_key_pressed(KC_PGUP,KM_NONE))
      {
        goptn = gui_move_active_box_option(hpbox,-2);
        clear_key_pressed(KC_PGUP);
        result = true;
      }
      if (is_key_pressed(KC_PGDOWN,KM_NONE))
      {
        goptn = gui_move_active_box_option(hpbox,2);
        clear_key_pressed(KC_PGDOWN);
        result = true;
      }
    }
*/
    SYNCDBG(9,"Returning %s",result?"true":"false");
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
