#pragma once
#include"DataType.h"
extern "C"
{
    #include<libavcodec/avcodec.h>
}

class EncodeOutput
{
public:

    virtual ~EncodeOutput()=default;

    virtual bool Write(const void *,const uint)=0;
};//class EncodeOutput

class VideoEncoder
{
protected:

    uint width;
    uint height;
    AVRational frame_rate;
    uint bit_rate;

    EncodeOutput *output;

public:

    VideoEncoder(EncodeOutput *eo,const uint br)
    {
        width=height=0;
        bit_rate=br;

        output=eo;
    }

    virtual ~VideoEncoder()=default;

    virtual void Set(const uint w,const uint h,const AVRational &fr)
    {
        width=w;
        height=h;
        frame_rate=fr;
    }

    virtual bool Init()=0;

    virtual bool Put(const uint8 *)=0;

    virtual bool Finish()=0;
};//class VideoEncoder

VideoEncoder *CreateVideoEncoder(EncodeOutput *eo,const uint bit_rate);