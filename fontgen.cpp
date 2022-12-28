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

int main(int argc, char **argv) {
    PdfGenerationData opts;
    opts.page_size.h = 100;
    opts.page_size.w = 400;
    const char *fontfile;
    if(argc > 1) {
        fontfile = argv[1];
    } else {
        fontfile = "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf";
    }
    opts.mediabox.x = opts.mediabox.y = 0;
    opts.mediabox.w = opts.page_size.w;
    opts.mediabox.h = opts.page_size.h;
    opts.title = "Font layout test";
    PdfGen gen("fonttest.pdf", opts);
    auto ctx = gen.new_page();
    auto fid = gen.load_font(fontfile);
    ctx.render_utf8_text(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖ abcdefghijklmnopqrstuvwxyzåäö", fid, 10, 5, 10);
    ctx.render_utf8_text("0123456789", fid, 12, 25, 32);
    return 0;
}
