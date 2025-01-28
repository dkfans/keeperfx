#include "pre_inc.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_inputctrl.h"
#include "bflib_keybrd.h"
#include "bflib_vidsurface.h"
#include "bflib_fileio.h"

// See: https://trac.ffmpeg.org/ticket/3626
extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/imgutils.h>
	#include <libswresample/swresample.h>
    #pragma GCC diagnostic warning "-Wdeprecated-declarations"
}

#include <cstdio>
#include <string>
#include <memory>
#include <stdexcept>
#include <chrono>
#include "thread.hpp"
#include <vector>
#include <SDL2/SDL.h>
#include "post_inc.h"

namespace {

void copy_to_screen_pxquad(unsigned char *srcbuf, unsigned char *dstbuf, long width, long dst_shift)
{
	const auto s = dst_shift >> 2;
	auto w = ((unsigned long)width) >> 2;
	auto * src = reinterpret_cast<unsigned long *>(srcbuf);
	auto * dst = reinterpret_cast<unsigned long *>(dstbuf);
	do {
		const auto c = *src++;
		const auto i1 = c & 0xFF;
		const auto k1 = (c >> 8) & 0xFF;
		const auto n1 = (k1 << 24) + (k1 << 16) + (i1 << 8) + i1;
		dst[0] = n1;
		dst[s] = n1;
		const auto i2 = (c >> 16) & 0xFF;
		const auto k2 = (c >> 24) & 0xFF;
		const auto n2 = (k2 << 24) + (k2 << 16) + (i2 << 8) + i2;
		dst[1] = n2;
		dst[s+1] = n2;
		dst += 2;
		w--;
	}
	while (w > 0);
}

void copy_to_screen_pxdblh(unsigned char *srcbuf, unsigned char *dstbuf, long width, long dst_shift)
{
	const auto s = dst_shift >> 2;
	auto w = ((unsigned long)width) >> 2;
	auto src = (unsigned long *)srcbuf;
	auto dst = (unsigned long *)dstbuf;
	do {
		const auto n = *src++;
		dst[0] = n;
		dst[s] = n;
		dst++;
		w--;
	}
	while (w > 0);
}

void copy_to_screen_pxdblw(unsigned char *srcbuf, unsigned char *dstbuf, long width)
{
	auto w = ((unsigned long)width) >> 2;
	auto src = (unsigned long *)srcbuf;
	auto dst = (unsigned long *)dstbuf;
	do {
		const auto c = *src++;
		const auto i1 = c & 0xFF;
		const auto k1 = (c >> 8) & 0xFF;
		dst[0] = (k1 << 24) + (k1 << 16) + (i1 << 8) + i1;
		const auto i2 = (c >> 16) & 0xFF;
		const auto k2 = (c >> 24) & 0xFF;
		dst[1] = (k2 << 24) + (k2 << 16) + (i2 << 8) + i2;
		dst += 2;
		w--;
	}
	while (w > 0);
}

void copy_to_screen(const AVFrame & frame, const int flags)
{
	const auto src_pitch = frame.linesize[0];
	auto srcbuf = frame.data[0];
	long buf_center;
	if (flags & (SMK_PixelDoubleLine | SMK_InterlaceLine)) {
		buf_center = lbDisplay.GraphicsScreenWidth * ((LbScreenHeight() - 2 * frame.height) >> 1);
	} else {
		buf_center = lbDisplay.GraphicsScreenWidth * ((LbScreenHeight() - frame.height) >> 1);
	}
	auto w = frame.width;
	if (flags & SMK_PixelDoubleWidth) {
		w = 2 * frame.width;
	}
	auto dstbuf = &lbDisplay.WScreen[buf_center + ((LbScreenWidth() - w) >> 1)];
	if (flags & SMK_PixelDoubleLine) {
		if (flags & SMK_PixelDoubleWidth) {
			for (int h = frame.height; h > 0; h--) {
				copy_to_screen_pxquad(srcbuf, dstbuf, frame.width, lbDisplay.GraphicsScreenWidth);
				dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
				srcbuf += src_pitch;
			}
		} else {
			for (int h = frame.height; h > 0; h--) {
				copy_to_screen_pxdblh(srcbuf, dstbuf, frame.width, lbDisplay.GraphicsScreenWidth);
				dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
				srcbuf += src_pitch;
			}
		}
	} else {
		if (flags & SMK_PixelDoubleWidth) {
				if (flags & SMK_InterlaceLine) {
					for (int h = frame.height; h > 0; h--) {
						copy_to_screen_pxdblw(srcbuf, dstbuf, frame.width);
						dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
						srcbuf += src_pitch;
					}
				} else {
					for (int h = frame.height; h > 0; h--) {
						copy_to_screen_pxdblw(srcbuf, dstbuf, frame.width);
						dstbuf += lbDisplay.GraphicsScreenWidth;
						srcbuf += src_pitch;
					}
				}
		} else if (flags & SMK_InterlaceLine) {
			for (int h = frame.height; h > 0; h--) {
				memcpy(dstbuf, srcbuf, frame.width);
				dstbuf += 2 * lbDisplay.GraphicsScreenWidth;
				srcbuf += src_pitch;
			}
		} else {
			for (int h = frame.height; h > 0; h--) {
				memcpy(dstbuf, srcbuf, frame.width);
				dstbuf += lbDisplay.GraphicsScreenWidth;
				srcbuf += src_pitch;
			}
		}
	}
}

void copy_to_screen_scaled(const AVFrame & frame, const int flags)
{
	const auto src_pitch = frame.linesize[0];
	const auto src_buf = frame.data[0];
	const auto dst_buf = &lbDisplay.WScreen[0];
	// Compute scaling ratio -> Output co-ordinates and output size
	const int scanline = lbDisplay.GraphicsScreenWidth;
	const int nlines = lbDisplay.GraphicsScreenHeight;
	int spw = 0;
	int sph = 0;
	int dst_width = 0;
	int dst_height = 0;

	if ((flags & SMK_FullscreenStretch) && !(flags & SMK_FullscreenFit)) {
		// Use full screen resolution and fill the whole canvas by "stretching"
		dst_width = scanline;
		dst_height = nlines;
	} else {
		// Calculate the correct output size
		int in_width = frame.width;
		int in_height = frame.height;
		float units_per_px = 0;
		// relative aspect ratio difference between the source frame and destination frame
		const float relative_ar_difference = (in_width * 1.0 / in_height * 1.0) / (scanline * 1.0 / nlines * 1.0);
		// when keeping aspect ratio, instead of stretching, this is inverted depending on if we want to crop or fit
		float comparison_ratio = 1;
		if ((flags & SMK_FullscreenStretch) && (flags & SMK_FullscreenFit)) {
			// stretch source from 320x200(16:10) to 320x240 (4:3) (i.e. vertical x 1.2) - "preserve *original* aspect ratio mode"
			if (frame.width == 320 && frame.height == 200) {
				in_height = (int)(in_height * 1.2);
			}
		}
		if ((flags & SMK_FullscreenCrop) && !(flags & SMK_FullscreenFit)) {
			// fill screen (will crop)
			comparison_ratio = relative_ar_difference;
		} else {
			// fit to full screen, preserve aspect ratio (pillar/letter boxed)
			comparison_ratio = 1.0 / relative_ar_difference;
		}
		// take either the destination width or height, depending on whether
		// the destination is wider or narrower than the source
		// (same aspect ratio is treated the same as wider),
		// and also if we want to crop or fit
		if (comparison_ratio <= 1.0) {
			units_per_px = (scanline>nlines?scanline:nlines)/((in_width>in_height?in_width:in_height)/16.0);
		} else {
			units_per_px = (scanline>nlines?nlines:scanline)/((in_width>in_height?in_height:in_width)/16.0);
		}
		if ((flags & SMK_FullscreenCrop) && (flags & SMK_FullscreenFit)) {
			// Find the highest integer scale possible
			if (flags & SMK_FullscreenStretch) {
				//4:3 stretch mode (crop off to the nearest 5x/6x scale
				if (frame.width == 320 && frame.height == 200) {
					// make sure the multiple is integer divisible by 5. Use 5x as a minimum,
					// otherwise there will be no video (resolutions smaller than 1600x1200
					// will have a cropped image from a buffer of that size).
					units_per_px = (max(5, (int)(units_per_px / 16.0 / 5.0) * 5) * 16);
				}
			}
			// scale to the nearest integer multiple of the source resolution.
			units_per_px = ((int)(units_per_px / 16.0) * 16);
		}
		// Starting point coords and width for the destination buffer (based on desired aspect ratio)
		spw = (int)((scanline - in_width * units_per_px / 16.0) / 2.0);
		sph = (int)((nlines - in_height * units_per_px / 16.0) / 2.0);
		dst_width = (int)(in_width * units_per_px / 16.0);
		dst_height = (int)(in_height * units_per_px / 16.0);
	}

	// Clearing top of the canvas
	for (int sh = 0; sh < sph; sh++) {
		memset(&dst_buf[sh * scanline], 0, scanline);
	}
	// Clearing bottom of the canvas
	// (Note: it must be done before drawing, to make sure we won't overwrite last line)
	for (int sh = sph + dst_height; sh < nlines; sh++) {
		memset(&dst_buf[sh * scanline], 0, scanline);
	}
	// Now drawing
	auto dhstart = sph;
	for (int sh = 0; sh < frame.height; sh++) {
		const auto dhend = sph + (dst_height * (sh + 1) / frame.height);
		const auto src = &src_buf[sh * src_pitch];
		// make for(k=0;k<dhend-dhstart;k++) but restrict k to draw area
		const auto mhmin = max(0, -dhstart);
		const auto mhmax = min(dhend - dhstart, nlines - dhstart);
		for (int k = mhmin; k < mhmax; k++) {
			const auto dst = &dst_buf[(dhstart + k) * scanline];
			int dwstart = spw;
			if (dwstart > 0) {
				memset(dst, 0, dwstart);
			}
			for (int sw = 0; sw < frame.width; sw++) {
				const auto dwend = spw + (dst_width * (sw + 1) / frame.width);
				// make for(i=0;i<dwend-dwstart;i++) but restrict i to draw area
				const auto mwmin = max(0, -dwstart);
				const auto mwmax = min(dwend - dwstart, scanline - dwstart);
				for (int i = mwmin; i < mwmax; i++) {
					dst[dwstart+i] = src[sw];
				}
				dwstart = dwend;
			}
			if (dwstart < scanline) {
				memset(dst+dwstart, 0, scanline-dwstart);
			}
		}
		dhstart = dhend;
	}
}

struct movie_t {

	using clock = std::chrono::high_resolution_clock;
	using duration = clock::duration;
	using time_point = clock::time_point;
	using nanoseconds = std::chrono::nanoseconds;

	AVFormatContext * m_format_context = nullptr;
	const AVCodec * m_audio_codec = nullptr;
	const AVCodec * m_video_codec = nullptr;
	AVStream * m_audio_stream = nullptr;
	AVStream * m_video_stream = nullptr;
	AVCodecContext * m_audio_context = nullptr;
	AVCodecContext * m_video_context = nullptr;
	AVPacket * m_packet = nullptr;
	AVFrame * m_frame = nullptr;
	SwrContext * m_resampler = nullptr;
	time_point m_video_start;
	AVRational m_time_base;
	SDL_AudioDeviceID m_audio_device = 0;

	int m_audio_index;
	int m_video_index;
	int m_flags;

	int m_output_audio_channels;
	int m_output_audio_frequency;
	AVChannelLayout m_output_audio_layout;
	AVSampleFormat m_output_audio_format;

	movie_t(const char * filename, const int flags) {
		m_flags = flags;
		m_video_start = time_point();
		open_input(filename);
		open_audio_device();
		find_stream_info();
		setup_audio();
		setup_video();
		make_packet();
		make_frame();
		if (m_audio_context) {
			make_resampler();
		}
		m_time_base = m_format_context->streams[m_video_index]->time_base;
	}

	~movie_t() noexcept {
		if (m_format_context) {
			avformat_close_input(&m_format_context);
		}
		if (m_audio_context) {
			avcodec_free_context(&m_audio_context);
		}
		if (m_video_context) {
			avcodec_free_context(&m_video_context);
		}
		if (m_frame) {
			av_frame_free(&m_frame);
		}
		if (m_packet) {
			av_packet_free(&m_packet);
		}
		if (m_resampler) {
			swr_free(&m_resampler);
		}
		if (m_audio_device > 0) {
			SDL_CloseAudioDevice(m_audio_device);
			m_audio_device = 0;
		}
	}

	void open_input(const char * filename) {
		if (avformat_open_input(&m_format_context, filename, nullptr, nullptr) != 0) {
			throw std::runtime_error("Cannot open source file");
		}
	}

	AVSampleFormat sdl_to_ffmpeg_format(SDL_AudioFormat format) {
		switch (format) {
			case AUDIO_S8: return AV_SAMPLE_FMT_U8;
			case AUDIO_S16SYS: return AV_SAMPLE_FMT_S16;
			case AUDIO_S32SYS: return AV_SAMPLE_FMT_S32;
			case AUDIO_F32SYS: return AV_SAMPLE_FMT_FLT;
			default: return AV_SAMPLE_FMT_NONE;
		}
	}

	AVChannelLayout channels_to_ffmpeg_layout(int channels) {
		switch (channels) {
			case 1: return AV_CHANNEL_LAYOUT_MONO;
			case 2: return AV_CHANNEL_LAYOUT_STEREO;
			case 3: return AV_CHANNEL_LAYOUT_SURROUND;
			case 4: return AV_CHANNEL_LAYOUT_QUAD;
			case 5: return AV_CHANNEL_LAYOUT_4POINT1;
			case 6: return AV_CHANNEL_LAYOUT_5POINT1;
			case 7: return AV_CHANNEL_LAYOUT_6POINT1;
			case 8: return AV_CHANNEL_LAYOUT_7POINT1;
			default: return {};
		}
	}

	void open_audio_device() {
		SDL_AudioSpec desired, obtained;
		desired.freq = 44100;
		desired.format = AUDIO_F32SYS;
		desired.channels = 2;
		desired.silence = 0;
		desired.samples = 0;
		desired.padding = 0;
		desired.size = 0;
		desired.callback = nullptr;
		desired.userdata = nullptr;
		m_audio_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
		if (m_audio_device <= 0) {
			throw std::runtime_error("Cannot open audio device");
		}
		m_output_audio_channels = obtained.channels;
		m_output_audio_frequency = obtained.freq;
		m_output_audio_format = sdl_to_ffmpeg_format(obtained.format);
		m_output_audio_layout = channels_to_ffmpeg_layout(obtained.channels);
	}

	void find_stream_info() {
		if (avformat_find_stream_info(m_format_context, nullptr) < 0) {
			throw std::runtime_error("Could not find stream information");
		}
	}

	AVCodecContext * make_context(const AVCodec * codec) {
		const auto context = avcodec_alloc_context3(codec);
		if (!context) {
			throw std::runtime_error("Failed to allocate codec context");
		}
		return context;
	}

	const AVCodec * find_codec(const AVStream * stream) {
		return avcodec_find_decoder(stream->codecpar->codec_id);
	}

	void copy_parameters(AVCodecContext * codec_context, const AVStream * stream) {
		if (avcodec_parameters_to_context(codec_context, stream->codecpar) != 0) {
			throw std::runtime_error("Failed to copy codec parameters to decoder context");
		}
	}

	void open_codec(AVCodecContext * codec_context, const AVCodec * codec) {
		if (avcodec_open2(codec_context, codec, nullptr) != 0) {
			throw std::runtime_error("Failed to open codec");
		}
	}

	int find_best_stream(AVMediaType type) {
		return av_find_best_stream(m_format_context, type, -1, -1, nullptr, 0);
	}

	void setup_audio() {
		m_audio_index = find_best_stream(AVMEDIA_TYPE_AUDIO);
		if (m_audio_index >= 0) {
			m_audio_stream = m_format_context->streams[m_audio_index];
			m_audio_codec = find_codec(m_audio_stream);
			if (m_audio_codec) {
				m_audio_context = make_context(m_audio_codec);
				copy_parameters(m_audio_context, m_audio_stream);
				open_codec(m_audio_context, m_audio_codec);
			}
		}
	}

	void setup_video() {
		m_video_index = find_best_stream(AVMEDIA_TYPE_VIDEO);
		if (m_video_index < 0) {
			throw std::runtime_error("Could not find video stream");
		}
		m_video_stream = m_format_context->streams[m_video_index];
		m_video_codec = find_codec(m_video_stream);
		if (!m_video_codec) {
			throw std::runtime_error("Failed to find codec");
		}
		m_video_context = make_context(m_video_codec);
		copy_parameters(m_video_context, m_video_stream);
		open_codec(m_video_context, m_video_codec);
	}

	void make_frame() {
		m_frame = av_frame_alloc();
		if (!m_frame) {
			throw std::runtime_error("Could not allocate frame");
		}
	}

	void make_packet() {
		m_packet = av_packet_alloc();
		if (!m_packet) {
			throw std::runtime_error("Could not allocate packet");
		}
	}

	void make_resampler() {
		if (swr_alloc_set_opts2(
			&m_resampler,
			&m_output_audio_layout,
			m_output_audio_format,
			m_output_audio_frequency,
			&m_audio_context->ch_layout,
			m_audio_context->sample_fmt,
			m_audio_context->sample_rate,
			0,
			nullptr
		) != 0) {
			throw std::runtime_error("Cannot create resampler");
		}
		if (swr_init(m_resampler) != 0) {
			throw std::runtime_error("Could not initialize resampler");
		}
	}

	duration time_since_video_start() {
		if (m_video_start == time_point()) {
			m_video_start = clock::now();
			return duration();
		}
		return clock::now() - m_video_start;
	}

	void output_audio_frame() {
		const auto sample_size = av_get_bytes_per_sample(m_output_audio_format);
		const auto buffer_samples = swr_get_out_samples(m_resampler, m_frame->nb_samples);
		uint8_t * buffer = nullptr;
		av_samples_alloc(
			&buffer,
			nullptr,
			m_output_audio_channels,
			buffer_samples,
			m_output_audio_format,
			1
		);
		const auto num_samples = m_output_audio_channels * swr_convert(
			m_resampler,
			&buffer,
			buffer_samples,
			m_frame->data,
			m_frame->nb_samples
		);
		SDL_QueueAudio(m_audio_device, buffer, num_samples * sample_size);
		SDL_PauseAudioDevice(m_audio_device, 0);
		av_freep(&buffer);
	}

	void output_video_frame() {
		if (m_frame->palette_has_changed) {
			JUSTLOG("palette changed");
			SDL_Color palette[PALETTE_COLORS];
			for (size_t i = 0; i < PALETTE_COLORS; ++i) {
				palette[i].b = m_frame->data[1][(i * 4) + 0]; // blue
				palette[i].g = m_frame->data[1][(i * 4) + 1]; // green
				palette[i].r = m_frame->data[1][(i * 4) + 2]; // red
			}
			LbScreenWaitVbi(); // this is a no-op today
			// LbPaletteSet expects values in range 0-63 for reasons, nuking 75% of the color range
			SDL_SetPaletteColors(lbDrawSurface->format->palette, palette, 0, PALETTE_COLORS);
		}
		if (LbScreenLock() != Lb_SUCCESS) {
			return;
		} else if (m_flags & (SMK_FullscreenFit | SMK_FullscreenStretch | SMK_FullscreenCrop)) { // new scaling mode
			copy_to_screen_scaled(*m_frame, m_flags);
		} else {
			copy_to_screen(*m_frame, m_flags);
		}
		LbScreenUnlock();
		LbScreenSwap();
	}

	bool output_audio_frames() {
		while (true) {
			const auto result = avcodec_receive_frame(m_audio_context, m_frame);
			if (result != 0) {
				if (result == AVERROR(EAGAIN)) {
					return true;
				} else {
					return false;
				}
			}
			output_audio_frame();
		}
	}

	void wait_for_pts() {
		const duration pts = nanoseconds((int64_t(m_frame->pts) * (1000000000 / m_time_base.den)) * m_time_base.num);
		const auto now = time_since_video_start();
		const auto delta = pts - now;
		if (delta > duration()) {
			std::this_thread::sleep_for(delta);
		}
	}

	bool output_video_frames() {
		while (true) {
			const auto result = avcodec_receive_frame(m_video_context, m_frame);
			if (result != 0) {
				if (result == AVERROR(EAGAIN)) {
					return true;
				} else {
					return false;
				}
			}
			wait_for_pts();
			output_video_frame();
			if (!LbWindowsControl()) {
				return false;
			} else if (m_flags & SMK_NoStopOnUserInput) {
				return true;
			} else if (lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_RETURN] || lbKeyOn[KC_SPACE] || lbDisplay.LeftButton) {
				return false;
			}
		}
	}

	bool decode_audio() {
		if (avcodec_send_packet(m_audio_context, m_packet) != 0) {
			return false;
		}
		return output_audio_frames();
	}

	bool decode_video() {
		if (avcodec_send_packet(m_video_context, m_packet) != 0) {
			return false;
		}
		return output_video_frames();
	}

	void flush_audio() {
		output_audio_frames();
	}

	void flush_video() {
		output_video_frames();
	}

	bool read_frame() {
		return av_read_frame(m_format_context, m_packet) >= 0;
	}

	void play() {
		while (read_frame()) {
			if (m_packet->stream_index == m_audio_index && m_audio_context) {
				if (!decode_audio()) {
					break;
				}
			} else if (m_packet->stream_index == m_video_index) {
				if (!decode_video()) {
					break;
				}
			}
		}
		if (m_audio_context) {
			flush_audio();
		}
		flush_video();
	}
};

} // local

extern "C" TbBool play_smk(const char * filename, const int flags) {
	try {
		lbDisplay.LeftButton = 0; // hack?
		movie_t movie(filename, flags);
		movie.play();
		return true;
	} catch (const std::exception & e) {
		ERRORLOG("Error playing %s: %s", filename, e.what());
	}
	return false;
}

namespace {

enum {
	FLI_PREFIX = 0xF100,
	FLI_COLOR256 = 4,
	FLI_SS2 = 7,
	FLI_COLOR = 11,
	FLI_LC = 12,
	FLI_BLACK = 13,
	FLI_BRUN = 15,
	FLI_COPY = 16,
	FLI_PSTAMP = 18,
};

#pragma pack(1)
struct AnimFLIHeader { // sizeof=0x80
	unsigned long dsize;
	unsigned short magic;
	unsigned short frames;
	short width;
	short height;
	unsigned short depth;
	unsigned short flags;
	unsigned long speed;
	short reserved2;
	unsigned long created;
	unsigned long creator;
	unsigned long updated;
	unsigned long updater;
	short aspectx;
	short aspecty;
	char reserved3[38];
	unsigned long oframe1;
	unsigned long oframe2;
	char reserved4[40];
};
#pragma pack()

#pragma pack(1)
struct AnimFLIChunk { //sizeof=0x6
	long csize;
	unsigned short ctype;
};
#pragma pack()

#pragma pack(1)
struct AnimFLIPrefix { //sizeof=0x6
	long csize;
	unsigned short ctype;
	short nchunks;
	char reserved[8];
};
#pragma pack()

struct Animation {
	long field_0;
	unsigned char *videobuf;
	unsigned char *chunkdata;
	unsigned char *field_C;
	TbFileHandle inpfhndl;
	TbFileHandle outfhndl;
	short field_18;
	short field_1A;
	unsigned char palette[768];
	long field_31C;
	long field_320;
	long field_324;
	AnimFLIHeader header;
	AnimFLIChunk chunk;
	AnimFLIPrefix prefix;
	AnimFLIChunk subchunk;
	char field_3C4[12];
};

Animation animation;

/**
 * Writes the data into FLI animation.
 * @return Returns false on error, true on success.
 */
short anim_write_data(void *buf, long size)
{
	return LbFileWrite(animation.outfhndl,buf,size) == size;
}

/**
 * Stores data into FLI buffer.
 * @return Returns false on error, true on success.
 */
short anim_store_data(void *buf, long size)
{
	memcpy(animation.field_C, buf, size);
	animation.field_C += size;
	return true;
}

/**
 * Reads the data from FLI animation.
 * @return Returns false on error, true on success.
 */
short anim_read_data(void *buf, long size)
{
	if (buf == NULL) {
		LbFileSeek(animation.inpfhndl,size,Lb_FILE_SEEK_CURRENT);
		return true;
	} else if (LbFileRead(animation.inpfhndl,buf,size) == size) {
		return true;
	}
	return false;
}

long anim_make_FLI_COPY(unsigned char *screenbuf)
{
	int scrpoints = animation.header.height * animation.header.width;
	memcpy(animation.field_C, screenbuf, scrpoints);
	animation.field_C += scrpoints;
	return scrpoints;
}

long anim_make_FLI_COLOUR256(unsigned char *palette)
{
	if (memcmp(animation.palette, palette, 768) == 0) {
		return 0;
	}
	unsigned short *change_count;
	unsigned char *kept_count;
	short colridx;
	short change_chunk_len;
	short kept_chunk_len;
	change_chunk_len = 0;
	kept_chunk_len = 0;
	change_count = (unsigned short *)animation.field_C;
	kept_count = NULL;
	animation.field_C += 2;
	for (colridx = 0; colridx < 256; colridx++) {
		unsigned char *anipal;
		unsigned char *srcpal;
		anipal = &animation.palette[3 * colridx];
		srcpal = &palette[3 * colridx];
		if (memcmp(anipal, srcpal, 3) == 0) {
			change_chunk_len = 0;
			kept_chunk_len++;
		} else {
			if (!change_chunk_len) {
				*animation.field_C = kept_chunk_len;
				kept_chunk_len = 0;
				animation.field_C++;
				kept_count = (unsigned char *)animation.field_C;
				animation.field_C++;
			}
			++change_chunk_len;
			*animation.field_C = 4 * srcpal[0];
			animation.field_C++;
			*animation.field_C = 4 * srcpal[1];
			animation.field_C++;
			*animation.field_C = 4 * srcpal[2];
			animation.field_C++;
			++(*kept_count);
		}
		if (change_chunk_len == 1) {
			++(*change_count);
		}
	}
	return 1;
}

/**
 * Compress data into FLI's BRUN block (8-bit Run-Length compression).
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_BRUN(unsigned char *screenbuf) {
	unsigned char *blk_begin = animation.field_C;
	short w;
	short h;
	short k;
	short count;
	unsigned char *sbuf = screenbuf;
	for ( h = animation.header.height; h>0; h-- ) {
		animation.field_C++;
		for (w=animation.header.width; w>0; ) {
			count = 0;
			// Counting size of RLE block
			for ( k=1; w>1; k++ ) {
				if (sbuf[k] != sbuf[0]) break;
				if (count == 127) break;
				w--;
				count++;
			}
			// If RLE block would be valid
			if ( count>0 ) {
				if ( count < 127 ) {
					count++;
					w--;
				}
				*animation.field_C = (char)count;
				animation.field_C++;
				*animation.field_C = sbuf[0];
				animation.field_C++;
				sbuf += count;
			} else {
				if ( w > 1 ) {
					count=0;
					// Find the next block of at least 4 same pixels
					for ( k = 0; w>0; k++ ) {
						if ( (sbuf[k+1]==sbuf[k]) && (sbuf[k+2]==sbuf[k]) && (sbuf[k+3]==sbuf[k]) ) break;
						if ( count == -127 ) break;
						count--;
						w--;
					}
				} else {
					count=-1;
					w--;
				}
				if ( count!=0 ) {
					*animation.field_C = (char)count;
					animation.field_C++;
					memcpy(animation.field_C, sbuf, -count);
					sbuf -= count;
					animation.field_C -= count;
				}
			}
		}
	}
	// Make the block size even
	if ((int)animation.field_C & 1) {
		*animation.field_C='\0';
		animation.field_C++;
	}
	return (animation.field_C - blk_begin);
}

/**
 * Compress data into FLI's SS2 block.
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_SS2(unsigned char *curdat, unsigned char *prvdat)
{
	unsigned char *blk_begin;
	blk_begin=animation.field_C;
	unsigned char *cbuf;
	unsigned char *pbuf;
	unsigned char *cbf;
	unsigned char *pbf;
	short h;
	short w;
	short k;
	short nsame;
	short ndiff;
	short wend;
	short wendt;
	cbuf = curdat;
	pbuf = prvdat;
	unsigned short *lines_count;
	unsigned short *pckt_count;
	lines_count = (unsigned short *)animation.field_C;
	animation.field_C += 2;
	pckt_count = (unsigned short *)animation.field_C;

	wend = 0;
	for (h=animation.header.height; h>0; h--) {
		cbf = cbuf;
		pbf = pbuf;
		if (wend == 0) {
			pckt_count = (unsigned short *)animation.field_C;
			animation.field_C += 2;
			(*lines_count)++;
		}
		for (w=animation.header.width;w>0;) {
			for ( k=0; w>0; k++) {
				if ( *(unsigned short *)(pbf+2*(long)k) != *(unsigned short *)(cbf+2*(long)k) ) break;
				w -= 2;
			}
			if (2*(long)k == animation.header.width) {
				wend--;
				cbf += LbGraphicsScreenWidth();
				pbf += LbGraphicsScreenWidth();
				continue;
			}
			if ( w > 0 ) {
				if (wend != 0) {
					(*pckt_count) = wend;
					pckt_count = (unsigned short *)animation.field_C;
					animation.field_C += 2;
				}
				wendt = 2*k;
				wend = wendt;
				while (wend > 255) {
					*(unsigned char *)animation.field_C = 255;
					animation.field_C++;
					*(unsigned char *)animation.field_C = 0;
					animation.field_C++;
					wend -= 255;
					(*pckt_count)++;
				}
				cbf += wendt;
				pbf += wendt;
				for (nsame=0; nsame<127; nsame++) {
					if (w <= 2) break;
					if ((*(unsigned short *)(pbf+2*nsame+0) == *(unsigned short *)(cbf+2*nsame+0)) &&
						(*(unsigned short *)(pbf+2*nsame+2) == *(unsigned short *)(cbf+2*nsame+2))) {
						break;
					}
					if ( *(unsigned short *)(cbf+2*nsame+2) != *(unsigned short *)(cbf) ) break;
					w -= 2;
				}
				if (nsame > 0) {
					if (nsame < 127) {
						nsame++;
						w -= 2;
					}
					*(unsigned char *)animation.field_C = wend;
					animation.field_C++;
					*(unsigned char *)animation.field_C = -nsame;
					animation.field_C++;
					*(unsigned short *)animation.field_C = *(unsigned short *)cbf;
					animation.field_C+=2;
					pbf += 2*nsame;
					cbf += 2*nsame;
					wend = 0;
					(*pckt_count)++;
				} else {
					if (w == 2) {
						ndiff = 1;
						w -= 2;
					} else {
						for (ndiff=0; ndiff<127; ndiff++) {
							if (w <= 0) break;
							if ( *(unsigned short *)(pbf+2*ndiff) == *(unsigned short *)(cbf+2*ndiff) )  break;
							if ((*(unsigned short *)(cbf+2*(ndiff+1)) == *(unsigned short *)(cbf+2*ndiff)) &&
							(*(unsigned short *)(cbf+2*(ndiff+2)) == *(unsigned short *)(cbf+2*ndiff)) ) {
								break;
							}
							w -= 2;
						}
					}
					if (ndiff>0) {
						*(unsigned char *)animation.field_C = wend;
						animation.field_C++;
						*(unsigned char *)animation.field_C = ndiff;
						animation.field_C++;
						memcpy(animation.field_C, cbf, 2*(long)ndiff);
						animation.field_C += 2*(long)ndiff;
						pbf += 2*(long)ndiff;
						cbf += 2*(long)ndiff;
						wend = 0;
						(*pckt_count)++;
					}
				}
			}
		}
		cbuf += LbGraphicsScreenWidth();
		pbuf += LbGraphicsScreenWidth();
	}

	if (animation.header.height+wend == 0) {
		(*lines_count) = 1;
		(*pckt_count) = 1;
		*(unsigned char *)animation.field_C = 0;
		animation.field_C++;
		*(unsigned char *)animation.field_C = 0;
		animation.field_C++;
	} else if (wend != 0) {
		animation.field_C -= 2;
		(*lines_count)--;
	}
	// Make the data size even
	animation.field_C = (unsigned char *)(((unsigned int)animation.field_C + 1) & 0xFFFFFFFE);
	return animation.field_C - blk_begin;
}

/**
 * Compress data into FLI's LC block.
 * @return Returns unpacked size of the block which was compressed.
 */
long anim_make_FLI_LC(unsigned char *curdat, unsigned char *prvdat)
{
	unsigned char *blk_begin;
	blk_begin=animation.field_C;
	unsigned char *cbuf;
	unsigned char *pbuf;
	unsigned char *cbf;
	unsigned char *pbf;
	unsigned char *outptr;
	short h;
	short w;
	short hend;
	short wend;
	short hdim;
	short wendt;
	short k;
	short nsame;
	short ndiff;
	int blksize;

	cbuf = curdat;
	pbuf = prvdat;
	for (hend = animation.header.height; hend>0;  hend--) {
		wend = 0;
		for (w = animation.header.width; w>0; w--) {
			if (cbuf[wend] != pbuf[wend]) break;
			++wend;
		}
		if ( wend != animation.header.width ) break;
		cbuf += LbGraphicsScreenWidth();
		pbuf += LbGraphicsScreenWidth();
	}
	if (hend != 0) {
		hend = animation.header.height - hend;
		blksize = animation.header.width * (long)(animation.header.height-1);
		cbuf = curdat+blksize;
		pbuf = prvdat+blksize;
		for (h=animation.header.height; h>0; h--) {
			wend = 0;
			for (w=animation.header.width; w>0; w--) {
				if (cbuf[wend] != pbuf[wend]) break;
				wend++;
			}
			if ( wend != animation.header.width ) break;
			cbuf -= LbGraphicsScreenWidth();
			pbuf -= LbGraphicsScreenWidth();
		}
		hdim = h - hend;
		blksize = animation.header.width * (long)hend;
		cbuf = curdat+blksize;
		pbuf = prvdat+blksize;
		*(unsigned short *)animation.field_C = hend;
		animation.field_C += 2;
		*(unsigned short *)animation.field_C = hdim;
		animation.field_C += 2;

		for (h = hdim; h>0; h--) {
			cbf = cbuf;
			pbf = pbuf;
			outptr = animation.field_C++;
			for (w=animation.header.width; w>0; ) {
				for ( wend=0; w>0; wend++) {
					if ( cbf[wend] != pbf[wend]) break;
					w--;
				}
				wendt = wend;
				if (animation.header.width == wend) continue;
				if ( w <= 0 ) break;
				while ( wend > 255 ) {
					*(unsigned char *)animation.field_C = 255;
					animation.field_C++;
					*(unsigned char *)animation.field_C = 0;
					animation.field_C++;
					wend -= 255;
					(*(unsigned char *)outptr)++;
				}
				cbf += wendt;
				pbf += wendt;
				k = 0;
				nsame = 0;
				while ( w > 1 ) {
					if ( nsame == -127 ) break;
					if ((cbf[k+0] == pbf[k+0]) && (cbf[k+1] == pbf[k+1]) && (cbf[k+2] == pbf[k+2])) break;
					if (cbf[k+1] != cbf[0]) break;
					w--;
					k++;
					nsame--;
				}
				if ( nsame ) {
					if ( nsame != -127 ) {
						nsame--;
						w--;
					}
					*(unsigned char *)animation.field_C = wend;
					animation.field_C++;
					*(unsigned char *)animation.field_C = nsame;
					animation.field_C++;
					*(unsigned char *)animation.field_C = cbf[0];
					cbf -= nsame;
					pbf -= nsame;
					animation.field_C++;
					(*(unsigned char *)outptr)++;
				} else {
					if ( w == 1 ) {
						ndiff = nsame + 1;
						w--;
					} else {
						k = 0;
						ndiff = 0;
						while (w != 0) {
							if ( ndiff == 127 ) break;
							if ((cbf[k+0] == pbf[k+0]) && (cbf[k+1] == pbf[k+1]) && (cbf[k+2] == pbf[k+2]))
								break;
							if ((cbf[k+1] == cbf[k+0]) && (cbf[k+2] == cbf[k+0]) && (cbf[k+3] == cbf[k+0]))
								break;
							w--;
							k++;
							ndiff++;
						}
					}
					if (ndiff != 0) {
						*(unsigned char *)animation.field_C = wend;
						animation.field_C++;
						*(unsigned char *)animation.field_C = ndiff;
						animation.field_C++;
						memcpy(animation.field_C, cbf, ndiff);
						animation.field_C += ndiff;
						cbf += ndiff;
						pbf += ndiff;
						(*(unsigned char *)outptr)++;
					}
				}
			}
			cbuf += LbGraphicsScreenWidth();
			pbuf += LbGraphicsScreenWidth();
		}
	} else {
		*(short *)animation.field_C = 0;
		animation.field_C += 2;
		*(short *)animation.field_C = 1;
		animation.field_C += 2;
		*(char *)animation.field_C = 0;
		animation.field_C++;
	}
	// Make the data size even
	animation.field_C = (unsigned char *)(((unsigned int)animation.field_C + 1) & 0xFFFFFFFE);
	return animation.field_C - blk_begin;
}

/*
 * Returns size of the FLI movie frame buffer, for given width
 * and height of animation. The buffer of returned size is big enough
 * to store one frame of any kind (any compression).
 */
long anim_buffer_size(int width,int height,int bpp)
{
	int n = (bpp>>3);
	if (bpp%8) n++;
	return abs(width)*abs(height)*n + 32767;
}

/*
 * Returns size of the FLI movie frame buffer, for given width
 * and height of animation. The buffer of returned size is big enough
 * to store one frame of any kind (any compression).
 */
short anim_format_matches(int width,int height,int bpp)
{
	if (width != animation.header.width) {
		return false;
	} else if (height != animation.header.height) {
		return false;
	} else if (bpp != animation.header.depth) {
		return false;
	}
	return true;
}

short anim_open(char *fname, int arg1, short arg2, int width, int height, int bpp, unsigned int flags)
{
	if ( flags & animation.field_0 ) {
		ERRORLOG("Cannot record movie");
		return false;
	}
	if (flags & 0x01) {
		SYNCLOG("Starting to record new movie, \"%s\".",fname);
		memset(&animation, 0, sizeof(Animation));
		animation.field_0 |= flags;
		animation.videobuf = static_cast<unsigned char *>(calloc(2 * height*width, 1));
		if (animation.videobuf==NULL) {
			ERRORLOG("Cannot allocate video buffer.");
			return false;
		}
		long max_chunk_size = anim_buffer_size(width,height,bpp);
		animation.chunkdata = static_cast<unsigned char *>(calloc(max_chunk_size, 1));
		if (animation.chunkdata==NULL) {
			ERRORLOG("Cannot allocate chunk buffer.");
			return false;
		}
		animation.outfhndl = LbFileOpen(fname, Lb_FILE_MODE_NEW);
		if (!animation.outfhndl) {
			ERRORLOG("Can't open movie file.");
			return false;
		}
		animation.header.dsize = 128;
		animation.header.magic = 0xAF12;
		animation.header.depth = bpp;
		animation.header.flags = 3;
		animation.header.speed = 57;
		animation.header.created = 0;
		animation.header.frames = 0;
		animation.header.width = width;
		animation.header.updated = 0;
		animation.header.aspectx = 6;
		animation.header.height = height;
		animation.header.reserved2 = 0;
		animation.header.creator = 0x464C4942;//'BILF'
		animation.header.aspecty = 5;
		animation.header.updater = 0x464C4942;
		memset(animation.header.reserved3, 0, sizeof(animation.header.reserved3));
		animation.header.oframe1 = 0;
		animation.header.oframe2 = 0;
		memset(animation.header.reserved4, 0, sizeof(animation.header.reserved4));
		animation.field_18 = arg2;
		if ( !anim_write_data(&animation.header, sizeof(AnimFLIHeader)) ) {
			ERRORLOG("Movie write error.");
			LbFileClose(animation.outfhndl);
			return false;
		}
		animation.field_31C = 0;
		animation.field_320 = height*width + 1024;
		memset(animation.palette, -1, sizeof(animation.palette));
	}
	if (flags & 0x02)  {
		SYNCLOG("Resuming movie recording, \"%s\".",fname);
		animation.field_0 |= flags;
		animation.inpfhndl = LbFileOpen(fname, 2);
		if (!animation.inpfhndl) {
			return false;
		}
		// Reading header
		if (!anim_read_data(&animation.header, sizeof(AnimFLIHeader))) {
			ERRORLOG("Movie header read error.");
			LbFileClose(animation.inpfhndl);
			return false;
		}
		// Now we can allocate chunk buffer
		long max_chunk_size = anim_buffer_size(animation.header.width,animation.header.height,animation.header.depth);
		animation.chunkdata = static_cast<unsigned char *>(calloc(max_chunk_size, 1));
		if (animation.chunkdata==NULL) {
			return false;
		}
		if (!anim_read_data(&animation.chunk, sizeof(AnimFLIChunk))) {
			ERRORLOG("Movie chunk read error.");
			LbFileClose(animation.inpfhndl);
			return false;
		}
		if (animation.chunk.ctype == FLI_PREFIX) {
			if (!anim_read_data(animation.chunkdata, animation.chunk.csize-sizeof(AnimFLIChunk))) {
				ERRORLOG("Movie data read error.");
				LbFileClose(animation.inpfhndl);
				return false;
			}
		} else {
			LbFileSeek(animation.inpfhndl, -sizeof(AnimFLIChunk), Lb_FILE_SEEK_CURRENT);
		}
		animation.field_31C = 0;
	}
	return true;
}

TbBool anim_make_next_frame(unsigned char *screenbuf, unsigned char *palette)
{
	SYNCDBG(7,"Starting");
	unsigned long max_chunk_size;
	unsigned char *dataptr;
	long brun_size;
	long lc_size;
	long ss2_size;
	int width = animation.header.width;
	int height = animation.header.height;
	animation.field_C = animation.chunkdata;
	max_chunk_size = anim_buffer_size(width,height,animation.header.depth);
	memset(animation.chunkdata, 0, max_chunk_size);
	animation.prefix.ctype = 0xF1FAu;
	animation.prefix.nchunks = 0;
	animation.prefix.csize = 0;
	memset(animation.prefix.reserved, 0, sizeof(animation.prefix.reserved));
	AnimFLIPrefix *prefx = (AnimFLIPrefix *)animation.field_C;
	anim_store_data(&animation.prefix, sizeof(AnimFLIPrefix));
	animation.subchunk.ctype = 0;
	animation.subchunk.csize = 0;
	AnimFLIChunk *subchnk = (AnimFLIChunk *)animation.field_C;
	anim_store_data(&animation.subchunk, sizeof(AnimFLIChunk));
	if ( animation.field_31C == 0 ) {
		animation.header.oframe1 = animation.header.dsize;
	} else if ( animation.field_31C == 1 ) {
		animation.header.oframe2 = animation.header.dsize;
	}
	if ( anim_make_FLI_COLOUR256(palette) ) {
		prefx->nchunks++;
		subchnk->ctype = 4;
		subchnk->csize = animation.field_C-(unsigned char *)subchnk;
		animation.subchunk.ctype = 0;
		animation.subchunk.csize = 0;
		subchnk = (AnimFLIChunk *)animation.field_C;
		anim_store_data(&animation.subchunk, sizeof(AnimFLIChunk));
	}
	int scrpoints = animation.header.height * (long)animation.header.width;
	if (animation.field_31C == 0) {
		if ( anim_make_FLI_BRUN(screenbuf) ) {
			prefx->nchunks++;
			subchnk->ctype = FLI_BRUN;
		} else {
			anim_make_FLI_COPY(screenbuf);
			prefx->nchunks++;
			subchnk->ctype = FLI_COPY;
		}
	} else {
		// Determining the best compression method
		dataptr = animation.field_C;
		brun_size = anim_make_FLI_BRUN(screenbuf);
		memset(dataptr, 0, brun_size);
		animation.field_C = dataptr;
		ss2_size = anim_make_FLI_SS2(screenbuf, animation.videobuf);
		memset(dataptr, 0, ss2_size);
		animation.field_C = dataptr;
		lc_size = anim_make_FLI_LC(screenbuf, animation.videobuf);
		if ((lc_size < ss2_size) && (lc_size < brun_size)) {
			// Store the LC compressed data
			prefx->nchunks++;
			subchnk->ctype = FLI_LC;
		} else if (ss2_size < brun_size) {
			// Clear the LC compressed data
			memset(dataptr, 0, lc_size);
			animation.field_C = dataptr;
			// Compress with SS2 method
			anim_make_FLI_SS2(screenbuf, animation.videobuf);
			prefx->nchunks++;
			subchnk->ctype = FLI_SS2;
		} else if ( brun_size < scrpoints+16 ) {
			// Clear the LC compressed data
			memset(dataptr, 0, lc_size);
			animation.field_C = dataptr;
			// Compress with BRUN method
			anim_make_FLI_BRUN(screenbuf);
			prefx->nchunks++;
			subchnk->ctype = FLI_BRUN;
		} else {
			// Clear the LC compressed data
			memset(dataptr, 0, lc_size);
			animation.field_C = dataptr;
			// Store uncompressed frame data
			anim_make_FLI_COPY(screenbuf);
			prefx->nchunks++;
			subchnk->ctype = FLI_COPY;
		}
	}
	subchnk->csize = animation.field_C-(unsigned char *)subchnk;
	prefx->csize = animation.field_C - animation.chunkdata;
	if ( !anim_write_data(animation.chunkdata, animation.field_C-animation.chunkdata) ) {
		//LbSyncLog("Finished frame w/error.\n");
		return false;
	}
	memcpy(animation.videobuf, screenbuf, height*width);
	memcpy(animation.palette, palette, sizeof(animation.palette));
	animation.header.frames++;
	animation.field_31C++;
	animation.header.dsize += animation.field_C-animation.chunkdata;
	//LbSyncLog("Finished frame ok.\n");
	return true;
}

} // local

extern "C" short anim_stop()
{
	SYNCLOG("Finishing movie recording.");
	if ( ((animation.field_0 & 0x01)==0) || (!animation.outfhndl)) {
	  ERRORLOG("Can't stop recording movie");
	  return false;
	}
	LbFileSeek(animation.outfhndl, 0, Lb_FILE_SEEK_BEGINNING);
	animation.header.frames--;
	LbFileWrite(animation.outfhndl, &animation.header, sizeof(AnimFLIHeader));
	if ( LbFileClose(animation.outfhndl) == -1 ) {
		ERRORLOG("Can't close movie file");
		return false;
	}
	animation.outfhndl = nullptr;
	free(animation.chunkdata);
	animation.chunkdata=NULL;
	animation.field_0 = 0;
	return true;
}

extern "C" TbBool anim_record_frame(unsigned char *screenbuf, unsigned char *palette)
{
	if ((animation.field_0 & 0x01)==0) {
		return false;
	} else if (!anim_format_matches(MyScreenWidth/pixel_size,MyScreenHeight/pixel_size,LbGraphicsScreenBPP())) {
		return false;
	}
	return anim_make_next_frame(screenbuf, palette);
}

extern "C" short anim_record()
{
	SYNCDBG(7,"Starting");
	static char finalname[255];
	if (LbGraphicsScreenBPP() != 8) {
		ERRORLOG("Cannot record movie in non-8bit screen mode");
		return 0;
	}
	int idx;
	for (idx=0; idx < 10000; idx++) {
		sprintf(finalname, "%s/game%04d.flc","scrshots",idx);
		if (LbFileExists(finalname)) {
			continue;
		}
		return anim_open(finalname, 0, 0, MyScreenWidth/pixel_size,MyScreenHeight/pixel_size,8, 1);
	}
	ERRORLOG("No free file name for recorded movie");
	return 0;
}
