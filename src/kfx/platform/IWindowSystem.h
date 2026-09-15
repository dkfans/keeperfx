#ifndef IWINDOWSYSTEM_H
#define IWINDOWSYSTEM_H

#include "kfx/platform/IGLContext.h"
#include <memory>

/** Abstract interface for platform windowing, focus, and OS cursor management.
 *
 */
class IWindowSystem {
public:
    virtual ~IWindowSystem() = default;

    // ----- Focus / activity -----

    /** Returns true if the application window currently has OS focus.
     *  On consoles that own the display exclusively this always returns true. */
    virtual bool IsAppActive() const { return true; }

    /** Called by the SDL event loop when the window gains focus. */
    virtual void OnFocusGained() {}

    /** Called by the SDL event loop when the window loses focus. */
    virtual void OnFocusLost() {}

    /** Returns true if the platform has a real OS-managed cursor that can be
     *  grabbed, hidden, and warped (e.g. SDL desktop).  Returns false on console
     *  platforms where the "cursor" is a virtual game-layer concept.
     *
     *  Any function that applies OS cursor policy should early-return when this
     *  returns false. */
    virtual bool HasOSCursor() const { return false; }

    /** Grab or release the OS cursor. */
    virtual void SetCursorGrab(bool /*grab*/) {}

    /** Select how a subsequent SetCursorGrab(true) locks the cursor */
    virtual void SetUseRelativeMouse(bool /*relative*/) {}

    /** Show or hide the OS cursor. */
    virtual void SetCursorVisible(bool /*visible*/) {}

    /** Warp the cursor to (x, y) in game-surface coordinates. */
    virtual void WarpCursor(int /*x*/, int /*y*/) {}

    /** Move the OS cursor to the middle of the window. */
    virtual void RecenterCursor() {}

    /** Cursor position in game-surface coordinates. False when unavailable. */
    virtual bool GetCursorPosition(int* /*out_x*/, int* /*out_y*/) const { return false; }

    /** Game-surface pixels per unit of OS cursor movement. */
    virtual void GetCursorScale(float* out_sx, float* out_sy) const
    {
        if (out_sx) *out_sx = 1.0f;
        if (out_sy) *out_sy = 1.0f;
    }

    /** True if the OS cursor is within the window bounds. 
     * On platforms that own the display exclusively the cursor can never leave
     * so the default is true. */
    virtual bool IsCursorInWindow() const { return true; }

    // ----- Window management -----

    virtual bool HasWindow() const { return false; }
    virtual unsigned int GetWindowFlags() const { return 0; }
    virtual void GetWindowSize(int* out_w, int* out_h) const
    {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
    }
    /** Size of the window's drawable area in pixels. */
    virtual void GetDrawableSize(int* out_w, int* out_h) const
    {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
    }
    /** Size in pixels of the frame the game draws. */
    virtual void SetGameSurfaceSize(int /*w*/, int /*h*/) {}
    /** Where renderers place the game frame, in drawable pixels from the
     *  top-left corner: scaled to fit the on-screen part of the window,
     *  keeping its aspect ratio. */
    virtual void GetPresentRect(int* out_x, int* out_y, int* out_w, int* out_h) const
    {
        if (out_x) *out_x = 0;
        if (out_y) *out_y = 0;
        GetDrawableSize(out_w, out_h);
    }
    virtual int GetWindowDisplayIndex() const { return -1; }
    virtual int GetNumVideoDisplays() const { return 0; }
    virtual int GetDesktopDisplayMode(int /*display*/, int* out_w, int* out_h) const
    {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return -1;
    }
    virtual int GetDisplayBounds(int /*display*/, int* out_x, int* out_y, int* out_w, int* out_h) const
    {
        if (out_x) *out_x = 0;
        if (out_y) *out_y = 0;
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return -1;
    }
    virtual int GetClosestDisplayMode(int /*display*/, int /*desired_w*/, int /*desired_h*/, int* out_w, int* out_h) const
    {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return 0;
    }
    virtual int SetWindowDisplayMode(int /*w*/, int /*h*/) { return -1; }
    virtual void SetWindowSize(int /*w*/, int /*h*/) {}
    virtual int SetWindowFullscreen(unsigned int /*flags*/) { return -1; }
    virtual void SetWindowBordered(int /*bordered*/) {}
    virtual void SetWindowPosition(int /*x*/, int /*y*/) {}
    /** Create the game window. The renderer backend is already resolved by
     *  this point (see RendererManager.h's RendererResolveType()), so `flags`
     *  already carries whatever backend-required capability flags (e.g.
     *  KFX_WF_OPENGL) the active renderer needs -- CreateWindow() is the only
     *  place the window is ever constructed, and it is born with the correct
     *  flags. There is deliberately no "recreate window to add/remove a
     *  renderer capability flag" API: that need only existed because window
     *  creation used to be decided before the renderer type was known. */
    virtual bool CreateWindow(const char* /*title*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/, unsigned int /*flags*/) { return false; }
    virtual bool SetWindowTitle(const char* /*title*/) { return false; }
    virtual void ShowWindow() {}

    /** Create an OpenGL context for a window created with KFX_WF_OPENGL. The
     *  context is current on the calling thread. Returns nullptr on failure. */
    virtual std::unique_ptr<IGLContext> CreateGLContext() { return nullptr; }


    // ----- Display info -----

    /** Returns the refresh rate (Hz) of the display the game window is on.
     *  Returns 0 when unavailable or not applicable (consoles with fixed rate). */
    virtual int GetDisplayRefreshRate() const { return 0; }

    // ----- Per-frame poll -----

    /** Called once per event-poll cycle.  SDL: no-op (input arrives via events). */
    virtual void PollInput() {}
};

#endif // IWINDOWSYSTEM_H
