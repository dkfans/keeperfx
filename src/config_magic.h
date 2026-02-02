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
#include "creature_instances.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define MAGIC_ITEMS_MAX         2000
#define SPELL_MAX_LEVEL         9
#define POWER_MAX_LEVEL         8
#define MAGIC_OVERCHARGE_LEVELS (POWER_MAX_LEVEL+1)
#define POWER_TYPES_MAX         2000

enum CreatureSpellAffectedFlags {
    CSAfF_Slow         = 0x000001,
    CSAfF_Speed        = 0x000002,
    CSAfF_Armour       = 0x000004,
    CSAfF_Rebound      = 0x000008,
    CSAfF_Flying       = 0x000010,
    CSAfF_Invisibility = 0x000020,
    CSAfF_Sight        = 0x000040,
    CSAfF_Light        = 0x000080,
    CSAfF_Disease      = 0x000100,
    CSAfF_Chicken      = 0x000200,
    CSAfF_PoisonCloud  = 0x000400,
    CSAfF_Freeze       = 0x000800,
    CSAfF_MadKilling   = 0x001000,
    CSAfF_Fear         = 0x002000,
    CSAfF_Heal         = 0x004000,
    CSAfF_Teleport     = 0x008000,
    CSAfF_Timebomb     = 0x010000,
    CSAfF_Wind         = 0x020000,
};

enum SpellPropertiesFlags {
    SPF_FixedDamage    = 0x01, // Damage or healing does not increase based on the creature's level.
    SPF_PercentBased   = 0x02, // Damage or healing is based on a percentage of current health instead of a flat value.
    SPF_MaxHealth      = 0x04, // Damage or healing is based on a percentage of max health instead of a flat value.
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
    PwrK_PICKUPCRTR, // 20
    PwrK_PICKUPGOLD,
    PwrK_PICKUPFOOD,
    PwrK_REBOUND,
    PwrK_FREEZE,
    PwrK_SLOW, // 25
    PwrK_FLIGHT,
    PwrK_VISION,
    PwrK_MKTUNNELLER,
};

enum CostFormulas {
    Cost_Default = 0,
    Cost_Digger,
    Cost_Dwarf,
};

/** Contains properties of a shot model, to be stored in ShotConfigStats.
 */
enum ShotModelFlags {
    /** Set if the shot can be slapped with hand of evil of owning player. */
    ShMF_Slappable      = 0x0001,
    ShMF_Navigable      = 0x0002,
    ShMF_Boulder        = 0x0004,
    ShMF_ReboundImmune  = 0x0008,
    ShMF_Digging        = 0x0010,
    ShMF_LifeDrain      = 0x0020,
    ShMF_GroupUp        = 0x0040,
    ShMF_NoStun         = 0x0080,
    ShMF_NoHit          = 0x0100,
    ShMF_StrengthBased  = 0x0200,
    ShMF_AlarmsUnits    = 0x0400,
    ShMF_CanCollide     = 0x0800,
    ShMF_Disarming      = 0x1000,
    ShMF_Exploding      = 0x2000,
    ShMF_BlocksRebirth  = 0x4000,
    ShMF_Penetrating    = 0x8000,
    ShMF_NeverBlock     = 0x10000,
    ShMF_WallPierce     = 0x20000,
};

#define PwCast_None           (0LL)
    /** Allow casting the spell on enemy creatures kept in custody. */
#define PwCast_CustodyCrtrs   (1LL << 1)
    /** Allow casting the spell on owned creatures not captured by enemy. */
#define PwCast_OwnedCrtrs     (1LL << 2)
    /** Allow casting the spell on creatures of allied players. */
#define PwCast_AlliedCrtrs    (1LL << 3)
    /** Allow casting the spell on creatures of enemy players. */
#define PwCast_EnemyCrtrs     (1LL << 4)
    /** Allow casting the spell on creatures which are unconscious or dying. */
#define PwCast_NConscCrtrs    (1LL << 5)
    /** Allow casting the spell on creatures which are bound by state (dragged, being sacrificed, teleported etc.). */
#define PwCast_BoundCrtrs     (1LL << 6)

    /** Allow casting the spell on neutral walkable tiles - path, water, lava. */
#define PwCast_UnclmdGround   (1LL << 7)
    /** Allow casting the spell on neutral ground - rooms floor and neutral claimed ground. */
#define PwCast_NeutrlGround   (1LL << 8)
    /** Allow casting the spell on owned ground - rooms floor and claimed ground. */
#define PwCast_OwnedGround    (1LL << 9)
    /** Allow casting the spell on allied players ground - rooms floor and claimed ground. */
#define PwCast_AlliedGround   (1LL << 10)
    /** Allow casting the spell on enemy players ground - rooms floor and claimed ground. */
#define PwCast_EnemyGround    (1LL << 11)

    /** Allow casting the spell on neutral tall slabs - earth, wall, gold. */
#define PwCast_NeutrlTall     (1LL << 12)
    /** Allow casting the spell on owned tall slabs - own fortified wall. */
#define PwCast_OwnedTall      (1LL << 13)
    /** Allow casting the spell on tall slabs owned by allies - their fortified walls. */
#define PwCast_AlliedTall     (1LL << 14)
    /** Allow casting the spell on tall slabs owned by enemies - their fortified walls. */
#define PwCast_EnemyTall      (1LL << 15)

    /** Allow casting the spell on owned food things (chickens). */
#define PwCast_OwnedFood      (1LL << 16)
    /** Allow casting the spell on neutral food things. */
#define PwCast_NeutrlFood     (1LL << 17)
    /** Allow casting the spell on enemy food things. */
#define PwCast_EnemyFood      (1LL << 18)
    /** Allow casting the spell on owned gold things (piles,pots etc.). */
#define PwCast_OwnedGold      (1LL << 19)
    /** Allow casting the spell on neutral gold things. */
#define PwCast_NeutrlGold     (1LL << 20)
    /** Allow casting the spell on enemy gold things. */
#define PwCast_EnemyGold      (1LL << 21)
    /** Allow casting the spell on owned spell books. */
#define PwCast_OwnedSpell     (1LL << 22)
    /** Allow casting the spell on owned deployed trap things. */
#define PwCast_OwnedBoulders  (1LL << 23)
    /** Allow casting the spell only after a small delay from previous cast. */
#define PwCast_NeedsDelay     (1LL << 24)
    /** Allow casting the spell only on claimable/fortificable slabs (for ground - path or claimed, for tall - earth or fortified). */
#define PwCast_Claimable      (1LL << 25)
    /** Allow casting the spell on un-revealed tiles. */
#define PwCast_Unrevealed     (1LL << 26)
    /** Allow casting the spell on temporarily revealed tiles (with SOE spell). */
#define PwCast_RevealedTemp   (1LL << 27)
    /** Allow casting if only one of map-related and thing-related conditions is met. */
#define PwCast_ThingOrMap     (1LL << 28)
    /** There are no map-related conditions - allow casting the spell anywhere on revealed map. */
#define PwCast_Anywhere       (1LL << 29)
#define PwCast_DiggersOnly    (1LL << 30)
#define PwCast_DiggersNot     (1LL << 31)
#define PwCast_OwnedObjects   (1LL << 32)
#define PwCast_NeutrlObjects  (1LL << 33)
#define PwCast_EnemyObjects   (1LL << 34)

#define PwCast_AllCrtrs (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs|PwCast_NConscCrtrs|PwCast_BoundCrtrs)
#define PwCast_AllFood (PwCast_OwnedFood|PwCast_NeutrlFood|PwCast_EnemyFood)
#define PwCast_AllGold (PwCast_OwnedGold|PwCast_NeutrlGold|PwCast_EnemyGold)
#define PwCast_AllObjects (PwCast_OwnedObjects|PwCast_NeutrlObjects|PwCast_EnemyObjects)
#define PwCast_AllThings (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs|PwCast_AllFood|PwCast_AllGold|PwCast_OwnedSpell|PwCast_OwnedBoulders|PwCast_AllObjects)
#define PwCast_AllGround (PwCast_UnclmdGround|PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround|PwCast_EnemyGround)
#define PwCast_NotEnemyGround (PwCast_UnclmdGround|PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround)
#define PwCast_AllTall (PwCast_NeutrlTall|PwCast_OwnedTall|PwCast_AlliedTall|PwCast_EnemyTall)

enum PowerConfigFlags {
    PwCF_Instinctive  = 0x0001, /**< Set if the power doesn't need to be selected from menu to be activated. */
    PwCF_HasProgress  = 0x0002, /**< Set if the power has a progress bar when active. */
    PwCF_IsParent     = 0x0004, /**< Set if the power has children and is just an aggregate. */
};

enum OverchargeChecks {
    OcC_Null,
    OcC_General_expand,
    OcC_SightOfEvil_expand,
    OcC_CallToArms_expand,
    OcC_do_not_expand
};

/**
 * Configuration parameters for spells.
 */
struct SpellConfigStats {
    char code_name[COMMAND_WORD_LEN];
};

struct ShotHitConfig {
    ThingModel effect_model; /**< Effect kind to be created when the shot hits. */
    short sndsample_idx; /**< Base sound sample to be played on hit. */
    unsigned char sndsample_range; /**< Range for random sound sample selection. */
    unsigned char withstand; /**< Whether the shot can withstand a hit without getting destroyed; could be converted to flags. */
};

struct ShotDetonateConfig {
    EffectOrEffElModel effect1_model;
    EffectOrEffElModel effect2_model;
    short around_effect1_model;
    short around_effect2_model;
};

struct ShotVisualConfig {
    EffectOrEffElModel effect_model;
    unsigned char amount;
    short random_range;
    HitPoints shot_health;
};

/**
 * Configuration parameters for shots.
 */
struct ShotConfigStats {
    char code_name[COMMAND_WORD_LEN];
    uint32_t model_flags;
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
    short damage;
    short speed;
    TbBool is_magical;
    struct ShotHitConfig hit_generic;
    struct ShotHitConfig hit_door;
    struct ShotHitConfig hit_water;
    struct ShotHitConfig hit_lava;
    struct ShotHitConfig hit_creature;
    struct ShotHitConfig dig;
    struct ShotHitConfig hit_heart;
    struct ShotDetonateConfig explode;
    struct ShotVisualConfig visual;
    short firing_sound;
    short shot_sound;
    short sound_priority;
    unsigned char firing_sound_variants;
    short max_range;
    unsigned short sprite_anim_idx;
    unsigned short sprite_size_max;
    short size_xy;
    short size_z;
    unsigned char fall_acceleration;
    unsigned char cast_spell_kind;
    char push_on_hit;
    unsigned char hidden_projectile;
    unsigned char destroy_on_first_hit;
    short experience_given_to_shooter;
    short inertia_floor;
    short inertia_air;
    short bounce_angle;
    short wind_immune;
    short no_air_damage;
    unsigned char target_hitstop_turns;
    short animation_transparency;
    short fixed_damage;
    short light_radius;
    unsigned char light_intensity;
    unsigned char light_flags;
    unsigned char unshaded;
    unsigned char soft_landing;
    EffectOrEffElModel effect_id;
    EffectOrEffElModel effect_bleeding;
    EffectOrEffElModel effect_frozen;
    unsigned char fire_logic; // see enum ShotFireLogics
    short update_logic; // see enum ShotUpdateLogics
    unsigned short effect_spacing;
    unsigned char effect_amount;
    unsigned short periodical;
    short spread_xy;
    short spread_z;
    short speed_deviation;
};

typedef unsigned char (*Expand_Check_Func)(void);

/**
 * Configuration parameters for powers.
 */
struct PowerConfigStats {
    char code_name[COMMAND_WORD_LEN];
    ThingModel artifact_model;
    uint64_t can_cast_flags;
    uint32_t config_flags;
    unsigned char overcharge_check_idx;
    uint32_t work_state;
    PowerKind parent_power;
    /** Sprite index of big symbol icon representing the power. */
    short bigsym_sprite_idx;
    /** Sprite index of medium symbol icon representing the power. */
    short medsym_sprite_idx;
    unsigned short name_stridx;
    unsigned short tooltip_stridx;
    short select_sample_idx;
    short pointer_sprite_idx;
    uint32_t panel_tab_idx;
    unsigned short select_sound_idx;
    short cast_cooldown;
    unsigned char cost_formula;
    SpellKind spell_idx;
    EffectOrEffElModel effect_id;
    FuncIdx magic_use_func_idx;
    ThingModel creature_model;
    GoldAmount cost[MAGIC_OVERCHARGE_LEVELS];
    GameTurnDelta duration;
    int32_t strength[MAGIC_OVERCHARGE_LEVELS+1];
};

/**
 * Configuration parameters for specials.
 */
struct SpecialConfigStats {
    char code_name[COMMAND_WORD_LEN];
    ThingModel artifact_model;
    TextStringId tooltip_stridx;
    short speech;
    short effect_id;
    short value;
};

 /**
 * Spell information structure.
 * Stores configuration of spells; to be replaced with SpellConfigStats when all fields are in CFG.
 * It no longer matches the similar struct from DK - fields were added at end.
 */
struct SpellConfig {
    /** Informs if the spell can be targeted on a thing. */
    unsigned char cast_at_thing;
    /** Shot model to be fired while casting. */
    ThingModel shot_model;
    /** Informs if caster is affected by the spell. */
    unsigned char caster_affected;
    /** Effect model created while casting. */
    EffectOrEffElModel cast_effect_model;
    /** If caster is affected by the spell, indicates sound sample to be played. */
    unsigned short caster_affect_sound;
    /** Sprite index of big symbol icon representing the spell. */
    short bigsym_sprite_idx;
    /** Sprite index of medium symbol icon representing the spell. */
    short medsym_sprite_idx;
    short cast_sound;
    unsigned char caster_sounds_count;
    ThingModel crtr_summon_model;
    short crtr_summon_level;
    short crtr_summon_amount;
    PowerKind linked_power;
    GameTurnDelta countdown;
    GameTurnDelta duration;
    EffectOrEffElModel aura_effect;
    GameTurnDelta aura_duration;
    GameTurnDelta aura_frequency;
    HitPoints healing_recovery;
    HitPoints damage;
    GameTurnDelta damage_frequency;
    uint32_t spell_flags;
    uint32_t cleanse_flags;
    unsigned char properties_flags;
};

struct MagicConfig {
    int32_t spell_types_count;
    struct SpellConfig spell_config[MAGIC_ITEMS_MAX];// should get merged into SpellConfigStats
    struct SpellConfigStats spell_cfgstats[MAGIC_ITEMS_MAX];
    int32_t shot_types_count;
    struct ShotConfigStats shot_cfgstats[MAGIC_ITEMS_MAX];
    int32_t power_types_count;
    struct PowerConfigStats power_cfgstats[MAGIC_ITEMS_MAX];
    int32_t special_types_count;
    struct SpecialConfigStats special_cfgstats[MAGIC_ITEMS_MAX];
    struct InstanceInfo instance_info[INSTANCE_TYPES_MAX]; //count in crtr_conf
};

/******************************************************************************/
extern const struct ConfigFileData keeper_magic_file_data;
extern struct NamedCommand spell_desc[];
extern struct NamedCommand shot_desc[];
extern struct NamedCommand power_desc[];
extern struct SpellConfig spell_config[];
extern const struct NamedCommand spell_effect_flags[];
extern const struct NamedCommand powermodel_properties_commands[];
extern const struct LongNamedCommand powermodel_castability_commands[];
extern const struct NamedCommand powermodel_expand_check_func_type[];
extern const struct NamedCommand magic_power_commands[];
extern const Expand_Check_Func powermodel_expand_check_func_list[];
extern const struct NamedCommand magic_use_func_commands[];
extern const struct NamedCommand magic_cost_formula_commands[];
/******************************************************************************/
struct SpellConfig *get_spell_config(SpellKind spell_idx);
TbBool spell_config_is_invalid(struct SpellConfig *mgcinfo);
TextStringId get_power_description_strindex(PowerKind pwkind);
TextStringId get_power_name_strindex(PowerKind pwkind);
TbBool power_is_instinctive(int pwkind);
int32_t get_power_index_for_work_state(int32_t work_state);
int32_t get_special_description_strindex(int spckind);
struct SpellConfigStats *get_spell_model_stats(SpellKind spmodel);
struct ShotConfigStats *get_shot_model_stats(ThingModel tngmodel);
struct PowerConfigStats *get_power_model_stats(PowerKind pwmodel);
TbBool power_model_stats_invalid(const struct PowerConfigStats *powerst);
struct SpecialConfigStats *get_special_model_stats(SpecialKind spckind);
const char *spell_code_name(SpellKind spmodel);
const char *shot_code_name(ThingModel tngmodel);
const char *power_code_name(PowerKind pwkind);
int power_model_id(const char * code_name);
/******************************************************************************/
TbBool make_all_powers_cost_free(void);
TbBool make_all_powers_researchable(PlayerNumber plyr_idx);
TbBool set_power_available(PlayerNumber plyr_idx, PowerKind spl_idx, long resrch, long avail);
TbBool is_power_available(PlayerNumber plyr_idx, PowerKind spl_idx);
TbBool is_power_obtainable(PlayerNumber plyr_idx, PowerKind pwkind);
TbBool add_power_to_player(PowerKind spl_idx, PlayerNumber plyr_idx);
void remove_power_from_player(PowerKind spl_idx, PlayerNumber plyr_idx);
TbBool make_available_all_researchable_powers(PlayerNumber plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
