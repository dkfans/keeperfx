/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file IModSubsystem.hpp
 *     Base class for C++ subsystems that participate in the unified mod
 *     resource-loading lifecycle.
 *
 * @par Purpose:
 *     A subsystem subclasses IModSubsystem and calls registerWithSystem() at
 *     startup.  The kfx_trigger_load_event() dispatch then calls onLoadEvent()
 *     for each relevant event.
 *
 *     C subsystems use kfx_register_subsystem() from tier_stack.h instead.
 */
/******************************************************************************/
#ifndef KFX_IMODSUBSYSTEM_HPP
#define KFX_IMODSUBSYSTEM_HPP

#include "mod_location.h"
#include "tier_stack.h"
#include "ModWalker.hpp"

/**
 * Base class for C++ subsystems in the mod loading system.
 *
 * Subclasses override onLoadEvent() to react to lifecycle events.
 * The default implementations do nothing (pure virtual is intentionally
 * avoided so partial implementations compile without warnings).
 */
class IModSubsystem {
public:
    virtual ~IModSubsystem() = default;

    /**
     * Register this subsystem with the global event dispatch.
     * Must be called once at game startup; safe to call from constructors.
     */
    void registerWithSystem();

    /**
     * Called by kfx_trigger_load_event() when an event fires that is
     * relevant to this subsystem's declared lifetimes.
     *
     * @param event  The lifecycle event that triggered this call.
     * @param stack  The freshly rebuilt tier stack (read-only).
     */
    virtual void onLoadEvent(KfxLoadEvent event, const KfxTierStack *stack) = 0;

protected:
    IModSubsystem() = default;

private:
    /** Trampoline forwarded from the C subsystem registry */
    static void dispatchReload(KfxLoadEvent event,
                               const KfxTierStack *stack,
                               void *userdata);
};

#endif /* KFX_IMODSUBSYSTEM_HPP */
