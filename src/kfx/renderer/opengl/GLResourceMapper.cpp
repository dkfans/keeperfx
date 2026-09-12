#include "pre_inc.h"
#include "kfx/renderer/opengl/GLResourceMapper.h"
#include "kfx/renderer/RendererThread.h"
#include "kfx/renderer/RendererFrameCounter.h"
#include "globals.h" // WARNLOG/ERRORLOG
#include "post_inc.h"

#include <cstring>

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
    switch (kind)
    {
        case GpuResourceKind::Texture:
            if (index < (uint32_t)m_textures.size() && m_textures[index].generation == generation)
                valid = true;
            break;
        case GpuResourceKind::RenderTarget:
            if (index < (uint32_t)m_render_targets.size() && m_render_targets[index].generation == generation)
                valid = true;
            break;
        case GpuResourceKind::Program:
            if (index < (uint32_t)m_programs.size() && m_programs[index].generation == generation)
                valid = true;
            break;
        case GpuResourceKind::GeometryBuffer:
            if (index < (uint32_t)m_geometry_buffers.size() && m_geometry_buffers[index].generation == generation)
                valid = true;
            break;
        default:
            break;
    }

    if (!valid)
    {
        WARNLOG("GLResourceMapper::RequestRelease: invalid handle");
        return;
    }

    m_pending_destroys.push_back({handle, RendererFrameCounter_Current()});
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

    m_pending_destroys.push_back({handle, RendererFrameCounter_Current()});
    m_textures[index].desc = new_desc;
    m_textures[index].state = SlotState::Pending;
    ++m_textures[index].generation;

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

    m_pending_destroys.push_back({handle, RendererFrameCounter_Current()});
    m_render_targets[index].desc = new_desc;
    m_render_targets[index].state = SlotState::Pending;
    ++m_render_targets[index].generation;

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

    m_pending_destroys.push_back({handle, RendererFrameCounter_Current()});
    m_programs[index].desc = new_desc;
    m_programs[index].state = SlotState::Pending;
    ++m_programs[index].generation;

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

    m_pending_destroys.push_back({handle, RendererFrameCounter_Current()});
    m_geometry_buffers[index].desc = new_desc;
    m_geometry_buffers[index].state = SlotState::Pending;
    ++m_geometry_buffers[index].generation;

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
    {
        if (RealizeTexture(slot.desc, slot.realized))
        {
            slot.state = SlotState::Realized;
            return &slot.realized;
        }
        else
        {
            ERRORLOG("GLResourceMapper::ResolveTexture: realization failed for '%s'", slot.desc.debug_name);
            slot.state = SlotState::Failed;
            return nullptr;
        }
    }

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
    {
        if (RealizeRenderTarget(slot.desc, slot.realized))
        {
            slot.state = SlotState::Realized;
            return &slot.realized;
        }
        else
        {
            ERRORLOG("GLResourceMapper::ResolveRenderTarget: realization failed for '%s'", slot.desc.debug_name);
            slot.state = SlotState::Failed;
            return nullptr;
        }
    }

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
        if (RealizeProgram(slot.desc, slot.realized))
        {
            slot.state = SlotState::Realized;
            return &slot.realized;
        }
        else
        {
            ERRORLOG("GLResourceMapper::ResolveProgram: realization failed for '%s'", slot.desc.debug_name);
            slot.state = SlotState::Failed;
            return nullptr;
        }
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
    {
        if (RealizeGeometryBuffer(slot.desc, slot.realized))
        {
            slot.state = SlotState::Realized;
            return &slot.realized;
        }
        else
        {
            ERRORLOG("GLResourceMapper::ResolveGeometryBuffer: realization failed for '%s'", slot.desc.debug_name);
            slot.state = SlotState::Failed;
            return nullptr;
        }
    }

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
            const GpuResourceKind kind = GpuResourceHandle_Kind(it->handle);
            const uint32_t index = GpuResourceHandle_Index(it->handle);

            switch (kind)
            {
                case GpuResourceKind::Texture:
                    if (index < (uint32_t)m_textures.size())
                    {
                        auto& slot = m_textures[index];
                        if (slot.state == SlotState::Realized)
                            DestroyTexture(slot.realized);
                        slot.state = SlotState::Free;
                        ++slot.generation;
                    }
                    break;

                case GpuResourceKind::RenderTarget:
                    if (index < (uint32_t)m_render_targets.size())
                    {
                        auto& slot = m_render_targets[index];
                        if (slot.state == SlotState::Realized)
                            DestroyRenderTarget(slot.realized);
                        slot.state = SlotState::Free;
                        ++slot.generation;
                    }
                    break;

                case GpuResourceKind::Program:
                    if (index < (uint32_t)m_programs.size())
                    {
                        auto& slot = m_programs[index];
                        if (slot.state == SlotState::Realized)
                            DestroyProgram(slot.realized);
                        slot.state = SlotState::Free;
                        ++slot.generation;
                    }
                    break;

                case GpuResourceKind::GeometryBuffer:
                    if (index < (uint32_t)m_geometry_buffers.size())
                    {
                        auto& slot = m_geometry_buffers[index];
                        if (slot.state == SlotState::Realized)
                            DestroyGeometryBuffer(slot.realized);
                        slot.state = SlotState::Free;
                        ++slot.generation;
                    }
                    break;

                default:
                    break;
            }

            it = m_pending_destroys.erase(it);
        }
    }
}

void GLResourceMapper::ShutdownAll()
{
    ASSERT_RENDER_THREAD();
    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto& slot : m_textures)
    {
        if (slot.state == SlotState::Realized)
            DestroyTexture(slot.realized);
        slot.state = SlotState::Free;
    }

    for (auto& slot : m_render_targets)
    {
        if (slot.state == SlotState::Realized)
            DestroyRenderTarget(slot.realized);
        slot.state = SlotState::Free;
    }

    for (auto& slot : m_programs)
    {
        if (slot.state == SlotState::Realized)
            DestroyProgram(slot.realized);
        slot.state = SlotState::Free;
    }

    for (auto& slot : m_geometry_buffers)
    {
        if (slot.state == SlotState::Realized)
            DestroyGeometryBuffer(slot.realized);
        slot.state = SlotState::Free;
    }

    m_pending_destroys.clear();
}

bool GLResourceMapper::RealizeTexture(const GpuTextureDesc& desc, GLTexture& out)
{
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
        default:
            break;
    }

    if (desc.array_layers > 1)
    {
        glTexImage3D(target, 0, internal_format, desc.width, desc.height, desc.array_layers, 0,
                     gl_format, gl_type, desc.initial_pixels.empty() ? nullptr : desc.initial_pixels.data());
    }
    else
    {
        glTexImage2D(target, 0, internal_format, desc.width, desc.height, 0,
                     gl_format, gl_type, desc.initial_pixels.empty() ? nullptr : desc.initial_pixels.data());
    }

    glBindTexture(target, 0);

    out.id = tex_id;
    out.target = target;
    out.width = desc.width;
    out.height = desc.height;
    out.array_layers = desc.array_layers;

    return true;
}

bool GLResourceMapper::RealizeRenderTarget(const GpuRenderTargetDesc& desc, GLRenderTarget& out)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    if (fbo == 0)
    {
        ERRORLOG("GLResourceMapper::RealizeRenderTarget: glGenFramebuffers failed");
        return false;
    }

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
            ERRORLOG("GLResourceMapper::RealizeRenderTarget: failed to create attachment texture");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fbo);
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

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        ERRORLOG("GLResourceMapper::RealizeRenderTarget: FBO incomplete (status 0x%X)", status);
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

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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
