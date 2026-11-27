/******************************************************************************/
/** @file GLCaps.h
 *     Queried OpenGL limits and the checks that decide whether GL can run.
 */
/******************************************************************************/
#ifndef KFX_GLCAPS_H
#define KFX_GLCAPS_H

#include <cstdint>
#include <string>

/** Smallest limits the renderer can work with. The tile atlas is 2048x1024 per
 *  layer and holds 32 texture sets. */
constexpr int32_t kGLMinMajor = 3;
constexpr int32_t kGLMinMinor = 3;
constexpr int32_t kGLMinTextureSize = 2048;
constexpr int32_t kGLMinArrayLayers = 32;

/** Limits read from the driver once at startup. All sizing reads from here. */
struct GLCaps {
    std::string vendor;
    std::string renderer;
    std::string version;
    int32_t major = 0;
    int32_t minor = 0;
    int32_t max_texture_size = 0;
    int32_t max_array_layers = 0;
    int32_t max_renderbuffer_size = 0;
    int32_t max_viewport_w = 0;
    int32_t max_viewport_h = 0;
    bool has_khr_debug = false;
    bool debug_context = false;
};

/** Returns an empty string when the caps are enough, else a plain reason safe
 *  to show in a dialog. */
std::string GLCaps_CheckHard(const GLCaps& caps);

/** Smallest doubling of current that holds needed, capped at limit. A result
 *  below needed means the limit is reached. */
int32_t AtlasNextCapacity(int32_t current, int32_t needed, int32_t limit);

/** Largest window side the renderer can draw: the smallest of the texture,
 *  renderbuffer and viewport limits. */
int32_t GLCaps_MaxScreenSide(const GLCaps& caps);

/** Lowers the limits for -glcaps low. Never raises one. */
void GLCaps_ApplyLow(GLCaps& caps);

/** Which startup check -glfail forces to fail. Debug builds only. */
enum class GLFailStage {
    None,     ///< Nothing forced.
    Context,  ///< Context creation fails.
    Version,  ///< The driver reports OpenGL 3.1.
    Limits,   ///< The driver reports limits below the floor.
    Shader,   ///< A required shader fails to build.
    Texture,  ///< A required texture fails to create.
};

/** Command-line options for the GL renderer, set before the renderer starts. */
struct GLStartupOptions {
    bool debug_context = false;           ///< -gldebug, or any debug build.
    GLFailStage fail = GLFailStage::None; ///< -glfail <stage>
    bool caps_low = false;                ///< -glcaps low
};

GLStartupOptions& GLStartupOptions_Get();

/** Parses a -glfail stage name, case-insensitive. */
bool GLStartupOptions_ParseFailStage(const char* text, GLFailStage* out);

#endif
