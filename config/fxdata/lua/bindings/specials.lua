---@meta
------------
--specials-
------------

---Activates the effect of an 'Increase Level' dungeon special.
---@param player Player
---@param count? integer How many times the special is activated. Accepts negative values.
function UseSpecialIncreaseLevel(player,count) end

---Activates the effect of an 'Multiply Creatures' dungeon special.
---@param player Player
---@param count integer How many times the special is activated.
function UseSpecialMultiplyCreatures(player,count) end

---Opens the transfer creature special menu for the player, allowing the transfer of a creature.
---@param player Player
function UseSpecialTransferCreature(player) end

---Creates a custom tooltip for Custom special boxes.
---@param boxnumber integer The ID of the custom box. With a new ADiKtEd or the Add_object_to_level command you can set a number. Multiple boxes may have the same number, and they will get the same tooltip and functionality.
---@param tooltip string The text that will displayed when you hover your mouse over the Special box.
function SetBoxTooltip(boxnumber,tooltip) end

---Sets a Tooltip on a custom special box, with a text from the language files.
---@param boxnumber integer The ID of the custom box.
---@param TooltipTextID integer The number of the message, assigned to it in .po or .pot translation file.
function SetBoxTooltipId(boxnumber,TooltipTextID) end

---Has the same effect as a 'Locate Hidden World' dungeon special.
function LocateHiddenWorld() end

---fortifies all of the Dungeon Wall of target player.
---@param player Player
function MakeSafe(player) end

---Removes all the fortifications of the Dungeon Wall of target player.
---@param player Player
function MakeUnsafe(player) end
