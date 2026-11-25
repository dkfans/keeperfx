/******************************************************************************/
/** @file SpriteCacheLayers.h
 *     Which creature sprite frame lives where in the layered sprite cache.
 */
/******************************************************************************/
#ifndef KFX_SPRITECACHELAYERS_H
#define KFX_SPRITECACHELAYERS_H

#include "kfx/renderer/opengl/ShelfPacker.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

class SpriteCacheLayers {
public:
    struct Entry {
        int32_t layer;
        int32_t x, y, w, h;
        bool    darkens;
        bool    fresh_layer; ///< Set by Place when the layer must be zeroed first.
    };
    enum PlaceResult {
        Placed,           ///< Fits in a layer the cache already has.
        PlacedAfterEvict, ///< An old layer was emptied for it.
        NeedsGrow,        ///< Room only by growing; call SetCapacity and retry.
        NoRoom,           ///< At the ceiling and every layer was used this frame.
        TooBig,           ///< Larger than a layer.
    };

    SpriteCacheLayers(int32_t layer_dim, int32_t margin);

    void SetCapacity(int32_t layers);   ///< Layers the texture has now.
    void SetCeiling(int32_t layers);    ///< Most layers it may ever have.
    int32_t Capacity() const { return (int32_t)m_layers.size(); }
    int32_t Ceiling() const { return m_ceiling; }

    bool Find(int32_t sprite_id, Entry* out) const;
    PlaceResult Place(int32_t sprite_id, int32_t w, int32_t h, uint64_t frame, Entry* out);
    void SetDarkens(int32_t sprite_id, bool darkens);
    void MarkUsed(int32_t layer, uint64_t frame);
    void Clear();

    int32_t UsedLayers() const { return m_used_layers; }
    int32_t PeakLayers() const { return m_peak_layers; }
    uint32_t Evictions() const { return m_evictions; }

private:
    struct Layer {
        ShelfPacker packer;
        uint64_t last_used = 0;
        bool in_use = false;
        bool needs_zero = true;
    };
    int32_t m_layer_dim;
    int32_t m_margin;
    int32_t m_ceiling = 0;
    int32_t m_used_layers = 0;
    int32_t m_peak_layers = 0;
    uint32_t m_evictions = 0;
    std::vector<Layer> m_layers;
    std::unordered_map<int32_t, Entry> m_entries;

    bool try_layer(int32_t layer, int32_t sprite_id, int32_t w, int32_t h, uint64_t frame, Entry* out);
};

#endif
