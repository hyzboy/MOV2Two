#ifndef __AUDIOPARAMS__
#define __AUDIOPARAMS__

/*
* 封装音频参数
*/
class AudioParams
{
public:
	int codec_id;              //音频编码id
	long codec_rate;           //音频编码码率
	int freq;                  //音频采样率
	int channels;              //音频声道数
	long channel_layout;       //音频声道布局
	enum AVSampleFormat fmt;   //音频格式
	int frame_size;
};

#endif
