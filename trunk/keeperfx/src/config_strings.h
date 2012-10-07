/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_strings.h
 *     Header file for config_strings.c.
 * @par Purpose:
 *     List of language-specific strings support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 31 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGSTRINGS_H
#define DK_CFGSTRINGS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct GameCampaign;

enum GUIStrings {
    GUIStr_SuccessLandIsYours = 0, // per-campaign
    GUIStr_LevelEventMessage = 1, // range 1..200, per-campaign
    GUIStr_Empty = 201,
    GUIStr_LevelName = 202, // range 202..221, per-campaign
    GUIStr_NameAndHealthDesc = 222,
    GUIStr_ExperienceDesc,
    GUIStr_HungerDesc,
    GUIStr_Cancel,
    GUIStr_CreatureSpellDesc = 226, // range 226..257
    GUIStr_CreatureKind1 = 258, // range 258..286, per-campaign
    GUIStr_CreatureQueryDesc = 287,
    GUIStr_SelectGame,
    GUIStr_NetworkMenu,
    GUIStr_MainMenu,
    GUIStr_CreatureAngerDesc = 291,
    GUIStr_CreatureKillsDesc,
    GUIStr_CreatureStrengthDesc,
    GUIStr_CreatureWageDesc,
    GUIStr_CreatureGoldHeldDesc,
    GUIStr_CreatureDefenceDesc,
    GUIStr_CreatureSkillDesc,
    GUIStr_CreatureTimeInDungeonDesc,
    GUIStr_CreatureDexterityDesc,
    GUIStr_CreatureLuckDesc,
    GUIStr_CreatureBloodTypeDesc,
    GUIStr_CreatureIdleDesc,
    GUIStr_CreatureWorkingDesc,
    GUIStr_CreatureFightingDesc,
    GUIStr_CreatureFightDesc,
    GUIStr_CreatureFleeDesc,
    GUIStr_CreatureImprisonDesc,
    GUIStr_CreatureDefendingDesc,
    GUIStr_ConfirmYouSure = 309,
    GUIStr_ConfirmYes,
    GUIStr_ConfirmNo,
    GUIStr_For,
    GUIStr_OptionShadowsDesc = 313,
    GUIStr_OptionViewTypeDesc,
    GUIStr_OptionWallHeightDesc,
    GUIStr_OptionViewDistanceDesc,
    GUIStr_OptionGammaCorrectionDesc,
    GUIStr_Of,
    GUIStr_TerrainEmptyLairDesc = 319, // per-campaign
    GUIStr_PausedMsg = 320,
    GUIStr_PaneZoomInDesc,
    GUIStr_PaneZoomOutDesc,
    GUIStr_PaneLargeMapDesc,
    GUIStr_TerrainVarDesc1 = 324, // range 324..332, per-campaign
    GUIStr_PaneMore = 333,
    GUIStr_LevelWon = 334, // per-campaign
    GUIStr_LevelLost, // per-campaign
    GUIStr_RecMovie = 336,
    GUIStr_RecMovieFail,
    GUIStr_RecMovieDone,
    GUIStr_OptionSoundFx = 340,
    GUIStr_OptionMusic,
    GUIStr_SlotUnused = 342,
    GUIStr_MnuMainMenu = 343,
    GUIStr_MnuLoadMenu,
    GUIStr_MnuLoadGame,
    GUIStr_MnuContinueGame,
    GUIStr_MnuMultiplayer = 347,
    GUIStr_MnuReturnToMain,
    GUIStr_MnuPlayIntro,
    GUIStr_NetServiceMenu,
    GUIStr_NetSessionMenu,
    GUIStr_NetSpeed = 352,
    GUIStr_NetComPort,
    GUIStr_NetPhoneNumber,
    GUIStr_NetIrq,
    GUIStr_MnuStatistics = 356,
    GUIStr_LevelCompleted = 357,
    GUIStr_MnuUnused = 358,
    GUIStr_MnuQuit = 359,
    GUIStr_MnuStartNewGame = 360,
    GUIStr_TastyHeroes = 361,
    GUIStr_CreditsHead1 = 362, // range 362..394
    GUIStr_NetSessions = 395,
    GUIStr_NetName,
    GUIStr_NetServices,
    GUIStr_NetMessages,
    GUIStr_NetCreateGame,
    GUIStr_NetJoinGame,
    GUIStr_NetStartGame,
    GUIStr_MnuGameMenu = 402,
    GUIStr_MnuCancel,
    GUIStr_MnuNoName,
    GUIStr_MnuPlayers,
    GUIStr_MnuLevel,
    GUIStr_MnuLevels,
    GUIStr_MnuGames = 408,
    GUIStr_NetModemMenu = 409,
    GUIStr_NetSerialMenu,
    GUIStr_NetInit = 411,
    GUIStr_NetHangup,
    GUIStr_NetClear,
    GUIStr_NetAnswer,
    GUIStr_NetStart,
    GUIStr_NetAlly = 416,
    GUIStr_NetAlliance,
    GUIStr_Credits = 418,
    GUIStr_MnuOk = 419,
    GUIStr_SpecRevealMapDesc = 420,
    GUIStr_SpecResurrectCreatureDesc,
    GUIStr_SpecTransferCreatureDesc,
    GUIStr_SpecStealHeroDesc,
    GUIStr_SpecMultiplyCreaturesDesc,
    GUIStr_SpecIncreaseLevelDesc,
    GUIStr_SpecMakeSafeDesc,
    GUIStr_SpecLocateHiddenWorldDesc,
    GUIStr_SpecResurrectCreature,
    GUIStr_SpecTransferCreature,
    GUIStr_BonusLevel = 430,
    GUIStr_MnuHighScoreTable = 431,
    GUIStr_GoToQueryMode,
    GUIStr_MoreInformation = 433,
    GUIStr_BackToMainQueryScreen,
    GUIStr_SelectedAction,
    GUIStr_TeamChooseParty = 436,
    GUIStr_TeamEnterDungeon,
    GUIStr_TeamPartyMembers,
    GUIStr_TeamAvailCreatures,
    GUIStr_TeamCreature,
    GUIStr_TeamMoneyAvailable,
    GUIStr_TeamLeader,
    GUIStr_TeamHire,
    GUIStr_TeamFire,
    GUIStr_TeamCost,
    GUIStr_TeamType,
    GUIStr_InformationPanelDesc = 447,
    GUIStr_RoomPanelDesc = 448,
    GUIStr_ResearchPanelDesc = 449,
    GUIStr_WorkshopPanelDesc = 450,
    GUIStr_CreaturePanelDesc = 451,
    GUIStr_ResearchTimeDesc = 452,
    GUIStr_WorkshopTimeDesc = 453,
    GUIStr_PayTimeDesc = 454,
    GUIStr_NumberOfRoomsDesc = 455,
    GUIStr_NumberOfCreaturesDesc = 456,
    GUIStr_TeamChooseGame,
    GUIStr_TeamGameType,
    GUIStr_TeamKeeperVsKeeper,
    GUIStr_TeamKeeperVsHeroes,
    GUIStr_TeamDeathmatch,
    GUIStr_SellRoomDesc = 462,
    GUIStr_SellItemDesc,
    GUIStr_NextBattleDesc,
    GUIStr_CloseWindow,
    GUIStr_ZoomToArea,
    GUIStr_NoMouseInstalled,
    GUIStr_DefineKeys,
    GUIStr_AllyWithPlayer,
    GUIStr_PressAKey = 470,
    GUIStr_CtrlUp,
    GUIStr_CtrlDown,
    GUIStr_CtrlLeft,
    GUIStr_CtrlRight,
    GUIStr_CtrlRotate,
    GUIStr_CtrlSpeed,
    GUIStr_CtrlRotateLeft,
    GUIStr_CtrlRotateRight,
    GUIStr_CtrlZoomIn,
    GUIStr_CtrlZoomOut,
    GUIStr_KeyLeftControl = 481,
    GUIStr_KeyRightControl,
    GUIStr_KeyLeftShift,
    GUIStr_KeyRightShift,
    GUIStr_KeyLeftAlt,
    GUIStr_KeyRightAlt,
    GUIStr_KeySpace,
    GUIStr_KeyReturn,
    GUIStr_KeyTab,
    GUIStr_KeyCapsLock,
    GUIStr_KeyBackspace,
    GUIStr_KeyInsert,
    GUIStr_KeyDelete,
    GUIStr_KeyHome,
    GUIStr_KeyEnd,
    GUIStr_KeyPageUp,
    GUIStr_KeyPageDown,
    GUIStr_KeyNumLock = 498,
    GUIStr_KeyNumSlash,
    GUIStr_KeyNumMul,
    GUIStr_KeyNumSub,
    GUIStr_KeyNumAdd,
    GUIStr_KeyNumEnter,
    GUIStr_KeyNumDelete,
    GUIStr_KeyNum1,
    GUIStr_KeyNum2,
    GUIStr_KeyNum3,
    GUIStr_KeyNum4,
    GUIStr_KeyNum5,
    GUIStr_KeyNum6,
    GUIStr_KeyNum7,
    GUIStr_KeyNum8,
    GUIStr_KeyNum9,
    GUIStr_KeyNum0,
    GUIStr_KeyF1 = 515,
    GUIStr_KeyF2,
    GUIStr_KeyF3,
    GUIStr_KeyF4,
    GUIStr_KeyF5,
    GUIStr_KeyF6,
    GUIStr_KeyF7,
    GUIStr_KeyF8,
    GUIStr_KeyF9,
    GUIStr_KeyF10,
    GUIStr_KeyF11,
    GUIStr_KeyF12,
    GUIStr_KeyUp,
    GUIStr_KeyDown,
    GUIStr_KeyLeft,
    GUIStr_KeyRight,
    GUIStr_NetInitingModem = 531,
    GUIStr_NetConnectnModem,
    GUIStr_NetDial,
    GUIStr_NetContinue,
    GUIStr_NetLineEngaged,
    GUIStr_NetUnknownError,
    GUIStr_NetNoCarrier,
    GUIStr_NetNoDialTone,
    GUIStr_NetNoResponse,
    GUIStr_NetNoServer,
    GUIStr_NetUnableToInit = 541,
    GUIStr_NetUnableToCrGame,
    GUIStr_NetUnableToJoin,
    GUIStr_TerrainVarDesc2 = 544, // range 544..545, per-campaign
    GUIStr_CreatureKind2 = 546, // range 546..547, per-campaign
    GUIStr_CompAssNowAggressive = 548,
    GUIStr_CompAssNowDefensive,
    GUIStr_CompAssNowConstruction,
    GUIStr_CompAssNowMoveOnly,
    GUIStr_RoomKind1 = 552, // range 552..566, per-campaign
    GUIStr_StateFight = 567,
    GUIStr_StateAnnoyed = 568,
    GUIStr_KeyShift = 569,
    GUIStr_KeyControl,
    GUIStr_KeyAlt,
    GUIStr_CreditsHead2 = 572, // range 572..577
    GUIStr_BoulderTrap = 578, // per-campaign
    GUIStr_TrapKindAlarm, // per-campaign
    GUIStr_TrapKindPoisonGas, // per-campaign
    GUIStr_TrapKindLightning, // per-campaign
    GUIStr_TrapKindWordOfPower, // per-campaign
    GUIStr_TrapKindLava, // per-campaign
    GUIStr_TrapBoulderDesc = 584, // per-campaign
    GUIStr_AlarmTrapDesc, // per-campaign
    GUIStr_PoisonGasTrapDesc, // per-campaign
    GUIStr_LightningTrapDesc, // per-campaign
    GUIStr_WordOfPowerTrapDesc, // per-campaign
    GUIStr_LavaTrapDesc, // per-campaign
    GUIStr_DoorKindWooden = 590, // per-campaign
    GUIStr_DoorKindBraced, // per-campaign
    GUIStr_DoorKindIron, // per-campaign
    GUIStr_DoorKindMagic, // per-campaign
    GUIStr_WoodenDoorDesc = 594, // per-campaign
    GUIStr_BracedDoorDesc, // per-campaign
    GUIStr_IronDoorDesc, // per-campaign
    GUIStr_MagicDoorDesc, // per-campaign
    GUIStr_RoomKind2 = 598, // range 598..614, per-campaign
    GUIStr_RoomDesc1 = 615, // range 615..629, per-campaign
    GUIStr_PowerKind1 = 630, // range 630..646, per-campaign
    GUIStr_PowerDesc1 = 647, // range 647..663, per-campaign
    GUIStr_EventTreasureRoomFull = 664,
    GUIStr_EventScavengingDetected,
    GUIStr_EventCreaturePayday,
    GUIStr_EventNewSpellPickedUp,
    GUIStr_EventNewRoomTakenOver,
    GUIStr_EventNewAreaDiscovered,
    GUIStr_EventInformation,
    GUIStr_EventRoomLost,
    GUIStr_EventHeartAttacked,
    GUIStr_EventFight,
    GUIStr_EventObjective,
    GUIStr_EventBreach,
    GUIStr_EventNewSpellResearched,
    GUIStr_EventNewRoomResearched = 677,
    GUIStr_EventNewTrap,
    GUIStr_EventNewDoor,
    GUIStr_EventNewCreature,
    GUIStr_EventCreatureAnnoyed,
    GUIStr_EventNoMoreLivingSpace,
    GUIStr_EventAlarmTriggered,
    GUIStr_EventRoomUnderAttack,
    GUIStr_EventTreasureRoomNeeded,
    GUIStr_EventCreaturesHungry,
    GUIStr_EventTrapCrateFound,
    GUIStr_EventDoorCrateFound,
    GUIStr_EventDnSpecialFound = 689,
    GUIStr_EventTreasureRoomFullDesc = 690,
    GUIStr_EventCreatureScavengedDesc,
    GUIStr_EventCreaturePaydayDesc,
    GUIStr_EventNewSpellPickedUpDesc,
    GUIStr_EventNewRoomTakenOverDesc,
    GUIStr_EventNewAreaDiscoveredDesc,
    GUIStr_EventInformationDesc,
    GUIStr_EventRoomLostDesc,
    GUIStr_EventDnHeartAttackedDesc = 698,
    GUIStr_EventFightDesc,
    GUIStr_EventObjectiveDesc,
    GUIStr_EventBreachDesc,
    GUIStr_EventNewSpellResrchDesc,
    GUIStr_EventNewRoomResrchDesc,
    GUIStr_EventNewTrapDesc,
    GUIStr_EventNewDoorDesc,
    GUIStr_EventNewCreatureDesc,
    GUIStr_EventCreatureIsAnnoyedDesc,
    GUIStr_EventNoMoreLivingSetDesc,
    GUIStr_EventAlarmTriggeredDesc,
    GUIStr_EventRoomUnderAttackDesc,
    GUIStr_EventNeedTreasureRoomDesc,
    GUIStr_EventCreaturesHungryDesc,
    GUIStr_EventTrapCrateFoundDesc,
    GUIStr_EventDoorCrateFoundDesc,
    GUIStr_EventDnSpecialFoundDesc = 715,
    GUIStr_MnuOptions = 716,
    GUIStr_MnuGraphicsOptions,
    GUIStr_MnuSoundOptions,
    GUIStr_MnuLoad,
    GUIStr_MnuSave,
    GUIStr_MnuComputerAssist,
    GUIStr_MnuOptionsDesc = 722,
    GUIStr_GraphicsMenuDesc = 723,
    GUIStr_SoundMenuDesc,
    GUIStr_LoadGameDesc,
    GUIStr_SaveGameDesc,
    GUIStr_QuitGameDesc = 727,
    GUIStr_ComputerAssistDesc = 728,
    GUIStr_AggressiveAssistDesc,
    GUIStr_DefensiveAssistDesc,
    GUIStr_ConstructionAssistDesc,
    GUIStr_MoveOnlyAssistDesc,
    GUIStr_PickCreatrMostExpdesc = 733,
    GUIStr_PickCreatrIdleDesc,
    GUIStr_PickCreatrWorkingDesc,
    GUIStr_PickCreatrFightingDesc,
    GUIStr_OptionsInvertMouse = 737,
    GUIStr_OptionsPossessMouseSensitivity,
    GUIStr_OptionsMoreSensitive,
    GUIStr_OptionsLessSensitive = 740,
    GUIStr_StatisticsNames1 = 741, // range 741..802
    GUIStr_EasterPoems = 803, // range 803..839
    GUIStr_MnuRetToOptions = 840,
    GUIStr_MnuExit = 841,
    GUIStr_MnuAudio,
    GUIStr_MnuInvertMouse,
    GUIStr_MnuMouseSensitivity,
    GUIStr_MnuComputer,
    GUIStr_MnuComputerPlayers,
    GUIStr_On = 847,
    GUIStr_Off = 848,
    GUIStr_Sensitivity,
    GUIStr_MouseOptions,
    GUIStr_Mouse,
    GUIStr_UndoPickup,
    GUIStr_Pause,
    GUIStr_Map = 854,
    GUIStr_InsufficientMemory,
    GUIStr_UnableToChangeRes,
    GUIStr_Query = 857,
    GUIStr_CreditsHead3 = 858, // range 858..867
    GUIStr_NetAttemptingToJoin = 868,
    GUIStr_NetResyncing,
    GUIStr_Net1Player = 870,
    GUIStr_Net2Players,
    GUIStr_Net3Players,
    GUIStr_Net4Players,
    GUIStr_NetSerial = 874,
    GUIStr_NetModem,
    GUIStr_NetIpx,
    GUIStr_MapN = 877,
    GUIStr_MapE,
    GUIStr_MapS,
    GUIStr_MapW,
    GUIStr_Vs = 881,
    GUIStr_CreditsHead4 = 882, // range 882..884
    GUIStr_HappyBirthday = 885,
    GUIStr_Error,
    GUIStr_ErrorSaving,
    GUIStr_NewLevels = 888,
    GUIStr_InsertDataCD = 889,
    GUIStr_InsertKeeperCD,
    GUIStr_InbsertDeeperCD,
    GUIStr_LevelEventMessageDD = 892, // range 892..940, Deeper Dungeons
    GUIStr_MnuFreePlayLevels = 941,
    GUIStr_MnuLandSelection = 942,
    GUIStr_MnuCampaigns = 943,
    GUIStr_MnuAddComputer = 944,
};

enum CampaignStrings {
    CpgStr_Empty = 201,
};

/******************************************************************************/
TbBool setup_gui_strings_data(void);
TbBool free_gui_strings_data(void);
TbBool reset_strings(char **strings);
const char * gui_string(unsigned int index);
const char * cmpgn_string(unsigned int index);
TbBool setup_campaign_strings_data(struct GameCampaign *campgn);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
