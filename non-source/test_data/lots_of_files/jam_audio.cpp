/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Audio Layer.
 */

internal void
wav_open_file(char *filename, Wav_File *wav_file){
	File file = system_load_file(filename);
	if (file.data && file.size > sizeof(Wav_File)){
		*wav_file = {};
		wav_file->file = file;
		wav_file->header = *(Wav_Header*)file.data;
		
		Assert(wav_file->header.chunk.id == WAV_CHUNK_RIFF);
		Assert(wav_file->header.wav_id == WAV_CHUNK_WAVE);
	}
}

internal i32
wav_data_requirement(Wav_File *wav_file){
	u8 *cursor = (u8*)wav_file->file.data + sizeof(Wav_Header);
	u64 size = wav_file->file.size;
	u8 *end = (u8*)wav_file->file.data + size;
	
	i32 requirement = 0;
	i32 channels = 0;
	
	while (1){
		Wav_Chunk chunk;
		chunk = *(Wav_Chunk*)cursor;
		
		if (chunk.id == WAV_CHUNK_data){
			Assert(requirement == 0);
			requirement = chunk.size;
			cursor += sizeof(Wav_Chunk) + chunk.size;
			if (chunk.size % 2 == 1){
				++cursor;
			}
		}
		else{
			if (chunk.id == WAV_CHUNK_fmt){
				Wav_Format_Chunk_16 fmt;
				fmt = *(Wav_Format_Chunk_16*)(cursor);
				channels = fmt.channels;
			}
			cursor += sizeof(Wav_Chunk) + chunk.size;
		}
		
		if (cursor >= end){
			break;
		}
	}
	
	return requirement*(2/channels) + 4;
}

internal bool32
wav_fill_sound(Wav_File *wav_file, Sound *sound,
			   i32 system_sample_per_second){
	u8 *cursor = (u8*)wav_file->file.data + sizeof(Wav_Header);
	u64 size = wav_file->file.size;
	u8 *end = (u8*)wav_file->file.data + size;
	
	bool32 found_data = 0;
	
	while (1){
		Wav_Chunk chunk;
		chunk = *(Wav_Chunk*)cursor;
		
		if (chunk.id == WAV_CHUNK_data){
			Assert(found_data == 0);
			found_data = 1;
			sound->memory_size = chunk.size*(2/sound->channels);
			sound->sample_count = (chunk.size/sound->channels)/sound->bytes_per_sample;
			
			i16 *sample_src = (i16*)(cursor + sizeof(Wav_Chunk));
			i16 *sample_dest = sound->samples;
			if (sound->channels == 1){
				for (i32 i = 0; i < sound->sample_count; ++i){
					*sample_dest++ = *sample_src;
					*sample_dest++ = *sample_src++;
				}
				*sample_dest++ = 0;
				*sample_dest++ = 0;
			}
			else{
				for (i32 i = 0; i < sound->sample_count; ++i){
					*sample_dest++ = *sample_src++;
					*sample_dest++ = *sample_src++;
				}
				*sample_dest++ = 0;
				*sample_dest++ = 0;
			}
			cursor += sizeof(Wav_Chunk) + chunk.size;
			if (chunk.size % 2 == 1){
				++cursor;
			}
		}
		
		else{
			if (chunk.id == WAV_CHUNK_fmt){
				Wav_Format_Chunk_16 fmt;
				fmt = *(Wav_Format_Chunk_16*)(cursor);
				
				Assert(fmt.channels == 1 || fmt.channels == 2);
				
				sound->channels = fmt.channels;
				sound->samples_per_second = fmt.samples_per_second;
				sound->bytes_per_sample = fmt.bits_per_sample >> 3;
				sound->scan_speed = ((real32)sound->samples_per_second/system_sample_per_second);
			}
			cursor += sizeof(Wav_Chunk) + chunk.size;
		}
		
		if (cursor >= end){
			break;
		}
	}

	return found_data;
}
