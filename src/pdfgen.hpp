/*
 * Copyright 2023 Jussi Pakkanen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <pdfdrawcontext.hpp>

#include <cstdio>
#include <cstdint>

#include <vector>
#include <string_view>
#include <optional>
#include <filesystem>

namespace A4PDF {

class PdfGen;

struct DrawContextPopper {
    PdfGen *g;
    PdfDrawContext ctx;

    explicit DrawContextPopper(PdfGen *g,
                               PdfDocument *doc,
                               PdfColorConverter *cm,
                               A4PDF_Draw_Context_Type dtype)
        : g{g}, ctx{doc, cm, dtype} {}

    DrawContextPopper() = delete;
    DrawContextPopper(const DrawContextPopper &) = delete;

    ~DrawContextPopper();
};

class PdfGen {
public:
    static rvoe<std::unique_ptr<PdfGen>> construct(const char *ofname, const PdfGenerationData &d);
    PdfGen(std::filesystem::path ofilename,
           std::unique_ptr<FT_LibraryRec_, FT_Error (*)(FT_LibraryRec_ *)> ft,
           PdfDocument pdoc)
        : ofilename(std::move(ofilename)), ft(std::move(ft)), pdoc(std::move(pdoc)) {}
    PdfGen(PdfGen &&o) = default;
    ~PdfGen();

    ErrorCode write();
    void new_page();

    rvoe<A4PDF_ImageId> load_image(const char *fname) { return pdoc.load_image(fname); }
    rvoe<A4PDF_ImageId> embed_jpg(const char *fname) { return pdoc.embed_jpg(fname); }
    rvoe<A4PDF_FontId> load_font(const char *fname) { return pdoc.load_font(ft.get(), fname); };
    ImageSize get_image_info(A4PDF_ImageId img_id) { return pdoc.image_info.at(img_id.id).s; }
    SeparationId create_separation(std::string_view name, const DeviceCMYKColor &fallback) {
        return pdoc.create_separation(name, fallback);
    }
    GstateId add_graphics_state(const GraphicsState &state) {
        return pdoc.add_graphics_state(state);
    }

    FunctionId add_function(const FunctionType2 &func) { return pdoc.add_function(func); }

    ShadingId add_shading(const ShadingType2 &shade) { return pdoc.add_shading(shade); }
    ShadingId add_shading(const ShadingType3 &shade) { return pdoc.add_shading(shade); }

    LabId add_lab_colorspace(const LabColorSpace &lab) { return pdoc.add_lab_colorspace(lab); }

    rvoe<A4PDF_IccColorSpaceId> load_icc_file(const char *fname) {
        return pdoc.load_icc_file(fname);
    }

    rvoe<A4PDF_AnnotationId> create_form_checkbox(PdfBox loc,
                                                  A4PDF_FormXObjectId onstate,
                                                  A4PDF_FormXObjectId offstate,
                                                  std::string_view partial_name) {
        return pdoc.create_form_checkbox(loc, onstate, offstate, partial_name);
    }

    DrawContextPopper guarded_page_context();
    PdfDrawContext *new_page_draw_context();

    PdfDrawContext new_form_xobject(double w, double h) {
        return PdfDrawContext(&this->pdoc, &pdoc.cm, A4PDF_Form_XObject_Context, w, h);
    }

    ColorPatternBuilder new_color_pattern_builder(double w, double h);

    rvoe<PageId> add_page(PdfDrawContext &ctx);
    rvoe<A4PDF_FormXObjectId> add_form_xobject(PdfDrawContext &ctx);
    rvoe<PatternId> add_pattern(ColorPatternBuilder &cp);

    OutlineId
    add_outline(std::string_view title_utf8, PageId dest, std::optional<OutlineId> parent) {
        return pdoc.add_outline(title_utf8, dest, parent);
    }

    int32_t num_pages() const { return (int32_t)pdoc.pages.size(); }

    std::optional<double>
    glyph_advance(A4PDF_FontId fid, double pointsize, uint32_t codepoint) const {
        return pdoc.glyph_advance(fid, pointsize, codepoint);
    }

    rvoe<double> utf8_text_width(const char *utf8_text, A4PDF_FontId fid, double pointsize) const;

private:
    std::filesystem::path ofilename;
    std::unique_ptr<FT_LibraryRec_, FT_Error (*)(FT_LibraryRec_ *)> ft;
    PdfDocument pdoc;
};

struct GenPopper {
    std::unique_ptr<PdfGen> g;
    GenPopper(const char *ofname, const PdfGenerationData &d)
        : g(PdfGen::construct(ofname, d).value()) {}
    ~GenPopper() {
        auto rc = g->write();
        if(rc != ErrorCode::NoError) {
            fprintf(stderr, "%s\n", error_text(rc));
            std::abort();
        }
    }
};

} // namespace A4PDF
