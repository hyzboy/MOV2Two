#include "AudioDecoder.h"
#include "AudioFrameResize.h"
#include "AudioResample.h"
#include "AudioEncoder.h"

AudioDecoder::AudioDecoder()
{

}

AudioDecoder::~AudioDecoder()
{
	if (pInputCodecCtx)
	{
		avcodec_close(pInputCodecCtx);
		avcodec_free_context(&pInputCodecCtx);
	}

	if (audioResample)
	{
		delete audioResample;
		audioResample = nullptr;
	}

	if (audioFrameResize)
	{
		delete audioFrameResize;
		audioFrameResize = nullptr;
	}

	pFmtInputCtx = nullptr;
	pAudioInputFmt = nullptr;
	pInputCodecCtx = nullptr;
}

bool AudioDecoder::init(AVFormatContext* fmtCtx, int index)
{
	audioIndex = index;
	pFmtInputCtx = fmtCtx;
	//打开音频解码器
	pInputCodecCtx = avcodec_alloc_context3(NULL);
	if (avcodec_parameters_to_context(pInputCodecCtx, pFmtInputCtx->streams[audioIndex]->codecpar) < 0)
	{
		return false;
	}

	AVCodec* decodec = avcodec_find_decoder(pInputCodecCtx->codec_id);
	if (avcodec_open2(pInputCodecCtx, decodec, NULL) < 0)
	{
		return false;
	}

	if (!pInputCodecCtx->channel_layout)
		pInputCodecCtx->channel_layout = av_get_default_channel_layout(pInputCodecCtx->channels);

	return true;
}

bool AudioDecoder::CreateAudioProcesser(AVCodecContext* outCodecCtx)
{
	pOutputCodecCtx = outCodecCtx;
	audioResample = CreateAudioReSample(pInputCodecCtx, outCodecCtx);
	if (!audioResample)
		return false;

	audioFrameResize = CreateAudioFrameResize(outCodecCtx);
	if (!audioFrameResize)
		return false;

	return true;
}

CAudioResample* AudioDecoder::CreateAudioReSample(AVCodecContext* inCodecCtx, AVCodecContext* outCodecCtx)
{
	CAudioResample* _audioresample = new CAudioResample();
	if (!_audioresample->InitFilter(inCodecCtx, outCodecCtx))
	{
		delete _audioresample;
		return nullptr;
	}

	return _audioresample;
}

CAudioFrameResize* AudioDecoder::CreateAudioFrameResize(AVCodecContext* outCodecCtx)
{
	CAudioFrameResize* _audioframeresize = new CAudioFrameResize();
	if (_audioframeresize->Init(outCodecCtx) < 0)
	{
		delete _audioframeresize;
		return nullptr;
	}
	return _audioframeresize;
}

//原始音频数据重采样，重组帧
bool AudioDecoder::AudioDataProcess(AVFrame* frame_in)
{
	AVFrame* frameFilter = NULL;

	int ret = 0;
	//原始音频数据格式与目标数据格式不一致时重采样
	if (pInputCodecCtx->sample_fmt != pOutputCodecCtx->sample_fmt ||
		pInputCodecCtx->channels != pOutputCodecCtx->channels ||
		pInputCodecCtx->sample_rate != pOutputCodecCtx->sample_rate)
	{
		frameFilter = av_frame_alloc();

		ret = audioResample->AudioResample_Frame(frame_in, 0, frameFilter);
		av_frame_unref(frame_in);
		if (ret < 0)
		{
			av_frame_unref(frameFilter);
			av_frame_free(&frameFilter);
			return false;
		}
	}
	else
	{
		frameFilter = frame_in;
	}

	//重组帧大小
	ret = audioFrameResize->WriteFrame(frameFilter);
	if (frameFilter != frame_in)
	{
		av_frame_free(&frameFilter);
	}

	if (ret < 0)
		return false;

	return true;
}

void AudioDecoder::DecodePackete(AVPacket* pkt)
{
	AVFrame* frame_in = av_frame_alloc();

	int ret = 0;
	if ((ret = avcodec_send_packet(pInputCodecCtx, pkt)) != 0)
	{
		av_packet_unref(pkt);
	}

	if (ret >= 0 && (ret = avcodec_receive_frame(pInputCodecCtx, frame_in)) >= 0)
	{
		if (AudioDataProcess(frame_in))
		{
			AVFrame* frame = av_frame_alloc();
			//读取缓冲区数据
			while (audioFrameResize->ReadFrame(frame) > 0)
			{
				if (m_encoder)
				{
					m_encoder->EncoderFrame(frame);
				}
			}
		}
	}

	av_frame_free(&frame_in);
	av_packet_unref(pkt);
}
