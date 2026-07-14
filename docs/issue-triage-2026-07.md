# Open-issue triage — 2026-07-14

A full classification of every open issue against this repository's existing `Component-*`, `Type-*`, and `Priority-*` labels, intended as a starting point for clearing the backlog. It was generated with **read-only access**, so nothing here is applied — a maintainer with triage rights would review and bulk-apply.

## Snapshot

- **411** open issues, all classified.
- **140** (34%) carried no labels at all; every gap is now filled with a proposed Component / Type / Priority.
- **281** have had no activity in over a year.
- **58** are flagged **closeable?** (advisory): a can't-reproduce / duplicate / verify status, or a >2yr enhancement request with zero discussion.
- **12** are **low-confidence** — the keyword rules were ambiguous; review these by hand first (marked ⚠ below).

## Method

Issue titles and bodies were matched against the repo's label vocabulary with deterministic keyword rules. Any label an issue already carried was **preserved**; only missing dimensions were inferred (inferred components are marked `*`). Priority rubric: **Critical** = crash / save-loss / hard blocker · **High** = broken core gameplay (pathfinding, computer player, combat) · **Medium** = noticeable but has a workaround · **Low** = cosmetic / niche / aged request. An interactive, sortable version of this same data is available as a dashboard (link in the PR description).

## Distribution

**By type**

| Value | Count |
|---|---:|
| Defect | 191 |
| Enhancement | 130 |
| Other | 86 |
| Task | 3 |
| Review | 1 |

**By priority**

| Value | Count |
|---|---:|
| Critical | 7 |
| High | 23 |
| Medium | 227 |
| Low | 154 |

## Issues by component

Legend: `*` = component inferred (was unlabeled) · ⚠ = low confidence, review first · 🗓 = stale >1yr · 🗑 = flagged closeable.

### Creatures (105)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4758](https://github.com/dkfans/keeperfx/issues/4758) | Fix infinite loop when sweeping creatures group | Defect | Critical * |  |
| [#3158](https://github.com/dkfans/keeperfx/issues/3158) | Additional Trap Properties and functions (Issue + request) | Enhancement | Critical * |  |
| [#4543](https://github.com/dkfans/keeperfx/issues/4543) | Custom Creature sounds | Defect | High * |  |
| [#3420](https://github.com/dkfans/keeperfx/issues/3420) | Keyboard shortcut to cycle creatures being scavenged by enemy | Defect | High * | 🗓 |
| [#3275](https://github.com/dkfans/keeperfx/issues/3275) | Incorrect creature combat behavior | Defect | High * |  |
| [#3166](https://github.com/dkfans/keeperfx/issues/3166) | Creature stuttering when walking towards combat | Defect | High * |  |
| [#2421](https://github.com/dkfans/keeperfx/issues/2421) | Ranged units back themselves into rock then refuse to attack | Defect | High * | 🗓 |
| [#1871](https://github.com/dkfans/keeperfx/issues/1871) | Ranged units will hit corners because they don't consider the projectile width | Defect | High * | 🗓 |
| [#697](https://github.com/dkfans/keeperfx/issues/697) | Computer player builds replacement imps before they are all sacrificed | Defect | High * | 🗓 |
| [#693](https://github.com/dkfans/keeperfx/issues/693) | Computer Player should mine efficiently from gems | Enhancement | High * | 🗓 |
| [#4837](https://github.com/dkfans/keeperfx/issues/4837) | Creatures overusing religion as an excuse to skip their war duties. | Other | Medium * |  |
| [#4827](https://github.com/dkfans/keeperfx/issues/4827) | Enemy prisoners leave the dungeon if dropped on the portal. | Other | Medium * |  |
| [#4527](https://github.com/dkfans/keeperfx/issues/4527) | Changing the Room roles | Enhancement | Medium * |  |
| [#4171](https://github.com/dkfans/keeperfx/issues/4171) | Creature with ranged preference and flame breath will not move | Other | Medium * |  |
| [#3828](https://github.com/dkfans/keeperfx/issues/3828) | Secret door cheese | Other | Medium * | 🗓 |
| [#3777](https://github.com/dkfans/keeperfx/issues/3777) | Variable for total creatures left in pool without considering attraction requirements | Defect | Medium * | 🗓 |
| [#3722](https://github.com/dkfans/keeperfx/issues/3722) | Working creatures can damage allied players. | Other | Medium * | 🗓 |
| [#3680](https://github.com/dkfans/keeperfx/issues/3680) | NEW JOBS/ACTIVITIES FOR CREATURES | Other | Medium * | 🗓 |
| [#3647](https://github.com/dkfans/keeperfx/issues/3647) | NEW GROUPS/TYPES FOR TILES | Enhancement | Medium * | 🗓 |
| [#3643](https://github.com/dkfans/keeperfx/issues/3643) | Alt (or other key) to reveal health petals of all creatures on screen | Enhancement | Medium * |  |
| [#3634](https://github.com/dkfans/keeperfx/issues/3634) | TRANSFORMATION OF ONE CREATURE INTO ANOTHER | Other | Medium * | 🗓 |
| [#3633](https://github.com/dkfans/keeperfx/issues/3633) | SECRET PASSAGE or SECRET DOORS  can only be discovered by SELECTED CREATURES | Other | Medium * | 🗓 |
| [#3579](https://github.com/dkfans/keeperfx/issues/3579) | ADD and REMOVE "Creatures Propertie"s to be able to get hooks in scripts. | Enhancement | Medium * |  |
| [#3575](https://github.com/dkfans/keeperfx/issues/3575) | Tentacles don't defect | Defect | Medium * | 🗑 |
| [#3532](https://github.com/dkfans/keeperfx/issues/3532) | Animation glitches out when Bile Demon completes a trap or door in the workshop when sped up (speed/slap/must obey) | Other | Medium * | 🗓 |
| [#3454](https://github.com/dkfans/keeperfx/issues/3454) | Ranged minions has trouble to take their distance when attacking secret doors just in a corner | Defect | Medium * | 🗓 |
| [#3437](https://github.com/dkfans/keeperfx/issues/3437) | Creature count not displayed when string too long | Enhancement | Medium * |  |
| [#3427](https://github.com/dkfans/keeperfx/issues/3427) | Imps get stuck and kill prisoners on the way to prison | Defect | Medium * | 🗓 |
| [#3412](https://github.com/dkfans/keeperfx/issues/3412) | Add room size indicator for guard room | Enhancement | Medium * | 🗓 |
| [#3368](https://github.com/dkfans/keeperfx/issues/3368) | Prisoners instantly converting to skeletons when imprisoned. | Other | Medium * | 🗓 🗑 |
| [#3236](https://github.com/dkfans/keeperfx/issues/3236) | Creatures sometimes not taking damage from poison cloud / fart when hitting doors | Other | Medium * | 🗓 |
| [#3169](https://github.com/dkfans/keeperfx/issues/3169) | HAIL type Projectile not impacting | Defect | Medium * | 🗓 🗑 |
| [#3131](https://github.com/dkfans/keeperfx/issues/3131) | Creatures keep fighting after alliances have changed. | Other | Medium * | 🗓 |
| [#3127](https://github.com/dkfans/keeperfx/issues/3127) | Status flower jumps up and down when there is 2 different statusses | Other | Medium * |  |
| [#2983](https://github.com/dkfans/keeperfx/issues/2983) | Clear up confusion between DestroyOnHit an WithstandHitAgainst | Other | Medium * | 🗓 |
| [#2608](https://github.com/dkfans/keeperfx/issues/2608) | 'Sacrifice imps' should be separate computer check | Enhancement | Medium * | 🗓 🗑 |
| [#2532](https://github.com/dkfans/keeperfx/issues/2532) | Creatures are marked as in combat in the Battle Window when they are not. | Other | Medium * |  |
| [#2530](https://github.com/dkfans/keeperfx/issues/2530) | Drain/Lightning does not push unconscious creatures from heart | Defect | Medium * |  |
| [#2516](https://github.com/dkfans/keeperfx/issues/2516) | Map script executes before pre-placed creatures are countable | Other | Medium * | 🗓 |
| [#2451](https://github.com/dkfans/keeperfx/issues/2451) | USE_POWER_ON_CREATURE with criterion AT_ACTION_POINT should exclude stunned/captured creatures | Other | Medium * | 🗓 |
| [#2430](https://github.com/dkfans/keeperfx/issues/2430) | Enemy creatures attack doors in the middle of a fight | Other | Medium * | 🗓 |
| [#2361](https://github.com/dkfans/keeperfx/issues/2361) | Train imps to level 3 - Computer Assistance | Other | Medium * | 🗓 |
| [#2318](https://github.com/dkfans/keeperfx/issues/2318) | Getting turned into a chicken while possessing a creature let you still use your primary attack | Other | Medium * | 🗓 |
| [#2186](https://github.com/dkfans/keeperfx/issues/2186) | AT_ACTION_POINT[#] criterion does not target all creatures in range | Defect | Medium * | 🗓 |
| [#1916](https://github.com/dkfans/keeperfx/issues/1916) | Imp task behavior worse than before | Defect | Medium * | 🗓 |
| [#1298](https://github.com/dkfans/keeperfx/issues/1298) | Unable to pick up imp fighting door in tunnel | Defect | Medium * | 🗓 |
| [#1067](https://github.com/dkfans/keeperfx/issues/1067) | Game crashes with specific screen resolutions | Defect | Medium * | 🗓 |
| [#750](https://github.com/dkfans/keeperfx/issues/750) | Torture chamber issue | Defect | Medium * | 🗓 🗑 |
| [#727](https://github.com/dkfans/keeperfx/issues/727) | Imps refuse to pick up small piles of gold | Defect | Medium * | 🗓 |
| [#687](https://github.com/dkfans/keeperfx/issues/687) | Computer players don't build enough imps | Defect | Medium * | 🗓 |
| [#682](https://github.com/dkfans/keeperfx/issues/682) | Computer players don't pick up creatures that can't reach the rooms they need | Defect | Medium * | 🗓 |
| [#679](https://github.com/dkfans/keeperfx/issues/679) | Imps stuck trying to go through reinforced corner (stable) | Defect | Medium * | 🗓 |
| [#657](https://github.com/dkfans/keeperfx/issues/657) | Ghosts keep getting stuck in ceiling | Defect | Medium |  |
| [#643](https://github.com/dkfans/keeperfx/issues/643) | Computer player attacking too soon | Defect | Medium * | 🗓 |
| [#615](https://github.com/dkfans/keeperfx/issues/615) | Vampires stuck getting gold | Defect | Medium * | 🗓 |
| [#585](https://github.com/dkfans/keeperfx/issues/585) | Ranged creatures have troubles attacking creatures who can't fight back (lava, etc) | Defect | Medium | 🗓 |
| [#529](https://github.com/dkfans/keeperfx/issues/529) | Broke CP to sell rooms to build imps | Enhancement | Medium * | 🗓 🗑 |
| [#496](https://github.com/dkfans/keeperfx/issues/496) | Bridge claimed in multiple segments | Defect | Medium * | 🗓 |
| [#489](https://github.com/dkfans/keeperfx/issues/489) | Must obey causes damaged creatures to stay awake | Defect | Medium * | 🗓 |
| [#484](https://github.com/dkfans/keeperfx/issues/484) | Groups of idle imps wander the map | Defect | Medium * | 🗓 |
| [#476](https://github.com/dkfans/keeperfx/issues/476) | Fighting occasionally bugs out | Defect | Medium | 🗓 |
| [#450](https://github.com/dkfans/keeperfx/issues/450) | Creatures with Teleport wander around | Defect | Medium * | 🗓 |
| [#430](https://github.com/dkfans/keeperfx/issues/430) | Improved Creature Properties | Enhancement | Medium * | 🗓 |
| [#405](https://github.com/dkfans/keeperfx/issues/405) | Imps attack enemy when dungeon heart behind locked door | Defect | Medium * | 🗓 |
| [#317](https://github.com/dkfans/keeperfx/issues/317) | Vampires teleport out of locked rooms | Defect | Medium * | 🗓 |
| [#269](https://github.com/dkfans/keeperfx/issues/269) | Creatures assigned to a guard post go to a random guard post after eating | Defect | Medium * | 🗓 |
| [#266](https://github.com/dkfans/keeperfx/issues/266) | Escaped and woken up prisoners do not attack | Defect | Medium * | 🗓 |
| [#248](https://github.com/dkfans/keeperfx/issues/248) | Creatures getting stuck in the middle of nowhere | Defect | Medium * | 🗓 |
| [#223](https://github.com/dkfans/keeperfx/issues/223) | Flies fighting against doors until death to starvation | Defect | Medium | 🗓 |
| [#192](https://github.com/dkfans/keeperfx/issues/192) | Imps should drop gold when they walk over treasure room. | Enhancement | Medium * | 🗓 🗑 |
| [#1](https://github.com/dkfans/keeperfx/issues/1) | Dropping creatures with backspace may do bad | Defect | Medium * | 🗓 |
| [#4816](https://github.com/dkfans/keeperfx/issues/4816) | Allow prisoner trading to allies in multiplayer games. | Enhancement | Low * |  |
| [#3681](https://github.com/dkfans/keeperfx/issues/3681) | CREATURES CAN KICK CORPSES/STUNS | Enhancement | Low * |  |
| [#3624](https://github.com/dkfans/keeperfx/issues/3624) | Creature Spell STOPS TIME | Other | Low * |  |
| [#3619](https://github.com/dkfans/keeperfx/issues/3619) | Creature behaviors and improvements | Enhancement | Low * | 🗓 |
| [#3414](https://github.com/dkfans/keeperfx/issues/3414) | Reduce 'no access to library, treasure' message frequency to below spam levels. | Enhancement | Low * |  |
| [#3165](https://github.com/dkfans/keeperfx/issues/3165) | Mod option to have creatures regen health | Enhancement | Low * |  |
| [#3163](https://github.com/dkfans/keeperfx/issues/3163) | Bigger sacrifice recipes, mod option to show what is sacrificed | Enhancement | Low * |  |
| [#3161](https://github.com/dkfans/keeperfx/issues/3161) | New HitTypes to hit everything but the caster creature | Enhancement | Low * | 🗓 |
| [#3160](https://github.com/dkfans/keeperfx/issues/3160) | Additional [SHOT] properties: Shot create Shot, shot origin, more... | Enhancement | Low * | 🗓 |
| [#3157](https://github.com/dkfans/keeperfx/issues/3157) | Line of sight check | Defect | Low * | 🗓 |
| [#3156](https://github.com/dkfans/keeperfx/issues/3156) | Creatures dont group (while sleeping) | Defect | Low * | 🗓 |
| [#3154](https://github.com/dkfans/keeperfx/issues/3154) | Shots with speeds over 255 aim too high | Defect | Low * |  |
| [#3145](https://github.com/dkfans/keeperfx/issues/3145) | Enabling 'Order creature mode' cheat while in possession breaks the interface | Other | Low * |  |
| [#2993](https://github.com/dkfans/keeperfx/issues/2993) | Invisible cubes - Invisible wall - Block creatures from walking somewhere | Enhancement | Low * | 🗓 🗑 |
| [#2919](https://github.com/dkfans/keeperfx/issues/2919) | room Creaturetype blacklist/whitelist | Enhancement | Low * | 🗓 🗑 |
| [#2866](https://github.com/dkfans/keeperfx/issues/2866) | Computer could pickup fleeing imps | Enhancement | Low * | 🗓 🗑 |
| [#2825](https://github.com/dkfans/keeperfx/issues/2825) | Fleeing units should teleport away | Enhancement | Low * | 🗓 |
| [#2790](https://github.com/dkfans/keeperfx/issues/2790) | Display a notification when you successfully scavenge a creature. | Enhancement | Low * | 🗓 |
| [#2746](https://github.com/dkfans/keeperfx/issues/2746) | Add a thought bubble when a creature trained to its max level in a training room | Enhancement | Low * | 🗓 |
| [#2729](https://github.com/dkfans/keeperfx/issues/2729) | Add a thought bubble when a creature does not want to do a job | Enhancement | Low * | 🗓 |
| [#2521](https://github.com/dkfans/keeperfx/issues/2521) | Creatures can't walk/fly through custom "tunnels" if you go 1 cube higher than the floor cube | Defect | Low * | 🗓 |
| [#2404](https://github.com/dkfans/keeperfx/issues/2404) | Researching Creatures? | Enhancement | Low * | 🗓 |
| [#2190](https://github.com/dkfans/keeperfx/issues/2190) | [Bug] creature skill "cooldown bars" don't look the same as they do in the original/GoG release | Defect | Low * | 🗓 |
| [#1789](https://github.com/dkfans/keeperfx/issues/1789) | Creatures attempt to eat non-food things in Hatcheries | Defect | Low | 🗓 |
| [#1272](https://github.com/dkfans/keeperfx/issues/1272) | Ranged units sometimes shoot at Dungeon Heart pillars instead of the Heart itself | Defect | Low * | 🗓 |
| [#795](https://github.com/dkfans/keeperfx/issues/795) | Running through corners | Defect | Low * | 🗓 |
| [#784](https://github.com/dkfans/keeperfx/issues/784) | Imps will never dig through own fortified walls when assigned to gems | Defect | Low * | 🗓 |
| [#743](https://github.com/dkfans/keeperfx/issues/743) | Create automated Human Player parties via Barracks | Enhancement | Low * | 🗓 |
| [#717](https://github.com/dkfans/keeperfx/issues/717) | Imps walk to other side of wall to reinforce corner | Enhancement | Low * | 🗓 |
| [#695](https://github.com/dkfans/keeperfx/issues/695) | Computer player drops creatures on wrong side of rock for fight | Defect | Low * | 🗓 |
| [#680](https://github.com/dkfans/keeperfx/issues/680) | Imps don't claim tiles and reinforce walls in disconnected dungeon (stable) | Defect | Low * | 🗓 |
| [#464](https://github.com/dkfans/keeperfx/issues/464) | Untrained annoyance will prevent training | Defect | Low * | 🗓 |
| [#300](https://github.com/dkfans/keeperfx/issues/300) | Computer player to place creatures in barracks | Enhancement | Low | 🗓 🗑 |
| [#237](https://github.com/dkfans/keeperfx/issues/237) | heroes getting stuck in hero´s gate | Defect | Low | 🗓 |

### Maps (51)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4517](https://github.com/dkfans/keeperfx/issues/4517) | Game hangs on exit and level completion | Defect | Critical * |  |
| [#4597](https://github.com/dkfans/keeperfx/issues/4597) | Softlock on low-res mode | Defect | High * |  |
| [#1840](https://github.com/dkfans/keeperfx/issues/1840) | Effect line hangs the game through log file | Defect | High * | 🗓 |
| [#5022](https://github.com/dkfans/keeperfx/issues/5022) | Lava Rock looks bad on old maps, especially snow | Defect | Medium * |  |
| [#4996](https://github.com/dkfans/keeperfx/issues/4996) | Custom music does not switch between maps | Defect | Medium * |  |
| [#4817](https://github.com/dkfans/keeperfx/issues/4817) | Deposit gold directly into allies treasury. | Defect | Medium * |  |
| [#3319](https://github.com/dkfans/keeperfx/issues/3319) | Hidden liquids show wobbly edges when marked while wobbly walls are disabled | Other | Medium * | 🗓 |
| [#3239](https://github.com/dkfans/keeperfx/issues/3239) | Boulders sometimes not touching the ground | Other | Medium * | 🗓 |
| [#3233](https://github.com/dkfans/keeperfx/issues/3233) | Extreme slowdowns | Other | Medium * | 🗓 |
| [#2926](https://github.com/dkfans/keeperfx/issues/2926) | NG+13 Buffy Oak computers often attack too soon | Other | Medium * | 🗓 |
| [#2758](https://github.com/dkfans/keeperfx/issues/2758) | Changing slab through script does not update corner slabs | Defect | Medium * | 🗓 |
| [#2611](https://github.com/dkfans/keeperfx/issues/2611) | first subtile row and column are not displayed | Other | Medium * | 🗓 |
| [#2440](https://github.com/dkfans/keeperfx/issues/2440) | Secret level 1 too hard because of WoP trap rebalance | Other | Medium * | 🗓 |
| [#2291](https://github.com/dkfans/keeperfx/issues/2291) | Heart disappear on large maps | Other | Medium * | 🗓 |
| [#1633](https://github.com/dkfans/keeperfx/issues/1633) | Enemies revealed in frail rock columns | Defect | Medium * | 🗓 |
| [#828](https://github.com/dkfans/keeperfx/issues/828) | One-Click-Build: Slab missing for room-suggestion in F-shaped room | Defect | Medium * | 🗓 |
| [#808](https://github.com/dkfans/keeperfx/issues/808) | Units on the edge of lava are not attacked | Defect | Medium * | 🗓 |
| [#600](https://github.com/dkfans/keeperfx/issues/600) | Lost map for no reason | Defect | Medium * | 🗓 🗑 |
| [#502](https://github.com/dkfans/keeperfx/issues/502) | Campaigns aren't listed as beaten | Defect | Medium * | 🗓 |
| [#449](https://github.com/dkfans/keeperfx/issues/449) | Tunnellers unable to tunnel around lava | Enhancement | Medium * | 🗓 |
| [#388](https://github.com/dkfans/keeperfx/issues/388) | Bonus levels that are only available for a limited time | Enhancement | Medium * | 🗓 🗑 |
| [#387](https://github.com/dkfans/keeperfx/issues/387) | 'Transfer Spellbook' level special | Enhancement | Medium * | 🗓 |
| [#371](https://github.com/dkfans/keeperfx/issues/371) | Tunneler party stops at locked doors. | Defect | Medium * | 🗓 |
| [#292](https://github.com/dkfans/keeperfx/issues/292) | Several dungeon hearts not spawning | Defect | Medium | 🗓 |
| [#166](https://github.com/dkfans/keeperfx/issues/166) | about level 15.Woodly Rhyme | Defect | Medium | 🗓 |
| [#3924](https://github.com/dkfans/keeperfx/issues/3924) | Allow -campaign parameter to start a campaign instead of only a single level | Enhancement | Low * |  |
| [#3831](https://github.com/dkfans/keeperfx/issues/3831) | Decorative objects burst into gas when destroyed by lava. | Defect | Low * | 🗓 |
| [#3525](https://github.com/dkfans/keeperfx/issues/3525) | Separate continue slots per campaign or per new game, the ability to save in landview | Enhancement | Low * | 🗓 |
| [#3399](https://github.com/dkfans/keeperfx/issues/3399) | Optional low walls slab | Enhancement | Low * | 🗓 |
| [#3191](https://github.com/dkfans/keeperfx/issues/3191) | Individually translatable map names | Defect | Low |  |
| [#3187](https://github.com/dkfans/keeperfx/issues/3187) | Switching language during a level does not affect current objectives and information messages | Defect | Low |  |
| [#2992](https://github.com/dkfans/keeperfx/issues/2992) | Map packs look for credits in landview folder | Defect | Low * |  |
| [#2710](https://github.com/dkfans/keeperfx/issues/2710) | Swipes on instance level instead of creature level | Enhancement | Low * | 🗓 |
| [#2534](https://github.com/dkfans/keeperfx/issues/2534) | Map specific custom music | Enhancement | Low * | 🗓 🗑 |
| [#2529](https://github.com/dkfans/keeperfx/issues/2529) | Ideas for Custom Door Implementation | Enhancement | Low * | 🗓 🗑 |
| [#2490](https://github.com/dkfans/keeperfx/issues/2490) | Allow images as campaign intro/outro in place of videos (or support better video formats). | Enhancement | Low * | 🗓 |
| [#2487](https://github.com/dkfans/keeperfx/issues/2487) | Campaign Intro | Enhancement | Low * | 🗓 |
| [#2221](https://github.com/dkfans/keeperfx/issues/2221) | [Feature] When selecting a point on the map, restore the camera angle | Enhancement | Low * | 🗓 |
| [#2192](https://github.com/dkfans/keeperfx/issues/2192) | CREATE_EFFECTS_LINE 'speed' param does not work as described | Defect | Low * | 🗓 |
| [#2139](https://github.com/dkfans/keeperfx/issues/2139) | Feature: One-click gold mining | Enhancement | Low * | 🗓 🗑 |
| [#1831](https://github.com/dkfans/keeperfx/issues/1831) | Switching between perspective modes should keep zoom level | Enhancement | Low * | 🗓 🗑 |
| [#1551](https://github.com/dkfans/keeperfx/issues/1551) | Transfer creature spell boxes work before you win the level | Defect | Low * | 🗓 |
| [#1124](https://github.com/dkfans/keeperfx/issues/1124) | Store maps inside zip files with all their parts. | Enhancement | Low * | 🗓 |
| [#1081](https://github.com/dkfans/keeperfx/issues/1081) | Implement "growing" cubes | Enhancement | Low * | 🗓 🗑 |
| [#897](https://github.com/dkfans/keeperfx/issues/897) | New Slab: Finite gems | Enhancement | Low * | 🗓 |
| [#896](https://github.com/dkfans/keeperfx/issues/896) | New Slab: Soft Eart, Padded Earth | Enhancement | Low * | 🗓 🗑 |
| [#545](https://github.com/dkfans/keeperfx/issues/545) | 'BLOCKS_ROOM_PLACE'  flag | Defect | Low * | 🗓 🗑 |
| [#540](https://github.com/dkfans/keeperfx/issues/540) | 'Reveal map' does not reveal gold locations | Defect | Low * | 🗓 🗑 |
| [#501](https://github.com/dkfans/keeperfx/issues/501) | High score list doesn't always show up when level beaten | Defect | Low * | 🗓 🗑 |
| [#468](https://github.com/dkfans/keeperfx/issues/468) | Things on the map are there to stay | Defect | Low * | 🗓 |
| [#6](https://github.com/dkfans/keeperfx/issues/6) | Sub dungeons could work better | Defect | Low * | 🗓 |

### UI (38)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4957](https://github.com/dkfans/keeperfx/issues/4957) | Opening the parchment map view can cause massive FPS drops | Defect | Medium * |  |
| [#4901](https://github.com/dkfans/keeperfx/issues/4901) | Text window in Landview | Enhancement | Medium * |  |
| [#4808](https://github.com/dkfans/keeperfx/issues/4808) | Visualization of simultaneous variables | Other | Medium |  |
| [#4694](https://github.com/dkfans/keeperfx/issues/4694) | Add text/icon to the “DISPLAY_VARIABLE” label | Enhancement | Medium * |  |
| [#4556](https://github.com/dkfans/keeperfx/issues/4556) | Displaying an image or text box during a game. | Enhancement | Medium * |  |
| [#4452](https://github.com/dkfans/keeperfx/issues/4452) | Dungeon diggers | Other | Medium * |  |
| [#3580](https://github.com/dkfans/keeperfx/issues/3580) | Minimap briefly flashes pink on level start | Other | Medium * |  |
| [#3088](https://github.com/dkfans/keeperfx/issues/3088) | Duplicate battles in battle menu | Other | Medium * |  |
| [#3059](https://github.com/dkfans/keeperfx/issues/3059) | Japanese menu title texts are too far to the right | Other | Medium |  |
| [#3058](https://github.com/dkfans/keeperfx/issues/3058) | Buttons get lost from sidebar | Defect | Medium * |  |
| [#2399](https://github.com/dkfans/keeperfx/issues/2399) | Low-res displays high-res icons for Druid and Time Mage in the hand | Defect | Medium * | 🗓 |
| [#2160](https://github.com/dkfans/keeperfx/issues/2160) | Hangs on main menu - Frontend state 34 unknown | Defect | Medium * | 🗓 |
| [#2115](https://github.com/dkfans/keeperfx/issues/2115) | No credits when there's no recognised current campaign | Defect | Medium | 🗓 |
| [#1914](https://github.com/dkfans/keeperfx/issues/1914) | Hero party stopped attacking and gone from minimap | Defect | Medium * | 🗓 |
| [#1090](https://github.com/dkfans/keeperfx/issues/1090) | Additional settings will break compatibility with old ones | Other | Medium * | 🗓 |
| [#995](https://github.com/dkfans/keeperfx/issues/995) | Missing Anti-Aliasing | Defect | Medium * | 🗓 |
| [#738](https://github.com/dkfans/keeperfx/issues/738) | Content of creature panel disappears when it becomes new type of creature while possesing it | Defect | Medium | 🗓 |
| [#571](https://github.com/dkfans/keeperfx/issues/571) | Build/dig lines are missing on high resolution and "straight view" | Defect | Medium | 🗓 |
| [#4232](https://github.com/dkfans/keeperfx/issues/4232) | The click area of the Options button is covered by the Zoom In button. | Other | Low | 🗑 |
| [#4150](https://github.com/dkfans/keeperfx/issues/4150) | Suggestion on side panel | Other | Low * |  |
| [#3706](https://github.com/dkfans/keeperfx/issues/3706) | Distinct icon for Secret Door warning | Other | Low |  |
| [#3346](https://github.com/dkfans/keeperfx/issues/3346) | [Windows inside a game for Workshp] It's a idea as Foundation game (screenshot) | Enhancement | Low * | 🗓 |
| [#3107](https://github.com/dkfans/keeperfx/issues/3107) | Chosen Computer Assistant not stored | Enhancement | Low * | 🗓 🗑 |
| [#3104](https://github.com/dkfans/keeperfx/issues/3104) | Numpad enter does not "return" | Other | Low | 🗓 |
| [#2941](https://github.com/dkfans/keeperfx/issues/2941) | Landview renders text pixels in the wrong colour | Defect | Low * | 🗑 |
| [#2812](https://github.com/dkfans/keeperfx/issues/2812) | Frame on landview a pixel too high up in high resolutions | Defect | Low * |  |
| [#2809](https://github.com/dkfans/keeperfx/issues/2809) | New objective closes Resurrect/Transfer Creature menus. | Other | Low |  |
| [#2784](https://github.com/dkfans/keeperfx/issues/2784) | One-Click: oddity with room panel room icon and cost count when holding down CTRL to tag a 5x5 area for digging | Other | Low * |  |
| [#2620](https://github.com/dkfans/keeperfx/issues/2620) | Replace bad landview voices on remaining campaigns | Enhancement | Low * | 🗓 |
| [#2507](https://github.com/dkfans/keeperfx/issues/2507) | Creature names are rendered behind GUI | Other | Low | 🗓 |
| [#2143](https://github.com/dkfans/keeperfx/issues/2143) | Black bar/game screen isn't sized properly after changing resolution in possession | Defect | Low | 🗓 |
| [#2003](https://github.com/dkfans/keeperfx/issues/2003) | Menu partially loads on packetload | Defect | Low * | 🗓 |
| [#1723](https://github.com/dkfans/keeperfx/issues/1723) | Crash on battle window with floating spirits on the map | Defect | Low * | 🗓 |
| [#1355](https://github.com/dkfans/keeperfx/issues/1355) | make ui scaling toggleable | Enhancement | Low | 🗓 🗑 |
| [#1352](https://github.com/dkfans/keeperfx/issues/1352) | rework ally icon | Enhancement | Low | 🗓 |
| [#801](https://github.com/dkfans/keeperfx/issues/801) | Enemy keeper drop speed and difficulty Slider in "Options" menu | Enhancement | Low * | 🗓 |
| [#574](https://github.com/dkfans/keeperfx/issues/574) | Text moves right as it gets longer in highscore table | Defect | Low | 🗓 |
| [#8](https://github.com/dkfans/keeperfx/issues/8) | Allow any number of save games trough UI | Enhancement | Low | 🗓 |

### CompPlayer (28)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#235](https://github.com/dkfans/keeperfx/issues/235) | Enemy keeper casts call to arms beyond LOS | Defect | Critical | 🗓 |
| [#2028](https://github.com/dkfans/keeperfx/issues/2028) | Computer might pick up already dropped units for attack | Defect | High * | 🗓 |
| [#1756](https://github.com/dkfans/keeperfx/issues/1756) | Computer Players should be able to bridge across lava with rocks in it | Defect | High * | 🗓 |
| [#786](https://github.com/dkfans/keeperfx/issues/786) | Water blocks ComputerPlayer processes, even with bridge available | Defect | High | 🗓 |
| [#604](https://github.com/dkfans/keeperfx/issues/604) | Computer player refuses to mine gold | Defect | High | 🗓 |
| [#594](https://github.com/dkfans/keeperfx/issues/594) | CP throws unhappy unpaid creatures in temple | Defect | High | 🗓 |
| [#5023](https://github.com/dkfans/keeperfx/issues/5023) | Computer sells players traps | Other | Medium * |  |
| [#3393](https://github.com/dkfans/keeperfx/issues/3393) | commands for allied AI keepers | Other | Medium |  |
| [#2930](https://github.com/dkfans/keeperfx/issues/2930) | Computer Players should be able to expand rooms | Enhancement | Medium | 🗓 🗑 |
| [#723](https://github.com/dkfans/keeperfx/issues/723) | Computer player constantly drops creatures in | Other | Medium | 🗓 |
| [#711](https://github.com/dkfans/keeperfx/issues/711) | Computer player to scavenge only when useful | Enhancement | Medium | 🗓 🗑 |
| [#706](https://github.com/dkfans/keeperfx/issues/706) | Computer destroys outer walls with wallhug algorithm | Defect | Medium * | 🗓 |
| [#688](https://github.com/dkfans/keeperfx/issues/688) | Computer player should build rooms adjacent to each other (no double walls) | Enhancement | Medium | 🗓 |
| [#631](https://github.com/dkfans/keeperfx/issues/631) | Computer Player to build bridge around gems in lava | Enhancement | Medium * | 🗓 🗑 |
| [#434](https://github.com/dkfans/keeperfx/issues/434) | [AI Keeper] - Sells doors/traps unnecessarily | Defect | Medium | 🗓 🗑 |
| [#417](https://github.com/dkfans/keeperfx/issues/417) | Computer player to use 'Destroy Walls' | Enhancement | Medium | 🗓 |
| [#204](https://github.com/dkfans/keeperfx/issues/204) | Computer Player to build treasure room right up against gems | Enhancement | Medium | 🗓 |
| [#189](https://github.com/dkfans/keeperfx/issues/189) | AI tries to dig through rock to reach gold | Defect | Medium | 🗓 |
| [#180](https://github.com/dkfans/keeperfx/issues/180) | Computer player won't dig to gems surrounded by gems | Defect | Medium | 🗓 |
| [#4094](https://github.com/dkfans/keeperfx/issues/4094) | Computer Player instantly casts speed on multiple workers. | Defect | Low * | 🗓 |
| [#2646](https://github.com/dkfans/keeperfx/issues/2646) | Computer Players who build 4x4 lairs and treasure rooms need this! | Enhancement | Low * | 🗓 🗑 |
| [#1530](https://github.com/dkfans/keeperfx/issues/1530) | Add Events to computer player | Enhancement | Low | 🗓 🗑 |
| [#948](https://github.com/dkfans/keeperfx/issues/948) | Computer Players should drop units on enemy heart. | Enhancement | Low | 🗓 🗑 |
| [#741](https://github.com/dkfans/keeperfx/issues/741) | Computer player should strategically drop and remove creatures in fights | Enhancement | Low | 🗓 |
| [#619](https://github.com/dkfans/keeperfx/issues/619) | Computer player should function on lava-map | Enhancement | Low | 🗓 |
| [#487](https://github.com/dkfans/keeperfx/issues/487) | AI and room expanding | Defect | Low | 🗓 |
| [#486](https://github.com/dkfans/keeperfx/issues/486) | What is the AI looking for when digging around the edges of the map? | Other | Low | 🗓 |
| [#481](https://github.com/dkfans/keeperfx/issues/481) | Computer player must use doors to make rooms more efficient | Enhancement | Low | 🗓 |

### Engine (23)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4360](https://github.com/dkfans/keeperfx/issues/4360) | Crash in LbSpriteDrawUsingScalingUpDataSolidLR | Defect | Critical * |  |
| [#2276](https://github.com/dkfans/keeperfx/issues/2276) | Crash to desktop on alt+r during frameskip | Defect | High * | 🗓 |
| [#4988](https://github.com/dkfans/keeperfx/issues/4988) | Easter Egg messages go haywire with Delta Time | Defect | Medium * |  |
| [#4956](https://github.com/dkfans/keeperfx/issues/4956) | KeeperFX fails without error on command line with no `-` | Defect | Medium * | ⚠ |
| [#4703](https://github.com/dkfans/keeperfx/issues/4703) | config_rules.c qsort issue (bad comparator) | Defect | Medium * |  |
| [#3471](https://github.com/dkfans/keeperfx/issues/3471) | Incorrect cubes for path touching enemy path and liquid | Defect | Medium * | ⚠ 🗓 |
| [#2749](https://github.com/dkfans/keeperfx/issues/2749) | Non-dynamic lights are off-center | Defect | Medium * | ⚠ 🗓 |
| [#2690](https://github.com/dkfans/keeperfx/issues/2690) | Intermittent hangs under Wine | Other | Medium * | ⚠ 🗑 |
| [#2531](https://github.com/dkfans/keeperfx/issues/2531) | Blood particles do not splatter on walls | Other | Medium * | ⚠ |
| [#2474](https://github.com/dkfans/keeperfx/issues/2474) | Corpses moving into impenetrable rock with WoP trap effect | Other | Medium * | ⚠ 🗓 |
| [#2135](https://github.com/dkfans/keeperfx/issues/2135) | Overflow in draw_onscreen_direct_messages | Defect | Medium * | 🗓 |
| [#1897](https://github.com/dkfans/keeperfx/issues/1897) | Rebound effect is sometimes created many times, in a large spiral | Defect | Medium * | ⚠ 🗓 |
| [#1404](https://github.com/dkfans/keeperfx/issues/1404) | Slab textures are misaligned in both default and forced perspectives | Other | Medium | 🗓 |
| [#75](https://github.com/dkfans/keeperfx/issues/75) | Bile demon lair clipping | Defect | Medium | 🗓 |
| [#37](https://github.com/dkfans/keeperfx/issues/37) | Sprites cropped to right corner at top part of the screen | Defect | Medium | 🗓 |
| [#3400](https://github.com/dkfans/keeperfx/issues/3400) | odd camera zoom-in/zoom-out when switching between standard view and forced perspective with delta time enabled | Other | Low * |  |
| [#3382](https://github.com/dkfans/keeperfx/issues/3382) | Information tortured from enemy does not include disconnected land. | Defect | Low * | ⚠ |
| [#2902](https://github.com/dkfans/keeperfx/issues/2902) | Option to limit the max fps when delta time is enabled | Enhancement | Low * | 🗓 🗑 |
| [#2700](https://github.com/dkfans/keeperfx/issues/2700) | add macos support | Enhancement | Low * | ⚠ 🗓 |
| [#2060](https://github.com/dkfans/keeperfx/issues/2060) | Candles on top of Gold hoards | Defect | Low * | ⚠ 🗓 |
| [#1794](https://github.com/dkfans/keeperfx/issues/1794) | Flowers and room flags drawn wrong way around | Defect | Low * | ⚠ 🗓 |
| [#445](https://github.com/dkfans/keeperfx/issues/445) | No flag on converted hero guardpost | Defect | Low * | ⚠ 🗓 |
| [#141](https://github.com/dkfans/keeperfx/issues/141) | Adding shadows and shaking ground effects to both drawing modes. | Enhancement | Low | 🗓 |

### Input (22)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4719](https://github.com/dkfans/keeperfx/issues/4719) | possess a creature, the F12 key unusable.（keeperfx-1_3_2_5005_Alpha） | Defect | Medium * |  |
| [#4475](https://github.com/dkfans/keeperfx/issues/4475) | Feature request: Adopt touchscreen control | Enhancement | Medium |  |
| [#4200](https://github.com/dkfans/keeperfx/issues/4200) | Dragging the cursor on ground or liquid while in drag mode for highlighting messes up packet positions | Other | Medium * | 🗑 |
| [#3502](https://github.com/dkfans/keeperfx/issues/3502) | Key binding for game.save 0 and game.load 0 | Other | Medium * | 🗓 |
| [#3469](https://github.com/dkfans/keeperfx/issues/3469) | Mousewheel while hovering over battle window scrolls it | Defect | Medium * |  |
| [#3394](https://github.com/dkfans/keeperfx/issues/3394) | Accessibility feature request to make petals, certain fonts and cursor glow bigger | Enhancement | Medium * | 🗓 |
| [#3329](https://github.com/dkfans/keeperfx/issues/3329) | Move keybinds from settings.dat on a more user-friendly text file | Other | Medium * | 🗓 |
| [#2331](https://github.com/dkfans/keeperfx/issues/2331) | Selling large rooms with SHIFT only sells 20x20 tiles counting from the top-left | Defect | Medium * | 🗓 |
| [#1938](https://github.com/dkfans/keeperfx/issues/1938) | Mouse moves weird since KeeperFX 0.4.9 (can be reproduced on OracleVM executing Win10 from a Mac) | Defect | Medium * | 🗓 |
| [#1332](https://github.com/dkfans/keeperfx/issues/1332) | [Wine Linux] alt-tabbing out, the UI becomes unresponsive. | Defect | Medium * | 🗓 |
| [#822](https://github.com/dkfans/keeperfx/issues/822) | Refactor Keyboard inputs | Task | Medium | 🗓 |
| [#3404](https://github.com/dkfans/keeperfx/issues/3404) | cursor light vanishes when you get near the left or top corners of the floor next to a wall | Other | Low * |  |
| [#2870](https://github.com/dkfans/keeperfx/issues/2870) | Boundbox does not draw floor level with cursor on high ground | Defect | Low * |  |
| [#2726](https://github.com/dkfans/keeperfx/issues/2726) | Additional Hotkeys/Accessibility Ideas | Enhancement | Low * | 🗓 |
| [#1790](https://github.com/dkfans/keeperfx/issues/1790) | Moving the cursor over a wall that's being fortified no longer shows the Imp's Health Flower. | Defect | Low * | 🗓 |
| [#1424](https://github.com/dkfans/keeperfx/issues/1424) | possession looking around speed shouldn't depend on aspect ratio | Enhancement | Low | 🗓 🗑 |
| [#1392](https://github.com/dkfans/keeperfx/issues/1392) | Middle Mouse Button to rotate camera | Enhancement | Low | 🗓 |
| [#1378](https://github.com/dkfans/keeperfx/issues/1378) | default movement keys should depend on keyboard layout | Enhancement | Low | 🗓 |
| [#829](https://github.com/dkfans/keeperfx/issues/829) | One-Click-Build: Scroll to tolerance level that makes a difference | Enhancement | Low * | 🗓 🗑 |
| [#827](https://github.com/dkfans/keeperfx/issues/827) | One-Click-Build: Cursor too far off center on square fallback roomspace | Enhancement | Low * | 🗓 🗑 |
| [#766](https://github.com/dkfans/keeperfx/issues/766) | CapsLock is neglected when input on High Score Table | Enhancement | Low | 🗓 🗑 |
| [#409](https://github.com/dkfans/keeperfx/issues/409) | Don't show 'Hand' cursor when player has no POWER_HAND | Enhancement | Low * | 🗓 |

### Pathfinding (18)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4961](https://github.com/dkfans/keeperfx/issues/4961) | Heart attackers have trouble finding a spot to attack from | Other | High |  |
| [#3819](https://github.com/dkfans/keeperfx/issues/3819) | Reinforcing walls does not update creature pathfinding | Defect | High | 🗓 |
| [#3144](https://github.com/dkfans/keeperfx/issues/3144) | Slab becomes impossible to cross | Defect | High |  |
| [#4139](https://github.com/dkfans/keeperfx/issues/4139) | Creatures teleport through allied locked doors | Other | Medium |  |
| [#2517](https://github.com/dkfans/keeperfx/issues/2517) | Imps path through corner and get stuck | Defect | Medium |  |
| [#1015](https://github.com/dkfans/keeperfx/issues/1015) | Creatures get stuck in temple corner | Defect | Medium |  |
| [#720](https://github.com/dkfans/keeperfx/issues/720) | Tunnelers get stuck on edge of water | Defect | Medium | 🗓 |
| [#707](https://github.com/dkfans/keeperfx/issues/707) | Imps cross lava by cutting corners | Defect | Medium | 🗓 |
| [#661](https://github.com/dkfans/keeperfx/issues/661) | pathfinding: creatures take strange detours | Defect | Medium | 🗓 |
| [#602](https://github.com/dkfans/keeperfx/issues/602) | Imps stuck at dungeon heart | Defect | Medium |  |
| [#399](https://github.com/dkfans/keeperfx/issues/399) | Imp pathfinding | Defect | Medium * | 🗓 |
| [#355](https://github.com/dkfans/keeperfx/issues/355) | Pathfinding error - imps get stuck in narrow treasury | Defect | Medium |  |
| [#144](https://github.com/dkfans/keeperfx/issues/144) | Gold Extracting + Imps teleporting | Defect | Medium | 🗓 🗑 |
| [#3767](https://github.com/dkfans/keeperfx/issues/3767) | Freestanding doors | Defect | Low * | 🗓 |
| [#3162](https://github.com/dkfans/keeperfx/issues/3162) | Extend NAVIGATE shot property with turning speed and target death settings | Enhancement | Low * | 🗓 🗑 |
| [#690](https://github.com/dkfans/keeperfx/issues/690) | Imp stuck on book shelve | Defect | Low |  |
| [#325](https://github.com/dkfans/keeperfx/issues/325) | Heroes shattering their own fortress | Defect | Low | 🗓 |
| [#275](https://github.com/dkfans/keeperfx/issues/275) | Bile demon stuck in hallway | Defect | Low | 🗓 |

### Scripts (18)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4146](https://github.com/dkfans/keeperfx/issues/4146) | Allow directly calling temple rewards via script, or allow rewards even when player has no temple | Enhancement | Medium * |  |
| [#3360](https://github.com/dkfans/keeperfx/issues/3360) | Enhancement - SET_HAND_RULE params included as general Criterion. | Other | Medium * |  |
| [#2811](https://github.com/dkfans/keeperfx/issues/2811) | REBOUND_IMMUNE shot property makes the shot glide along walls. | Other | Medium * |  |
| [#2461](https://github.com/dkfans/keeperfx/issues/2461) | More customisation for custom columns/slabs | Defect | Medium | 🗓 |
| [#1668](https://github.com/dkfans/keeperfx/issues/1668) | Repeating conceal_map script fills log with error | Defect | Medium | 🗓 |
| [#1091](https://github.com/dkfans/keeperfx/issues/1091) | Update DOOR/TRAP_AVAILABLE to match ROOM_AVAILABLE script command | Enhancement | Medium | 🗓 🗑 |
| [#790](https://github.com/dkfans/keeperfx/issues/790) | Tunneler party won't 'head for' Dungeon | Defect | Medium | 🗓 |
| [#534](https://github.com/dkfans/keeperfx/issues/534) | ADD_PARTY vs ADD_TUNNELLER_PARTY | Enhancement | Medium | 🗓 |
| [#429](https://github.com/dkfans/keeperfx/issues/429) | Unify action points/hero gates – trigger hero gates | Enhancement | Medium | 🗓 🗑 |
| [#395](https://github.com/dkfans/keeperfx/issues/395) | New script command: PLAYER_NEUTRAL | Enhancement | Medium | 🗓 |
| [#4431](https://github.com/dkfans/keeperfx/issues/4431) | Query/Creature menu corrupts on SET_PLAYER_COLOUR | Enhancement | Low * |  |
| [#2801](https://github.com/dkfans/keeperfx/issues/2801) | Script support for EffectGenerators | Enhancement | Low | 🗓 🗑 |
| [#2800](https://github.com/dkfans/keeperfx/issues/2800) | Feature: rectangular actions points | Enhancement | Low * | 🗓 |
| [#2675](https://github.com/dkfans/keeperfx/issues/2675) | SET_HAND_RULE ANY_CREATURE overwrites other rules | Enhancement | Low * | 🗓 🗑 |
| [#2184](https://github.com/dkfans/keeperfx/issues/2184) | add script variable to know if player is controlled by computer, human or not at all | Enhancement | Low | 🗓 🗑 |
| [#1391](https://github.com/dkfans/keeperfx/issues/1391) | Using CHANGE_SLAB_TYPE next to water leads to patchy gold | Defect | Low * | 🗓 |
| [#980](https://github.com/dkfans/keeperfx/issues/980) | Have EVIL_CREATURE and GOOD_CREATURE instead of creature model in level script | Enhancement | Low | 🗓 🗑 |
| [#922](https://github.com/dkfans/keeperfx/issues/922) | New script command: CREATE_EFFECTS_CIRCLE | Enhancement | Low | 🗓 🗑 |

### Video (18)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#3175](https://github.com/dkfans/keeperfx/issues/3175) | Broken animation | Defect | High * | 🗓 |
| [#5020](https://github.com/dkfans/keeperfx/issues/5020) | The floor looks so clean now; it doesn't have that filthy dungeon feel . | Defect | Medium * |  |
| [#4710](https://github.com/dkfans/keeperfx/issues/4710) | Transparency 3 looks different from before | Defect | Medium * |  |
| [#4647](https://github.com/dkfans/keeperfx/issues/4647) | Screen tearing | Other | Medium * |  |
| [#4512](https://github.com/dkfans/keeperfx/issues/4512) | Ceiling.txt is not readable and changable by humans | Other | Medium * |  |
| [#4346](https://github.com/dkfans/keeperfx/issues/4346) | [Bug] Fairies animate slower than intended while moving | Defect | Medium * |  |
| [#3713](https://github.com/dkfans/keeperfx/issues/3713) | MAP AREAS WITHOUT CEILINGS | Other | Medium * | 🗓 |
| [#3638](https://github.com/dkfans/keeperfx/issues/3638) | GRAPHICS: Sprites rotations of objects and creatures from 5 to 8! | Other | Medium * | 🗓 |
| [#3583](https://github.com/dkfans/keeperfx/issues/3583) | EffectModel for custom beam/breath not working properly if EffectElement has holes in the config | Defect | Medium * | 🗓 |
| [#1248](https://github.com/dkfans/keeperfx/issues/1248) | Unmark mode not entered when holding unit | Defect | Medium * | 🗓 |
| [#437](https://github.com/dkfans/keeperfx/issues/437) | [Graphic Glitch] - Subtile overlay | Defect | Medium * | 🗓 |
| [#431](https://github.com/dkfans/keeperfx/issues/431) | High resolution sprites in KeeperFX | Enhancement | Medium | 🗓 |
| [#154](https://github.com/dkfans/keeperfx/issues/154) | Eyefinity Support | Enhancement | Medium | 🗓 |
| [#4447](https://github.com/dkfans/keeperfx/issues/4447) | Use standard fixed-length types | Defect | Low * |  |
| [#3838](https://github.com/dkfans/keeperfx/issues/3838) | GPU renderer | Task | Low * | 🗓 |
| [#2877](https://github.com/dkfans/keeperfx/issues/2877) | Spells Panel switches to first page when changing resolution. | Other | Low |  |
| [#934](https://github.com/dkfans/keeperfx/issues/934) | Script warnings for non-existing missing textures | Defect | Low * | 🗓 |
| [#433](https://github.com/dkfans/keeperfx/issues/433) | Only one flag per Guardpost | Enhancement | Low * | 🗓 |

### Spells (17)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#3665](https://github.com/dkfans/keeperfx/issues/3665) | Events stop working | Defect | High * |  |
| [#705](https://github.com/dkfans/keeperfx/issues/705) | Computer player does not assign creatures to CTA/Fight | Defect | High | 🗓 |
| [#5016](https://github.com/dkfans/keeperfx/issues/5016) | #New Spells: “DRAIN LEVEL” and “LOWER LEVEL” | Enhancement | Medium * |  |
| [#4971](https://github.com/dkfans/keeperfx/issues/4971) | Add the option to view all powers | Enhancement | Medium * |  |
| [#4734](https://github.com/dkfans/keeperfx/issues/4734) | ROOM_ROLE_POWERS_STORAGE | Other | Medium * |  |
| [#4439](https://github.com/dkfans/keeperfx/issues/4439) | Adding the Turncoat Spell | Other | Medium * |  |
| [#4375](https://github.com/dkfans/keeperfx/issues/4375) | NEW COMMAND "TRANSFORM" | Other | Medium * |  |
| [#4279](https://github.com/dkfans/keeperfx/issues/4279) | Mod Metadata Standard | Enhancement | Medium * |  |
| [#3411](https://github.com/dkfans/keeperfx/issues/3411) | Creatures in guard room don't respond to call to arms | Other | Medium * | 🗓 |
| [#3132](https://github.com/dkfans/keeperfx/issues/3132) | Make-Safe does not reinforce walls in disconnected dungeon. | Defect | Medium * | 🗑 |
| [#1423](https://github.com/dkfans/keeperfx/issues/1423) | Expanding library may switch off Must Obey | Defect | Medium | 🗓 |
| [#796](https://github.com/dkfans/keeperfx/issues/796) | Call to Arms improvement | Defect | Medium * | 🗓 |
| [#149](https://github.com/dkfans/keeperfx/issues/149) | Enemies get stuck in doors after cast rocks | Defect | Medium * | 🗓 |
| [#3627](https://github.com/dkfans/keeperfx/issues/3627) | Scripted SPELL_DISEASE doesn't spread | Defect | Low * |  |
| [#3611](https://github.com/dkfans/keeperfx/issues/3611) | Resurrecting spell | Enhancement | Low * |  |
| [#3470](https://github.com/dkfans/keeperfx/issues/3470) | Accessibility options/settings | Defect | Low * | 🗓 |
| [#3153](https://github.com/dkfans/keeperfx/issues/3153) | Spells not being queued | Other | Low * | 🗓 |

### Lua (14)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4725](https://github.com/dkfans/keeperfx/issues/4725) | Lua: Unlock player_broken_into_flags | Enhancement | Medium |  |
| [#4534](https://github.com/dkfans/keeperfx/issues/4534) | Lua: AddCreatureToLevel does not have 'NONE' spawn-type | Defect | Medium |  |
| [#4484](https://github.com/dkfans/keeperfx/issues/4484) | Lua: Provide cheap distance functions for mapmakers | Enhancement | Medium |  |
| [#4483](https://github.com/dkfans/keeperfx/issues/4483) | Lua: VSCode says numbers aren't allowed on strings | Defect | Medium |  |
| [#4372](https://github.com/dkfans/keeperfx/issues/4372) | Lua: AddCreatureToLevel documention inconsistent | Other | Medium * |  |
| [#4371](https://github.com/dkfans/keeperfx/issues/4371) | Lua: AtActionPoint list | Other | Medium |  |
| [#4370](https://github.com/dkfans/keeperfx/issues/4370) | Lua: AddCreatureToLevel has incorrect default | Defect | Medium * |  |
| [#4356](https://github.com/dkfans/keeperfx/issues/4356) | Lua: Unlock ActionPoints | Other | Medium |  |
| [#4147](https://github.com/dkfans/keeperfx/issues/4147) | add SHOW_BONUS_LEVEL HIDE_BONUS_LEVEL to lua | Enhancement | Medium |  |
| [#4145](https://github.com/dkfans/keeperfx/issues/4145) | Lua: add globals | Enhancement | Medium |  |
| [#4138](https://github.com/dkfans/keeperfx/issues/4138) | Lua: Add OnDropped function to Things | Enhancement | Medium |  |
| [#4087](https://github.com/dkfans/keeperfx/issues/4087) | Lua add events | Enhancement | Medium |  |
| [#4137](https://github.com/dkfans/keeperfx/issues/4137) | Lua: Special box functions configurable | Enhancement | Low |  |
| [#4100](https://github.com/dkfans/keeperfx/issues/4100) | Lua: Add/Remove light from thing | Enhancement | Low |  |

### Sound (13)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4363](https://github.com/dkfans/keeperfx/issues/4363) | Few notices about audio | Defect | Critical * |  |
| [#4946](https://github.com/dkfans/keeperfx/issues/4946) | Stacking sounds louder than before | Other | Medium * |  |
| [#4923](https://github.com/dkfans/keeperfx/issues/4923) | Constant hissing sound | Other | Medium | 🗑 |
| [#4915](https://github.com/dkfans/keeperfx/issues/4915) | Map specific sounds | Other | Medium * |  |
| [#4542](https://github.com/dkfans/keeperfx/issues/4542) | Custom Spell with Sound | Enhancement | Medium * |  |
| [#4541](https://github.com/dkfans/keeperfx/issues/4541) | Custom Level with Sound | Other | Medium * |  |
| [#3886](https://github.com/dkfans/keeperfx/issues/3886) | Remaining Issues for OpenAl | Defect | Medium * | 🗓 |
| [#3875](https://github.com/dkfans/keeperfx/issues/3875) | Sounds reduce while observation | Defect | Medium * | 🗓 |
| [#1407](https://github.com/dkfans/keeperfx/issues/1407) | MUTE_AUDIO_ON_FOCUS_LOST only affects music, not sounds | Defect | Medium | 🗓 |
| [#1249](https://github.com/dkfans/keeperfx/issues/1249) | Most sounds stop working after a while | Defect | Medium | 🗓 |
| [#1047](https://github.com/dkfans/keeperfx/issues/1047) | Music Volume slider has no effect on CD audio volume | Defect | Medium | 🗓 |
| [#744](https://github.com/dkfans/keeperfx/issues/744) | Stange behaviour of music playback | Defect | Medium | 🗓 |
| [#1046](https://github.com/dkfans/keeperfx/issues/1046) | Pause/Resume all playing samples as game enters and leaves a paused/frozen state | Enhancement | Low | 🗓 🗑 |

### Configs (10)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4730](https://github.com/dkfans/keeperfx/issues/4730) | move all cfg over to NamedFields format | Task | Medium * |  |
| [#4727](https://github.com/dkfans/keeperfx/issues/4727) | lua system for reading config values | Enhancement | Medium |  |
| [#4706](https://github.com/dkfans/keeperfx/issues/4706) | No log error when entering incorrect value in effects.toml | Defect | Medium * |  |
| [#4222](https://github.com/dkfans/keeperfx/issues/4222) | Change "Smoothen Video" to "Smoothen Edges" | Other | Medium * |  |
| [#3083](https://github.com/dkfans/keeperfx/issues/3083) | Annoyance for waken up not working | Defect | Medium * |  |
| [#3037](https://github.com/dkfans/keeperfx/issues/3037) | remove remaining count fields | Other | Medium * | 🗓 |
| [#4628](https://github.com/dkfans/keeperfx/issues/4628) | Supports modifying only a single value of a configuration entry | Defect | Low |  |
| [#3159](https://github.com/dkfans/keeperfx/issues/3159) | Have less text in configuration files by copying information from other items | Enhancement | Low * | 🗓 |
| [#2980](https://github.com/dkfans/keeperfx/issues/2980) | Buffs bonus could be configurable as percentage | Enhancement | Low * | 🗓 🗑 |
| [#2551](https://github.com/dkfans/keeperfx/issues/2551) | -connect CLI command crashes with invalid IP | Defect | Low * |  |

### Possession (10)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4390](https://github.com/dkfans/keeperfx/issues/4390) | Possession Swipe Graphics configurable on instance level | Other | Medium |  |
| [#3601](https://github.com/dkfans/keeperfx/issues/3601) | 1ST PERSON MODE (enhancement) | Enhancement | Medium * | 🗓 |
| [#3065](https://github.com/dkfans/keeperfx/issues/3065) | Improve interaction with Things during imp possession | Other | Medium * | 🗓 |
| [#1854](https://github.com/dkfans/keeperfx/issues/1854) | Computer assistent interferes with possession | Defect | Medium * | 🗓 |
| [#807](https://github.com/dkfans/keeperfx/issues/807) | Heroes on guardposts too easy to exploit | Enhancement | Medium * | 🗓 |
| [#299](https://github.com/dkfans/keeperfx/issues/299) | Show enemy health in possession mode | Enhancement | Medium | 🗓 🗑 |
| [#2635](https://github.com/dkfans/keeperfx/issues/2635) | Possessionable traps | Enhancement | Low * | 🗓 |
| [#2010](https://github.com/dkfans/keeperfx/issues/2010) | Possessing on level lost zooms in too fast. | Defect | Low * | 🗓 |
| [#1581](https://github.com/dkfans/keeperfx/issues/1581) | Portal icon missing when claiming in possession. | Defect | Low | 🗓 |
| [#1335](https://github.com/dkfans/keeperfx/issues/1335) | possession diagonal movement speed not normalized | Enhancement | Low | 🗓 🗑 |

### Network (7)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#2001](https://github.com/dkfans/keeperfx/issues/2001) | Game crashes on multiplayer match with added AI | Defect | Critical * | 🗓 |
| [#1377](https://github.com/dkfans/keeperfx/issues/1377) | Zoom keys only work for host in multiplayer game | Defect | High * | 🗓 |
| [#4890](https://github.com/dkfans/keeperfx/issues/4890) | Slap sound in multiplayer landview sometimes doesn't play at all | Defect | Medium * |  |
| [#4650](https://github.com/dkfans/keeperfx/issues/4650) | Players section in lobby menu non-functional | Other | Medium |  |
| [#2629](https://github.com/dkfans/keeperfx/issues/2629) | Set computer allies in skirmish / multiplayer | Enhancement | Low * | 🗓 🗑 |
| [#2128](https://github.com/dkfans/keeperfx/issues/2128) | Multiplayer: Competitive Ruleset | Other | Low | 🗓 |
| [#1931](https://github.com/dkfans/keeperfx/issues/1931) | Multiplayer - Torture Dungeon is not shown if loser exits the game first | Defect | Low |  |

### Resolution (6)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#1287](https://github.com/dkfans/keeperfx/issues/1287) | Launcher application to set all resolutions with one option | Enhancement | Medium | 🗓 |
| [#580](https://github.com/dkfans/keeperfx/issues/580) | Added new test parameter | Review | Medium * | 🗓 |
| [#2312](https://github.com/dkfans/keeperfx/issues/2312) | Vertically Fit Splash screens in widescreen | Defect | Low * | 🗓 |
| [#2081](https://github.com/dkfans/keeperfx/issues/2081) | Creatures appear slightly out of position in low-res | Defect | Low | 🗓 |
| [#1926](https://github.com/dkfans/keeperfx/issues/1926) | Map volume box not visible in straight view when zoomed in too closely on resolutions above 640x480 | Defect | Low * | 🗓 |
| [#1029](https://github.com/dkfans/keeperfx/issues/1029) | 320x200 mode is broken graphically | Defect | Low | 🗓 |

### Mentor Messages (5)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#1802](https://github.com/dkfans/keeperfx/issues/1802) | Mentor volume separate from sound fx volume | Other | Medium * | 🗓 |
| [#267](https://github.com/dkfans/keeperfx/issues/267) | "Your workshop is not big enough" voice message spam | Defect | Medium | 🗓 |
| [#1886](https://github.com/dkfans/keeperfx/issues/1886) | Add subtitles for mentor voices in landscape selection screen | Enhancement | Low * | 🗓 🗑 |
| [#1555](https://github.com/dkfans/keeperfx/issues/1555) | 'No living space' message could be spammed on maps without lairs | Enhancement | Low | 🗓 |
| [#1288](https://github.com/dkfans/keeperfx/issues/1288) | Zoom to last voice-line-event | Enhancement | Low | 🗓 🗑 |

### Persistence (3)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#4409](https://github.com/dkfans/keeperfx/issues/4409) | Can I bring my old game save files to the upgraded version? | Defect | Medium * |  |
| [#1815](https://github.com/dkfans/keeperfx/issues/1815) | Savegame files must have the correct (exact) size | Defect | Medium * | 🗓 |
| [#3481](https://github.com/dkfans/keeperfx/issues/3481) | Load save file from command line | Enhancement | Low * | 🗓 |

### Translation (3)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#3125](https://github.com/dkfans/keeperfx/issues/3125) | Load .po/.pot language files directly (using libintl and gettext) | Other | Medium |  |
| [#1201](https://github.com/dkfans/keeperfx/issues/1201) | Cheat menus are not translatable | Other | Medium | 🗓 |
| [#770](https://github.com/dkfans/keeperfx/issues/770) | Slab description of rival claimed path says it is "Your dungeon" | Enhancement | Low | 🗓 |

### Audio (2)

| Issue | Title | Type | Priority | Flags |
|---|---|---|---|---|
| [#3574](https://github.com/dkfans/keeperfx/issues/3574) | SetMusicPlayerVolume logging should be less verbose | Other | Medium * | 🗓 |
| [#4429](https://github.com/dkfans/keeperfx/issues/4429) | Multiplayer: Resync causes Music track to start over | Enhancement | Low * |  |

---
_Generated 2026-07-14 from 411 open issues. Proposed classification only; not applied._
