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

int main() {
    PdfGenerationData opts;
    opts.page_size = Area::a4();
    opts.mediabox.x = opts.mediabox.y = 0;
    opts.mediabox.w = opts.page_size.w;
    opts.mediabox.h = opts.page_size.h;
    PdfGen gen("test.pdf", opts);
    return 0;
}
