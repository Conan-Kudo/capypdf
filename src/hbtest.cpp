// SPDX-License-Identifier: Apache-2.0
// Copyright 2022-2024 Jussi Pakkanen

#include <pdfgen.hpp>
#include <pdftext.hpp>
#include <pdfcommon.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>

#include <cstdio>
#include <cassert>
#include <cstring>

#define CHCK(command)                                                                              \
    {                                                                                              \
        auto rc = command;                                                                         \
        if(!rc) {                                                                                  \
            fprintf(stderr, "%s\n", error_text(rc.error()));                                       \
            std::abort();                                                                          \
        }                                                                                          \
    }

using namespace capypdf;

// The FFI ligature has Unicode codepoint U+FB03 (64259)
// In Noto Serif it has glyph id 2132

// const char sampletext[] = "This is sample text. AV To.";
const char sampletext[] = "Affi.";
const char fontfile[] = "/usr/share/fonts/truetype/noto/NotoSerif-Regular.ttf";
// const char fontfile[] = "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf";
const double ptsize = 12;

size_t
get_endpoint(hb_glyph_info_t *glyph_info, size_t glyph_count, size_t i, const char *sampletext) {
    if(i + 1 < glyph_count) {
        return glyph_info[i + 1].cluster;
    }
    return strlen(sampletext);
}

void do_harfbuzz(PdfGen &gen, PdfDrawContext &ctx, CapyPDF_FontId pdffont) {
    FT_Library ft;
    FT_Face ftface;
    FT_Error fte;
    fte = FT_Init_FreeType(&ft);
    assert(fte == 0);
    fte = FT_New_Face(ft, fontfile, 0, &ftface);
    assert(fte == 0);

    hb_buffer_t *buf;
    const double num_steps = 64;
    const double hbscale = ptsize * num_steps;
    buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, sampletext, -1, 0, -1);
    hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
    hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
    hb_buffer_set_language(buf, hb_language_from_string("en", -1));

    hb_buffer_guess_segment_properties(buf);

    hb_blob_t *blob = hb_blob_create_from_file(fontfile);
    hb_face_t *face = hb_face_create(blob, 0);
    hb_font_t *font = hb_font_create(face);
    hb_font_set_scale(font, hbscale, hbscale);

    hb_shape(font, buf, nullptr, 0);
    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
    double cursor_x = 10;
    double cursor_y = 100;
    //    assert(strlen(sampletext) == glyph_count);

    TextSequence full_line;
    for(unsigned int i = 0; i < glyph_count; i++) {
        const hb_glyph_info_t *current = glyph_info + i;
        const hb_glyph_position_t *curpos = glyph_pos + i;
        hb_codepoint_t glyphid = current->codepoint;
        hb_position_t x_offset = curpos->x_offset;
        // hb_position_t y_offset = curpos->y_offset;
        hb_position_t x_advance = curpos->x_advance;
        hb_position_t y_advance = curpos->y_advance;
        std::string original_text(sampletext + glyph_info[i].cluster,
                                  sampletext +
                                      get_endpoint(glyph_info, glyph_count, i, sampletext));
        // FIXME: max inefficient. Creates a text object per glyph.
        PdfText txt(&ctx);
        CHCK(txt.cmd_Tf(pdffont, ptsize));
        txt.cmd_Td(cursor_x, cursor_y);
        // auto ft_glyphid = FT_Get_Char_Index(ftface, sampletext[i]);
        // assert(ft_glyphid == glyphid);
        fte = FT_Load_Glyph(ftface, glyphid, 0);
        assert(fte == 0);
        const auto hb_advance_in_font_units = x_advance / hbscale * ftface->units_per_EM;
        printf(
            "%s %u %d %.2f\n", original_text.c_str(), glyphid, x_offset, hb_advance_in_font_units);
        printf("  %f\n",
               ftface->glyph->advance.x - hb_advance_in_font_units); // / ftface->units_per_EM);
        cursor_x += x_advance / num_steps;
        cursor_y += y_advance / num_steps;
        CHCK(ctx.render_text(txt));
        const int32_t kerning_delta = int32_t(
            (ftface->glyph->advance.x - hb_advance_in_font_units) / ftface->units_per_EM * 1000);
        full_line.append_raw_glyph(glyphid, 0);
        if(kerning_delta != 0) {
            full_line.append_kerning(kerning_delta);
        }
    }
    {
        auto p = ctx.push_gstate();
        ctx.translate(10, 90);
        PdfText txt(&ctx);
        CHCK(txt.cmd_Tf(pdffont, ptsize));
        CHCK(txt.cmd_TJ(full_line));
        CHCK(ctx.render_text(txt));
    }
    hb_buffer_destroy(buf);
    hb_font_destroy(font);
    hb_face_destroy(face);
    hb_blob_destroy(blob);
}

void hardcoded() {
    PdfGenerationData opts;
    opts.default_page_properties.mediabox = PdfRectangle(0, 0, 200, 200);
    opts.lang = asciistring::from_cstr("en-US").value();
    GenPopper genpop("shapedtext.pdf", opts);
    PdfGen &gen = *genpop.g;

    auto ctxguard = gen.guarded_page_context();
    auto &ctx = ctxguard.ctx;

    auto pdffont = gen.load_font(fontfile).value();
    TextSequence ts;
    ts.append_unicode('A');
    ctx.translate(10, 100);
    CHCK(ts.append_actualtext_start(u8string::from_cstr("ffi").value()));
    //ts.append_raw_glyph(2132, 0xFB03);
    ts.append_unicode(0xFB03);
    CHCK(ts.append_actualtext_end());
    ts.append_unicode('.');
    PdfText txt(&ctx);
    CHCK(txt.cmd_Tf(pdffont, ptsize));
    CHCK(txt.cmd_TJ(ts));
    CHCK(ctx.render_text(txt));
}

void whole_shebang() {
    PdfGenerationData opts;
    opts.default_page_properties.mediabox = PdfRectangle(0, 0, 200, 200);
    opts.lang = asciistring::from_cstr("en-US").value();
    GenPopper genpop("harfbuzz.pdf", opts);
    PdfGen &gen = *genpop.g;

    auto ctxguard = gen.guarded_page_context();
    auto &ctx = ctxguard.ctx;

    auto pdffont = gen.load_font(fontfile).value();
    ctx.render_text(capypdf::u8string::from_cstr(sampletext).value(), pdffont, ptsize, 10, 110);
    do_harfbuzz(gen, ctx, pdffont);
}

int main(int, char **) {
    hardcoded();
    // whole_shebang();
    return 0;
}
