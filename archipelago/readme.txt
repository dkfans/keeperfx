#################################################################################
#                                                                               #
#                                                                               #
#                        Dungeon Keeper FX Archipelago                          #
#                                                                               #
#                                                                               #
#################################################################################

Setup:
* you obviously need a legal copy of DK to play this, so make sure you have that
* use the launcher of your choice to start the game. For qtlauncher obviously don't update the game, not that it should ask you to. Otherwise the settings should all be fine. There is a cheat mode, I don't care if you use that, it's your game and your fun.
* hopefully the menu screen is obvious enough about what you need to do to connect to Archipelago, but Server is the IP address/server name, Port is the AP port needed (usually 38281), Slot Name is the name stated in your yaml, Password is the password (leave blank if no password).
* Once you're connected, you should just be able to play! If this is your first time, Start New Game should be the only highlighed choice of the top three. Click it to go to the overworld.
* There will be one level available, with the Archipelago logo as the flag, this is the hub level. It contains a physical representation of the ingame items sent and received. Go in it and then leave again, and it should unlock your starting level(s).
* There are also some options for the yaml to make your game more interesting: You can also add FX additions to the pool of items (additional creatures, spells, traps, doors and recipes), add heroes, change colour/tileset, and make it harder!

How it works:
* The way this game works with AP is that AP information is only updated inside levels, not in the overworld, so if levels have been unlocked, this will only update when you go into a level.
* This randomizer has some differences from the standard game in terms of how it works: every level unlocked will be available at all times. Each ensign is numbered with the level number and this will update with a tick if the level is completed, and a star if all AP boxes have been found (and both if both!), allowing for easy visual tracking.
* This is also represented in the hub level: there is a section for each level, showing its checks and what they were if found (as AP items, greyed out if not found). Next to the level number is an indicator of level status: a key if locked (and bedrock floor where checks would be), a banner if open, and a flame if completed.
* The hub level also tells you what you've unlocked in terms of levels, creatures, rooms, spells, traps/doors, (heroes to be done soon), recipes, progressives (top right area) and filler.
* if you're starting a new AP game but have game data already from a previous AP game, it's as easy as clicking Start New Game. Similarly, if you accidentally hit Start New Game while you've made progress in an AP game, going into any level should automatically update every level ensign to what it was before anyway, and vice versa, i.e. if you start a new game and click Continue, you can technically cheat and play a level you haven't unlocked (and I sure could add a "you're not supposed to be here, die" check sometime, but you could just behave), but again going in any level should update the unlocked levels.

In a level:
* In each level there is also a tracker in the top-right telling you the number of boxes remaining to find in that level.
* Each level contains a number of AP items (effectively functioning as Dungeon Specials with the Archipelago logo instead). For most levels, these replace where Dungeon Specials would be in the original levels.
* For a few levels (especially levels which did not have any or enough Dungeon Specials), some have been added in significant locations.
* Your Archipelago list of locations/checks should hopefully give you enough hints for how to find these, but anyone with enough experience with DK or enough exploration should be able to find them (and worst case, digging everywhere will mean you eventually find them!).
* These AP items even have a tooltip stating what the item is and who it's for!
* Additionally, you will also send an item for completing the level, and for completing Temple recipes (which are also listed in the Hub level)

Notes and expectations:
* Because this is a randomizer, you may likely have to tackle levels in ways you wouldn't normally. This potentially includes cheesing and sequence breaking, which is fine and basically encouraged! After all, you're evil! Of course you're gonna use underhanded tactics! There may be levels in which you're underprepared for a battle at full strength so much instead rely on rushing the opponent before they're ready, carefully managing invasions from heroes, and potentially even using spells and Possession on levels when you would otherwise have a Bridge. Even though a level might feel impossible with what you have, there may sometimes be things on the map which might give you the strength you need, so look around!
* Another fun aspect of this randomizer is the save system: game saves and AP progress work independently, meaning once an AP item is sent or received, it has been "in all timelines". Even if digging to an AP item would get you killed, if you activate it, it's collected forever, and you can leave the level without penalty (but obviously you'd have to start the level again if you enter it again). This also means, if you save your game, collect a check, then load to before you collected it, the item is also gone "in the past"! This is also true for all items received, so if you have made progress in a level but can't make any more, you can save your progress and reload later, and the itmes received later will automatically unlock when that save is loaded.

(also at some point I will make the hub prettier, add useful text explaining stuff, and try to add on/off toggles for certain unlocks so in the lategame you can streamline your playthrough of each level by e.g. turning off certain rooms and creatures)

FAQ:

* "aaaaaaaaaa I received levels but I don't see them on the overworld!"
  "I got all the checks and/or completed a level but the ensign didn't update correctly!"
  "It crashed and I don't think it updated properly"

Go into any level and then leave again, it should update the unlocked levels and ensigns.

* "Don't I need to have certain rooms to progress in levels 1-3?"
  "The ingame text keeps telling me to use a room/creature/spell/trap/door/whatever I don't have yet!"

Level scripts (mainly levels 1-3) should have been slightly modified so that you don't need specific creatures or things researched for the level script to progress further. The tutorial sparkles might still point to those rooms and spells even if you don't have them, or it might suggest you build a room or use a spell you don't have, but you can ignore them.

* "Help, I'm stuck on level 2!"

You can call the Heroes early by digging straight into their base. I will probably change this script slightly in the future.

* "How am I supposed to beat Level X without [creature/room/spell/trap/door/whatever]?"

It depends on what setting you're playing with or what the level is! Sometimes this can mean that you will just have to come back to this level later (don't forget you can save and reload later when you have more unlocks!), but if you genuinely can't seem to make any more progress, then you might have to get creative with how you beat the level! There may be cases where you have to beat Level 4-10 without a Bridge, which is doable if you have access to Possession (or CTA, or you wait for your enemy to bridge to you) and sheer determination.
There may also be levels you feel like you might have to potentially beat without a Training Room or Library, and a shocking number of levels are actually beatable this way if you're careful, quick or clever. Don't forget that some levels also have hidden neutral creatures/rooms or other secrets!

If you have any other questions or issues/bug reports, consider posting them on the GitHub or the Archipelago thread on Discord.