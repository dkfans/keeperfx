---@meta
-- bindings/lens.lua
-- LuaLS type annotations for the Lens Effect API
-- See src/lua_api_lens.c for implementation details

---@class LensContext
---@field width integer Screen width in pixels
---@field height integer Screen height in pixels
---@field srcbuf userdata Source buffer (read-only)
---@field dstbuf userdata Destination buffer (write)

---@class PixelOperation
---@field x integer X coordinate
---@field y integer Y coordinate
---@field w integer Width of rectangle
---@field h integer Height of rectangle
---@field color integer Palette index (0-255)

---@class OffsetOperation
---@field x integer X coordinate
---@field y integer Y coordinate
---@field w integer Width of rectangle
---@field h integer Height of rectangle
---@field offset integer Signed color offset (-255 to +255)

---@class BlendOperation
---@field x integer X coordinate
---@field y integer Y coordinate
---@field w integer Width of rectangle
---@field h integer Height of rectangle
---@field color integer Target palette index (0-255)
---@field alpha integer Blend amount (0=transparent, 255=opaque)

---@class RemapOperation
---@field x integer X coordinate
---@field y integer Y coordinate
---@field w integer Width of rectangle
---@field h integer Height of rectangle

---Creates a new custom lens effect with the specified name.
---The lens must be created before setting callbacks or activating.
---@param name string Unique lens identifier (e.g., "LENS_PLASMA")
---@param config? table Optional configuration table with effect properties
---@return boolean success True if lens was created successfully
function CreateLens(name, config) return true end

---Sets a Lua function to be called each frame when the lens is active.
---The callback receives a context table with buffers and dimensions.
---@param lens_name string Lens identifier (must exist from CreateLens)
---@param callback fun(ctx: LensContext): boolean Drawing callback that returns true if buffer was modified
---@return boolean success True if callback was set successfully
function SetLensDrawCallback(lens_name, callback) return true end

---Activates a lens by name (custom) or index (built-in).
---Pass 0 or nil to disable the active lens.
---@param lens string|integer Lens name for custom lenses, or index for built-in (0=none, 1=dark elf, etc.)
---@return boolean success True if lens was activated
function SetActiveLens(lens) return true end

---Returns the currently active lens index or name.
---@return integer|string lens Current lens identifier (0 = no lens active)
---@nodiscard
function GetActiveLens() return 0 end

---Sets a runtime parameter for a custom lens effect.
---Note: Not yet implemented.
---@param lens_name string Lens identifier
---@param param_name string Parameter name (e.g., "intensity", "fog_speed")
---@param value number Parameter value
---@return boolean success True if parameter was set
function SetLensParameter(lens_name, param_name, value) return false end

---Loads a custom asset for use in lens effects.
---Uses the standard asset loading fallback (JSON → mods → data).
---@param filename string Asset filename (e.g., "my_fog.raw")
---@param asset_type string Asset type: "mist", "overlay", or "palette"
---@return integer asset_id Asset ID for reference (-1 on failure)
---@nodiscard
function LoadLensAsset(filename, asset_type) return 1 end

---Controls accessibility settings for lens effects.
---Allows players to disable specific effect types that may cause discomfort.
---Note: Not yet implemented.
---@param effect_type string Effect type: "mist", "displacement", "overlay", "palette", "flyeye"
---@param enabled boolean True to enable, false to disable
---@return boolean success True if setting was changed
function SetLensEnabled(effect_type, enabled) return false end

---Checks if a lens effect type is enabled (accessibility check).
---Note: Not yet implemented.
---@param effect_type string Effect type to check
---@return boolean enabled True if the effect type is enabled
---@nodiscard
function IsLensEnabled(effect_type) return true end

---High-performance buffer copy operation. Copies entire source buffer to destination
---in native C without Lua overhead. Use this before applying effects that modify pixels.
---@param src userdata Source buffer (ctx.srcbuf)
---@param dst userdata Destination buffer (ctx.dstbuf)
---@return boolean success True if copy succeeded
function CopyBuffer(src, dst) return true end

---High-performance batch pixel submission. Processes all pixel operations
---in native C with a single boundary crossing. Much faster than individual SetPixel calls.
---@param buffer userdata Buffer handle (ctx.srcbuf or ctx.dstbuf)
---@param operations PixelOperation[] Array of pixel fill operations
---@return boolean success True if batch was processed
function SubmitPixelBatch(buffer, operations) return true end

---High-performance batch color offset application. Reads existing pixels,
---applies signed offsets, and writes back clamped values (0-255).
---Perfect for effects that modulate existing scene colors (brightness, tint).
---@param buffer userdata Buffer handle (must already contain base image)
---@param operations OffsetOperation[] Array of offset operations (offset can be negative)
---@return boolean success True if batch was processed
function ApplyColorOffsetBatch(buffer, operations) return true end

---High-performance batch alpha blending. Blends a target color with existing pixels
---using proper RGB color space blending. Uses Porter-Duff compositing.
---@param buffer userdata Buffer handle
---@param operations BlendOperation[] Array of blend operations with alpha values
---@return boolean success True if batch was processed
function BlendColorBatch(buffer, operations) return true end

---Ultra-fast palette remapping using a single lookup table for all operations.
---The remap table is read ONCE and applied to all rectangles in the batch.
---@param buffer userdata Buffer handle
---@param remap_table integer[] 256-entry lookup table (index 1-256 maps palette 0-255)
---@param operations RemapOperation[] Array of rectangle operations
---@return boolean success True if batch was processed
function RemapPixelBatch(buffer, remap_table, operations) return true end

---Builds a proper darkening lookup table using the current game palette.
---Uses RGB color space blending toward black and finds closest palette match.
---@param strength number Darkening strength (0.0 = no change, 1.0 = full black)
---@return integer[]|nil remap_table 256-entry lookup table, or nil on failure
---@nodiscard
function BuildDarkeningLUT(strength) return {} end
