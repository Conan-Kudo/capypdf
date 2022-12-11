/*
 * Copyright 2022 Jussi Pakkanen
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

#include <pdfgen.hpp>

namespace {

constexpr double mm2pt(const double x) { return x * 2.8346456693; }

const double page_w = mm2pt(130);
const double page_h = mm2pt(210);
const double spine_w = mm2pt(10);
const double bleed = mm2pt(10);
const double margin = mm2pt(20);

const double paper_height = page_h + 2 * margin;
const double paper_width = 2 * (margin + page_w) + spine_w;

void draw_registration_cross(PdfPage &ctx, double x, double y) {
    const double cross_size = mm2pt(10); // diameter, not radius
    const double circle_size = 0.6 * cross_size;
    ctx.cmd_q();
    ctx.translate(x, y);
    ctx.cmd_m(-cross_size / 2, 0);
    ctx.cmd_l(cross_size / 2, 0);
    ctx.cmd_m(0, -cross_size / 2);
    ctx.cmd_l(0, cross_size / 2);
    ctx.cmd_S();
    ctx.cmd_w(1 / circle_size);
    ctx.scale(circle_size, circle_size);
    ctx.draw_unit_circle();
    ctx.cmd_S();
    ctx.cmd_Q();
}

void draw_colorbox(PdfPage &ctx,
                   double box_size,
                   double xloc,
                   double yloc,
                   double c,
                   double m,
                   double y,
                   double k) {
    ctx.cmd_q();
    ctx.translate(xloc, yloc);
    ctx.scale(box_size, box_size);
    ctx.cmd_k(c, m, y, k);
    ctx.draw_unit_box();
    ctx.cmd_f();
    ctx.cmd_Q();
}

void draw_colorbar(PdfPage &ctx) {
    const double box_size = mm2pt(5);
    draw_colorbox(ctx, box_size, 2 * margin, margin / 2, 1.0, 0.0, 0.0, 0.0);
    draw_colorbox(ctx, box_size, 2 * margin + box_size, margin / 2, 0.0, 1.0, 0.0, 0.0);
    draw_colorbox(ctx, box_size, 2 * margin + 2 * box_size, margin / 2, 0.0, 0.0, 1.0, 0.0);
    draw_colorbox(ctx, box_size, 2 * margin + 3 * box_size, margin / 2, 1.0, 1.0, 0.0, 0.0);
    draw_colorbox(ctx, box_size, 2 * margin + 4 * box_size, margin / 2, 1.0, 0.0, 1.0, 0.0);
    draw_colorbox(ctx, box_size, 2 * margin + 5 * box_size, margin / 2, 0.0, 1.0, 1.0, 0.0);
}

void draw_graybar(PdfPage &ctx) {
    const double box_size = mm2pt(5);
    const double xloc = paper_width / 2 + margin;
    const double yloc = paper_height - margin / 2;
    for(int i = 1; i < 11; ++i) {
        draw_colorbox(ctx, box_size, xloc + i * box_size, yloc, 0.0, 0.0, 0.0, i / 10.0);
    }
}

void draw_printer_marks(PdfPage &ctx) {
    draw_registration_cross(ctx, margin / 2, paper_height / 2);
    draw_registration_cross(ctx, paper_width - margin / 2, paper_height / 2);
    draw_registration_cross(ctx, paper_width / 2, margin / 2);
    draw_registration_cross(ctx, paper_width / 2, paper_height - margin / 2);
}

} // namespace

int main(int argc, char **argv) {
    PdfGenerationData opts;
    opts.page_size.h = paper_height;
    opts.page_size.w = paper_width;

    opts.mediabox.x = opts.mediabox.y = 0;
    opts.mediabox.w = opts.page_size.w;
    opts.mediabox.h = opts.page_size.h;

    opts.title = "Book cover generation experiment";
    opts.author = "G. R. Aphicdesigner";
    opts.output_colorspace = PDF_DEVICE_CMYK;

    try {
        PdfGen gen("cover.pdf", opts);
        {
            auto ctx = gen.new_page();
            ctx.cmd_w(1.0);
            ctx.set_nonstroke_color(DeviceRGBColor{0, 0, 0});
            draw_printer_marks(ctx);
            draw_colorbar(ctx);
            draw_graybar(ctx);
            ctx.cmd_re(
                margin, margin, opts.page_size.w - 2 * margin, opts.page_size.h - 2 * margin);
            ctx.cmd_S();
        }
    } catch(const std::exception &e) {
        printf("ERROR: %s\n", e.what());
        return 1;
    }

    return 0;
}
