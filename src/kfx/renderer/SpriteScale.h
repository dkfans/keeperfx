/******************************************************************************/
// Dungeon Keeper - Renderer Abstraction Layer
/******************************************************************************/
/** @file SpriteScale.h
 *     Placement of a scaled sprite on screen, shared by every backend.
 * @par Purpose:
 *    SpriteScale is a backend-neutral description of how a sprite frame is
 *    placed and scaled on the screen. It helps fix the shitty positioning in the original sprite sheets,
 *    and is used by every backend to place a sprite frame on the screen. so I dont have to do this
 *    in every backend.
 */
/******************************************************************************/
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A sprite frame scaled from frame_src to frame_dst pixels at frame_x/y, with
 *  the drawn content starting content_x/y source pixels into the frame.
 *  Coordinates are relative to the graphics window of window_w x window_h. */
struct SpriteScale {
    int32_t frame_x, frame_y;
    int32_t frame_src_w, frame_src_h;
    int32_t frame_dst_w, frame_dst_h;
    int32_t content_x, content_y;
    int32_t window_w, window_h;
};

/** One axis of the mapping. Destination pixel dst_start + q (0 <= q < dst_len)
 *  shows content source pixel
 *      j = max(((q + 1) << 16) - phase - 1, 0) / step
 *  clamped to the content length. */
struct SpriteScaleAxis {
    int32_t dst_start;
    int32_t dst_len;
    int32_t phase;
    int32_t step;
};

/** Resolve one axis for content_len source pixels starting at content_off. */
static inline void sprite_scale_axis(int32_t origin, int32_t src_len, int32_t dst_len,
    int32_t window_len, int32_t content_off, int32_t content_len, struct SpriteScaleAxis *out)
{
    out->dst_start = 0;
    out->dst_len = 0;
    out->phase = 0;
    out->step = 1;
    if ((src_len <= 0) || (dst_len <= 0) || (content_len <= 0))
        return;
    const int32_t step = (int32_t)(((int64_t)dst_len << 16) / src_len);
    if (step <= 0)
        return;
    const int64_t base = (int64_t)(step >> 1) + ((int64_t)origin << 16) + (int64_t)content_off * step;
    int32_t start = (int32_t)(base >> 16);
    const int32_t end = (int32_t)((base + (int64_t)content_len * step) >> 16);
    // The clipped table starts the frame's first source pixel at the frame origin.
    if ((content_off == 0) && ((origin < 0) || (dst_len + src_len + origin >= window_len)))
    {
        const int32_t first = (origin > 0) ? origin : 0;
        if (first < start)
            start = first;
    }
    if (end <= start)
        return;
    out->dst_start = start;
    out->dst_len = end - start;
    out->phase = (int32_t)(base - ((int64_t)start << 16));
    out->step = step;
}

#ifdef __cplusplus
}
#endif
