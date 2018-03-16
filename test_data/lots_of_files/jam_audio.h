/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Audio Layer.
 */

struct Sound{
	i16 *samples;
	i32 channels;
	i32 samples_per_second;
	i32 bytes_per_sample;
	i32 memory_size;
	i32 sample_count;
	real32 scan_speed;
};

#pragma pack(push, 1)
struct Wav_Chunk{
	i32 id;
	i32 size;
};

struct Wav_Header{
	Wav_Chunk chunk;
	i32 wav_id;
};

struct Wav_Format_Chunk_16{
	Wav_Chunk chunk;
	i16 format_code;
	i16 channels;
	i32 samples_per_second;
	i32 avg_bytes_per_second;
	i16 block_align;
	i16 bits_per_sample;
};

struct Wav_Format_Chunk_18{
	Wav_Format_Chunk_16 s;
	i16 extension_size;
};

struct Wav_Format_Chunk_40{
	Wav_Format_Chunk_16 s;
	i16 extension_size;
	i16 valid_bits_per_sample;
	i32 channel_mask;
	u8 guid[16];
};

struct Wav_Fact_Chunk{
	Wav_Chunk chunk;
	i32 samples_per_channel;
};

struct Wav_Data_Chunk{
	Wav_Chunk chunk;
};

#define WAV_CHUNK_RIFF 0x46464952
#define WAV_CHUNK_WAVE 0x45564157
#define WAV_CHUNK_fmt 0x20746D66
#define WAV_CHUNK_fact 0x74666163
#define WAV_CHUNK_data 0x61746164

#define WAV_FORMAT_PCM 0x0001

#pragma pack(pop)

struct Wav_File{
	File file;
	Wav_Header header;
};

