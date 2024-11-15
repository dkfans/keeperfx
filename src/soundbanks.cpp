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
#include <minizip/unzip.h>
#include <memory>
#include <fstream>
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
std::vector<SoundSample> g_campaign_sounds;
SoundSmplID g_first_speech_sample = 0;
SoundSmplID g_first_campaign_sample = 0;
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

auto remove_suffix(const char * input)
{
	const auto dot = strrchr(input, '.');
	if (dot) {
		return std::string(input, dot - input);
	}
	return std::string(input);
}

auto load_sound_dat(const char * filename, long a5)
{
	std::ifstream stream;
	stream.exceptions(std::ios::badbit);
	stream.open(filename, std::ios::in | std::ios::binary);
	std::vector<SoundSample> samples;
	const auto directory_index = calc_directory_index(a5);
	if (directory_index < 0) {
		throw formatted_error("Cannot locate directory");
	}
	stream.seekg(-4, std::ios::end);
	int32_t head_offset;
	stream.read(reinterpret_cast<char *>(&head_offset), sizeof(head_offset));
	head_offset = read_int32_le_buf(reinterpret_cast<unsigned char *>(&head_offset));
	stream.seekg(head_offset);
	SoundBankHead bhead;
	stream.read(reinterpret_cast<char *>(&bhead), sizeof(bhead));
	SoundBankEntry bentries[9];
	stream.read(reinterpret_cast<char *>(bentries), sizeof(bentries));
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
	samples.emplace_back(SoundSample{0, "invalid", {}});
	for (int i = 1; i < sample_count; ++i) {
		stream.seekg(directory.field_0 + (sizeof(sample) * i));
		stream.read(reinterpret_cast<char *>(&sample), sizeof(sample));
		stream.seekg(directory.field_4 + sample.field_12);
		std::vector<uint8_t> buffer(sample.data_size);
		stream.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
		samples.emplace_back(SoundSample{sample.sfxid, remove_suffix(sample.filename), std::move(buffer)});
	}
	return samples;
}

struct unz_deleter
{
	void operator()(void * ptr) const noexcept
	{
		unzClose(ptr);
	}
};

using unz_ptr = std::unique_ptr<void, unz_deleter>;

bool load_custom_sample(unzFile zip, std::vector<SoundSample> & samples)
{
	unz_file_info fi;
	if (unzGetCurrentFileInfo(zip, &fi, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK) {
		return false;
	} else if (fi.size_filename <= 0) {
		return false;
	}
	std::vector<char> name(fi.size_filename + 1);
	if (unzGetCurrentFileInfo(zip, &fi, name.data(), name.size(), nullptr, 0, nullptr, 0) != UNZ_OK) {
		return false;
	}
	name[fi.size_filename] = 0;
	if (unzOpenCurrentFile(zip) != UNZ_OK) {
		return false;
	}
	std::vector<uint8_t> buffer(fi.uncompressed_size);
	if (unzReadCurrentFile(zip, buffer.data(), buffer.size()) != buffer.size()) {
		return false;
	}
	samples.emplace_back(SoundSample{0, remove_suffix(name.data()), std::move(buffer)});
	return unzGoToNextFile(zip) == UNZ_OK;
}

auto load_sound_zip(const char * filename)
{
	// deeper dungeons ./levels/deepdngn/sound.zip
	// tempest ./campgns/tempkpr/sound.zip
	unz_ptr zip(unzOpen(filename));
	if (!zip) {
		throw formatted_error("Cannot open file");
	}
	std::vector<SoundSample> samples;
	while (load_custom_sample(zip.get(), samples));
	return samples;
}

void load_default_effects(long a5)
{
	const auto filename = prepare_file_fmtpath(FGrp_LrgSound, "sound.dat");
	try {
		g_default_effects = load_sound_dat(filename, a5);
		g_first_speech_sample = g_default_effects.size();
		g_first_campaign_sample = g_first_speech_sample;
		g_last_sample = g_first_speech_sample;
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
		g_first_campaign_sample = g_first_speech_sample + g_default_speech.size();
		g_last_sample = g_first_campaign_sample;
		SYNCLOG("Loaded %u sound samples from %s into bank 1", g_default_speech.size() - 1, filename);
	} catch (const std::exception & e) {
		throw formatted_error("Cannot load sound bank 1 from %s: %s", filename, e.what());
	}
}

void index_sample_names()
{
	for (size_t i = 0; i < g_default_effects.size(); ++i) {
		g_sample_ids[g_default_effects[i].name] = i;
	}
	for (size_t i = 0; i < g_default_speech.size(); ++i) {
		g_sample_ids[g_default_speech[i].name] = g_first_speech_sample + i;
	}
}

void index_campaign_sample_names()
{
	for (size_t i = 0; i < g_campaign_sounds.size(); ++i) {
		g_sample_ids[g_campaign_sounds[i].name] = g_first_campaign_sample + i;
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
	}
}

extern "C" void load_campaign_sounds()
{
	const auto filename = prepare_file_path(FGrp_CmpgLvls, "sound.zip");
	try {
		index_sample_names(); // reset any previous mappings
		g_campaign_sounds.clear();
		if (LbFileExists(filename)) {
			g_campaign_sounds = load_sound_zip(filename);
			index_campaign_sample_names();
			SYNCLOG("Loaded %u sound samples from %s into bank 2", g_campaign_sounds.size(), filename);
		}
		g_last_sample = g_first_campaign_sample + g_campaign_sounds.size();
	} catch (const std::exception & e) {
		ERRORLOG("Cannot load sound bank 2 from %s: %s", filename, e.what());
	}
}

const SoundSample * get_sample(SoundSmplID smptbl_id)
{
	if (smptbl_id < 0) {
		return nullptr;
	} else if (smptbl_id < g_first_speech_sample) {
		return &g_default_effects[smptbl_id];
	} else if (smptbl_id < g_first_campaign_sample) {
		return &g_default_speech[smptbl_id - g_first_speech_sample];
	} else if (smptbl_id < g_last_sample) {
		return &g_campaign_sounds[smptbl_id - g_first_campaign_sample];
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
