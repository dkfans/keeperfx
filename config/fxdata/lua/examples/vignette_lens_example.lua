-- vignette_lens_example.lua
-- Example: Custom lens effect using the Lens API
-- 
-- This demonstrates how to create a pulsating vignette (edge darkening) effect
-- using the Lens API's high-performance batch operations.
--
-- To use in your campaign:
--   1. Copy this file to your campaign's lua/ folder
--   2. Add `require "vignette_lens_example"` to your init.lua
--   3. Call InitVignetteLens() from OnCampaignGameStart()
--
-- To test manually:
--   Run TestVignetteLens() from the Lua console

--------------------------------------------------------------------------------
-- Configuration
--------------------------------------------------------------------------------

---@class VignetteConfig
---@field step integer Pixel step size for batch operations (4 = 4x4 blocks)
---@field intensity integer Maximum darkening intensity (0-255)
---@field speed number Animation speed multiplier
---@field lens_name string Unique lens identifier

---Configuration for the vignette effect
---@type VignetteConfig
local CONFIG = {
    step = 4,           -- Process in 4x4 pixel blocks for performance
    intensity = 255,    -- Full darkening at edges
    speed = 0.1,        -- Animation speed
    lens_name = "LENS_VIGNETTE",
}

--------------------------------------------------------------------------------
-- State
--------------------------------------------------------------------------------

-- Animation time accumulator
local g_time = 0

-- Pre-computed darkening lookup tables (11 levels: 0%, 10%, 20%, ..., 100%)
---@type table<integer, integer[]>
local g_darkening_tables = {}

--------------------------------------------------------------------------------
-- Initialization
--------------------------------------------------------------------------------

---Pre-compute darkening lookup tables using RGB color space blending.
---This is done once at startup for maximum performance during rendering.
local function BuildDarkeningTables()
    print("[Vignette] Building darkening lookup tables...")
    for i = 0, 10 do
        local strength = i / 10.0
        g_darkening_tables[i] = BuildDarkeningLUT(strength)
    end
    print("[Vignette] Darkening tables ready (11 levels)")
end

---Initialize the vignette lens effect.
---@return boolean success True if lens was created and callback was set
function InitVignetteLens()
    -- Build lookup tables first
    BuildDarkeningTables()
    
    -- Create the custom lens
    local success = CreateLens(CONFIG.lens_name)
    if not success then
        print("[Vignette] ERROR: Failed to create lens")
        return false
    end
    
    -- Set the draw callback
    success = SetLensDrawCallback(CONFIG.lens_name, VignetteDrawCallback)
    if not success then
        print("[Vignette] ERROR: Failed to set draw callback")
        return false
    end
    
    print("[Vignette] Lens initialized successfully")
    return true
end

--------------------------------------------------------------------------------
-- Rendering
--------------------------------------------------------------------------------

---Vignette rendering callback. Called every frame when the lens is active.
---
---The algorithm:
---  1. Copy source buffer to destination (preserves original scene)
---  2. Calculate distance from screen center for each pixel block
---  3. Group blocks by darkening level (0-10)
---  4. Apply each darkening table in a single batch call
---
---@param ctx LensContext The rendering context with buffers and dimensions
---@return boolean modified True if the buffer was modified
function VignetteDrawCallback(ctx)
    -- Advance animation time
    g_time = g_time + CONFIG.speed
    
    -- Get buffer info
    local src = ctx.srcbuf
    local dst = ctx.dstbuf
    local width = ctx.width
    local height = ctx.height
    
    -- Start with a copy of the source
    CopyBuffer(src, dst)
    
    -- Calculate pulsing intensity (0.0 to 1.0)
    local pulse = (math.sin(g_time) + 1.0) * 0.5
    
    -- Screen center for distance calculation
    local half_width = width * 0.5
    local half_height = height * 0.5
    local max_dist = math.sqrt(half_width * half_width + half_height * half_height)
    
    -- Group pixel blocks by darkening level
    -- This minimizes RemapPixelBatch calls (max 11 instead of thousands)
    local batches = {}
    for i = 0, 10 do
        batches[i] = {}
    end
    
    -- Calculate darkening level for each block
    local step = CONFIG.step
    for y = 0, height - 1, step do
        local dy = y - half_height
        for x = 0, width - 1, step do
            local dx = x - half_width
            
            -- Distance from center, normalized to 0-1
            local dist = math.sqrt(dx * dx + dy * dy)
            local edge_factor = dist / max_dist
            
            -- Combine pulse animation with edge distance
            local strength = pulse * edge_factor * (CONFIG.intensity / 255.0)
            
            -- Quantize to one of 11 darkening levels
            local table_idx = math.floor(strength * 10)
            if table_idx < 0 then table_idx = 0 end
            if table_idx > 10 then table_idx = 10 end
            
            -- Add block to appropriate batch
            table.insert(batches[table_idx], {x = x, y = y, w = step, h = step})
        end
    end
    
    -- Apply all batches (each uses pre-computed darkening table)
    for i = 0, 10 do
        if #batches[i] > 0 then
            RemapPixelBatch(dst, g_darkening_tables[i], batches[i])
        end
    end
    
    return true
end

--------------------------------------------------------------------------------
-- Activation Helpers
--------------------------------------------------------------------------------

---Activate the vignette lens effect.
function ActivateVignetteLens()
    SetActiveLens(CONFIG.lens_name)
end

---Deactivate the vignette lens effect.
function DeactivateVignetteLens()
    if GetActiveLens() == CONFIG.lens_name then
        SetActiveLens(0)
    end
end

--------------------------------------------------------------------------------
-- Testing
--------------------------------------------------------------------------------

---Manual test function. Initializes and activates the vignette lens.
---Use this from the Lua console for testing.
function TestVignetteLens()
    print("=== VIGNETTE LENS TEST ===")
    
    if InitVignetteLens() then
        ActivateVignetteLens()
        print("Vignette lens active! Use SetActiveLens(0) to disable.")
    else
        print("ERROR: Failed to initialize vignette lens")
    end
end
