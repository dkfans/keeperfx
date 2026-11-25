/******************************************************************************/
/** @file ShelfPacker.h
 *     Shelf rectangle packer with a fixed margin between rects.
 */
/******************************************************************************/
#ifndef KFX_SHELFPACKER_H
#define KFX_SHELFPACKER_H

#include <cstdint>

class ShelfPacker {
public:
    ShelfPacker(int32_t width = 0, int32_t height = 0, int32_t margin = 1);

    /** Places a w x h rect; out is its top-left. False when it doesn't fit. */
    bool Alloc(int32_t w, int32_t h, int32_t* out_x, int32_t* out_y);
    void Reset();
    /** Grows the area. Packed rects stay where they are. */
    void Resize(int32_t width, int32_t height);

    int32_t Width() const { return m_width; }
    int32_t Height() const { return m_height; }

private:
    int32_t m_width;
    int32_t m_height;
    int32_t m_margin;
    int32_t m_cursor_x = 0;
    int32_t m_shelf_y = 0;
    int32_t m_shelf_h = 0;
};

#endif
