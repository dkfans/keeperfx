/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config.h
 *     Header file for config.c.
 * @par Purpose:
 *     Configuration and campaign files support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     30 Jan 2009 - 11 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_CONFIG_H
#define DK_CONFIG_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct CreditsItem;
struct GameCampaign;
/******************************************************************************/
#define SINGLEPLAYER_FINISHED        -1
#define SINGLEPLAYER_NOTSTARTED       0
#define LEVELNUMBER_ERROR            -2

#define MIN_CONFIG_FILE_SIZE          4

#define LANDVIEW_MAP_WIDTH         1280
#define LANDVIEW_MAP_HEIGHT         960

enum TbFileGroups {
        FGrp_None,
        FGrp_StdData,
        FGrp_LrgData,
        FGrp_FxData,
        FGrp_LoData,
        FGrp_HiData,
        FGrp_VarLevels,
        FGrp_Save,
        FGrp_SShots,
        FGrp_StdSound,
        FGrp_LrgSound,
        FGrp_AtlSound,
        FGrp_Main,
        FGrp_Campgn,
        FGrp_CmpgLvls,
        FGrp_LandView,
        FGrp_CrtrData,
        FGrp_CmpgCrtrs,
        FGrp_CmpgConfig,
        FGrp_CmpgMedia,
        FGrp_Music,
};

enum TbExtraLevels {
    ExLv_None      =  0,
    ExLv_FullMoon  =  1,
    ExLv_NewMoon   =  2,
};

enum TbLevelOptions {
    LvOp_None      =  0x00,
    LvOp_IsSingle  =  0x01,
    LvOp_IsMulti   =  0x02,
    LvOp_IsBonus   =  0x04,
    LvOp_IsExtra   =  0x08,
    LvOp_IsFree    =  0x10,
    LvOp_AlwsVisbl =  0x20,
    LvOp_Tutorial  =  0x40,
};

enum TbLevelState {
    LvSt_Hidden    =  0,
    LvSt_HalfShow  =  1,
    LvSt_Visible   =  2,
};

enum TbLevelLocation {
    LvLc_VarLevels =  0,
    LvLc_Campaign  =  1,
    LvLc_Custom    =  2,
};



enum TbConfigLoadFlags {
    CnfLd_Standard      =  0x00, /**< Standard load, no special behavior. */
    CnfLd_ListOnly      =  0x01, /**< Load only list of items and their names, don't parse actual options (when applicable). */
    CnfLd_AcceptPartial =  0x02, /**< Accept partial files (with only some options set), and don't clear previous configuration. */
    CnfLd_IgnoreErrors  =  0x04, /**< Do not log error message on failures (still, return with error). */
};

#pragma pack(1)


/******************************************************************************/

enum confCommandResults
{
    ccr_comment = 0,
    ccr_ok = 1,
    ccr_endOfFile = -1,
    ccr_unrecognised = -2,
    ccr_endOfBlock = -3,
    ccr_error = -4,
};

enum confChangeSource
{
    ccs_CfgFile,
    ccs_DkScript,
};

enum dataTypes
{
    dt_default,
    dt_uchar,
    dt_schar,
    dt_char,
    dt_short,
    dt_ushort,
    dt_int,
    dt_uint,
    dt_long,
    dt_ulong,
    dt_longlong,
    dt_ulonglong,
    dt_float,
    dt_double,
    dt_longdouble,
    dt_void,
    dt_charptr,
};

#define var_type(expr)\
    (_Generic((expr),\
              unsigned char: dt_uchar, char: dt_char, \
              short: dt_short, unsigned short: dt_ushort, \
              int: dt_int, unsigned int: dt_uint, \
              long: dt_long, unsigned long: dt_ulong, \
              long long: dt_longlong, unsigned long long: dt_ulonglong, \
              float: dt_float, \
              double: dt_double, \
              long double: dt_longdouble, \
              void*: dt_void, \
              char*: dt_charptr, \
              default: dt_default))

#define field(field)\
    &field, var_type(field)

/******************************************************************************/
struct CommandWord {
    char text[COMMAND_WORD_LEN];
};

struct NamedCommand {
    const char *name;
    int num;
};

struct LongNamedCommand {
    const char* name;
    long long num;
};

struct NamedFieldSet;

struct NamedField {
    const char *name;
    char argnum; //for fields that assign multiple values, -1 passes full string to assign function
    void* field;
    uchar type;
    int64_t default_value;
    int64_t min;
    int64_t max;
    const struct NamedCommand *namedCommand;
    int64_t (*parse_func)(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src); // converts the text to the a number
    void (*assign_func)(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
};

struct NamedFieldSet {
    long *const count_field;
    const char* block_basename;
    const struct NamedField* named_fields;
    struct NamedCommand* names;
    const int max_count;
    const size_t struct_size;
    const void* struct_base;
    const char* src_strs[2]; 
};

#define NAMFIELDWRNLOG(format, ...) LbWarnLog("%s(line %lu): " format "\n", named_fields_set->src_strs[src] , text_line_number, ##__VA_ARGS__)

extern TbBool AssignCpuKeepers;

extern unsigned int vid_scale_flags;

extern const struct NamedCommand logicval_type[];

/******************************************************************************/
extern char keeper_runtime_directory[152];

#pragma pack()
/******************************************************************************/
extern unsigned long text_line_number;
/******************************************************************************/
char *prepare_file_path_buf(char *ffullpath,short fgroup,const char *fname);
char *prepare_file_path(short fgroup,const char *fname);
char *prepare_file_fmtpath(short fgroup, const char *fmt_str, ...);
unsigned char *load_data_file_to_buffer(long *ldsize, short fgroup, const char *fmt_str, ...);
/******************************************************************************/
short load_configuration(void);
void process_cmdline_overrides(void);
/******************************************************************************/
short is_bonus_level(LevelNumber lvnum);
short is_extra_level(LevelNumber lvnum);
short is_singleplayer_level(LevelNumber lvnum);
short is_singleplayer_like_level(LevelNumber lvnum);
short is_multiplayer_level(LevelNumber lvnum);
short is_campaign_level(LevelNumber lvnum);
short is_freeplay_level(LevelNumber lvnum);
int array_index_for_bonus_level(LevelNumber bn_lvnum);
int array_index_for_extra_level(LevelNumber ex_lvnum);
int array_index_for_singleplayer_level(LevelNumber sp_lvnum);
int array_index_for_multiplayer_level(LevelNumber mp_lvnum);
int array_index_for_freeplay_level(LevelNumber fp_lvnum);
int storage_index_for_bonus_level(LevelNumber bn_lvnum);
LevelNumber first_singleplayer_level(void);
LevelNumber last_singleplayer_level(void);
LevelNumber next_singleplayer_level(LevelNumber sp_lvnum);
LevelNumber prev_singleplayer_level(LevelNumber sp_lvnum);
LevelNumber bonus_level_for_singleplayer_level(LevelNumber sp_lvnum);
LevelNumber first_multiplayer_level(void);
LevelNumber last_multiplayer_level(void);
LevelNumber next_multiplayer_level(LevelNumber mp_lvnum);
LevelNumber prev_multiplayer_level(LevelNumber mp_lvnum);
LevelNumber first_freeplay_level(void);
LevelNumber last_freeplay_level(void);
LevelNumber next_freeplay_level(LevelNumber fp_lvnum);
LevelNumber prev_freeplay_level(LevelNumber fp_lvnum);
LevelNumber first_extra_level(void);
LevelNumber next_extra_level(LevelNumber ex_lvnum);
LevelNumber get_extra_level(unsigned short elv_kind);
// Level info support for active campaign
struct LevelInformation *get_level_info(LevelNumber lvnum);
struct LevelInformation *get_or_create_level_info(LevelNumber lvnum, unsigned long lvoptions);
struct LevelInformation *get_first_level_info(void);
struct LevelInformation *get_last_level_info(void);
struct LevelInformation *get_next_level_info(struct LevelInformation *previnfo);
struct LevelInformation *get_prev_level_info(struct LevelInformation *nextinfo);
short set_level_info_text_name(LevelNumber lvnum, char *name, unsigned long lvoptions);
short set_level_info_string_index(LevelNumber lvnum, char *stridx, unsigned long lvoptions);
short get_level_fgroup(LevelNumber lvnum);
const char *get_current_language_str(void);
const char *get_language_lwrstr(int lang_id);
/******************************************************************************/
TbBool reset_credits(struct CreditsItem *credits);
TbBool setup_campaign_credits_data(struct GameCampaign *campgn);
/******************************************************************************/
TbBool parameter_is_number(const char* parstr);

short find_conf_block(const char *buf,long *pos,long buflen,const char *blockname);
TbBool iterate_conf_blocks(const char * buf, long * pos, long buflen, const char ** name, int * namelen);
int recognize_conf_command(const char *buf,long *pos,long buflen,const struct NamedCommand *commands);
TbBool skip_conf_to_next_line(const char *buf,long *pos,long buflen);
int get_conf_parameter_single(const char *buf,long *pos,long buflen,char *dst,long dstlen);
int get_conf_parameter_whole(const char *buf,long *pos,long buflen,char *dst,long dstlen);
int get_conf_parameter_quoted(const char *buf,long *pos,long buflen,char *dst,long dstlen);

int get_conf_list_int(const char *buf, const char **state, int *dst);
TbBool parse_named_field_block(const char *buf, long len, const char *config_textname, unsigned short flags,const char* blockname,
    const struct NamedField named_field[], const struct NamedFieldSet* named_fields_set, int idx);
TbBool parse_named_field_blocks(char *buf, long len, const char *config_textname, unsigned short flags,
        const struct NamedFieldSet* named_fields_set);
int recognize_conf_parameter(const char *buf,long *pos,long buflen,const struct NamedCommand *commands);
int assign_conf_command_field(const char *buf,long *pos,long buflen,const struct NamedField *commands, const struct NamedFieldSet* named_fields_set, int idx);
void assign_named_field_value(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
const char *get_conf_parameter_text(const struct NamedCommand commands[],int num);
long get_named_field_id(const struct NamedField *desc, const char *itmname);
long get_id(const struct NamedCommand *desc, const char *itmname);
long long get_long_id(const struct LongNamedCommand* desc, const char* itmname);
long get_rid(const struct NamedCommand *desc, const char *itmname);
/******************************************************************************/
int64_t value_name           (const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
int64_t value_default        (const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
int64_t value_flagsfield     (const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
int64_t value_flagsfieldshift(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
int64_t value_icon           (const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
int64_t value_effOrEffEl     (const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
int64_t value_animid         (const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);

void assign_icon   (const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
void assign_default(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
void assign_null   (const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);

int64_t parse_named_field_value(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
int64_t get_named_field_value(const struct NamedField* named_field, const struct NamedFieldSet* named_fields_set, int idx);

#ifdef __cplusplus
}
#endif
#endif
