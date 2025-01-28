#include "pre_inc.h"
#include "bflib_sprite.h"
#include "bflib_filelst.h"
#include "bflib_dernc.h"
#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include "post_inc.h"

struct TbSpriteSheet {
    std::vector<TbSprite> sprites; // needs to be contiguous to avoid crashing some code
    std::vector<std::vector<unsigned char>> data;
};

namespace {

#pragma pack(1)
#ifdef SPRITE_FORMAT_V2
    struct sprite_entry {
        uint32_t offset;
        uint16_t width;
        uint16_t height;
    };
#else
    struct sprite_entry {
        uint32_t offset;
        uint8_t width;
        uint8_t height;
    };
#endif
#pragma pack(0)

using offset_list = std::vector<std::pair<uint32_t, size_t>>;

bool load_index_file(TbSpriteSheet & sheet, offset_list & offsets, const char * filename)
{
    const auto fname = modify_data_load_filename_function(filename);
    const size_t index_size = LbFileLengthRnc(fname);
    if (index_size <= 0) return false;
    const size_t num_sprites = index_size / sizeof(sprite_entry);
    if (num_sprites == 0) return false;
    std::vector<sprite_entry> entries(num_sprites);
    if (LbFileLoadAt(fname, entries.data()) != index_size) return false;
    sheet.sprites.reserve(num_sprites);
    sheet.data.resize(num_sprites);
    offsets.reserve(num_sprites + 1);
    for (size_t i = 0; i < num_sprites; ++i) {
        const auto & entry = entries[i];
        sheet.sprites.emplace_back(TbSprite{nullptr, entry.width, entry.height});
        offsets.emplace_back(entry.offset, i);
    }
    return true;
}

bool load_data_file(TbSpriteSheet & sheet, offset_list & offsets, const char * filename)
{
    const auto num_sprites = sheet.sprites.size();
    const auto fname = modify_data_load_filename_function(filename);
    const size_t data_size = LbFileLengthRnc(fname);
    if (data_size <= 0) return false;
    // figure out the size of each sprite by sorting offsets
    offsets.emplace_back(data_size, num_sprites);
    std::sort(offsets.begin(), offsets.end(), [] (const auto & a, const auto & b) -> auto {
        return a.first < b.first;
    });
    std::vector<unsigned char> buffer(data_size);
    if (LbFileLoadAt(fname, buffer.data()) != data_size) return false;
    // populate sprite data
    for (size_t i = 0; i < num_sprites; ++i) {
        const auto offset = offsets[i].first;
        const auto size = offsets[i + 1].first - offset;
        const auto sprite_idx = offsets[i].second;
        auto & sprite = sheet.sprites[sprite_idx];
        auto & data = sheet.data[sprite_idx];
        data = std::move(std::vector<unsigned char>(&buffer[offset], &buffer[offset + size]));
        sprite.Data = data.data();
    }
    return true;
}

} // local

extern "C" TbSpriteSheet * create_spritesheet()
{
    try {
        return new TbSpriteSheet();
    } catch (const std::exception & e) {
        ERRORLOG("Failed to create sprite sheet: %s", e.what());
    }
    return nullptr;
}

extern "C" TbSpriteSheet * load_spritesheet(const char * data_fname, const char * index_fname)
{
    try {
        auto sheet = std::make_unique<TbSpriteSheet>();
        offset_list offsets;
        if (!load_index_file(*sheet, offsets, index_fname)) return nullptr;
        if (!load_data_file(*sheet, offsets, data_fname)) return nullptr;
        return sheet.release();
    } catch (const std::exception & e) {
        ERRORLOG("Failed to load sprite sheet from %s %s: %s", data_fname, index_fname, e.what());
    }
    return nullptr;
}

extern "C" void free_spritesheet(TbSpriteSheet ** sheet)
{
    if (sheet) {
        delete *sheet;
        *sheet = NULL;
    }
}

extern "C" const TbSprite * get_sprite(const TbSpriteSheet * sheet, const long index)
{
    if (!sheet) {
        return NULL;
    } else if (index >= sheet->sprites.size()) {
        return NULL;
    }
    return &sheet->sprites[index];
}

#ifdef SPRITE_FORMAT_V2
extern "C" TbBool add_sprite(TbSpriteSheet * sheet, unsigned short width, unsigned short height, int size, const void * data)
#else
extern "C" TbBool add_sprite(TbSpriteSheet * sheet, unsigned char width, unsigned char height, int size, const void * data)
#endif
{
    try {
        sheet->data.emplace_back(std::vector<unsigned char >(static_cast<const unsigned char *>(data), static_cast<const unsigned char *>(data) + size));
        try {
            sheet->sprites.emplace_back(TbSprite{sheet->data.back().data(), width, height});
        } catch (...) {
            sheet->data.pop_back();
            throw;
        }
        return true;
    } catch (const std::exception & e) {
        ERRORLOG("Failed to add sprite: %s", e.what());
    }
    return false;
}

extern "C" long num_sprites(const TbSpriteSheet * sheet)
{
    if (!sheet) {
        return 0;
    }
    return sheet->sprites.size();
}
