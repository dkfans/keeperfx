/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_effects.h
 *     Header file for thing_effects.c.
 * @par Purpose:
 *     Effect generators and effect elements support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     01 Jan 2010 - 12 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TNGEFFECT_H
#define DK_TNGEFFECT_H

#include "globals.h"
#include "bflib_basics.h"

#include "light_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
enum ThingHitTypes {
    THit_None = 0,
    THit_CrtrsNObjcts, // Affect all creatures and all objects
    THit_CrtrsOnly, // Affect only creatures
    THit_CrtrsNObjctsNotOwn, // Affect not own creatures and objects
    THit_CrtrsOnlyNotOwn, // Affect not own creatures
    THit_CrtrsNotArmourNotOwn, // Affect not own creatures which are not protected by Armour spell
    THit_All, // Affect all things
    THit_HeartOnly, // Affect only dungeon hearts
    THit_HeartOnlyNotOwn, // Affect only not own dungeon hearts
    THit_CrtrsNObjctsNShot, // Affect all creatures and all objects, also allow colliding with other shots
    THit_TypesCount, // Last item in enumeration, allows checking amount of valid types
};

enum AreaAffectTypes {
    AAffT_None = 0,
    AAffT_GasDamage,
    AAffT_Unkn2,
    AAffT_GasSlow,
    AAffT_WOPDamage,
    AAffT_GasSlowDamage,
    AAffT_GasDisease,
};

enum ThingEffectKind {
    TngEff_None = 0,
    TngEff_Explosion1, // expl small
    TngEff_Explosion2,
    TngEff_Explosion3,
    TngEff_Explosion4,
    TngEff_Explosion5, // expl big
    TngEff_HitBleedingUnit,
    TngEff_ChickenBlood,
    TngEff_Blood3,
    TngEff_Blood4,
    TngEff_Blood5, // blood big
    TngEff_Gas1, // fart
    TngEff_Gas2, // fart
    TngEff_Gas3, // fart
    TngEff_WoPExplosion,
    TngEff_IceShard, // Ice shard
    TngEff_HarmlessGas1, // fart without damage
    TngEff_HarmlessGas2, // fart without damage
    TngEff_HarmlessGas3, // fart without damage
    TngEff_Drip1, // water drip
    TngEff_Drip2,
    TngEff_Drip3,
    TngEff_HitFrozenUnit,
    TngEff_Hail,
    TngEff_DeathIceExplosion,
    TngEff_RockChips, // less dirt
    TngEff_DirtRubble,
    TngEff_Unknown27, // more dirt
    TngEff_ImpSpangleRed,
    TngEff_Drip4, // ice drip?
    TngEff_Cloud, // super long cloud?
    TngEff_HarmlessGas4, // super wide cloud?
    TngEff_GoldRubble1, // small gold coins
    TngEff_GoldRubble2,
    TngEff_GoldRubble3, // big gold chunks
    TngEff_TempleSplash,
    TngEff_CeilingBreach,
    TngEff_StrangeGas1, // strange gas
    TngEff_StrangeGas2, // strange gas
    TngEff_StrangeGas3, // strange gas
    TngEff_DecelerationGas1, // fart slow
    TngEff_DecelerationGas2,
    TngEff_DecelerationGas3,
    TngEff_Eruption,
    TngEff_HearthCollapse,
    TngEff_Explosion6, // claim with sound???
    TngEff_SpangleRedBig,
    TngEff_ColouredRingOfFire, // spiral fx
    TngEff_Flash, // flash with whiteout
    TngEff_DamageBlood,
    TngEff_Explosion7, // temple? explosion with sound
    TngEff_FeatherPuff,
    TngEff_Explosion8,
    TngEff_ResearchComplete,
    TngEff_RoomSparkeSmall,
    TngEff_RoomSparkeMedium,
    TngEff_RoomSparkeLarge,
    TngEff_ImpSpangleBlue,
    TngEff_ImpSpangleGreen,
    TngEff_ImpSpangleYellow,
    TngEff_BallPuffRed, // teleport puff red
    TngEff_BallPuffBlue,
    TngEff_BallPuffGreen,
    TngEff_BallPuffYellow, // teleport puff yellow
    TngEff_BallPuffWhite, // teleport puff white
    TngEff_BloodyFootstep,
    TngEff_Blood7, // blood splat
    TngEff_SpecialBox,
    TngEff_BoulderSink, // boulder sink
    TngEff_Unknown69,
};

enum ThingEffectElements {
    TngEffElm_None = 0,
    TngEffElm_Blast1,
    TngEffElm_Blood1,
    TngEffElm_Blood2,
    TngEffElm_Blood3,
    TngEffElm_Unknown05,
    TngEffElm_SpikedBall,
    TngEffElm_Cloud1,
    TngEffElm_SmallSparkles,
    TngEffElm_BallOfLight,
    TngEffElm_RedFlameBig, // 10
    TngEffElm_IceShard,
    TngEffElm_Leaves1,
    TngEffElm_Thingy2,
    TngEffElm_Thingy3,
    TngEffElm_TinyFlash1,
    TngEffElm_FlashBall1,
    TngEffElm_RedFlash,
    TngEffElm_FlashBall2,
    TngEffElm_TinyFlash2,
    TngEffElm_PurpleStars, // 20
    TngEffElm_Cloud2,
    TngEffElm_Drip1,
    TngEffElm_Blood4,
    TngEffElm_IceMelt1,
    TngEffElm_IceMelt2,
    TngEffElm_TinyRock,
    TngEffElm_MedRock,
    TngEffElm_LargeRock1,
    TngEffElm_Drip2,
    TngEffElm_LavaFlameStationary, // 30
    TngEffElm_Unknown31,
    TngEffElm_LavaFlameMoving,
    TngEffElm_LargeRock2,
    TngEffElm_Unknown34,
    TngEffElm_Unknown35,
    TngEffElm_Unknown36,
    TngEffElm_EntranceMist,
    TngEffElm_Splash,
    TngEffElm_Blast2,
    TngEffElm_Drip3, // 40
    TngEffElm_Price,
    TngEffElm_ElectricBall1,
    TngEffElm_RedTwinkle,
    TngEffElm_RedTwinkle2,
    TngEffElm_Heal,
    TngEffElm_Unknown46,
    TngEffElm_Cloud3,
    TngEffElm_LargeRock3,
    TngEffElm_Gold1,
    TngEffElm_Gold2, // 50
    TngEffElm_Gold3,
    TngEffElm_Flash,
    TngEffElm_ElectricBall2,
    TngEffElm_RedPuff,
    TngEffElm_RedFlame,
    TngEffElm_BlueFlame,
    TngEffElm_GreenFlame,
    TngEffElm_YellowFlame,
    TngEffElm_Chicken,
    TngEffElm_ElectricBall3, // 60
    TngEffElm_Feathers,
    TngEffElm_Unknown62,
    TngEffElm_WhiteSparklesSmall,
    TngEffElm_GreenSparklesSmall,
    TngEffElm_RedSparklesSmall,
    TngEffElm_BlueSparklesSmall,
    TngEffElm_WhiteSparklesMed,
    TngEffElm_GreenSparklesMed,
    TngEffElm_RedSparklesMed,
    TngEffElm_BlueSparklesMed, // 70
    TngEffElm_WhiteSparklesLarge,
    TngEffElm_GreenSparklesLarge,
    TngEffElm_RedSparklesLarge,
    TngEffElm_BlueSparklesLarge,
    TngEffElm_RedSmokePuff,
    TngEffElm_BlueSmokePuff,
    TngEffElm_GreenSmokePuff,
    TngEffElm_YellowSmokePuff,
    TngEffElm_BluePuff,
    TngEffElm_GreenPuff, // 80
    TngEffElm_YellowPuff,
    TngEffElm_WhitePuff,
    TngEffElm_RedTwinkle3,
    TngEffElm_Thingy4,
    TngEffElm_BloodSplat,
    TngEffElm_BlueTwinkle,
    TngEffElm_GreenTwinkle,
    TngEffElm_YellowTwinkle,
    TngEffElm_CloudDisperse,
    TngEffElm_BlueTwinke2, // 90
    TngEffElm_GreenTwinkle2,
    TngEffElm_YellowTwinkle2,
    TngEffElm_RedDot,
    TngEffElm_IceMelt3,
    TngEffElm_DiseaseFly,
};

/******************************************************************************/
#pragma pack(1)

struct InitEffect;
struct Thing;

struct EffectGeneratorStats { // sizeof = 57
    long genation_delay_min;
    long genation_delay_max;
    long genation_amount;
    long effect_sound;
    unsigned char field_10;
    long field_11;
    long acc_x_min;
    long acc_x_max;
    long acc_y_min;
    long acc_y_max;
    long acc_z_min;
    long acc_z_max;
    long sound_sample_idx;
    long sound_sample_rng;
    long sound_sample_sec;
};

struct EffectElementStats { // sizeof = 79
  unsigned char draw_class;
  unsigned char field_1;
  unsigned char field_2;
  short numfield_3;
  short numfield_5;
  short sprite_idx;
  short sprite_size_min;
  short sprite_size_max;
  unsigned char field_D;
  unsigned short sprite_speed_min;
  unsigned short sprite_speed_max;
  unsigned char field_12;
  unsigned char field_13;
  unsigned char field_14;  // transparency flags in bits 4-5
  unsigned char field_15;
  unsigned char field_16;
  unsigned char field_17;
  unsigned char field_18;
  unsigned char field_19;
  unsigned char field_1A;
  unsigned char field_1B;
  unsigned char field_1C;
  unsigned char field_1D;
  unsigned short subeffect_model;
  unsigned short subeffect_delay;
  unsigned char field_22;
  unsigned short effmodel_23;
  unsigned short solidgnd_snd_smpid;
  unsigned short solidgnd_loudness;
  unsigned char solidgnd_destroy_on_impact;
  unsigned short water_effmodel;
  unsigned short water_snd_smpid;
  unsigned short water_loudness;
  unsigned char water_destroy_on_impact;
  unsigned short lava_effmodel;
  unsigned short lava_snd_smpid;
  unsigned short lava_loudness;
  unsigned char lava_destroy_on_impact;
  unsigned short transform_model;
  unsigned short field_3A;
  unsigned char field_3C;
  long field_3D;
  long field_41;
  long field_45;
  long field_49;
  unsigned char field_4D;
  unsigned char affected_by_wind;
};

struct InitEffect { // sizeof = 39
    /** Health; decreases by 1 on every turn, so it works also as lifespan. */
  short start_health;
  unsigned char generation_type;
  short accel_xy_min;
  short accel_xy_max;
  short accel_z_min;
  short accel_z_max;
  unsigned char field_B;
  short effect_sound;
  unsigned char kind_min;
  unsigned char kind_max;
  unsigned char area_affect_type;
  unsigned char field_11;
  struct InitLight ilght;
  unsigned char affected_by_wind;
};

#pragma pack()
/******************************************************************************/
extern const int birth_effect_element[];
/******************************************************************************/
struct InitEffect *get_effect_info(ThingModel effmodel);
struct InitEffect *get_effect_info_for_thing(const struct Thing *thing);
struct EffectElementStats *get_effect_element_model_stats(ThingModel tngmodel);

TbBool thing_is_effect(const struct Thing *thing);
struct Thing *create_effect(const struct Coord3d *pos, ThingModel effmodel, PlayerNumber owner);
struct Thing *create_effect_generator(struct Coord3d *pos, unsigned short model, unsigned short range, unsigned short owner, long parent_idx);
struct Thing *create_effect_element(const struct Coord3d *pos, unsigned short a2, unsigned short a3);
TngUpdateRet update_effect_element(struct Thing *thing);
TngUpdateRet update_effect(struct Thing *thing);
TngUpdateRet process_effect_generator(struct Thing *thing);
void process_spells_affected_by_effect_elements(struct Thing *thing);
TbBool destroy_effect_thing(struct Thing *thing);
struct Thing *create_special_used_effect(const struct Coord3d *pos, long plyr_idx);
struct Thing *create_price_effect(const struct Coord3d *pos, long plyr_idx, long price);

TbBool area_effect_can_affect_thing(const struct Thing *thing, HitTargetFlags hit_targets, PlayerNumber shot_owner);
long explosion_affecting_area(struct Thing *tngsrc, const struct Coord3d *pos, MapCoord max_dist,
    HitPoints max_damage, long blow_strength, HitTargetFlags hit_targets, DamageType damage_type);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
