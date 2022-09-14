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
long gf_change_creature_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
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
long gfa_single_player_mode(struct GuiBox* gbox, struct GuiBoxOption* goptn, long* tag);
long gf_all_doors(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);
long gf_all_traps(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag);

struct GuiBoxOption gui_main_cheat_list[] = { //gui_main_option_list in beta
  {"Null mode",                1,           NULL,              gf_change_player_state, 0, 0, 0,               PSt_None, 0, 0, 0, true},
  {"Place digger mode",        1,           NULL,              gf_change_player_state, 0, 0, 0,           PSt_MkDigger, 0, 0, 0, true},
  {"Place creature mode",      1,           NULL,              gf_change_player_state, 0, 0, 0,        PSt_MkBadCreatr, 0, 0, 0, true},
  {"Place hero mode",          1,           NULL,              gf_change_player_state, 0, 0, 0,       PSt_MkGoodCreatr, 4, 0, 0, true},
  {"Destroy walls mode",       1,           NULL,              gf_change_player_state, 0, 0, 0,   PSt_FreeDestroyWalls, 0, 0, 0, true},
  {"Disease mode",             1,           NULL,              gf_change_player_state, 0, 0, 0,    PSt_FreeCastDisease, 0, 0, 0, true},
  {"Peter mode",               1,           NULL,              gf_change_player_state, 0, 0, 0,    PSt_FreeTurnChicken, 0, 0, 0, true},
  {"Create gold mode",         1,           NULL,              gf_change_player_state, 0, 0, 0,          PSt_MkGoldPot, 0, 0, 0, true},
  {"Steal room mode",          1,           NULL,              gf_change_player_state, 0, 0, 0,          PSt_StealRoom, 0, 0, 0, true},
  {"Destroy room mode",        1,           NULL,              gf_change_player_state, 0, 0, 0,        PSt_DestroyRoom, 0, 0, 0, true},
  {"Steal slab mode",          1,           NULL,              gf_change_player_state, 0, 0, 0,          PSt_StealSlab, 0, 0, 0, true},
  {"Place terrain mode",       1,           NULL,              gf_change_player_state, 0, 0, 0,       PSt_PlaceTerrain, 0, 0, 0, true},
  {"",                         2,           NULL,                       NULL, 0, 0, 0,                        PSt_None, 0, 0, 0, false},
  {"Passenger control mode",   1,           NULL,              gf_change_player_state, 0, 0, 0,    PSt_FreeCtrlPassngr, 0, 0, 0, true},
  {"Direct control mode",      1,           NULL,              gf_change_player_state, 0, 0, 0,     PSt_FreeCtrlDirect, 0, 0, 0, true},
  {"Order creature mode",      1,           NULL,              gf_change_player_state, 0, 0, 0,        PSt_OrderCreatr, 0, 0, 0, true},
  {"Kill creature mode",       1,           NULL,              gf_change_player_state, 0, 0, 0,         PSt_KillCreatr, 0, 0, 0, true},
  {"Destroy thing mode",       1,           NULL,              gf_change_player_state, 0, 0, 0,       PSt_DestroyThing, 0, 0, 0, true},
  {"Turncoat mode",            1,           NULL,              gf_change_player_state, 0, 0, 0,      PSt_ConvertCreatr, 0, 0, 0, true},
  {"Level up mode",            1,           NULL,              gf_change_player_state, 0, 0, 0,    PSt_LevelCreatureUp, 0, 0, 0, true},
  {"Level down mode",          1,           NULL,              gf_change_player_state, 0, 0, 0,  PSt_LevelCreatureDown, 0, 0, 0, true},
  {"Query mode",               1,           NULL,              gf_change_player_state, 0, 0, 0,           PSt_QueryAll, 0, 0, 0, true},
  {"Make happy mode",          1,           NULL,              gf_change_player_state, 0, 0, 0,            PSt_MkHappy, 0, 0, 0, true},
  {"Make angry mode",          1,           NULL,              gf_change_player_state, 0, 0, 0,            PSt_MkAngry, 0, 0, 0, true},
  {"",                         2,           NULL,                        NULL, 0, 0, 0,                       PSt_None, 0, 0, 0, false},
  {"Kill player mode",         1,           NULL,              gf_change_player_state, 0, 0, 0,         PSt_KillPlayer, 0, 0, 0, true},
  {"Edit heart health",        1,  gfa_single_player_mode,     gf_change_player_state, 0, 0, 0,        PSt_HeartHealth, 0, 0, 0, true},
  {"!",                        0,           NULL,                        NULL, 0, 0, 0,                       PSt_None, 0, 0, 0, false},
};

struct GuiBoxOption gui_creature_cheat_option_list[] = {
 {"Everything is free",        1,           NULL,     gf_make_everything_free, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Give controlled creature spells",1,gfa_can_give_controlled_creature_spells,gf_give_controlled_creature_spells, 0, 0, 0, 0, 0, 0, 0, 0},
 {"Give all creatures spells", 1,           NULL,gf_give_all_creatures_spells, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Explore everywhere",        1,           NULL,       gf_explore_everywhere, 0, 0, 0,               0, 0, 0, 0, 0},
 {"All rooms and magic researchable",1,     NULL,         gf_all_researchable, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Research all magic",        1,           NULL,           gf_research_magic, 0, 0, 0,               0, 0, 0, 0, 0},
 {"Research all rooms",        1,           NULL,           gf_research_rooms, 0, 0, 0,               0, 0, 0, 0, 0},
 {"All doors manufacturable",  1,           NULL,           gf_all_doors,      0, 0, 0,               0, 0, 0, 0, 0},
 {"All traps manufacturable",  1,           NULL,           gf_all_traps,      0, 0, 0,               0, 0, 0, 0, 0},
 {"Win the level instantly",   1,           NULL,           gf_decide_victory, 0, 0, 0,               1, 0, 0, 0, 0},
 {"Lose the level instantly",  1,           NULL,           gf_decide_victory, 0, 0, 0,               0, 0, 0, 0, 0},
 {"!",                         0,           NULL,                        NULL, 0, 0, 0,               0, 0, 0, 0, 0},
};

struct GuiBoxOption gui_instance_option_list[] = {
 {"Fireball",1,NULL,gf_change_creature_instance, CrInst_FIREBALL, 0, 0,  CrInst_FIREBALL, 0, 0, 0, true},
 {"Meteor",1, NULL, gf_change_creature_instance, CrInst_FIRE_BOMB, 0, 0,  CrInst_FIRE_BOMB, 0, 0, 0, true},
 {"Freeze",1, NULL, gf_change_creature_instance, CrInst_FREEZE, 0, 0,  CrInst_FREEZE, 0, 0, 0, true},
 {"Armour",1, NULL, gf_change_creature_instance, CrInst_ARMOUR, 0, 0,  CrInst_ARMOUR, 0, 0, 0, true},
 {"Lightning",1,NULL,gf_change_creature_instance,CrInst_LIGHTNING, 0, 0,  CrInst_LIGHTNING, 0, 0, 0, true},
 {"Rebound",1,NULL, gf_change_creature_instance,CrInst_REBOUND, 0, 0, CrInst_REBOUND, 0, 0, 0, true},
 {"Heal",1,   NULL, gf_change_creature_instance,CrInst_HEAL, 0, 0, CrInst_HEAL, 0, 0, 0, true},
 {"Poison Cloud",1,NULL,gf_change_creature_instance,CrInst_POISON_CLOUD,0,0,CrInst_POISON_CLOUD,0, 0, 0, true},
 {"Invisibility",1,NULL,gf_change_creature_instance,CrInst_INVISIBILITY,0,0,CrInst_INVISIBILITY,0, 0, 0, true},
 {"Teleport",1,NULL,gf_change_creature_instance,CrInst_TELEPORT, 0, 0, CrInst_TELEPORT, 0, 0, 0, true},
 {"Speed", 1, NULL, gf_change_creature_instance,CrInst_SPEED, 0, 0, CrInst_SPEED, 0, 0, 0, true},
 {"Slow",  1, NULL, gf_change_creature_instance,CrInst_SLOW, 0, 0, CrInst_SLOW, 0, 0, 0, true},
 {"Drain", 1, NULL, gf_change_creature_instance,CrInst_DRAIN, 0, 0, CrInst_DRAIN, 0, 0, 0, true},
 {"Fear",  1, NULL, gf_change_creature_instance,CrInst_FEAR, 0, 0, CrInst_FEAR, 0, 0, 0, true},
 {"Missile",1,NULL, gf_change_creature_instance,CrInst_MISSILE, 0, 0, CrInst_MISSILE, 0, 0, 0, true},
 {"Homer", 1, NULL, gf_change_creature_instance,CrInst_NAVIGATING_MISSILE, 0, 0, CrInst_NAVIGATING_MISSILE, 0, 0, 0, true},
 {"Breath",1, NULL, gf_change_creature_instance,CrInst_FLAME_BREATH, 0, 0, CrInst_FLAME_BREATH, 0, 0, 0, true},
 {"Wind",  1, NULL, gf_change_creature_instance,CrInst_WIND, 0, 0, CrInst_WIND, 0, 0, 0, true},
 {"Light", 1, NULL, gf_change_creature_instance,CrInst_LIGHT, 0, 0, CrInst_LIGHT, 0, 0, 0, true},
 {"Fly",   1, NULL, gf_change_creature_instance,CrInst_FLY, 0, 0, CrInst_FLY, 0, 0, 0, true},
 {"Sight", 1, NULL, gf_change_creature_instance,CrInst_SIGHT, 0, 0, CrInst_SIGHT, 0, 0, 0, true},
 {"Grenade",1,NULL, gf_change_creature_instance,CrInst_GRENADE, 0, 0, CrInst_GRENADE, 0, 0, 0, true},
 {"Hail",  1, NULL, gf_change_creature_instance,CrInst_HAILSTORM, 0, 0, CrInst_HAILSTORM, 0, 0, 0, true},
 {"WOP",   1, NULL, gf_change_creature_instance,CrInst_WORD_OF_POWER, 0, 0, CrInst_WORD_OF_POWER, 0, 0, 0, true},
 {"Fart",  1, NULL, gf_change_creature_instance,CrInst_FART, 0, 0, CrInst_FART, 0, 0, 0, true},
 {"Dig",   1, NULL, gf_change_creature_instance,CrInst_FIRST_PERSON_DIG, 0, 0, CrInst_FIRST_PERSON_DIG, 0, 0, 0, true},
 {"Arrow", 1, NULL, gf_change_creature_instance, CrInst_FIRE_ARROW, 0, 0,  CrInst_FIRE_ARROW, 0, 0, 0, true},
 {"Lizard", 1, NULL, gf_change_creature_instance,CrInst_LIZARD, 0, 0, CrInst_LIZARD, 0, 0, 0, true},
 {"Disease",1,NULL, gf_change_creature_instance,CrInst_CAST_SPELL_DISEASE, 0, 0, CrInst_CAST_SPELL_DISEASE, 0, 0, 0, true},
 {"Chicken",1,NULL, gf_change_creature_instance,CrInst_CAST_SPELL_CHICKEN, 0, 0, CrInst_CAST_SPELL_CHICKEN, 0, 0, 0, true},
 {"Time Bomb",1,NULL, gf_change_creature_instance,CrInst_CAST_SPELL_TIME_BOMB, 0, 0, CrInst_CAST_SPELL_TIME_BOMB, 0, 0, 0, true},
 {"!",     0,                          NULL,                             NULL, 0, 0, 0,  0, 0, 0, 0, false},
};

// Boxes used for service/cheat menu
struct GuiBox *gui_cheat_box_1=NULL;
struct GuiBox *gui_cheat_box_2=NULL;
struct GuiBox *gui_cheat_box_3=NULL;

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
  struct PlayerInfo *player = get_my_player();
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

long gf_change_creature_instance(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    set_players_packet_action(player, PckA_CheatCtrlCrtrSetInstnc, *tag, 0, 0, 0);
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

long gf_all_doors(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatAllDoors, 0, 0, 0, 0);
    return 1;
}

long gf_all_traps(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *tag)
{
    struct PlayerInfo* player = get_my_player();
    //  if (player->cheat_mode == 0) return false; -- there's no cheat_mode flag yet
    set_players_packet_action(player, PckA_CheatAllTraps, 0, 0, 0, 0);
    return 1;
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
  return (gbox->flags & GBoxF_Allocated) == 0;
}

void gui_insert_box_at_list_top(struct GuiBox *gbox)
{
  if (gbox->flags & GBoxF_InList)
  {
    ERRORLOG("GuiBox is already in list");
    return;
  }
  gbox->flags |= GBoxF_InList;
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
            gbox->flags |= GBoxF_Allocated;
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
  if ((gbox->flags & GBoxF_InList) == 0)
  {
    ERRORLOG("Cannot remove box from list when it is not in one!");
    return;
  }
  gbox->flags &= 0xFDu;
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
    if (gbox == NULL)
    {
        ERRORLOG("Trying to move cheat box that does not exist");
        return false;
    }
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
  case Fnt_CenterLeftPos:
      gbox->pos_x = x - (gbox->width >> 2);
      gbox->pos_y = y - (gbox->height >> 2);
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
 * Closes cheat menu.
 * Returns true if the menu was closed.
 */
TbBool close_main_cheat_menu(void)
{
    if (gui_box_is_not_valid(gui_cheat_box_1))
        return false;
    gui_delete_box(gui_cheat_box_1);
    gui_cheat_box_1 = NULL;
    return true;
}

/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * @return Gives true if the menu was toggled, false if cheat is not allowed.
 */
short toggle_main_cheat_menu(void)
{
  long mouse_x = GetMouseX();
  long mouse_y = GetMouseY();
  if ((gui_cheat_box_1==NULL) || (gui_box_is_not_valid(gui_cheat_box_1)))
  {
    if ((game.flags_font & FFlg_AlexCheat) == 0)
      return false;
    gui_cheat_box_1 = gui_create_box(mouse_x,mouse_y,gui_main_cheat_list);
    gui_move_box(gui_cheat_box_1, mouse_x, mouse_y, Fnt_CenterLeftPos);
  } else
  {
    gui_delete_box(gui_cheat_box_1);
    gui_cheat_box_1=NULL;
  }
  return true;
}


/**
 * Closes cheat menu.
 * Returns true if the menu was closed.
 */
TbBool close_instance_cheat_menu(void)
{
    if (gui_box_is_not_valid(gui_cheat_box_3))
        return false;
    gui_delete_box(gui_cheat_box_3);
    gui_cheat_box_3 = NULL;
    return true;
}
/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * @return Gives true if the menu was toggled, false if cheat is not allowed.
 */
short toggle_instance_cheat_menu(void)
{
    long mouse_x = GetMouseX();
    long mouse_y = GetMouseY();
    if (gui_box_is_not_valid(gui_cheat_box_3))
    {
        if ((game.flags_font & FFlg_AlexCheat) == 0)
            return false;
       gui_cheat_box_3 = gui_create_box(200,20,gui_instance_option_list);
       if (gui_cheat_box_3 == NULL)
       {
           return false;
       }
       else
       {
           gui_move_box(gui_cheat_box_3, mouse_x, mouse_y, Fnt_CenterLeftPos);
       }
/*
          player->unknownbyte  |= 0x08;
          game.unknownbyte |= 0x08;
*/
    } else
    {
        gui_delete_box(gui_cheat_box_3);
        gui_cheat_box_3=NULL;
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
  long mouse_x = GetMouseX();
  long mouse_y = GetMouseY();
  if ((game.flags_font & FFlg_AlexCheat) == 0)
    return false;
  if (!gui_box_is_not_valid(gui_cheat_box_2))
    return false;
  gui_cheat_box_2 = gui_create_box(150,20,gui_creature_cheat_option_list);
  gui_move_box(gui_cheat_box_2, mouse_x, mouse_y, Fnt_CenterLeftPos);
  return (!gui_box_is_not_valid(gui_cheat_box_2));
}

/**
 * Closes cheat menu.
 * Returns true if the menu was closed.
 */
TbBool close_creature_cheat_menu(void)
{
  if (gui_box_is_not_valid(gui_cheat_box_2))
    return false;
  gui_delete_box(gui_cheat_box_2);
  gui_cheat_box_2 = NULL;
  return true;
}

/**
 * Toggles cheat menu. It should not allow cheats in Network mode.
 * Returns true if the menu was toggled, false if cheat is not allowed.
 */
TbBool toggle_creature_cheat_menu(void)
{
  // Cheat sub-menus
  if (gui_box_is_not_valid(gui_cheat_box_2))
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
                if ((gboptn->numfield_4 == 2) || (gboptn->enabled == 0))
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
      goptn->callback(gbox, goptn, 1, &goptn->cb_param1);
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
            goptn->enabled = (goptn->active_cb)(gbox, goptn, &goptn->acb_param1);
          else
            goptn->enabled = 1;
          if (!goptn->enabled)
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
              goptn->enabled = (goptn->active_cb)(gbox, goptn, &goptn->acb_param1);
            else
              goptn->enabled = 1;
            if (!goptn->enabled)
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
        goptn->callback(gbox, goptn, button_num, &goptn->cb_param1);
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

TbBool point_is_over_gui_box(ScreenCoord x, ScreenCoord y)
{
    struct GuiBox *gbox = gui_get_box_point_over(x, y);
    return (gbox != NULL);
}

long gfa_single_player_mode(struct GuiBox* gbox, struct GuiBoxOption* goptn, long* tag)
{
    return ((game.system_flags & GSF_NetworkActive) == 0);
}

TbBool cheat_menu_is_active()
{
    if (!gui_box_is_not_valid(gui_cheat_box_1))
    {
        if ((gui_cheat_box_1->flags & GBoxF_InList) != 0)
        {
            return true;
        }
    }
    if (!gui_box_is_not_valid(gui_cheat_box_2))
    {
        if ((gui_cheat_box_2->flags & GBoxF_InList) != 0)
        {
            return true;
        }
    }
    if (!gui_box_is_not_valid(gui_cheat_box_3))
    {
        if ((gui_cheat_box_3->flags & GBoxF_InList) != 0)
        {
            return true;
        }
    }
    return false;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
