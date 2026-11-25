#include "pre_inc.h"
#include "kfx/renderer/opengl/atlas/SpriteCacheLayers.h"
#include <iterator>
#include "post_inc.h"

SpriteCacheLayers::SpriteCacheLayers(int32_t layer_dim, int32_t margin)
    : m_layer_dim(layer_dim), m_margin(margin) {}

void SpriteCacheLayers::SetCapacity(int32_t layers)
{
    while ((int32_t)m_layers.size() < layers)
    {
        Layer l;
        l.packer = ShelfPacker(m_layer_dim, m_layer_dim, m_margin);
        m_layers.push_back(l);
    }
}

void SpriteCacheLayers::SetCeiling(int32_t layers) { m_ceiling = layers; }

bool SpriteCacheLayers::Find(int32_t sprite_id, Entry* out) const
{
    auto it = m_entries.find(sprite_id);
    if (it == m_entries.end())
        return false;
    *out = it->second;
    out->fresh_layer = false;
    return true;
}

bool SpriteCacheLayers::try_layer(int32_t layer, int32_t sprite_id, int32_t w, int32_t h, uint64_t frame, Entry* out)
{
    Layer& l = m_layers[layer];
    int32_t x, y;
    if (!l.packer.Alloc(w, h, &x, &y))
        return false;
    Entry e{ layer, x, y, w, h, false, l.needs_zero };
    l.needs_zero = false;
    if (!l.in_use)
    {
        l.in_use = true;
        ++m_used_layers;
        if (m_used_layers > m_peak_layers)
            m_peak_layers = m_used_layers;
    }
    l.last_used = frame;
    m_entries[sprite_id] = e;
    *out = e;
    return true;
}

SpriteCacheLayers::PlaceResult SpriteCacheLayers::Place(int32_t sprite_id, int32_t w, int32_t h, uint64_t frame, Entry* out)
{
    if (w + m_margin > m_layer_dim || h + m_margin > m_layer_dim)
        return TooBig;
    for (int32_t i = (int32_t)m_layers.size() - 1; i >= 0; --i)
        if (m_layers[i].in_use && try_layer(i, sprite_id, w, h, frame, out))
            return Placed;
    for (int32_t i = 0; i < (int32_t)m_layers.size(); ++i)
        if (!m_layers[i].in_use && try_layer(i, sprite_id, w, h, frame, out))
            return Placed;
    if ((int32_t)m_layers.size() < m_ceiling)
        return NeedsGrow;

    int32_t victim = -1;
    for (int32_t i = 0; i < (int32_t)m_layers.size(); ++i)
    {
        if (m_layers[i].last_used == frame)
            continue;
        if (victim < 0 || m_layers[i].last_used < m_layers[victim].last_used)
            victim = i;
    }
    if (victim < 0)
        return NoRoom;
    for (auto it = m_entries.begin(); it != m_entries.end(); )
        it = (it->second.layer == victim) ? m_entries.erase(it) : std::next(it);
    Layer& v = m_layers[victim];
    v.packer.Reset();
    v.needs_zero = true;
    v.in_use = false;
    --m_used_layers;
    ++m_evictions;
    return try_layer(victim, sprite_id, w, h, frame, out) ? PlacedAfterEvict : NoRoom;
}

void SpriteCacheLayers::SetDarkens(int32_t sprite_id, bool darkens)
{
    auto it = m_entries.find(sprite_id);
    if (it != m_entries.end())
        it->second.darkens = darkens;
}

void SpriteCacheLayers::MarkUsed(int32_t layer, uint64_t frame)
{
    if (layer >= 0 && layer < (int32_t)m_layers.size())
        m_layers[layer].last_used = frame;
}

void SpriteCacheLayers::Clear()
{
    m_entries.clear();
    for (Layer& l : m_layers)
    {
        l.packer.Reset();
        l.in_use = false;
        l.needs_zero = true;
        l.last_used = 0;
    }
    m_used_layers = 0;
}
