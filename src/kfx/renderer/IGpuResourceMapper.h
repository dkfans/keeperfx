#ifndef RENDERER_IGPURESOURCEMAPPER_H
#define RENDERER_IGPURESOURCEMAPPER_H

#include "kfx/renderer/GpuResourceHandle.h"
#include "kfx/renderer/GpuResourceDesc.h"

/// Owns the full lifecycle of every GL object used by the OpenGL backend.
class IGpuResourceMapper
{
public:
    virtual ~IGpuResourceMapper() = default;

    /// @brief Allocates a texture slot. Callable from any thread.
    /// @param desc Texture description.
    /// @return Handle to the new slot.
    virtual GpuResourceHandle RequestCreateTexture(const GpuTextureDesc& desc) = 0;

    /// @brief Allocates a render target slot. Callable from any thread.
    /// @param desc Render target description.
    /// @return Handle to the new slot.
    virtual GpuResourceHandle RequestCreateRenderTarget(const GpuRenderTargetDesc& desc) = 0;

    /// @brief Allocates a program slot. Callable from any thread.
    /// @param desc Program description.
    /// @return Handle to the new slot.
    virtual GpuResourceHandle RequestCreateProgram(const GpuProgramDesc& desc) = 0;

    /// @brief Allocates a geometry buffer slot. Callable from any thread.
    /// @param desc Geometry buffer description.
    /// @return Handle to the new slot.
    virtual GpuResourceHandle RequestCreateGeometryBuffer(const GpuGeometryBufferDesc& desc) = 0;

    /// @brief Marks a handle for deferred destruction. Game thread only.
    /// @param handle Handle to release.
    virtual void RequestRelease(GpuResourceHandle handle) = 0;

    /// @brief Replaces a texture's content, keeping the slot. Game thread only.
    /// @param handle Handle to reload.
    /// @param new_desc Replacement description.
    /// @return New handle, same index, incremented generation.
    virtual GpuResourceHandle RequestReloadTexture(GpuResourceHandle handle, const GpuTextureDesc& new_desc) = 0;

    /// @brief Replaces a render target's content, keeping the slot. Game thread only.
    /// @param handle Handle to reload.
    /// @param new_desc Replacement description.
    /// @return New handle, same index, incremented generation.
    virtual GpuResourceHandle RequestReloadRenderTarget(GpuResourceHandle handle, const GpuRenderTargetDesc& new_desc) = 0;

    /// @brief Replaces a program's content, keeping the slot. Game thread only.
    /// @param handle Handle to reload.
    /// @param new_desc Replacement description.
    /// @return New handle, same index, incremented generation.
    virtual GpuResourceHandle RequestReloadProgram(GpuResourceHandle handle, const GpuProgramDesc& new_desc) = 0;

    /// @brief Replaces a geometry buffer's content, keeping the slot. Game thread only.
    /// @param handle Handle to reload.
    /// @param new_desc Replacement description.
    /// @return New handle, same index, incremented generation.
    virtual GpuResourceHandle RequestReloadGeometryBuffer(GpuResourceHandle handle, const GpuGeometryBufferDesc& new_desc) = 0;

    /// @brief Resolves a texture handle. Render thread only.
    /// @param handle Handle to resolve.
    /// @return Realized texture, or null if invalid, stale, or realization failed.
    virtual const struct GLTexture* ResolveTexture(GpuResourceHandle handle) = 0;

    /// @brief Resolves a render target handle. Render thread only.
    /// @param handle Handle to resolve.
    /// @return Realized render target, or null if invalid, stale, or realization failed.
    virtual const struct GLRenderTarget* ResolveRenderTarget(GpuResourceHandle handle) = 0;

    /// @brief Resolves a program handle. Render thread only.
    /// @param handle Handle to resolve.
    /// @return Realized program, or null if invalid, stale, or realization failed.
    virtual const struct GLProgram* ResolveProgram(GpuResourceHandle handle) = 0;

    /// @brief Resolves a geometry buffer handle. Render thread only.
    /// @param handle Handle to resolve.
    /// @return Realized geometry buffer, or null if invalid, stale, or realization failed.
    virtual const struct GLGeometryBuffer* ResolveGeometryBuffer(GpuResourceHandle handle) = 0;

    /// @brief Destroys anything safe to reclaim as of the given frame. Render thread only.
    /// @param frame_being_rendered Sealed frame number of the content about to be drawn.
    virtual void ProcessDeferredDestroys(uint64_t frame_being_rendered) = 0;
};

#endif // RENDERER_IGPURESOURCEMAPPER_H
