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

#include <pdfpage.hpp>
#include <pdfgen.hpp>
#include <fmt/core.h>

PdfPage::PdfPage(PdfGen *g) : g(g) {}

PdfPage::~PdfPage() {
    try {
        if(!is_finalized) {
            finalize();
        }
    } catch(const std::exception &e) {
        printf("FAIL: %s\n", e.what());
    }
}

void PdfPage::finalize() {
    if(is_finalized) {
        throw std::runtime_error("Tried to finalize a page object twice.");
    }
    is_finalized = true;
    std::string buf;
    build_resource_dict();
    fmt::format_to(std::back_inserter(buf),
                   R"(<<
  /Length {}
>>
stream
{}
endstream
)",
                   commands.size(),
                   commands);
    g->add_page(resources, buf);
}

void PdfPage::build_resource_dict() {
    resources = "<<\n";
    if(!used_images.empty()) {
        resources += "  /XObject <<\n";
        for(const auto &i : used_images) {
            fmt::format_to(std::back_inserter(resources), "    /Image{} {} 0 R\n", i, i);
        }

        resources += "  >>";
    }
    if(!used_fonts.empty()) {
        resources += "  /Font <<\n";
        for(const auto &i : used_fonts) {
            fmt::format_to(std::back_inserter(resources), "    /Font{} {} 0 R\n", i, i);
        }
        resources += "  >>\n";
    }
    resources += ">>\n";
}

void PdfPage::save() { commands += "q\n"; }

void PdfPage::restore() { commands += "Q\n"; }

void PdfPage::rectangle(double x, double y, double w, double h) {
    fmt::format_to(std::back_inserter(commands), "{} {} {} {} re\n", x, y, w, h);
}

void PdfPage::fill() { commands += "f\n"; }

void PdfPage::stroke() { commands += "S\n"; }

void PdfPage::set_line_width(double w) {
    fmt::format_to(std::back_inserter(commands), "{} w\n", w);
}

void PdfPage::set_stroke_color(const DeviceRGBColor &c) {
    fmt::format_to(std::back_inserter(commands), "{} {} {} RG\n", c.r.v(), c.g.v(), c.b.v());
}

void PdfPage::set_nonstroke_color(const DeviceRGBColor &c) {
    fmt::format_to(std::back_inserter(commands), "{} {} {} rg\n", c.r.v(), c.g.v(), c.b.v());
}

void PdfPage::draw_image(int32_t obj_num) {
    used_images.insert(obj_num);
    fmt::format_to(std::back_inserter(commands), "/Image{} Do\n", obj_num);
}

void PdfPage::concatenate_matrix(double m1, double m2, double m3, double m4, double m5, double m6) {
    fmt::format_to(std::back_inserter(commands), "{} {} {} {} {} {} cm\n", m1, m2, m3, m4, m5, m6);
}

void PdfPage::scale(double xscale, double yscale) {
    concatenate_matrix(xscale, 0, 0, yscale, 0, 0);
}
void PdfPage::translate(double xtran, double ytran) {
    concatenate_matrix(0, 0, 0, 0, xtran, ytran);
}

void PdfPage::simple_text(
    const char *u8text, int32_t font_id, double pointsize, double x, double y) {
    used_fonts.insert(font_id);
    fmt::format_to(std::back_inserter(commands),
                   R"(BT
  /Font{} {} Tf
  {} {} Td
  ({}) Tj
ET
)",
                   font_id,
                   pointsize,
                   x,
                   y,
                   u8text);
}
