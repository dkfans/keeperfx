#ifndef IGLCONTEXT_H
#define IGLCONTEXT_H

/** An OpenGL context for the game window. Created on the game thread, then
 *  made current on whichever thread renders. Destroy it on the thread it is
 *  current on. */
class IGLContext {
public:
    using ProcLoader = void* (*)(const char* name);

    virtual ~IGLContext() = default;

    /** Make the context current on the calling thread. */
    virtual bool MakeCurrent() = 0;
    /** Release the context from the calling thread. */
    virtual bool ReleaseCurrent() = 0;
    virtual void SwapBuffers() = 0;
    /** 0 = no vsync, 1 = vsync. Applies to the calling thread's current context. */
    virtual bool SetSwapInterval(int interval) = 0;
    /** Looks up GL entry points by name, for the GL function loader. */
    virtual ProcLoader GetProcLoader() const = 0;
};

#endif // IGLCONTEXT_H
