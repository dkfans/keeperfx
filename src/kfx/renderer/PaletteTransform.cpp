#include "pre_inc.h"
#include "kfx/renderer/PaletteTransform.h"
#include "post_inc.h"

#include <cmath>
#include <cstring>
#include <utility>

PaletteTransform PaletteTransformFit(const unsigned char* base, const unsigned char* pal)
{
    PaletteTransform xform;
    if (base == nullptr || pal == nullptr || std::memcmp(base, pal, 768) == 0)
        return xform;

    // Normal equations for colour = x0*r + x1*g + x2*b + x3, one right-hand
    // side per output channel. see : https://web.mit.edu/10.001/Web/Course_Notes/GaussElimPivoting.html
    double ata[4][4] = {};
    double atb[4][3] = {};
    for (int i = 0; i < 256; i++)
    {
        const double row[4] = { (double)base[i * 3], (double)base[i * 3 + 1], (double)base[i * 3 + 2], 1.0 };
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
                ata[r][c] += row[r] * row[c];
            for (int ch = 0; ch < 3; ch++)
                atb[r][ch] += row[r] * (double)pal[i * 3 + ch];
        }
    }

    // Gauss-Jordan elimination with partial pivoting.
    for (int col = 0; col < 4; col++)
    {
        int pivot = col;
        for (int r = col + 1; r < 4; r++)
            if (std::fabs(ata[r][col]) > std::fabs(ata[pivot][col]))
                pivot = r;
        if (std::fabs(ata[pivot][col]) < 1e-9)
            return xform;
        if (pivot != col)
        {
            for (int c = 0; c < 4; c++) std::swap(ata[pivot][c], ata[col][c]);
            for (int ch = 0; ch < 3; ch++) std::swap(atb[pivot][ch], atb[col][ch]);
        }
        for (int r = 0; r < 4; r++)
        {
            if (r == col)
                continue;
            const double f = ata[r][col] / ata[col][col];
            for (int c = 0; c < 4; c++)
                ata[r][c] -= f * ata[col][c];
            for (int ch = 0; ch < 3; ch++)
                atb[r][ch] -= f * atb[col][ch];
        }
    }

    for (int ch = 0; ch < 3; ch++)
    {
        for (int c = 0; c < 3; c++)
            xform.m[ch][c] = (float)(atb[c][ch] / ata[c][c]);
        xform.offset[ch] = (float)(atb[3][ch] / ata[3][3]);
    }
    return xform;
}
