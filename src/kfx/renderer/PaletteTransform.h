#ifndef RENDERER_PALETTETRANSFORM_H
#define RENDERER_PALETTETRANSFORM_H

/// @brief Colour change between two palettes as colour = m * base + offset.
///        Colour-space effects that were built for one palette, such as the
///        additive glow steps built for the engine palette, go through it to
///        follow the palette actually drawn with.
struct PaletteTransform {
    float m[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
    float offset[3] = { 0.0f, 0.0f, 0.0f };
};

/// @brief Least-squares fit of the transform taking each colour of @p base to
///        the same index in @p pal. Both are 768-byte palettes of the same scale.
///        Equal or missing palettes give the identity.
PaletteTransform PaletteTransformFit(const unsigned char* base, const unsigned char* pal);

#endif // RENDERER_PALETTETRANSFORM_H
