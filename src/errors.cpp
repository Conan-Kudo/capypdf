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

#include <errors.hpp>

namespace A4PDF {

// clang-format off

const std::array<const char *, (std::size_t)ErrorCode::NumErrors> error_texts{
"No error.",
"Invalid index.",
"Negative line width.",
"No pages defined.",
"Color component out of range.",
"Bad ID number.",
};

// clang-format on

const char *error_text(ErrorCode ec) noexcept {
    const int index = (int32_t)ec;
    if(index < 0 || (std::size_t)index >= error_texts.size()) {
        return "Invalid error code.";
    }
    return error_texts[index];
}

} // namespace A4PDF
