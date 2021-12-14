#pragma once

#include"Bitmap.h"
extern "C"
{
    #include<libavcodec/avcodec.h>
}

class FrameRecviver;
class AudioDecoder;
/**
 * 解码器
 */
class VideoDecoder
{
protected:
    AVFormatContext* ctx;

    int width,height;
    double fps,frame_time;
    
    int audio_stream_index = -1;

    AudioDecoder* audio_decoder = nullptr;

public:

    const int       GetWidth    ()const{return width;}
    const int       GetHeight   ()const{return height;}
    const double    GetFPS      ()const{return fps;}
    const double    GetFrameTime()const{return frame_time;}
    const int       GetAudioIndex()const { return audio_stream_index; };

    AVFormatContext* GetFrmCtx() {
        return ctx
            ;
    };

    void SetAudioDecoder(AudioDecoder* audiodecoder) { audio_decoder = audiodecoder; };

    virtual const AVRational &GetFrameRate()=0;

    virtual const AVRational GetAudioTimeBase() = 0;

public:

    virtual ~VideoDecoder()=default;

    virtual void Start()=0;                                                     ///<开始解码
    virtual bool NextFrame()=0;                                                 ///<获取一帧图片

    void Abort();                                                               ///<强退
};//class VideoDecoder

VideoDecoder *CreateVideoDecoder(const char *,FrameRecviver *,const bool);
