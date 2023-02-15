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

#include <imageops.hpp>
#include <utils.hpp>
#include <png.h>
#include <jpeglib.h>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <memory>

namespace A4PDF {

namespace {

rgb_image load_rgb_png(png_image &image) {
    rgb_image result;
    result.w = image.width;
    result.h = image.height;
    result.pixels.resize(PNG_IMAGE_SIZE(image));

    png_image_finish_read(
        &image, NULL, result.pixels.data(), PNG_IMAGE_SIZE(image) / image.height, NULL);
    if(PNG_IMAGE_FAILED(image)) {
        std::string msg("PNG file reading failed: ");
        msg += image.message;
        throw std::runtime_error(std::move(msg));
    }
    return result;
}

rgb_image load_rgba_png(png_image &image) {
    rgb_image result;
    std::string buf;
    result.w = image.width;
    result.h = image.height;
    buf.resize(PNG_IMAGE_SIZE(image));
    result.alpha = std::string();

    png_image_finish_read(&image, NULL, buf.data(), PNG_IMAGE_SIZE(image) / image.height, NULL);
    if(PNG_IMAGE_FAILED(image)) {
        std::string msg("PNG file reading failed: ");
        msg += image.message;
        throw std::runtime_error(std::move(msg));
    }
    assert(buf.size() % 4 == 0);
    result.pixels.reserve(PNG_IMAGE_SIZE(image) * 3 / 4);
    result.alpha->reserve(PNG_IMAGE_SIZE(image) / 4);
    for(size_t i = 0; i < buf.size(); i += 4) {
        result.pixels += buf[i];
        result.pixels += buf[i + 1];
        result.pixels += buf[i + 2];
        *result.alpha += buf[i + 3];
    }

    return result;
}

mono_image load_mono_png(png_image &image) {
    mono_image result;
    std::string buf;
    result.w = image.width;
    result.h = image.height;
    size_t final_size = (result.w + 7) / 8 * result.h;
    result.pixels.reserve(final_size);

    auto bufsize = PNG_IMAGE_SIZE(image);
    buf.resize(bufsize);
    std::string colormap;
    colormap.resize(PNG_IMAGE_COLORMAP_SIZE(image));

    png_image_finish_read(
        &image, NULL, buf.data(), PNG_IMAGE_SIZE(image) / image.height, colormap.data());
    if(PNG_IMAGE_FAILED(image)) {
        std::string msg("PNG file reading failed: ");
        msg += image.message;
        throw std::runtime_error(std::move(msg));
    }
    size_t offset = 0;
    const int white_pixel = colormap[0] == 1 ? 1 : 0;
    const int num_padding_bits = (result.w % 8) == 0 ? 0 : 8 - result.w % 8;
    for(int j = 0; j < result.h; ++j) {
        unsigned char current_byte = 0;
        for(int i = 0; i < result.w; ++i) {
            current_byte <<= 1;
            if(buf[offset] == white_pixel) {
                current_byte |= 1;
            }
            if((i % 8 == 0) && i > 0) {
                result.pixels.push_back(~current_byte);
                current_byte = 0;
            }
            ++offset;
        }
        // PDF spec 8.9.3 "Sample representation"

        if(num_padding_bits > 0) {
            current_byte <<= num_padding_bits;
            result.pixels.push_back(~current_byte);
        }
    }
    auto blub = result.pixels.size();
    assert(result.pixels.size() == final_size);
    return result;
}

} // namespace

RasterImage load_image_file(const char *fname) {
    png_image image;
    std::unique_ptr<png_image, decltype(&png_image_free)> pngcloser(&image, &png_image_free);

    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;

    if(png_image_begin_read_from_file(&image, fname) != 0) {
        if(image.format == PNG_FORMAT_RGBA) {
            return load_rgba_png(image);
        } else if(image.format == PNG_FORMAT_RGB) {
            return load_rgb_png(image);
        } else if(image.format & PNG_FORMAT_FLAG_COLORMAP) {
            if(!(image.format & PNG_FORMAT_FLAG_COLOR)) {
                throw std::runtime_error("Colormap format not supported.\n");
            }
            if(image.colormap_entries != 2) {
                throw std::runtime_error("Only monochrome colormap images supported.\n");
            }
            return load_mono_png(image);
        } else {
            throw std::runtime_error("Only RGB images supported.");
        }
    } else {
        std::string msg("Opening a PNG file failed: ");
        msg += image.message;
        throw std::runtime_error(std::move(msg));
    }
    throw std::runtime_error("Unreachable code.");
}

jpg_image load_jpg(const char *fname) {
    jpg_image im;
    im.file_contents = load_file(fname);
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    std::unique_ptr<jpeg_decompress_struct, decltype(&jpeg_destroy_decompress)> jpgcloser(
        &cinfo, &jpeg_destroy_decompress);

    jpeg_mem_src(&cinfo, (const unsigned char *)im.file_contents.data(), im.file_contents.length());
    if(jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        throw std::runtime_error(std::string{"Not a valid jpg file: "} + fname);
    }
    jpeg_start_decompress(&cinfo);
    im.h = cinfo.image_height;
    im.w = cinfo.image_width;
    // Fixme, validate that this is an 8bpp RGB image.
    return im;
}

} // namespace A4PDF
