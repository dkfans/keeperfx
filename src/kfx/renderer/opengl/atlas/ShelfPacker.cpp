#include "pre_inc.h"
#include "kfx/renderer/opengl/atlas/ShelfPacker.h"
#include "post_inc.h"

ShelfPacker::ShelfPacker(int32_t width, int32_t height, int32_t margin)
    : m_width(width), m_height(height), m_margin(margin) {}

bool ShelfPacker::Alloc(int32_t w, int32_t h, int32_t* out_x, int32_t* out_y)
{
    const int32_t aw = w + m_margin;
    const int32_t ah = h + m_margin;
    int32_t cursor_x = m_cursor_x;
    int32_t shelf_y = m_shelf_y;
    int32_t shelf_h = m_shelf_h;
    if (cursor_x + aw > m_width)
    {
        shelf_y += shelf_h;
        cursor_x = 0;
        shelf_h = 0;
    }
    if (aw > m_width || shelf_y + ah > m_height)
        return false;
    if (ah > shelf_h)
        shelf_h = ah;
    *out_x = cursor_x;
    *out_y = shelf_y;
    m_cursor_x = cursor_x + aw;
    m_shelf_y = shelf_y;
    m_shelf_h = shelf_h;
    return true;
}

void ShelfPacker::Reset()
{
    m_cursor_x = 0;
    m_shelf_y = 0;
    m_shelf_h = 0;
}

void ShelfPacker::Resize(int32_t width, int32_t height)
{
    m_width = width;
    m_height = height;
}
