#include "pre_inc.h"
#include "daimon/DaiCtlWorld.h"
#include "daimon/DaiDropSpot.h"
#include "post_inc.h"

namespace {
const DaiCtlWorld &world_of(void *ctx) { return *static_cast<const DaiCtlWorld *>(ctx); }

bool reached(const DaiKnowledge &k, DaiSubtlCoord x, DaiSubtlCoord y)
{
    return k.Reach().Reached(DaiSubtileSlab(x), DaiSubtileSlab(y));
}

TbBool connected(void *ctx, DaiSubtlCoord ax, DaiSubtlCoord ay, DaiSubtlCoord bx, DaiSubtlCoord by)
{
    const DaiCtlWorld &w = world_of(ctx);
    return w.Answer(reached(w.Know(), ax, ay) && reached(w.Know(), bx, by));
}

TbBool can_reach(void *ctx, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y)
{
    (void)creature;
    const DaiCtlWorld &w = world_of(ctx);
    return w.Answer(reached(w.Know(), x, y));
}

/** A target the keeper knows of: an enemy it sees, or one of its own creatures. */
TbBool target_known(void *ctx, DaiThingIndex target)
{
    const DaiCtlWorld &w = world_of(ctx);
    const DaiKnowledge &k = w.Know();
    bool known = false;
    for (const DaiThingLook &e : k.Enemies()) known = known || (e.ref.idx == target);
    known = known || (k.CreatureByIdx(target) != nullptr);
    return w.Answer(known);
}

/** Daimon's drop-spot finder over its knowledge (DaiDropSpot.h, P4.7). */
const char *drop_spot(void *ctx, PlayerNumber plyr, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y, int64_t radius,
    DaiSubtlCoord reach_x, DaiSubtlCoord reach_y, DaiSubtlCoord *out_x, DaiSubtlCoord *out_y)
{
    (void)creature;
    const DaiCtlWorld &w = world_of(ctx);
    const char *err = DaiFindDropSpot(w.Know(), plyr, x, y, radius, reach_x, reach_y, out_x, out_y);
    w.Answer(err == nullptr);
    return err;
}
}

DaiCtlWorld::DaiCtlWorld(const DaiKnowledge &k)
    : m_k(k), m_world{ this, connected, can_reach, target_known, drop_spot }
{
}
