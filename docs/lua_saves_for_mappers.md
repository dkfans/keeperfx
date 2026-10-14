# Lua scripts and save games

KeeperFX saves the state of your level's Lua script (your triggers, the `Game` table) inside the save file.
Functions are the one thing in that state that is awkward to store. This page explains how to write your script
so saves keep loading on every platform (Windows, Linux, 32-bit and 64-bit builds) and after you edit the script.

Nothing here is required. A script that ignores it still works; this only changes how portable its saves are.

## Give trigger functions a global name

Functions stored by name are always portable. Define the function globally and pass it by name or by value:

```lua
function OnHeartAttacked(eventData, triggerData)
    -- ...
end

local trigger = CreateTrigger("HeartAttacked", "OnHeartAttacked")   -- by name
local trigger2 = CreateTrigger("HeartAttacked", OnHeartAttacked)    -- by value, same result

TriggerAddCondition(trigger, "OnlyWhenWinning")
```

The function has to be global. Passing a name that isn't a global is an error ("Function '...' not found"), and a
`local function` passed by value is treated as an anonymous function (see below).

## Anonymous functions

Anonymous functions work too. When the game saves one, it records the script file and line where the function is
defined, plus the value of any number, string or boolean it captured. On load it reads that part of the script
again and rebuilds the function. This is portable as long as:

- the function starts on its own line and ends on its own line:

  ```lua
  CreateTrigger("HeartAttacked",
      function(eventData, triggerData)
          return PlayMessage(...)
      end)
  ```

  A one-liner such as `CreateTrigger(ev, function(e) ... end)` can't be rebuilt on its own, and neither can two
  anonymous functions starting on the same line;
- it only captures numbers, strings and booleans. Capturing a table or another function is rejected by
  `CreateTrigger` and `TriggerAddCondition`.

## What if auto-naming doesn't work

The save keeps a copy of the function's compiled form as well, and the game prints a line to the log:

```
Serialisation: <file>:<line> doesn't compile as a standalone expression, storing that function as bytecode instead
```

The same compiled copy is used when you load a save after editing the script, or when the script file is missing:

```
Serialisation: script '<file>' has changed since this save (line N); restored the function at <file>:N from saved bytecode instead
```

The function still works, but only on the same platform and game version that made the save. Look for these lines
in the log and give those functions a global name (or put them on their own lines) if you want the saves to move
between platforms.
