-- Might be worth having a "hub" map at the start of the game, which perhaps lets you turn unlocked stuff on/off (e.g. turn off things like alarm traps, guard posts, demon spawn so it's easier later)
-- Could also be a useful way to check which levels are complete and which aren't (unless we are able to do this on the overworld map screen with a code change)
-- Could also allow for things like unlocking a small pool of creatures you can transfer to whichever next level, or a pool of single-use specials you can somehow send to the next level.

-- Every check must have a unique integer ID associated with it.
ChecksTable = {
--CREATURES --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------    
     [1]  = {id=1,   internal_name="FLY",                 name="Fly",                          string="264",       text="Attract Fly"},           --unlocked from start in default settings
     [2]  = {id=2,   internal_name="BUG",                 name="Beetle",                       string="260",       text="Attract Beetle"},        --unlocked from start in default settings
     [3]  = {id=3,   internal_name="SPIDER",              name="Spider",                       string="265",       text="Attract Spider"},        --unlocked from start in default settings
     [4]  = {id=4,   internal_name="DEMONSPAWN",          name="Demon Spawn",                  string="262",       text="Attract Demon Spawn"},   --unlocked from start in default settings
     [5]  = {id=5,   internal_name="SORCEROR",            name="Warlock",                      string="263",       text="Attract Warlock"},       --unlocked from start in default settings
     [6]  = {id=6,   internal_name="TROLL",               name="Troll",                        string="261",       text="Attract Troll"},
     [7]  = {id=7,   internal_name="BILE_DEMON",          name="Bile Demon",                   string="273",       text="Attract Bile Demon"},
     [8]  = {id=8,   internal_name="ORC",                 name="Orc",                          string="278",       text="Attract Orc"},
     [9]  = {id=9,   internal_name="DARK_MISTRESS",       name="Mistress",                     string="272",       text="Attract Mistress"},
     [10] = {id=10,  internal_name="DRAGON",              name="Dragon",                       string="268",       text="Attract Dragon"},
     [11] = {id=11,  internal_name="SKELETON",            name="Skeleton",                     string="266",       text="Attract Skeleton"},      --not usually attracted from Portal but I think that's fine and adds variety
     [12] = {id=12,  internal_name="GHOST",               name="Ghost",                        string="271",       text="Attract Ghost"},         --not usually attracted from Portal but I think that's fine and adds variety
     [13] = {id=13,  internal_name="TENTACLE",            name="Tentacle",                     string="269",       text="Attract Tentacle"},
     [14] = {id=14,  internal_name="HELL_HOUND",          name="Hound",                        string="270",       text="Attract Hound"},
     [15] = {id=15,  internal_name="HORNY",               name="Horned Reaper",                string="267",       text="Attract Horned Reaper"}, --not usually attracted from Portal but I think that's fine and adds variety
     [16] = {id=16,  internal_name="VAMPIRE",             name="Vampire",                      string="274",       text="Attract Vampire"},       --not usually attracted from Portal but I think that's fine and adds variety
--   [17] = {id=17,  internal_name="DRUID",               name="Druid",                        string="1042",      text="Attract Druid"},
--   [18] = {id=18,  internal_name="MAIDEN",              name="Maiden",                       string="1045",      text="Attract Maiden"},
--   [19] = {id=19,  internal_name="",                    name="(others?)",                    string="",          text="Attract (others?)"},
-- ROOMS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    [101] = {id=101, internal_name="TREASURE",            name="Treasure Room",                string="599",       text="Treasure Room Researchable"},           --unlocked from start in default settings
    [102] = {id=102, internal_name="LAIR",                name="Lair",                         string="609",       text="Lair Researchable"},                    --unlocked from start in default settings
    [103] = {id=103, internal_name="GARDEN",              name="Hatchery",                     string="608",       text="Hatchery Researchable"},                --unlocked from start in default settings
    [104] = {id=104, internal_name="TRAINING",            name="Training Room",                string="603",       text="Training Room Researchable"},           --unlocked from start in default settings
    [105] = {id=105, internal_name="RESEARCH",            name="Library",                      string="600",       text="Library Researchable"},                 --unlocked from start in default settings
    [106] = {id=106, internal_name="BRIDGE",              name="Bridge",                       string="610",       text="Bridge Researchable"},
    [107] = {id=107, internal_name="GUARD_POST",          name="Guard Post",                   string="611",       text="Guard Post Researchable"},
    [108] = {id=108, internal_name="WORKSHOP",            name="Workshop",                     string="605",       text="Workshop Researchable"},                --fine to allow trap/door creation if you somehow get one
    [109] = {id=109, internal_name="PRISON",              name="Prison (+make skel)",          string="601",       text="Prison (+make skel) Researchable"},     --i.e. if you get one in a map you can't make Skeletons until you unlock this
    [110] = {id=110, internal_name="TORTURE",             name="Tort Cham (+make ghost)",      string="602",       text="Tort Cham (+make ghost) Researchable"}, --i.e. if you get one in a map you can't make Ghosts until you unlock this
    [111] = {id=111, internal_name="BARRACKS",            name="Barracks",                     string="607",       text="Barracks Researchable"},
    [112] = {id=112, internal_name="TEMPLE",              name="Temple (see recipes)",         string="612",       text="Temple (see recipes) Researchable"},    --fine to allow recipes if you somehow get one
    [113] = {id=113, internal_name="GRAVEYARD",           name="Graveyard (+make Vamps)",      string="606",       text="Graveyard (+make Vamps) Researchable"}, --i.e. if you get one in a map you can't make Vampires until you unlock this
    [114] = {id=114, internal_name="SCAVENGER",           name="Scavenger Room",               string="613",       text="Scavenger Room Researchable"},
-- TRAPS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    [201] = {id=201, internal_name="ALARM",               name="Alarm Trap",                   string="579",       text="Alarm Trap Manufacturable"},
    [202] = {id=202, internal_name="POISON_GAS",          name="Poison Gas Trap",              string="580",       text="Poison Gas Trap Manufacturable"},
    [203] = {id=203, internal_name="LIGHTNING",           name="Lightning Trap",               string="581",       text="Lightning Trap Manufacturable"},
    [204] = {id=204, internal_name="LAVA",                name="Lava Trap",                    string="583",       text="Lava Trap Manufacturable"},
    [205] = {id=205, internal_name="BOULDER",             name="Boulder Tap",                  string="578",       text="Boulder Tap Manufacturable"},
    [206] = {id=206, internal_name="WORD_OF_POWER",       name="WOP Trap",                     string="582",       text="WOP Trap Manufacturable"},
--  [207] = {id=207, internal_name="TNT",                 name="Demolition Trap",              string="1036",      text="Demolition Trap Manufacturable"},
--  [208] = {id=208, internal_name="SENTRY",              name="Sentry Trap",                  string="984",       text="Sentry Trap Manufacturable"},
--  [209] = {id=209, internal_name="BALLISTA",            name="Ballista Trap",                string="1039",      text="Ballista Trap Manufacturable"},
-- DOORS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    [301] = {id=301, internal_name="WOOD",                name="Wooden Door",                  string="590",       text="Wooden Door Manufacturable"},
    [302] = {id=302, internal_name="BRACED",              name="Braced Door",                  string="591",       text="Braced Door Manufacturable"},
    [303] = {id=303, internal_name="STEEL",               name="Iron Door",                    string="592",       text="Iron Door Manufacturable"},
    [304] = {id=304, internal_name="MAGIC",               name="Magic Door",                   string="593",       text="Magic Door Manufacturable"},
--  [305] = {id=305, internal_name="SECRET",              name="Secret Door",                  string="935",       text="Secret Door Manufacturable"},
--  [306] = {id=306, internal_name="MIDAS",               name="Midas Door",                   string="1076",      text="Midas Door Manufacturable"},
--SPELLS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    [401] = {id=401, internal_name="POWER_HAND",          name="Hand of Evil",                 string="961",       text="Hand of Evil Researchable"},     --unlocked from start in default settings
    [402] = {id=402, internal_name="POWER_SLAP",          name="Slap",                         string="962",       text="Slap Researchable"},             --unlocked from start in default settings
    [403] = {id=403, internal_name="POWER_POSSESS",       name="Possess Creature",             string="630",       text="Possess Creature Researchable"}, --unlocked from start in default settings
    [404] = {id=404, internal_name="POWER_IMP",           name="Create Imp",                   string="631",       text="Create Imp Researchable"},          --unlocked from start in default settings
    [405] = {id=405, internal_name="POWER_SIGHT",         name="Sight of Evil",                string="632",       text="Sight of Evil Researchable"},
    [406] = {id=406, internal_name="POWER_SPEED",         name="Speed Monster",                string="637",       text="Speed Monster Researchable"},
    [407] = {id=407, internal_name="POWER_OBEY",          name="Must Obey",                    string="636",       text="Must Obey Researchable"},
    [408] = {id=408, internal_name="POWER_CALL_TO_ARMS",  name="CTA",                          string="633",       text="CTA Researchable"},
    [409] = {id=409, internal_name="POWER_CONCEAL",       name="Conceal",                      string="639",       text="Conceal Researchable"},
    [410] = {id=410, internal_name="POWER_HOLD_AUDIENCE", name="Hold Audience",                string="634",       text="Hold Audience Researchable"},
    [411] = {id=411, internal_name="POWER_CAVE_IN",       name="Cave-In",                      string="635",       text="Cave-In Researchable"},
    [412] = {id=412, internal_name="POWER_HEAL_CREATURE", name="Heal",                         string="644",       text="Heal Researchable"},
    [413] = {id=413, internal_name="POWER_LIGHTNING",     name="Lightning Strike",             string="640",       text="Lightning Strike Researchable"},
    [414] = {id=414, internal_name="POWER_PROTECT",       name="Protect Monster",              string="638",       text="Protect Monster Researchable"},
    [415] = {id=415, internal_name="POWER_CHICKEN",       name="Chicken",                      string="641",       text="Chicken Researchable"},
    [416] = {id=416, internal_name="POWER_DISEASE",       name="Disease",                      string="642",       text="Disease Researchable"},
    [417] = {id=417, internal_name="POWER_ARMAGEDDON",    name="Armageddon",                   string="646",       text="Armageddon Researchable"},
    [418] = {id=418, internal_name="POWER_DESTROY_WALLS", name="Destroy Walls",                string="643",       text="Destroy Walls Researchable"},
--  [419] = {id=419, internal_name="POWER_TIME_BOMB",     name="Time Bomb",                    string="645",       text="Time Bomb Researchable"},
--  [420] = {id=420, internal_name="POWER_SLOW",          name="Slow",                         string="1055",      text="Slow Researchable"},
--  [421] = {id=421, internal_name="POWER_FREEZE",        name="Freeze",                       string="1054",      text="Freeze Researchable"},
--  [422] = {id=422, internal_name="POWER_REBOUND",       name="Rebound",                      string="1053",      text="Rebound Researchable"},
--  [423] = {id=423, internal_name="POWER_FLIGHT",        name="Flight",                       string="1056",      text="Flight Researchable"},
--  [424] = {id=424, internal_name="POWER_VISION",        name="Vision",                       string="1058",      text="Vision Researchable"},
--  [425] = {id=425, internal_name="POWER_TUNNELLER",     name="Recruit Tunneller",            string="1072",      text="Recruit Tunneller Researchable"},
--  [426] = {id=426, internal_name="POWER_CLEANSE",       name="Cleanse",                      string="",          text="Cleanse Researchable"},          --not made yet
--  could optionally split POWER_HAND up into POWER_PICKUP_CREATURE, POWER_PICKUP_GOLD, POWER_PICKUP_FOOD
-- LEVELS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
--    --Levels
--    --Three of these should be unlocked by default.
--    --If you assume an initial level cap of 3, a creature cap of 10, and only bugs, demonspawn and warlocks I would say definitely levels 1-4 are doable, as are 101,103-105.
--    --Maybe others too, but I think it would be extremely hard.
--   --Sphere 1, candidate for being unlocked from start:
--       1-4, 101, 103-105
--   --Sphere 2, recommended some of e.g. level 5 cap, biles/orcs/skeletons/hounds, prison, speed/cta
--       5-11
--   --Sphere 3, recommended some of  e.g. level 7 cap, mistress/dragon/vampire, prison+torture, heal
--       10-15
--   --Sphere 4, tougher, best to restrict until you have a cap of 7+, decent creatures, prison/torture/temple/graveyard, heal/speed/cta/lightning/cave-in
--       16-20
--   --Not sure:
--       100: Sphere 2/3? not sure, doable with extreme care in possession, or still pretty handily with a cap of level 7. If you have certain spells and rooms you can cheese it way earlier.
--       102: not sure, requires a way to kill imps en masse, e.g. cave-in, a transferred creature, placeable boulder traps
    [501] = {id=501, internal_name="",                    name="1",                            string="202",       text="Level 1 Unlocked"},
    [502] = {id=502, internal_name="",                    name="2",                            string="203",       text="Level 2 Unlocked"},
    [503] = {id=503, internal_name="",                    name="3",                            string="204",       text="Level 3 Unlocked"},
    [504] = {id=504, internal_name="",                    name="4",                            string="205",       text="Level 4 Unlocked"},
    [505] = {id=505, internal_name="",                    name="5",                            string="206",       text="Level 5 Unlocked"},
    [506] = {id=506, internal_name="",                    name="6",                            string="207",       text="Level 6 Unlocked"},
    [507] = {id=507, internal_name="",                    name="7",                            string="208",       text="Level 7 Unlocked"},
    [508] = {id=508, internal_name="",                    name="8",                            string="209",       text="Level 8 Unlocked"},
    [509] = {id=509, internal_name="",                    name="9",                            string="210",       text="Level 9 Unlocked"},
    [510] = {id=510, internal_name="",                    name="10",                           string="211",       text="Level 10 Unlocked"},
    [511] = {id=511, internal_name="",                    name="11",                           string="212",       text="Level 11 Unlocked"},
    [512] = {id=512, internal_name="",                    name="12",                           string="213",       text="Level 12 Unlocked"},
    [513] = {id=513, internal_name="",                    name="13",                           string="214",       text="Level 13 Unlocked"},
    [514] = {id=514, internal_name="",                    name="14",                           string="215",       text="Level 14 Unlocked"},
    [515] = {id=515, internal_name="",                    name="15",                           string="216",       text="Level 15 Unlocked"},
    [516] = {id=516, internal_name="",                    name="16",                           string="217",       text="Level 16 Unlocked"},
    [517] = {id=517, internal_name="",                    name="17",                           string="218",       text="Level 17 Unlocked"},
    [518] = {id=518, internal_name="",                    name="18",                           string="219",       text="Level 18 Unlocked"},
    [519] = {id=519, internal_name="",                    name="19",                           string="220",       text="Level 19 Unlocked"},
    [520] = {id=520, internal_name="",                    name="20",                           string="221",       text="Level 20 Unlocked"},
    [521] = {id=521, internal_name="",                    name="100",                          string="430",       text="Level 100 Unlocked"}, --string is just "Bonus"
    [522] = {id=522, internal_name="",                    name="101",                          string="430",       text="Level 101 Unlocked"}, --string is just "Bonus"
    [523] = {id=523, internal_name="",                    name="102",                          string="430",       text="Level 102 Unlocked"}, --string is just "Bonus"
    [524] = {id=524, internal_name="",                    name="103",                          string="430",       text="Level 103 Unlocked"}, --string is just "Bonus"
    [525] = {id=525, internal_name="",                    name="104",                          string="430",       text="Level 104 Unlocked"}, --string is just "Bonus"
    [526] = {id=526, internal_name="",                    name="105",                          string="430",       text="Level 105 Unlocked"}, --string is just "Bonus"
-- RECIPES --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    [601] = {id=601, internal_name="",                    name="Cheaper Imps",                 string="",       text="Cheaper Imps Recipe Unlocked"},
    [602] = {id=602, internal_name="",                    name="Complete Manufacturing",       string="",       text="Complete Manufacturing Recipe Unlocked"},
    [603] = {id=603, internal_name="",                    name="Complete Research",            string="",       text="Complete Research Recipe Unlocked"},
    [604] = {id=604, internal_name="",                    name="Bile Demon",                   string="",       text="Bile Demon Recipe Unlocked"},
    [605] = {id=605, internal_name="",                    name="Warlock",                      string="",       text="Warlock Recipe Unlocked"},
    [606] = {id=606, internal_name="",                    name="Mistress",                     string="",       text="Mistress Recipe Unlocked"},
    [607] = {id=607, internal_name="",                    name="Horned Reaper",                string="",       text="Horned Reaper Recipe Unlocked"},
--  [608] = {id=608, internal_name="",                    name="Wishing Well",                 string="",       text="Wishing Well Recipe Unlocked"}, --default, might be hardcoded, would probably be stupid to include
--  [609] = {id=609, internal_name="",                    name="All chickens die 1",           string="",       text="All chickens die 1 Recipe Unlocked"}, --default, unlock would probably be stupid to include outside of a Templesanity
--  [610] = {id=610, internal_name="",                    name="All chickens die 2",           string="",       text="All chickens die 2 Recipe Unlocked"}, -default, unlock would probably be stupid to include outside of a Templesanity
--  [611] = {id=611, internal_name="",                    name="Disease creatures",            string="",       text="Disease creatures Recipe Unlocked"}, -default, unlock would probably be stupid to include outside of a Templesanity
--  [612] = {id=612, internal_name="",                    name="All creatures angry",          string="",       text="All creatures angry Recipe Unlocked"}, -default, unlock would probably be stupid to include outside of a Templesanity
--  [613] = {id=613, internal_name="",                    name="Chicken creatures",            string="",       text="Chicken creatures Recipe Unlocked"}, -default, unlock would probably be stupid to include outside of a Templesanity
--  [614] = {id=614, internal_name="",                    name="Spider easter egg",            string="",       text="Spider easter egg Recipe Unlocked"}, --default, hardcoded easter egg and not really a recipe, would probably be stupid to include
--  [615] = {id=615, internal_name="",                    name="Good skeleton",                string="",       text="Good skeleton Recipe Unlocked"}, --default, unlock would probably be stupid to include outside of a Templesanity
--  [616] = {id=616, internal_name="",                    name="Tentacle",                     string="",       text="Tentacle Recipe Unlocked"},
--  [617] = {id=617, internal_name="",                    name="Hound",                        string="",       text="Hound Recipe Unlocked"},
--  [618] = {id=618, internal_name="",                    name="Speed creatures",              string="",       text="Speed creatures Recipe Unlocked"},
--  [619] = {id=619, internal_name="",                    name="Conceal creatures",            string="",       text="Conceal creatures Recipe Unlocked"},
--  [620] = {id=620, internal_name="",                    name="Heal creatures",               string="",       text="Heal creatures Recipe Unlocked"},
--  [621] = {id=621, internal_name="",                    name="Rebound creatures",            string="",       text="Rebound creatures Recipe Unlocked"},
--  [622] = {id=622, internal_name="",                    name="Protect creatures",            string="",       text="Protect creatures Recipe Unlocked"},
--  [623] = {id=623, internal_name="",                    name="Flight creatures",             string="",       text="Flight creatures Recipe Unlocked"},
--  [624] = {id=624, internal_name="",                    name="Freeze creatures",             string="",       text="Freeze creatures Recipe Unlocked"},
--  [625] = {id=625, internal_name="",                    name="Slow creatures",               string="",       text="Slow creatures Recipe Unlocked"},
-- PROGRESSIVES --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    [701] = {id=701, internal_name="",                    name="Progressive Level Cap 1",      string="",       text="Progressive Level Cap 1 Unlocked"},      --Increase max creature level by 1 (starts max level 3): 4
    [702] = {id=702, internal_name="",                    name="Progressive Level Cap 2",      string="",       text="Progressive Level Cap 2 Unlocked"},      --5
    [703] = {id=703, internal_name="",                    name="Progressive Level Cap 3",      string="",       text="Progressive Level Cap 3 Unlocked"},      --6
    [704] = {id=704, internal_name="",                    name="Progressive Level Cap 4",      string="",       text="Progressive Level Cap 4 Unlocked"},      --7
    [705] = {id=705, internal_name="",                    name="Progressive Level Cap 5",      string="",       text="Progressive Level Cap 5 Unlocked"},      --8
    [706] = {id=706, internal_name="",                    name="Progressive Level Cap 6",      string="",       text="Progressive Level Cap 6 Unlocked"},      --9
    [707] = {id=707, internal_name="",                    name="Progressive Level Cap 7",      string="",       text="Progressive Level Cap 7 Unlocked"},      --10 and growup
    [711] = {id=711, internal_name="",                    name="Progressive Creature Limit 1", string="",       text="Progressive Creature Limit 1 Unlocked"}, --Increase creature limit by 5 (starts at max 10): 15
    [712] = {id=712, internal_name="",                    name="Progressive Creature Limit 2", string="",       text="Progressive Creature Limit 2 Unlocked"}, --20
    [713] = {id=713, internal_name="",                    name="Progressive Creature Limit 3", string="",       text="Progressive Creature Limit 3 Unlocked"}, --25
    [714] = {id=714, internal_name="",                    name="Progressive Creature Limit 4", string="",       text="Progressive Creature Limit 4 Unlocked"}, --30
    [715] = {id=715, internal_name="",                    name="Progressive Creature Limit 5", string="",       text="Progressive Creature Limit 5 Unlocked"}, --35
    [716] = {id=716, internal_name="",                    name="Progressive Creature Limit 6", string="",       text="Progressive Creature Limit 6 Unlocked"}, --40
    [721] = {id=721, internal_name="",                    name="Progressive Starting Gold 1",  string="",       text="Progressive Starting Gold 1 Unlocked"},  --Increase starting gold by 1250 (starts at 2500): 3750
    [722] = {id=722, internal_name="",                    name="Progressive Starting Gold 2",  string="",       text="Progressive Starting Gold 2 Unlocked"},  --5000
    [723] = {id=723, internal_name="",                    name="Progressive Starting Gold 3",  string="",       text="Progressive Starting Gold 3 Unlocked"},  --6250
    [724] = {id=724, internal_name="",                    name="Progressive Starting Gold 4",  string="",       text="Progressive Starting Gold 4 Unlocked"},  --7500
    [725] = {id=725, internal_name="",                    name="Progressive Starting Gold 5",  string="",       text="Progressive Starting Gold 5 Unlocked"},  --8750
    [726] = {id=726, internal_name="",                    name="Progressive Starting Gold 6",  string="",       text="Progressive Starting Gold 6 Unlocked"},  --10000
}
--    --Others e.g. progressive starting imps number/level, progressive auto-researched (e.g. at 1, bridge/guardpost and SOE are unlocked, at 2, workshop and speed are unlocked and so on (IF THOSE ARE UNLOCKED)},
--    --progressive auto-manufacturing (at 1, you get an alarm/gas trap and wooden door at start, at 2 you get a lightning trap and braced door, at 3 you get WOP trap and iron door, at 4 you get lava/boulder and magic door (IF THOSE ARE UNLOCKED))


--    -----------------------------------------------------------
--    --Filler
--    --okay I feel like a lot of the stuff in this game could be considered filler, like you could beat the whole game without using traps or doors, or half the rooms or spells or creatures, but yeah
--
--    --Do we want temporary things? I know Doom has powerups as filler, we could do the same with the normal specials.
--    --Maybe they'd have to be spawned in on your heart when unlocked or on map start (would you have a way to hold on to them until later?)
--    --Increase Level (x10)
--    --Make Safe (x10)
--    --Multiply Creatures (x2)
--    --Resurrect Creature (x10)
--    --Reveal Map (x5)
--    --Steal Hero (x5)
--    --Transfer Creature (x5)
--    --the bonus specials e.g. Increase Gold and Heal All
--
--    --Would there be a way to transfer creatures?
--
--    --Traps (i.e. bad AP unlocks)
--    --Negative Temple recipes cast on you
--    --Creatures are debuffed
--    --Some creatures turn white (turncoat)
--    --Creatures die
--    --Imps die
--    --Lose gold
--    --Spammed with taunts
--    --player colours are shuffled around

function ReceivedLocations.ReceivedItemCheck(itemid)
      if itemid >= 1 and itemid <= 100 then
            UnlockCreature(itemid)
      elseif itemid > 100 and itemid <= 200 then
            UnlockRoom(itemid)
      elseif itemid > 200 and itemid <= 300 then
            UnlockTrap(itemid)
      elseif itemid > 300 and itemid <= 400 then
            UnlockDoor(itemid)
      elseif itemid > 400 and itemid <= 500 then
            UnlockSpell(itemid)
      --elseif itemid > 500 and itemid <= 600 then
      --      UnlockLevel(itemid)
      --elseif itemid > 600 and itemid <= 700 then
      --      UnlockRecipe(itemid)
      --elseif itemid > 700 and itemid <= 800 then
      --    UnlockProgressive(itemid)
      --don't think these work this way.
      --elseif itemid > 800 and itemid <= 900 then
      --    UnlockFiller(itemid)
      --elseif itemid > 900 and itemid <= 1000 then
      --    UnlockTrap(itemid)
      else
            print("Unknown item ID " .. itemid)
      end
end

function UnlockCreature(itemid)
      print("Creature " .. itemid .. "( " .. ChecksTable[itemid].name .. ") Unlocked")
      CreatureAvailable("PLAYER0",ChecksTable[itemid].internal_name,true,0)
end
function UnlockRoom(itemid)
      print("Room " .. itemid .. "( " .. ChecksTable[itemid].name .. ") Unlocked")
      RoomAvailable("PLAYER0",ChecksTable[itemid].internal_name,2,true)
end
function UnlockTrap(itemid)
      print("Trap " .. itemid .. "( " .. ChecksTable[itemid].name .. ") Unlocked")
      TrapAvailable("PLAYER0",ChecksTable[itemid].internal_name,true,0)
end
function UnlockDoor(itemid)
      print("Door " .. itemid .. "( " .. ChecksTable[itemid].name .. ") Unlocked")
      DoorAvailable("PLAYER0",ChecksTable[itemid].internal_name,true,0)
end
function UnlockSpell(itemid)
      print("Spell " .. itemid .. "( " .. ChecksTable[itemid].name .. ") Unlocked")
      MagicAvailable("PLAYER0",ChecksTable[itemid].internal_name,true,0)
end

--Level
--function UnlockLevel(itemid)
--
--end

--Recipe
--function UnlockRecipe(itemid)
--
--end

--Progressive

--need to write these.
function UnlockProgressive(itemid)
      if itemid >= 701 and itemid <= 707 then
            IncreaseLevelCap()
      elseif itemid >= 711 and itemid <= 716 then
            IncreaseCreatureLimit()
      elseif itemid >= 721 and itemid <= 726 then
            IncreaseStartingGold()
      end
end