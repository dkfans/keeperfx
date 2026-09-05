#include "pre_inc.h"
#include "kfx/renderer/opengl/GLTextRenderer.h"
#include "kfx/renderer/opengl/GLUIRenderer.h"
#include "kfx/renderer/ir/TextCommands.h"
#include "kfx/renderer/RendererManager.h"
#include "bflib_sprfnt.h"
#include "bflib_sprite.h"
#include "bflib_text.h" // read_utf_8_codepoint
#include "bflib_video.h" // Lb_TEXT_*/Lb_SPRITE_* flags, lbDisplayEx
#include "post_inc.h"

namespace {
inline float chan6_to_unit(unsigned char v) { return (float)((v * 255) / 63) / 255.0f; }

// TRANSPAR4/8 aren't a real alpha blend on the CPU path -- matches
// GLUIRenderer.cpp's identical alpha_from_draw_flags() (kept as a separate
// copy here since text and UI quads are drawn through different immediate
// paths; same constants, same develop-derived defaults).
constexpr float kTextTranspar4Alpha = 0.5f;
constexpr float kTextTranspar8Alpha = 0.25f;
float text_alpha_from_draw_flags(uint32_t draw_flags)
{
    if (draw_flags & Lb_SPRITE_TRANSPAR4) return kTextTranspar4Alpha;
    if (draw_flags & Lb_SPRITE_TRANSPAR8) return kTextTranspar8Alpha;
    return 1.0f;
}

// Explicit-font line height, mirroring LbTextLineHeight()'s dbc/western
// branch but taking the font as a parameter instead of reading
// lbFontPtr/active_dbcfont (see TextCommands.h's snapshot-safety comment).
float LineHeightExplicit(const struct TbSpriteSheet* font, const struct AsianFont* dbc_font)
{
    return dbc_font ? (float)LbDbcCharHeight(dbc_font) : (float)LbSprFontCharHeight(font, ' ');
}
} // namespace

void GLTextRenderer::DrawGlyphs(const IRTextDrawCmd& cmd)
{
    if (!m_ui) return;

    // Once GL genuinely threads (P5.6), this replay runs on the render
    // thread while the game thread may already be building the next frame --
    // if a font reload happened in that gap, cmd.font is a dangling
    // TbSpriteSheet* (the same class of bug the Phase 4 cursor fix was
    // about). Mirrors ITextRenderer::ReplayTextCommand's identical guard.
    if (cmd.font_generation != LbTextGetFontGeneration())
        return;

    const struct TbSpriteSheet* font = (const struct TbSpriteSheet*)cmd.font;
    const struct AsianFont* dbc_font = cmd.dbc_enabled ? (const struct AsianFont*)cmd.dbc_font : nullptr;
    if (!font && !dbc_font) return;

    DrawState state{ cmd.draw_colour, cmd.draw_flags };
    LayoutAndDraw(cmd, font, dbc_font, state);
}

// Mirrors LbTextDrawResizedImmediate()'s word-wrap loop (bflib_sprfnt.c),
// which GL can't call directly -- it draws straight into the software
// framebuffer. Operates purely on cmd's already-snapshotted fields plus the
// explicit-font measurement helpers in bflib_sprfnt.h/.c, never the live
// lbFontPtr/lbTextJustifyWindow/lbTextClipWindow/active_dbcfont globals
// those functions read (this call can run out of step with the game thread
// once GL genuinely threads).
void GLTextRenderer::LayoutAndDraw(const IRTextDrawCmd& cmd, const struct TbSpriteSheet* font,
                                   const struct AsianFont* dbc_font, DrawState& state)
{
    const int ups = cmd.units_per_px;

    auto char_width = [&](uint32_t chr) -> float {
        return dbc_font ? (float)LbDbcCharWidthM(dbc_font, chr, ups)
                         : (float)(LbSprFontCharWidth(font, chr) * ups / 16);
    };
    auto word_width = [&](const char* s) -> float {
        return (float)LbTextWordWidthExplicit(font, dbc_font, dbc_font != nullptr, s, ups);
    };
    auto is_duospace = [&](uint32_t chr) -> bool {
        // Only meaningful once DBC is actually active -- active_dbcfont is
        // always non-null on this branch (LbTextSetFont() sets it
        // unconditionally), so gating on dbc_font here (rather than
        // replicating is_duospace_char()'s unconditional check) avoids
        // resetting the word-boundary counter for stray >=0xFF codepoints
        // in ordinary western text, which never matters in practice.
        return dbc_font && LbDbcIsDuospaceChar(dbc_font, chr);
    };

    const float h = LineHeightExplicit(font, dbc_font) * ups / 16.0f;
    const float justifyx = (float)(cmd.justify_x - cmd.clip_x);
    const float justifyy = (float)(cmd.justify_y - cmd.clip_y);
    float posx = (float)cmd.pos_x + justifyx;
    const float startx = posx;
    float starty = (float)cmd.pos_y + justifyy;
    const float justify_w = (float)cmd.justify_w;

    // LbTextDrawResizedImmediate() computes posx/starty (and everything
    // derived from them below) in a coordinate space local to
    // lbTextClipWindow -- it draws through a graphics window rebased to
    // (clip_x, clip_y), so the software renderer's actual screen position is
    // clip_x/clip_y + local coordinate. GL has no such window-offset
    // concept -- DrawGlyphQuad()/DrawSolidRect() take absolute screen
    // pixels -- so clip_x/clip_y have to be added back in explicitly at the
    // point a local (x, y) is materialized into an actual draw call
    // (FlushSegment() below), even though they're otherwise not needed for
    // any of the wrap/justify math itself (which the original algorithm
    // performs entirely in that same local space).
    const float clip_x = (float)cmd.clip_x;
    const float clip_y = (float)cmd.clip_y;

    long count = 0;
    const char* sbuf = cmd.text;
    const char* ebuf = cmd.text;

    while (*ebuf != '\0')
    {
        const char* seg_break = ebuf; // position just before this character
        size_t seq_len = 0;
        uint32_t chr = read_utf_8_codepoint(ebuf, &seq_len);
        if (seq_len == 0) break;
        ebuf += seq_len;

        if (chr > 32)
        {
            float w = char_width(chr);
            if (is_duospace(chr))
                count = 0;
            if ((posx + w - justifyx <= justify_w) || (count > 0) || !LbAlignMethodSet((unsigned short)state.flags))
            {
                posx += w;
                continue;
            }
            // Glyph doesn't fit and alignment is set -- emit the segment
            // built so far and wrap to the next line.
            w = char_width(' ');
            posx += w;
            long lstartx = (long)startx, lposx = (long)posx, lw = (long)w;
            float x = (float)LbGetJustifiedCharPosX(lstartx, lposx, lw, 1, (unsigned short)state.flags);
            float y = (float)LbGetJustifiedCharPosY((long)starty, (long)h, (long)h, (unsigned short)state.flags);
            float len = (float)LbGetJustifiedCharWidth(lposx, lw, count, ups, (unsigned short)state.flags);
            FlushSegment(sbuf, seg_break, x + clip_x, y + clip_y, len, ups, font, dbc_font, cmd, state);
            posx = startx;
            sbuf = seg_break;
            ebuf = seg_break;
            starty += h;
            count = 0;
        }
        else if ((chr == ' ') || (chr == 0xA0))
        {
            float w = char_width(' ');
            float wordw = word_width(ebuf);
            if (posx + w + wordw - justifyx <= justify_w)
            {
                count++;
                posx += w;
                continue;
            }
            posx += w;
            float x = (float)LbGetJustifiedCharPosX((long)startx, (long)posx, (long)w, 1, (unsigned short)state.flags);
            float y = (float)LbGetJustifiedCharPosY((long)starty, (long)h, (long)h, (unsigned short)state.flags);
            float len = (float)LbGetJustifiedCharWidth((long)posx, (long)w, count, ups, (unsigned short)state.flags);
            FlushSegment(sbuf, ebuf, x + clip_x, y + clip_y, len, ups, font, dbc_font, cmd, state);
            if (LbAlignMethodSet((unsigned short)state.flags))
            {
                posx = startx;
                sbuf = ebuf;
                starty += h;
            }
            count = 0;
        }
        else if (chr == '\n')
        {
            float x = (float)LbGetJustifiedCharPosX((long)startx, (long)posx, 0, 1, (unsigned short)state.flags);
            float len = char_width(' ');
            float y = starty;
            FlushSegment(sbuf, ebuf, x + clip_x, y + clip_y, len, ups, font, dbc_font, cmd, state);
            sbuf = ebuf;
            posx = startx;
            starty += h;
            count = 0;
        }
        else if (chr == '\t')
        {
            unsigned char spaces_per_tab = LbTextGetSpacesPerTab();
            float w = char_width(' ');
            posx += (float)spaces_per_tab * w;
            float len = word_width(ebuf);
            if (posx + len - justifyx <= justify_w)
            {
                count += spaces_per_tab;
                continue;
            }
            float x = (float)LbGetJustifiedCharPosX((long)startx, (long)posx, (long)w, spaces_per_tab, (unsigned short)state.flags);
            float y = (float)LbGetJustifiedCharPosY((long)starty, (long)h, (long)h, (unsigned short)state.flags);
            len = (float)LbGetJustifiedCharWidth((long)posx, (long)w, count, ups, (unsigned short)state.flags);
            FlushSegment(sbuf, ebuf, x + clip_x, y + clip_y, len, ups, font, dbc_font, cmd, state);
            if (LbAlignMethodSet((unsigned short)state.flags))
            {
                posx = startx;
                sbuf = ebuf;
                starty += h;
            }
            count = 0;
        }
        else if ((chr == DKChr_AlignLeft) || (chr == DKChr_AlignRight) || (chr == DKChr_AlignCenter))
        {
            // DKChr_AlignJustify shares the value of '\t' (see the enum's own
            // comment in bflib_sprfnt.h) -- the '\t' branch above already
            // claims that value, matching LbTextDrawResizedImmediate's own
            // if/else-if precedence exactly.
            if (posx - justifyx > justify_w)
            {
                float len = char_width(' ');
                FlushSegment(sbuf, ebuf, startx + clip_x, starty + clip_y, len, ups, font, dbc_font, cmd, state);
                posx = startx;
                sbuf = ebuf;
                count = 0;
                starty += h;
            }
            // Switches on chr (the align code just matched), not the next
            // byte -- LbTextDrawResizedImmediate's own switch reads *ebuf at
            // this point instead, which is the byte AFTER this align code
            // and only coincidentally matches one of these cases. develop's
            // ported shared layout engine (ITextRenderer::TextLayout()) uses
            // chr here too, so this matches its evidently-intentional fix
            // rather than perpetuating what reads as an oversight in the
            // original immediate-mode function (still used verbatim by the
            // software renderer, unchanged by this port).
            switch (chr)
            {
            case DKChr_AlignLeft:   state.flags ^= Lb_TEXT_HALIGN_LEFT;   break;
            case DKChr_AlignRight:  state.flags ^= Lb_TEXT_HALIGN_RIGHT;  break;
            case DKChr_AlignCenter: state.flags ^= Lb_TEXT_HALIGN_CENTER; break;
            }
        }
        // else: embedded colour-marker codepoints (>0xF100) already fall
        // into the chr>32 branch above (0 width, no wrap decision -- same as
        // the original); other control codes (transpar/outline/flip/
        // underline/one-colour toggles, 1-5 and 11-14) are pure passthrough
        // at this layout level, exactly like LbTextDrawResizedImmediate's
        // own if/else-if chain (no final else) -- FlushSegment() interprets
        // them when it re-scans this segment's text range.
    }

    float x = (float)LbGetJustifiedCharPosX((long)startx, (long)posx, 0, 1, (unsigned short)state.flags);
    float y = (float)LbGetJustifiedCharPosY((long)starty, (long)h, (long)h, (unsigned short)state.flags);
    float len = char_width(' ');
    FlushSegment(sbuf, ebuf, x + clip_x, y + clip_y, len, ups, font, dbc_font, cmd, state);
}

// Mirrors put_down_sprites() (bflib_sprfnt.c): draws one already-wrapped
// line segment [sbuf, ebuf), interpreting embedded colour-marker and
// toggle-flag control codes as it scans -- these mutate `state`, which
// persists across FlushSegment() calls within the same DrawGlyphs() call
// (matching how the software path mutates RendererGetDrawColour()/
// RendererGetDrawFlags() in place).
void GLTextRenderer::FlushSegment(const char* sbuf, const char* ebuf, float x, float y,
                                  float space_len, int units_per_px,
                                  const struct TbSpriteSheet* font, const struct AsianFont* dbc_font,
                                  const IRTextDrawCmd& cmd, DrawState& state)
{
    for (const char* c = sbuf; c < ebuf; )
    {
        size_t seq_len = 0;
        uint32_t chr = read_utf_8_codepoint(c, &seq_len);
        if (seq_len == 0) break;
        c += seq_len;

        if (chr > colour_modifiers_begin && chr < colour_modifiers_end)
        {
            state.colour = (uint8_t)(chr - colour_modifiers_begin);
        }
        else if ((chr == 0xA0) || (chr == ' '))
        {
            float w = space_len;
            if (state.flags & Lb_TEXT_UNDERLINE)
                EmitUnderline(x, y, w, LineHeightExplicit(font, dbc_font) * units_per_px / 16.0f, units_per_px, state);
            x += w;
        }
        else if (chr > 32)
        {
            float w = dbc_font ? DrawDbcGlyph(dbc_font, chr, x, y, units_per_px, state, cmd.dbc_colour0, cmd.dbc_colour1)
                                : DrawWesternGlyph(font, chr, x, y, units_per_px, state);
            x += w;
        }
        else if (chr == '\t')
        {
            float w = space_len * (float)LbTextGetSpacesPerTab();
            if (state.flags & Lb_TEXT_UNDERLINE)
                EmitUnderline(x, y, w, LineHeightExplicit(font, dbc_font) * units_per_px / 16.0f, units_per_px, state);
            x += w;
        }
        else
        {
            switch (chr)
            {
            case DKChr_Modifier_Transparent4: state.flags ^= Lb_SPRITE_TRANSPAR4;  break;
            case DKChr_Modifier_Transparent8: state.flags ^= Lb_SPRITE_TRANSPAR8;  break;
            case DKChr_Modifier_Outline:      state.flags ^= Lb_SPRITE_OUTLINE;    break;
            case DKChr_Modifier_FlipHoriz:    state.flags ^= Lb_SPRITE_FLIP_HORIZ; break;
            case DKChr_Modifier_FlipVertic:   state.flags ^= Lb_SPRITE_FLIP_VERTIC;break;
            case DKChr_Modifier_Underline:    state.flags ^= Lb_TEXT_UNDERLINE;    break;
            case DKChr_Modifier_OneColor:     state.flags ^= Lb_TEXT_ONE_COLOR;    break;
            default: break; // DKChr_NewLine/Return/Colour: no-op here, matches put_down_sprites()
            }
        }
    }
}

// Mirrors draw_simpletext_char()'s exact mode selection: the DEFAULT case
// (neither ONE_COLOR nor REMAP set -- the common one, plain menu/HUD text)
// shows the glyph in its own baked-in palette colour, no substitution at
// all. Only Lb_TEXT_ONE_COLOR opts into a solid draw_colour override.
// Lb_TEXT_REMAP (a fade_table index substitution) isn't ported here yet --
// falls back to the default, unremapped glyph colour rather than drawing
// something actively wrong; a disclosed gap, not silently different from
// develop.
float GLTextRenderer::DrawWesternGlyph(const struct TbSpriteSheet* font, uint32_t chr,
                                       float x, float y, int units_per_px, const DrawState& state)
{
    const struct TbSprite* spr = LbFontCharSprite(font, chr);
    if (!spr || spr->SWidth == 0 || spr->SHeight == 0)
        return 0.0f;

    const bool one_colour = (state.flags & Lb_TEXT_ONE_COLOR) != 0;
    float r = 1, g = 1, b = 1;
    if (one_colour)
    {
        const unsigned char* pal = RendererGetActivePalette();
        if (pal)
        {
            r = chan6_to_unit(pal[state.colour * 3 + 0]);
            g = chan6_to_unit(pal[state.colour * 3 + 1]);
            b = chan6_to_unit(pal[state.colour * 3 + 2]);
        }
    }
    const float a = text_alpha_from_draw_flags(state.flags);

    SpriteHandle h = m_ui->ResolveSprite(spr);
    m_ui->DrawGlyphQuad(h, x, y, units_per_px, r, g, b, a, /*sample_palette=*/!one_colour);

    float w = spr->SWidth * units_per_px / 16.0f;
    if (state.flags & Lb_TEXT_UNDERLINE)
        EmitUnderline(x, y, w, (float)LbSprFontCharHeight(font, ' ') * units_per_px / 16.0f, units_per_px, state);
    return w;
}

// Mirrors draw_dbc_char()/dbc_draw_font_sprite_text()'s colour selection and
// shadow pass. The GL glyph atlas caches the bitmap at native (16 units-per-
// px baseline) resolution -- DrawGlyphQuad() scales the quad geometry by
// units_per_px itself, so unlike the CPU path (which manually resamples the
// bitmap for non-16 scales), position metadata (vertical_offset, character
// spacing) is scaled here but the shadow's 1-pixel offset is not -- it's a
// flat output-pixel offset in the CPU path too, applied after that path's
// own resampling.
float GLTextRenderer::DrawDbcGlyph(const struct AsianFont* dbc_font, uint32_t chr,
                                   float x, float y, int units_per_px, const DrawState& state,
                                   long face_colour, long shadow_colour)
{
    SpriteHandle handle = m_ui->ResolveDbcGlyph(dbc_font, chr);
    if (handle == kInvalidSpriteHandle)
        return 0.0f;

    const unsigned char* data = nullptr;
    int scanline = 0, glyph_w = 0, glyph_h = 0, spacing = 0, voffset = 0;
    if (LbDbcGetGlyphBits(dbc_font, chr, &data, &scanline, &glyph_w, &glyph_h, &spacing, &voffset) != 0)
        return 0.0f;

    const bool one_colour = (state.flags & Lb_TEXT_ONE_COLOR) != 0;
    // Lb_TEXT_REMAP isn't ported for DBC glyphs either -- same disclosed gap
    // as DrawWesternGlyph(), falls back to the unremapped colour.
    long colour_idx = one_colour ? (long)state.colour : face_colour;

    const unsigned char* pal = RendererGetActivePalette();
    float r = 1, g = 1, b = 1, sr = 0, sg = 0, sb = 0;
    if (pal)
    {
        r  = chan6_to_unit(pal[colour_idx    * 3 + 0]);
        g  = chan6_to_unit(pal[colour_idx    * 3 + 1]);
        b  = chan6_to_unit(pal[colour_idx    * 3 + 2]);
        sr = chan6_to_unit(pal[shadow_colour * 3 + 0]);
        sg = chan6_to_unit(pal[shadow_colour * 3 + 1]);
        sb = chan6_to_unit(pal[shadow_colour * 3 + 2]);
    }
    const float a = text_alpha_from_draw_flags(state.flags);
    const float scale = units_per_px / 16.0f;
    const float gy = y + (float)voffset * scale;

    // Drop shadow, always drawn (this branch's dbc_colour1 is always a
    // resolved palette index, never the "off" sentinel the CPU function
    // also supports via a negative colr3).
    m_ui->DrawGlyphQuad(handle, x + 1.0f, gy + 1.0f, units_per_px, sr, sg, sb, a, /*sample_palette=*/false);
    m_ui->DrawGlyphQuad(handle, x, gy, units_per_px, r, g, b, a, /*sample_palette=*/false);

    // Mirrors draw_dbc_char()'s own (slightly odd, height==16-gated) advance
    // width formula verbatim.
    float advance = (glyph_h == 16) ? (float)(spacing + glyph_w) * scale : (float)(spacing + glyph_w);
    if (state.flags & Lb_TEXT_UNDERLINE)
        EmitUnderline(x, y, advance, (float)LbDbcCharHeight(dbc_font) * scale, units_per_px, state);
    return advance;
}

// Mirrors LbDrawCharUnderline() (bflib_sprfnt.c) verbatim, including its
// shadow-then-main stacking (the main loop continues decrementing `h` from
// wherever the shadow loop left it, rather than resetting -- ported as-is,
// not "fixed", since this determines the exact row layout of a double
// underline/shadowed underline and nothing indicates it's unintentional).
void GLTextRenderer::EmitUnderline(float x, float y, float w, float height, int units_per_px, const DrawState& state)
{
    if (!(state.flags & Lb_TEXT_UNDERLINE) || w <= 0.0f || !m_ui)
        return;

    int ups = units_per_px < 1 ? 1 : units_per_px;
    long base_height = (long)(height * 16.0f / (float)ups);
    long thickness = ((base_height > 16) ? 2 : 1) * units_per_px / 16;
    if (thickness < 1) thickness = 1;

    const unsigned char* pal = RendererGetActivePalette();
    float dr = 1, dg = 1, db = 1;
    if (pal)
    {
        dr = chan6_to_unit(pal[state.colour * 3 + 0]);
        dg = chan6_to_unit(pal[state.colour * 3 + 1]);
        db = chan6_to_unit(pal[state.colour * 3 + 2]);
    }
    const float a = text_alpha_from_draw_flags(state.flags);

    float h = height;
    if (state.flags & Lb_TEXT_UNDERLNSHADOW)
    {
        long shadow_off = ((base_height > 32) ? 2 : 1) * units_per_px / 16;
        if (shadow_off < 1) shadow_off = 1;
        float shadow_x = x + (float)shadow_off;
        unsigned char shadow_colour = lbDisplayEx.ShadowColour;
        float sr = 0, sg = 0, sb = 0;
        if (pal)
        {
            sr = chan6_to_unit(pal[shadow_colour * 3 + 0]);
            sg = chan6_to_unit(pal[shadow_colour * 3 + 1]);
            sb = chan6_to_unit(pal[shadow_colour * 3 + 2]);
        }
        for (long i = 0; i < thickness; i++)
        {
            m_ui->DrawSolidRect(shadow_x, y + h, w, 1.0f, sr, sg, sb, a);
            h -= 1.0f;
        }
    }
    for (long i = 0; i < thickness; i++)
    {
        m_ui->DrawSolidRect(x, y + h, w, 1.0f, dr, dg, db, a);
        h -= 1.0f;
    }
}
