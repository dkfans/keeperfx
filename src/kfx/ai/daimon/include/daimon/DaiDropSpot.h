/** @file DaiDropSpot.h
 *  Where Daimon's hand may drop a creature (06 section 5.5), from the knowledge alone: the controller's ring search
 *  (kctl_find_drop_spot_reaching) mirrored -- ring by ring around the subtile, nearest first, the first in scan order
 *  -- on the keeper's own floor or rooms its imps can reach (the reach map), never in a portal, prison, torture
 *  chamber or temple (a drop there dismisses, imprisons, tortures or sacrifices), and, with a reach point, only where
 *  the reach map joins the two. A pure function: no simulation call (02 C10). The controller still checks the drop
 *  with the engine's own rules when the gesture runs. */
#ifndef DAIMON_DAIDROPSPOT_H
#define DAIMON_DAIDROPSPOT_H

#include "daimon_base_types.h"
#include "globals.h"
#include "daimon/DaiKnowledge.h"

/** Room roles a creature is never dropped in. */
constexpr uint64_t DAI_NO_DROP_ROLES = DAI_ROLE_PORTAL | DAI_ROLE_PRISON | DAI_ROLE_TORTURE | DAI_ROLE_SACRIFICE;

/** NULL with the spot in *out_x, *out_y (subtiles), or "NO_DROP_SPOT". `radius` in subtiles (at most 45, as the
 *  controller's); reach_x < 0: no reach point. */
const char *DaiFindDropSpot(const DaiKnowledge &k, PlayerNumber p, DaiSubtlCoord x, DaiSubtlCoord y, int64_t radius,
                            DaiSubtlCoord reach_x, DaiSubtlCoord reach_y, DaiSubtlCoord *out_x, DaiSubtlCoord *out_y);

#endif
