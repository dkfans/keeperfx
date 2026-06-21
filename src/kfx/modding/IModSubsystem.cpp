/******************************************************************************/
// Free implementation of KeeperFX.
/******************************************************************************/
/** @file IModSubsystem.cpp
 *     IModSubsystem — registration and dispatch trampoline.
 */
/******************************************************************************/

#include "../../pre_inc.h"
#include "IModSubsystem.hpp"

/* -----------------------------------------------------------------------
 * Trampoline
 * --------------------------------------------------------------------- */

void IModSubsystem::dispatchReload(KfxLoadEvent event,
                                   const KfxTierStack *stack,
                                   void *userdata)
{
    static_cast<IModSubsystem *>(userdata)->onLoadEvent(event, stack);
}

/* -----------------------------------------------------------------------
 * Registration
 * --------------------------------------------------------------------- */

void IModSubsystem::registerWithSystem()
{
    kfx_register_subsystem(dispatchReload, this);
}

#include "../../post_inc.h"
