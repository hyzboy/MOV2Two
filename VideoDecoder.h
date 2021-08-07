#pragma once

#include"Bitmap.h"
extern "C"
{
    #include<libavcodec/avcodec.h>
}

class FrameRecviver;

/**
 * 解码器
 */
class VideoDecoder
{
protected:

    int width,height;
    double fps,frame_time;

public:

    const int       GetWidth    ()const{return width;}
    const int       GetHeight   ()const{return height;}
    const double    GetFPS      ()const{return fps;}
    const double    GetFrameTime()const{return frame_time;}

    virtual const AVRational &GetFrameRate()=0;

public:

    virtual ~VideoDecoder()=default;

    virtual void Start()=0;                                                     ///<开始解码
    virtual bool NextFrame()=0;                                                 ///<获取一帧图片

    void Abort();                                                               ///<强退
};//class VideoDecoder

VideoDecoder *CreateVideoDecoder(const char *,FrameRecviver *);
