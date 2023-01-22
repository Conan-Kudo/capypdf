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

#include <vector>
#include <string>
#include <variant>

#include <cassert>
#include <byteswap.h>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_FONT_FORMATS_H
#include FT_OPENTYPE_VALIDATE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TRUETYPE_TAGS_H

namespace {

void byte_swap(int16_t &val) { val = bswap_16(val); }
void byte_swap(uint16_t &val) { val = bswap_16(val); }
void byte_swap(int32_t &val) { val = bswap_32(val); }
void byte_swap(uint32_t &val) { val = bswap_32(val); }
void byte_swap(int64_t &val) { val = bswap_64(val); }
void byte_swap(uint64_t &val) { val = bswap_64(val); }

} // namespace

#pragma pack(push, r1, 1)

struct TTOffsetTable {
    int32_t scaler = 0x10000;
    int16_t num_tables;
    int16_t search_range;
    int16_t entry_selector;
    int16_t range_shift;

    void swap_endian() {
        byte_swap(scaler);
        byte_swap(num_tables);
        byte_swap(search_range);
        byte_swap(entry_selector);
        byte_swap(range_shift);
    }

    void set_table_size(int16_t new_size) {
        num_tables = new_size;
        assert(new_size > 0);
        int i = 0;
        while((1 << i) < new_size) {
            ++i;
        }
        const auto pow2 = 1 << (i - 1);
        // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html
        //
        // Note that for table 4 the text description has a different definition for
        // entrySelector, i.e. whether it is multiplied by 16 or not.
        search_range = 16 * pow2;
        entry_selector = (int)log2(pow2);
        range_shift = num_tables * 16 - search_range;
    }
};

struct TTHead {
    int32_t version;
    int32_t revision;
    uint32_t checksum_adjustment;
    uint32_t magic;
    uint16_t flags;
    uint16_t units_per_em;
    uint64_t created;
    uint64_t modified;
    int16_t x_min;
    int16_t y_min;
    int16_t x_max;
    int16_t y_max;
    uint16_t mac_style;
    uint16_t lowest_rec_pppem;
    int16_t font_direction_hint;
    int16_t index_to_loc_format;
    int16_t glyph_data_format;

    void swap_endian() {
        byte_swap(version);
        byte_swap(revision);
        byte_swap(checksum_adjustment);
        byte_swap(magic);
        byte_swap(flags);
        byte_swap(units_per_em);
        byte_swap(created);
        byte_swap(modified);
        byte_swap(x_min);
        byte_swap(y_min);
        byte_swap(x_max);
        byte_swap(y_max);
        byte_swap(mac_style);
        byte_swap(lowest_rec_pppem);
        byte_swap(font_direction_hint);
        byte_swap(index_to_loc_format);
        byte_swap(glyph_data_format);
    }
};
static_assert(sizeof(TTHead) == 54);

struct TTDirEntry {
    char tag[4];
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;

    bool tag_is(const char *txt) const { return strncmp(tag, txt, 4) == 0; }

    void swap_endian() {
        // byte_swap(tag);
        byte_swap(checksum);
        byte_swap(offset);
        byte_swap(length);
    }
};

struct TTDsig {
    uint32_t version;
    uint16_t num_signatures;
    uint16_t flags;
    // Signature array follows

    void swap_endian() {
        byte_swap(version);
        byte_swap(num_signatures);
        byte_swap(flags);
    }
};

struct TTGDEF {
    uint16_t major;
    uint16_t minor;
    uint16_t glyph_class_offset;
    uint16_t attach_list_offset;
    uint16_t lig_caret_offset;
    uint16_t mark_attach_offset;
    uint16_t mark_glyph_sets_offset = -1; // Since version 1.2
    uint32_t item_var_offset = -1;        // Since version 1.3

    void swap_endian() {
        byte_swap(major);
        byte_swap(minor);
        byte_swap(glyph_class_offset);
        byte_swap(attach_list_offset);
        byte_swap(lig_caret_offset);
        byte_swap(mark_attach_offset);
    }
};

struct TTClassRangeRecord {
    uint16_t start_glyph_id;
    uint16_t end_glyph_id;
    uint16_t gclass;

    void swap_endian() {
        byte_swap(start_glyph_id);
        byte_swap(end_glyph_id);
        byte_swap(gclass);
    }
};

struct TTMaxp10 {
    uint32_t version;
    uint16_t num_glyphs;
    uint16_t max_points;
    uint16_t max_contours;
    uint16_t max_composite_points;
    uint16_t max_composite_contours;
    uint16_t max_zones;
    uint16_t max_twilight_points;
    uint16_t max_storage;
    uint16_t max_function_defs;
    uint16_t max_instruction_defs;
    uint16_t max_stack_elements;
    uint16_t max_sizeof_instructions;
    uint16_t max_component_elements;
    uint16_t max_component_depth;

    void swap_endian() {
        byte_swap(version);
        byte_swap(num_glyphs);
        byte_swap(max_points);
        byte_swap(max_contours);
        byte_swap(max_composite_points);
        byte_swap(max_composite_contours);
        byte_swap(max_zones);
        byte_swap(max_twilight_points);
        byte_swap(max_storage);
        byte_swap(max_function_defs);
        byte_swap(max_instruction_defs);
        byte_swap(max_stack_elements);
        byte_swap(max_sizeof_instructions);
        byte_swap(max_component_elements);
        byte_swap(max_component_depth);
    }
};

struct TTGlyphHeader {
    int16_t num_contours;
    int16_t x_min;
    int16_t y_min;
    int16_t x_max;
    int16_t y_max;

    void swap_endian() {
        byte_swap(num_contours);
        byte_swap(x_min);
        byte_swap(y_min);
        byte_swap(x_max);
        byte_swap(y_max);
    }
};

struct TTHhea {
    uint32_t version;
    int16_t ascender;
    int16_t descender;
    int16_t linegap;
    uint16_t advance_width_max;
    int16_t min_left_side_bearing;
    int16_t min_right_side_bearing;
    int16_t x_max_extent;
    int16_t caret_slope_rise;
    int16_t caret_slope_run;
    int16_t caret_offset;
    int16_t reserved0 = 0;
    int16_t reserved1 = 0;
    int16_t reserved2 = 0;
    int16_t reserved3 = 0;
    int16_t metric_data_format;
    uint16_t num_hmetrics;

    void swap_endian() {
        byte_swap(version);
        byte_swap(ascender);
        byte_swap(descender);
        byte_swap(linegap);
        byte_swap(advance_width_max);
        byte_swap(min_left_side_bearing);
        byte_swap(min_right_side_bearing);
        byte_swap(x_max_extent);
        byte_swap(caret_slope_rise);
        byte_swap(caret_slope_run);
        byte_swap(caret_offset);
        byte_swap(reserved0);
        byte_swap(reserved1);
        byte_swap(reserved2);
        byte_swap(reserved3);
        byte_swap(metric_data_format);
        byte_swap(num_hmetrics);
    }
};

struct TTLongHorMetric {
    uint16_t advance_width;
    int16_t lsb;

    void swap_endian() {
        byte_swap(advance_width);
        byte_swap(lsb);
    }
};

struct TTPost {
    uint16_t version_major;
    uint16_t version_minor;
    int32_t italic_angle;
    int16_t underline_position;
    int16_t underline_thickness;
    uint32_t is_fixed_pitch;
    uint32_t min_mem_type_42;
    uint32_t max_mem_type_42;
    uint32_t min_mem_type_1;
    uint32_t max_mem_type_1;

    void swap_endian() {
        byte_swap(version_major);
        byte_swap(version_minor);
        byte_swap(italic_angle);
        byte_swap(underline_position);
        byte_swap(underline_thickness);
        byte_swap(is_fixed_pitch);
        byte_swap(min_mem_type_42);
        byte_swap(max_mem_type_42);
        byte_swap(min_mem_type_1);
        byte_swap(max_mem_type_1);
    }
};

#pragma pack(pop, r1)

typedef std::variant<uint8_t, int16_t> CoordInfo;

struct SimpleGlyph {
    std::vector<uint16_t> contour_end_points;
    uint16_t instruction_length;
    std::vector<uint8_t> instructions;
    std::vector<uint8_t> flags;
    std::vector<CoordInfo> xcoord;
    std::vector<CoordInfo> ycoord;
};

static_assert(sizeof(TTDirEntry) == 4 * 4);

struct SubsetFont {
    TTOffsetTable offset;
    TTHead head;
    std::vector<TTDirEntry> directory;

    void swap_endian() {
        offset.swap_endian();
        head.swap_endian();
        for(auto &e : directory) {
            e.swap_endian();
        }
    }
};

namespace {

uint32_t str2tag(const unsigned char *txt) {
    return (txt[0] << 24) + (txt[1] << 16) + (txt[2] << 8) + txt[3];
}

uint32_t str2tag(const char *txt) { return str2tag((const unsigned char *)txt); }

const TTDirEntry *find_entry(const std::vector<TTDirEntry> &dir, const char *tag) {
    for(const auto &e : dir) {
        if(e.tag_is(tag)) {
            return &e;
        }
    }
    return nullptr;
}

TTMaxp10 get_maxes(const std::vector<TTDirEntry> &dir, const std::vector<char> &buf) {
    auto e = find_entry(dir, "maxp");
    uint32_t version;
    memcpy(&version, buf.data() + e->offset, sizeof(uint32_t));
    byte_swap(version);
    assert(version == 1 << 16);
    TTMaxp10 maxp;
    assert(e->length >= sizeof(maxp));
    memcpy(&maxp, buf.data() + e->offset, sizeof(maxp));
    maxp.swap_endian();
    return maxp;
    std::abort();
}

TTHead load_head(const std::vector<TTDirEntry> &dir, const std::vector<char> &buf) {
    auto e = find_entry(dir, "head");
    TTHead head;
    memcpy(&head, buf.data() + e->offset, sizeof(head));
    head.swap_endian();
    assert(head.magic == 0x5f0f3cf5);
    return head;
}

std::vector<int32_t> load_loca(const std::vector<TTDirEntry> &dir,
                               const std::vector<char> &buf,
                               uint16_t index_to_loc_format,
                               uint16_t num_glyphs) {
    auto loca = find_entry(dir, "loca");
    std::vector<int32_t> offsets;
    offsets.reserve(num_glyphs);
    if(index_to_loc_format == 0) {
        for(uint16_t i = 0; i <= num_glyphs; ++i) {
            uint16_t offset;
            memcpy(&offset, buf.data() + loca->offset + i * sizeof(uint16_t), sizeof(uint16_t));
            byte_swap(offset);
            offsets.push_back(offset);
        }
    } else {
        assert(index_to_loc_format == 1);
        for(uint16_t i = 0; i <= num_glyphs; ++i) {
            int32_t offset;
            memcpy(&offset, buf.data() + loca->offset + i * sizeof(int32_t), sizeof(int32_t));
            byte_swap(offset);
            offsets.push_back(offset);
        }
    }
    return offsets;
}

TTHhea load_hhea(const std::vector<TTDirEntry> &dir, const std::vector<char> &buf) {
    auto e = find_entry(dir, "hhea");

    TTHhea hhea;
    assert(sizeof(TTHhea) == e->length);
    memcpy(&hhea, buf.data() + e->offset, sizeof(hhea));
    hhea.swap_endian();
    assert(hhea.version == 1 << 16);
    assert(hhea.metric_data_format == 0);
    return hhea;
}

void write_font(const char *ofname, FT_Face face, const std::vector<uint32_t> &glyphs) {
    SubsetFont sf;
    FT_ULong bufsize = 0; // sizeof(sf.head);
    TT_Header *tmp = static_cast<TT_Header *>(FT_Get_Sfnt_Table(face, FT_SFNT_HEAD));
    // static_assert(sizeof(TT_Header) == sizeof(TTHead));
    static_assert(sizeof(TT_Header) == 96);
    printf("Sizeof TT_HEADER: %d\n", (int)sizeof(TT_Header));
    printf("Sizeof TTHead: %d\n", (int)sizeof(TTHead));
    printf("Sizeof long: %d\n", (int)sizeof(long));
    auto ec =
        FT_Load_Sfnt_Table(face, TTAG_head, 0, reinterpret_cast<FT_Byte *>(&sf.head), nullptr);
    assert(ec == 0);
    sf.head.swap_endian();
    assert(sf.head.magic == 0x5f0f3cf5);
    sf.offset.set_table_size(0xa);
    sf.directory.emplace_back(TTDirEntry{{'c', 'm', 'a', 'p'}, 0, 0, 0});
    FILE *f = fopen(ofname, "w");
    sf.swap_endian();
    fwrite(&sf.offset, 1, sizeof(TTOffsetTable), f);
    for(const auto &e : sf.directory) {
        fwrite(&e, 1, sizeof(TTDirEntry), f);
    }
    fclose(f);
}

/* Mandatory TTF tables.
 *
 * 'cmap' character to glyph mapping
 * 'glyf' glyph data
 * 'head' font header
 * 'hhea' horizontal header
 * 'hmtx' horizontal metrics
 * 'loca' index to location
 * 'maxp' maximum profile
 * 'name' naming     <-Cairo and LO do not create this table
 * 'post' postscript <-Cairo and LO do not create this table
 */

/* In addition, the following may be in files created by Cairo and LO:
 *
 * cvt
 * fpgm
 * prep
 */

void debug_font(const char *ifile) {
    FILE *f = fopen(ifile, "r");
    fseek(f, 0, SEEK_END);
    const auto fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<char> buf(fsize, 0);
    fread(buf.data(), 1, fsize, f);
    TTOffsetTable off;
    memcpy(&off, buf.data(), sizeof(off));
    off.swap_endian();
    std::vector<TTDirEntry> directory;
    for(int i = 0; i < off.num_tables; ++i) {
        TTDirEntry e;
        memcpy(&e, buf.data() + sizeof(off) + i * sizeof(e), sizeof(e));
        e.swap_endian();
        directory.emplace_back(std::move(e));
    }
    const auto head = load_head(directory, buf);
    const auto maxes = get_maxes(directory, buf);
    const auto loca = load_loca(directory, buf, head.index_to_loc_format, maxes.num_glyphs);
    const auto hhea = load_hhea(directory, buf);
    for(const auto &e : directory) {
        char tagbuf[5];
        tagbuf[4] = 0;
        memcpy(tagbuf, e.tag, 4);
        printf("%s off: %d size: %d\n", tagbuf, e.offset, e.length);
        if(e.tag_is("head")) {
            //
        } else if(e.tag_is("DSIG")) {
            // Not actually needed for subsetting.
            TTDsig sig;
            memcpy(&sig, buf.data() + e.offset, sizeof(sig));
            sig.swap_endian();
            assert(sig.version == 1);
            assert(sig.num_signatures == 0);
        } else if(e.tag_is("GDEF")) {
            // This neither.
            TTGDEF gdef;
            assert(e.length > sizeof(gdef));
            memcpy(&gdef, buf.data() + e.offset, sizeof(gdef));
            gdef.swap_endian();
            assert(gdef.major == 1);
            assert(gdef.minor == 2);
            gdef.item_var_offset = -1;
            uint16_t classdef_version;
            memcpy(&classdef_version,
                   buf.data() + e.offset + gdef.glyph_class_offset,
                   sizeof(classdef_version));
            byte_swap(classdef_version);
            assert(classdef_version == 2);
            uint16_t num_records;
            memcpy(&num_records,
                   buf.data() + e.offset + gdef.glyph_class_offset + sizeof(classdef_version),
                   sizeof(num_records));
            byte_swap(num_records);
            const char *array_start = buf.data() + e.offset + gdef.glyph_class_offset +
                                      sizeof(classdef_version) + sizeof(num_records);
            for(uint16_t i = 0; i < num_records; ++i) {
                TTClassRangeRecord range;
                memcpy(&range, array_start + i * sizeof(range), sizeof(range));
                range.swap_endian();
            }
        } else if(e.tag_is("prep")) {
            std::string prep(buf.data() + e.offset, e.length);
        } else if(e.tag_is("cvt ")) {
            // Store as bytes, I guess?
        } else if(e.tag_is("fpgm")) {
            // Store as bytes, I guess?
        } else if(e.tag_is("glyf")) {
            std::vector<std::string> glyph_data;
            const char *glyf_start = buf.data() + e.offset;
            for(uint16_t i = 0; i < maxes.num_glyphs; ++i) {
                glyph_data.emplace_back(std::string(glyf_start + loca[i], loca[i + 1] - loca[i]));
                int16_t num_contours;
                memcpy(&num_contours, glyph_data.back().data(), sizeof(num_contours));
                byte_swap(num_contours);
                if(num_contours >= 0) {

                } else {
                    const char *composite_data = glyph_data.back().data() + sizeof(int16_t);
                    const uint16_t MORE_COMPONENTS = 0x20;
                    const uint16_t ARGS_ARE_WORDS = 0x01;
                    uint16_t component_flag;
                    uint16_t glyph_index;
                    do {
                        memcpy(&component_flag, composite_data, sizeof(uint16_t));
                        byte_swap(component_flag);
                        composite_data += sizeof(uint16_t);
                        memcpy(&glyph_index, composite_data, sizeof(uint16_t));
                        byte_swap(glyph_index);
                        composite_data += sizeof(uint16_t);
                        if(component_flag & ARGS_ARE_WORDS) {
                            composite_data += 2 * sizeof(int16_t);
                        } else {
                            composite_data += 2 * sizeof(int8_t);
                        }
                    } while(component_flag & MORE_COMPONENTS);
                    // Instruction data would be here, but we don't need to parse it.
                }
            }
        } else if(e.tag_is("hhea")) {

        } else if(e.tag_is("maxp")) {

        } else if(e.tag_is("loca")) {

        } else if(e.tag_is("hmtx")) {
            std::vector<TTLongHorMetric> longhor;
            std::vector<int16_t> left_side_bearings;
            for(uint16_t i = 0; i < hhea.num_hmetrics; ++i) {
                TTLongHorMetric hm;
                memcpy(&hm,
                       buf.data() + e.offset + i * sizeof(TTLongHorMetric),
                       sizeof(TTLongHorMetric));
                hm.swap_endian();
                longhor.push_back(hm);
            }
            for(int i = 0; i < maxes.num_glyphs - hhea.num_hmetrics; ++i) {
                int16_t lsb;
                memcpy(&lsb,
                       buf.data() + hhea.num_hmetrics * sizeof(TTLongHorMetric) +
                           i * sizeof(int16_t),
                       sizeof(int16_t));
                byte_swap(lsb);
                left_side_bearings.push_back(lsb);
            }
        } else if(e.tag_is("post")) {
            TTPost post;
            memcpy(&post, buf.data() + e.offset, sizeof(TTPost));
            post.swap_endian();
            assert(post.is_fixed_pitch == 0);
        } else if(e.tag_is("cmap")) {
            // Maybe we don't need to parse this table, but
            // instead get it from Freetype as needed
            // when generating output?
        } else if(e.tag_is("GPOS")) {
        } else if(e.tag_is("GSUB")) {
        } else if(e.tag_is("OS/2")) {
        } else if(e.tag_is("gasp")) {
        } else if(e.tag_is("name")) {
        } else {
            printf("Unknown tag %s.\n", tagbuf);
            std::abort();

            // TT fonts contain a ton of additional data tables.
            // We ignore all of them.
        }
    }

    fclose(f);
}

} // namespace

int main(int argc, char **argv) {
    const char *fontfile;
    if(argc > 1) {
        fontfile = argv[1];
    } else {
        fontfile = "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf";
    }
    //  const char *fontfile = "/usr/share/fonts/truetype/noto/NotoSans-Bold.ttf";
    //   const char *fontfile =
    //   "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf";
    // const char *fontfile = "font2.ttf";
    const char *outfile = "font_dump.ttf";
    FT_Library ft;
    auto ec = FT_Init_FreeType(&ft);
    if(ec) {
        printf("FT init fail.\n");
        return 1;
    }
    FT_Face face;
    ec = FT_New_Face(ft, fontfile, 0, &face);
    if(ec) {
        printf("Font opening failed.\n");
        return 1;
    }
    printf("Font opened successfully.\n");
    std::vector<uint32_t> glyphs{'A'};
    debug_font(fontfile);
    write_font(outfile, face, glyphs);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 0;
}
