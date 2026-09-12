#ifndef RENDERER_GPURESOURCEDESC_H
#define RENDERER_GPURESOURCEDESC_H

#include <cstdint>
#include <string>
#include <vector>

enum class GpuTextureFormat : uint8_t { R8, RGBA8, RGBA16F, Depth24Stencil8 };
enum class GpuTextureFilter : uint8_t { Nearest, Linear };
enum class GpuTextureWrap   : uint8_t { Clamp, Repeat };

/// Describes a 2D texture or 2D texture array.
struct GpuTextureDesc
{
    int32_t width        = 0;
    int32_t height       = 0;
    int32_t array_layers = 1; ///< 1 = plain 2D texture, >1 = texture array.
    GpuTextureFormat format     = GpuTextureFormat::RGBA8;
    GpuTextureFilter min_filter = GpuTextureFilter::Nearest;
    GpuTextureFilter mag_filter = GpuTextureFilter::Nearest;
    GpuTextureWrap   wrap       = GpuTextureWrap::Clamp;
    std::vector<uint8_t> initial_pixels; ///< Owned. Empty means uninitialized storage.
    const char* debug_name = ""; ///< Literal.
};

/// Describes one attachment of a render target.
struct GpuRenderTargetAttachmentDesc
{
    GpuTextureFormat format = GpuTextureFormat::RGBA8;
    bool is_depth_stencil   = false;
};

/// Describes a framebuffer with N attachments.
struct GpuRenderTargetDesc
{
    int32_t width  = 0;
    int32_t height = 0;
    std::vector<GpuRenderTargetAttachmentDesc> attachments; ///< Order is attachment index.
    const char* debug_name = ""; ///< Literal.
};

/// Describes a linked shader program.
struct GpuProgramDesc
{
    std::string vertex_src;
    std::string fragment_src;
    const char* debug_name = ""; ///< Literal.
};

enum class GpuVertexAttribType : uint8_t { Float, Int, UByteNorm };

/// Describes one vertex attribute binding.
struct GpuVertexAttribDesc
{
    uint32_t location            = 0;
    int32_t components           = 0; ///< 1 to 4.
    GpuVertexAttribType type     = GpuVertexAttribType::Float;
    uint32_t offset              = 0; ///< Byte offset within one vertex.
};

/// Describes a VAO plus VBO, and optionally an EBO.
struct GpuGeometryBufferDesc
{
    uint32_t vertex_stride = 0;
    std::vector<GpuVertexAttribDesc> attribs;
    bool dynamic                 = true;  ///< Streamed vs. static upload.
    bool has_index_buffer        = false; ///< Allocates an EBO too.
    size_t initial_vertex_capacity = 0;   ///< Bytes. Zero defers allocation.
    size_t initial_index_capacity  = 0;   ///< Bytes. Only used with an EBO.
    const char* debug_name = ""; ///< Literal.
};

#endif // RENDERER_GPURESOURCEDESC_H
