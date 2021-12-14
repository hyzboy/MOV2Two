#ifndef __AUDIOENCODER__
#define __AUDIOENCODER__

#include "AudioParams.h"
extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat/avformat.h>
};
/*
* 音频解码器
*/
class VideoEncoder;
class AudioEncoder
{
private:
	//音频目标参数
	AudioParams* audio_params_tgt          = nullptr;   

	//音频编码器
	AVCodecContext*     pOutputCodecCtx    = nullptr;  

	int m_count = -1;

	VideoEncoder* video_encoder1 = nullptr;
	VideoEncoder* video_encoder2 = nullptr;

public:
	AudioEncoder();
	virtual ~AudioEncoder();

	////设置推流器
	//virtual void SetPlugFlow(IPlugFlow* flug) { m_XPlugFlowBase = flug; };
	virtual AVCodecContext* GetEncoderCtx() { return pOutputCodecCtx; };

	//编码一帧视频数据
	virtual bool EncoderFrame(AVFrame* frame);

	//释放资源
	virtual void Dispose();

public:
	//初始化音频编码器
	bool Init();

	void SetVideoEncoder(VideoEncoder* encoder1, VideoEncoder* encoder2)
	{
		video_encoder1 = encoder1;

		video_encoder2 = encoder2;
	}

private:
	//创建音频编码器
	bool CreateEncoder(AudioParams* audio_params_tgt);
};

#endif
