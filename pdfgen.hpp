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

#include <pdfpage.hpp>
#include <pdfcolorconverter.hpp>

#include <cstdio>
#include <cstdint>

#include <vector>
#include <string_view>
#include <string>
#include <unordered_map>
#include <optional>
#include <memory>
#include <variant>

// To avoid pulling all of LittleCMS in this file.
typedef void *cmsHPROFILE;
// Ditto for Freetype
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_FaceRec_ *FT_Face;
typedef int FT_Error;

FT_Error guarded_face_close(FT_Face face);

struct TtfFont {
    std::unique_ptr<FT_FaceRec_, FT_Error (*)(FT_Face)> face;
    std::string fontdata;
};

struct PdfBox {
    double x;
    double y;
    double w;
    double h;
};

struct Area {
    double w;
    double h;

    static Area a4() { return Area{595.28, 841.89}; }
};

struct ColorProfiles {
    const char *rgb_profile_file = nullptr;
    const char *gray_profile_file = nullptr;
    const char *cmyk_profile_file = nullptr;
};

struct PdfGenerationData {
    Area page_size;
    PdfBox mediabox;
    std::optional<PdfBox> cropbox;
    std::optional<PdfBox> bleedbox;
    std::optional<PdfBox> trimbox;
    std::optional<PdfBox> artbox;

    std::string title;
    std::string author;
    PdfColorSpace output_colorspace = PDF_DEVICE_RGB;
    ColorProfiles prof;
};

struct PageOffsets {
    int32_t resource_obj_num;
    int32_t commands_obj_num;
};

struct ImageSize {
    int32_t w;
    int32_t h;
};

struct ImageInfo {
    ImageSize s;
    int32_t obj;
};

struct FontInfo {
    int32_t font_file_obj;
    int32_t font_descriptor_obj;
    int32_t font_obj;
    size_t font_index_tmp;
};

struct FullPDFObject {
    std::string dictionary;
    std::string stream;
};

struct DelayedFontData {
    size_t font_offset;
};

struct DelayedFontDescriptor {
    size_t font_offset;
    int32_t font_file_object;
};

struct DelayedCmap {
    size_t font_offset;
};

struct DelayedFont {
    size_t font_offset;
    int32_t to_unicode_obj;
    int32_t font_descriptor_obj;
};

struct FontSubset {
    FontId fid;
    int32_t subset_id;

    bool operator==(const FontSubset &other) const {
        return fid.id == other.fid.id && subset_id == other.subset_id;
    }

    bool operator!=(const FontSubset &other) const { return !(*this == other); }

    FontSubset &operator=(const FontSubset &other) {
        if(this != &other) {
            fid = other.fid;
            subset_id = other.subset_id;
        }
        return *this;
    }
};

struct SubsetGlyph {
    FontSubset ss;
    uint8_t glyph_id;
};

typedef std::
    variant<FullPDFObject, DelayedFontData, DelayedFontDescriptor, DelayedFont, DelayedCmap>
        ObjectType;

class PdfGen {
public:
    explicit PdfGen(const char *ofname, const PdfGenerationData &d);
    ~PdfGen();

    PdfPage new_page();

    int32_t add_object(ObjectType object);
    void add_page(std::string_view resource_data, std::string_view page_data);

    ImageId load_image(const char *fname);
    FontId load_font(const char *fname);
    ImageSize get_image_info(ImageId img_id) { return image_info.at(img_id.id).s; }
    SeparationId create_separation(std::string_view name, const DeviceCMYKColor &fallback);

    SubsetGlyph get_subset_glyph(FontId fid, uint32_t glyph);

    friend class PdfPage;

private:
    FontId get_builtin_font_id(BuiltinFonts font);
    int32_t image_object_number(ImageId iid) { return image_info.at(iid.id).obj; }
    int32_t font_object_number(FontId fid) { return font_objects.at(fid.id).font_obj; }
    int32_t separation_object_number(SeparationId sid) { return separation_objects.at(sid.id); }

    int32_t store_icc_profile(std::string_view contents, int32_t num_channels);

    uint32_t glyph_for_codepoint(FT_Face face, uint32_t ucs4);

    void create_catalog();
    void write_pages();
    void write_header();
    void write_info();
    void write_cross_reference_table(const std::vector<uint64_t> object_offsets);
    void write_trailer(int64_t xref_offset);

    void close_file();
    void write_finished_object(int32_t object_number,
                               std::string_view dict_data,
                               std::string_view stream_data);
    void write_bytes(const char *buf, size_t buf_size); // With error checking.
    void write_bytes(std::string_view view) { write_bytes(view.data(), view.size()); }

    void write_font_file(int32_t object_num, const TtfFont &font);
    void write_font_descriptor(int32_t object_num, const TtfFont &font, int32_t font_file_obj);
    void write_font(int32_t object_num,
                    const TtfFont &font,
                    int32_t tounicode_obj,
                    int32_t font_descriptor_obj);
    void write_cmap(int32_t object_number, const TtfFont &font);

    std::vector<uint64_t> write_objects();

    FILE *ofile;
    PdfGenerationData opts;
    PdfColorConverter cm;
    FT_Library ft;
    std::vector<ObjectType> document_objects;
    std::vector<PageOffsets> pages; // Refers to object num.
    std::vector<ImageInfo> image_info;
    std::unordered_map<BuiltinFonts, FontId> builtin_fonts;
    std::vector<FontInfo> font_objects;
    std::vector<int32_t> separation_objects;
    std::vector<TtfFont> fonts;
    int32_t rgb_profile_obj, gray_profile_obj, cmyk_profile_obj;
};
