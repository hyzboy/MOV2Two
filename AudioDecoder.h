#pragma once
extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat/avformat.h>
};

class CAudioResample;
class CAudioFrameResize;
class AudioEncoder;
/// <summary>
///音频解码器
/// </summary>
class AudioDecoder
{
private:
	AVFormatContext* pFmtInputCtx = nullptr;  //音频输入解封装上下文

	AVInputFormat* pAudioInputFmt = nullptr;  //音频输入样式

	AVCodecContext* pInputCodecCtx = nullptr;  //音频解码器

	AVCodecContext* pOutputCodecCtx = nullptr;  //音频编码器

	//音频采样器
	CAudioResample* audioResample = nullptr;

	//音频帧重置器
	CAudioFrameResize* audioFrameResize = nullptr;

	int  audioIndex = -1;       //音频流序号

	AudioEncoder* m_encoder = nullptr;

public:
	AudioDecoder();

	~AudioDecoder();

	bool init(AVFormatContext* fmtCtx,int index);

	void SetEncoder(AudioEncoder* encoder) { m_encoder = encoder; };

	AVCodecContext* GetEncoderCtx() { return pInputCodecCtx; };

	bool CreateAudioProcesser(AVCodecContext* outCodecCtx);

	void DecodePackete(AVPacket* pkt);

private:
	//创建音频重采样对象
	CAudioResample* CreateAudioReSample(AVCodecContext* inCodecCtx, AVCodecContext* outCodecCtx);

	//创建音频缓冲区
	CAudioFrameResize* CreateAudioFrameResize(AVCodecContext* outCodecCtx);

	//原始音频数据重采样，重组帧
	bool AudioDataProcess(AVFrame* frame_in);
};

