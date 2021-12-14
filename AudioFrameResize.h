#ifndef __AUDIOFRAMERESIZE__
#define __AUDIOFRAMERESIZE__
/*
*  使用fifo重置不同格式的音频帧大小
*/


extern "C"
{
#include <libavcodec\avcodec.h>
#include "libavutil/audio_fifo.h"
};


class CAudioFrameResize
{
public:
	CAudioFrameResize();
	~CAudioFrameResize();

public:
	int Init(AVCodecContext* dec_ctx);
	int WriteFrame(AVFrame* frame);
	int ReadFrame(AVFrame* frame);

private:
	AVCodecContext* mOutAVcodec_ctx;
	AVAudioFifo* mFifo;
};

#endif

