-- ShotFunction.lua
-- Provides specific update functions for shots.

---@param shot Thing the shot thing that is updating
function SmokeWhenFired(shot)
    if shot.health >= shot.max_health-1 then
        CreateEffectAtCoords("EFFECT_SENTRY_SMOKE", shot.pos.val_x, shot.pos.val_y,shot.pos.val_z)
    end
end