/******************************************************************************/
/** @file KeeperSpriteDecode.h
 *     Decodes creature sprite RLE into the RG (palette index, coverage) layout
 *     the GL sprite shaders read.
 */
/******************************************************************************/
#ifndef KFX_KEEPERSPRITEDECODE_H
#define KFX_KEEPERSPRITEDECODE_H

#include <cstdint>

/** Largest sprite side, and the row stride of the decode buffer. */
constexpr int32_t k_kspr_decode_dim = 256;

/** RG scratch, k_kspr_decode_dim square. Render thread only. */
extern uint8_t kspr_decode_buf[k_kspr_decode_dim * k_kspr_decode_dim * 2];

/** Decodes TbSprite RLE into dst at stride k_kspr_decode_dim. Every pixel in
 *  a run is opaque, index 0 included, so coverage is stored apart from the index. */
void decode_keeper_rle(uint8_t* dst, const uint8_t* data, int w, int h);

/** True when the sprite just decoded into kspr_decode_buf has black glow
 *  pixels (codes 50-56), which a glow draw subtracts in a second pass. */
bool decoded_sprite_darkens(int w, int h);

#endif
