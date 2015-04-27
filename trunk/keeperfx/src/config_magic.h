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
    CSAfF_PoisonCloud  = 0x0400,
    CSAfF_CalledToArms = 0x0800,
    CSAfF_MadKilling   = 0x1000,
    CSAfF_Unkn2000     = 0x2000,
    CSAfF_ExpLevelUp   = 0x4000,
    /** For creature which are normally flying, this informs that its grounded due to spells or its condition. */
    CSAfF_Grounded     = 0x8000,
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
    ShMF_Boulder    = 0x0004,
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
    /** Allow casting the spell on creatures which are unconscious or dying. */
    PwCast_NConscCrtrs   = 0x00000010,
    /** Allow casting the spell on creatures which are bound by state (dragged, being sacrificed, teleported etc.). */
    PwCast_BoundCrtrs    = 0x00000020,

    /** Allow casting the spell on neutral walkable tiles - path, water, lava. */
    PwCast_UnclmdGround  = 0x00000080,
    /** Allow casting the spell on neutral ground - rooms floor and neutral claimed ground. */
    PwCast_NeutrlGround  = 0x00000100,
    /** Allow casting the spell on owned ground - rooms floor and claimed ground. */
    PwCast_OwnedGround   = 0x00000200,
    /** Allow casting the spell on allied players ground - rooms floor and claimed ground. */
    PwCast_AlliedGround  = 0x00000400,
    /** Allow casting the spell on enemy players ground - rooms floor and claimed ground. */
    PwCast_EnemyGround   = 0x00000800,

    /** Allow casting the spell on neutral tall slabs - earth, wall, gold. */
    PwCast_NeutrlTall    = 0x00001000,
    /** Allow casting the spell on owned tall slabs - own fortified wall. */
    PwCast_OwnedTall     = 0x00002000,
    /** Allow casting the spell on tall slabs owned by allies - their fortified walls. */
    PwCast_AlliedTall    = 0x00004000,
    /** Allow casting the spell on tall slabs owned by enemies - their fortified walls. */
    PwCast_EnemyTall     = 0x00008000,

    /** Allow casting the spell on owned food things (chickens). */
    PwCast_OwnedFood     = 0x00020000,
    /** Allow casting the spell on neutral food things. */
    PwCast_NeutrlFood    = 0x00040000,
    /** Allow casting the spell on enemy food things. */
    PwCast_EnemyFood     = 0x00080000,
    /** Allow casting the spell on owned gold things (piles,pots etc.). */
    PwCast_OwnedGold     = 0x00100000,
    /** Allow casting the spell on neutral gold things. */
    PwCast_NeutrlGold    = 0x00200000,
    /** Allow casting the spell on enemy gold things. */
    PwCast_EnemyGold     = 0x00400000,
    /** Allow casting the spell on owned spell books. */
    PwCast_OwnedSpell    = 0x00800000,
    /** Allow casting the spell on owned deployed trap things. */
    PwCast_OwnedBoulders = 0x01000000,
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
#define PwCast_AllCrtrs (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs|PwCast_NConscCrtrs|PwCast_BoundCrtrs)
#define PwCast_AllFood (PwCast_OwnedFood|PwCast_NeutrlFood|PwCast_EnemyFood)
#define PwCast_AllGold (PwCast_OwnedGold|PwCast_NeutrlGold|PwCast_EnemyGold)
#define PwCast_AllThings (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs|PwCast_AllFood|PwCast_AllGold|PwCast_OwnedSpell|PwCast_OwnedBoulders)
#define PwCast_AllGround (PwCast_UnclmdGround|PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround|PwCast_EnemyGround)
#define PwCast_NotEnemyGround (PwCast_UnclmdGround|PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround)
#define PwCast_AllTall (PwCast_NeutrlTall|PwCast_OwnedTall|PwCast_AlliedTall|PwCast_EnemyTall)

enum PowerConfigFlags {
    PwCF_Instinctive  = 0x0001, /**< Set if the power doesn't need to be selected from menu to be activated. */
    PwCF_HasProgress  = 0x0002, /**< Set if the power has a progress bar when active. */
    PwCF_IsParent     = 0x0004, /**< Set if the power has children and is just an aggregate. */
};

struct SpellConfigStats {
    char code_name[COMMAND_WORD_LEN];
};

struct ShotConfigStats {
    char code_name[COMMAND_WORD_LEN];
    unsigned long model_flags;
    /** Health of a shot decreases by 1 on every turn, so it works also as lifespan. */
    HitPoints health;
    /** Range of area damage, if the spell causes area damage. */
    MapCoordDelta area_range;
    /** Amount of area damage points inflicted, if the spell causes area damage. */
    HitPoints area_damage;
    /** Hit type used for area damage, controls which things are affected. */
    ThingHitType area_hit_type;
    /** Strength of the blow which pushes creatures on explosion. */
    MapCoordDelta area_blow;
    /** Type of the damage inflicted by this shot. */
    DamageType damage_type;
    struct ShotStats *old;
};

typedef unsigned char (*Expand_Check_Func)(void);

struct PowerConfigStats {
    char code_name[COMMAND_WORD_LEN];
    ThingModel artifact_model;
    unsigned long can_cast_flags;
    unsigned long config_flags;
    Expand_Check_Func overcharge_check;
    long work_state;
    PowerKind parent_power;
    short bigsym_sprite_idx;
    short medsym_sprite_idx;
    unsigned short name_stridx;
    unsigned short tooltip_str_idx;
    short select_sample_idx;
    short pointer_sprite_idx;
    long panel_tab_idx;
};

struct SpecialConfigStats {
    char code_name[COMMAND_WORD_LEN];
    ThingModel artifact_model;
    TextStringId tooltip_str_idx;
};

struct MagicConfig {
    long spell_types_count;
    struct SpellConfigStats spell_cfgstats[MAGIC_ITEMS_MAX];
    long shot_types_count;
    struct ShotConfigStats shot_cfgstats[MAGIC_ITEMS_MAX];
    long power_types_count;
    struct PowerConfigStats power_cfgstats[MAGIC_ITEMS_MAX];
    long special_types_count;
    struct SpecialConfigStats special_cfgstats[MAGIC_ITEMS_MAX];
};

#pragma pack(1)

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
  short size_xy;
  short field_B;
  short field_D;
  unsigned char field_F;
  unsigned char field_10;
  unsigned char field_11;
  unsigned char field_12;
  unsigned char field_13;
  short health_UNUSED;
  short damage;
  unsigned char destroy_on_first_hit;
  short speed;
  short firing_sound;
  unsigned char firing_sound_variants;
  short shot_sound;
  short field_20;
  short hit_sound;
  unsigned char field_24;
  short cast_spell_kind;
  unsigned char health_drain;
  unsigned char cannot_hit_thing;
  unsigned char field_29;
  unsigned char push_on_hit;
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
  short area_range_UNUSED;
  short area_damage_UNUSED;
  short is_boulder;
  unsigned char field_47;
  unsigned char is_melee;
  unsigned char field_49;
  unsigned char area_hit_type_UNUSED;
  unsigned char group_with_shooter;
  unsigned char deals_magic_damage;
  unsigned char cannot_make_target_unconscious;
  short experience_given_to_shooter;
  short field_50;
  unsigned char field_52;
  unsigned char field_53;
  unsigned char field_54[4];
  unsigned char field_58[8];
  unsigned char field_60[4];
  unsigned char affected_by_wind;
};

struct MagicStats {  // sizeof=0x4C
  long cost[MAGIC_OVERCHARGE_LEVELS];
  long time;
  long strength[MAGIC_OVERCHARGE_LEVELS];
};

/**
 * Spell information structure.
 * Stores configuration of powers; to be replaced with PowerConfigStats.
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
  unsigned short cast_field_4;
  /** If caster is affected by the spell, indicates sound sample to be played. */
  unsigned short caster_affect_sound;
};

/**
 * Powers config structure.
 * Stores configuration of powers; to be replaced with PowerConfigStats.
 */
struct SpellData {
      long pcktype;
      long work_state;
      unsigned char has_progress;
      short bigsym_sprite_idx;
      short medsym_sprite_idx;
      unsigned short name_stridx;
      unsigned short tooltip_str_idx;
      short select_sample_idx;
      short pointer_sprite_idx;
      Expand_Check_Func overcharge_check;
      unsigned long can_cast_flags;
};

#pragma pack()
/******************************************************************************/
DLLIMPORT struct ShotStats _DK_shot_stats[30];
#define shot_stats _DK_shot_stats
DLLIMPORT struct SpellInfo _DK_spell_info[];
/******************************************************************************/
extern struct MagicConfig magic_conf;
extern const char keeper_magic_file[];
extern struct NamedCommand spell_desc[];
extern struct NamedCommand shot_desc[];
extern struct NamedCommand power_desc[];
extern struct SpellData spell_data[];
extern struct SpellInfo spell_info[];
/******************************************************************************/
struct SpellInfo *get_magic_info(int mgc_idx);
TbBool magic_info_is_invalid(const struct SpellInfo *mgcinfo);
struct SpellData *get_power_data(int pwkind);
TextStringId get_power_description_strindex(int pwkind);
TextStringId get_power_name_strindex(int pwkind);
TbBool power_is_instinctive(int pwkind);
long get_power_index_for_work_state(long work_state);
long get_special_description_strindex(int spckind);
struct SpellConfigStats *get_spell_model_stats(SpellKind spmodel);
struct ShotConfigStats *get_shot_model_stats(ThingModel tngmodel);
struct PowerConfigStats *get_power_model_stats(PowerKind pwmodel);
TbBool power_model_stats_invalid(const struct PowerConfigStats *powerst);
struct MagicStats *get_power_dynamic_stats(PowerKind pwkind);
struct SpecialConfigStats *get_special_model_stats(SpecialKind spckind);
const char *spell_code_name(SpellKind spmodel);
const char *shot_code_name(ThingModel tngmodel);
const char *power_code_name(PowerKind pwkind);
int power_model_id(const char * code_name);
/******************************************************************************/
TbBool load_magic_config(const char *conf_fname,unsigned short flags);
TbBool make_all_powers_cost_free(void);
TbBool make_all_powers_researchable(PlayerNumber plyr_idx);
TbBool set_power_available(PlayerNumber plyr_idx, PowerKind spl_idx, long resrch, long avail);
TbBool is_power_available(PlayerNumber plyr_idx, PowerKind spl_idx);
TbBool add_power_to_player(PowerKind spl_idx, PlayerNumber plyr_idx);
void remove_power_from_player(PowerKind spl_idx, PlayerNumber plyr_idx);
TbBool make_available_all_researchable_powers(PlayerNumber plyr_idx);
TbBool shot_can_collide_other_shots(ThingModel shotkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
