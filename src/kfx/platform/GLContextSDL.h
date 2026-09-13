#ifndef GLCONTEXTSDL_H
#define GLCONTEXTSDL_H

#include "kfx/platform/IGLContext.h"
#include <memory>

struct SDL_Window;
struct SDL_GLContextState;

/** SDL3 implementation of IGLContext. */
class GLContextSDL : public IGLContext {
public:
    /** Set the context and pixel format attributes. Must run before the
     *  window is created, since SDL fixes the pixel format at creation. */
    static void RequestWindowAttributes();

    /** Create a context for a window made with SDL_WINDOW_OPENGL. The context
     *  is current on the calling thread. Returns nullptr on failure. */
    static std::unique_ptr<GLContextSDL> Create(SDL_Window* window);

    ~GLContextSDL() override;

    bool MakeCurrent() override;
    bool ReleaseCurrent() override;
    void SwapBuffers() override;
    bool SetSwapInterval(int interval) override;
    ProcLoader GetProcLoader() const override;

private:
    GLContextSDL(SDL_Window* window, SDL_GLContextState* context);

    SDL_Window* m_window = nullptr;
    SDL_GLContextState* m_context = nullptr;
};

#endif // GLCONTEXTSDL_H
