#ifndef RENDERER_GPURESOURCEHANDLE_H
#define RENDERER_GPURESOURCEHANDLE_H

#include <cstdint>

/// Resource kinds the mapper owns.
enum class GpuResourceKind : uint8_t
{
    Invalid        = 0,
    Texture        = 1,
    RenderTarget   = 2,
    Program        = 3,
    GeometryBuffer = 4,
};

/// Opaque resource handle: kind (8 bits) | generation (24 bits) | index (32 bits).
using GpuResourceHandle = uint64_t;

constexpr GpuResourceHandle kInvalidGpuResource = 0;

constexpr uint32_t kGpuHandleKindBits       = 8;
constexpr uint32_t kGpuHandleGenerationBits = 24;
constexpr uint32_t kGpuHandleIndexBits      = 32;

constexpr uint32_t kGpuHandleKindShift       = kGpuHandleGenerationBits + kGpuHandleIndexBits;
constexpr uint32_t kGpuHandleGenerationShift = kGpuHandleIndexBits;

constexpr uint64_t kGpuHandleGenerationMask = (uint64_t{1} << kGpuHandleGenerationBits) - 1;
constexpr uint64_t kGpuHandleIndexMask      = UINT32_MAX;
constexpr uint64_t kGpuHandleKindMask       = (uint64_t{1} << kGpuHandleKindBits) - 1;

/// @brief Packs a kind, generation, and index into one handle.
/// @param kind Resource kind.
/// @param generation Slot generation at allocation time.
/// @param index Slot index within the kind's table.
/// @return The packed handle.
constexpr GpuResourceHandle GpuResourceHandle_Make(
    const GpuResourceKind kind, const uint32_t generation, const uint32_t index)
{
    return (uint64_t{static_cast<uint8_t>(kind)} << kGpuHandleKindShift)
         | ((uint64_t{generation} & kGpuHandleGenerationMask) << kGpuHandleGenerationShift)
         | (uint64_t{index} & kGpuHandleIndexMask);
}

/// @brief Reads the kind field back out of a handle.
/// @param handle Handle to read.
/// @return The handle's kind.
constexpr GpuResourceKind GpuResourceHandle_Kind(const GpuResourceHandle handle)
{
    return static_cast<GpuResourceKind>((handle >> kGpuHandleKindShift) & kGpuHandleKindMask);
}

/// @brief Reads the generation field back out of a handle.
/// @param handle Handle to read.
/// @return The handle's generation.
constexpr uint32_t GpuResourceHandle_Generation(const GpuResourceHandle handle)
{
    return static_cast<uint32_t>((handle >> kGpuHandleGenerationShift) & kGpuHandleGenerationMask);
}

/// @brief Reads the index field back out of a handle.
/// @param handle Handle to read.
/// @return The handle's slot index.
constexpr uint32_t GpuResourceHandle_Index(const GpuResourceHandle handle)
{
    return static_cast<uint32_t>(handle & kGpuHandleIndexMask);
}

#endif // RENDERER_GPURESOURCEHANDLE_H
