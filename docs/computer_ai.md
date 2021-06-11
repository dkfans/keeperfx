## Overview

Computer logic consists of following primitives:

* Process
* Event
* Task

There is some "emergency mode" if any of these cases:

* AI lack more than 1000 gold for payday
* AI has less than 3 imps

There is an option `sim_before_dig` that affects how AI routes its corridors

## Process

Computer process is high-level aspect of AI activity.

Each process is consist of following functions:

* check
* setup
* process
* finish
* paused

All functions are interchangable but most combinations are useless

* check_build_all_rooms - sets Value4 to next available room that is not built
* setup_any_room_continue - creates a task to build a room.\
  Decrease room size one by one if failed.
* check_any_room - Checks if Value4 room available and some capacity needed
* setup_any_room - Setup task to build a room
* check_dig_to_entrance - Wait some time and fail if there is no claimable entrance
* setup_dig_to_entrance - Check time, choose an entrance and setup dig towards it
* check_dig_to_gold - Wait if AI has more gold than Value2 
* setup_dig_to_gold - Setup a task to dig for gold (or wait)  
* check_sight_of_evil - Wait for available SoE spell
* setup_sight_of_evil - Count how many casts are done and wait.\
  Then finish process after Param5 amount  
* process_sight_of_evil - Cast a SoE on map
* check_attack1 -
* setup_attack1 -
* completed_attack1 - ?Pickup creatures and turn off CTA?
* check_safe_attack -
* process_task - finish process
* completed_build_a_room -
* paused_task - pause process\
  Difference vs process_task should be explained
* completed_task - complete process\
  Difference vs process_task should be explained
* none - does nothing

There are `Param` settings. They are initial settings for some turns and counters. Most of them are used to choose next
process.

* Param1 - last continue turn (never used)
* Param2 - completed turn (completed or failed or suspended or reset)
* Param3 - last fail turn
* Param4 - last run turn
* Param5 - used as task_id or counter for SoE

## Room building processes

```
[process1]
Name = BUILD ALL ROOM 3x3
Mnemonic = RoomAll3
; Values = <priority> <room_width> <room_height> <room_kind> <room_link>
; <priority> - priority of the process; this parameter controls which process to choose if more than one process has met the conditions to be conducted
; <room_width> - number of tiles the room will be wide
; <room_height> - number of tiles the room will be high, height and weight may differ no more than 1.
; <room_kind> - the room kind to be built
Values = 0 3 3 0 -1
Functions = check_build_all_rooms setup_any_room_continue process_task completed_build_a_room paused_task
Params = 0 0 0 0 0 0
```

```
[process2]
Name = BUILD ALL ROOM 4x4
Mnemonic = RoomAll4
; Values = <priority> <room_width> <room_height> <room_kind> <room_link>
Values = 0 4 4 0 -1
Functions = check_build_all_rooms setup_any_room_continue process_task completed_task paused_task
Params = 0 0 0 0 0 0
```

## Per room processes

Common settings:

```
; Values = <priority> <room_width> <room_height> <room_kind> <room_link>
; <priority> - priority of the process; this parameter controls which process to choose if more than one process has met the conditions to be conducted
; <room_width> - number of tiles width the room should have
; <room_height> - number of tiles height the room should have; this value should not differ from room_width by more than 1, if it does then a room with non-rectangular shape can result
; <room_kind> - the room kind to be built
; <room_link> - the room kind which we'd prefer to build a connection from; if not available then build anywhere
Functions = check_any_room setup_any_room process_task completed_task paused_task
Params = 0 0 0 0 0 0
```

* Each time AI tried to build a room and failed it will try to build smaller room `player_compprocs.c:284`
* AI tries to build a room near _linked one_. Next room (by room number) is taken if failed. `player_computer.c:262`
* Each process is completed whenever room is built.
* Lair size is impacted by MAX_CREATURES in level script `player_computer.c:230`

```
[process3]
Name = BUILD A PRISON ROOM
Mnemonic = RoomPrisn
Values = 0 3 4 4 12
```

* Linked to a barracks ?

```
[process4]
Name = BUILD A TORTURE ROOM
Mnemonic = RoomTortr
Values = 0 3 4 5 12
```

* Linked to a barracks ?

```
[process5]
Name = BUILD A SCAVENGER ROOM
Mnemonic = RoomScavn
Values = 0 3 3 9 13
```

* Linked to a hatchery

```
[process6]
Name = BUILD A TEMPLE ROOM
Mnemonic = RoomTempl
Values = 0 3 3 10 12
```

* Linked to a barracks ???

```
[process7]
Name = BUILD A GRAVEYARD ROOM
Mnemonic = RoomGrave
Values = 0 3 4 11 14
```

* Linked to a lair ???

```
[process8]
Name = BUILD A BARRACK ROOM
Mnemonic = RoomBarrc
Values = 0 3 4 12 9
```

* Linked to a Scavenger room?

```
[process9]
Name = BUILD A TREASURE ROOM
Mnemonic = RoomTresr
Values = 10 5 5 2 6
```

* Linked to a training room

```
[process10]
Name = BUILD A RESEARCH ROOM
Mnemonic = RoomRsrch
Values = 0 5 5 3 7
```

* Linked to Heart

```
[process11]
Name = BUILD A HATCHERY
Mnemonic = RoomHatch
Values = 0 6 5 13 7
```

* Linked to a Heart

```
[process12]
Name = BUILD A LAIR ROOM
Mnemonic = RoomLair
; Lair size is also impacted by MAX_CREATURES in level script.
Values = 0 5 5 14 7
```

* Linked to a heart

```
[process13]
Name = BUILD A TRAINING ROOM
Mnemonic = RoomTrain
Values = 0 5 6 6 13
```

* Linked to a hatchery

```
[process14]
Name = BUILD A WORKSHOP ROOM
Mnemonic = RoomWrksh
Values = 0 6 6 8 13
```

* Linked to a hatchery

```
[process17]
Name = BUILD A TREASURE ROOM 4x4
Mnemonic = RoomTres4
Values = 10 4 4 2 7
```

* Linked to a heart

```
[process18]
Name = BUILD A LAIR ROOM 4x4
Mnemonic = RoomLair4
Values = 0 4 4 14 7
```

* Linked to a heart

## Dig to an entrance

```
[process15]
Name = DIG TO AN ENTRANCE
Mnemonic = DigEntrn
Values = 0 1700 0 0 0
Functions = check_dig_to_entrance setup_dig_to_entrance process_task completed_task paused_task
```

# Dig to gold

Ai want to dig some gold. These processes mark few slabs of gold starting from nearest to any room

Common:

```
Functions = check_dig_to_gold setup_dig_to_gold process_task completed_task paused_task
```

```
[process16]
; Process of tunnelling to gold slabs and marking them for digging
Name = DIG TO GOLD
Mnemonic = DigGold
; Values = <priority> <money_below> <distance_inc_turns> <slabs_at_once> <initial_distance>
; <priority> - priority of the process, it is increased automatically evey time computer player lacks money
; <money_below> - gold amount; the process is started only if prediction of gold left after next payday falls below it
; <distance_inc_turns> - max digging distance increases with length of the gameplay; every given gameturns amount it expands one subtile
; <slabs_at_once> - amount of slabs marked for diggind during one run of the process
; <initial_distance> - initial max distance at which  digging is allowed, in subtiles; it is increased every few gameturns
Values = 0 10999 150 7 0
```

```
[process19]
Name = DIG TO CLOSE GOLD
Mnemonic = DigClGold
; Values = <priority> <money_below> <distance_inc_turns> <slabs_at_once> <initial_distance>
Values = 0 30999 500 5 71
```

```
[process20]
Name = DIG TO GREEDY GOLD
Mnemonic = DigGrGold
; Values = <priority> <money_below> <distance_inc_turns> <slabs_at_once> <initial_distance>
Values = 0 40999 400 7 900
``````

```
[process21]
Name = DIG TO GREEDY GOLD2
Mnemonic = DigGrGld2
; Values = <priority> <money_below> <distance_inc_turns> <slabs_at_once> <initial_distance>
Values = 1 40999 200 10 900
```

## Sight of Evil

AI doesnt get anything from SoE spell. It is just to show some activity. Also AI will cast SoE only on unexplored areas

```
[process22]
Name = SIGHT OF EVIL
Mnemonic = SplSOE
Values = 0 8 64 1500 0
Functions = check_sight_of_evil setup_sight_of_evil process_sight_of_evil completed_task paused_task
```

```
[process23]
Name = SIGHT OF EVIL SCARE
Mnemonic = SOEScare
Values = 0 8 10 5000 0
Functions = check_sight_of_evil setup_sight_of_evil process_sight_of_evil completed_task paused_task
```

## Attacks

* Each attack is based on hate level.

Both attacks are performed only if all conditions are met

* No attack task
* `Value3 > NumOfNotFightingCreatures * Value2 / 100` - There are less than X fighting creatures
* `Value4 / 100 > NumOfActiveCreatures / MaxCreatures` - Attracted non less than % of max creatures

#### Plan1 Attack

This process will attack ANY enemy

```
[process24]
Name = ATTACK PLAN 1
Mnemonic = Attck1
Values = 0 55 6 80 0
Functions = check_attack1 setup_attack1 process_task completed_attack1 paused_task
```

#### Safe Attack

This attack only weak enemy (`check_safe_attack`)`player_compprocs.c:1079`

To perform this attack AI should have more than `1 + Value2/100 ` creatures than target.

```
[process25]
Name = ATTACK SAFE ATTACK
Mnemonic = AttckSafe
Values = 0 25 4 80 0
; Priority 
Functions = check_safe_attack setup_attack1 process_task completed_attack1 paused_task
```

# Event

Events are similar to Event boxes with exclamation mark when something happens.

```
Functions = <event_fn> <test_fn>
```

There is a difference between event_fn and test_fn \
_event_fn_ is watching for event list in game.event \
_test_fn_ is a periodic check \
Both of them also contain reaction on it

#### Combat events

```
[event1]
Name = EVENT DUNGEON BREACH
Mnemonic = DnBreach
; <type> <kind> <test_interval>
; kind is from EvKind enum
Values = 0 4 295
Functions = event_battle none
Params = 75 1 0 0
```

```
[event2]
Name = EVENT ROOM ATTACK
Mnemonic = AttkRom1
Values = 0 19 295
Functions = event_battle none
Params = 75 1 0 0
```

```
[event3]
Name = EVENT ROOM ATTACK
Mnemonic = AttkRom2
Values = 0 19 295
Functions = event_battle none
Params = 75 3 0 0
```

```
[event4]
Name = EVENT HEART UNDER ATTACK
Mnemonic = AttkHrt1
Values = 0 1 295
Functions = event_battle none
Params = 99 3 0 0
```

```
[event7]
Name = EVENT FIGHT
Mnemonic = Fight1
Values = 0 2 -2
Functions = event_battle none
Params = 75 1 0 0
```

```
[event8]
Name = EVENT FIGHT
Mnemonic = Fight2
Values = 0 2 -2
Functions = event_battle none
Params = 25 1 0 0
```

#### Lack of something

Both of these events are just reactivate their related Processes

```
[event5]
Name = EVENT TREASURE ROOM FULL
Mnemonic = RomFTrsr
Values = 0 11 1195
Functions = event_find_link none
Process = RoomTresr
Params = 0 0 0 0

[event6]
Name = EVENT LIVING SPACE FULL
Mnemonic = RomFLair
Values = 0 17 1195
Functions = event_find_link none
Process = RoomLair
Params = 0 0 0 0
```

#### Check for full rooms

This check will activate each room-related process to build rooms if they are needed. Will do nothing in emergency
state.

```
[event12]
Name = EVENT CHECK ROOMS FULL
Mnemonic = RomFull
Values = 3 3 400
Functions = none event_check_rooms_full
Params = 0 0 72 0
```

Param3 is global maximum size of a room in tiles. No one room may ever grow larger. It will sell traps/doors to free
space.

#### Payday

```
[event14]
Name = EVENT PAY DAY
Mnemonic = PayDay1
Values = 0 12 1195
Functions = event_check_payday none
Params = 0 0 0 0
```

If AI lack of money to pay its minions it will do one of following:

* if there is no such task yet it will create a task to sell doors/traps up to 1.5 of lacking money
* otherwise it will create a task to grab some money laying around up to 1.5 of **total wage**

#### Battle test

```
[event9]
Name = EVENT FIGHT TEST
Mnemonic = FghTest
Values = 1 1 10
Functions = none event_battle_test
Params = 75 1 0 0
```

* This event create a task to move up to `Param1/100 * AvailiableCreatures` to combat.
* Then it raises a CTA banner if possible

#### Check fighters

```
[event10]
Name = EVENT CHECK FIGHTERS
Mnemonic = FghtrChk
Values = 1 1 100
Functions = none event_check_fighters
Params = 5 0 0 0
```

This event create a task to cast haste/protect on creatures.

Param1 is power level

#### Cast offensive spells

```
[event11]
Name = EVENT MAGIC FOE
Mnemonic = MagcFoe
Values = 2 2 100
Functions = none event_attack_magic_foe
Params = 1 5 0 0
```

#### Protect own imps

```
[event13]
Name = EVENT SAVE IMPS
Mnemonic = SaveImp
Values = 4 4 40
Functions = none event_check_imps_danger
Params = 0 0 0 0
```

Drag imps from combat. Conditions are one of:

* less than 50% HP
* combat VS non Imp

Also this Check may cast Conceal with 1/150 chance

# Task

Task is lowest level part of an AI logic.

Task is holding context of some AI action i.e. "build that room with following size there". Also task MAY contain
reference to Process that it belongs. Some tasks have no such reference (i.e. spawned by a script). Each action by AI
should be performed as a task to prevent collisions.

Tasks are mutating themselves so "build a Treasure room" proceed following stages:

* CTT_DigRoomPassage
* (optionally) CTT_WaitForBridge
* CTT_DigRoom
* CTT_CheckRoomDug
* CTT_PlaceRoom

All task functions:

* task_dig_room_passage - digging a passage to build a room
* task_dig_room - digging space for room
* task_check_room_dug - waiting for imps to actually dug the room
* task_place_room - building room tiles
* task_dig_to_entrance -
* task_dig_to_gold -
* task_dig_to_attack -
* task_magic_call_to_arms -
* task_pickup_for_attack -
* task_move_creature_to_room -
* task_move_creature_to_pos -
* task_move_creatures_to_defend -
* task_slap_imps -
* task_dig_to_neutral -
* task_magic_speed_up -
* task_wait_for_bridge - waiting for a bridge \
  AI need to build across lava/water and have no bridge or no money
* task_attack_magic -
* task_sell_traps_and_doors -
* task_move_gold_to_treasury -

