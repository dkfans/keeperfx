#include "pre_inc.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/RendererThread.h"
#include "kfx/renderer/RendererFrameCounter.h"
#include "globals.h" // WARNLOG/ERRORLOG
#include "kfx/renderer/opengl/GLLog.h"
#include "post_inc.h"

#include <algorithm>
#include <cstring>
#include <string>

static const char* gl_error_name(GLenum err)
{
    switch (err)
    {
        case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
        default:                               return "unknown GL error";
    }
}

static const char* gl_error_plain(GLenum err)
{
    return (err == GL_OUT_OF_MEMORY) ? "Not enough video memory for"
                                     : "The graphics driver refused to create";
}

// Clears errors left by earlier calls so they aren't blamed on the next resource.
static void drain_earlier_gl_errors(const char* name)
{
    for (int i = 0; i < 16; ++i)
    {
        const GLenum err = glGetError();
        if (err == GL_NO_ERROR)
            return;
        GLWARN("An earlier graphics call failed; found before creating '%s', not caused by it. (%s)",
               name ? name : "?", gl_error_name(err));
    }
}

static const char* format_name(GpuTextureFormat f)
{
    switch (f)
    {
        case GpuTextureFormat::R8:              return "R8";
        case GpuTextureFormat::RG8:             return "RG8";
        case GpuTextureFormat::RGBA8:           return "RGBA8";
        case GpuTextureFormat::RGBA16F:         return "RGBA16F";
        case GpuTextureFormat::Depth24Stencil8: return "D24S8";
        case GpuTextureFormat::R16UI:           return "R16UI";
        case GpuTextureFormat::RG16UI:          return "RG16UI";
        default:                                return "?";
    }
}

template <typename Desc, typename Realized>
void GLResourceMapper::DeferPreviousDestroy(Slot<Desc, Realized>& slot, GpuResourceHandle handle,
                                            Realized PendingDestroy::* snapshot)
{
    if (!slot.has_previous)
        return;
    PendingDestroy pd;
    pd.handle = handle;
    pd.destroy_after_frame = RendererFrameCounter_Current();
    pd.had_realized = true;
    pd.*snapshot = slot.previous;
    m_pending_destroys.push_back(std::move(pd));
    slot.previous = {};
    slot.has_previous = false;
}

template <typename Desc, typename Realized, typename RealizeFn>
const Realized* GLResourceMapper::RealizePending(Slot<Desc, Realized>& slot, GpuResourceHandle handle,
                                                 RealizeFn realize, Realized PendingDestroy::* snapshot)
{
    Realized fresh = {};
    if (realize(slot.desc, fresh))
    {
        DeferPreviousDestroy(slot, handle, snapshot);
        slot.realized = fresh;
        slot.state = SlotState::Realized;
        return &slot.realized;
    }
    // Callers already sized their draws for the new object, so the old one is
    // not a safe stand-in: drop it and let the draws skip.
    DeferPreviousDestroy(slot, handle, snapshot);
    slot.state = SlotState::Failed;
    return nullptr;
}

static void LogShaderCompileError(const char* stage, GLuint shader)
{
    GLint log_len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0)
    {
        std::string log(log_len, '\0');
        glGetShaderInfoLog(shader, log_len, nullptr, &log[0]);
        WARNLOG("Shader %s compile error: %s", stage, log.c_str());
    }
}

static void LogProgramLinkError(GLuint program)
{
    GLint log_len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0)
    {
        std::string log(log_len, '\0');
        glGetProgramInfoLog(program, log_len, nullptr, &log[0]);
        WARNLOG("Program link error: %s", log.c_str());
    }
}

GpuResourceHandle GLResourceMapper::RequestCreateTexture(const GpuTextureDesc& desc)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t idx = UINT32_MAX;
    for (uint32_t i = 0; i < (uint32_t)m_textures.size(); ++i)
    {
        if (m_textures[i].state == SlotState::Free)
        {
            idx = i;
            break;
        }
    }

    if (idx == UINT32_MAX)
    {
        idx = (uint32_t)m_textures.size();
        if (idx >= UINT32_MAX)
        {
            ERRORLOG("GLResourceMapper: texture table overflow");
            return kInvalidGpuResource;
        }
        m_textures.emplace_back();
    }

    m_textures[idx].state = SlotState::Pending;
    m_textures[idx].desc = desc;
    return GpuResourceHandle_Make(GpuResourceKind::Texture, m_textures[idx].generation, idx);
}

GpuResourceHandle GLResourceMapper::RequestCreateRenderTarget(const GpuRenderTargetDesc& desc)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t idx = UINT32_MAX;
    for (uint32_t i = 0; i < (uint32_t)m_render_targets.size(); ++i)
    {
        if (m_render_targets[i].state == SlotState::Free)
        {
            idx = i;
            break;
        }
    }

    if (idx == UINT32_MAX)
    {
        idx = (uint32_t)m_render_targets.size();
        if (idx >= UINT32_MAX)
        {
            ERRORLOG("GLResourceMapper: render target table overflow");
            return kInvalidGpuResource;
        }
        m_render_targets.emplace_back();
    }

    m_render_targets[idx].state = SlotState::Pending;
    m_render_targets[idx].desc = desc;
    return GpuResourceHandle_Make(GpuResourceKind::RenderTarget, m_render_targets[idx].generation, idx);
}

GpuResourceHandle GLResourceMapper::RequestCreateProgram(const GpuProgramDesc& desc)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t idx = UINT32_MAX;
    for (uint32_t i = 0; i < (uint32_t)m_programs.size(); ++i)
    {
        if (m_programs[i].state == SlotState::Free)
        {
            idx = i;
            break;
        }
    }

    if (idx == UINT32_MAX)
    {
        idx = (uint32_t)m_programs.size();
        if (idx >= UINT32_MAX)
        {
            ERRORLOG("GLResourceMapper: program table overflow");
            return kInvalidGpuResource;
        }
        m_programs.emplace_back();
    }

    m_programs[idx].state = SlotState::Pending;
    m_programs[idx].desc = desc;
    return GpuResourceHandle_Make(GpuResourceKind::Program, m_programs[idx].generation, idx);
}

GpuResourceHandle GLResourceMapper::RequestCreateGeometryBuffer(const GpuGeometryBufferDesc& desc)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    uint32_t idx = UINT32_MAX;
    for (uint32_t i = 0; i < (uint32_t)m_geometry_buffers.size(); ++i)
    {
        if (m_geometry_buffers[i].state == SlotState::Free)
        {
            idx = i;
            break;
        }
    }

    if (idx == UINT32_MAX)
    {
        idx = (uint32_t)m_geometry_buffers.size();
        if (idx >= UINT32_MAX)
        {
            ERRORLOG("GLResourceMapper: geometry buffer table overflow");
            return kInvalidGpuResource;
        }
        m_geometry_buffers.emplace_back();
    }

    m_geometry_buffers[idx].state = SlotState::Pending;
    m_geometry_buffers[idx].desc = desc;
    return GpuResourceHandle_Make(GpuResourceKind::GeometryBuffer, m_geometry_buffers[idx].generation, idx);
}

void GLResourceMapper::RequestRelease(GpuResourceHandle handle)
{
    ASSERT_GAME_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const GpuResourceKind kind = GpuResourceHandle_Kind(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);
    const uint32_t index = GpuResourceHandle_Index(handle);

    bool valid = false;
    PendingDestroy pd;
    pd.handle = handle;
    pd.destroy_after_frame = RendererFrameCounter_Current();
    pd.is_release = true;
    switch (kind)
    {
        case GpuResourceKind::Texture:
            if (index < (uint32_t)m_textures.size() && m_textures[index].generation == generation)
            {
                valid = true;
                if (m_textures[index].state == SlotState::Realized)
                {
                    pd.had_realized = true;
                    pd.realized_texture = m_textures[index].realized;
                }
            }
            break;
        case GpuResourceKind::RenderTarget:
            if (index < (uint32_t)m_render_targets.size() && m_render_targets[index].generation == generation)
            {
                valid = true;
                if (m_render_targets[index].state == SlotState::Realized)
                {
                    pd.had_realized = true;
                    pd.realized_render_target = m_render_targets[index].realized;
                }
            }
            break;
        case GpuResourceKind::Program:
            if (index < (uint32_t)m_programs.size() && m_programs[index].generation == generation)
            {
                valid = true;
                if (m_programs[index].state == SlotState::Realized)
                {
                    pd.had_realized = true;
                    pd.realized_program = m_programs[index].realized;
                }
            }
            break;
        case GpuResourceKind::GeometryBuffer:
            if (index < (uint32_t)m_geometry_buffers.size() && m_geometry_buffers[index].generation == generation)
            {
                valid = true;
                if (m_geometry_buffers[index].state == SlotState::Realized)
                {
                    pd.had_realized = true;
                    pd.realized_geometry_buffer = m_geometry_buffers[index].realized;
                }
            }
            break;
        default:
            break;
    }

    if (!valid)
    {
        WARNLOG("GLResourceMapper::RequestRelease: invalid handle");
        return;
    }

    m_pending_destroys.push_back(std::move(pd));
    switch (kind)
    {
        case GpuResourceKind::Texture:        DeferPreviousDestroy(m_textures[index], handle, &PendingDestroy::realized_texture); break;
        case GpuResourceKind::RenderTarget:   DeferPreviousDestroy(m_render_targets[index], handle, &PendingDestroy::realized_render_target); break;
        case GpuResourceKind::Program:        DeferPreviousDestroy(m_programs[index], handle, &PendingDestroy::realized_program); break;
        case GpuResourceKind::GeometryBuffer: DeferPreviousDestroy(m_geometry_buffers[index], handle, &PendingDestroy::realized_geometry_buffer); break;
        default: break;
    }
}

GpuResourceHandle GLResourceMapper::RequestReloadTexture(GpuResourceHandle handle, const GpuTextureDesc& new_desc)
{
    ASSERT_GAME_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_textures.size() || m_textures[index].generation != generation)
    {
        WARNLOG("GLResourceMapper::RequestReloadTexture: invalid handle");
        return kInvalidGpuResource;
    }
    if (IsReleasePending(handle))
    {
        WARNLOG("GLResourceMapper::RequestReloadTexture: handle already released");
        return kInvalidGpuResource;
    }

    auto& slot = m_textures[index];
    if (slot.state == SlotState::Realized)
    {
        slot.previous      = slot.realized;
        slot.has_previous  = true;
    }
    // A second reload before the first realized keeps the original previous.
    slot.desc  = new_desc;
    slot.state = SlotState::Pending;
    ++slot.generation;

    return GpuResourceHandle_Make(GpuResourceKind::Texture, m_textures[index].generation, index);
}

GpuResourceHandle GLResourceMapper::RequestReloadRenderTarget(GpuResourceHandle handle, const GpuRenderTargetDesc& new_desc)
{
    ASSERT_GAME_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_render_targets.size() || m_render_targets[index].generation != generation)
    {
        WARNLOG("GLResourceMapper::RequestReloadRenderTarget: invalid handle");
        return kInvalidGpuResource;
    }
    if (IsReleasePending(handle))
    {
        WARNLOG("GLResourceMapper::RequestReloadRenderTarget: handle already released");
        return kInvalidGpuResource;
    }

    auto& slot = m_render_targets[index];
    if (slot.state == SlotState::Realized)
    {
        slot.previous      = slot.realized;
        slot.has_previous  = true;
    }
    // A second reload before the first realized keeps the original previous.
    slot.desc  = new_desc;
    slot.state = SlotState::Pending;
    ++slot.generation;

    return GpuResourceHandle_Make(GpuResourceKind::RenderTarget, m_render_targets[index].generation, index);
}

GpuResourceHandle GLResourceMapper::RequestReloadProgram(GpuResourceHandle handle, const GpuProgramDesc& new_desc)
{
    ASSERT_GAME_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_programs.size() || m_programs[index].generation != generation)
    {
        WARNLOG("GLResourceMapper::RequestReloadProgram: invalid handle");
        return kInvalidGpuResource;
    }
    if (IsReleasePending(handle))
    {
        WARNLOG("GLResourceMapper::RequestReloadProgram: handle already released");
        return kInvalidGpuResource;
    }

    auto& slot = m_programs[index];
    if (slot.state == SlotState::Realized)
    {
        slot.previous      = slot.realized;
        slot.has_previous  = true;
    }
    // A second reload before the first realized keeps the original previous.
    slot.desc  = new_desc;
    slot.state = SlotState::Pending;
    ++slot.generation;

    return GpuResourceHandle_Make(GpuResourceKind::Program, m_programs[index].generation, index);
}

GpuResourceHandle GLResourceMapper::RequestReloadGeometryBuffer(GpuResourceHandle handle, const GpuGeometryBufferDesc& new_desc)
{
    ASSERT_GAME_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_geometry_buffers.size() || m_geometry_buffers[index].generation != generation)
    {
        WARNLOG("GLResourceMapper::RequestReloadGeometryBuffer: invalid handle");
        return kInvalidGpuResource;
    }
    if (IsReleasePending(handle))
    {
        WARNLOG("GLResourceMapper::RequestReloadGeometryBuffer: handle already released");
        return kInvalidGpuResource;
    }

    auto& slot = m_geometry_buffers[index];
    if (slot.state == SlotState::Realized)
    {
        slot.previous      = slot.realized;
        slot.has_previous  = true;
    }
    // A second reload before the first realized keeps the original previous.
    slot.desc  = new_desc;
    slot.state = SlotState::Pending;
    ++slot.generation;

    return GpuResourceHandle_Make(GpuResourceKind::GeometryBuffer, m_geometry_buffers[index].generation, index);
}

const GLTexture* GLResourceMapper::ResolveTexture(GpuResourceHandle handle)
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_textures.size())
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveTexture: handle index out of range");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    auto& slot = m_textures[index];
    if (slot.generation != generation)
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveTexture: stale handle (generation mismatch)");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    if (slot.state == SlotState::Realized)
        return &slot.realized;

    if (slot.state == SlotState::Failed)
        return nullptr;

    if (slot.state == SlotState::Pending)
        return RealizePending(slot, handle, [this](const GpuTextureDesc& d, GLTexture& o) { return RealizeTexture(d, o); }, &PendingDestroy::realized_texture);

    return nullptr;
}

const GLRenderTarget* GLResourceMapper::ResolveRenderTarget(GpuResourceHandle handle)
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_render_targets.size())
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveRenderTarget: handle index out of range");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    auto& slot = m_render_targets[index];
    if (slot.generation != generation)
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveRenderTarget: stale handle (generation mismatch)");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    if (slot.state == SlotState::Realized)
        return &slot.realized;

    if (slot.state == SlotState::Failed)
        return nullptr;

    if (slot.state == SlotState::Pending)
        return RealizePending(slot, handle, [this](const GpuRenderTargetDesc& d, GLRenderTarget& o) { return RealizeRenderTarget(d, o); }, &PendingDestroy::realized_render_target);

    return nullptr;
}

const GLProgram* GLResourceMapper::ResolveProgram(GpuResourceHandle handle)
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_programs.size())
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveProgram: handle index out of range");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    auto& slot = m_programs[index];
    if (slot.generation != generation)
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveProgram: stale handle (generation mismatch)");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    if (slot.state == SlotState::Realized)
        return &slot.realized;

    if (slot.state == SlotState::Failed)
        return nullptr;

    if (slot.state == SlotState::Pending)
    {
        const GLProgram* realized = RealizePending(slot, handle, [this](const GpuProgramDesc& d, GLProgram& o) { return RealizeProgram(d, o); }, &PendingDestroy::realized_program);
        if (realized == nullptr)
            ERRORLOG("GLResourceMapper::ResolveProgram: realization failed for '%s'", slot.desc.debug_name);
        return realized;
    }

    return nullptr;
}

const GLGeometryBuffer* GLResourceMapper::ResolveGeometryBuffer(GpuResourceHandle handle)
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    const uint32_t index = GpuResourceHandle_Index(handle);
    const uint32_t generation = GpuResourceHandle_Generation(handle);

    if (index >= (uint32_t)m_geometry_buffers.size())
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveGeometryBuffer: handle index out of range");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    auto& slot = m_geometry_buffers[index];
    if (slot.generation != generation)
    {
        if (m_warned_stale.find(handle) == m_warned_stale.end())
        {
            WARNLOG("GLResourceMapper::ResolveGeometryBuffer: stale handle (generation mismatch)");
            m_warned_stale.insert(handle);
        }
        return nullptr;
    }

    if (slot.state == SlotState::Realized)
        return &slot.realized;

    if (slot.state == SlotState::Failed)
        return nullptr;

    if (slot.state == SlotState::Pending)
        return RealizePending(slot, handle, [this](const GpuGeometryBufferDesc& d, GLGeometryBuffer& o) { return RealizeGeometryBuffer(d, o); }, &PendingDestroy::realized_geometry_buffer);

    return nullptr;
}

void GLResourceMapper::ProcessDeferredDestroys(uint64_t frame_being_rendered)
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_pending_destroys.begin();
    while (it != m_pending_destroys.end())
    {
        if (it->destroy_after_frame >= frame_being_rendered)
        {
            ++it;
        }
        else
        {
            // Always destroy the SNAPSHOT captured at request time.
            {
                switch (GpuResourceHandle_Kind(it->handle))
                {
                    case GpuResourceKind::Texture:        DestroyTexture(it->realized_texture); break;
                    case GpuResourceKind::RenderTarget:    DestroyRenderTarget(it->realized_render_target); break;
                    case GpuResourceKind::Program:         DestroyProgram(it->realized_program); break;
                    case GpuResourceKind::GeometryBuffer:  DestroyGeometryBuffer(it->realized_geometry_buffer); break;
                    default: break;
                }
            }

            if (it->is_release)
            {
                const GpuResourceKind kind = GpuResourceHandle_Kind(it->handle);
                const uint32_t index = GpuResourceHandle_Index(it->handle);
                switch (kind)
                {
                    case GpuResourceKind::Texture:
                        if (index < (uint32_t)m_textures.size()
                            && m_textures[index].generation == GpuResourceHandle_Generation(it->handle))
                        {
                            m_textures[index].state = SlotState::Free;
                            ++m_textures[index].generation;
                        }
                        break;
                    case GpuResourceKind::RenderTarget:
                        if (index < (uint32_t)m_render_targets.size()
                            && m_render_targets[index].generation == GpuResourceHandle_Generation(it->handle))
                        {
                            m_render_targets[index].state = SlotState::Free;
                            ++m_render_targets[index].generation;
                        }
                        break;
                    case GpuResourceKind::Program:
                        if (index < (uint32_t)m_programs.size()
                            && m_programs[index].generation == GpuResourceHandle_Generation(it->handle))
                        {
                            m_programs[index].state = SlotState::Free;
                            ++m_programs[index].generation;
                        }
                        break;
                    case GpuResourceKind::GeometryBuffer:
                        if (index < (uint32_t)m_geometry_buffers.size()
                            && m_geometry_buffers[index].generation == GpuResourceHandle_Generation(it->handle))
                        {
                            m_geometry_buffers[index].state = SlotState::Free;
                            ++m_geometry_buffers[index].generation;
                        }
                        break;
                    default:
                        break;
                }
            }

            it = m_pending_destroys.erase(it);
        }
    }
}

bool GLResourceMapper::IsReleasePending(GpuResourceHandle handle) const
{
    for (const PendingDestroy& pd : m_pending_destroys)
        if (pd.is_release && pd.handle == handle)
            return true;
    return false;
}

void GLResourceMapper::ReportSkippedDraw(GpuResourceHandle handle, const char* what)
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_warned_skipped.insert(handle).second)
        return;
    GLWARN("Skipping %s: its graphics resource could not be created. Logged once.", what);
}

void GLResourceMapper::ShutdownAll()
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& slot : m_textures)
    {
        if (slot.state == SlotState::Realized)
            DestroyTexture(slot.realized);
        if (slot.has_previous)
            DestroyTexture(slot.previous);
        slot.has_previous = false;
        slot.state = SlotState::Free;
    }

    for (auto& slot : m_render_targets)
    {
        if (slot.state == SlotState::Realized)
            DestroyRenderTarget(slot.realized);
        if (slot.has_previous)
            DestroyRenderTarget(slot.previous);
        slot.has_previous = false;
        slot.state = SlotState::Free;
    }

    for (auto& slot : m_programs)
    {
        if (slot.state == SlotState::Realized)
            DestroyProgram(slot.realized);
        if (slot.has_previous)
            DestroyProgram(slot.previous);
        slot.has_previous = false;
        slot.state = SlotState::Free;
    }

    for (auto& slot : m_geometry_buffers)
    {
        if (slot.state == SlotState::Realized)
            DestroyGeometryBuffer(slot.realized);
        if (slot.has_previous)
            DestroyGeometryBuffer(slot.previous);
        slot.has_previous = false;
        slot.state = SlotState::Free;
    }

    m_pending_destroys.clear();
}

bool GLResourceMapper::RealizeTexture(const GpuTextureDesc& desc, GLTexture& out)
{
    drain_earlier_gl_errors(desc.debug_name);
    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    if (tex_id == 0)
    {
        ERRORLOG("GLResourceMapper::RealizeTexture: glGenTextures failed");
        return false;
    }

    GLenum target = (desc.array_layers > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    glBindTexture(target, tex_id);

    GLenum min_filter = (desc.min_filter == GpuTextureFilter::Linear) ? GL_LINEAR : GL_NEAREST;
    GLenum mag_filter = (desc.mag_filter == GpuTextureFilter::Linear) ? GL_LINEAR : GL_NEAREST;
    GLenum wrap = (desc.wrap == GpuTextureWrap::Repeat) ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);

    GLenum gl_format = GL_RGBA;
    GLenum gl_type = GL_UNSIGNED_BYTE;
    GLint internal_format = GL_RGBA8;

    switch (desc.format)
    {
        case GpuTextureFormat::R8:
            gl_format = GL_RED;
            gl_type = GL_UNSIGNED_BYTE;
            internal_format = GL_R8;
            break;
        case GpuTextureFormat::RGBA8:
            gl_format = GL_RGBA;
            gl_type = GL_UNSIGNED_BYTE;
            internal_format = GL_RGBA8;
            break;
        case GpuTextureFormat::RGBA16F:
            gl_format = GL_RGBA;
            gl_type = GL_HALF_FLOAT;
            internal_format = GL_RGBA16F;
            break;
        case GpuTextureFormat::Depth24Stencil8:
            gl_format = GL_DEPTH_STENCIL;
            gl_type = GL_UNSIGNED_INT_24_8;
            internal_format = GL_DEPTH24_STENCIL8;
            break;
        case GpuTextureFormat::R16UI:
            gl_format = GL_RED_INTEGER;
            gl_type = GL_UNSIGNED_SHORT;
            internal_format = GL_R16UI;
            break;
        case GpuTextureFormat::RG16UI:
            gl_format = GL_RG_INTEGER;
            gl_type = GL_UNSIGNED_SHORT;
            internal_format = GL_RG16UI;
            break;
        case GpuTextureFormat::RG8:
            gl_format = GL_RG;
            gl_type = GL_UNSIGNED_BYTE;
            internal_format = GL_RG8;
            break;
        default:
            break;
    }

    GLsizei width = desc.width;
#ifndef NDEBUG
    if (!m_force_fail_texture.empty() && desc.debug_name != nullptr
        && m_force_fail_texture == desc.debug_name)
    {
        width = 1 << 30; // larger than any GL_MAX_TEXTURE_SIZE: the driver rejects it
        m_force_fail_texture.clear();
    }
#endif
    const void* pixels = (width != desc.width || desc.initial_pixels.empty()) ? nullptr : desc.initial_pixels.data();

    if (desc.array_layers > 1)
        glTexImage3D(target, 0, internal_format, width, desc.height, desc.array_layers, 0, gl_format, gl_type, pixels);
    else
        glTexImage2D(target, 0, internal_format, width, desc.height, 0, gl_format, gl_type, pixels);

    const GLenum err = glGetError();
    glBindTexture(target, 0);
    if (err != GL_NO_ERROR)
    {
        glDeleteTextures(1, &tex_id);
        GLERR("%s '%s'. Anything that uses it will not be drawn. (%dx%dx%d %s, %s)",
              gl_error_plain(err), desc.debug_name ? desc.debug_name : "?",
              (int)desc.width, (int)desc.height, std::max(1, (int)desc.array_layers),
              format_name(desc.format), gl_error_name(err));
        return false;
    }

    out.id = tex_id;
    out.target = target;
    out.width = desc.width;
    out.height = desc.height;
    out.array_layers = desc.array_layers;

    return true;
}

bool GLResourceMapper::RealizeRenderTarget(const GpuRenderTargetDesc& desc, GLRenderTarget& out)
{
    drain_earlier_gl_errors(desc.debug_name);
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    if (fbo == 0)
    {
        ERRORLOG("GLResourceMapper::RealizeRenderTarget: glGenFramebuffers failed");
        return false;
    }

    // Realization can happen mid-frame, so put back whatever was bound.
    GLint prev_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<GLuint> color_attachments;
    GLuint depth_stencil = 0;
    std::vector<GLenum> color_attachment_enums;

    for (size_t i = 0; i < desc.attachments.size(); ++i)
    {
        const auto& att = desc.attachments[i];

        GpuTextureDesc tex_desc;
        tex_desc.width = desc.width;
        tex_desc.height = desc.height;
        tex_desc.format = att.format;
        tex_desc.min_filter = GpuTextureFilter::Nearest;
        tex_desc.mag_filter = GpuTextureFilter::Nearest;
        tex_desc.wrap = GpuTextureWrap::Clamp;
        tex_desc.debug_name = desc.debug_name;

        GLTexture att_tex;
        if (!RealizeTexture(tex_desc, att_tex))
        {
            glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prev_fbo);
            glDeleteFramebuffers(1, &fbo);
            for (GLuint tex : color_attachments)
                glDeleteTextures(1, &tex);
            if (depth_stencil != 0)
                glDeleteTextures(1, &depth_stencil);
            return false;
        }

        if (att.is_depth_stencil)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, att_tex.id, 0);
            depth_stencil = att_tex.id;
        }
        else
        {
            GLenum color_attachment = GL_COLOR_ATTACHMENT0 + (GLenum)i;
            glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment, GL_TEXTURE_2D, att_tex.id, 0);
            color_attachments.push_back(att_tex.id);
            color_attachment_enums.push_back(color_attachment);
        }
    }

    if (!color_attachments.empty())
    {
        glDrawBuffers((GLsizei)color_attachment_enums.size(), color_attachment_enums.data());
    }

    const GLenum err = glGetError();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prev_fbo);

    if (err != GL_NO_ERROR || status != GL_FRAMEBUFFER_COMPLETE)
    {
        GLERR("%s render target '%s'. What draws into it will be skipped. (%dx%d, %s, status 0x%X)",
              gl_error_plain(err), desc.debug_name ? desc.debug_name : "?",
              (int)desc.width, (int)desc.height,
              err != GL_NO_ERROR ? gl_error_name(err) : "no GL error", (unsigned)status);
        glDeleteFramebuffers(1, &fbo);
        for (GLuint tex : color_attachments)
            glDeleteTextures(1, &tex);
        if (depth_stencil != 0)
            glDeleteTextures(1, &depth_stencil);
        return false;
    }

    out.fbo = fbo;
    out.color_attachments = color_attachments;
    out.depth_stencil_attachment = depth_stencil;
    out.width = desc.width;
    out.height = desc.height;

    return true;
}

bool GLResourceMapper::RealizeProgram(const GpuProgramDesc& desc, GLProgram& out)
{
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vs_src = desc.vertex_src.c_str();
    glShaderSource(vertex_shader, 1, &vs_src, nullptr);
    glCompileShader(vertex_shader);

    GLint vs_ok = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vs_ok);
    if (!vs_ok)
    {
        LogShaderCompileError("vertex", vertex_shader);
        glDeleteShader(vertex_shader);
        return false;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fs_src = desc.fragment_src.c_str();
    glShaderSource(fragment_shader, 1, &fs_src, nullptr);
    glCompileShader(fragment_shader);

    GLint fs_ok = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fs_ok);
    if (!fs_ok)
    {
        LogShaderCompileError("fragment", fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint link_ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok)
    {
        LogProgramLinkError(program);
        glDeleteProgram(program);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    out.id = program;
    return true;
}

bool GLResourceMapper::RealizeGeometryBuffer(const GpuGeometryBufferDesc& desc, GLGeometryBuffer& out)
{
    drain_earlier_gl_errors(desc.debug_name);
    GLuint vao = 0, vbo = 0, ebo = 0;

    glGenVertexArrays(1, &vao);
    if (vao == 0)
    {
        ERRORLOG("GLResourceMapper::RealizeGeometryBuffer: glGenVertexArrays failed");
        return false;
    }

    glGenBuffers(1, &vbo);
    if (vbo == 0)
    {
        ERRORLOG("GLResourceMapper::RealizeGeometryBuffer: glGenBuffers (VBO) failed");
        glDeleteVertexArrays(1, &vao);
        return false;
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLenum usage = desc.dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    if (desc.initial_vertex_capacity > 0)
    {
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)desc.initial_vertex_capacity, nullptr, usage);
    }

    for (const auto& attr : desc.attribs)
    {
        glEnableVertexAttribArray(attr.location);

        switch (attr.type)
        {
            case GpuVertexAttribType::Float:
                glVertexAttribPointer(attr.location, attr.components, GL_FLOAT, GL_FALSE,
                                      (GLsizei)desc.vertex_stride, (const void*)(uintptr_t)attr.offset);
                break;
            case GpuVertexAttribType::Int:
                glVertexAttribIPointer(attr.location, attr.components, GL_INT,
                                       (GLsizei)desc.vertex_stride, (const void*)(uintptr_t)attr.offset);
                break;
            case GpuVertexAttribType::UByteNorm:
                glVertexAttribPointer(attr.location, attr.components, GL_UNSIGNED_BYTE, GL_TRUE,
                                      (GLsizei)desc.vertex_stride, (const void*)(uintptr_t)attr.offset);
                break;
        }
    }

    if (desc.has_index_buffer)
    {
        glGenBuffers(1, &ebo);
        if (ebo == 0)
        {
            ERRORLOG("GLResourceMapper::RealizeGeometryBuffer: glGenBuffers (EBO) failed");
            glDeleteBuffers(1, &vbo);
            glDeleteVertexArrays(1, &vao);
            return false;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        if (desc.initial_index_capacity > 0)
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)desc.initial_index_capacity, nullptr, usage);
        }
    }

    const GLenum err = glGetError();
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (err != GL_NO_ERROR)
    {
        if (ebo != 0)
            glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        GLERR("%s buffer '%s'. What uses it will not be drawn. (%u bytes, %s)",
              gl_error_plain(err), desc.debug_name ? desc.debug_name : "?",
              (unsigned)(desc.initial_vertex_capacity + desc.initial_index_capacity), gl_error_name(err));
        return false;
    }

    out.vao = vao;
    out.vbo = vbo;
    out.ebo = ebo;
    out.vertex_capacity = desc.initial_vertex_capacity;
    out.index_capacity = desc.initial_index_capacity;

    return true;
}

void GLResourceMapper::DestroyTexture(GLTexture& texture)
{
    if (texture.id != 0)
    {
        glDeleteTextures(1, &texture.id);
        texture.id = 0;
    }
}

void GLResourceMapper::DestroyRenderTarget(GLRenderTarget& target)
{
    for (GLuint color_tex : target.color_attachments)
    {
        glDeleteTextures(1, &color_tex);
    }
    if (target.depth_stencil_attachment != 0)
    {
        glDeleteTextures(1, &target.depth_stencil_attachment);
    }
    if (target.fbo != 0)
    {
        glDeleteFramebuffers(1, &target.fbo);
        target.fbo = 0;
    }
    target.color_attachments.clear();
    target.depth_stencil_attachment = 0;
}

void GLResourceMapper::DestroyProgram(GLProgram& program)
{
    if (program.id != 0)
    {
        glDeleteProgram(program.id);
        program.id = 0;
    }
}

void GLResourceMapper::DestroyGeometryBuffer(GLGeometryBuffer& buffer)
{
    if (buffer.vao != 0)
    {
        glDeleteVertexArrays(1, &buffer.vao);
        buffer.vao = 0;
    }
    if (buffer.vbo != 0)
    {
        glDeleteBuffers(1, &buffer.vbo);
        buffer.vbo = 0;
    }
    if (buffer.ebo != 0)
    {
        glDeleteBuffers(1, &buffer.ebo);
        buffer.ebo = 0;
    }
    buffer.vertex_capacity = 0;
    buffer.index_capacity = 0;
}
