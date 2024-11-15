#include "pre_inc.h"
#include "config.h"
#include "bflib_sound.h"
#include "bflib_fileio.h"
#include "bflib_bufrw.h"
#include <string>
#include <vector>
#include <exception>
#include <map>
#include <stdarg.h>
#include "post_inc.h"

namespace {

#pragma pack(1)
struct SoundBankHead {
    uint8_t field_0[14];
    uint32_t field_E;
};
#pragma pack()

#pragma pack(1)
struct SoundBankSample {
    /** Name of the sound file the sample comes from. */
    char filename[18];
    /** Offset of the sample data. */
    uint32_t field_12;
    uint32_t field_16;
    /** Size of the sample file. */
    uint32_t data_size;
    uint8_t sfxid;
    uint8_t field_1F;
};
#pragma pack()

#pragma pack(1)
struct SoundBankEntry {
    uint32_t field_0;
    uint32_t field_4;
    uint32_t field_8;
    uint32_t field_C;
};
#pragma pack()

struct SoundSample {
    SoundSFXID sfxid;
    std::string name;
    std::vector<uint8_t> data;
};

std::vector<SoundSample> g_default_effects;
std::vector<SoundSample> g_default_speech;
SoundSmplID g_first_speech_sample = 0;
SoundSmplID g_last_sample = 0;
std::map<std::string, SoundSmplID> g_sample_ids;

auto calc_directory_index(long a5)
{
    switch (a5) {
        case 1610: return 5;
        case 822: return 6;
        case 811: return 7;
        case 800: return 8;
        case 1611: return 4;
        case 1620: return 3;
        case 1622: return 2;
        case 1640: return 1;
        case 1644: return 0;
    }
    return -1;
}

struct formatted_error : std::exception {

    formatted_error(const char * fmt, ...) __attribute__ ((format(printf, 2, 3)))
    {
        va_list args;
        va_start(args, fmt);
        const auto len = vsnprintf(nullptr, 0, fmt, args);
        va_end(args);
        m_message.resize(len + 1);
        vsnprintf(m_message.data(), m_message.size(), fmt, args);
    }

    virtual const char * what() const noexcept override
    {
        return m_message.data();
    }

    std::vector<char> m_message;
};

auto load_sound_dat(const char * filename, long a5)
{
    const auto handle = LbFileOpen(filename, Lb_FILE_MODE_READ_ONLY);
    if (!handle) {
        throw formatted_error("Cannot open file");
    }
    std::vector<SoundSample> samples;
    const auto directory_index = calc_directory_index(a5);
    if (directory_index < 0) {
        throw formatted_error("Cannot locate directory");
    }
    const auto fsize = LbFileLengthHandle(handle);
    if (fsize < 4) {
        throw formatted_error("File is too small");
    } else if (LbFileSeek(handle, -4, Lb_FILE_SEEK_END) < 0) {
        throw formatted_error("Cannot read sound bank head offset");
    }
    int head_offset;
    if (LbFileRead(handle, &head_offset, sizeof(head_offset)) < sizeof(head_offset)) {
        throw formatted_error("Cannot read sound bank head offset");
    }
    head_offset = read_int32_le_buf(reinterpret_cast<unsigned char *>(&head_offset));
    if (head_offset > fsize) {
        throw formatted_error("Invalid sound bank head offset");
    } else  if (LbFileSeek(handle, head_offset, Lb_FILE_SEEK_BEGINNING) < 0) {
        throw formatted_error("Cannot read sound bank head");
    }
    SoundBankHead bhead;
    if (LbFileRead(handle, &bhead, sizeof(bhead)) < sizeof(bhead)) {
        throw formatted_error("Cannot read sound bank head");
    }
    SoundBankEntry bentries[9];
    if (LbFileRead(handle, bentries, sizeof(bentries)) < sizeof(bentries)) {
        throw formatted_error("Cannot read sound bank directory");
    }
    const auto & directory = bentries[directory_index];
    if (directory.field_0 == 0) {
        throw formatted_error("Invalid sound bank directory");
    } else if (directory.field_8 == 0) {
        throw formatted_error("Invalid sound bank directory");
    }
    const auto sample_count = directory.field_8 / sizeof(SoundBankSample);
    samples.reserve(sample_count);
    struct SoundBankSample sample;
    // The first sample spans the entire sound bank, skip
    samples.emplace_back(SoundSample{0, "", {}});
    for (int i = 1; i < sample_count; ++i) {
        if (LbFileSeek(handle, directory.field_0 + (sizeof(sample) * i), Lb_FILE_SEEK_BEGINNING) < 0) {
            throw formatted_error("Cannot read sound bank sample");
        } else if (LbFileRead(handle, &sample, sizeof(sample)) < sizeof(sample)) {
            throw formatted_error("Cannot read sound bank sample");
        } else if (LbFileSeek(handle, directory.field_4 + sample.field_12, Lb_FILE_SEEK_BEGINNING) < 0) {
            throw formatted_error("Cannot read sound bank sample");
        }
        std::vector<uint8_t> buffer(sample.data_size);
        if (LbFileRead(handle, buffer.data(), buffer.size()) < buffer.size()) {
            throw formatted_error("Cannot read sound bank sample");
        }
        samples.emplace_back(SoundSample{sample.sfxid, sample.filename, std::move(buffer)});
    }
    return samples;
}

void load_default_effects(long a5)
{
    const auto filename = prepare_file_fmtpath(FGrp_LrgSound, "sound.dat");
    try {
        g_default_effects = load_sound_dat(filename, a5);
        g_first_speech_sample = g_default_effects.size();
        SYNCLOG("Loaded %u sound samples from %s into bank 0", g_default_effects.size() - 1, filename);
    } catch (const std::exception & e) {
        throw formatted_error("Cannot load sound bank 0 from %s: %s", filename, e.what());
    }
}

void load_default_speech(long a5)
{
    auto filename = prepare_file_fmtpath(FGrp_LrgSound, "speech_%s.dat", get_language_lwrstr(install_info.lang_id));
    if (!LbFileExists(filename)) {
        // fall back to default speech file
        filename = prepare_file_path(FGrp_LrgSound, "speech.dat");
    }
    if (!LbFileExists(filename)) {
        // fall back to English
        filename = prepare_file_fmtpath(FGrp_LrgSound, "speech_%s.dat", get_language_lwrstr(1));
    }
    try {
        g_default_speech = load_sound_dat(filename, a5);
        g_last_sample = g_first_speech_sample + g_default_speech.size();
        SYNCLOG("Loaded %u sound samples from %s into bank 1", g_default_speech.size() - 1, filename);
    } catch (const std::exception & e) {
        ERRORLOG("%s", e.what());
        throw formatted_error("Cannot load sound bank 1 from %s: %s", filename, e.what());
    }
}

void index_sample_names()
{
    g_sample_ids.clear();
    for (size_t i = 0; i < g_default_effects.size(); ++i) {
        g_sample_ids[g_default_effects[i].name] = i;
    }
    for (size_t i = 0; i < g_default_speech.size(); ++i) {
        g_sample_ids[g_default_speech[i].name] = g_first_speech_sample + i;
    }
}

} // local

extern "C" void load_standard_sounds()
{
    try {
        if (!SoundDisabled)
        {
            load_default_effects(1622);
            load_default_speech(1622);
            index_sample_names();
        }
    } catch (const std::exception & e) {
        ERRORLOG("%s", e.what());
        SoundDisabled = true;
    }
}

const SoundSample * get_sample(SoundSmplID smptbl_id)
{
    if (smptbl_id < 0) {
        return nullptr;
    } else if (smptbl_id < g_first_speech_sample) {
        return &g_default_effects[smptbl_id];
    } else if (smptbl_id < g_last_sample) {
        return &g_default_speech[smptbl_id - g_first_speech_sample];
    } else {
        return nullptr;
    }
}

extern "C" SoundSFXID get_sample_sfxid(SoundSmplID smptbl_id)
{
    if (const auto sample = get_sample(smptbl_id)) {
        return sample->sfxid;
    }
    return 0;
}

extern "C" const void * get_sample_data(SoundSmplID smptbl_id)
{
    if (const auto sample = get_sample(smptbl_id)) {
        return sample->data.data();
    }
    return nullptr;
}

extern "C" uint32_t get_sample_size(SoundSmplID smptbl_id)
{
    if (const auto sample = get_sample(smptbl_id)) {
        return sample->data.size();
    }
    return 0;
}

extern "C" const char * get_sample_name(SoundSmplID smptbl_id)
{
    if (const auto sample = get_sample(smptbl_id)) {
        return sample->name.c_str();
    }
    return nullptr;
}

extern "C" SoundSmplID get_sample_id(const char * name)
{
    const auto it = g_sample_ids.find(name);
    if (it != g_sample_ids.end()) {
        return it->second;
    }
    return -1;
}
