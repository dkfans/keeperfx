#include "pre_inc.h"
#include "kfx/renderer/opengl/GLCaps.h"
#include <algorithm>
#include <cstdio>
#include <strings.h>
#include "post_inc.h"

std::string GLCaps_CheckHard(const GLCaps& caps)
{
    char reason[256];
    if (caps.major < kGLMinMajor || (caps.major == kGLMinMajor && caps.minor < kGLMinMinor))
    {
        std::snprintf(reason, sizeof(reason),
            "this graphics driver offers OpenGL %d.%d, the game needs %d.%d",
            (int)caps.major, (int)caps.minor, (int)kGLMinMajor, (int)kGLMinMinor);
        return reason;
    }
    if (caps.max_texture_size < kGLMinTextureSize)
    {
        std::snprintf(reason, sizeof(reason),
            "this graphics driver's largest texture is %d pixels, the game needs %d",
            (int)caps.max_texture_size, (int)kGLMinTextureSize);
        return reason;
    }
    if (caps.max_array_layers < kGLMinArrayLayers)
    {
        std::snprintf(reason, sizeof(reason),
            "this graphics driver allows %d texture layers, the game needs %d",
            (int)caps.max_array_layers, (int)kGLMinArrayLayers);
        return reason;
    }
    return std::string();
}

int32_t AtlasNextCapacity(int32_t current, int32_t needed, int32_t limit)
{
    int32_t next = std::max<int32_t>(current, 1);
    while (next < needed && next < limit)
        next = (next > limit / 2) ? limit : next * 2;
    return std::min(next, limit);
}

void GLCaps_ApplyLow(GLCaps& caps)
{
    caps.max_texture_size = std::min<int32_t>(caps.max_texture_size, 2048);
    caps.max_array_layers = std::min<int32_t>(caps.max_array_layers, 64);
}

GLStartupOptions& GLStartupOptions_Get()
{
    static GLStartupOptions options;
    return options;
}

bool GLStartupOptions_ParseFailStage(const char* text, GLFailStage* out)
{
    if (text == nullptr || out == nullptr)
        return false;
    static const struct { const char* name; GLFailStage stage; } stages[] = {
        { "context", GLFailStage::Context },
        { "version", GLFailStage::Version },
        { "limits",  GLFailStage::Limits },
        { "shader",  GLFailStage::Shader },
        { "texture", GLFailStage::Texture },
    };
    for (const auto& s : stages)
    {
        if (strcasecmp(text, s.name) == 0)
        {
            *out = s.stage;
            return true;
        }
    }
    return false;
}
