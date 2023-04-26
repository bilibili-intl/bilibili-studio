#include <obs-module.h>
#include <util/platform.h>

#define do_log(level, format, ...) \
	blog(level, "[mic-record-filter: '%s'] " format, \
			obs_source_get_name(rd->context), ##__VA_ARGS__)

#define warn(format, ...)  do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...)  do_log(LOG_INFO,    format, ##__VA_ARGS__)

struct wave_tag
{
	//资源交换文件标志
	char riff[4];
	//文件长度
	int file_size;
	//文件标志
	char wave[4];
};

struct wave_fmt
{
	//波形格式标志
	char fmt[4];
	//文件内部信息格式大小
	int chunsize;
	//音频数据编码方式，1为PCM
	short format_tag;
	//声道数
	short channel;
	//采样率
	int sample_rate;
	//比特率 采样率*声道数*位/8
	int byte_rate;
	//声道数*位/8
	short block_align;
	//位
	short bits_per_sample;
};

struct wave_data
{
	//数据标志位
	char data[4];
	//数据总长度
	int data_size;
};

struct wave_head
{
	struct wave_tag tag;
	struct wave_fmt fmt;
	struct wave_data data;
};

struct mic_record_data {
	obs_source_t* context;
	bool is_recording;
	bool is_start;
	bool is_end;
	FILE* file;
	int64_t file_len;
	struct wave_head head;
};

static inline int16_t FloatToS16(float v) {
	if (v > 0)
		return v >= 1 ? INT16_MAX
		: (int16_t)(v * INT16_MAX + 0.5f);
	return v <= -1 ? INT16_MIN
		: (int16_t)(-v * INT16_MIN - 0.5f);
}


static const char* mic_record_name(void* unused)
{
	return obs_module_text("mic record filter");
}

static void mic_record_update(void* data, obs_data_t* s)
{
	struct mic_record_data* md = data;
	if (md && s)
	{
		md->is_start = obs_data_get_bool(s, "is_start");
		if (md->is_start)
		{
			md->file_len = 0;
			const char* record_file_path = obs_data_get_string(s, "record_file_path");
			if (strlen(record_file_path) > 0)
			{
				md->file = os_fopen(record_file_path, "wb+");
				if (md->file)
				{
					md->head.tag.riff[0] = 'R';
					md->head.tag.riff[1] = 'I';
					md->head.tag.riff[2] = 'F';
					md->head.tag.riff[3] = 'F';
					md->head.tag.wave[0] = 'W';
					md->head.tag.wave[1] = 'A';
					md->head.tag.wave[2] = 'V';
					md->head.tag.wave[3] = 'E';
					md->head.fmt.fmt[0] = 'f';
					md->head.fmt.fmt[1] = 'm';
					md->head.fmt.fmt[2] = 't';
					md->head.fmt.fmt[3] = ' ';
					md->head.fmt.chunsize = 16;
					md->head.fmt.format_tag = 1;
					md->head.fmt.channel = (short)audio_output_get_channels(obs_get_audio());
					md->head.fmt.sample_rate = audio_output_get_sample_rate(obs_get_audio());
					md->head.fmt.byte_rate = md->head.fmt.sample_rate * md->head.fmt.channel* 16 /8;
					md->head.fmt.block_align = md->head.fmt.channel * 16 / 8;
					md->head.fmt.bits_per_sample = 16;

					md->head.data.data[0] = 'd';
					md->head.data.data[1] = 'a';
					md->head.data.data[2] = 't';
					md->head.data.data[3] = 'a';

					fseek(md->file, sizeof(md->head), SEEK_SET);

					md->is_recording = true;
				}
			}

		}

		md->is_end = obs_data_get_bool(s, "is_end");
		if (md->is_end)
		{
			if (md->file)
			{
				md->is_recording = false;
				fseek(md->file, 0, SEEK_SET);
				md->head.data.data_size = md->file_len;
				md->head.tag.file_size = md->file_len + sizeof(md->head);
				fwrite(&md->head, sizeof(md->head), 1, md->file);
				fclose(md->file);
				md->file_len = 0;
			}
		}
	}

}

static void* mic_record_create(obs_data_t* settings, obs_source_t* filter)
{
	struct mic_record_data* md = bzalloc(sizeof(*md));
	if (md)
	{
		md->context = filter;
		md->is_recording = false;
		mic_record_update(md, settings);
	}
	return md;
}

static void mic_record_destroy(void* data)
{
	struct mic_record_data* md = data;
	if (md)
	{
		bfree(md);
	}
}


static struct obs_audio_data* mic_record_filter_audio(void* data, struct obs_audio_data* audio)
{
	struct mic_record_data* md = data;
	if (md && md->is_recording && audio && md->file)
	{
		float* adata[2] = { (float*)audio->data[0], (float*)audio->data[1] };
		if (adata[0] && adata[1])
		{
			for (int i = 0; i < audio->frames; i++)
			{
				int16_t left = FloatToS16(*(adata[0] + i));
				int16_t right = FloatToS16(*(adata[1] + i));
				fwrite(&left, sizeof(int16_t), 1, md->file);
				fwrite(&right, sizeof(int16_t), 1, md->file);
			}
			md->file_len += audio->frames * sizeof(int16_t) * 2;
		}
		else if (adata[0] && !adata[1])
		{
			for (int i = 0; i < audio->frames; i++)
			{
				int16_t temp = FloatToS16(*(adata[0] + i));
				fwrite(&temp, sizeof(int16_t), 1, md->file);
			}
			md->file_len += audio->frames * sizeof(int16_t);
		}
	}
	return audio;
}

static void mic_record_defaults(obs_data_t* s)
{
	if (s)
	{
		obs_data_set_default_bool(s, "is_start", false);
		obs_data_set_default_bool(s, "is_end", false);
		obs_data_set_default_string(s, "record_file_path", "");
	}
}

static obs_properties_t* mic_record_properties(void* data)
{
	obs_properties_t* ppts = obs_properties_create();
	return ppts;
}

struct obs_source_info mic_record_filter = {
	.id = "mic_record_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = mic_record_name,
	.create = mic_record_create,
	.destroy = mic_record_destroy,
	.update = mic_record_update,
	.filter_audio = mic_record_filter_audio,
	.get_defaults = mic_record_defaults,
	.get_properties = mic_record_properties,
};