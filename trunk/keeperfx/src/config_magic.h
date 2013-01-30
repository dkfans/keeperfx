/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_magic.h
 *     Header file for config_magic.c.
 * @par Purpose:
 *     Support of configuration files for magic spells.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGMAGIC_H
#define DK_CFGMAGIC_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define MAGIC_ITEMS_MAX        64
#define SPELL_MAX_LEVEL         8
#define MAGIC_OVERCHARGE_LEVELS (SPELL_MAX_LEVEL+1)
#define MAGIC_TYPES_COUNT      30
#define POWER_TYPES_COUNT      20

enum SpellKinds {
    SplK_None = 0,
    SplK_Fireball,
    SplK_FireBomb,
    SplK_Freeze,
    SplK_Armour,
    SplK_Lightning,
    SplK_Rebound,
    SplK_Heal,
    SplK_PoisonCloud,
    SplK_Invisibility,
    SplK_Teleport,//[10]
    SplK_Speed,
    SplK_Slow,
    SplK_Drain,
    SplK_Fear,
    SplK_Missile,//[15]
    SplK_NavigMissile,
    SplK_FlameBreath,
    SplK_Wind,
    SplK_Light,
    SplK_Fly,//[20]
    SplK_Sight,
    SplK_Grenade,
    SplK_Hailstorm,
    SplK_WordOfPower,//[24]
    SplK_CrazyGas,
    SplK_Disease,
    SplK_Chicken,
    SplK_TimeBomb,//[28]
};

enum CreatureSpellAffectedFlags {
    CSAfF_Slow         = 0x0001,
    CSAfF_Speed        = 0x0002,
    CSAfF_Armour       = 0x0004,
    CSAfF_Rebound      = 0x0008,
    CSAfF_Flying       = 0x0010,
    CSAfF_Invisibility = 0x0020,
    CSAfF_Sight        = 0x0040,
    CSAfF_Unkn0080     = 0x0080,
    CSAfF_Disease      = 0x0100,
    CSAfF_Chicken      = 0x0200,
    CSAfF_Unkn0400     = 0x0400,
    CSAfF_Unkn0800     = 0x0800,
    CSAfF_Unkn1000     = 0x1000,
    CSAfF_Unkn2000     = 0x2000,
    CSAfF_ExpLevelUp     = 0x4000,
    CSAfF_Freeze       = 0x8000,
};

enum PowerKinds {
    PwrK_None = 0,
    PwrK_HAND,
    PwrK_MKDIGGER,
    PwrK_OBEY,
    PwrK_SLAP,
    PwrK_SIGHT, // 5
    PwrK_CALL2ARMS,
    PwrK_CAVEIN,
    PwrK_HEALCRTR,
    PwrK_HOLDAUDNC,
    PwrK_LIGHTNING, // 10
    PwrK_SPEEDCRTR,
    PwrK_PROTECT,
    PwrK_CONCEAL,
    PwrK_DISEASE,
    PwrK_CHICKEN, // 15
    PwrK_DESTRWALLS,
    PwrK_TIMEBOMB,
    PwrK_POSSESS,
    PwrK_ARMAGEDDON,
};

/** Contains properties of a shot model, to be stored in ShotConfigStats.
 */
enum ShotModelFlags {
    /** Set if the shot can be slapped with hand of evil of owning player. */
    ShMF_Slappable  = 0x0001,
    ShMF_Navigable  = 0x0002,
};

enum PowerCanCastFlags {
    PwCast_None          = 0x00000000,
    /** Allow casting the spell on enemy creatures kept in custody. */
    PwCast_CustodyCrtrs  = 0x00000001,
    /** Allow casting the spell on owned creatures not captured by enemy. */
    PwCast_OwnedCrtrs    = 0x00000002,
    /** Allow casting the spell on creatures of allied players. */
    PwCast_AlliedCrtrs   = 0x00000004,
    /** Allow casting the spell on creatures of enemy players. */
    PwCast_EnemyCrtrs    = 0x00000008,

    /** Allow casting the spell on neutral walkable tiles - path, water, lava, rooms owned by neutrals. */
    PwCast_NeutrlGround  = 0x00000010,
    /** Allow casting the spell on owned ground - rooms floor and claimed ground. */
    PwCast_OwnedGround   = 0x00000020,
    /** Allow casting the spell on allied players ground - rooms floor and claimed ground. */
    PwCast_AlliedGround  = 0x00000040,
    /** Allow casting the spell on enemy players ground - rooms floor and claimed ground. */
    PwCast_EnemyGround   = 0x00000080,

    /** Allow casting the spell on neutral tall slabs - earth, wall, gold. */
    PwCast_NeutrlTall    = 0x00000100,
    /** Allow casting the spell on owned tall slabs - own fortified wall. */
    PwCast_OwnedTall     = 0x00000200,
    /** Allow casting the spell on tall slabs owned by allies - their fortified walls. */
    PwCast_AlliedTall    = 0x00000400,
    /** Allow casting the spell on tall slabs owned by enemies - their fortified walls. */
    PwCast_EnemyTall     = 0x00000800,

    /** Allow casting the spell on owned food things (chickens). */
    PwCast_OwnedFood     = 0x00010000,
    /** Allow casting the spell on owned and neutral gold things (piles,pots etc.). */
    PwCast_OwnedGold     = 0x00020000,
    /** Allow casting the spell on owned spell books. */
    PwCast_OwnedSpell    = 0x00040000,
    /** Allow casting the spell on owned deployed trap things. */
    PwCast_OwnedTraps    = 0x01000000,
    /** Allow casting the spell only after a small delay from previous cast. */
    PwCast_NeedsDelay    = 0x04000000,
    /** Allow casting the spell only on claimable/fortificable slabs (for ground - path or claimed, for tall - earth or fortified). */
    PwCast_Claimable     = 0x08000000,
    /** Allow casting the spell on un-revealed tiles. */
    PwCast_Unrevealed    = 0x10000000,
    /** Allow casting the spell on temporarily revealed tiles (with SOE spell). */
    PwCast_RevealedTemp  = 0x20000000,
    /** Allow casting if only one of map-related and thing-related conditions is met. */
    PwCast_ThingOrMap    = 0x40000000,
    /** There are no map-related conditions - allow casting the spell anywhere on revealed map. */
    PwCast_Anywhere      = 0x80000000,
};
#define PwCast_AllCrtrs (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs)
#define PwCast_AllThings (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs|PwCast_OwnedFood|PwCast_OwnedGold|PwCast_OwnedSpell|PwCast_OwnedTraps)
#define PwCast_AllGround (PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround|PwCast_EnemyGround)
#define PwCast_NotEnemyGround (PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround)
#define PwCast_AllTall (PwCast_NeutrlTall|PwCast_OwnedTall|PwCast_AlliedTall|PwCast_EnemyTall)

struct SpellConfigStats {
    char code_name[COMMAND_WORD_LEN];
};

struct ShotConfigStats {
    char code_name[COMMAND_WORD_LEN];
    unsigned long model_flags;
    struct ShotStats *old;
};

struct PowerConfigStats {
    char code_name[COMMAND_WORD_LEN];
};

struct MagicConfig {
    long spell_types_count;
    struct SpellConfigStats spell_cfgstats[MAGIC_ITEMS_MAX];
    long shot_types_count;
    struct ShotConfigStats shot_cfgstats[MAGIC_ITEMS_MAX];
    long power_types_count;
    struct PowerConfigStats power_cfgstats[MAGIC_ITEMS_MAX];
};

#pragma pack(1)

typedef unsigned char (*Expand_Check_Func)(void);

struct SpellConfig { // sizeof=4
  int duration;
};

struct ShotStats // sizeof = 101
{
  short numfield_0;
  short numfield_2;
  unsigned char field_4[2];
  unsigned char field_6;
  unsigned char field_7;
  unsigned char field_8;
  short field_9;
  short field_B;
  short field_D;
  unsigned char field_F;
  unsigned char field_10;
  unsigned char field_11;
  unsigned char field_12;
  unsigned char field_13;
  short health;
  short damage;
  unsigned char destroy_on_first_hit;
  short speed;
  short firing_sound;
  unsigned char firing_sound_variants;
  short shot_sound;
  short field_20;
  short field_22;
  unsigned char field_24;
  short cast_spell_kind;
  unsigned char health_drain;
  unsigned char field_28;
  unsigned char field_29;
  unsigned char field_2A;
  short field_2B;
  short field_2D;
  unsigned char field_2F;
  unsigned char field_30;
  short field_31;
  short field_33;
  unsigned char field_35;
  unsigned char field_36;
  short field_37;
  short field_39;
  unsigned char field_3B;
  short field_3C;
  short field_3E;
  unsigned char field_40;
  short field_41;
  short field_43;
  short field_45;
  unsigned char field_47;
  unsigned char field_48;
  unsigned char field_49;
  unsigned char field_4A;
  unsigned char field_4B;
  unsigned char field_4C;
  unsigned char cannot_make_target_unconscious;
  short experience_given_to_shooter;
  short field_50;
  unsigned char field_52;
  unsigned char field_53;
  unsigned char field_54[4];
  unsigned char field_58[8];
  unsigned char field_60[4];
  unsigned char field_64;
};

struct MagicStats {  // sizeof=0x4C
  long cost[MAGIC_OVERCHARGE_LEVELS];
  long time;
  long power[MAGIC_OVERCHARGE_LEVELS];
};

/**
 * Spell information structure.
 * Stores configuration of spells.
 * It no longer matches the similar struct from DK - fields were added at end.
 */
struct SpellInfo {
  /** Informs if the spell can be targeted on a thing. */
  unsigned char cast_at_thing;
  /** Shot model to be fired while casting. */
  unsigned char shot_model;
  /** Informs if caster is affected by the spell. */
  unsigned char caster_affected;
  /** Effect model created while casting. */
  unsigned char cast_effect_model;
  unsigned short field_4;
  /** If caster is affected by the spell, indicates sound sample to be played. */
  unsigned short caster_affect_sound;
  /** Range of area damage, if the spell causes area damage. */
  unsigned long area_range;
  /** Amount of area damage points inflicted, if the spell causes area damage. */
  long area_damage;
  /** Hit type used for area damage, controls which things are affected. */
  unsigned char area_hit_type;
  /** Strength of the blow which pushes creatures on explosion. */
  long area_blow;
};

struct SpellData {
      long field_0;
      long field_4;
      unsigned char flag_8;
      short field_9;
      short field_B;
      unsigned short name_stridx;
      unsigned short tooltip_stridx;
      short field_11;
      short field_13;
      Expand_Check_Func field_15;
      unsigned long can_cast_flags;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT struct ShotStats _DK_shot_stats[30];
#define shot_stats _DK_shot_stats
DLLIMPORT struct SpellInfo _DK_spell_info[];
//DLLIMPORT struct SpellData _DK_spell_data[POWER_TYPES_COUNT+1];
/******************************************************************************/
extern const char keeper_magic_file[];
extern struct NamedCommand spell_desc[];
extern struct NamedCommand shot_desc[];
extern struct NamedCommand power_desc[];
extern struct SpellData spell_data[];
extern struct SpellInfo spell_info[];
/******************************************************************************/
struct SpellInfo *get_magic_info(int mgc_idx);
TbBool magic_info_is_invalid(const struct SpellInfo *mgcinfo);
struct SpellData *get_power_data(int pwr_idx);
long get_power_description_strindex(int pwr_idx);
long get_power_name_strindex(int pwr_idx);
TbBool power_data_is_invalid(const struct SpellData *pwrdata);
TbBool spell_is_stupid(int sptype);
long get_power_index_for_work_state(long work_state);
struct SpellConfigStats *get_spell_model_stats(SpellKind spmodel);
struct ShotConfigStats *get_shot_model_stats(ThingModel tngmodel);
struct PowerConfigStats *get_power_model_stats(PowerKind pwmodel);
const char *spell_code_name(SpellKind spmodel);
const char *shot_code_name(ThingModel tngmodel);
const char *power_code_name(PowerKind pwmodel);
int power_model_id(const char * code_name);
/******************************************************************************/
TbBool load_magic_config(const char *conf_fname,unsigned short flags);
TbBool make_all_powers_cost_free(void);
TbBool make_all_powers_researchable(PlayerNumber plyr_idx);
TbBool set_power_available(PlayerNumber plyr_idx, PowerKind spl_idx, long resrch, long avail);
TbBool is_power_available(PlayerNumber plyr_idx, PowerKind spl_idx);
TbBool add_spell_to_player(PowerKind spl_idx, PlayerNumber plyr_idx);
void remove_spell_from_player(PowerKind spl_idx, PlayerNumber plyr_idx);
TbBool make_available_all_researchable_powers(PlayerNumber plyr_idx);
unsigned long spell_to_creature_affected_flag(SpellKind spkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
