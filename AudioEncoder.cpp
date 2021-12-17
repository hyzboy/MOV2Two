#include "AudioEncoder.h"
#include "VideoEncoder.h"

AudioEncoder::AudioEncoder()
{

}

AudioEncoder::~AudioEncoder()
{
	Dispose();
}

bool AudioEncoder::CreateEncoder(AudioParams* audio_params_tgt)
{
	AVCodecID codec_id = (AVCodecID)audio_params_tgt->codec_id;

	AVCodec *audioEnCoder = NULL;
	audioEnCoder = avcodec_find_encoder(codec_id);
	if (audioEnCoder == NULL)
		return false;

	pOutputCodecCtx = avcodec_alloc_context3(audioEnCoder);
	if (pOutputCodecCtx == NULL)
		return false;

	pOutputCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;

	pOutputCodecCtx->codec_id = codec_id;
	pOutputCodecCtx->bit_rate = audio_params_tgt->codec_rate;
	pOutputCodecCtx->sample_rate = audio_params_tgt->freq;
	pOutputCodecCtx->channel_layout = audio_params_tgt->channel_layout;
	pOutputCodecCtx->sample_fmt = audio_params_tgt->fmt;

	pOutputCodecCtx->channels = av_get_channel_layout_nb_channels(pOutputCodecCtx->channel_layout);
	pOutputCodecCtx->frame_size = audio_params_tgt->frame_size;

	AVRational time_base = { 1, pOutputCodecCtx->sample_rate };
	pOutputCodecCtx->time_base = time_base;
	pOutputCodecCtx->block_align = 0;
	pOutputCodecCtx->codec_tag = 0;

	if (audioEnCoder->id == AV_CODEC_ID_AAC)
	{
		pOutputCodecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	}

	pOutputCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	if (avcodec_open2(pOutputCodecCtx, pOutputCodecCtx->codec, 0) < 0)
	{
		return false;
	}

	return true;
}

bool AudioEncoder::Init()
{
	audio_params_tgt = new AudioParams();
	audio_params_tgt->codec_id = AV_CODEC_ID_AAC;
	audio_params_tgt->codec_rate = 120000;
	audio_params_tgt->freq = 48000;
	audio_params_tgt->channel_layout = AV_CH_LAYOUT_STEREO;
	audio_params_tgt->fmt = AV_SAMPLE_FMT_FLTP;
	audio_params_tgt->frame_size = 1024;
	
	return CreateEncoder(audio_params_tgt);
}

bool AudioEncoder::EncoderFrame(AVFrame* frame)
{
	AVFrame *frame_in    = frame;
	
	AVPacket *pkt_out,*pkt_out2;
	bool result = true;

	//±àÂë
	int ret = avcodec_send_frame(pOutputCodecCtx, frame_in);

	if (ret < 0)
		goto __END;

	pkt_out=av_packet_alloc();
	pkt_out2=av_packet_alloc();

	ret = avcodec_receive_packet(pOutputCodecCtx, pkt_out);
	if (ret != 0)
	{
		av_packet_unref(pkt_out);
		goto __END;
	}

	pkt_out->pts = m_count++;

	av_packet_ref(pkt_out2, pkt_out);

	if (video_encoder1)
	{
		video_encoder1->WriteFrame(pkt_out);
	}
	if (video_encoder2)
	{
		video_encoder2->WriteFrame(pkt_out2);
	}

	av_packet_unref(pkt_out);
	av_packet_unref(pkt_out2);
	
__END:
	
	av_frame_unref(frame_in);
	av_frame_free(&frame_in);

	return result;
}

void AudioEncoder::Dispose()
{
	if (pOutputCodecCtx)
	{
		avcodec_close(pOutputCodecCtx);
		avcodec_free_context(&pOutputCodecCtx);
	}

	if (audio_params_tgt)
	{
		delete audio_params_tgt;
	}

	pOutputCodecCtx = nullptr;
	audio_params_tgt = nullptr;
}
