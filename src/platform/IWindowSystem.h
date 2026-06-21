#ifndef IWINDOWSYSTEM_H
#define IWINDOWSYSTEM_H

/** Abstract interface for platform windowing, focus, and OS cursor management.
 *
 *  Stub implementation — full window-system abstraction will be migrated from
 *  the pc-opengl-renderer fork when that work is in scope.  All methods have
 *  safe defaults so that IPlatform::GetWindowSystem() compiles and links
 *  cleanly without any concrete window-system implementation.
 */
class IWindowSystem {
public:
    virtual ~IWindowSystem() = default;
};

#endif // IWINDOWSYSTEM_H
