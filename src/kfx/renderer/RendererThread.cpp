#include "pre_inc.h"
#include "kfx/renderer/RendererThread.h"
#include "post_inc.h"

static std::thread::id s_game_thread_id;
static std::thread::id s_render_thread_id;

void RendererThread_RegisterGameThread()
{
    s_game_thread_id = std::this_thread::get_id();
}

void RendererThread_RegisterRenderThread()
{
    s_render_thread_id = std::this_thread::get_id();
}

bool RendererThread_IsGameThread()
{
    return std::this_thread::get_id() == s_game_thread_id;
}

bool RendererThread_IsRenderThread()
{
    return std::this_thread::get_id() == s_render_thread_id;
}
