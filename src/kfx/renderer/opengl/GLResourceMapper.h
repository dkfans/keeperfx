#ifndef RENDERER_OPENGL_GLRESOURCEMAPPER_H
#define RENDERER_OPENGL_GLRESOURCEMAPPER_H

#include "kfx/renderer/IGpuResourceMapper.h"
#include "kfx/renderer/opengl/GLResourceTypes.h"
#include <mutex>
#include <vector>
#include <unordered_set>

class GLResourceMapper : public IGpuResourceMapper
{
public:
    GpuResourceHandle RequestCreateTexture(const GpuTextureDesc& desc) override;
    GpuResourceHandle RequestCreateRenderTarget(const GpuRenderTargetDesc& desc) override;
    GpuResourceHandle RequestCreateProgram(const GpuProgramDesc& desc) override;
    GpuResourceHandle RequestCreateGeometryBuffer(const GpuGeometryBufferDesc& desc) override;
    void RequestRelease(GpuResourceHandle handle) override;
    GpuResourceHandle RequestReloadTexture(GpuResourceHandle handle, const GpuTextureDesc& new_desc) override;
    GpuResourceHandle RequestReloadRenderTarget(GpuResourceHandle handle, const GpuRenderTargetDesc& new_desc) override;
    GpuResourceHandle RequestReloadProgram(GpuResourceHandle handle, const GpuProgramDesc& new_desc) override;
    GpuResourceHandle RequestReloadGeometryBuffer(GpuResourceHandle handle, const GpuGeometryBufferDesc& new_desc) override;
    const GLTexture* ResolveTexture(GpuResourceHandle handle) override;
    const GLRenderTarget* ResolveRenderTarget(GpuResourceHandle handle) override;
    const GLProgram* ResolveProgram(GpuResourceHandle handle) override;
    const GLGeometryBuffer* ResolveGeometryBuffer(GpuResourceHandle handle) override;
    void ProcessDeferredDestroys(uint64_t frame_being_rendered) override;

    /// @brief Destroys every realized object unconditionally. Render thread only, at shutdown.
    void ShutdownAll();

private:
    enum class SlotState : uint8_t { Free, Pending, Realized, Failed };

    template <typename Desc, typename Realized>
    struct Slot
    {
        SlotState state      = SlotState::Free;
        uint32_t generation  = 0;
        Desc desc            = {};
        Realized realized    = {};
    };

    std::vector<Slot<GpuTextureDesc, GLTexture>> m_textures;
    std::vector<Slot<GpuRenderTargetDesc, GLRenderTarget>> m_render_targets;
    std::vector<Slot<GpuProgramDesc, GLProgram>> m_programs;
    std::vector<Slot<GpuGeometryBufferDesc, GLGeometryBuffer>> m_geometry_buffers;

    struct PendingDestroy
    {
        GpuResourceHandle handle;
        uint64_t destroy_after_frame;

        bool is_release = false;
        bool had_realized = false;
        GLTexture         realized_texture;
        GLRenderTarget    realized_render_target;
        GLProgram         realized_program;
        GLGeometryBuffer  realized_geometry_buffer;
    };
    std::vector<PendingDestroy> m_pending_destroys;

    std::mutex m_mutex;
    std::unordered_set<GpuResourceHandle> m_warned_stale;

    /// @brief Issues the gl* calls to bring a texture into existence.
    /// @param desc Description to realize.
    /// @param out Receives the realized object.
    /// @return False on failure.
    bool RealizeTexture(const GpuTextureDesc& desc, GLTexture& out);

    /// @brief Issues the gl* calls to bring a render target into existence.
    /// @param desc Description to realize.
    /// @param out Receives the realized object.
    /// @return False on failure.
    bool RealizeRenderTarget(const GpuRenderTargetDesc& desc, GLRenderTarget& out);

    /// @brief Compiles and links a program.
    /// @param desc Description to realize.
    /// @param out Receives the realized object.
    /// @return False on failure.
    bool RealizeProgram(const GpuProgramDesc& desc, GLProgram& out);

    /// @brief Issues the gl* calls to bring a geometry buffer into existence.
    /// @param desc Description to realize.
    /// @param out Receives the realized object.
    /// @return False on failure.
    bool RealizeGeometryBuffer(const GpuGeometryBufferDesc& desc, GLGeometryBuffer& out);

    /// @brief Deletes a realized texture.
    /// @param texture Object to delete.
    void DestroyTexture(GLTexture& texture);

    /// @brief Deletes a realized render target and its attachments.
    /// @param target Object to delete.
    void DestroyRenderTarget(GLRenderTarget& target);

    /// @brief Deletes a realized program.
    /// @param program Object to delete.
    void DestroyProgram(GLProgram& program);

    /// @brief Deletes a realized geometry buffer.
    /// @param buffer Object to delete.
    void DestroyGeometryBuffer(GLGeometryBuffer& buffer);
};

#endif // RENDERER_OPENGL_GLRESOURCEMAPPER_H
